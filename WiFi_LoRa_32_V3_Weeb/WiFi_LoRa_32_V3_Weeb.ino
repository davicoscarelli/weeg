#include <DNSServer.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include <AsyncWebSocket.h>
#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "HT_SSD1306Wire.h"
#include <Wire.h>
#include "Images.h"
#include <ArduinoJson.h>
#include "FS.h"
#include "LittleFS.h"

#define RF_FREQUENCY 433000000 // Hz

#define TX_OUTPUT_POWER 15 // dBm

#define LORA_BANDWIDTH 0         // [0: 125 kHz,
                                 //  1: 250 kHz,
                                 //  2: 500 kHz,
                                 //  3: Reserved]
#define LORA_SPREADING_FACTOR 12 // [SF7..SF12]
#define LORA_CODINGRATE 1        // [1: 4/5,
                                 //  2: 4/6,
                                 //  3: 4/7,
                                 //  4: 4/8]
#define LORA_PREAMBLE_LENGTH 8   // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT 0    // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON false
#define LORA_IQ_INVERSION_ON false

#define RX_TIMEOUT_VALUE 1000
#define BUFFER_SIZE 30

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];
char Repacket[BUFFER_SIZE];
char serialData[BUFFER_SIZE];
char lastResponse[BUFFER_SIZE];

const char *apiKey = "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";

StaticJsonDocument<8192> chatHistory;
JsonArray messages = chatHistory.createNestedArray("messages");

const int responseBufferSize = 4096;
char responseBuffer[responseBufferSize];

char message_string[30];

static RadioEvents_t RadioEvents;
void OnTxDone(void);
void OnTxTimeout(void);
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
void OnRxTimeout(void);
void displayMcuInit();
void displaySendReceive();

const char *ssid = "WIFI";
const char *password = "12345678";

// Device type flag (1 for Worker mode, 0 for Nest mode)
int deviceType = 0;

typedef enum
{
    STATUS_LOWPOWER,
    STATUS_RX,
    STATUS_TX
} States_t;

int16_t txNumber;
States_t state;
int16_t Rssi, rxSize;

bool isConnected = false;

uint32_t license[4] = {ESP.getEfuseMac()};

extern SSD1306Wire display;

DNSServer dnsServer;
AsyncWebServer server(80);

AsyncWebSocket ws("/ws");

bool message_received = false;

class CaptiveRequestHandler : public AsyncWebHandler
{
public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request)
    {
        return true;
    }

    void handleRequest(AsyncWebServerRequest *request)
    {
        File file = SPIFFS.open("/hive_local.html", "r");
        equest->streamFile(file, "text/html");
    }
};

void setupServer()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        File file = SPIFFS.open("/hive_local.html", "r");
        if(file){
            request->streamFile(file, "text/html");
            file.close();
        } else {
            request->send(404, "text/plain", "File Not Found");
        } });

    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
}

const char *askOpenAI(const String &question)
{
    if (!chatHistory.containsKey("model"))
    {
        chatHistory["model"] = "gpt-3.5-turbo";
    }

    JsonObject userMessage = messages.createNestedObject();
    userMessage["role"] = "user";
    userMessage["content"] = question;

    String requestBody;
    serializeJson(chatHistory, requestBody);

    HTTPClient http;
    http.begin("https://api.openai.com/v1/chat/completions");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + String(apiKey));

    Serial.println("Request Body:");
    Serial.println(requestBody);

    int httpResponseCode = http.POST(requestBody);
    if (httpResponseCode > 0)
    {
        String response = http.getString();

        Serial.println("Response:");
        Serial.println(response);

        DynamicJsonDocument responseDoc(4096);
        deserializeJson(responseDoc, response);
        if (responseDoc.containsKey("choices"))
        {
            strncpy(responseBuffer, responseDoc["choices"][0]["message"]["content"], responseBufferSize - 1);
            responseBuffer[responseBufferSize - 1] = '\0';

            JsonObject aiMessage = messages.createNestedObject();
            aiMessage["role"] = "assistant";
            aiMessage["content"] = responseBuffer;
        }
        else
        {
            strncpy(responseBuffer, "Failed to parse OpenAI response.", responseBufferSize - 1);
            responseBuffer[responseBufferSize - 1] = '\0';
        }
    }
    else
    {
        Serial.print("HTTP Request failed: ");
        Serial.println(httpResponseCode);

        strncpy(responseBuffer, "Error in HTTP request", responseBufferSize - 1);
        responseBuffer[responseBufferSize - 1] = '\0';
    }

    http.end();
    return responseBuffer;
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
               void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        Serial.println("WebSocket client connected");
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        Serial.println("WebSocket client disconnected");
    }
    else if (type == WS_EVT_DATA)
    {
        data[len] = 0;
        Serial.printf("WebSocket message: %s\n", data);

        memcpy(serialData, data, len);
        serialData[len] = '\0';
    }
}

void setup()
{
    Serial.begin(115200);

    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    displayMcuInit();

    if (deviceType == 1)
    {
        Serial.println("Setting up AP Mode");
        WiFi.mode(WIFI_AP);
        WiFi.softAP("Weeg WiFi");
        Serial.print("AP IP address: ");
        Serial.println(WiFi.softAPIP());
        Serial.println("Setting up Async WebServer");
        setupServer();
        Serial.println("Starting DNS Server");
        dnsServer.start(53, "*", WiFi.softAPIP());
        server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
        server.begin();
        Serial.println("All Done!");
    }
    else if (deviceType == 0)
    {
        Serial.println("Connecting to WiFi");
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);

        while (WiFi.status() != WL_CONNECTED)
        {

            Serial.println(WiFi.status());
            delay(500);
            Serial.print(".");
            displaySendReceive();
        }
        Serial.println(WiFi.status());
        Serial.println("");
        Serial.print("Connected to WiFi. IP address: ");
        Serial.println(WiFi.localIP());
    }

    while (!Serial)
        ;
    SPI.begin(SCK, MISO, MOSI, SS);
    Mcu.begin();

    txNumber = 0;
    Rssi = 0;

    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.RxTimeout = OnRxTimeout;

    Radio.Init(&RadioEvents);
    Radio.SetChannel(RF_FREQUENCY);
    Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                      true, 0, 0, LORA_IQ_INVERSION_ON, 3000);

    Radio.SetRxConfig(MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                      LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                      0, true, 0, 0, LORA_IQ_INVERSION_ON, true);
    state = STATUS_TX;
}

void loop()
{
    delay(500);
    dnsServer.processNextRequest();
    if (message_received)
    {
        Serial.print("User request: ");
        Serial.println(message_string);
        message_received = false;

        strcpy(serialData, message_string);
    }

    switch (state)
    {
    case STATUS_TX:
        Serial.println("into TX mode");

        if (strlen(serialData) > 0)
        {

            sprintf(txpacket, serialData, BUFFER_SIZE - strlen(txpacket) - 1);
            Radio.Send((uint8_t *)txpacket, strlen(txpacket));
            memset(serialData, 0, sizeof(serialData));
        }
        else
        {
            sprintf(txpacket, "connected %d Rssi: %d", txNumber, Rssi);
            Radio.Send((uint8_t *)txpacket, strlen(txpacket));
        }

        Serial.printf("\r\nsending packet \"%s\" , length %d\r\n", txpacket, strlen(txpacket));

        state = STATUS_LOWPOWER;
        break;
    case STATUS_RX:

        Serial.println("into RX mode");
        Radio.Rx(5000);

        state = STATUS_LOWPOWER;
        break;
    case STATUS_LOWPOWER:

        displaySendReceive();
        LoRaWAN.sleep(CLASS_A);
        break;
    default:
        break;
    }
}

void OnTxDone(void)
{
    Serial.print("TX done......");
    sprintf(Repacket, "%s", "");

    state = STATUS_RX;
    displaySendReceive();
}

void OnTxTimeout(void)
{
    Radio.Sleep();
    Serial.print("TX Timeout......");
    sprintf(Repacket, "%s", "TX Timeout,Retransmission");
    isConnected = false;

    state = STATUS_RX;
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    memset(rxpacket, 0, sizeof(rxpacket));
    Serial.print("RX done......");
    Rssi = rssi;
    rxSize = size;

    memcpy(rxpacket, payload, size);
    rxpacket[size] = '\0';

    Serial.printf("\r\nreceived packet \"%s\" with Rssi %d, length %d\r\n", rxpacket, Rssi, rxSize);

    if (deviceType == 0)
    {

        if (strlen(rxpacket) > 0 && strstr(rxpacket, "connected") == NULL)
        {

            const char *response = askOpenAI(rxpacket);
            Serial.println(response);

            strcpy(serialData, response);
        }
    }
    else if (deviceType == 1)
    {
        if (strlen(rxpacket) > 0 && strstr(rxpacket, "connected") == NULL)
        {

            ws.textAll(rxpacket);
        }
    }
    isConnected = strstr(rxpacket, "connected") != NULL;

    state = STATUS_TX;

    displaySendReceive();
}

void OnRxTimeout(void)
{
    Radio.Sleep();
    Serial.print("RX Timeout......");
    sprintf(Repacket, "%s", "RX Timeout,Retransmission");
    isConnected = false;

    state = STATUS_TX;
}

void displayMcuInit()
{
    display.init();
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_RIGHT);

    display.clear();

    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.drawString(display.getWidth() / 2, display.getHeight() / 2, "Starting...");

    display.display();
}

void displaySendReceive()
{
    display.clear();

    if (deviceType == 0 && WiFi.status() != WL_CONNECTED)
    {

        display.drawXbm(0, 0, wifiDisconnectedSymbol_width, wifiDisconnectedSymbol_height, wifiDisconnectedSymbol);
    }
    else if (deviceType == 0 && WiFi.status() == WL_CONNECTED)
    {
        display.drawXbm(0, 0, wifiConnectedSymbol_width, wifiConnectedSymbol_height, wifiConnectedSymbol);
    }

    if (isConnected)
    {
        display.drawXbm(128 - connectedSymbol_width, 0, connectedSymbol_width, connectedSymbol_height, connectedSymbol);
    }
    else
    {
        display.drawXbm(128 - disconnectedSymbol_width, 0, disconnectedSymbol_width, disconnectedSymbol_height, disconnectedSymbol);
    }

    display.drawString((display.getWidth() / 2) + 12, (display.getHeight() / 2), "Weeg");

    display.drawXbm(25, (display.getHeight() / 2) - 4, logo_width, logo_height, logo_bits);

    display.setFont(ArialMT_Plain_16);
    display.setTextAlignment(TEXT_ALIGN_CENTER);

    if (strlen(rxpacket) > 0 && strstr(rxpacket, "connected") == NULL)
    {
        display.drawString(display.getWidth() / 2, display.getHeight() / 2, rxpacket);
    }
    else if (strlen(serialData) > 0)
    {
        char message[64];
        snprintf(message, sizeof(message), "Sending %s", serialData);
        display.drawString(display.getWidth() / 2, display.getHeight() / 2, message);
        memset(rxpacket, 0, sizeof(rxpacket));
    }

    display.display();
}
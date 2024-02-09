#include "Arduino.h"
#include "WiFi.h"
#include "HT_SSD1306Wire.h"
#include "LoRaWan_APP.h"
#include <Wire.h>  

// LoRa parameters
#define RF_FREQUENCY                                868000000 // Hz
#define TX_OUTPUT_POWER                             10        // dBm
#define LORA_BANDWIDTH                              0         // 125 kHz
#define LORA_SPREADING_FACTOR                       7         // SF7
#define LORA_CODINGRATE                             1         // 4/5
#define LORA_PREAMBLE_LENGTH                        8         
#define LORA_SYMBOL_TIMEOUT                         0         
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#define RX_TIMEOUT_VALUE                            1000
#define BUFFER_SIZE                                 30 // Define the payload size here

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;

void OnTxDone(void);
void OnTxTimeout(void);
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);

typedef enum {
    LOWPOWER,
    STATE_RX,
    STATE_TX
} States_t;

States_t state;

void OnTxDone(void) {
    Serial.println("OnTxDone: TX completed.");
    state = STATE_RX;
}

void OnTxTimeout(void) {
    Serial.println("OnTxTimeout: TX timeout.");
    state = STATE_TX;
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr) {
    memcpy(rxpacket, payload, size);
    rxpacket[size] = '\0'; // Ensure null-termination
    Serial.printf("OnRxDone: Received packet \"%s\" RSSI: %d Size: %d\n", rxpacket, rssi, size);
    state = STATE_TX;
}

void setupLoRa(void) {
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxDone = OnRxDone;

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
    state = STATE_TX;
}

void setup() {
    Serial.begin(115200);
    while (!Serial); // Wait for serial port to connect.
    Serial.println("LoRa Sender/Receiver");

    setupLoRa();
}

void loop() {
    switch (state) {
        case STATE_TX:
            sprintf(txpacket, "Hello LoRa %d", millis());
            Serial.printf("Sending packet: %s\n", txpacket);
            Radio.Send((uint8_t *)txpacket, strlen(txpacket));
            break;
        case STATE_RX:
            Serial.println("Switching to RX mode");
            Radio.Rx(RX_TIMEOUT_VALUE);
            break;
        case LOWPOWER:
            // Optional: Implement low power functionality
            break;
        default:
            break;
    }

    // Optional: Implement low power handling
}

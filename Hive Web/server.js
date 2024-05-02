const express = require('express');
const app = express();
const path = require('path');
require('dotenv').config();

const GOOGLE_API_KEY = "AIzaSyAQIVKrYAJdtp00GQJ2qFF17y5PVXAXPko";

const { GoogleGenerativeAI } = require("@google/generative-ai");

let conversations = {};

app.use(express.json());
app.use(express.static('public'));

app.post('/api/chat', async (req, res) => {
    const { sessionId, userMessage } = req.body;

    try {
        if (!conversations[sessionId]) {
            conversations[sessionId] = [
                {
                    "role": "system",
                    "content": "You are a helpful assistant."
                }
            ];
        }

        const genAI = new GoogleGenerativeAI(GOOGLE_API_KEY);
        const model = genAI.getGenerativeModel({ model: "gemini-pro" });

        const chat = model.startChat({
            history: conversations[sessionId],
            generationConfig: {
                maxOutputTokens: 100,
            },
        });

        const result = await chat.sendMessage(userMessage);
        const response = await result.response;
        const aiMessage = await response.text();

        conversations[sessionId].push({
            "role": "user",
            "content": userMessage
        });

        conversations[sessionId].push({
            "role": "assistant",
            "content": aiMessage
        });

        res.json({ aiMessage });
    } catch (error) {
        console.error('Error calling Gemini:', error);
        res.status(500).send('Error calling Gemini');
    }
});

app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => console.log(`Server running on port ${PORT}`));

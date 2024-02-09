const express = require('express');
const app = express();
const path = require('path');
require('dotenv').config();

const OPENAI_API_KEY = process.env.OPENAI_API_KEY;

let conversations = {};

app.use(express.json());
app.use(express.static('public'));

app.post('/api/chat', async (req, res) => {
    const { sessionId, userMessage } = req.body;


    if (!conversations[sessionId]) {
        conversations[sessionId] = [
            {
                "role": "system",
                "content": "You are a helpful assistant."
            }
        ];
    }


    conversations[sessionId].push({
        "role": "user",
        "content": userMessage
    });

    try {
        const fetch = (await import('node-fetch')).default;
        const response = await fetch('https://api.openai.com/v1/chat/completions', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'Authorization': `Bearer ${OPENAI_API_KEY}`
            },
            body: JSON.stringify({
                model: "gpt-3.5-turbo",
                messages: conversations[sessionId]
            })
        });

        const data = await response.json();

    
        const aiMessage = data.choices[0].message.content;
        conversations[sessionId].push({
            "role": "assistant",
            "content": aiMessage
        });

        res.json({ aiMessage });
    } catch (error) {
        console.error('Error calling OpenAI:', error);
        res.status(500).send('Error calling OpenAI');
    }
});

app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => console.log(`Server running on port ${PORT}`));

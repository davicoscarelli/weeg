# Hive Web Application

The Hive Web Application serves as an educational platform, leveraging the power of OpenAI's GPT models and Azure to provide interactive learning experiences. It is designed to work outside the Weeg ecosystem, offering a scalable and accessible AI-driven educational resource for non-isolated communities.

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites

Before you begin, ensure you have the following installed:
- [Node.js](https://nodejs.org/) (version 12 or later recommended)
- npm (comes with Node.js)
- An active internet connection for initial setup

### Installation

1. **Clone the Repository**

```bash
git clone https://github.com/yourusername/hive-web.git
cd hive-web
```

2. **Install Dependencies**

Run the following command in the project directory to install the necessary packages:

```bash
npm install
```

3. **Environment Configuration**

Create a `.env` file in the root of your project and add your OpenAI API key:

```env
OPENAI_API_KEY=<your_openai_api_key_here>
```

Replace `<your_openai_api_key_here>` with your actual OpenAI API key.

### Running the Application

To start the application, run:

```bash
node server.js
```

The application will be available at `http://localhost:3000`. Navigate to this URL in a web browser to interact with the Hive Web Application.

## Usage

- **Chat Interface**: The main feature is the chat interface where users can input questions or messages, and the AI responds accordingly.
- **Markdown and LaTeX Support**: Responses from the AI that include Markdown or LaTeX are properly formatted for an enhanced educational experience.

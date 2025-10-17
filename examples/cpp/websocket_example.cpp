/**
 * @file websocket_example.cpp
 * @brief WebSocket chat server example
 */

#include "crest/crest.hpp"
#include "crest/websocket.hpp"
#include <iostream>
#include <map>

int main() {
    crest::App app;
    crest::WebSocketServer ws;
    
    std::map<std::string, std::string> users;
    
    // Handle new connections
    ws.on_connect([&users](std::shared_ptr<crest::WebSocketConnection> conn) {
        std::cout << "[WebSocket] Client connected: " << conn->get_id() << std::endl;
        users[conn->get_id()] = "Anonymous";
        
        // Send welcome message
        conn->send(R"({
            "type": "welcome",
            "message": "Welcome to Crest Chat!",
            "id": ")" + conn->get_id() + R"("
        })");
    });
    
    // Handle incoming messages
    ws.on_message([&ws, &users](std::shared_ptr<crest::WebSocketConnection> conn, const std::string& message) {
        std::cout << "[WebSocket] Message from " << conn->get_id() << ": " << message << std::endl;
        
        // Parse message type (simplified JSON parsing)
        if (message.find("\"type\":\"join\"") != std::string::npos) {
            // Extract username (simplified)
            size_t pos = message.find("\"username\":\"");
            if (pos != std::string::npos) {
                pos += 12;
                size_t end = message.find("\"", pos);
                std::string username = message.substr(pos, end - pos);
                users[conn->get_id()] = username;
                
                // Broadcast join message
                ws.broadcast(R"({
                    "type": "join",
                    "username": ")" + username + R"(",
                    "message": ")" + username + R"( joined the chat"
                })");
            }
        }
        else if (message.find("\"type\":\"message\"") != std::string::npos) {
            // Extract message text (simplified)
            size_t pos = message.find("\"text\":\"");
            if (pos != std::string::npos) {
                pos += 8;
                size_t end = message.find("\"", pos);
                std::string text = message.substr(pos, end - pos);
                
                // Broadcast chat message
                ws.broadcast(R"({
                    "type": "message",
                    "username": ")" + users[conn->get_id()] + R"(",
                    "text": ")" + text + R"(",
                    "timestamp": ")" + std::to_string(std::time(nullptr)) + R"("
                })");
            }
        }
        else if (message.find("\"type\":\"typing\"") != std::string::npos) {
            // Broadcast typing indicator
            ws.broadcast(R"({
                "type": "typing",
                "username": ")" + users[conn->get_id()] + R"("
            })");
        }
    });
    
    // Handle binary messages
    ws.on_binary([](std::shared_ptr<crest::WebSocketConnection> conn, const std::vector<uint8_t>& data) {
        std::cout << "[WebSocket] Binary data received: " << data.size() << " bytes" << std::endl;
    });
    
    // Handle disconnections
    ws.on_close([&ws, &users](std::shared_ptr<crest::WebSocketConnection> conn, int code, const std::string& reason) {
        std::cout << "[WebSocket] Client disconnected: " << conn->get_id() 
                  << " (code: " << code << ", reason: " << reason << ")" << std::endl;
        
        std::string username = users[conn->get_id()];
        users.erase(conn->get_id());
        
        // Broadcast leave message
        ws.broadcast(R"({
            "type": "leave",
            "username": ")" + username + R"(",
            "message": ")" + username + R"( left the chat"
        })");
    });
    
    // Handle errors
    ws.on_error([](std::shared_ptr<crest::WebSocketConnection> conn, const std::string& error) {
        std::cerr << "[WebSocket] Error on " << conn->get_id() << ": " << error << std::endl;
    });
    
    // HTTP endpoint to serve chat page
    app.get("/", [](crest::Request& req, crest::Response& res) {
        std::string html = R"html(
<!DOCTYPE html>
<html>
<head>
    <title>Crest Chat</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 800px; margin: 50px auto; }
        #messages { border: 1px solid #ccc; height: 400px; overflow-y: scroll; padding: 10px; margin-bottom: 10px; }
        #input { width: 80%; padding: 10px; }
        #send { padding: 10px 20px; }
        .message { margin: 5px 0; }
        .system { color: #888; font-style: italic; }
    </style>
</head>
<body>
    <h1>Crest WebSocket Chat</h1>
    <div id="messages"></div>
    <input type="text" id="username" placeholder="Enter username" />
    <button onclick="join()">Join</button>
    <br><br>
    <input type="text" id="input" placeholder="Type a message..." />
    <button id="send" onclick="sendMessage()">Send</button>
    
    <script>
        const ws = new WebSocket('ws://localhost:8000/ws');
        const messages = document.getElementById('messages');
        const input = document.getElementById('input');
        
        ws.onopen = () => {
            console.log('Connected to chat server');
        };
        
        ws.onmessage = (event) => {
            const data = JSON.parse(event.data);
            const div = document.createElement('div');
            div.className = 'message';
            
            if (data.type === 'welcome') {
                div.className += ' system';
                div.textContent = data.message;
            } else if (data.type === 'join' || data.type === 'leave') {
                div.className += ' system';
                div.textContent = data.message;
            } else if (data.type === 'message') {
                div.textContent = data.username + ': ' + data.text;
            }
            
            messages.appendChild(div);
            messages.scrollTop = messages.scrollHeight;
        };
        
        function join() {
            const username = document.getElementById('username').value;
            if (username) {
                ws.send(JSON.stringify({type: 'join', username: username}));
                document.getElementById('username').disabled = true;
            }
        }
        
        function sendMessage() {
            const text = input.value;
            if (text) {
                ws.send(JSON.stringify({type: 'message', text: text}));
                input.value = '';
            }
        }
        
        input.addEventListener('keypress', (e) => {
            if (e.key === 'Enter') sendMessage();
        });
    </script>
</body>
</html>
        )html";
        res.html(200, html);
    });
    
    // Stats endpoint
    app.get("/stats", [&ws](crest::Request& req, crest::Response& res) {
        res.json(200, R"({
            "connections": )" + std::to_string(ws.connection_count()) + R"(,
            "status": "running"
        })");
    });
    
    std::cout << "WebSocket Chat Server running on http://0.0.0.0:8000" << std::endl;
    std::cout << "Open http://localhost:8000 in your browser to join the chat" << std::endl;
    std::cout << "WebSocket endpoint: ws://localhost:8000/ws" << std::endl;
    
    app.run("0.0.0.0", 8000);
    return 0;
}

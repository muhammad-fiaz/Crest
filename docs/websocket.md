# WebSocket Support

Real-time bidirectional communication with WebSocket support.

## Overview

WebSockets enable real-time communication between client and server for:
- Chat applications
- Live notifications
- Real-time dashboards
- Multiplayer games
- Collaborative editing

## Basic Usage

```cpp
#include "crest/crest.hpp"
#include "crest/websocket.hpp"

int main() {
    crest::App app;
    crest::WebSocketServer ws;
    
    // Handle new connections
    ws.on_connect([](auto conn) {
        std::cout << "Client connected: " << conn->get_id() << std::endl;
        conn->send(R"({"type":"welcome","message":"Connected!"})");
    });
    
    // Handle messages
    ws.on_message([&ws](auto conn, const std::string& message) {
        std::cout << "Received: " << message << std::endl;
        // Broadcast to all clients
        ws.broadcast(message);
    });
    
    // Handle disconnections
    ws.on_close([](auto conn, int code, const std::string& reason) {
        std::cout << "Client disconnected: " << conn->get_id() << std::endl;
    });
    
    app.run("0.0.0.0", 8000);
    return 0;
}
```

## Connection Management

### Send Messages

```cpp
// Send text message
conn->send(R"({"type":"notification","text":"Hello!"})");

// Send binary data
std::vector<uint8_t> data = {0x01, 0x02, 0x03};
conn->send_binary(data);
```

### Close Connection

```cpp
conn->close(1000, "Normal closure");
```

### Check Connection Status

```cpp
if (conn->is_open()) {
    conn->send("Still connected!");
}
```

## Broadcasting

```cpp
// Broadcast to all connected clients
ws.broadcast(R"({"type":"announcement","text":"Server message"})");

// Broadcast binary data
std::vector<uint8_t> data = {0xFF, 0xFE};
ws.broadcast_binary(data);
```

## Event Handlers

### On Connect

```cpp
ws.on_connect([](std::shared_ptr<crest::WebSocketConnection> conn) {
    // New client connected
});
```

### On Message

```cpp
ws.on_message([](auto conn, const std::string& message) {
    // Text message received
});
```

### On Binary

```cpp
ws.on_binary([](auto conn, const std::vector<uint8_t>& data) {
    // Binary data received
});
```

### On Close

```cpp
ws.on_close([](auto conn, int code, const std::string& reason) {
    // Connection closed
});
```

### On Error

```cpp
ws.on_error([](auto conn, const std::string& error) {
    // Error occurred
});
```

## Chat Application Example

```cpp
#include "crest/crest.hpp"
#include "crest/websocket.hpp"
#include <map>

int main() {
    crest::App app;
    crest::WebSocketServer ws;
    
    std::map<std::string, std::string> users;
    
    ws.on_connect([&users](auto conn) {
        users[conn->get_id()] = "Anonymous";
    });
    
    ws.on_message([&ws, &users](auto conn, const std::string& message) {
        // Parse message (simplified)
        if (message.find("\"type\":\"join\"") != std::string::npos) {
            // Extract username
            users[conn->get_id()] = "User123";
            ws.broadcast(R"({"type":"join","user":"User123"})");
        } else {
            // Broadcast chat message
            std::string msg = R"({"type":"message","user":")" + 
                            users[conn->get_id()] + 
                            R"(","text":")" + message + R"("})";
            ws.broadcast(msg);
        }
    });
    
    ws.on_close([&ws, &users](auto conn, int code, const std::string& reason) {
        std::string user = users[conn->get_id()];
        users.erase(conn->get_id());
        ws.broadcast(R"({"type":"leave","user":")" + user + R"("})");
    });
    
    app.run("0.0.0.0", 8000);
    return 0;
}
```

## Client-Side JavaScript

```javascript
const ws = new WebSocket('ws://localhost:8000/ws');

ws.onopen = () => {
    console.log('Connected');
    ws.send(JSON.stringify({type: 'join', username: 'Alice'}));
};

ws.onmessage = (event) => {
    const data = JSON.parse(event.data);
    console.log('Received:', data);
};

ws.onclose = () => {
    console.log('Disconnected');
};

ws.onerror = (error) => {
    console.error('Error:', error);
};
```

## Best Practices

- Validate all incoming messages
- Implement heartbeat/ping-pong for connection health
- Handle reconnection on client side
- Use JSON for structured messages
- Implement authentication before accepting connections
- Set message size limits
- Clean up resources on disconnect

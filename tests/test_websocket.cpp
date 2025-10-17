/**
 * @file test_websocket.cpp
 * @brief Test cases for WebSocket support
 */

#include "crest/websocket.hpp"
#include <cassert>
#include <iostream>

void test_websocket_server_creation() {
    std::cout << "Testing WebSocket server creation..." << std::endl;
    
    crest::WebSocketServer ws;
    
    assert(ws.connection_count() == 0);
    
    std::cout << "  ✓ WebSocket server created" << std::endl;
}

void test_websocket_callbacks() {
    std::cout << "Testing WebSocket callbacks..." << std::endl;
    
    crest::WebSocketServer ws;
    
    bool connect_called = false;
    bool message_called = false;
    bool close_called = false;
    
    ws.on_connect([&connect_called](auto conn) {
        connect_called = true;
    });
    
    ws.on_message([&message_called](auto conn, const std::string& msg) {
        message_called = true;
    });
    
    ws.on_close([&close_called](auto conn, int code, const std::string& reason) {
        close_called = true;
    });
    
    std::cout << "  ✓ WebSocket callbacks registered" << std::endl;
}

void test_websocket_broadcast() {
    std::cout << "Testing WebSocket broadcast..." << std::endl;
    
    crest::WebSocketServer ws;
    
    ws.broadcast("{\"type\":\"test\"}");
    
    std::cout << "  ✓ Broadcast message sent" << std::endl;
}

int main() {
    std::cout << "\n=== WebSocket Tests ===" << std::endl;
    
    test_websocket_server_creation();
    test_websocket_callbacks();
    test_websocket_broadcast();
    
    std::cout << "\n✅ All WebSocket tests passed!" << std::endl;
    return 0;
}

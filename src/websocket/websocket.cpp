/**
 * @file websocket.cpp
 * @brief WebSocket implementation
 */

#include "crest/websocket.hpp"
#include <algorithm>

namespace crest {

WebSocketServer::WebSocketServer() {}

WebSocketServer::~WebSocketServer() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& pair : connections_) {
        if (pair.second && pair.second->is_open()) {
            pair.second->close();
        }
    }
    connections_.clear();
}

void WebSocketServer::broadcast(const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& pair : connections_) {
        if (pair.second && pair.second->is_open()) {
            pair.second->send(message);
        }
    }
}

void WebSocketServer::broadcast_binary(const std::vector<uint8_t>& data) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& pair : connections_) {
        if (pair.second && pair.second->is_open()) {
            pair.second->send_binary(data);
        }
    }
}

size_t WebSocketServer::connection_count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return connections_.size();
}

std::vector<std::shared_ptr<WebSocketConnection>> WebSocketServer::get_connections() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::shared_ptr<WebSocketConnection>> result;
    result.reserve(connections_.size());
    for (const auto& pair : connections_) {
        result.push_back(pair.second);
    }
    return result;
}

} // namespace crest

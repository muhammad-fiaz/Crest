/**
 * @file websocket.hpp
 * @brief WebSocket support for Crest framework
 * @version 0.0.0
 */

#ifndef CREST_WEBSOCKET_HPP
#define CREST_WEBSOCKET_HPP

#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <map>
#include <mutex>

namespace crest {

class WebSocketConnection {
public:
    virtual ~WebSocketConnection() = default;
    
    virtual void send(const std::string& message) = 0;
    virtual void send_binary(const std::vector<uint8_t>& data) = 0;
    virtual void close(int code = 1000, const std::string& reason = "") = 0;
    virtual bool is_open() const = 0;
    virtual std::string get_id() const = 0;
};

using OnConnectCallback = std::function<void(std::shared_ptr<WebSocketConnection>)>;
using OnMessageCallback = std::function<void(std::shared_ptr<WebSocketConnection>, const std::string&)>;
using OnBinaryCallback = std::function<void(std::shared_ptr<WebSocketConnection>, const std::vector<uint8_t>&)>;
using OnCloseCallback = std::function<void(std::shared_ptr<WebSocketConnection>, int code, const std::string& reason)>;
using OnErrorCallback = std::function<void(std::shared_ptr<WebSocketConnection>, const std::string& error)>;

class WebSocketServer {
public:
    WebSocketServer();
    ~WebSocketServer();
    
    void on_connect(OnConnectCallback callback) { on_connect_ = callback; }
    void on_message(OnMessageCallback callback) { on_message_ = callback; }
    void on_binary(OnBinaryCallback callback) { on_binary_ = callback; }
    void on_close(OnCloseCallback callback) { on_close_ = callback; }
    void on_error(OnErrorCallback callback) { on_error_ = callback; }
    
    void broadcast(const std::string& message);
    void broadcast_binary(const std::vector<uint8_t>& data);
    
    size_t connection_count() const;
    std::vector<std::shared_ptr<WebSocketConnection>> get_connections() const;

private:
    OnConnectCallback on_connect_;
    OnMessageCallback on_message_;
    OnBinaryCallback on_binary_;
    OnCloseCallback on_close_;
    OnErrorCallback on_error_;
    
    std::map<std::string, std::shared_ptr<WebSocketConnection>> connections_;
    mutable std::mutex mutex_;
};

} // namespace crest

#endif /* CREST_WEBSOCKET_HPP */

/**
 * @file crest.hpp
 * @brief Crest RESTful API Framework - C++ API Header
 * @version 0.0.0
 * @author Muhammad Fiaz <contact@muhammadfiaz.com>
 * 
 * Production-ready RESTful API framework for C++ projects
 * Inspired by modern API frameworks for simplicity and performance
 */

#ifndef CREST_HPP
#define CREST_HPP

#include "crest.h"
#include <string>
#include <functional>
#include <memory>
#include <map>
#include <vector>

namespace crest {

constexpr const char* VERSION = CREST_VERSION;

enum class Method {
    GET = CREST_GET,
    POST = CREST_POST,
    PUT = CREST_PUT,
    DELETE = CREST_DELETE,
    PATCH = CREST_PATCH,
    HEAD = CREST_HEAD,
    OPTIONS = CREST_OPTIONS
};

enum class Status {
    OK = 200,
    CREATED = 201,
    ACCEPTED = 202,
    NO_CONTENT = 204,
    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    INTERNAL_ERROR = 500,
    NOT_IMPLEMENTED = 501,
    SERVICE_UNAVAILABLE = 503
};

class Request {
public:
    Request(crest_request_t* req) : req_(req) {}
    
    std::string path() const;
    std::string method() const;
    std::string body() const;
    std::string query(const std::string& key) const;
    std::string header(const std::string& key) const;
    std::map<std::string, std::string> queries() const;
    std::map<std::string, std::string> headers() const;
    
    crest_request_t* raw() { return req_; }
    
private:
    crest_request_t* req_;
};

class Response {
public:
    Response(crest_response_t* res) : res_(res) {}
    
    void json(Status status, const std::string& json);
    void json(int status, const std::string& json);
    void text(Status status, const std::string& text);
    void text(int status, const std::string& text);
    void html(Status status, const std::string& html);
    void html(int status, const std::string& html);
    void set_header(const std::string& key, const std::string& value);
    
    crest_response_t* raw() { return res_; }
    
private:
    crest_response_t* res_;
};

using Handler = std::function<void(Request&, Response&)>;

struct Config {
    std::string title = "Crest API";
    std::string description = "RESTful API built with Crest";
    std::string version = "0.0.0";
    bool docs_enabled = true;
    std::string docs_path = "/docs";
    std::string openapi_path = "/openapi.json";
    std::string proxy_url;
    int max_connections = 1000;
    int timeout_seconds = 30;
};

class App {
public:
    /**
     * @brief Create a new Crest application
     */
    App();
    
    /**
     * @brief Create a new Crest application with configuration
     * @param config Configuration object
     */
    explicit App(const Config& config);
    
    ~App();
    
    App(const App&) = delete;
    App& operator=(const App&) = delete;
    App(App&&) noexcept;
    App& operator=(App&&) noexcept;
    
    /**
     * @brief Register a GET route
     * @param path Route path
     * @param handler Handler function
     * @param description Route description
     * @return Reference to this app for chaining
     */
    App& get(const std::string& path, Handler handler, const std::string& description = "");
    
    /**
     * @brief Register a POST route
     * @param path Route path
     * @param handler Handler function
     * @param description Route description
     * @return Reference to this app for chaining
     */
    App& post(const std::string& path, Handler handler, const std::string& description = "");
    
    /**
     * @brief Register a PUT route
     * @param path Route path
     * @param handler Handler function
     * @param description Route description
     * @return Reference to this app for chaining
     */
    App& put(const std::string& path, Handler handler, const std::string& description = "");
    
    /**
     * @brief Register a DELETE route
     * @param path Route path
     * @param handler Handler function
     * @param description Route description
     * @return Reference to this app for chaining
     */
    App& del(const std::string& path, Handler handler, const std::string& description = "");
    
    /**
     * @brief Register a PATCH route
     * @param path Route path
     * @param handler Handler function
     * @param description Route description
     * @return Reference to this app for chaining
     */
    App& patch(const std::string& path, Handler handler, const std::string& description = "");
    
    /**
     * @brief Register a route with specific method
     * @param method HTTP method
     * @param path Route path
     * @param handler Handler function
     * @param description Route description
     * @return Reference to this app for chaining
     */
    App& route(Method method, const std::string& path, Handler handler, const std::string& description = "");
    
    /**
     * @brief Set request schema for a route
     * @param method HTTP method
     * @param path Route path
     * @param schema JSON schema (e.g., "{\"name\": \"string\", \"age\": \"number\"}")
     * @return Reference to this app for chaining
     */
    App& set_request_schema(Method method, const std::string& path, const std::string& schema);
    
    /**
     * @brief Set response schema for a route
     * @param method HTTP method
     * @param path Route path
     * @param schema JSON schema (e.g., "{\"id\": \"number\", \"status\": \"string\"}")
     * @return Reference to this app for chaining
     */
    App& set_response_schema(Method method, const std::string& path, const std::string& schema);
    
    /**
     * @brief Start the server
     * @param host Host address
     * @param port Port number
     */
    void run(const std::string& host = "0.0.0.0", int port = 8000);
    
    /**
     * @brief Stop the server
     */
    void stop();
    
    /**
     * @brief Set application title
     * @param title Application title
     */
    void set_title(const std::string& title);
    
    /**
     * @brief Set application description
     * @param description Application description
     */
    void set_description(const std::string& description);
    
    /**
     * @brief Enable/disable documentation
     * @param enabled true to enable, false to disable
     */
    void set_docs_enabled(bool enabled);
    
    /**
     * @brief Set proxy configuration
     * @param proxy_url Proxy URL
     */
    void set_proxy(const std::string& proxy_url);
    
    /**
     * @brief Enable or disable console logging
     * @param enabled true to enable, false to disable
     */
    static void set_logging_enabled(bool enabled) {
        crest_log_set_enabled(enabled);
    }
    
    /**
     * @brief Enable or disable timestamps in logs
     * @param enabled true to enable, false to disable
     */
    static void set_timestamp_enabled(bool enabled) {
        crest_log_set_timestamp(enabled);
    }
    
    crest_app_t* raw() { return app_; }
    
private:
    crest_app_t* app_;
    std::map<void*, Handler> handlers_;
};

class Exception : public std::exception {
public:
    explicit Exception(const std::string& message, int code = 500)
        : message_(message), code_(code) {}
    
    const char* what() const noexcept override { return message_.c_str(); }
    int code() const noexcept { return code_; }
    
private:
    std::string message_;
    int code_;
};

} // namespace crest

#endif /* CREST_HPP */

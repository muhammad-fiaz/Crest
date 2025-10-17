/**
 * @file app.cpp
 * @brief C++ application implementation
 */

#include "crest/crest.hpp"
#include "crest/internal/app_internal.h"
#include <cstring>

namespace crest {

std::string Request::path() const {
    const char* p = crest_request_get_path(req_);
    return p ? std::string(p) : "";
}

std::string Request::method() const {
    const char* m = crest_request_get_method(req_);
    return m ? std::string(m) : "";
}

std::string Request::body() const {
    const char* b = crest_request_get_body(req_);
    return b ? std::string(b) : "";
}

std::string Request::query(const std::string& key) const {
    const char* v = crest_request_get_query(req_, key.c_str());
    return v ? std::string(v) : "";
}

std::string Request::header(const std::string& key) const {
    const char* v = crest_request_get_header(req_, key.c_str());
    return v ? std::string(v) : "";
}

std::map<std::string, std::string> Request::queries() const {
    return {};
}

std::map<std::string, std::string> Request::headers() const {
    return {};
}

void Response::json(Status status, const std::string& json) {
    crest_response_json(res_, static_cast<int>(status), json.c_str());
}

void Response::json(int status, const std::string& json) {
    crest_response_json(res_, status, json.c_str());
}

void Response::text(Status status, const std::string& text) {
    crest_response_text(res_, static_cast<int>(status), text.c_str());
}

void Response::text(int status, const std::string& text) {
    crest_response_text(res_, status, text.c_str());
}

void Response::html(Status status, const std::string& html) {
    crest_response_html(res_, static_cast<int>(status), html.c_str());
}

void Response::html(int status, const std::string& html) {
    crest_response_html(res_, status, html.c_str());
}

void Response::set_header(const std::string& key, const std::string& value) {
    crest_response_set_header(res_, key.c_str(), value.c_str());
}

App::App() : app_(crest_create()) {}

App::App(const Config& config) {
    crest_config_t c_config;
    c_config.title = config.title.c_str();
    c_config.description = config.description.c_str();
    c_config.version = config.version.c_str();
    c_config.docs_enabled = config.docs_enabled;
    app_ = crest_create_with_config(&c_config);
}

App::~App() {
    if (app_) {
        crest_destroy(app_);
    }
}

App::App(App&& other) noexcept : app_(other.app_), handlers_(std::move(other.handlers_)) {
    other.app_ = nullptr;
}

App& App::operator=(App&& other) noexcept {
    if (this != &other) {
        if (app_) crest_destroy(app_);
        app_ = other.app_;
        handlers_ = std::move(other.handlers_);
        other.app_ = nullptr;
    }
    return *this;
}

App& App::route(Method method, const std::string& path, Handler handler, const std::string& description) {
    if (!app_) throw Exception("Invalid app instance");
    
    Handler* handler_ptr = new Handler(std::move(handler));
    
    auto c_handler = [](crest_request_t* req, crest_response_t* res) {
        // C handler wrapper - actual handler called via cpp_handler pointer
    };
    
    int result = crest_route(app_, static_cast<crest_method_t>(method), 
                            path.c_str(), c_handler, description.c_str());
    
    if (result != 0) {
        delete handler_ptr;
        throw Exception("Failed to register route: " + path);
    }
    
    handlers_[handler_ptr] = *handler_ptr;
    
    if (app_->route_count > 0) {
        app_->routes[app_->route_count - 1].cpp_handler = handler_ptr;
    }
    
    return *this;
}

App& App::get(const std::string& path, Handler handler, const std::string& description) {
    return route(Method::GET, path, std::move(handler), description);
}

App& App::post(const std::string& path, Handler handler, const std::string& description) {
    return route(Method::POST, path, std::move(handler), description);
}

App& App::put(const std::string& path, Handler handler, const std::string& description) {
    return route(Method::PUT, path, std::move(handler), description);
}

App& App::del(const std::string& path, Handler handler, const std::string& description) {
    return route(Method::DELETE, path, std::move(handler), description);
}

App& App::patch(const std::string& path, Handler handler, const std::string& description) {
    return route(Method::PATCH, path, std::move(handler), description);
}

void App::run(const std::string& host, int port) {
    if (!app_) throw Exception("Invalid app instance");
    
    int result = crest_run(app_, host.c_str(), port);
    if (result != 0) {
        throw Exception("Failed to start server");
    }
}

void App::stop() {
    if (app_) crest_stop(app_);
}

void App::set_title(const std::string& title) {
    if (app_) crest_set_title(app_, title.c_str());
}

void App::set_description(const std::string& description) {
    if (app_) crest_set_description(app_, description.c_str());
}

void App::set_docs_enabled(bool enabled) {
    if (app_) crest_set_docs_enabled(app_, enabled);
}

void App::set_proxy(const std::string& proxy_url) {
    if (app_) crest_set_proxy(app_, proxy_url.c_str());
}

App& App::set_request_schema(Method method, const std::string& path, const std::string& schema) {
    if (app_) crest_set_request_schema(app_, static_cast<crest_method_t>(method), path.c_str(), schema.c_str());
    return *this;
}

App& App::set_response_schema(Method method, const std::string& path, const std::string& schema) {
    if (app_) crest_set_response_schema(app_, static_cast<crest_method_t>(method), path.c_str(), schema.c_str());
    return *this;
}

} // namespace crest

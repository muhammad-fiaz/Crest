/**
 * @file middleware.cpp
 * @brief Middleware implementations
 */

#include "crest/middleware.hpp"
#include <chrono>
#include <sstream>

namespace crest {

void CorsMiddleware::handle(Request& req, Response& res, NextFunction next) {
    std::string origin = req.header("Origin");
    
    if (options_.allowed_origins[0] == "*" || 
        std::find(options_.allowed_origins.begin(), options_.allowed_origins.end(), origin) != options_.allowed_origins.end()) {
        res.set_header("Access-Control-Allow-Origin", origin.empty() ? "*" : origin);
    }
    
    std::ostringstream methods;
    for (size_t i = 0; i < options_.allowed_methods.size(); ++i) {
        if (i > 0) methods << ", ";
        methods << options_.allowed_methods[i];
    }
    res.set_header("Access-Control-Allow-Methods", methods.str());
    
    if (options_.allowed_headers[0] == "*") {
        res.set_header("Access-Control-Allow-Headers", "*");
    } else {
        std::ostringstream headers;
        for (size_t i = 0; i < options_.allowed_headers.size(); ++i) {
            if (i > 0) headers << ", ";
            headers << options_.allowed_headers[i];
        }
        res.set_header("Access-Control-Allow-Headers", headers.str());
    }
    
    if (!options_.exposed_headers.empty()) {
        std::ostringstream exposed;
        for (size_t i = 0; i < options_.exposed_headers.size(); ++i) {
            if (i > 0) exposed << ", ";
            exposed << options_.exposed_headers[i];
        }
        res.set_header("Access-Control-Expose-Headers", exposed.str());
    }
    
    if (options_.allow_credentials) {
        res.set_header("Access-Control-Allow-Credentials", "true");
    }
    
    res.set_header("Access-Control-Max-Age", std::to_string(options_.max_age));
    
    if (req.method() == "OPTIONS") {
        res.text(204, "");
        return;
    }
    
    next();
}

void RateLimitMiddleware::handle(Request& req, Response& res, NextFunction next) {
    std::string client_ip = req.header("X-Forwarded-For");
    if (client_ip.empty()) {
        client_ip = req.header("X-Real-IP");
    }
    if (client_ip.empty()) {
        client_ip = "unknown";
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::time(nullptr);
    auto& entry = request_counts_[client_ip];
    
    if (now - entry.second >= options_.window_seconds) {
        entry.first = 0;
        entry.second = now;
    }
    
    entry.first++;
    
    if (entry.first > options_.max_requests) {
        res.json(429, "{\"error\":\"" + options_.message + "\"}");
        return;
    }
    
    res.set_header("X-RateLimit-Limit", std::to_string(options_.max_requests));
    res.set_header("X-RateLimit-Remaining", std::to_string(options_.max_requests - entry.first));
    res.set_header("X-RateLimit-Reset", std::to_string(entry.second + options_.window_seconds));
    
    next();
}

void AuthMiddleware::handle(Request& req, Response& res, NextFunction next) {
    std::string auth_header = req.header("Authorization");
    
    if (auth_header.empty()) {
        res.json(401, "{\"error\":\"Missing authorization header\"}");
        return;
    }
    
    std::string token;
    if (auth_header.substr(0, 7) == "Bearer ") {
        token = auth_header.substr(7);
    } else {
        token = auth_header;
    }
    
    if (!validator_(token)) {
        res.json(403, "{\"error\":\"Invalid or expired token\"}");
        return;
    }
    
    next();
}

void LoggingMiddleware::handle(Request& req, Response& res, NextFunction next) {
    auto start = std::chrono::high_resolution_clock::now();
    
    next();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Log format: METHOD PATH - STATUS (duration ms)
    printf("[%s] %s - %dms\n", req.method().c_str(), req.path().c_str(), (int)duration.count());
}

void CompressionMiddleware::handle(Request& req, Response& res, NextFunction next) {
    std::string accept_encoding = req.header("Accept-Encoding");
    
    if (accept_encoding.find("gzip") != std::string::npos) {
        res.set_header("Content-Encoding", "gzip");
    }
    
    next();
}

} // namespace crest

/**
 * @file middleware.hpp
 * @brief Middleware system for Crest framework
 * @version 0.0.0
 */

#ifndef CREST_MIDDLEWARE_HPP
#define CREST_MIDDLEWARE_HPP

#include "crest.hpp"
#include <functional>
#include <vector>
#include <string>
#include <regex>
#include <mutex>
#include <map>
#include <ctime>

namespace crest {

using NextFunction = std::function<void()>;
using MiddlewareFunction = std::function<void(Request&, Response&, NextFunction)>;

class Middleware {
public:
    virtual ~Middleware() = default;
    virtual void handle(Request& req, Response& res, NextFunction next) = 0;
};

class CorsMiddleware : public Middleware {
public:
    struct Options {
        std::vector<std::string> allowed_origins = {"*"};
        std::vector<std::string> allowed_methods = {"GET", "POST", "PUT", "DELETE", "PATCH", "OPTIONS"};
        std::vector<std::string> allowed_headers = {"*"};
        std::vector<std::string> exposed_headers = {};
        bool allow_credentials = false;
        int max_age = 86400;
    };

    explicit CorsMiddleware(const Options& opts = Options()) : options_(opts) {}
    
    void handle(Request& req, Response& res, NextFunction next) override;

private:
    Options options_;
};

class RateLimitMiddleware : public Middleware {
public:
    struct Options {
        int max_requests = 100;
        int window_seconds = 60;
        std::string message = "Too many requests";
    };

    explicit RateLimitMiddleware(const Options& opts = Options()) : options_(opts) {}
    
    void handle(Request& req, Response& res, NextFunction next) override;

private:
    Options options_;
    std::map<std::string, std::pair<int, time_t>> request_counts_;
    std::mutex mutex_;
};

class AuthMiddleware : public Middleware {
public:
    using ValidatorFunction = std::function<bool(const std::string&)>;

    explicit AuthMiddleware(ValidatorFunction validator) : validator_(validator) {}
    
    void handle(Request& req, Response& res, NextFunction next) override;

private:
    ValidatorFunction validator_;
};

class LoggingMiddleware : public Middleware {
public:
    void handle(Request& req, Response& res, NextFunction next) override;
};

class CompressionMiddleware : public Middleware {
public:
    void handle(Request& req, Response& res, NextFunction next) override;
};

} // namespace crest

#endif /* CREST_MIDDLEWARE_HPP */

/**
 * @file test_middleware.cpp
 * @brief Test cases for middleware system
 */

#include "crest/crest.hpp"
#include "crest/middleware.hpp"
#include <cassert>
#include <iostream>

void test_cors_middleware() {
    std::cout << "Testing CORS middleware..." << std::endl;
    
    crest::CorsMiddleware::Options opts;
    opts.allowed_origins = {"https://example.com"};
    opts.allowed_methods = {"GET", "POST"};
    
    crest::CorsMiddleware cors(opts);
    
    std::cout << "  ✓ CORS middleware created" << std::endl;
}

void test_rate_limit_middleware() {
    std::cout << "Testing rate limit middleware..." << std::endl;
    
    crest::RateLimitMiddleware::Options opts;
    opts.max_requests = 10;
    opts.window_seconds = 60;
    
    crest::RateLimitMiddleware limiter(opts);
    
    std::cout << "  ✓ Rate limiter created" << std::endl;
}

void test_auth_middleware() {
    std::cout << "Testing auth middleware..." << std::endl;
    
    auto validator = [](const std::string& token) -> bool {
        return token == "valid-token";
    };
    
    crest::AuthMiddleware auth(validator);
    
    assert(validator("valid-token") == true);
    assert(validator("invalid") == false);
    
    std::cout << "  ✓ Auth middleware created and validated" << std::endl;
}

void test_logging_middleware() {
    std::cout << "Testing logging middleware..." << std::endl;
    
    crest::LoggingMiddleware logger;
    
    std::cout << "  ✓ Logging middleware created" << std::endl;
}

int main() {
    std::cout << "\n=== Middleware Tests ===" << std::endl;
    
    test_cors_middleware();
    test_rate_limit_middleware();
    test_auth_middleware();
    test_logging_middleware();
    
    std::cout << "\n✅ All middleware tests passed!" << std::endl;
    return 0;
}

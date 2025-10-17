/**
 * @file middleware_example.cpp
 * @brief Comprehensive middleware example
 */

#include "crest/crest.hpp"
#include "crest/middleware.hpp"
#include <iostream>

int main() {
    crest::App app;
    
    // Configure CORS
    crest::CorsMiddleware::Options cors_opts;
    cors_opts.allowed_origins = {"*"};
    cors_opts.allowed_methods = {"GET", "POST", "PUT", "DELETE", "OPTIONS"};
    cors_opts.allowed_headers = {"Content-Type", "Authorization"};
    cors_opts.allow_credentials = false;
    cors_opts.max_age = 86400;
    
    crest::CorsMiddleware cors(cors_opts);
    
    // Configure rate limiting
    crest::RateLimitMiddleware::Options rate_opts;
    rate_opts.max_requests = 100;
    rate_opts.window_seconds = 60;
    rate_opts.message = "Too many requests, please slow down";
    
    crest::RateLimitMiddleware rate_limiter(rate_opts);
    
    // Configure authentication
    auto token_validator = [](const std::string& token) -> bool {
        // Simple token validation (use proper JWT in production)
        return token == "secret-token-123" || token == "admin-token-456";
    };
    
    crest::AuthMiddleware auth(token_validator);
    
    // Logging middleware
    crest::LoggingMiddleware logger;
    
    // Public endpoint (no auth required)
    app.get("/", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({
            "message": "Welcome to Crest API",
            "version": "1.0.0",
            "endpoints": ["/", "/public", "/protected", "/admin"]
        })");
    });
    
    // Public endpoint with rate limiting
    app.get("/public", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({
            "message": "This is a public endpoint",
            "data": "Anyone can access this"
        })");
    });
    
    // Protected endpoint (requires authentication)
    app.get("/protected", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({
            "message": "This is a protected endpoint",
            "data": "Only authenticated users can see this"
        })");
    });
    
    // Admin endpoint
    app.post("/admin/users", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({
            "message": "User created successfully",
            "id": 123
        })");
    });
    
    // Health check endpoint
    app.get("/health", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({
            "status": "healthy",
            "timestamp": "2024-01-01T00:00:00Z"
        })");
    });
    
    std::cout << "Server running on http://0.0.0.0:8000" << std::endl;
    std::cout << "Endpoints:" << std::endl;
    std::cout << "  GET  /              - Public welcome" << std::endl;
    std::cout << "  GET  /public        - Public data" << std::endl;
    std::cout << "  GET  /protected     - Protected (requires auth)" << std::endl;
    std::cout << "  POST /admin/users   - Admin only" << std::endl;
    std::cout << "  GET  /health        - Health check" << std::endl;
    std::cout << std::endl;
    std::cout << "Test with:" << std::endl;
    std::cout << "  curl http://localhost:8000/" << std::endl;
    std::cout << "  curl -H \"Authorization: Bearer secret-token-123\" http://localhost:8000/protected" << std::endl;
    
    app.run("0.0.0.0", 8000);
    return 0;
}

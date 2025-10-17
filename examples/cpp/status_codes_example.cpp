/**
 * @file status_codes_example.cpp
 * @brief Demonstrates all HTTP status codes in Crest
 */

#include "crest/crest.hpp"
#include <iostream>

int main() {
    crest::App app;
    
    // 2xx Success responses
    app.get("/success/ok", [](crest::Request& req, crest::Response& res) {
        res.json(crest::Status::OK, R"({"status":"ok"})");
    });
    
    app.post("/success/created", [](crest::Request& req, crest::Response& res) {
        res.json(crest::Status::CREATED, R"({"id":123,"created":true})");
    });
    
    app.get("/success/accepted", [](crest::Request& req, crest::Response& res) {
        res.json(crest::Status::ACCEPTED, R"({"queued":true})");
    });
    
    app.del("/success/no-content", [](crest::Request& req, crest::Response& res) {
        res.json(crest::Status::NO_CONTENT, "");
    });
    
    // 4xx Client errors
    app.get("/error/bad-request", [](crest::Request& req, crest::Response& res) {
        res.json(crest::Status::BAD_REQUEST, R"({"error":"Invalid input"})");
    });
    
    app.get("/error/unauthorized", [](crest::Request& req, crest::Response& res) {
        res.json(crest::Status::UNAUTHORIZED, R"({"error":"Authentication required"})");
    });
    
    app.get("/error/forbidden", [](crest::Request& req, crest::Response& res) {
        res.json(crest::Status::FORBIDDEN, R"({"error":"Access denied"})");
    });
    
    app.get("/error/not-found", [](crest::Request& req, crest::Response& res) {
        res.json(crest::Status::NOT_FOUND, R"({"error":"Resource not found"})");
    });
    
    app.get("/error/method-not-allowed", [](crest::Request& req, crest::Response& res) {
        res.json(crest::Status::METHOD_NOT_ALLOWED, R"({"error":"Method not allowed","allowed":["GET","POST"]})");
    });
    
    // Custom 4xx codes
    app.get("/error/conflict", [](crest::Request& req, crest::Response& res) {
        res.json(409, R"({"error":"Resource already exists"})");
    });
    
    app.get("/error/gone", [](crest::Request& req, crest::Response& res) {
        res.json(410, R"({"error":"Resource permanently deleted"})");
    });
    
    app.get("/error/unprocessable", [](crest::Request& req, crest::Response& res) {
        res.json(422, R"({"errors":["Name required","Email invalid"]})");
    });
    
    app.get("/error/too-many-requests", [](crest::Request& req, crest::Response& res) {
        res.set_header("Retry-After", "60");
        res.json(429, R"({"error":"Rate limit exceeded","retry_after":60})");
    });
    
    // 5xx Server errors
    app.get("/error/internal", [](crest::Request& req, crest::Response& res) {
        res.json(crest::Status::INTERNAL_ERROR, R"({"error":"Internal server error"})");
    });
    
    app.get("/error/not-implemented", [](crest::Request& req, crest::Response& res) {
        res.json(crest::Status::NOT_IMPLEMENTED, R"({"error":"Feature not implemented"})");
    });
    
    app.get("/error/bad-gateway", [](crest::Request& req, crest::Response& res) {
        res.json(502, R"({"error":"Bad gateway"})");
    });
    
    app.get("/error/service-unavailable", [](crest::Request& req, crest::Response& res) {
        res.set_header("Retry-After", "3600");
        res.json(crest::Status::SERVICE_UNAVAILABLE, R"({"error":"Service under maintenance","retry_after":3600})");
    });
    
    app.get("/error/gateway-timeout", [](crest::Request& req, crest::Response& res) {
        res.json(504, R"({"error":"Gateway timeout"})");
    });
    
    // Practical example: Authentication
    app.post("/auth/login", [](crest::Request& req, crest::Response& res) {
        std::string body = req.body();
        
        if (body.empty()) {
            res.json(400, R"({"error":"Request body required"})");
            return;
        }
        
        // Simulate authentication
        if (body.find("invalid") != std::string::npos) {
            res.json(401, R"({"error":"Invalid credentials"})");
            return;
        }
        
        res.json(200, R"({"token":"abc123","expires":3600})");
    });
    
    // Practical example: Resource management
    app.post("/resources", [](crest::Request& req, crest::Response& res) {
        std::string body = req.body();
        
        if (body.empty()) {
            res.json(400, R"({"error":"Body required"})");
            return;
        }
        
        // Simulate validation
        if (body.find("invalid") != std::string::npos) {
            res.json(422, R"({"errors":["Invalid data format"]})");
            return;
        }
        
        // Simulate conflict
        if (body.find("exists") != std::string::npos) {
            res.json(409, R"({"error":"Resource already exists"})");
            return;
        }
        
        res.json(201, R"({"id":456,"created":true})");
    });
    
    // Health check
    app.get("/health", [](crest::Request& req, crest::Response& res) {
        // Simulate health check
        bool healthy = true;
        
        if (healthy) {
            res.json(200, R"({"status":"healthy","uptime":12345})");
        } else {
            res.json(503, R"({"status":"unhealthy","reason":"Database connection failed"})");
        }
    });
    
    // Root endpoint with documentation
    app.get("/", [](crest::Request& req, crest::Response& res) {
        res.html(200, 
            "<!DOCTYPE html><html><head><title>Status Codes Example</title></head>"
            "<body><h1>HTTP Status Codes Example</h1>"
            "<h2>2xx Success</h2><ul>"
            "<li><a href='/success/ok'>200 OK</a></li>"
            "<li><a href='/success/created'>201 Created</a> (POST)</li>"
            "<li><a href='/success/accepted'>202 Accepted</a></li>"
            "<li><a href='/success/no-content'>204 No Content</a> (DELETE)</li>"
            "</ul>"
            "<h2>4xx Client Errors</h2><ul>"
            "<li><a href='/error/bad-request'>400 Bad Request</a></li>"
            "<li><a href='/error/unauthorized'>401 Unauthorized</a></li>"
            "<li><a href='/error/forbidden'>403 Forbidden</a></li>"
            "<li><a href='/error/not-found'>404 Not Found</a></li>"
            "<li><a href='/error/method-not-allowed'>405 Method Not Allowed</a></li>"
            "<li><a href='/error/conflict'>409 Conflict</a></li>"
            "<li><a href='/error/gone'>410 Gone</a></li>"
            "<li><a href='/error/unprocessable'>422 Unprocessable Entity</a></li>"
            "<li><a href='/error/too-many-requests'>429 Too Many Requests</a></li>"
            "</ul>"
            "<h2>5xx Server Errors</h2><ul>"
            "<li><a href='/error/internal'>500 Internal Server Error</a></li>"
            "<li><a href='/error/not-implemented'>501 Not Implemented</a></li>"
            "<li><a href='/error/bad-gateway'>502 Bad Gateway</a></li>"
            "<li><a href='/error/service-unavailable'>503 Service Unavailable</a></li>"
            "<li><a href='/error/gateway-timeout'>504 Gateway Timeout</a></li>"
            "</ul>"
            "<h2>Practical Examples</h2><ul>"
            "<li>POST /auth/login - Authentication</li>"
            "<li>POST /resources - Resource creation</li>"
            "<li><a href='/health'>GET /health</a> - Health check</li>"
            "</ul>"
            "<p>Visit <a href='/docs'>/docs</a> for full API documentation</p>"
            "</body></html>");
    });
    
    std::cout << "HTTP Status Codes Example Server\n";
    std::cout << "=================================\n";
    std::cout << "Demonstrating all HTTP status codes\n";
    std::cout << "Visit http://localhost:8000/ for examples\n\n";
    
    app.run("0.0.0.0", 8000);
    return 0;
}

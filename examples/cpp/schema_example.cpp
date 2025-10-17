/**
 * @file schema_example.cpp
 * @brief Demonstrates custom schema definitions for API documentation
 */

#include "crest/crest.hpp"
#include <iostream>

int main() {
    crest::App app;
    
    // Example 1: Simple GET with custom response schema
    app.get("/user", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"id": 123, "name": "John Doe", "email": "john@example.com"})");
    }, "Get user information");
    
    // Set custom schema - types will be displayed as: number, string, string
    app.set_response_schema(crest::Method::GET, "/user", 
        R"({"id": "number", "name": "string", "email": "string"})");
    
    // Example 2: POST with request and response schemas
    app.post("/user", [](crest::Request& req, crest::Response& res) {
        res.json(201, R"({"id": 456, "status": "created"})");
    }, "Create new user");
    
    app.set_request_schema(crest::Method::POST, "/user",
        R"({"name": "string", "email": "string", "age": "number"})");
    app.set_response_schema(crest::Method::POST, "/user",
        R"({"id": "number", "status": "string"})");
    
    // Example 3: Complex nested schema
    app.get("/profile", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({
            "user": {"id": 1, "name": "Alice"},
            "settings": {"theme": "dark", "notifications": true},
            "stats": {"posts": 42, "followers": 1337}
        })");
    }, "Get user profile");
    
    app.set_response_schema(crest::Method::GET, "/profile",
        R"({"user": "object", "settings": "object", "stats": "object"})");
    
    // Example 4: Array response
    app.get("/users", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"([
            {"id": 1, "name": "Alice"},
            {"id": 2, "name": "Bob"}
        ])");
    }, "List all users");
    
    app.set_response_schema(crest::Method::GET, "/users",
        R"([{"id": "number", "name": "string"}])");
    
    // Example 5: Boolean and null types
    app.get("/status", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({
            "online": true,
            "maintenance": false,
            "message": null,
            "uptime": 3600
        })");
    }, "Get service status");
    
    app.set_response_schema(crest::Method::GET, "/status",
        R"({"online": "boolean", "maintenance": "boolean", "message": "null", "uptime": "number"})");
    
    // Example 6: PUT with full schemas
    app.put("/settings", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"updated": true, "timestamp": 1234567890})");
    }, "Update settings");
    
    app.set_request_schema(crest::Method::PUT, "/settings",
        R"({"theme": "string", "language": "string", "notifications": "boolean"})");
    app.set_response_schema(crest::Method::PUT, "/settings",
        R"({"updated": "boolean", "timestamp": "number"})");
    
    // Example 7: DELETE with minimal response
    app.del("/user", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"deleted": true})");
    }, "Delete user");
    
    app.set_response_schema(crest::Method::DELETE, "/user",
        R"({"deleted": "boolean"})");
    
    // Example 8: PATCH with partial update
    app.patch("/user", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"updated_fields": ["name", "email"]})");
    }, "Partial user update");
    
    app.set_request_schema(crest::Method::PATCH, "/user",
        R"({"name": "string", "email": "string"})");
    app.set_response_schema(crest::Method::PATCH, "/user",
        R"({"updated_fields": "array"})");
    
    // Example 9: No request body (GET)
    app.get("/health", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"status": "healthy", "version": "1.0.0"})");
    }, "Health check");
    
    // GET methods automatically show "None" for request schema
    app.set_response_schema(crest::Method::GET, "/health",
        R"({"status": "string", "version": "string"})");
    
    // Example 10: Mixed types
    app.post("/analytics", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({
            "event_id": "evt_123",
            "count": 42,
            "percentage": 85.5,
            "active": true,
            "metadata": {"source": "web"},
            "tags": ["analytics", "metrics"]
        })");
    }, "Track analytics event");
    
    app.set_request_schema(crest::Method::POST, "/analytics",
        R"({"event": "string", "user_id": "number", "properties": "object"})");
    app.set_response_schema(crest::Method::POST, "/analytics",
        R"({"event_id": "string", "count": "number", "percentage": "number", "active": "boolean", "metadata": "object", "tags": "array"})");
    
    std::cout << "Schema Example Server\n";
    std::cout << "====================\n";
    std::cout << "Demonstrating custom schema definitions\n";
    std::cout << "Visit http://localhost:8000/docs to see schemas\n\n";
    std::cout << "Schemas display types:\n";
    std::cout << "  - string: Text values\n";
    std::cout << "  - number: Numeric values (int/float)\n";
    std::cout << "  - boolean: true/false\n";
    std::cout << "  - object: Nested objects\n";
    std::cout << "  - array: Lists/arrays\n";
    std::cout << "  - null: Null values\n\n";
    
    app.run("0.0.0.0", 8000);
    return 0;
}

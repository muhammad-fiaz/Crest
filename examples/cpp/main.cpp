/**
 * @file main.cpp
 * @brief C++ example using Crest framework
 */

#include "crest/crest.hpp"
#include <iostream>

int main() {
    try {
        // Create app with configuration
        crest::Config config;
        config.title = "My C++ API";
        config.description = "Example RESTful API built with Crest";
        config.version = "1.0.0";
        config.docs_enabled = true;
        
        crest::App app(config);
        
        // Register routes
        app.get("/", [](crest::Request& req, crest::Response& res) {
            res.json(crest::Status::OK, R"({"message":"Welcome to Crest API","version":"1.0.0"})");
        }, "Root endpoint");
        
        app.get("/hello", [](crest::Request& req, crest::Response& res) {
            res.json(crest::Status::OK, R"({"message":"Hello from Crest!"})");
        }, "Hello endpoint");
        
        app.post("/users", [](crest::Request& req, crest::Response& res) {
            std::string body = req.body();
            res.json(crest::Status::CREATED, R"({"message":"User created","data":)" + body + "}");
        }, "Create a new user");
        
        app.get("/users/:id", [](crest::Request& req, crest::Response& res) {
            res.json(crest::Status::OK, R"({"id":1,"name":"John Doe","email":"john@example.com"})");
        }, "Get user by ID");
        
        app.put("/users/:id", [](crest::Request& req, crest::Response& res) {
            res.json(crest::Status::OK, R"({"message":"User updated"})");
        }, "Update user by ID");
        
        app.del("/users/:id", [](crest::Request& req, crest::Response& res) {
            res.json(crest::Status::OK, R"({"message":"User deleted"})");
        }, "Delete user by ID");
        
        // Start server
        std::cout << "Starting C++ Crest server..." << std::endl;
        app.run("127.0.0.1", 8000);
        
    } catch (const crest::Exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

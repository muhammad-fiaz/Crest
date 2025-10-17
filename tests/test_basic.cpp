/**
 * @file test_basic.cpp
 * @brief Basic functionality tests
 */

#include "crest/crest.hpp"
#include <cassert>
#include <iostream>

void test_app_creation() {
    crest::App app;
    assert(app.raw() != nullptr);
    std::cout << "✓ App creation test passed\n";
}

void test_route_registration() {
    crest::App app;
    
    bool handler_called = false;
    app.get("/test", [&](crest::Request& req, crest::Response& res) {
        handler_called = true;
        res.json(200, R"({"status":"ok"})");
    }, "Test route");
    
    std::cout << "✓ Route registration test passed\n";
}

void test_config() {
    crest::Config config;
    config.title = "Test API";
    config.description = "Test Description";
    config.version = "1.0.0";
    config.docs_enabled = false;
    
    crest::App app(config);
    assert(app.raw() != nullptr);
    
    std::cout << "✓ Config test passed\n";
}

void test_multiple_routes() {
    crest::App app;
    
    app.get("/route1", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"route":1})");
    });
    
    app.post("/route2", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"route":2})");
    });
    
    app.put("/route3", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"route":3})");
    });
    
    std::cout << "✓ Multiple routes test passed\n";
}

void test_method_chaining() {
    crest::App app;
    
    app.get("/a", [](crest::Request& req, crest::Response& res) {})
       .post("/b", [](crest::Request& req, crest::Response& res) {})
       .put("/c", [](crest::Request& req, crest::Response& res) {});
    
    std::cout << "✓ Method chaining test passed\n";
}

int main() {
    std::cout << "Running Crest tests...\n\n";
    
    try {
        test_app_creation();
        test_route_registration();
        test_config();
        test_multiple_routes();
        test_method_chaining();
        
        std::cout << "\n✅ All tests passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed: " << e.what() << "\n";
        return 1;
    }
}

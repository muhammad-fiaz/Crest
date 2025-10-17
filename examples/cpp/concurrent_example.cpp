/**
 * @file concurrent_example.cpp
 * @brief High-performance concurrent API example
 */

#include "crest/crest.hpp"
#include <mutex>
#include <unordered_map>
#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

int main() {
    // Disable docs to use reserved routes for API
    crest::Config config;
    config.title = "High-Performance Concurrent API";
    config.description = "Handles lakhs of concurrent requests";
    config.version = "1.0.0";
    config.docs_enabled = false;  // Use /docs, /playground for custom routes
    
    crest::App app(config);
    
    // Thread-safe request counter
    std::atomic<uint64_t> request_count{0};
    
    // Thread-safe data store
    std::mutex data_mutex;
    std::unordered_map<std::string, std::string> cache;
    
    // Health check endpoint (no locking needed)
    app.get("/", [&](crest::Request& req, crest::Response& res) {
        uint64_t count = request_count.fetch_add(1);
        res.json(200, "{\"status\":\"ok\",\"requests\":" + std::to_string(count) + "}");
    });
    
    // Fast read endpoint
    app.get("/cache", [&](crest::Request& req, crest::Response& res) {
        request_count.fetch_add(1);
        std::string key = req.query("key");
        
        std::lock_guard<std::mutex> lock(data_mutex);
        auto it = cache.find(key);
        
        if (it != cache.end()) {
            res.json(200, "{\"key\":\"" + key + "\",\"value\":\"" + it->second + "\"}");
        } else {
            res.json(404, "{\"error\":\"Key not found\"}");
        }
    });
    
    // Fast write endpoint
    app.post("/cache", [&](crest::Request& req, crest::Response& res) {
        request_count.fetch_add(1);
        std::string key = req.query("key");
        std::string value = req.body();
        
        std::lock_guard<std::mutex> lock(data_mutex);
        cache[key] = value;
        
        res.json(201, "{\"status\":\"created\",\"key\":\"" + key + "\"}");
    });
    
    // Delete endpoint
    app.del("/cache", [&](crest::Request& req, crest::Response& res) {
        request_count.fetch_add(1);
        std::string key = req.query("key");
        
        std::lock_guard<std::mutex> lock(data_mutex);
        size_t erased = cache.erase(key);
        
        if (erased > 0) {
            res.json(200, "{\"status\":\"deleted\",\"key\":\"" + key + "\"}");
        } else {
            res.json(404, "{\"error\":\"Key not found\"}");
        }
    });
    
    // Stats endpoint
    app.get("/stats", [&](crest::Request& req, crest::Response& res) {
        request_count.fetch_add(1);
        
        std::lock_guard<std::mutex> lock(data_mutex);
        size_t cache_size = cache.size();
        uint64_t total_requests = request_count.load();
        
        res.json(200, "{\"total_requests\":" + std::to_string(total_requests) + 
                      ",\"cache_size\":" + std::to_string(cache_size) + "}");
    });
    
    // Custom docs endpoint (since docs_enabled = false)
    app.get("/docs", [](crest::Request& req, crest::Response& res) {
        res.html(200, 
            "<!DOCTYPE html><html><head><title>API Docs</title></head>"
            "<body><h1>Custom API Documentation</h1>"
            "<p>This demonstrates using reserved routes for your application.</p>"
            "<ul>"
            "<li>GET / - Health check</li>"
            "<li>GET /cache?key=name - Get cached value</li>"
            "<li>POST /cache?key=name - Set cached value</li>"
            "<li>DELETE /cache?key=name - Delete cached value</li>"
            "<li>GET /stats - View statistics</li>"
            "</ul></body></html>");
    });
    
    // Custom playground endpoint
    app.get("/playground", [](crest::Request& req, crest::Response& res) {
        res.json(200, "{\"message\":\"Custom playground - docs disabled\"}");
    });
    
    std::cout << "High-Performance Concurrent API Server\n";
    std::cout << "======================================\n";
    std::cout << "Thread Pool: " << std::thread::hardware_concurrency() * 2 << " workers\n";
    std::cout << "Ready to handle lakhs of concurrent requests!\n\n";
    
    // Start server with automatic thread pool
    app.run("0.0.0.0", 8000);
    
    return 0;
}

# Middleware System

Crest provides a powerful middleware system for request/response processing.

## Overview

Middleware functions execute before route handlers, allowing you to:
- Add CORS headers
- Rate limit requests
- Authenticate users
- Log requests
- Compress responses

## Built-in Middleware

### CORS Middleware

```cpp
#include "crest/middleware.hpp"

crest::CorsMiddleware::Options cors_opts;
cors_opts.allowed_origins = {"https://example.com", "https://app.example.com"};
cors_opts.allowed_methods = {"GET", "POST", "PUT", "DELETE"};
cors_opts.allowed_headers = {"Content-Type", "Authorization"};
cors_opts.allow_credentials = true;
cors_opts.max_age = 86400;

crest::CorsMiddleware cors(cors_opts);
```

### Rate Limiting

```cpp
crest::RateLimitMiddleware::Options rate_opts;
rate_opts.max_requests = 100;
rate_opts.window_seconds = 60;
rate_opts.message = "Too many requests, please try again later";

crest::RateLimitMiddleware rate_limiter(rate_opts);
```

### Authentication

```cpp
auto validator = [](const std::string& token) -> bool {
    return token == "valid-token-123";
};

crest::AuthMiddleware auth(validator);
```

### Logging

```cpp
crest::LoggingMiddleware logger;
```

### Compression

```cpp
crest::CompressionMiddleware compression;
```

## Custom Middleware

Create custom middleware by extending the Middleware class:

```cpp
class CustomMiddleware : public crest::Middleware {
public:
    void handle(crest::Request& req, crest::Response& res, crest::NextFunction next) override {
        // Pre-processing
        std::string api_key = req.header("X-API-Key");
        
        if (api_key.empty()) {
            res.json(401, R"({"error":"Missing API key"})");
            return;
        }
        
        // Call next middleware or handler
        next();
        
        // Post-processing
        res.set_header("X-Processed-By", "CustomMiddleware");
    }
};
```

## Usage

```cpp
#include "crest/crest.hpp"
#include "crest/middleware.hpp"

int main() {
    crest::App app;
    
    // Apply middleware
    crest::CorsMiddleware cors;
    crest::RateLimitMiddleware rate_limiter;
    crest::LoggingMiddleware logger;
    
    app.get("/api/data", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"data":"value"})");
    });
    
    app.run("0.0.0.0", 8000);
    return 0;
}
```

## Middleware Chain

Middleware executes in order:
1. CORS
2. Rate Limiting
3. Authentication
4. Logging
5. Route Handler
6. Compression

## Best Practices

- Apply CORS first for preflight requests
- Use rate limiting to prevent abuse
- Authenticate before accessing protected routes
- Log all requests for debugging
- Compress responses to reduce bandwidth

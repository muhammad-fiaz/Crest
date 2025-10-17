# Crest HTTP Status Codes Guide

Complete guide to using HTTP status codes in Crest.

## Overview

Crest supports **all standard HTTP status codes** (100-599). You can use predefined enums or custom integer values.

## Using Status Codes

### C API

```c
// Use any integer status code
crest_response_json(res, 200, "{\"status\":\"ok\"}");
crest_response_json(res, 401, "{\"error\":\"Unauthorized\"}");
crest_response_json(res, 503, "{\"error\":\"Service Unavailable\"}");
```

### C++ API

```cpp
// Use Status enum
res.json(crest::Status::OK, R"({"status":"ok"})");
res.json(crest::Status::UNAUTHORIZED, R"({"error":"Unauthorized"})");

// Or use any integer
res.json(200, R"({"status":"ok"})");
res.json(401, R"({"error":"Unauthorized"})");
res.json(503, R"({"error":"Service Unavailable"})");
```

## Predefined Status Codes

### C++ Enum (crest::Status)

```cpp
enum class Status {
    OK = 200,
    CREATED = 201,
    ACCEPTED = 202,
    NO_CONTENT = 204,
    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    INTERNAL_ERROR = 500,
    NOT_IMPLEMENTED = 501,
    SERVICE_UNAVAILABLE = 503
};
```

### C Enum (crest_status_t)

```c
typedef enum {
    CREST_STATUS_OK = 200,
    CREST_STATUS_CREATED = 201,
    CREST_STATUS_BAD_REQUEST = 400,
    CREST_STATUS_NOT_FOUND = 404,
    CREST_STATUS_INTERNAL_ERROR = 500
} crest_status_t;
```

## Complete Status Code Reference

### 1xx Informational

| Code | Name | Usage |
|------|------|-------|
| 100 | Continue | Client should continue request |
| 101 | Switching Protocols | Server switching protocols |
| 102 | Processing | Server processing request |

```cpp
res.json(100, R"({"status":"continue"})");
res.json(101, R"({"protocol":"websocket"})");
```

### 2xx Success

| Code | Name | Usage |
|------|------|-------|
| 200 | OK | Request successful |
| 201 | Created | Resource created |
| 202 | Accepted | Request accepted for processing |
| 203 | Non-Authoritative | Modified response |
| 204 | No Content | Success with no body |
| 205 | Reset Content | Reset document view |
| 206 | Partial Content | Partial GET response |

```cpp
res.json(crest::Status::OK, R"({"data":"value"})");
res.json(crest::Status::CREATED, R"({"id":123})");
res.json(crest::Status::ACCEPTED, R"({"queued":true})");
res.json(crest::Status::NO_CONTENT, "");
res.json(206, R"({"range":"bytes 0-1023"})");
```

### 3xx Redirection

| Code | Name | Usage |
|------|------|-------|
| 300 | Multiple Choices | Multiple options available |
| 301 | Moved Permanently | Resource moved permanently |
| 302 | Found | Temporary redirect |
| 303 | See Other | See other URI |
| 304 | Not Modified | Cached version valid |
| 307 | Temporary Redirect | Temporary redirect (preserve method) |
| 308 | Permanent Redirect | Permanent redirect (preserve method) |

```cpp
res.set_header("Location", "/new-path");
res.json(301, R"({"moved":"/new-path"})");

res.set_header("Location", "/other");
res.json(302, R"({"redirect":"/other"})");

res.json(304, "");  // Not Modified
```

### 4xx Client Errors

| Code | Name | Usage |
|------|------|-------|
| 400 | Bad Request | Invalid request |
| 401 | Unauthorized | Authentication required |
| 402 | Payment Required | Payment required |
| 403 | Forbidden | Access denied |
| 404 | Not Found | Resource not found |
| 405 | Method Not Allowed | HTTP method not allowed |
| 406 | Not Acceptable | Cannot produce acceptable response |
| 407 | Proxy Authentication Required | Proxy auth required |
| 408 | Request Timeout | Request timeout |
| 409 | Conflict | Request conflicts with current state |
| 410 | Gone | Resource permanently deleted |
| 411 | Length Required | Content-Length required |
| 412 | Precondition Failed | Precondition failed |
| 413 | Payload Too Large | Request entity too large |
| 414 | URI Too Long | URI too long |
| 415 | Unsupported Media Type | Unsupported media type |
| 416 | Range Not Satisfiable | Range not satisfiable |
| 417 | Expectation Failed | Expectation failed |
| 418 | I'm a teapot | April Fools' joke (RFC 2324) |
| 422 | Unprocessable Entity | Validation error |
| 423 | Locked | Resource locked |
| 424 | Failed Dependency | Failed dependency |
| 425 | Too Early | Too early |
| 426 | Upgrade Required | Upgrade required |
| 428 | Precondition Required | Precondition required |
| 429 | Too Many Requests | Rate limit exceeded |
| 431 | Request Header Fields Too Large | Headers too large |
| 451 | Unavailable For Legal Reasons | Censored |

```cpp
// Authentication errors
res.json(crest::Status::UNAUTHORIZED, R"({"error":"Login required"})");
res.json(crest::Status::FORBIDDEN, R"({"error":"Access denied"})");

// Validation errors
res.json(crest::Status::BAD_REQUEST, R"({"error":"Invalid input"})");
res.json(422, R"({"errors":["Name required","Email invalid"]})");

// Resource errors
res.json(crest::Status::NOT_FOUND, R"({"error":"User not found"})");
res.json(409, R"({"error":"Email already exists"})");
res.json(410, R"({"error":"Resource deleted"})");

// Rate limiting
res.json(429, R"({"error":"Too many requests","retry_after":60})");

// Method errors
res.json(crest::Status::METHOD_NOT_ALLOWED, R"({"error":"Method not allowed"})");
res.json(405, R"({"allowed":["GET","POST"]})");
```

### 5xx Server Errors

| Code | Name | Usage |
|------|------|-------|
| 500 | Internal Server Error | Server error |
| 501 | Not Implemented | Feature not implemented |
| 502 | Bad Gateway | Invalid gateway response |
| 503 | Service Unavailable | Service unavailable |
| 504 | Gateway Timeout | Gateway timeout |
| 505 | HTTP Version Not Supported | HTTP version not supported |
| 506 | Variant Also Negotiates | Configuration error |
| 507 | Insufficient Storage | Insufficient storage |
| 508 | Loop Detected | Infinite loop detected |
| 510 | Not Extended | Further extensions required |
| 511 | Network Authentication Required | Network auth required |

```cpp
// Server errors
res.json(crest::Status::INTERNAL_ERROR, R"({"error":"Internal error"})");
res.json(crest::Status::NOT_IMPLEMENTED, R"({"error":"Feature not implemented"})");
res.json(crest::Status::SERVICE_UNAVAILABLE, R"({"error":"Service unavailable"})");

// Gateway errors
res.json(502, R"({"error":"Bad gateway"})");
res.json(504, R"({"error":"Gateway timeout"})");

// Storage errors
res.json(507, R"({"error":"Insufficient storage"})");
```

## Practical Examples

### Authentication System

```cpp
app.post("/login", [](crest::Request& req, crest::Response& res) {
    std::string body = req.body();
    
    if (body.empty()) {
        res.json(400, R"({"error":"Body required"})");
        return;
    }
    
    // Parse credentials...
    bool valid = check_credentials(body);
    
    if (!valid) {
        res.json(401, R"({"error":"Invalid credentials"})");
        return;
    }
    
    res.json(200, R"({"token":"abc123","expires":3600})");
});

app.get("/protected", [](crest::Request& req, crest::Response& res) {
    std::string token = req.header("Authorization");
    
    if (token.empty()) {
        res.json(401, R"({"error":"Authorization header required"})");
        return;
    }
    
    if (!verify_token(token)) {
        res.json(403, R"({"error":"Invalid or expired token"})");
        return;
    }
    
    res.json(200, R"({"data":"protected resource"})");
});
```

### CRUD Operations

```cpp
// Create
app.post("/users", [](crest::Request& req, crest::Response& res) {
    std::string body = req.body();
    
    if (!validate_user(body)) {
        res.json(422, R"({"errors":["Name required","Email invalid"]})");
        return;
    }
    
    if (user_exists(body)) {
        res.json(409, R"({"error":"User already exists"})");
        return;
    }
    
    int id = create_user(body);
    res.json(201, "{\"id\":" + std::to_string(id) + "}");
});

// Read
app.get("/users", [](crest::Request& req, crest::Response& res) {
    std::string id = req.query("id");
    
    if (id.empty()) {
        res.json(400, R"({"error":"ID parameter required"})");
        return;
    }
    
    auto user = get_user(id);
    if (!user) {
        res.json(404, R"({"error":"User not found"})");
        return;
    }
    
    res.json(200, user_to_json(user));
});

// Update
app.put("/users", [](crest::Request& req, crest::Response& res) {
    std::string id = req.query("id");
    std::string body = req.body();
    
    if (!user_exists(id)) {
        res.json(404, R"({"error":"User not found"})");
        return;
    }
    
    if (!validate_user(body)) {
        res.json(422, R"({"errors":["Invalid data"]})");
        return;
    }
    
    update_user(id, body);
    res.json(200, R"({"status":"updated"})");
});

// Delete
app.del("/users", [](crest::Request& req, crest::Response& res) {
    std::string id = req.query("id");
    
    if (!user_exists(id)) {
        res.json(404, R"({"error":"User not found"})");
        return;
    }
    
    delete_user(id);
    res.json(204, "");  // No Content
});
```

### Rate Limiting

```cpp
std::map<std::string, int> rate_limits;
std::mutex rate_mutex;

app.get("/api/data", [&](crest::Request& req, crest::Response& res) {
    std::string ip = req.header("X-Forwarded-For");
    
    std::lock_guard<std::mutex> lock(rate_mutex);
    
    if (rate_limits[ip] >= 100) {
        res.set_header("Retry-After", "60");
        res.json(429, R"({"error":"Rate limit exceeded","retry_after":60})");
        return;
    }
    
    rate_limits[ip]++;
    res.json(200, R"({"data":"value"})");
});
```

### Service Health Check

```cpp
app.get("/health", [](crest::Request& req, crest::Response& res) {
    bool db_ok = check_database();
    bool cache_ok = check_cache();
    
    if (!db_ok || !cache_ok) {
        res.json(503, R"({"status":"unhealthy","database":)" + 
                 std::string(db_ok ? "true" : "false") + 
                 ",\"cache\":" + std::string(cache_ok ? "true" : "false") + "}");
        return;
    }
    
    res.json(200, R"({"status":"healthy"})");
});
```

### Maintenance Mode

```cpp
bool maintenance_mode = false;

app.get("/api/*", [&](crest::Request& req, crest::Response& res) {
    if (maintenance_mode) {
        res.set_header("Retry-After", "3600");
        res.json(503, R"({"error":"Service under maintenance","retry_after":3600})");
        return;
    }
    
    // Normal processing...
    res.json(200, R"({"data":"value"})");
});
```

### Feature Flags

```cpp
bool feature_enabled = false;

app.get("/beta/feature", [&](crest::Request& req, crest::Response& res) {
    if (!feature_enabled) {
        res.json(501, R"({"error":"Feature not implemented yet"})");
        return;
    }
    
    res.json(200, R"({"feature":"data"})");
});
```

## Best Practices

### 1. Use Appropriate Status Codes
```cpp
// ✅ Good
res.json(404, R"({"error":"User not found"})");

// ❌ Bad
res.json(200, R"({"error":"User not found"})");
```

### 2. Include Error Details
```cpp
// ✅ Good
res.json(400, R"({"error":"Invalid input","field":"email","message":"Email format invalid"})");

// ❌ Bad
res.json(400, R"({"error":"Error"})");
```

### 3. Use 201 for Creation
```cpp
// ✅ Good
res.json(201, R"({"id":123,"created":true})");

// ❌ Bad
res.json(200, R"({"id":123})");
```

### 4. Use 204 for No Content
```cpp
// ✅ Good
res.json(204, "");

// ❌ Bad
res.json(200, R"({"status":"deleted"})");
```

### 5. Set Headers for Redirects
```cpp
// ✅ Good
res.set_header("Location", "/new-path");
res.json(301, R"({"moved":"/new-path"})");

// ❌ Bad
res.json(301, R"({"moved":"/new-path"})");  // Missing Location header
```

## Summary

Crest supports **all HTTP status codes**:
- ✅ Use predefined enums: `crest::Status::OK`, `CREST_STATUS_OK`
- ✅ Use custom integers: `res.json(401, ...)`, `crest_response_json(res, 503, ...)`
- ✅ All standard codes: 100-599
- ✅ Common codes: 200, 201, 400, 401, 403, 404, 500, 501, 502, 503
- ✅ Custom codes: Any integer value

**You have complete control over HTTP status codes in Crest!** 🌊

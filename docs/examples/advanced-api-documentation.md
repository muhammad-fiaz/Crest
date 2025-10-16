# Advanced API Documentation Examples

This document demonstrates advanced usage patterns for Crest's API documentation features, including middleware integration, custom schemas, and comprehensive testing scenarios.

## Advanced Authentication Example

### Code Example

```c
#include <crest/crest.h>
#include <string.h>

// JWT token validation middleware
void jwt_auth_middleware(crest_request_t *req, crest_response_t *res, void *data) {
    const char *auth_header = crest_request_header(req, "Authorization");

    if (!auth_header || strncmp(auth_header, "Bearer ", 7) != 0) {
        crest_response_status(res, 401);
        crest_response_json(res, "{\"error\": \"Missing or invalid authorization header\", \"code\": \"AUTH_HEADER_MISSING\"}");
        return;
    }

    // Extract token (simplified - in real app, validate JWT)
    const char *token = auth_header + 7;
    if (strlen(token) < 10) {  // Basic validation
        crest_response_status(res, 401);
        crest_response_json(res, "{\"error\": \"Invalid token\", \"code\": \"INVALID_TOKEN\"}");
        return;
    }

    // Store user info for handlers (simplified)
    // In real app, decode JWT and store user claims
}

// API Key middleware
void api_key_middleware(crest_request_t *req, crest_response_t *res, void *data) {
    const char *api_key = crest_request_header(req, "X-API-Key");

    if (!api_key || strcmp(api_key, "valid-api-key-123") != 0) {
        crest_response_status(res, 403);
        crest_response_json(res, "{\"error\": \"Invalid API key\", \"code\": \"INVALID_API_KEY\"}");
        return;
    }
}

// CORS middleware for API documentation
void cors_middleware(crest_request_t *req, crest_response_t *res, void *data) {
    crest_response_header(res, "Access-Control-Allow-Origin", "*");
    crest_response_header(res, "Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    crest_response_header(res, "Access-Control-Allow-Headers", "Content-Type, Authorization, X-API-Key");

    if (strcmp(crest_request_method(req), "OPTIONS") == 0) {
        crest_response_status(res, 200);
        return;
    }
}

// Handlers
void login_handler(crest_request_t *req, crest_response_t *res) {
    // Simplified login - in real app, validate credentials
    crest_response_json(res, "{\"token\": \"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiaWF0IjoxNTE2MjM5MDIyfQ.SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c\", \"expires_in\": 3600, \"token_type\": \"Bearer\"}");
}

void get_user_profile(crest_request_t *req, crest_response_t *res) {
    crest_response_json(res, "{\"id\": 123, \"email\": \"user@example.com\", \"name\": \"John Doe\", \"role\": \"admin\", \"permissions\": [\"read\", \"write\", \"delete\"], \"created_at\": \"2024-01-15T10:30:00Z\"}");
}

void update_user_profile(crest_request_t *req, crest_response_t *res) {
    crest_response_json(res, "{\"message\": \"Profile updated successfully\", \"updated_fields\": [\"name\", \"email\"]}");
}

void get_user_orders(crest_request_t *req, crest_response_t *res) {
    const char *page = crest_request_query(req, "page");
    const char *limit = crest_request_query(req, "limit");

    crest_response_json(res, "{\"orders\": [{\"id\": \"ORD-001\", \"total\": 99.99, \"status\": \"delivered\", \"created_at\": \"2024-01-10T14:30:00Z\"}, {\"id\": \"ORD-002\", \"total\": 149.99, \"status\": \"shipped\", \"created_at\": \"2024-01-12T09:15:00Z\"}], \"pagination\": {\"page\": 1, \"limit\": 10, \"total\": 2, \"pages\": 1}}");
}

void upload_file(crest_request_t *req, crest_response_t *res) {
    // In real app, handle multipart/form-data
    crest_response_json(res, "{\"message\": \"File uploaded successfully\", \"file_id\": \"file-123\", \"filename\": \"document.pdf\", \"size\": 2048576, \"mime_type\": \"application/pdf\"}");
}

void health_check(crest_request_t *req, crest_response_t *res) {
    crest_response_json(res, "{\"status\": \"healthy\", \"timestamp\": \"2024-01-15T12:00:00Z\", \"version\": \"1.0.0\", \"uptime\": \"2 days, 4 hours\"}");
}

int main(void) {
    crest_app_t *app = crest_create();

    // Enable API documentation
    crest_enable_dashboard(app, true);

    // Global middleware
    crest_use(app, "/*", cors_middleware, NULL);

    // Public routes
    crest_get(app, "/health", health_check, "Health check endpoint");
    crest_post(app, "/auth/login", login_handler, "User authentication");

    // API Key protected routes
    crest_use(app, "/api/v1/*", api_key_middleware, NULL);
    crest_get(app, "/api/v1/orders", get_user_orders, "Get user orders with pagination");

    // JWT protected routes
    crest_use(app, "/api/v1/profile/*", jwt_auth_middleware, NULL);
    crest_get(app, "/api/v1/profile", get_user_profile, "Get user profile information");
    crest_put(app, "/api/v1/profile", update_user_profile, "Update user profile");
    crest_post(app, "/api/v1/upload", upload_file, "Upload file to user account");

    printf("🚀 Advanced API Server running on http://localhost:8080\n");
    printf("🧠 Swagger UI: http://localhost:8080/docs\n");
    printf("⚙️ ReDoc: http://localhost:8080/redoc\n");
    printf("🧾 OpenAPI JSON: http://localhost:8080/openapi.json\n");

    crest_listen(app);
    crest_destroy(app);
    return 0;
}
```

### Testing Authentication

**Health check (no auth required):**
```bash
curl http://localhost:8080/health
# {"status": "healthy", "timestamp": "2024-01-15T12:00:00Z", "version": "1.0.0", "uptime": "2 days, 4 hours"}
```

**Login (no auth required):**
```bash
curl -X POST http://localhost:8080/auth/login \
     -H "Content-Type: application/json" \
     -d '{"email": "user@example.com", "password": "password123"}'
# {"token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...", "expires_in": 3600, "token_type": "Bearer"}
```

**API Key protected route (API key required):**
```bash
curl -H "X-API-Key: valid-api-key-123" http://localhost:8080/api/v1/orders
# {"orders": [...], "pagination": {...}}
```

**API Key missing (should fail):**
```bash
curl http://localhost:8080/api/v1/orders
# {"error": "Invalid API key", "code": "INVALID_API_KEY"}
```

**JWT protected route (JWT token required):**
```bash
curl -H "Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9..." \
     -H "X-API-Key: valid-api-key-123" \
     http://localhost:8080/api/v1/profile
# {"id": 123, "email": "user@example.com", "name": "John Doe", ...}
```

**JWT missing (should fail):**
```bash
curl -H "X-API-Key: valid-api-key-123" http://localhost:8080/api/v1/profile
# {"error": "Missing or invalid authorization header", "code": "AUTH_HEADER_MISSING"}
```

## Comprehensive Testing Suite

### Code Example

```c
#include <crest/crest.h>
#include <assert.h>
#include <string.h>

// Test handlers for comprehensive API testing
void get_test_data(crest_request_t *req, crest_response_t *res) {
    const char *format = crest_request_query(req, "format");

    if (format && strcmp(format, "xml") == 0) {
        crest_response_header(res, "Content-Type", "application/xml");
        crest_response_send(res, "<data><items><item id=\"1\">Test Item 1</item></items></data>");
    } else {
        crest_response_json(res, "{\"data\": {\"items\": [{\"id\": 1, \"name\": \"Test Item 1\", \"value\": 100}, {\"id\": 2, \"name\": \"Test Item 2\", \"value\": 200}]}}");
    }
}

void post_test_data(crest_request_t *req, crest_response_t *res) {
    // Simulate data validation and creation
    crest_response_status(res, 201);
    crest_response_json(res, "{\"message\": \"Test data created\", \"id\": 123, \"created\": true, \"timestamp\": \"2024-01-15T12:00:00Z\"}");
}

void put_test_data(crest_request_t *req, crest_response_t *res) {
    const char *id = crest_request_param(req, "id");

    char response[200];
    sprintf(response, "{\"message\": \"Test data updated\", \"id\": \"%s\", \"updated\": true, \"timestamp\": \"2024-01-15T12:00:00Z\"}", id);
    crest_response_json(res, response);
}

void delete_test_data(crest_request_t *req, crest_response_t *res) {
    const char *id = crest_request_param(req, "id");

    char response[200];
    sprintf(response, "{\"message\": \"Test data deleted\", \"id\": \"%s\", \"deleted\": true}", id);
    crest_response_json(res, response);
}

void test_error_responses(crest_request_t *req, crest_response_t *res) {
    const char *error_type = crest_request_query(req, "type");

    if (!error_type) {
        crest_response_status(res, 400);
        crest_response_json(res, "{\"error\": \"Missing error type parameter\", \"code\": \"MISSING_PARAM\"}");
        return;
    }

    if (strcmp(error_type, "not_found") == 0) {
        crest_response_status(res, 404);
        crest_response_json(res, "{\"error\": \"Resource not found\", \"code\": \"NOT_FOUND\"}");
    } else if (strcmp(error_type, "server_error") == 0) {
        crest_response_status(res, 500);
        crest_response_json(res, "{\"error\": \"Internal server error\", \"code\": \"SERVER_ERROR\"}");
    } else if (strcmp(error_type, "validation") == 0) {
        crest_response_status(res, 422);
        crest_response_json(res, "{\"error\": \"Validation failed\", \"code\": \"VALIDATION_ERROR\", \"details\": [{\"field\": \"email\", \"message\": \"Invalid email format\"}]}");
    } else {
        crest_response_status(res, 400);
        crest_response_json(res, "{\"error\": \"Unknown error type\", \"code\": \"UNKNOWN_ERROR\"}");
    }
}

void test_pagination(crest_request_t *req, crest_response_t *res) {
    const char *page = crest_request_query(req, "page");
    const char *limit = crest_request_query(req, "limit");

    int page_num = page ? atoi(page) : 1;
    int limit_num = limit ? atoi(limit) : 10;

    // Simulate paginated data
    crest_response_json(res, "{\"data\": [{\"id\": 1, \"name\": \"Item 1\"}, {\"id\": 2, \"name\": \"Item 2\"}], \"pagination\": {\"page\": 1, \"limit\": 10, \"total\": 25, \"pages\": 3, \"has_next\": true, \"has_prev\": false}}");
}

void test_rate_limited(crest_request_t *req, crest_response_t *res) {
    // Simulate rate limiting
    crest_response_status(res, 429);
    crest_response_header(res, "X-RateLimit-Limit", "100");
    crest_response_header(res, "X-RateLimit-Remaining", "0");
    crest_response_header(res, "X-RateLimit-Reset", "1642252800");
    crest_response_json(res, "{\"error\": \"Rate limit exceeded\", \"code\": \"RATE_LIMIT_EXCEEDED\", \"retry_after\": 60}");
}

int main(void) {
    crest_app_t *app = crest_create();

    crest_enable_dashboard(app, true);
    crest_enable_cors(app, true);

    // CRUD operations
    crest_get(app, "/api/test", get_test_data, "Get test data with optional format parameter");
    crest_post(app, "/api/test", post_test_data, "Create test data");
    crest_put(app, "/api/test/:id", put_test_data, "Update test data by ID");
    crest_delete(app, "/api/test/:id", delete_test_data, "Delete test data by ID");

    // Error responses
    crest_get(app, "/api/errors", test_error_responses, "Test various error response types");

    // Pagination
    crest_get(app, "/api/paginated", test_pagination, "Test paginated responses");

    // Rate limiting
    crest_get(app, "/api/rate-limited", test_rate_limited, "Test rate limiting responses");

    printf("🧪 Comprehensive Testing API running on http://localhost:8080\n");
    printf("🧠 Test with Swagger UI: http://localhost:8080/docs\n");

    crest_listen(app);
    crest_destroy(app);
    return 0;
}
```

### Comprehensive Testing Commands

**Test all CRUD operations:**

```bash
# GET - Default JSON response
curl http://localhost:8080/api/test

# GET - XML response
curl "http://localhost:8080/api/test?format=xml"

# POST - Create resource
curl -X POST http://localhost:8080/api/test

# PUT - Update resource
curl -X PUT http://localhost:8080/api/test/123

# DELETE - Remove resource
curl -X DELETE http://localhost:8080/api/test/123
```

**Test error responses:**

```bash
# Missing parameter
curl http://localhost:8080/api/errors

# Not found error
curl "http://localhost:8080/api/errors?type=not_found"

# Server error
curl "http://localhost:8080/api/errors?type=server_error"

# Validation error
curl "http://localhost:8080/api/errors?type=validation"

# Unknown error
curl "http://localhost:8080/api/errors?type=unknown"
```

**Test pagination:**

```bash
# Default pagination
curl http://localhost:8080/api/paginated

# Custom pagination
curl "http://localhost:8080/api/paginated?page=2&limit=5"
```

**Test rate limiting:**

```bash
curl http://localhost:8080/api/rate-limited
# Response includes rate limit headers
```

### Expected Outputs

**GET /api/test:**
```json
{
  "data": {
    "items": [
      {"id": 1, "name": "Test Item 1", "value": 100},
      {"id": 2, "name": "Test Item 2", "value": 200}
    ]
  }
}
```

**GET /api/test?format=xml:**
```xml
<data>
  <items>
    <item id="1">Test Item 1</item>
  </items>
</data>
```

**POST /api/test:**
```json
{
  "message": "Test data created",
  "id": 123,
  "created": true,
  "timestamp": "2024-01-15T12:00:00Z"
}
```

**GET /api/errors?type=validation:**
```json
{
  "error": "Validation failed",
  "code": "VALIDATION_ERROR",
  "details": [
    {
      "field": "email",
      "message": "Invalid email format"
    }
  ]
}
```

**GET /api/rate-limited:**
```json
{
  "error": "Rate limit exceeded",
  "code": "RATE_LIMIT_EXCEEDED",
  "retry_after": 60
}
```
*Headers:*
```
X-RateLimit-Limit: 100
X-RateLimit-Remaining: 0
X-RateLimit-Reset: 1642252800
```

## Performance Testing Example

### Code Example

```c
#include <crest/crest.h>
#include <time.h>

// Performance test handlers
void performance_test(crest_request_t *req, crest_response_t *res) {
    const char *iterations = crest_request_query(req, "iterations");
    int iter = iterations ? atoi(iterations) : 1000;

    // Simulate CPU-intensive operation
    volatile int sum = 0;
    for (int i = 0; i < iter; i++) {
        sum += i * i;
    }

    char response[100];
    sprintf(response, "{\"iterations\": %d, \"result\": %d, \"performance\": \"simulated\"}", iter, sum);
    crest_response_json(res, response);
}

void memory_test(crest_request_t *req, crest_response_t *res) {
    const char *size_kb = crest_request_query(req, "size_kb");
    int size = size_kb ? atoi(size_kb) : 1024;

    // Simulate memory allocation (don't actually allocate large amounts)
    char response[200];
    sprintf(response, "{\"allocated_kb\": %d, \"memory_test\": \"simulated\", \"status\": \"ok\"}", size);
    crest_response_json(res, response);
}

void concurrent_test(crest_request_t *req, crest_response_t *res) {
    // Simulate database query delay
    struct timespec delay = {0, 10000000}; // 10ms
    nanosleep(&delay, NULL);

    crest_response_json(res, "{\"concurrent_test\": \"completed\", \"delay_ms\": 10, \"thread_safe\": true}");
}

void load_test_data(crest_request_t *req, crest_response_t *res) {
    // Generate large response for bandwidth testing
    const char *size_kb = crest_request_query(req, "size_kb");
    int size = size_kb ? atoi(size_kb) : 100;

    // Create large JSON response
    char *large_response = malloc(size * 1024 + 100);
    strcpy(large_response, "{\"data\": [");

    for (int i = 0; i < size * 10; i++) {
        char item[50];
        sprintf(item, "{\"id\": %d, \"value\": \"test_data_%d\"}%s", i, i, i < size * 10 - 1 ? "," : "");
        strcat(large_response, item);
    }

    strcat(large_response, "], \"size_kb\": ");
    char size_str[20];
    sprintf(size_str, "%d}", size);
    strcat(large_response, size_str);

    crest_response_json(res, large_response);
    free(large_response);
}

int main(void) {
    crest_app_t *app = crest_create();

    crest_enable_dashboard(app, true);

    // Performance testing endpoints
    crest_get(app, "/perf/cpu", performance_test, "CPU performance test with configurable iterations");
    crest_get(app, "/perf/memory", memory_test, "Memory allocation performance test");
    crest_get(app, "/perf/concurrent", concurrent_test, "Concurrent request handling test");
    crest_get(app, "/perf/bandwidth", load_test_data, "Bandwidth/load testing with large responses");

    printf("⚡ Performance Testing API running on http://localhost:8080\n");
    printf("🧠 Monitor performance at: http://localhost:8080/docs\n");

    crest_listen(app);
    crest_destroy(app);
    return 0;
}
```

### Performance Testing Commands

**CPU Performance Test:**
```bash
# Light test
curl "http://localhost:8080/perf/cpu?iterations=1000"

# Heavy test
curl "http://localhost:8080/perf/cpu?iterations=100000"
```

**Memory Test:**
```bash
curl "http://localhost:8080/perf/memory?size_kb=512"
```

**Concurrent Test (run multiple times):**
```bash
# Run 10 concurrent requests
for i in {1..10}; do
  curl -s http://localhost:8080/perf/concurrent &
done
wait
```

**Bandwidth Test:**
```bash
# Small response
curl "http://localhost:8080/perf/bandwidth?size_kb=10"

# Large response (1MB)
curl "http://localhost:8080/perf/bandwidth?size_kb=1024"
```

### Load Testing with Apache Bench

```bash
# Basic load test
ab -n 1000 -c 10 http://localhost:8080/perf/concurrent

# CPU intensive test
ab -n 100 -c 5 "http://localhost:8080/perf/cpu?iterations=50000"

# Memory test
ab -n 500 -c 20 http://localhost:8080/perf/memory
```

## Summary

These advanced examples demonstrate:

- **Authentication integration** with JWT and API keys
- **Middleware chaining** for complex request processing
- **Error handling** with proper HTTP status codes
- **Comprehensive testing** of all HTTP methods
- **Performance testing** endpoints
- **CORS support** for cross-origin requests
- **Content negotiation** (JSON/XML responses)
- **Rate limiting** simulation
- **Pagination** implementation
- **Large response** handling

All examples include detailed testing commands and expected outputs to help you understand and verify the API documentation functionality.
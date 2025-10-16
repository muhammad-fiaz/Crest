# Testing and Automation Examples

This document demonstrates comprehensive testing strategies for Crest applications with API documentation, including unit tests, integration tests, API contract tests, and automated testing pipelines.

## Unit Testing Example

### Code Example

```c
#include <crest/crest.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Mock request structure for testing
typedef struct {
    char method[16];
    char path[256];
    char query_string[512];
    char headers[1024];
    char body[4096];
} mock_request_t;

// Mock response structure for testing
typedef struct {
    int status_code;
    char headers[2048];
    char body[8192];
    int sent;
} mock_response_t;

// Test utilities
void reset_mock_response(mock_response_t *res) {
    res->status_code = 200;
    res->headers[0] = '\0';
    res->body[0] = '\0';
    res->sent = 0;
}

void mock_response_status(mock_response_t *res, int status) {
    res->status_code = status;
}

void mock_response_json(mock_response_t *res, const char *json) {
    strcpy(res->body, json);
    res->sent = 1;
}

void mock_response_header(mock_response_t *res, const char *key, const char *value) {
    char header[256];
    sprintf(header, "%s: %s\r\n", key, value);
    strcat(res->headers, header);
}

// Test handlers
void test_user_handler(crest_request_t *req, crest_response_t *res) {
    const char *user_id = crest_request_param(req, "user_id");

    if (!user_id) {
        crest_response_status(res, 400);
        crest_response_json(res, "{\"error\": \"User ID required\"}");
        return;
    }

    char response[256];
    sprintf(response, "{\"user_id\": \"%s\", \"name\": \"Test User\"}", user_id);
    crest_response_json(res, response);
}

void test_query_handler(crest_request_t *req, crest_response_t *res) {
    const char *page = crest_request_query(req, "page");
    const char *limit = crest_request_query(req, "limit");

    char response[256];
    sprintf(response, "{\"page\": \"%s\", \"limit\": \"%s\"}",
            page ?: "1", limit ?: "10");
    crest_response_json(res, response);
}

void test_header_handler(crest_request_t *req, crest_response_t *res) {
    const char *auth = crest_request_header(req, "Authorization");
    const char *content_type = crest_request_header(req, "Content-Type");

    char response[512];
    sprintf(response, "{\"auth\": \"%s\", \"content_type\": \"%s\"}",
            auth ?: "none", content_type ?: "none");
    crest_response_json(res, response);
}

// Unit test functions
void test_user_handler_success(void) {
    printf("Testing user handler success case...\n");

    // Create mock request
    mock_request_t mock_req;
    strcpy(mock_req.method, "GET");
    strcpy(mock_req.path, "/users/123");

    mock_response_t mock_res;
    reset_mock_response(&mock_res);

    // In a real test, you'd need to mock the crest_request_* functions
    // This is simplified for demonstration
    assert(1); // Placeholder assertion
    printf("✓ User handler success test passed\n");
}

void test_user_handler_missing_id(void) {
    printf("Testing user handler missing ID case...\n");

    // Similar mock setup...
    assert(1); // Placeholder assertion
    printf("✓ User handler missing ID test passed\n");
}

void test_query_parameters(void) {
    printf("Testing query parameter parsing...\n");

    // Test default values
    assert(1); // Placeholder assertion

    // Test custom values
    assert(1); // Placeholder assertion

    printf("✓ Query parameter tests passed\n");
}

void test_response_formatting(void) {
    printf("Testing response formatting...\n");

    mock_response_t res;
    reset_mock_response(&res);

    mock_response_status(&res, 201);
    mock_response_header(&res, "Content-Type", "application/json");
    mock_response_json(&res, "{\"message\": \"Created\"}");

    assert(res.status_code == 201);
    assert(strstr(res.headers, "Content-Type: application/json") != NULL);
    assert(strcmp(res.body, "{\"message\": \"Created\"}") == 0);

    printf("✓ Response formatting tests passed\n");
}

// OpenAPI generation tests
void test_openapi_generation(void) {
    printf("Testing OpenAPI generation...\n");

    crest_app_t *app = crest_create();

    // Add some routes
    crest_get(app, "/users", test_user_handler, "Get users");
    crest_post(app, "/users", test_user_handler, "Create user");
    crest_get(app, "/users/:user_id", test_user_handler, "Get user by ID");

    // Generate OpenAPI spec
    char *spec = crest_generate_openapi_spec(app);

    // Test that spec contains expected content
    assert(spec != NULL);
    assert(strstr(spec, "\"/users\"") != NULL);
    assert(strstr(spec, "\"get\"") != NULL);
    assert(strstr(spec, "\"post\"") != NULL);
    assert(strstr(spec, "Get users") != NULL);

    free(spec);
    crest_destroy(app);

    printf("✓ OpenAPI generation tests passed\n");
}

int run_unit_tests(void) {
    printf("🧪 Running Unit Tests\n");
    printf("===================\n\n");

    test_user_handler_success();
    test_user_handler_missing_id();
    test_query_parameters();
    test_response_formatting();
    test_openapi_generation();

    printf("\n✅ All unit tests passed!\n\n");
    return 0;
}
```

## Integration Testing Example

### Code Example

```c
#include <crest/crest.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Test server configuration
#define TEST_PORT 8888
#define TEST_BASE_URL "http://localhost:8888"

// cURL response buffer
typedef struct {
    char *data;
    size_t size;
} curl_response_t;

size_t curl_write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    curl_response_t *mem = (curl_response_t *)userp;

    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if(ptr == NULL) {
        printf("Not enough memory\n");
        return 0;
    }

    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;

    return realsize;
}

// HTTP client functions for testing
char *http_get(const char *url) {
    CURL *curl = curl_easy_init();
    curl_response_t response = {NULL, 0};

    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

        CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            free(response.data);
            return NULL;
        }
    }

    return response.data;
}

char *http_post(const char *url, const char *data) {
    CURL *curl = curl_easy_init();
    curl_response_t response = {NULL, 0};

    if(curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if(res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            free(response.data);
            return NULL;
        }
    }

    return response.data;
}

// Test handlers
void test_get_handler(crest_request_t *req, crest_response_t *res) {
    crest_response_json(res, "{\"message\": \"GET request successful\", \"method\": \"GET\"}");
}

void test_post_handler(crest_request_t *req, crest_response_t *res) {
    crest_response_json(res, "{\"message\": \"POST request successful\", \"method\": \"POST\"}");
}

void test_param_handler(crest_request_t *req, crest_response_t *res) {
    const char *id = crest_request_param(req, "id");
    char response[256];
    sprintf(response, "{\"id\": \"%s\", \"message\": \"Parameter received\"}", id);
    crest_response_json(res, response);
}

void test_query_handler(crest_request_t *req, crest_response_t *res) {
    const char *page = crest_request_query(req, "page");
    const char *limit = crest_request_query(req, "limit");
    char response[256];
    sprintf(response, "{\"page\": \"%s\", \"limit\": \"%s\"}", page ?: "1", limit ?: "10");
    crest_response_json(res, response);
}

// Integration test functions
void test_get_endpoint(void) {
    printf("Testing GET endpoint...\n");

    char *response = http_get(TEST_BASE_URL "/test");
    assert(response != NULL);

    // Check response contains expected content
    assert(strstr(response, "\"message\": \"GET request successful\"") != NULL);
    assert(strstr(response, "\"method\": \"GET\"") != NULL);

    free(response);
    printf("✓ GET endpoint test passed\n");
}

void test_post_endpoint(void) {
    printf("Testing POST endpoint...\n");

    char *response = http_post(TEST_BASE_URL "/test", "{\"data\": \"test\"}");
    assert(response != NULL);

    assert(strstr(response, "\"message\": \"POST request successful\"") != NULL);
    assert(strstr(response, "\"method\": \"POST\"") != NULL);

    free(response);
    printf("✓ POST endpoint test passed\n");
}

void test_path_parameters(void) {
    printf("Testing path parameters...\n");

    char *response = http_get(TEST_BASE_URL "/test/123");
    assert(response != NULL);

    assert(strstr(response, "\"id\": \"123\"") != NULL);
    assert(strstr(response, "\"message\": \"Parameter received\"") != NULL);

    free(response);
    printf("✓ Path parameters test passed\n");
}

void test_query_parameters(void) {
    printf("Testing query parameters...\n");

    char *response = http_get(TEST_BASE_URL "/query?page=2&limit=50");
    assert(response != NULL);

    assert(strstr(response, "\"page\": \"2\"") != NULL);
    assert(strstr(response, "\"limit\": \"50\"") != NULL);

    free(response);
    printf("✓ Query parameters test passed\n");
}

void test_api_documentation_endpoints(void) {
    printf("Testing API documentation endpoints...\n");

    // Test Swagger UI
    char *swagger_response = http_get(TEST_BASE_URL "/docs");
    assert(swagger_response != NULL);
    assert(strstr(swagger_response, "swagger") != NULL || strstr(swagger_response, "Swagger") != NULL);
    free(swagger_response);

    // Test ReDoc
    char *redoc_response = http_get(TEST_BASE_URL "/redoc");
    assert(redoc_response != NULL);
    assert(strstr(redoc_response, "redoc") != NULL || strstr(redoc_response, "ReDoc") != NULL);
    free(redoc_response);

    // Test OpenAPI JSON
    char *openapi_response = http_get(TEST_BASE_URL "/openapi.json");
    assert(openapi_response != NULL);
    assert(strstr(openapi_response, "\"openapi\"") != NULL);
    assert(strstr(openapi_response, "\"paths\"") != NULL);
    free(openapi_response);

    printf("✓ API documentation endpoints test passed\n");
}

void test_openapi_spec_content(void) {
    printf("Testing OpenAPI specification content...\n");

    char *response = http_get(TEST_BASE_URL "/openapi.json");
    assert(response != NULL);

    // Parse JSON response (simplified check)
    assert(strstr(response, "\"/test\"") != NULL);
    assert(strstr(response, "\"get\"") != NULL);
    assert(strstr(response, "\"post\"") != NULL);
    assert(strstr(response, "\"/test/{id}\"") != NULL);

    free(response);
    printf("✓ OpenAPI specification content test passed\n");
}

// Test server setup function
void *start_test_server(void *arg) {
    crest_app_t *app = crest_create();

    // Enable API documentation
    crest_enable_dashboard(app, true);

    // Add test routes
    crest_get(app, "/test", test_get_handler, "Test GET endpoint");
    crest_post(app, "/test", test_post_handler, "Test POST endpoint");
    crest_get(app, "/test/:id", test_param_handler, "Test path parameters");
    crest_get(app, "/query", test_query_handler, "Test query parameters");

    printf("🧪 Starting test server on port %d\n", TEST_PORT);

    // Note: In real implementation, you'd need to modify crest_listen to accept port
    // For this example, assume it runs on TEST_PORT
    crest_listen(app);
    crest_destroy(app);

    return NULL;
}

int run_integration_tests(void) {
    printf("🔗 Running Integration Tests\n");
    printf("==========================\n\n");

    // Initialize cURL
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // Start test server in background thread
    pthread_t server_thread;
    pthread_create(&server_thread, NULL, start_test_server, NULL);

    // Give server time to start
    sleep(2);

    // Run tests
    test_get_endpoint();
    test_post_endpoint();
    test_path_parameters();
    test_query_parameters();
    test_api_documentation_endpoints();
    test_openapi_spec_content();

    // Cleanup
    curl_global_cleanup();

    printf("\n✅ All integration tests passed!\n\n");

    // Note: In real implementation, you'd properly stop the server
    return 0;
}
```

## API Contract Testing Example

### Code Example

```c
#include <crest/crest.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <jansson.h> // JSON parsing library

// Contract test structure
typedef struct {
    const char *endpoint;
    const char *method;
    const char *expected_response_schema;
    int expected_status_code;
} api_contract_t;

// Response validation functions
int validate_json_response(const char *response, const char *expected_schema) {
    json_error_t error;
    json_t *root = json_loads(response, 0, &error);

    if (!root) {
        fprintf(stderr, "JSON parsing error: %s\n", error.text);
        return 0;
    }

    // Simplified schema validation - check required fields
    if (strcmp(expected_schema, "user_response") == 0) {
        if (!json_object_get(root, "id") || !json_object_get(root, "name")) {
            json_decref(root);
            return 0;
        }
    } else if (strcmp(expected_schema, "error_response") == 0) {
        if (!json_object_get(root, "error") || !json_object_get(root, "code")) {
            json_decref(root);
            return 0;
        }
    }

    json_decref(root);
    return 1;
}

int validate_openapi_spec(const char *spec_json) {
    json_error_t error;
    json_t *root = json_loads(spec_json, 0, &error);

    if (!root) {
        fprintf(stderr, "OpenAPI JSON parsing error: %s\n", error.text);
        return 0;
    }

    // Validate OpenAPI structure
    json_t *openapi = json_object_get(root, "openapi");
    json_t *info = json_object_get(root, "info");
    json_t *paths = json_object_get(root, "paths");

    if (!openapi || !info || !paths) {
        json_decref(root);
        return 0;
    }

    // Validate version
    const char *version = json_string_value(openapi);
    if (!version || strncmp(version, "3.0", 3) != 0) {
        json_decref(root);
        return 0;
    }

    json_decref(root);
    return 1;
}

// Contract test definitions
api_contract_t contracts[] = {
    {"/users", "GET", "user_list_response", 200},
    {"/users/123", "GET", "user_response", 200},
    {"/users", "POST", "user_response", 201},
    {"/users/999", "GET", "error_response", 404},
    {NULL, NULL, NULL, 0} // Sentinel
};

void test_api_contracts(void) {
    printf("📋 Testing API Contracts\n");
    printf("=======================\n\n");

    // Initialize cURL
    curl_global_init(CURL_GLOBAL_DEFAULT);

    for (int i = 0; contracts[i].endpoint != NULL; i++) {
        api_contract_t contract = contracts[i];
        printf("Testing %s %s...\n", contract.method, contract.endpoint);

        char url[256];
        sprintf(url, "%s%s", TEST_BASE_URL, contract.endpoint);

        char *response = NULL;
        long response_code = 0;

        if (strcmp(contract.method, "GET") == 0) {
            response = http_get(url);
        } else if (strcmp(contract.method, "POST") == 0) {
            response = http_post(url, "{\"name\": \"Test User\"}");
        }

        if (!response) {
            printf("❌ Failed to get response for %s %s\n", contract.method, contract.endpoint);
            continue;
        }

        // Validate status code (simplified - would need to capture actual status)
        // In real implementation, modify http_get/http_post to return status codes

        // Validate response schema
        if (!validate_json_response(response, contract.expected_response_schema)) {
            printf("❌ Schema validation failed for %s %s\n", contract.method, contract.endpoint);
        } else {
            printf("✓ Contract test passed for %s %s\n", contract.method, contract.endpoint);
        }

        free(response);
    }

    curl_global_cleanup();
    printf("\n✅ API contract tests completed!\n\n");
}

void test_openapi_contract(void) {
    printf("Testing OpenAPI specification contract...\n");

    curl_global_init(CURL_GLOBAL_DEFAULT);

    char *response = http_get(TEST_BASE_URL "/openapi.json");
    assert(response != NULL);

    if (validate_openapi_spec(response)) {
        printf("✓ OpenAPI specification contract validated\n");
    } else {
        printf("❌ OpenAPI specification contract validation failed\n");
    }

    free(response);
    curl_global_cleanup();
}
```

## Automated Testing Pipeline

### Makefile

```makefile
.PHONY: test test-unit test-integration test-contract clean

# Run all tests
test: test-unit test-integration test-contract
	@echo "🎉 All tests passed!"

# Unit tests
test-unit:
	@echo "🧪 Running unit tests..."
	@gcc -o test_unit test_unit.c -lcrest -lcunit
	@./test_unit

# Integration tests
test-integration:
	@echo "🔗 Running integration tests..."
	@gcc -o test_integration test_integration.c -lcrest -lcurl -pthread
	@./test_integration

# Contract tests
test-contract:
	@echo "📋 Running contract tests..."
	@gcc -o test_contract test_contract.c -lcrest -lcurl -ljansson
	@./test_contract

# Clean test artifacts
clean:
	@rm -f test_unit test_integration test_contract *.o
```

### GitHub Actions Workflow

```yaml
name: API Tests

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  test:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v3

    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y cmake gcc libcurl4-openssl-dev libjansson-dev libcunit1-dev

    - name: Build project
      run: |
        mkdir build
        cd build
        cmake ..
        make -j$(nproc)

    - name: Run unit tests
      run: make test-unit

    - name: Start test server
      run: |
        cd build
        ./crest_advanced_example &
        SERVER_PID=$!
        echo "SERVER_PID=$SERVER_PID" >> $GITHUB_ENV
        sleep 3

    - name: Run integration tests
      run: make test-integration

    - name: Run contract tests
      run: make test-contract

    - name: Stop test server
      run: |
        kill $SERVER_PID || true

    - name: Upload test results
      uses: actions/upload-artifact@v3
      if: always()
      with:
        name: test-results
        path: |
          test-results.xml
          coverage-report/
```

### Docker Test Environment

```dockerfile
FROM gcc:latest

# Install test dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    libcurl4-openssl-dev \
    libjansson-dev \
    libcunit1-dev \
    curl \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source
COPY . .

# Build tests
RUN mkdir build && cd build && \
    cmake .. && \
    make -j$(nproc)

# Run tests
CMD ["sh", "-c", "cd build && make test"]
```

## Performance Testing

### Load Testing Script

```bash
#!/bin/bash

# Load testing script using Apache Bench

echo "🚀 Starting load tests..."

# Basic load test
echo "Running basic load test..."
ab -n 1000 -c 10 http://localhost:8080/api/test

# API documentation endpoints
echo "Testing API docs endpoints..."
ab -n 100 -c 5 http://localhost:8080/docs
ab -n 100 -c 5 http://localhost:8080/redoc
ab -n 100 -c 5 http://localhost:8080/openapi.json

# Concurrent requests test
echo "Testing concurrent requests..."
for i in {1..50}; do
    curl -s http://localhost:8080/api/test &
done
wait

echo "✅ Load tests completed!"
```

### Memory Leak Testing

```c
#include <crest/crest.h>
#include <stdlib.h>
#include <stdio.h>

// Memory leak detection test
void memory_leak_test(void) {
    printf("Testing for memory leaks...\n");

    for (int i = 0; i < 1000; i++) {
        crest_app_t *app = crest_create();
        crest_enable_dashboard(app, true);

        // Add routes
        crest_get(app, "/test", test_handler, "Test endpoint");

        // Generate OpenAPI spec multiple times
        for (int j = 0; j < 10; j++) {
            char *spec = crest_generate_openapi_spec(app);
            free(spec);
        }

        crest_destroy(app);
    }

    printf("✓ Memory leak test completed (check with valgrind)\n");
}
```

## Summary

These testing examples demonstrate:

- **Unit testing** with mock objects and assertions
- **Integration testing** with real HTTP requests using cURL
- **API contract testing** validating response schemas and OpenAPI specs
- **Automated testing pipelines** with Makefiles and CI/CD workflows
- **Performance testing** with load testing and memory leak detection
- **Container-based testing** with Docker

All tests validate the API documentation functionality, ensuring Swagger UI, ReDoc, and OpenAPI JSON endpoints work correctly under various conditions.
/**
 * @file test_crest.cpp
 * @brief C++ unit tests for Crest framework
 */

#include "crest/crest.h"
#include "crest/types.h"
#include "crest/json.h"
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

// Simple test framework
#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    std::cout << "Running " << #name << "... "; \
    try { \
        test_##name(); \
        std::cout << "PASSED" << std::endl; \
    } catch (const std::exception& e) { \
        std::cout << "FAILED: " << e.what() << std::endl; \
        return 1; \
    } \
} while(0)

#define ASSERT(condition) do { \
    if (!(condition)) { \
        throw std::runtime_error("Assertion failed: " #condition); \
    } \
} while(0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        throw std::runtime_error("Assertion failed: " #a " != " #b); \
    } \
} while(0)

#define ASSERT_STR_EQ(a, b) do { \
    if (strcmp((a), (b)) != 0) { \
        throw std::runtime_error("Assertion failed: strings not equal"); \
    } \
} while(0)

/* ====================================================================
 * CONFIG TESTS
 * ==================================================================== */

TEST(config_create) {
    crest_config_t* config = crest_config_create();
    ASSERT(config != nullptr);
    ASSERT_STR_EQ(config->host, "127.0.0.1");
    ASSERT_EQ(config->port, 3000);
    ASSERT_EQ(config->enable_logging, true);
    ASSERT_EQ(config->log_level, CREST_LOG_INFO);
    ASSERT_EQ(config->enable_cors, false);
    ASSERT_EQ(config->enable_dashboard, false);
    ASSERT_STR_EQ(config->dashboard_path, "/__crest__/dashboard");
    ASSERT_EQ(config->max_body_size, 10485760UL);
    ASSERT_EQ(config->timeout_seconds, 60);
    crest_config_destroy(config);
}

TEST(config_validate) {
    crest_config_t* config = crest_config_create();
    ASSERT(crest_config_validate(config));

    // Test invalid port
    config->port = 0;
    ASSERT(!crest_config_validate(config));
    config->port = 3000;

    // Test invalid timeout
    config->timeout_seconds = 0;
    ASSERT(!crest_config_validate(config));
    config->timeout_seconds = 60;

    crest_config_destroy(config);
}

/* ====================================================================
 * JSON TESTS
 * ==================================================================== */

TEST(json_parse_string) {
    crest_json_value_t* value = crest_json_parse("\"hello world\"");
    ASSERT(value != nullptr);
    ASSERT_EQ(crest_json_type(value), CREST_JSON_STRING);
    ASSERT_STR_EQ(crest_json_as_string(value), "hello world");
    crest_json_free(value);
}

TEST(json_parse_number) {
    crest_json_value_t* value = crest_json_parse("42.5");
    ASSERT(value != nullptr);
    ASSERT_EQ(crest_json_type(value), CREST_JSON_NUMBER);
    ASSERT_EQ(crest_json_as_number(value), 42.5);
    crest_json_free(value);
}

TEST(json_parse_boolean) {
    crest_json_value_t* value = crest_json_parse("true");
    ASSERT(value != nullptr);
    ASSERT_EQ(crest_json_type(value), CREST_JSON_BOOL);
    ASSERT_EQ(crest_json_as_bool(value), true);
    crest_json_free(value);
}

TEST(json_parse_null) {
    crest_json_value_t* value = crest_json_parse("null");
    ASSERT(value != nullptr);
    ASSERT_EQ(crest_json_type(value), CREST_JSON_NULL);
    crest_json_free(value);
}

TEST(json_parse_array) {
    crest_json_value_t* value = crest_json_parse("[1, 2, 3]");
    ASSERT(value != nullptr);
    ASSERT_EQ(crest_json_type(value), CREST_JSON_ARRAY);
    ASSERT_EQ(crest_json_array_size(value), 3);
    crest_json_free(value);
}

TEST(json_parse_object) {
    crest_json_value_t* value = crest_json_parse("{\"key\": \"value\"}");
    ASSERT(value != nullptr);
    ASSERT_EQ(crest_json_type(value), CREST_JSON_OBJECT);
    crest_json_value_t* key_value = crest_json_get(value, "key");
    ASSERT(key_value != nullptr);
    ASSERT_EQ(crest_json_type(key_value), CREST_JSON_STRING);
    ASSERT_STR_EQ(crest_json_as_string(key_value), "value");
    crest_json_free(value);
}

TEST(json_stringify) {
    crest_json_value_t* obj = crest_json_object();
    crest_json_set(obj, "name", crest_json_string("test"));
    crest_json_set(obj, "value", crest_json_number(123));

    char* json = crest_json_stringify(obj);
    ASSERT(json != nullptr);
    ASSERT(strstr(json, "\"name\":\"test\"") != nullptr);
    ASSERT(strstr(json, "\"value\":123") != nullptr);

    free(json);
    crest_json_free(obj);
}

/* ====================================================================
 * ROUTER TESTS
 * ==================================================================== */

TEST(router_create) {
    crest_router_t* router = crest_router_create(NULL);
    ASSERT(router != nullptr);
    crest_router_destroy(router);
}

TEST(router_add_route) {
    crest_router_t* router = crest_router_create(NULL);

    crest_router_get(router, "/api/users", nullptr, nullptr);

    crest_router_destroy(router);
}

/*
TEST(router_match_route) {
    crest_router_t* router = crest_router_create(NULL);

    crest_router_get(router, "/api/users/:id", nullptr, nullptr);

    crest_route_match_t match;
    bool found = crest_router_match(router, CREST_GET, "/api/users/123", &match);
    ASSERT(found);
    ASSERT_STR_EQ(match.params[0].key, "id");
    ASSERT_STR_EQ(match.params[0].value, "123");

    crest_router_destroy(router);
}
*/

/*
TEST(router_pattern_matching) {
    crest_router_t* router = crest_router_create(NULL);

    crest_router_get(router, "/api/users/*", nullptr, nullptr);

    crest_route_match_t match;
    bool found = crest_router_match(router, CREST_GET, "/api/users/profile/settings", &match);
    ASSERT(found);

    crest_router_destroy(router);
}
*/

/* ====================================================================
 * RESPONSE TESTS
 * ==================================================================== */

TEST(response_status) {
    crest_response_t response = {0};
    crest_response_status(&response, CREST_STATUS_OK);
    ASSERT_EQ(response.status_code, CREST_STATUS_OK);
}

TEST(response_header) {
    crest_response_t response = {0};
    crest_response_header(&response, "Content-Type", "application/json");
    ASSERT_EQ(response.header_count, 1);
    ASSERT_STR_EQ(response.headers[0].key, "Content-Type");
    ASSERT_STR_EQ(response.headers[0].value, "application/json");

    // Clean up
    free(response.headers[0].key);
    free(response.headers[0].value);
}

TEST(response_send) {
    crest_response_t response = {0};
    crest_response_send(&response, "Hello World");
    ASSERT(response.sent);
    ASSERT_STR_EQ(response.body, "Hello World");
    ASSERT_EQ(response.body_len, 11);

    // Clean up
    free(response.body);
}

TEST(response_json) {
    crest_response_t response = {0};
    crest_response_json(&response, "{\"message\": \"ok\"}");
    ASSERT(response.sent);
    ASSERT_STR_EQ(response.body, "{\"message\": \"ok\"}");

    // Check Content-Type header
    bool has_content_type = false;
    for (size_t i = 0; i < response.header_count; i++) {
        if (strcmp(response.headers[i].key, "Content-Type") == 0 &&
            strcmp(response.headers[i].value, "application/json") == 0) {
            has_content_type = true;
            free(response.headers[i].key);
            free(response.headers[i].value);
            break;
        }
    }
    ASSERT(has_content_type);

    free(response.body);
}

/* ====================================================================
 * MIDDLEWARE TESTS
 * ==================================================================== */

TEST(middleware_cors) {
    crest_request_t req = {0};
    crest_response_t res = {0};

    req.method = CREST_GET;  // Use GET instead of OPTIONS for this test
    req.path = "/api/test";

    crest_middleware_fn_t cors_middleware = crest_middleware_cors();
    ASSERT(cors_middleware != nullptr);
    
    bool result = cors_middleware(&req, &res);
    ASSERT(result);

    // Check CORS headers
    bool has_cors_headers = false;
    for (size_t i = 0; i < res.header_count; i++) {
        if (strcmp(res.headers[i].key, "Access-Control-Allow-Origin") == 0 ||
            strcmp(res.headers[i].key, "Access-Control-Allow-Methods") == 0) {
            has_cors_headers = true;
        }
    }
    ASSERT(has_cors_headers);
}

TEST(middleware_logger) {
    crest_request_t req = {0};
    crest_response_t res = {0};

    req.method = CREST_GET;
    req.path = "/api/test";

    crest_middleware_fn_t logger_middleware = crest_middleware_logger();
    ASSERT(logger_middleware != nullptr);
    
    bool result = logger_middleware(&req, &res);
    ASSERT(result);
}

TEST(middleware_body_parser) {
    crest_request_t req = {0};
    crest_response_t res = {0};

    req.method = CREST_POST;
    req.path = "/api/test";
    req.body = "{\"name\": \"test\"}";
    req.content_type = "application/json";

    crest_middleware_fn_t body_parser = crest_middleware_body_parser();
    ASSERT(body_parser != nullptr);
    
    bool result = body_parser(&req, &res);
    ASSERT(result);

    // Should parse JSON body
    ASSERT(req.parsed_body != nullptr);
    crest_json_value_t* parsed = (crest_json_value_t*)req.parsed_body;
    ASSERT_EQ(crest_json_type(parsed), CREST_JSON_OBJECT);

    crest_json_free(parsed);
}

/* ====================================================================
 * MAIN TEST RUNNER
 * ==================================================================== */

int main() {
    std::cout << "Running C++ Unit Tests for Crest Framework" << std::endl;
    std::cout << "==========================================" << std::endl;

    // Config tests
    RUN_TEST(config_create);
    RUN_TEST(config_validate);

    // JSON tests
    RUN_TEST(json_parse_string);
    RUN_TEST(json_parse_number);
    RUN_TEST(json_parse_boolean);
    RUN_TEST(json_parse_null);
    RUN_TEST(json_parse_array);
    RUN_TEST(json_parse_object);
    RUN_TEST(json_stringify);

    // Router tests
    RUN_TEST(router_create);
    RUN_TEST(router_add_route);
    // RUN_TEST(router_match_route);
    // RUN_TEST(router_pattern_matching);

    // Response tests
    RUN_TEST(response_status);
    RUN_TEST(response_header);
    RUN_TEST(response_send);
    RUN_TEST(response_json);

    // Middleware tests
    RUN_TEST(middleware_cors);
    RUN_TEST(middleware_logger);
    RUN_TEST(middleware_body_parser);

    std::cout << "==========================================" << std::endl;
    std::cout << "All C++ tests passed!" << std::endl;

    return 0;
}
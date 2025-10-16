/**
 * @file test_integration.c
 * @brief Integration tests for Crest framework
 */

#include "crest/crest.h"
#include "crest/types.h"
#include "crest/json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifdef _WIN32
#include <windows.h>
#define sleep(seconds) Sleep((seconds) * 1000)
#else
#include <unistd.h>
#endif

/* ====================================================================
 * TEST HANDLERS
 * ==================================================================== */

void test_handler_get(crest_request_t *req, crest_response_t *res) {
    crest_response_json(res, "{\"message\": \"GET request successful\"}");
}

void test_handler_post(crest_request_t *req, crest_response_t *res) {
    if (req->parsed_body) {
        crest_json_value_t* parsed = (crest_json_value_t*)req->parsed_body;
        if (crest_json_type(parsed) == CREST_JSON_OBJECT) {
            crest_json_value_t *name = crest_json_get(parsed, "name");
            if (name && crest_json_type(name) == CREST_JSON_STRING) {
                char response[256];
                snprintf(response, sizeof(response), "{\"received\": \"%s\"}", crest_json_as_string(name));
                crest_response_json(res, response);
                return;
            }
        }
    }
    crest_response_status(res, CREST_STATUS_BAD_REQUEST);
    crest_response_json(res, "{\"error\": \"Invalid JSON or missing name field\"}");
}

void test_handler_params(crest_request_t *req, crest_response_t *res) {
    const char *id = crest_request_param(req, "id");
    const char *action = crest_request_param(req, "action");

    if (id && action) {
        char response[256];
        snprintf(response, sizeof(response), "{\"id\": \"%s\", \"action\": \"%s\"}", id, action);
        crest_response_json(res, response);
    } else {
        crest_response_status(res, CREST_STATUS_BAD_REQUEST);
        crest_response_json(res, "{\"error\": \"Missing parameters\"}");
    }
}

void test_handler_middleware(crest_request_t *req, crest_response_t *res) {
    // Check if middleware added custom header
    bool has_custom_header = false;
    for (size_t i = 0; i < req->header_count; i++) {
        if (strcmp(req->headers[i].key, "X-Test-Middleware") == 0) {
            has_custom_header = true;
            break;
        }
    }

    if (has_custom_header) {
        crest_response_json(res, "{\"middleware\": \"working\"}");
    } else {
        crest_response_status(res, CREST_STATUS_INTERNAL_SERVER_ERROR);
        crest_response_json(res, "{\"error\": \"Middleware not applied\"}");
    }
}

/* ====================================================================
 * TEST MIDDLEWARE
 * ==================================================================== */

bool test_middleware(crest_request_t *req, crest_response_t *res) {
    crest_response_header(res, "X-Test-Middleware", "applied");
    return true; // Continue processing
}

/* ====================================================================
 * INTEGRATION TEST SUITE
 * ==================================================================== */

typedef struct {
    const char *name;
    int (*test_func)(void);
} integration_test_t;

int test_basic_routing(void) {
    printf("Testing basic routing...\n");

    crest_app_t *app = crest_create();
    if (!app) {
        fprintf(stderr, "Failed to create app\n");
        return 1;
    }

    // Add routes
    crest_get(app, "/api/test", test_handler_get, NULL);
    crest_post(app, "/api/data", test_handler_post, NULL);

    // Test route registration
    if (app->route_count != 2) {
        fprintf(stderr, "Expected 2 routes, got %zu\n", app->route_count);
        crest_destroy(app);
        return 1;
    }

    crest_destroy(app);
    printf("Basic routing test passed\n");
    return 0;
}

int test_parameter_routing(void) {
    printf("Testing parameter routing...\n");

    crest_app_t *app = crest_create();
    if (!app) {
        fprintf(stderr, "Failed to create app\n");
        return 1;
    }

    // Add parameterized route
    crest_get(app, "/api/users/:id/:action", test_handler_params, NULL);

    // Test parameter extraction - simplified test
    if (app->route_count != 1) {
        fprintf(stderr, "Expected 1 route, got %zu\n", app->route_count);
        crest_destroy(app);
        return 1;
    }

    crest_destroy(app);
    printf("Parameter routing test passed\n");
    return 0;
}

int test_middleware_chain(void) {
    printf("Testing middleware chain...\n");

    crest_app_t *app = crest_create();
    if (!app) {
        fprintf(stderr, "Failed to create app\n");
        return 1;
    }

    // Add middleware
    crest_use(app, test_middleware);
    crest_use(app, crest_middleware_cors());
    crest_use(app, crest_middleware_body_parser());

    // Add route
    crest_get(app, "/api/middleware", test_handler_middleware, NULL);

    // Test middleware count
    if (app->middleware_count != 3) {
        fprintf(stderr, "Expected 3 middleware, got %zu\n", app->middleware_count);
        crest_destroy(app);
        return 1;
    }

    crest_destroy(app);
    printf("Middleware chain test passed\n");
    return 0;
}

int test_json_parsing_integration(void) {
    printf("Testing JSON parsing integration...\n");

    // Test JSON parsing in request context
    crest_request_t req = {0};
    req.method = CREST_POST;
    req.path = "/api/test";
    req.body = "{\"name\": \"integration test\", \"value\": 42}";
    req.content_type = "application/json";

    // Apply body parser middleware
    crest_response_t res = {0};
    crest_middleware_fn_t body_parser = crest_middleware_body_parser();
    bool result = body_parser(&req, &res);
    if (!result) {
        fprintf(stderr, "Body parser middleware failed\n");
        return 1;
    }

    if (!req.parsed_body) {
        fprintf(stderr, "Body parser failed to parse JSON\n");
        return 1;
    }

    crest_json_value_t* parsed = (crest_json_value_t*)req.parsed_body;
    if (crest_json_type(parsed) != CREST_JSON_OBJECT) {
        fprintf(stderr, "Parsed body is not an object\n");
        crest_json_free(parsed);
        return 1;
    }

    crest_json_value_t *name = crest_json_get(parsed, "name");
    crest_json_value_t *value = crest_json_get(parsed, "value");

    if (!name || crest_json_type(name) != CREST_JSON_STRING || strcmp(crest_json_as_string(name), "integration test") != 0) {
        fprintf(stderr, "Name field parsing failed\n");
        crest_json_free(parsed);
        return 1;
    }

    if (!value || crest_json_type(value) != CREST_JSON_NUMBER || crest_json_as_number(value) != 42) {
        fprintf(stderr, "Value field parsing failed\n");
        crest_json_free(parsed);
        return 1;
    }

    crest_json_free(parsed);
    printf("JSON parsing integration test passed\n");
    return 0;
}

int test_config_loading(void) {
    printf("Testing configuration loading...\n");

    // Create a temporary config file
    const char *config_content = "{\n"
        "  \"server\": {\n"
        "    \"host\": \"127.0.0.1\",\n"
        "    \"port\": 8080,\n"
        "    \"timeout\": 120\n"
        "  },\n"
        "  \"middleware\": {\n"
        "    \"cors\": true,\n"
        "    \"logging\": true,\n"
        "    \"dashboard\": true\n"
        "  }\n"
        "}\n";

    FILE *f = fopen("test_config.json", "w");
    if (!f) {
        fprintf(stderr, "Failed to create test config file\n");
        return 1;
    }
    fputs(config_content, f);
    fclose(f);

    // Load configuration
    crest_config_t *config = crest_config_load("test_config.json");
    if (!config) {
        fprintf(stderr, "Failed to load configuration\n");
        remove("test_config.json");
        return 1;
    }

    // Verify configuration
    if (strcmp(config->host, "127.0.0.1") != 0) {
        fprintf(stderr, "Host not loaded correctly\n");
        crest_config_destroy(config);
        remove("test_config.json");
        return 1;
    }

    if (config->port != 8080) {
        fprintf(stderr, "Port not loaded correctly\n");
        crest_config_destroy(config);
        remove("test_config.json");
        return 1;
    }

    if (config->timeout_seconds != 120) {
        fprintf(stderr, "Timeout not loaded correctly\n");
        crest_config_destroy(config);
        remove("test_config.json");
        return 1;
    }

    if (!config->enable_cors || !config->enable_logging || !config->enable_dashboard) {
        fprintf(stderr, "Middleware settings not loaded correctly\n");
        crest_config_destroy(config);
        remove("test_config.json");
        return 1;
    }

    crest_config_destroy(config);
    remove("test_config.json");

    printf("Configuration loading test passed\n");
    return 0;
}

/* ====================================================================
 * MAIN TEST RUNNER
 * ==================================================================== */

int main() {
    printf("Running Crest Integration Tests\n");
    printf("===============================\n");

    integration_test_t tests[] = {
        {"Basic Routing", test_basic_routing},
        {"Parameter Routing", test_parameter_routing},
        {"Middleware Chain", test_middleware_chain},
        {"JSON Parsing Integration", test_json_parsing_integration},
        {"Configuration Loading", test_config_loading},
        {NULL, NULL}
    };

    int passed = 0;
    int total = 0;

    for (int i = 0; tests[i].name; i++) {
        printf("\n[%d/%d] ", i + 1, (int)(sizeof(tests) / sizeof(tests[0]) - 1));
        int result = tests[i].test_func();
        if (result == 0) {
            passed++;
        }
        total++;
    }

    printf("\n===============================\n");
    printf("Integration Tests Results: %d/%d passed\n", passed, total);

    if (passed == total) {
        printf("All integration tests passed!\n");
        return 0;
    } else {
        printf("Some integration tests failed!\n");
        return 1;
    }
}
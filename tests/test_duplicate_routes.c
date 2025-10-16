/**
 * @file test_duplicate_routes.c
 * @brief Test program for duplicate route detection
 */

#include "crest/crest.h"
#include <stdio.h>
#include <stdlib.h>

void test_handler(crest_request_t *req, crest_response_t *res) {
    crest_response_json(res, "{\"message\": \"test\"}");
}

int main() {
    printf("Testing duplicate route detection...\n");

    crest_app_t *app = crest_create();
    if (!app) {
        fprintf(stderr, "Failed to create app\n");
        return 1;
    }

    crest_enable_logging(app, true);

    // Register initial routes
    printf("\n--- Registering initial routes ---\n");
    crest_get(app, "/users", test_handler, "Get all users");
    crest_post(app, "/users", test_handler, "Create user");
    crest_get(app, "/users/:id", test_handler, "Get user by ID");

    printf("\n--- Attempting to register duplicate routes ---\n");
    // Try to register duplicate routes
    crest_get(app, "/users", test_handler, "Duplicate GET /users");
    crest_post(app, "/users", test_handler, "Duplicate POST /users");

    printf("\n--- Test completed ---\n");
    printf("If you see warning messages above about duplicate routes, the feature is working!\n");

    crest_destroy(app);
    printf("\nTest completed successfully!\n");
    return 0;
}
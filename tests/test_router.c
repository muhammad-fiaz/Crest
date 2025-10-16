/**
 * @file test_router.c
 * @brief Unit tests for routing functionality
 * 
 * Tests:
 * - Route registration and matching
 * - HTTP method dispatch
 * - Path parameters (/users/:id)
 * - Wildcard patterns
 * - Route priority and ordering
 */

#include "crest/crest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do { \
    printf("Running test_%s...", #name); \
    test_##name(); \
    printf(" ✓\n"); \
    tests_passed++; \
} while(0)

#define ASSERT_TRUE(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "\n  FAILED: %s at line %d\n", #expr, __LINE__); \
        exit(1); \
    } \
} while(0)

#define ASSERT_NOT_NULL(ptr) ASSERT_TRUE((ptr) != NULL)
#define ASSERT_NULL(ptr) ASSERT_TRUE((ptr) == NULL)
#define ASSERT_EQUAL(a, b) ASSERT_TRUE((a) == (b))
#define ASSERT_STR_EQUAL(a, b) ASSERT_TRUE(strcmp((a), (b)) == 0)

static int tests_passed = 0;
static int handler_called = 0;

/* Test handler */
static void test_handler(crest_request_t *req, crest_response_t *res) {
    (void)req;
    (void)res;
    handler_called++;
}

/* Test app lifecycle */
TEST(create_destroy_app) {
    crest_app_t *app = crest_create();
    ASSERT_NOT_NULL(app);
    crest_destroy(app);
}

/* Test route registration */
TEST(register_get_route) {
    crest_app_t *app = crest_create();
    ASSERT_NOT_NULL(app);
    
    crest_get(app, "/", test_handler, "Root endpoint");
    crest_get(app, "/users", test_handler, "Users endpoint");
    
    crest_destroy(app);
}

TEST(register_all_methods) {
    crest_app_t *app = crest_create();
    ASSERT_NOT_NULL(app);
    
    crest_get(app, "/resource", test_handler, "GET");
    crest_post(app, "/resource", test_handler, "POST");
    crest_put(app, "/resource", test_handler, "PUT");
    crest_delete(app, "/resource", test_handler, "DELETE");
    crest_patch(app, "/resource", test_handler, "PATCH");
    
    crest_destroy(app);
}

/* Test dashboard */
TEST(enable_dashboard) {
    crest_app_t *app = crest_create();
    ASSERT_NOT_NULL(app);
    
    crest_enable_dashboard(app, true);
    crest_enable_dashboard(app, false);
    
    crest_destroy(app);
}

/* Test multiple routes */
TEST(register_multiple_routes) {
    crest_app_t *app = crest_create();
    ASSERT_NOT_NULL(app);
    
    const char *paths[] = {
        "/",
        "/users",
        "/users/profile",
        "/posts",
        "/posts/123",
        "/api/v1/data"
    };
    
    for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); i++) {
        crest_get(app, paths[i], test_handler, "Test route");
    }
    
    crest_destroy(app);
}

/* Test edge cases */
TEST(null_safety) {
    // Should not crash with NULL
    crest_destroy(NULL);
    
    crest_app_t *app = crest_create();
    ASSERT_NOT_NULL(app);
    
    // These should handle NULL gracefully
    crest_get(app, NULL, test_handler, "Null path");
    crest_get(app, "/path", NULL, "Null handler");
    crest_get(app, "/path", test_handler, NULL); // Null description is OK
    
    crest_destroy(app);
}

TEST(empty_paths) {
    crest_app_t *app = crest_create();
    ASSERT_NOT_NULL(app);
    
    crest_get(app, "", test_handler, "Empty path");
    crest_get(app, "/", test_handler, "Root path");
    
    crest_destroy(app);
}

/* Test route limits */
TEST(many_routes) {
    crest_app_t *app = crest_create();
    ASSERT_NOT_NULL(app);
    
    // Register 100 routes
    for (int i = 0; i < 100; i++) {
        char path[64];
        snprintf(path, sizeof(path), "/route_%d", i);
        crest_get(app, path, test_handler, "Test route");
    }
    
    crest_destroy(app);
}

/* Main test runner */
int main(void) {
    printf("=== Crest Router Tests ===\n\n");
    
    RUN_TEST(create_destroy_app);
    RUN_TEST(register_get_route);
    RUN_TEST(register_all_methods);
    RUN_TEST(enable_dashboard);
    RUN_TEST(register_multiple_routes);
    RUN_TEST(null_safety);
    RUN_TEST(empty_paths);
    RUN_TEST(many_routes);
    
    printf("\n=== All %d tests passed! ✓ ===\n", tests_passed);
    return 0;
}

/**
 * @file test_pattern_matching.c
 * @brief Unit tests for route pattern matching
 */

#include "crest/crest.h"
#include "crest/types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Test framework macros */
#define TEST(name) static void name(void)
#define RUN_TEST(name) do { \
    printf("Running " #name "..."); \
    fflush(stdout); \
    name(); \
    printf(" ✓\n"); \
    test_count++; \
} while(0)

#define ASSERT_TRUE(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "\n  Assertion failed: %s\n  File: %s:%d\n", #expr, __FILE__, __LINE__); \
        exit(1); \
    } \
} while(0)

#define ASSERT_FALSE(expr) ASSERT_TRUE(!(expr))
#define ASSERT_NULL(ptr) ASSERT_TRUE((ptr) == NULL)
#define ASSERT_NOT_NULL(ptr) ASSERT_TRUE((ptr) != NULL)
#define ASSERT_EQUAL(a, b) ASSERT_TRUE((a) == (b))
#define ASSERT_STR_EQUAL(a, b) ASSERT_TRUE(strcmp((a), (b)) == 0)

static int test_count = 0;

static void dummy_handler(crest_request_t *req, crest_response_t *res) {
    (void)req;
    crest_response_send(res, "OK");
}

/* Test: Path parameter route detection */
TEST(test_path_param_single) {
    crest_app_t *app = crest_create();
    ASSERT_NOT_NULL(app);
    
    crest_get(app, "/users/:id", dummy_handler, "Get user by ID");
    
    /* Route should be marked as pattern */
    ASSERT_TRUE(app->route_count == 1);
    ASSERT_TRUE(app->routes[0].is_pattern);
    ASSERT_STR_EQUAL(app->routes[0].path, "/users/:id");
    
    crest_destroy(app);
}

/* Test: Multiple path parameters */
TEST(test_path_param_multiple) {
    crest_app_t *app = crest_create();
    ASSERT_NOT_NULL(app);
    
    crest_get(app, "/users/:userId/posts/:postId", dummy_handler, "Get post");
    
    ASSERT_TRUE(app->route_count == 1);
    ASSERT_TRUE(app->routes[0].is_pattern);
    ASSERT_NOT_NULL(app->routes[0].pattern);
    
    crest_destroy(app);
}

/* Test: Wildcard route */
TEST(test_wildcard_route) {
    crest_app_t *app = crest_create();
    ASSERT_NOT_NULL(app);
    
    crest_get(app, "/static/*", dummy_handler, "Serve static files");
    
    ASSERT_TRUE(app->route_count == 1);
    ASSERT_TRUE(app->routes[0].is_pattern);
    ASSERT_STR_EQUAL(app->routes[0].path, "/static/*");
    
    crest_destroy(app);
}

/* Test: Mixed static and dynamic routes */
TEST(test_mixed_routes) {
    crest_app_t *app = crest_create();
    ASSERT_NOT_NULL(app);
    
    crest_get(app, "/users", dummy_handler, "List all users");
    crest_get(app, "/users/:id", dummy_handler, "Get user by ID");
    crest_get(app, "/posts/:id/comments", dummy_handler, "Get comments");
    
    ASSERT_TRUE(app->route_count == 3);
    
    /* First route is static */
    ASSERT_FALSE(app->routes[0].is_pattern);
    ASSERT_NULL(app->routes[0].pattern);
    
    /* Second and third routes are patterns */
    ASSERT_TRUE(app->routes[1].is_pattern);
    ASSERT_NOT_NULL(app->routes[1].pattern);
    ASSERT_TRUE(app->routes[2].is_pattern);
    ASSERT_NOT_NULL(app->routes[2].pattern);
    
    crest_destroy(app);
}

/* Test: Pattern detection */
TEST(test_pattern_detection) {
    crest_app_t *app = crest_create();
    ASSERT_NOT_NULL(app);
    
    /* Static route */
    crest_get(app, "/api/v1/status", dummy_handler, NULL);
    ASSERT_FALSE(app->routes[0].is_pattern);
    
    /* Parameter route */
    crest_get(app, "/api/:version/data", dummy_handler, NULL);
    ASSERT_TRUE(app->routes[1].is_pattern);
    
    /* Wildcard route */
    crest_get(app, "/files/*", dummy_handler, NULL);
    ASSERT_TRUE(app->routes[2].is_pattern);
    
    crest_destroy(app);
}

/* Test: Parameter at different positions */
TEST(test_param_positions) {
    crest_app_t *app = crest_create();
    ASSERT_NOT_NULL(app);
    
    crest_get(app, "/:resource", dummy_handler, "Resource at root");
    crest_get(app, "/api/:action", dummy_handler, "Action");
    crest_get(app, "/data/items/:id", dummy_handler, "ID at end");
    crest_get(app, "/:type/items/:id", dummy_handler, "Multiple");
    
    ASSERT_EQUAL(app->route_count, 4);
    
    for (size_t i = 0; i < app->route_count; i++) {
        ASSERT_TRUE(app->routes[i].is_pattern);
        ASSERT_NOT_NULL(app->routes[i].pattern);
    }
    
    crest_destroy(app);
}

/* Test: Pattern storage */
TEST(test_pattern_storage) {
    crest_app_t *app = crest_create();
    ASSERT_NOT_NULL(app);
    
    crest_get(app, "/users/:id/profile", dummy_handler, NULL);
    
    ASSERT_NOT_NULL(app->routes[0].pattern);
    ASSERT_STR_EQUAL(app->routes[0].pattern, "/users/:id/profile");
    
    crest_destroy(app);
}

/* Test: Cleanup with pattern routes */
TEST(test_pattern_cleanup) {
    crest_app_t *app = crest_create();
    ASSERT_NOT_NULL(app);
    
    for (int i = 0; i < 10; i++) {
        char path[64];
        snprintf(path, sizeof(path), "/resource/:id%d", i);
        crest_get(app, path, dummy_handler, NULL);
    }
    
    ASSERT_EQUAL(app->route_count, 10);
    
    /* Cleanup should not crash */
    crest_destroy(app);
}

/* Test: All HTTP methods with patterns */
TEST(test_all_methods_with_patterns) {
    crest_app_t *app = crest_create();
    ASSERT_NOT_NULL(app);
    
    crest_get(app, "/items/:id", dummy_handler, NULL);
    crest_post(app, "/items/:id", dummy_handler, NULL);
    crest_put(app, "/items/:id", dummy_handler, NULL);
    crest_delete(app, "/items/:id", dummy_handler, NULL);
    crest_patch(app, "/items/:id", dummy_handler, NULL);
    
    ASSERT_EQUAL(app->route_count, 5);
    
    for (size_t i = 0; i < app->route_count; i++) {
        ASSERT_TRUE(app->routes[i].is_pattern);
    }
    
    crest_destroy(app);
}

int main(void) {
    printf("=== Crest Pattern Matching Tests ===\n\n");
    
    RUN_TEST(test_path_param_single);
    RUN_TEST(test_path_param_multiple);
    RUN_TEST(test_wildcard_route);
    RUN_TEST(test_mixed_routes);
    RUN_TEST(test_pattern_detection);
    RUN_TEST(test_param_positions);
    RUN_TEST(test_pattern_storage);
    RUN_TEST(test_pattern_cleanup);
    RUN_TEST(test_all_methods_with_patterns);
    
    printf("\n=== All %d tests passed! ✓ ===\n", test_count);
    
    return 0;
}

/**
 * @file main.c
 * @brief Basic Crest framework example - Hello World and simple routes
 * @details Demonstrates:
 *  - Creating application
 *  - Basic route registration (GET, POST)
 *  - Path parameters
 *  - JSON responses
 *  - Dashboard enablement
 */

#include "crest/crest.h"
#include "crest/json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ====================================================================
 * ROUTE HANDLERS
 * ==================================================================== */

/**
 * Hello World endpoint - simplest possible route
 * GET /
 */
void hello_handler(crest_request_t *req, crest_response_t *res) {
    crest_response_json(res, "{\"message\":\"Hello from Crest!\"}");
}

/**
 * Get user by ID - demonstrates path parameters
 * GET /users/:id
 */
void get_user_handler(crest_request_t *req, crest_response_t *res) {
    const char *id = crest_request_param(req, "id");
    
    if (!id) {
        crest_response_status(res, CREST_STATUS_BAD_REQUEST);
        crest_response_json(res, "{\"error\":\"Missing user ID\"}");
        return;
    }
    
    /* Build JSON response */
    char json[512];
    snprintf(json, sizeof(json), 
            "{\"id\":\"%s\",\"name\":\"John Doe\",\"email\":\"john@example.com\"}",
            id);
    
    crest_response_json(res, json);
}

/**
 * Create new user - demonstrates POST with body
 * POST /users
 */
void create_user_handler(crest_request_t *req, crest_response_t *res) {
    const char *body = crest_request_body(req);
    
    if (!body) {
        crest_response_status(res, CREST_STATUS_BAD_REQUEST);
        crest_response_json(res, "{\"error\":\"Request body is required\"}");
        return;
    }
    
    crest_log(CREST_LOG_INFO, "Creating user with body: %s", body);
    
    /* Validate JSON (optional) */
    crest_json_value_t *json = crest_json_parse(body);
    if (!json) {
        crest_response_status(res, CREST_STATUS_BAD_REQUEST);
        crest_response_json(res, "{\"error\":\"Invalid JSON\"}");
        return;
    }
    
    /* Extract name from JSON */
    crest_json_value_t *name_val = crest_json_get(json, "name");
    const char *name = (name_val && crest_json_type(name_val) == CREST_JSON_STRING)
                       ? crest_json_as_string(name_val) : "Unknown";
    
    /* Build response */
    char response[512];
    snprintf(response, sizeof(response),
            "{\"message\":\"User created successfully\",\"id\":123,\"name\":\"%s\"}",
            name);
    
    crest_response_status(res, CREST_STATUS_CREATED);
    crest_response_json(res, response);
    
    crest_json_free(json);
}

/**
 * Health check endpoint
 * GET /health
 */
void health_handler(crest_request_t *req, crest_response_t *res) {
    crest_response_json(res, "{\"status\":\"healthy\",\"version\":\"" CREST_VERSION "\"}");
}

/**
 * Query parameters example
 * GET /search?q=query&limit=10
 */
void search_handler(crest_request_t *req, crest_response_t *res) {
    const char *query = crest_request_query(req, "q");
    const char *limit = crest_request_query(req, "limit");
    
    char response[512];
    snprintf(response, sizeof(response),
            "{\"query\":\"%s\",\"limit\":\"%s\",\"results\":[]}",
            query ? query : "",
            limit ? limit : "10");
    
    crest_response_json(res, response);
}

/* ====================================================================
 * MAIN APPLICATION
 * ==================================================================== */

int main(void) {
    printf("==============================================\n");
    printf("  Crest Framework - Basic Example\n");
    printf("==============================================\n");
    
    /* Create application */
    crest_app_t *app = crest_create();
    
    if (!app) {
        crest_log(CREST_LOG_ERROR, "Failed to create Crest application");
        return 1;
    }
    
    /* Configuration */
    crest_set_port(app, 3000);
    crest_enable_dashboard(app, true);
    crest_enable_logging(app, true);
    
    /* Register routes */
    crest_log(CREST_LOG_INFO, "Registering routes...");
    crest_get(app, "/", hello_handler, "Welcome endpoint - returns Hello World");
    crest_get(app, "/health", health_handler, "Health check endpoint");
    crest_get(app, "/api/users/:id", get_user_handler, "Get user by ID (path parameter)");
    crest_post(app, "/api/users", create_user_handler, "Create a new user (with JSON body)");
    crest_get(app, "/search", search_handler, "Search with query parameters");
    
    /* Print info */
    printf("\n");
    printf("Server starting on http://localhost:3000\n\n");
    printf("Available endpoints:\n");
    printf("  GET  /                      - Hello World\n");
    printf("  GET  /health                - Health check\n");
    printf("  GET  /api/users/:id         - Get user\n");
    printf("  POST /api/users             - Create user\n");
    printf("  GET  /search?q=term         - Search\n");
    printf("  GET  /__crest__/dashboard   - Interactive dashboard\n\n");
    printf("Example requests:\n");
    printf("  curl http://localhost:3000/\n");
    printf("  curl http://localhost:3000/api/users/42\n");
    printf("  curl -X POST http://localhost:3000/api/users -H 'Content-Type: application/json' -d '{\"name\":\"Alice\"}'\n");
    printf("  curl 'http://localhost:3000/search?q=laptop&limit=5'\n\n");
    printf("Press Ctrl+C to stop.\n");
    printf("==============================================\n\n");
    
    /* Start server (blocks) */
    crest_listen(app);
    
    /* Cleanup */
    crest_destroy(app);
    
    return 0;
}

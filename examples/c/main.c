/**
 * @file main.c
 * @brief C example using Crest framework
 */

#include "crest/crest.h"
#include <stdio.h>
#include <string.h>

void handle_root(crest_request_t* req, crest_response_t* res) {
    crest_response_json(res, CREST_STATUS_OK, 
        "{\"message\":\"Welcome to Crest API\",\"version\":\"1.0.0\"}");
}

void handle_hello(crest_request_t* req, crest_response_t* res) {
    crest_response_json(res, CREST_STATUS_OK, 
        "{\"message\":\"Hello from Crest!\"}");
}

void handle_create_user(crest_request_t* req, crest_response_t* res) {
    const char* body = crest_request_get_body(req);
    char response[512];
    snprintf(response, sizeof(response), 
        "{\"message\":\"User created\",\"data\":%s}", 
        body ? body : "{}");
    crest_response_json(res, CREST_STATUS_CREATED, response);
}

void handle_get_user(crest_request_t* req, crest_response_t* res) {
    crest_response_json(res, CREST_STATUS_OK, 
        "{\"id\":1,\"name\":\"John Doe\",\"email\":\"john@example.com\"}");
}

void handle_update_user(crest_request_t* req, crest_response_t* res) {
    crest_response_json(res, CREST_STATUS_OK, 
        "{\"message\":\"User updated\"}");
}

void handle_delete_user(crest_request_t* req, crest_response_t* res) {
    crest_response_json(res, CREST_STATUS_OK, 
        "{\"message\":\"User deleted\"}");
}

int main(void) {
    // Create app with configuration
    crest_config_t config = {
        .title = "My C API",
        .description = "Example RESTful API built with Crest",
        .version = "1.0.0",
        .docs_enabled = true
    };
    
    crest_app_t* app = crest_create_with_config(&config);
    if (!app) {
        fprintf(stderr, "Failed to create app\n");
        return 1;
    }
    
    // Register routes
    crest_route(app, CREST_GET, "/", handle_root, "Root endpoint");
    crest_route(app, CREST_GET, "/hello", handle_hello, "Hello endpoint");
    crest_route(app, CREST_POST, "/users", handle_create_user, "Create a new user");
    crest_route(app, CREST_GET, "/users/:id", handle_get_user, "Get user by ID");
    crest_route(app, CREST_PUT, "/users/:id", handle_update_user, "Update user by ID");
    crest_route(app, CREST_DELETE, "/users/:id", handle_delete_user, "Delete user by ID");
    
    // Set schemas to match actual responses
    crest_set_response_schema(app, CREST_GET, "/", "{\"message\": \"string\", \"version\": \"string\"}");
    crest_set_response_schema(app, CREST_GET, "/hello", "{\"message\": \"string\"}");
    crest_set_response_schema(app, CREST_GET, "/users/:id", "{\"id\": \"number\", \"name\": \"string\", \"email\": \"string\"}");
    crest_set_response_schema(app, CREST_PUT, "/users/:id", "{\"message\": \"string\"}");
    crest_set_response_schema(app, CREST_DELETE, "/users/:id", "{\"message\": \"string\"}");
    
    // Start server
    printf("Starting C Crest server...\n");
    int result = crest_run(app, "127.0.0.1", 8000);
    
    // Cleanup
    crest_destroy(app);
    
    return result == 0 ? 0 : 1;
}

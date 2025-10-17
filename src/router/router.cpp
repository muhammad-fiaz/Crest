/**
 * @file router.cpp
 * @brief Route registration and management
 */

#include "crest/crest.h"
#include "crest/internal/app_internal.h"
#include <cstring>
#include <cstdlib>
#include <mutex>

#ifdef _MSC_VER
#define strdup _strdup
#endif

extern "C" {

int crest_route(crest_app_t* app, crest_method_t method, const char* path,
                crest_handler_t handler, const char* description) {
    if (!app || !path || !handler) return -1;
    
    std::lock_guard<std::mutex> lock(*static_cast<std::mutex*>(app->route_mutex));
    
    // Check for duplicate routes
    for (size_t i = 0; i < app->route_count; i++) {
        if (app->routes[i].method == method && strcmp(app->routes[i].path, path) == 0) {
            return -1; // Duplicate route
        }
    }
    
    // Expand capacity if needed
    if (app->route_count >= app->route_capacity) {
        size_t new_capacity = app->route_capacity == 0 ? 16 : app->route_capacity * 2;
        crest_route_entry_t* new_routes = (crest_route_entry_t*)realloc(
            app->routes, new_capacity * sizeof(crest_route_entry_t));
        if (!new_routes) return -1;
        app->routes = new_routes;
        app->route_capacity = new_capacity;
    }
    
    // Add route
    crest_route_entry_t* entry = &app->routes[app->route_count];
    entry->method = method;
    entry->path = strdup(path);
    entry->handler = handler;
    entry->description = description ? strdup(description) : strdup("");
    entry->cpp_handler = nullptr;
    entry->request_schema = nullptr;
    entry->response_schema = nullptr;
    
    app->route_count++;
    return 0;
}

void crest_set_request_schema(crest_app_t* app, crest_method_t method, const char* path, const char* schema) {
    if (!app || !path || !schema) return;
    
    std::lock_guard<std::mutex> lock(*static_cast<std::mutex*>(app->route_mutex));
    
    for (size_t i = 0; i < app->route_count; i++) {
        if (app->routes[i].method == method && strcmp(app->routes[i].path, path) == 0) {
            free(app->routes[i].request_schema);
            app->routes[i].request_schema = strdup(schema);
            return;
        }
    }
}

void crest_set_response_schema(crest_app_t* app, crest_method_t method, const char* path, const char* schema) {
    if (!app || !path || !schema) return;
    
    std::lock_guard<std::mutex> lock(*static_cast<std::mutex*>(app->route_mutex));
    
    for (size_t i = 0; i < app->route_count; i++) {
        if (app->routes[i].method == method && strcmp(app->routes[i].path, path) == 0) {
            free(app->routes[i].response_schema);
            app->routes[i].response_schema = strdup(schema);
            return;
        }
    }
}

} // extern "C"

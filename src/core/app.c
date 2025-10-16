/**
 * @file app.c
 * @brief Core application implementation for Crest
 */

#include "crest/crest.h"
#include "crest/types.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

crest_app_t* crest_create(void) {
    crest_config_t *config = crest_config_create();
    return crest_create_with_config(config);
}

crest_app_t* crest_create_with_config(crest_config_t *config) {
    crest_app_t *app = (crest_app_t*)calloc(1, sizeof(crest_app_t));
    if (!app) {
        return NULL;
    }
    
    app->config = config ? config : crest_config_create();
    app->route_count = 0;
    app->middleware_count = 0;
    app->running = false;
    app->socket_fd = -1;
    app->user_data = NULL;
    
    return app;
}

static void register_route(crest_app_t *app, int method, const char *path, 
                          crest_handler_t handler, const char *description) {
    if (!app || !path || !handler) {
        return;
    }
    
    if (app->route_count >= CREST_MAX_ROUTES) {
        crest_log(CREST_LOG_ERROR, "Maximum routes (%d) exceeded", CREST_MAX_ROUTES);
        return;
    }
    
    /* Check for duplicate routes */
    for (size_t i = 0; i < app->route_count; i++) {
        if (app->routes[i].method == method && strcmp(app->routes[i].path, path) == 0) {
            const char *method_str[] = {"GET", "POST", "PUT", "DELETE", "PATCH", "HEAD", "OPTIONS"};
            crest_log(CREST_LOG_WARN, "Duplicate route detected - %s %s already exists", 
                    method_str[method], path);
            crest_log(CREST_LOG_WARN, "Previous registration will be overwritten");
            /* Overwrite the existing route instead of adding a new one */
            crest_route_t *route = &app->routes[i];
            free(route->path);
            free(route->description);
            free(route->pattern);
            
            route->method = method;
            route->path = strdup(path);
            route->handler = handler;
            route->description = description ? strdup(description) : NULL;
            route->is_pattern = (strchr(path, ':') != NULL || strchr(path, '*') != NULL);
            
            if (route->is_pattern) {
                route->pattern = strdup(path);
            } else {
                route->pattern = NULL;
            }
            
            if (app->config->enable_logging) {
                crest_log(CREST_LOG_INFO, "Re-registered %s %s%s", method_str[method], path,
                       route->is_pattern ? " (pattern)" : "");
            }
            return;
        }
    }
    
    crest_route_t *route = &app->routes[app->route_count];
    route->method = method;
    route->path = strdup(path);
    route->handler = handler;
    route->description = description ? strdup(description) : NULL;
    route->is_pattern = (strchr(path, ':') != NULL || strchr(path, '*') != NULL);
    
    if (route->is_pattern) {
        route->pattern = strdup(path);
    } else {
        route->pattern = NULL;
    }
    
    app->route_count++;
    
    if (app->config->enable_logging) {
        const char *method_str[] = {"GET", "POST", "PUT", "DELETE", "PATCH", "HEAD", "OPTIONS"};
        crest_log(CREST_LOG_INFO, "Registered %s %s%s", method_str[method], path,
               route->is_pattern ? " (pattern)" : "");
    }
}

void crest_get(crest_app_t *app, const char *path, crest_handler_t handler, const char *description) {
    register_route(app, CREST_GET, path, handler, description);
}

void crest_post(crest_app_t *app, const char *path, crest_handler_t handler, const char *description) {
    register_route(app, CREST_POST, path, handler, description);
}

void crest_put(crest_app_t *app, const char *path, crest_handler_t handler, const char *description) {
    register_route(app, CREST_PUT, path, handler, description);
}

void crest_delete(crest_app_t *app, const char *path, crest_handler_t handler, const char *description) {
    register_route(app, CREST_DELETE, path, handler, description);
}

void crest_patch(crest_app_t *app, const char *path, crest_handler_t handler, const char *description) {
    register_route(app, CREST_PATCH, path, handler, description);
}

void crest_use(crest_app_t *app, crest_middleware_fn_t middleware_fn) {
    if (!app || !middleware_fn) {
        return;
    }
    
    if (app->middleware_count >= CREST_MAX_MIDDLEWARE) {
        crest_log(CREST_LOG_ERROR, "Maximum middleware (%d) exceeded", CREST_MAX_MIDDLEWARE);
        return;
    }
    
    app->middleware[app->middleware_count].handler = middleware_fn;
    app->middleware_count++;
}

void crest_enable_dashboard(crest_app_t *app, bool enable) {
    if (app && app->config) {
        app->config->enable_dashboard = enable;
        
        if (enable) {
            /* Register reserved dashboard routes */
            /* These routes are protected and cannot be overridden by users */
            
            /* Swagger UI at /docs */
            crest_get(app, "/docs", crest_swagger_ui_handler, "Swagger UI - Interactive API Documentation");
            
            /* ReDoc UI at /redoc */
            crest_get(app, "/redoc", crest_redoc_ui_handler, "ReDoc UI - Read-Only API Documentation");
            
            /* OpenAPI JSON specification at /openapi.json */
            crest_get(app, "/openapi.json", crest_openapi_json_handler, "OpenAPI 3.0 Specification (JSON)");
            
            /* Legacy dashboard at /dashboard (optional) */
            crest_get(app, "/dashboard", crest_dashboard_handler, "Legacy Dashboard");
            
            /* Routes API at /api/routes */
            crest_get(app, "/api/routes", crest_api_routes_handler, "List all registered routes");
        }
    }
}

void crest_set_dashboard_path(crest_app_t *app, const char *path) {
    if (app && app->config && path) {
        free(app->config->dashboard_path);
        app->config->dashboard_path = strdup(path);
    }
}

void crest_destroy(crest_app_t *app) {
    if (!app) {
        return;
    }
    
    // Free routes
    for (size_t i = 0; i < app->route_count; i++) {
        free(app->routes[i].path);
        free(app->routes[i].pattern);
        free(app->routes[i].description);
    }
    
    // Free config
    crest_config_destroy(app->config);
    
    free(app);
}

const char* crest_version(void) {
    return CREST_VERSION;
}

void crest_set_logging(bool enable) {
    // Global logging setting
}

void crest_set_log_level(int level) {
    // Global log level setting
}

void crest_set_port(crest_app_t *app, int port) {
    if (app && app->config) {
        app->config->port = port;
    }
}

void crest_enable_logging(crest_app_t *app, bool enable) {
    if (app && app->config) {
        app->config->enable_logging = enable;
    }
}

int crest_listen(crest_app_t *app) {
    if (!app || !app->config) {
        return -1;
    }
    
    const char *host = app->config->host ? app->config->host : "0.0.0.0";
    int port = app->config->port > 0 ? app->config->port : 8080;
    
    return crest_run(app, host, port);
}

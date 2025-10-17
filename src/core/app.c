/**
 * @file app.c
 * @brief Core application implementation
 */

#include "crest/crest.h"
#include "../../include/crest/internal/app_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _MSC_VER
#define strdup _strdup
#endif

extern void* crest_mutex_create();
extern void crest_mutex_destroy(void* mutex);

crest_app_t* crest_create(void) {
    crest_app_t* app = (crest_app_t*)calloc(1, sizeof(crest_app_t));
    if (!app) return NULL;
    
    app->title = strdup("Crest API");
    app->description = strdup("RESTful API built with Crest");
    app->version = strdup(CREST_VERSION);
    app->docs_enabled = true;
    app->docs_path = strdup("/docs");
    app->openapi_path = strdup("/openapi.json");
    app->routes = NULL;
    app->route_count = 0;
    app->route_capacity = 0;
    app->running = false;
    app->route_mutex = crest_mutex_create();
    app->thread_pool = NULL;
    
    return app;
}

crest_app_t* crest_create_with_config(crest_config_t* config) {
    crest_app_t* app = crest_create();
    if (!app || !config) return app;
    
    if (config->title) {
        free(app->title);
        app->title = strdup(config->title);
    }
    if (config->description) {
        free(app->description);
        app->description = strdup(config->description);
    }
    if (config->version) {
        free(app->version);
        app->version = strdup(config->version);
    }
    app->docs_enabled = config->docs_enabled;
    
    return app;
}

void crest_destroy(crest_app_t* app) {
    if (!app) return;
    
    free(app->title);
    free(app->description);
    free(app->version);
    free(app->docs_path);
    free(app->openapi_path);
    free(app->proxy_url);
    
    for (size_t i = 0; i < app->route_count; i++) {
        free(app->routes[i].path);
        free(app->routes[i].description);
        free(app->routes[i].request_schema);
        free(app->routes[i].response_schema);
    }
    free(app->routes);
    
    if (app->route_mutex) {
        crest_mutex_destroy(app->route_mutex);
    }
    
    free(app);
}

void crest_set_docs_enabled(crest_app_t* app, bool enabled) {
    if (app) app->docs_enabled = enabled;
}

void crest_set_title(crest_app_t* app, const char* title) {
    if (app && title) {
        free(app->title);
        app->title = strdup(title);
    }
}

void crest_set_description(crest_app_t* app, const char* description) {
    if (app && description) {
        free(app->description);
        app->description = strdup(description);
    }
}

void crest_set_proxy(crest_app_t* app, const char* proxy_url) {
    if (app && proxy_url) {
        free(app->proxy_url);
        app->proxy_url = strdup(proxy_url);
    }
}

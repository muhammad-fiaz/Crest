/**
 * @file router.c
 * @brief Router implementation for Crest
 */

#include "crest/router.h"
#include "crest/crest.h"
#include "crest/types.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void router_register_route(crest_router_t *router, int method, const char *path,
                                 crest_handler_t handler, const char *description) {
    if (!router || !path || !handler) {
        return;
    }

    if (router->route_count >= CREST_MAX_ROUTES) {
        fprintf(stderr, "[CREST] Error: Router maximum routes (%d) exceeded\n", CREST_MAX_ROUTES);
        return;
    }

    /* Check for duplicate routes within this router */
    for (size_t i = 0; i < router->route_count; i++) {
        if (router->routes[i].method == method && strcmp(router->routes[i].path, path) == 0) {
            const char *method_str[] = {"GET", "POST", "PUT", "DELETE", "PATCH", "HEAD", "OPTIONS"};
            fprintf(stderr, "[CREST] Warning: Duplicate route in router - %s %s already exists\n",
                    method_str[method], path);
            fprintf(stderr, "[CREST] Warning: Previous registration will be overwritten\n");
            /* Overwrite the existing route */
            crest_route_t *route = &router->routes[i];
            free(route->path);
            free(route->description);
            free(route->pattern);

            route->method = method;
            route->path = strdup(path);
            route->handler = handler;
            route->description = description ? strdup(description) : NULL;
            /* Simple route - no pattern matching for now */
            route->is_pattern = false;
            route->pattern = NULL;
            return;
        }
    }

    /* Add new route */
    crest_route_t *route = &router->routes[router->route_count++];
    route->method = method;
    route->path = strdup(path);
    route->handler = handler;
    route->description = description ? strdup(description) : NULL;
    /* Simple route - no pattern matching for now */
    route->is_pattern = false;
    route->pattern = NULL;
}

crest_router_t* crest_router_create(const char *prefix) {
    crest_router_t *router = (crest_router_t*)calloc(1, sizeof(crest_router_t));
    if (router) {
        if (prefix) {
            router->prefix = strdup(prefix);
        } else {
            router->prefix = strdup("");
        }
        router->route_count = 0;
        router->middleware_count = 0;
    }
    return router;
}

void crest_router_get(crest_router_t *router, const char *path,
                     crest_handler_t handler, const char *description) {
    router_register_route(router, CREST_GET, path, handler, description);
}

void crest_router_post(crest_router_t *router, const char *path,
                      crest_handler_t handler, const char *description) {
    router_register_route(router, CREST_POST, path, handler, description);
}

void crest_router_put(crest_router_t *router, const char *path,
                     crest_handler_t handler, const char *description) {
    router_register_route(router, CREST_PUT, path, handler, description);
}

void crest_router_delete(crest_router_t *router, const char *path,
                        crest_handler_t handler, const char *description) {
    router_register_route(router, CREST_DELETE, path, handler, description);
}

void crest_router_use(crest_router_t *router, crest_middleware_fn_t middleware_fn) {
    if (!router || !middleware_fn) {
        return;
    }

    if (router->middleware_count >= CREST_MAX_MIDDLEWARE) {
        fprintf(stderr, "[CREST] Error: Router maximum middleware (%d) exceeded\n", CREST_MAX_MIDDLEWARE);
        return;
    }

    router->middleware[router->middleware_count++].handler = middleware_fn;
}

void crest_mount(crest_app_t *app, crest_router_t *router) {
    if (!app || !router) {
        return;
    }

    /* Mount middleware first */
    for (size_t i = 0; i < router->middleware_count; i++) {
        crest_use(app, router->middleware[i].handler);
    }

    /* Mount routes with prefix */
    for (size_t i = 0; i < router->route_count; i++) {
        crest_route_t *route = &router->routes[i];
        char prefixed_path[CREST_MAX_PATH_LEN];

        /* Combine prefix and path */
        if (strlen(router->prefix) > 0) {
            if (router->prefix[strlen(router->prefix) - 1] == '/' && route->path[0] == '/') {
                /* Both have slashes, remove one */
                snprintf(prefixed_path, sizeof(prefixed_path), "%s%s",
                        router->prefix, route->path + 1);
            } else if (router->prefix[strlen(router->prefix) - 1] != '/' && route->path[0] != '/') {
                /* Neither has slash, add one */
                snprintf(prefixed_path, sizeof(prefixed_path), "%s/%s",
                        router->prefix, route->path);
            } else {
                /* One has slash, combine as-is */
                snprintf(prefixed_path, sizeof(prefixed_path), "%s%s",
                        router->prefix, route->path);
            }
        } else {
            /* No prefix, use route path as-is */
            strncpy(prefixed_path, route->path, sizeof(prefixed_path) - 1);
            prefixed_path[sizeof(prefixed_path) - 1] = '\0';
        }

        /* Register the route on the app */
        switch (route->method) {
            case CREST_GET:
                crest_get(app, prefixed_path, route->handler, route->description);
                break;
            case CREST_POST:
                crest_post(app, prefixed_path, route->handler, route->description);
                break;
            case CREST_PUT:
                crest_put(app, prefixed_path, route->handler, route->description);
                break;
            case CREST_DELETE:
                crest_delete(app, prefixed_path, route->handler, route->description);
                break;
            case CREST_PATCH:
                crest_patch(app, prefixed_path, route->handler, route->description);
                break;
        }
    }
}

void crest_router_destroy(crest_router_t *router) {
    if (router) {
        /* Free routes */
        for (size_t i = 0; i < router->route_count; i++) {
            free(router->routes[i].path);
            free(router->routes[i].description);
            free(router->routes[i].pattern);
        }

        free(router->prefix);
        free(router);
    }
}

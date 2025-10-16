/**
 * @file router.h
 * @brief Router functionality for Crest
 */

#ifndef CREST_ROUTER_H
#define CREST_ROUTER_H

#include "crest.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a router (for grouping routes)
 */
typedef struct crest_router crest_router_t;

/**
 * @brief Create a new router
 * @param prefix Path prefix for all routes
 * @return New router instance
 */
CREST_API crest_router_t* crest_router_create(const char *prefix);

/**
 * @brief Add GET route to router
 */
CREST_API void crest_router_get(crest_router_t *router, const char *path, crest_handler_t handler, const char *description);

/**
 * @brief Add POST route to router
 */
CREST_API void crest_router_post(crest_router_t *router, const char *path, crest_handler_t handler, const char *description);

/**
 * @brief Add PUT route to router
 */
CREST_API void crest_router_put(crest_router_t *router, const char *path, crest_handler_t handler, const char *description);

/**
 * @brief Add DELETE route to router
 */
CREST_API void crest_router_delete(crest_router_t *router, const char *path, crest_handler_t handler, const char *description);

/**
 * @brief Add middleware to router
 */
CREST_API void crest_router_use(crest_router_t *router, crest_middleware_fn_t middleware_fn);

/**
 * @brief Mount router on application
 */
CREST_API void crest_mount(crest_app_t *app, crest_router_t *router);

/**
 * @brief Free router
 */
CREST_API void crest_router_destroy(crest_router_t *router);

#ifdef __cplusplus
}
#endif

#endif /* CREST_ROUTER_H */

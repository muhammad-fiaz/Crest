/**
 * @file crest.h
 * @brief Main header file for Crest - A modern C REST API framework
 * @author Muhammad Fiaz
 * @version 1.0.0
 * @date 2025-10-15
 * 
 * Crest is a lightweight, fast, and modular REST API framework for C/C++
 * with a beautiful web dashboard similar to FastAPI.
 */

#ifndef CREST_H
#define CREST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

/* Version information */
#define CREST_VERSION_MAJOR 1
#define CREST_VERSION_MINOR 0
#define CREST_VERSION_PATCH 0
#define CREST_VERSION "1.0.0"

/* DLL export/import macro */
#ifdef _WIN32
    #ifdef CREST_STATIC
        /* Static library - no dll import/export */
        #define CREST_API
    #elif defined(CREST_BUILD_SHARED)
        #define CREST_API __declspec(dllexport)
    #else
        #define CREST_API __declspec(dllimport)
    #endif
#else
    #define CREST_API
#endif

/* Logging macros */
#define CREST_LOG_DEBUG 0
#define CREST_LOG_INFO 1
#define CREST_LOG_WARN 2
#define CREST_LOG_ERROR 3

/* Logging functions */
CREST_API void crest_log(int level, const char *format, ...);
CREST_API void crest_log_set_level(int level);
CREST_API void crest_log_enable(int enable);

/* HTTP Methods */
typedef enum {
    CREST_GET,
    CREST_POST,
    CREST_PUT,
    CREST_DELETE,
    CREST_PATCH,
    CREST_HEAD,
    CREST_OPTIONS
} crest_method_t;

/* HTTP Status Codes */
typedef enum {
    CREST_STATUS_OK = 200,
    CREST_STATUS_CREATED = 201,
    CREST_STATUS_ACCEPTED = 202,
    CREST_STATUS_NO_CONTENT = 204,
    CREST_STATUS_PARTIAL_CONTENT = 206,
    CREST_STATUS_BAD_REQUEST = 400,
    CREST_STATUS_UNAUTHORIZED = 401,
    CREST_STATUS_FORBIDDEN = 403,
    CREST_STATUS_NOT_FOUND = 404,
    CREST_STATUS_METHOD_NOT_ALLOWED = 405,
    CREST_STATUS_CONFLICT = 409,
    CREST_STATUS_TOO_MANY_REQUESTS = 429,
    CREST_STATUS_REQUEST_ENTITY_TOO_LARGE = 413,
    CREST_STATUS_INTERNAL_SERVER_ERROR = 500,
    CREST_STATUS_NOT_IMPLEMENTED = 501,
    CREST_STATUS_SERVICE_UNAVAILABLE = 503
} crest_status_t;

/* Forward declarations */
typedef struct crest_app crest_app_t;
typedef struct crest_request crest_request_t;
typedef struct crest_response crest_response_t;
typedef struct crest_route crest_route_t;
typedef struct crest_config crest_config_t;
typedef struct crest_middleware crest_middleware_t;
typedef struct crest_thread_pool crest_thread_pool_t;
typedef struct crest_router crest_router_t;
typedef struct crest_route_match crest_route_match_t;

/* Route handler function type */
typedef void (*crest_handler_t)(crest_request_t *req, crest_response_t *res);

/* Middleware function type */
typedef bool (*crest_middleware_fn_t)(crest_request_t *req, crest_response_t *res);

/**
 * @brief Create a new Crest application
 * @return Pointer to the newly created application
 */
CREST_API crest_app_t* crest_create(void);

/**
 * @brief Create a Crest application with custom configuration
 * @param config Configuration structure
 * @return Pointer to the newly created application
 */
crest_app_t* crest_create_with_config(crest_config_t *config);

/**
 * @brief Register a GET route
 * @param app Application instance
 * @param path Route path (e.g., "/api/users")
 * @param handler Handler function
 * @param description Optional description for documentation
 */
CREST_API void crest_get(crest_app_t *app, const char *path, crest_handler_t handler, const char *description);

/**
 * @brief Register a POST route
 * @param app Application instance
 * @param path Route path
 * @param handler Handler function
 * @param description Optional description for documentation
 */
CREST_API void crest_post(crest_app_t *app, const char *path, crest_handler_t handler, const char *description);

/**
 * @brief Register a PUT route
 * @param app Application instance
 * @param path Route path
 * @param handler Handler function
 * @param description Optional description for documentation
 */
CREST_API void crest_put(crest_app_t *app, const char *path, crest_handler_t handler, const char *description);

/**
 * @brief Register a DELETE route
 * @param app Application instance
 * @param path Route path
 * @param handler Handler function
 * @param description Optional description for documentation
 */
CREST_API void crest_delete(crest_app_t *app, const char *path, crest_handler_t handler, const char *description);

/**
 * @brief Register a PATCH route
 * @param app Application instance
 * @param path Route path
 * @param handler Handler function
 * @param description Optional description for documentation
 */
CREST_API void crest_patch(crest_app_t *app, const char *path, crest_handler_t handler, const char *description);

/* Router functions */

/**
 * @brief Create a new router for grouping routes
 * @param prefix Optional path prefix for all routes
 * @return New router instance
 */
CREST_API crest_router_t* crest_router_create(const char *prefix);

/**
 * @brief Register a GET route on router
 * @param router Router instance
 * @param path Route path
 * @param handler Handler function
 * @param description Optional description
 */
CREST_API void crest_router_get(crest_router_t *router, const char *path, crest_handler_t handler, const char *description);

/**
 * @brief Register a POST route on router
 * @param router Router instance
 * @param path Route path
 * @param handler Handler function
 * @param description Optional description
 */
CREST_API void crest_router_post(crest_router_t *router, const char *path, crest_handler_t handler, const char *description);

/**
 * @brief Register a PUT route on router
 * @param router Router instance
 * @param path Route path
 * @param handler Handler function
 * @param description Optional description
 */
CREST_API void crest_router_put(crest_router_t *router, const char *path, crest_handler_t handler, const char *description);

/**
 * @brief Register a DELETE route on router
 * @param router Router instance
 * @param path Route path
 * @param handler Handler function
 * @param description Optional description
 */
CREST_API void crest_router_delete(crest_router_t *router, const char *path, crest_handler_t handler, const char *description);

/**
 * @brief Add middleware to router
 * @param router Router instance
 * @param middleware_fn Middleware function
 */
CREST_API void crest_router_use(crest_router_t *router, crest_middleware_fn_t middleware_fn);

/**
 * @brief Mount router on application
 * @param app Application instance
 * @param router Router instance to mount
 */
CREST_API void crest_mount(crest_app_t *app, crest_router_t *router);

/**
 * @brief Free router resources
 * @param router Router instance
 */
CREST_API void crest_router_destroy(crest_router_t *router);

/**
 * @brief Match a route against a path and method
 * @param router Router instance
 * @param method HTTP method
 * @param path Request path
 * @param match Output match structure
 * @return true if route matched, false otherwise
 */
CREST_API bool crest_router_match(crest_router_t *router, int method, const char *path, crest_route_match_t *match);

/* Middleware functions */

/**
 * @brief Create CORS middleware
 * @return Middleware function
 */
CREST_API crest_middleware_fn_t crest_middleware_cors(void);

/**
 * @brief Create logger middleware
 * @return Middleware function
 */
CREST_API crest_middleware_fn_t crest_middleware_logger(void);

/**
 * @brief Create body parser middleware
 * @return Middleware function
 */
CREST_API crest_middleware_fn_t crest_middleware_body_parser(void);

/**
 * @brief Create static file server middleware
 * @param dir Directory to serve files from
 * @return Middleware function
 */
CREST_API crest_middleware_fn_t crest_middleware_static(const char *dir);

/**
 * @brief Create rate limiting middleware
 * @param max_requests Maximum requests per window
 * @param window_seconds Time window in seconds
 * @return Middleware function
 */
CREST_API crest_middleware_fn_t crest_middleware_rate_limit(int max_requests, int window_seconds);

/**
 * @brief Create authentication middleware
 * @param validate_fn Function to validate credentials
 * @return Middleware function
 */
CREST_API crest_middleware_fn_t crest_middleware_auth(bool (*validate_fn)(const char *token));

/**
 * @brief Add middleware to the application
 * @param app Application instance
 * @param middleware_fn Middleware function
 */
CREST_API void crest_use(crest_app_t *app, crest_middleware_fn_t middleware_fn);

/**
 * @brief Start the server
 * @param app Application instance
 * @param host Host to bind (e.g., "0.0.0.0" or "localhost")
 * @param port Port number
 * @return 0 on success, -1 on error
 */
CREST_API int crest_run(crest_app_t *app, const char *host, int port);

/**
 * @brief Enable the web dashboard (like FastAPI docs)
 * @param app Application instance
 * @param enable True to enable, false to disable
 */
CREST_API void crest_enable_dashboard(crest_app_t *app, bool enable);

/**
 * @brief Set custom dashboard path (default: "/docs")
 * @param app Application instance
 * @param path Dashboard path
 */
CREST_API void crest_set_dashboard_path(crest_app_t *app, const char *path);

/* Dashboard route handlers (internal - automatically registered) */
CREST_API void crest_dashboard_handler(crest_request_t *req, crest_response_t *res);
CREST_API void crest_api_routes_handler(crest_request_t *req, crest_response_t *res);
CREST_API void crest_openapi_json_handler(crest_request_t *req, crest_response_t *res);
CREST_API void crest_swagger_ui_handler(crest_request_t *req, crest_response_t *res);
CREST_API void crest_redoc_ui_handler(crest_request_t *req, crest_response_t *res);

/**
 * @brief Free the application and all resources
 * @param app Application instance
 */
CREST_API void crest_destroy(crest_app_t *app);

/* Request functions */

/**
 * @brief Get request method
 */
CREST_API crest_method_t crest_request_method(crest_request_t *req);

/**
 * @brief Get request path
 */
CREST_API const char* crest_request_path(crest_request_t *req);

/**
 * @brief Get query parameter value
 */
CREST_API const char* crest_request_query(crest_request_t *req, const char *key);

/**
 * @brief Get header value
 */
CREST_API const char* crest_request_header(crest_request_t *req, const char *key);

/**
 * @brief Get request body
 */
CREST_API const char* crest_request_body(crest_request_t *req);

/**
 * @brief Get path parameter (e.g., /users/:id)
 */
CREST_API const char* crest_request_param(crest_request_t *req, const char *key);

/**
 * @brief Parse JSON body
 * @return JSON object (implementation specific)
 */
void* crest_request_json(crest_request_t *req);

/* Response functions */

/**
 * @brief Set response status code
 */
CREST_API void crest_response_status(crest_response_t *res, crest_status_t status);

/**
 * @brief Set response header
 */
CREST_API void crest_response_header(crest_response_t *res, const char *key, const char *value);

/**
 * @brief Send plain text response
 */
CREST_API void crest_response_send(crest_response_t *res, const char *body);

/**
 * @brief Send JSON response
 */
CREST_API void crest_response_json(crest_response_t *res, const char *json);

/**
 * @brief Send file as response
 */
void crest_response_file(crest_response_t *res, const char *filepath);

/**
 * @brief Send formatted response
 */
void crest_response_sendf(crest_response_t *res, const char *format, ...);

/* Configuration */

/**
 * @brief Create default configuration
 */
CREST_API crest_config_t* crest_config_create(void);

/**
 * @brief Load configuration from file
 */
CREST_API crest_config_t* crest_config_load(const char *filepath);

/**
 * @brief Free configuration
 */
CREST_API void crest_config_destroy(crest_config_t *config);

/* Thread Pool for Concurrent Processing */

/**
 * @brief Create a thread pool for concurrent request processing
 * @param thread_count Number of worker threads (0 for auto-detect CPU cores)
 * @return Thread pool instance or NULL on error
 */
crest_thread_pool_t* crest_thread_pool_create(size_t thread_count);

/**
 * @brief Destroy thread pool and free resources
 * @param pool Thread pool instance
 */
void crest_thread_pool_destroy(crest_thread_pool_t *pool);

/**
 * @brief Submit a task to the thread pool
 * @param pool Thread pool instance
 * @param function Task function
 * @param arg Task argument
 * @return true on success, false on error
 */
bool crest_thread_pool_submit(crest_thread_pool_t *pool, void (*function)(void *), void *arg);

/**
 * @brief Submit a request for concurrent processing
 * @param pool Thread pool instance
 * @param app Application instance
 * @param req Request instance (will be freed by thread pool)
 * @param res Response instance (will be freed by thread pool)
 * @return true on success, false on error
 */
bool crest_thread_pool_submit_request(crest_thread_pool_t *pool, crest_app_t *app,
                                     crest_request_t *req, crest_response_t *res);

/**
 * @brief Get number of threads in the pool
 * @param pool Thread pool instance
 * @return Number of threads
 */
size_t crest_thread_pool_get_thread_count(const crest_thread_pool_t *pool);

/**
 * @brief Get current queue size
 * @param pool Thread pool instance
 * @return Number of pending tasks
 */
size_t crest_thread_pool_get_queue_size(const crest_thread_pool_t *pool);

/* Utility functions */

/**
 * @brief Get Crest version string
 */
const char* crest_version(void);

/**
 * @brief Set server port
 * @param app Application instance
 * @param port Port number
 */
CREST_API void crest_set_port(crest_app_t *app, int port);

/**
 * @brief Enable logging for the application
 * @param app Application instance
 * @param enable True to enable logging
 */
CREST_API void crest_enable_logging(crest_app_t *app, bool enable);

/**
 * @brief Start listening for connections (convenience function)
 * @param app Application instance
 * @return 0 on success, -1 on error
 */
CREST_API int crest_listen(crest_app_t *app);

/**
 * @brief Validate configuration
 * @param config Configuration to validate
 * @return true if valid, false otherwise
 */
CREST_API bool crest_config_validate(const crest_config_t *config);

#ifdef __cplusplus
}
#endif

#endif /* CREST_H */

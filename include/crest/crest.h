/**
 * @file crest.h
 * @brief Crest RESTful API Framework - Main C API Header
 * @version 0.0.0
 * @author Muhammad Fiaz <contact@muhammadfiaz.com>
 * 
 * Production-ready RESTful API framework for C projects
 * Inspired by modern API frameworks for simplicity and performance
 */

#ifndef CREST_H
#define CREST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define CREST_VERSION "0.0.0"
#define CREST_VERSION_MAJOR 0
#define CREST_VERSION_MINOR 0
#define CREST_VERSION_PATCH 0

#ifdef CREST_BUILD_SHARED
    #ifdef CREST_WINDOWS
        #define CREST_API __declspec(dllexport)
    #else
        #define CREST_API __attribute__((visibility("default")))
    #endif
#else
    #define CREST_API
#endif

typedef struct crest_app crest_app_t;
typedef struct crest_request crest_request_t;
typedef struct crest_response crest_response_t;

typedef struct crest_config {
    const char* title;
    const char* description;
    const char* version;
    bool docs_enabled;
} crest_config_t;

typedef enum {
    CREST_GET,
    CREST_POST,
    CREST_PUT,
    CREST_DELETE,
    CREST_PATCH,
    CREST_HEAD,
    CREST_OPTIONS
} crest_method_t;

typedef enum {
    CREST_STATUS_OK = 200,
    CREST_STATUS_CREATED = 201,
    CREST_STATUS_BAD_REQUEST = 400,
    CREST_STATUS_NOT_FOUND = 404,
    CREST_STATUS_INTERNAL_ERROR = 500
} crest_status_t;

typedef void (*crest_handler_t)(crest_request_t* req, crest_response_t* res);

/**
 * @brief Create a new Crest application
 * @return Pointer to the created application
 */
CREST_API crest_app_t* crest_create(void);

/**
 * @brief Create a new Crest application with configuration
 * @param config Configuration structure
 * @return Pointer to the created application
 */
CREST_API crest_app_t* crest_create_with_config(crest_config_t* config);

/**
 * @brief Destroy a Crest application
 * @param app Application to destroy
 */
CREST_API void crest_destroy(crest_app_t* app);

/**
 * @brief Register a route handler
 * @param app Application instance
 * @param method HTTP method
 * @param path Route path
 * @param handler Handler function
 * @param description Route description for documentation
 * @return 0 on success, -1 on error
 */
CREST_API int crest_route(crest_app_t* app, crest_method_t method, const char* path, 
                          crest_handler_t handler, const char* description);

/**
 * @brief Set request schema for a route
 * @param app Application instance
 * @param method HTTP method
 * @param path Route path
 * @param schema JSON schema string (e.g., "{\"name\": \"string\", \"age\": \"number\"}")
 */
CREST_API void crest_set_request_schema(crest_app_t* app, crest_method_t method, const char* path, const char* schema);

/**
 * @brief Set response schema for a route
 * @param app Application instance
 * @param method HTTP method
 * @param path Route path
 * @param schema JSON schema string (e.g., "{\"id\": \"number\", \"status\": \"string\"}")
 */
CREST_API void crest_set_response_schema(crest_app_t* app, crest_method_t method, const char* path, const char* schema);

/**
 * @brief Start the server
 * @param app Application instance
 * @param host Host address (e.g., "0.0.0.0")
 * @param port Port number
 * @return 0 on success, -1 on error
 */
CREST_API int crest_run(crest_app_t* app, const char* host, int port);

/**
 * @brief Stop the server
 * @param app Application instance
 */
CREST_API void crest_stop(crest_app_t* app);

/**
 * @brief Get request path
 * @param req Request object
 * @return Path string
 */
CREST_API const char* crest_request_get_path(crest_request_t* req);

/**
 * @brief Get request method
 * @param req Request object
 * @return Method string
 */
CREST_API const char* crest_request_get_method(crest_request_t* req);

/**
 * @brief Get request body
 * @param req Request object
 * @return Body string
 */
CREST_API const char* crest_request_get_body(crest_request_t* req);

/**
 * @brief Get query parameter
 * @param req Request object
 * @param key Parameter key
 * @return Parameter value or NULL
 */
CREST_API const char* crest_request_get_query(crest_request_t* req, const char* key);

/**
 * @brief Get header value
 * @param req Request object
 * @param key Header key
 * @return Header value or NULL
 */
CREST_API const char* crest_request_get_header(crest_request_t* req, const char* key);

/**
 * @brief Send JSON response
 * @param res Response object
 * @param status HTTP status code
 * @param json JSON string
 */
CREST_API void crest_response_json(crest_response_t* res, int status, const char* json);

/**
 * @brief Send text response
 * @param res Response object
 * @param status HTTP status code
 * @param text Text content
 */
CREST_API void crest_response_text(crest_response_t* res, int status, const char* text);

/**
 * @brief Send HTML response
 * @param res Response object
 * @param status HTTP status code
 * @param html HTML content
 */
CREST_API void crest_response_html(crest_response_t* res, int status, const char* html);

/**
 * @brief Set response header
 * @param res Response object
 * @param key Header key
 * @param value Header value
 */
CREST_API void crest_response_set_header(crest_response_t* res, const char* key, const char* value);

/**
 * @brief Enable/disable Swagger UI
 * @param app Application instance
 * @param enabled true to enable, false to disable
 */
CREST_API void crest_set_docs_enabled(crest_app_t* app, bool enabled);

/**
 * @brief Set application title for documentation
 * @param app Application instance
 * @param title Application title
 */
CREST_API void crest_set_title(crest_app_t* app, const char* title);

/**
 * @brief Set application description for documentation
 * @param app Application instance
 * @param description Application description
 */
CREST_API void crest_set_description(crest_app_t* app, const char* description);

/**
 * @brief Configure proxy settings
 * @param app Application instance
 * @param proxy_url Proxy URL
 */
CREST_API void crest_set_proxy(crest_app_t* app, const char* proxy_url);

/**
 * @brief Enable or disable console logging
 * @param enabled true to enable, false to disable
 */
CREST_API void crest_log_set_enabled(bool enabled);

/**
 * @brief Enable or disable timestamps in logs
 * @param enabled true to enable, false to disable
 */
CREST_API void crest_log_set_timestamp(bool enabled);

#ifdef __cplusplus
}
#endif

#endif /* CREST_H */

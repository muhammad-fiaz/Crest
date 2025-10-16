/**
 * @file types.h
 * @brief Internal type definitions for Crest framework
 */

#ifndef CREST_TYPES_H
#define CREST_TYPES_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum limits */
#define CREST_MAX_ROUTES 256
#define CREST_MAX_HEADERS 64
#define CREST_MAX_PARAMS 32
#define CREST_MAX_QUERY_PARAMS 32
#define CREST_MAX_MIDDLEWARE 32
#define CREST_MAX_PATH_LEN 512
#define CREST_MAX_BODY_SIZE (10 * 1024 * 1024) /* 10MB */

/* Key-Value pair */
typedef struct {
    char *key;
    char *value;
} crest_kv_t;

/* Request structure */
struct crest_request {
    int method;
    char *path;
    char *body;
    size_t body_len;
    crest_kv_t headers[CREST_MAX_HEADERS];
    size_t header_count;
    crest_kv_t query_params[CREST_MAX_QUERY_PARAMS];
    size_t query_count;
    crest_kv_t path_params[CREST_MAX_PARAMS];
    size_t param_count;
    void *json_data;
    void *user_data;
    /* Parsed body data (from middleware) */
    void *parsed_body;
    char *content_type;
};

/* Response structure */
struct crest_response {
    int status_code;
    crest_kv_t headers[CREST_MAX_HEADERS];
    size_t header_count;
    char *body;
    size_t body_len;
    bool sent;
};

/* Route structure */
struct crest_route {
    int method;
    char *path;
    char *pattern;  /* For path parameters like /users/:id */
    crest_handler_t handler;
    char *description;
    char *summary;
    char *tags;
    bool is_pattern;
};

/* Middleware structure */
struct crest_middleware {
    crest_middleware_fn_t handler;
};

/* Configuration structure */
struct crest_config {
    char *host;
    int port;
    bool enable_logging;
    int log_level;
    bool enable_cors;
    bool enable_dashboard;
    char *dashboard_path;
    size_t max_body_size;
    int timeout_seconds;
    char *static_dir;
    char *upload_dir;
    /* Thread pool configuration */
    int thread_count;
    /* Rate limiting configuration */
    int rate_limit_max_requests;
    int rate_limit_window_seconds;
    /* Timeout configuration */
    int read_timeout_ms;
    int write_timeout_ms;
};

/* Application structure */
struct crest_app {
    crest_route_t routes[CREST_MAX_ROUTES];
    size_t route_count;
    crest_middleware_t middleware[CREST_MAX_MIDDLEWARE];
    size_t middleware_count;
    crest_config_t *config;
    int socket_fd;
    bool running;
    void *user_data;
};

/* Rate limiting types */
typedef struct rate_limit_entry {
    char ip[46]; /* IPv6 max length */
    int request_count;
    time_t window_start;
    struct rate_limit_entry *next;
} rate_limit_entry_t;

typedef struct {
    int max_requests;
    int window_seconds;
    rate_limit_entry_t *entries;
} rate_limit_data_t;

/* Route match structure */
struct crest_route_match {
    crest_handler_t handler;
    crest_kv_t params[CREST_MAX_PARAMS];
    size_t param_count;
};

/* Router structure */
struct crest_router {
    char *prefix;
    crest_route_t routes[CREST_MAX_ROUTES];
    size_t route_count;
    crest_middleware_t middleware[CREST_MAX_MIDDLEWARE];
    size_t middleware_count;
};

#ifdef __cplusplus
}
#endif

#endif /* CREST_TYPES_H */

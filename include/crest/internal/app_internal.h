/**
 * @file app_internal.h
 * @brief Internal application structures
 */

#ifndef CREST_APP_INTERNAL_H
#define CREST_APP_INTERNAL_H

#include "../crest.h"

#ifdef __cplusplus
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <vector>
#include <functional>
#endif

typedef struct {
    crest_method_t method;
    char* path;
    crest_handler_t handler;
    char* description;
    void* cpp_handler;
    char* request_schema;
    char* response_schema;
} crest_route_entry_t;

struct crest_app {
    char* title;
    char* description;
    char* version;
    bool docs_enabled;
    char* docs_path;
    char* openapi_path;
    char* proxy_url;
    crest_route_entry_t* routes;
    size_t route_count;
    size_t route_capacity;
    bool running;
    int server_socket;
    void* route_mutex;
    void* thread_pool;
};

struct crest_request {
    char* method;
    char* path;
    char* body;
    char* query_string;
    void* headers;
    void* queries;
};

struct crest_response {
    int status;
    char* body;
    void* headers;
    bool sent;
};



#endif /* CREST_APP_INTERNAL_H */

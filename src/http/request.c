/**
 * @file request.c
 * @brief HTTP request handling
 */

#include "crest/crest.h"
#include "crest/internal/app_internal.h"
#include <string.h>

const char* crest_request_get_path(crest_request_t* req) {
    return req ? req->path : NULL;
}

const char* crest_request_get_method(crest_request_t* req) {
    return req ? req->method : NULL;
}

const char* crest_request_get_body(crest_request_t* req) {
    return req ? req->body : NULL;
}

const char* crest_request_get_query(crest_request_t* req, const char* key) {
    if (!req || !key || !req->queries) return NULL;
    // Implementation would use a hash map
    return NULL;
}

const char* crest_request_get_header(crest_request_t* req, const char* key) {
    if (!req || !key || !req->headers) return NULL;
    // Implementation would use a hash map
    return NULL;
}

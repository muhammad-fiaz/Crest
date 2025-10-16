/**
 * @file request.c
 * @brief Request handling implementation
 */

#include "crest/crest.h"
#include "crest/types.h"
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

crest_method_t crest_request_method(crest_request_t *req) {
    return req ? (crest_method_t)req->method : CREST_GET;
}

const char* crest_request_path(crest_request_t *req) {
    return req ? req->path : NULL;
}

const char* crest_request_query(crest_request_t *req, const char *key) {
    if (!req || !key) {
        return NULL;
    }
    
    for (size_t i = 0; i < req->query_count; i++) {
        if (strcmp(req->query_params[i].key, key) == 0) {
            return req->query_params[i].value;
        }
    }
    
    return NULL;
}

const char* crest_request_header(crest_request_t *req, const char *key) {
    if (!req || !key) {
        return NULL;
    }
    
    for (size_t i = 0; i < req->header_count; i++) {
        if (strcasecmp(req->headers[i].key, key) == 0) {
            return req->headers[i].value;
        }
    }
    
    return NULL;
}

const char* crest_request_body(crest_request_t *req) {
    return req ? req->body : NULL;
}

const char* crest_request_param(crest_request_t *req, const char *key) {
    if (!req || !key) {
        return NULL;
    }
    
    for (size_t i = 0; i < req->param_count; i++) {
        if (strcmp(req->path_params[i].key, key) == 0) {
            return req->path_params[i].value;
        }
    }
    
    return NULL;
}

void* crest_request_json(crest_request_t *req) {
    if (!req || !req->body) {
        return NULL;
    }
    
    // Parse JSON from body
    // This would use the JSON parser from json.c
    return req->json_data;
}

/**
 * @file response.c
 * @brief HTTP response handling
 */

#include "crest/crest.h"
#include "crest/internal/app_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void crest_response_json(crest_response_t* res, int status, const char* json) {
    if (!res || res->sent) return;
    
    res->status = status;
    free(res->body);
    
    size_t len = strlen(json) + 256;
    res->body = (char*)malloc(len);
    snprintf(res->body, len,
        "HTTP/1.1 %d OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n%s", status, strlen(json), json);
    
    res->sent = true;
}

void crest_response_text(crest_response_t* res, int status, const char* text) {
    if (!res || res->sent) return;
    
    res->status = status;
    free(res->body);
    
    size_t len = strlen(text) + 256;
    res->body = (char*)malloc(len);
    snprintf(res->body, len,
        "HTTP/1.1 %d OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n%s", status, strlen(text), text);
    
    res->sent = true;
}

void crest_response_html(crest_response_t* res, int status, const char* html) {
    if (!res || res->sent) return;
    
    res->status = status;
    free(res->body);
    
    size_t len = strlen(html) + 256;
    res->body = (char*)malloc(len);
    snprintf(res->body, len,
        "HTTP/1.1 %d OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n%s", status, strlen(html), html);
    
    res->sent = true;
}

void crest_response_set_header(crest_response_t* res, const char* key, const char* value) {
    if (!res || !key || !value) return;
    // Implementation would use a hash map
}

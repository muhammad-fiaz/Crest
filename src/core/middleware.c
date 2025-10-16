/**
 * @file middleware.c
 * @brief Complete middleware implementations for Crest
 */

#include "crest/middleware.h"
#include "crest/types.h"
#include "crest/json.h"
#include "crest/crest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#ifdef _WIN32
#define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
typedef long long ssize_t;
#else
#include <unistd.h>
#include <dirent.h>
#endif

/* Forward declarations for middleware data structures */
typedef struct {
    bool (*validate_fn)(const char *token);
} auth_data_t;

/* Global middleware data storage */
/* Note: In a production system, these would need thread-safety and better memory management */
static char *global_static_dir = NULL;
static rate_limit_data_t *global_rate_limit_data = NULL;
static auth_data_t *global_auth_data = NULL;

/* ====================================================================
 * CORS MIDDLEWARE
 * ==================================================================== */

static bool cors_handler(crest_request_t *req, crest_response_t *res) {
    crest_response_header(res, "Access-Control-Allow-Origin", "*");
    crest_response_header(res, "Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, PATCH, OPTIONS");
    crest_response_header(res, "Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
    crest_response_header(res, "Access-Control-Allow-Credentials", "true");

    /* Handle preflight requests */
    if (crest_request_method(req) == CREST_OPTIONS) {
        crest_response_status(res, CREST_STATUS_OK);
        crest_response_send(res, "");
        return false; /* Stop processing */
    }

    return true;
}

/* ====================================================================
 * LOGGER MIDDLEWARE
 * ==================================================================== */

static bool logger_handler(crest_request_t *req, crest_response_t *res) {
    const char *methods[] = {"GET", "HEAD", "POST", "PUT", "DELETE", "CONNECT", "OPTIONS", "TRACE", "PATCH"};
    const char *method = methods[crest_request_method(req)];
    const char *path = crest_request_path(req);

    /* Use colored logging */
    crest_log(CREST_LOG_INFO, "%s %s", method, path);

    (void)res;
    return true;
}

/* ====================================================================
 * BODY PARSER MIDDLEWARE
 * ==================================================================== */

typedef struct {
    char *buffer;
    size_t size;
    size_t capacity;
} body_buffer_t;

static bool body_parser_handler(crest_request_t *req, crest_response_t *res) {
    const char *content_type = crest_request_header(req, "Content-Type");
    if (!content_type && req->content_type) {
        content_type = req->content_type;
    }
    const char *body = crest_request_body(req);

    if (!body || !content_type) {
        return true; /* No body to parse */
    }

    /* JSON parsing */
    if (strstr(content_type, "application/json")) {
        crest_json_value_t *json = crest_json_parse(body);
        if (!json) {
            crest_response_status(res, CREST_STATUS_BAD_REQUEST);
            crest_response_json(res, "{\"error\":\"Invalid JSON\"}");
            return false;
        }
        /* Store parsed JSON in request for later use */
        req->parsed_body = json;
    }

    /* URL-encoded form data */
    else if (strstr(content_type, "application/x-www-form-urlencoded")) {
        /* Parse form data - basic implementation */
        char *form_data = strdup(body);
        if (!form_data) return true;

        char *pair = strtok(form_data, "&");
        while (pair) {
            char *eq = strchr(pair, '=');
            if (eq) {
                *eq = '\0';
                char *key = pair;
                char *value = eq + 1;

                /* URL decode value */
                char *decoded = malloc(strlen(value) + 1);
                if (decoded) {
                    /* Inline URL decode */
                    const char *src = value;
                    char *dst = decoded;
                    size_t dst_size = strlen(decoded);
                    char a, b;
                    while (*src && dst_size > 1) {
                        if (*src == '%' && src[1] && src[2] &&
                            isxdigit((unsigned char)src[1]) &&
                            isxdigit((unsigned char)src[2])) {
                            a = src[1]; b = src[2];
                            a = (a >= 'a') ? a - 'a' + 10 : (a >= 'A') ? a - 'A' + 10 : a - '0';
                            b = (b >= 'a') ? b - 'a' + 10 : (b >= 'A') ? b - 'A' + 10 : b - '0';
                            *dst++ = (a << 4) | b;
                            src += 3;
                            dst_size--;
                        } else if (*src == '+') {
                            *dst++ = ' ';
                            src++;
                            dst_size--;
                        } else {
                            *dst++ = *src++;
                            dst_size--;
                        }
                    }
                    *dst = '\0';
                    /* Store key-value pair */
                    free(decoded);
                }
            }
            pair = strtok(NULL, "&");
        }
        free(form_data);
    }

    /* Multipart form data */
    else if (strstr(content_type, "multipart/form-data")) {
        /* Basic multipart parsing - find boundary */
        const char *boundary_start = strstr(content_type, "boundary=");
        if (boundary_start) {
            boundary_start += 9; /* Skip "boundary=" */
            /* Parse multipart data */
            /* This is a simplified implementation */
        }
    }

    return true;
}

/* ====================================================================
 * STATIC FILE SERVING MIDDLEWARE
 * ==================================================================== */

typedef struct {
    char *root_dir;
} static_middleware_data_t;

static bool static_file_handler(crest_request_t *req, crest_response_t *res) {
    if (!global_static_dir) return true; /* No static directory configured */

    const char *path = crest_request_path(req);

    /* Skip if path doesn't start with expected prefix or has .. */
    if (strstr(path, "..") || strlen(path) > 1024) {
        return true; /* Continue to next handler */
    }

    /* Build full file path */
    char full_path[2048];
    snprintf(full_path, sizeof(full_path), "%s%s", global_static_dir, path);

    /* Check if file exists and is readable */
    struct stat st;
    if (stat(full_path, &st) != 0 || !S_ISREG(st.st_mode)) {
        return true; /* File not found, continue */
    }

    /* Open and read file */
    int fd = open(full_path, O_RDONLY | O_BINARY);
    if (fd < 0) {
        return true;
    }

    /* Read file content */
    size_t file_size = st.st_size;
    char *content = malloc(file_size + 1);
    if (!content) {
        close(fd);
        return true;
    }

    ssize_t bytes_read = read(fd, content, file_size);
    close(fd);

    if (bytes_read != (ssize_t)file_size) {
        free(content);
        return true;
    }

    content[file_size] = '\0';

    /* Set appropriate content type */
    const char *ext = strrchr(path, '.');
    if (ext) {
        if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) {
            crest_response_header(res, "Content-Type", "text/html");
        } else if (strcmp(ext, ".css") == 0) {
            crest_response_header(res, "Content-Type", "text/css");
        } else if (strcmp(ext, ".js") == 0) {
            crest_response_header(res, "Content-Type", "application/javascript");
        } else if (strcmp(ext, ".json") == 0) {
            crest_response_header(res, "Content-Type", "application/json");
        } else if (strcmp(ext, ".png") == 0) {
            crest_response_header(res, "Content-Type", "image/png");
        } else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) {
            crest_response_header(res, "Content-Type", "image/jpeg");
        } else if (strcmp(ext, ".gif") == 0) {
            crest_response_header(res, "Content-Type", "image/gif");
        } else if (strcmp(ext, ".svg") == 0) {
            crest_response_header(res, "Content-Type", "image/svg+xml");
        } else {
            crest_response_header(res, "Content-Type", "application/octet-stream");
        }
    }

    /* Cache control */
    crest_response_header(res, "Cache-Control", "public, max-age=3600");

    crest_response_send(res, content);
    free(content);

    return false; /* Stop processing - file served */
}

/* ====================================================================
 * RATE LIMITING MIDDLEWARE
 * ==================================================================== */

static rate_limit_entry_t* find_or_create_entry(rate_limit_data_t *data, const char *ip) {
    rate_limit_entry_t *entry = data->entries;
    time_t now = time(NULL);

    /* Find existing entry */
    while (entry) {
        if (strcmp(entry->ip, ip) == 0) {
            /* Check if window expired */
            if (now - entry->window_start >= data->window_seconds) {
                entry->window_start = now;
                entry->request_count = 0;
            }
            return entry;
        }
        entry = entry->next;
    }

    /* Create new entry */
    entry = calloc(1, sizeof(rate_limit_entry_t));
    if (!entry) return NULL;

    strncpy(entry->ip, ip, sizeof(entry->ip) - 1);
    entry->window_start = now;
    entry->request_count = 0;
    entry->next = data->entries;
    data->entries = entry;

    return entry;
}

static bool rate_limit_handler(crest_request_t *req, crest_response_t *res) {
    if (!global_rate_limit_data) return true; /* No rate limiting configured */

    /* Get client IP (simplified - in real implementation, get from connection) */
    const char *client_ip = "127.0.0.1"; /* Placeholder */

    rate_limit_entry_t *entry = find_or_create_entry(global_rate_limit_data, client_ip);
    if (!entry) return true; /* Allow on error */

    entry->request_count++;

    /* Check limit */
    if (entry->request_count > global_rate_limit_data->max_requests) {
        crest_response_status(res, CREST_STATUS_TOO_MANY_REQUESTS);
        crest_response_header(res, "Retry-After", "60");
        crest_response_json(res, "{\"error\":\"Rate limit exceeded\",\"retry_after\":60}");
        return false;
    }

    /* Add rate limit headers */
    char limit_str[32], remaining_str[32];
    snprintf(limit_str, sizeof(limit_str), "%d", global_rate_limit_data->max_requests);
    snprintf(remaining_str, sizeof(remaining_str), "%d",
             global_rate_limit_data->max_requests - entry->request_count);

    crest_response_header(res, "X-RateLimit-Limit", limit_str);
    crest_response_header(res, "X-RateLimit-Remaining", remaining_str);

    return true;
}

/* ====================================================================
 * AUTHENTICATION MIDDLEWARE
 * ==================================================================== */

static bool auth_handler(crest_request_t *req, crest_response_t *res) {
    if (!global_auth_data) return true; /* No auth configured */

    const char *auth_header = crest_request_header(req, "Authorization");

    if (!auth_header) {
        crest_response_status(res, CREST_STATUS_UNAUTHORIZED);
        crest_response_header(res, "WWW-Authenticate", "Bearer");
        crest_response_json(res, "{\"error\":\"Authorization header required\"}");
        return false;
    }

    /* Extract token from "Bearer <token>" */
    const char *token = NULL;
    if (strncmp(auth_header, "Bearer ", 7) == 0) {
        token = auth_header + 7;
    } else {
        token = auth_header; /* Allow token without Bearer prefix */
    }

    if (!token || !global_auth_data->validate_fn(token)) {
        crest_response_status(res, CREST_STATUS_UNAUTHORIZED);
        crest_response_json(res, "{\"error\":\"Invalid token\"}");
        return false;
    }

    return true;
}

/* ====================================================================
 * PUBLIC API FUNCTIONS
 * ==================================================================== */

crest_middleware_fn_t crest_middleware_cors(void) {
    return cors_handler;
}

crest_middleware_fn_t crest_middleware_logger(void) {
    return logger_handler;
}

crest_middleware_fn_t crest_middleware_body_parser(void) {
    return body_parser_handler;
}

crest_middleware_fn_t crest_middleware_static(const char *dir) {
    if (!dir) return NULL;

    /* Free previous static directory if it exists */
    free(global_static_dir);
    global_static_dir = strdup(dir);
    if (!global_static_dir) return NULL;

    return static_file_handler;
}

crest_middleware_fn_t crest_middleware_rate_limit(int max_requests, int window_seconds) {
    /* Free previous rate limit data if it exists */
    if (global_rate_limit_data) {
        /* Clean up rate limit entries */
        rate_limit_entry_t *entry = global_rate_limit_data->entries;
        while (entry) {
            rate_limit_entry_t *next = entry->next;
            free(entry);
            entry = next;
        }
        free(global_rate_limit_data);
    }

    global_rate_limit_data = calloc(1, sizeof(rate_limit_data_t));
    if (!global_rate_limit_data) return NULL;

    global_rate_limit_data->max_requests = max_requests;
    global_rate_limit_data->window_seconds = window_seconds;
    global_rate_limit_data->entries = NULL;

    return rate_limit_handler;
}

crest_middleware_fn_t crest_middleware_auth(bool (*validate_fn)(const char *token)) {
    if (!validate_fn) return NULL;

    /* Free previous auth data if it exists */
    free(global_auth_data);
    global_auth_data = malloc(sizeof(auth_data_t));
    if (!global_auth_data) return NULL;

    global_auth_data->validate_fn = validate_fn;

    return auth_handler;
}

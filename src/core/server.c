/**
 * @file server.c
 * @brief HTTP server implementation for Crest
 */

#include "crest/crest.h"
#include "crest/types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #pragma comment(lib, "ws2_32.lib")
    #define close closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#include "crest/console.h"

/* Internal function declarations */
static int create_socket(const char *host, int port);
static void handle_connection(crest_app_t *app, int client_fd);
static crest_request_t* parse_request(const char *raw_request);
static crest_response_t* create_response(void);
static void send_response(int client_fd, crest_response_t *res);
static void free_request(crest_request_t *req);
static void free_response(crest_response_t *res);
static char* generate_detailed_404_error(crest_app_t *app, crest_request_t *req);

/* Generate detailed 404 error response with suggestions and available routes */
static crest_route_t* find_route(crest_app_t *app, int method, const char *path);
static bool match_route_pattern(const char *pattern, const char *path, crest_request_t *req);

int crest_run(crest_app_t *app, const char *host, int port) {
    if (!app) {
        crest_log(CREST_LOG_ERROR, "Invalid application instance");
        return -1;
    }
    
    #ifdef _WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        crest_log(CREST_LOG_ERROR, "WSAStartup failed");
        return -1;
    }
    #endif
    
    int server_fd = create_socket(host, port);
    if (server_fd < 0) {
        return -1;
    }
    
    app->socket_fd = server_fd;
    app->running = true;
    
    // Display startup banner with automatic colors
    crest_log(CREST_LOG_INFO, "");
    crest_log(CREST_LOG_INFO, "╔════════════════════════════════════════════════════════════╗");
    crest_log(CREST_LOG_INFO, "║                    CREST Framework                        ║");
    crest_log(CREST_LOG_INFO, "║                   Version %s                           ║", CREST_VERSION);
    crest_log(CREST_LOG_INFO, "╠════════════════════════════════════════════════════════════╣");
    crest_log(CREST_LOG_INFO, "║  Server running at: http://%s:%d                          ║", host, port);
    if (app->config->enable_dashboard) {
        crest_log(CREST_LOG_INFO, "║  Dashboard: http://%s:%d%s                              ║", host, port, app->config->dashboard_path);
    }
    crest_log(CREST_LOG_INFO, "║  Routes registered: %-35zu ║", app->route_count);
    crest_log(CREST_LOG_INFO, "╚════════════════════════════════════════════════════════════╝");
    crest_log(CREST_LOG_INFO, "Press Ctrl+C to stop the server");
    
    // Main server loop
    while (app->running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            continue;
        }
        
        handle_connection(app, client_fd);
        close(client_fd);
    }
    
    close(server_fd);
    
    #ifdef _WIN32
    WSACleanup();
    #endif
    
    return 0;
}

/* Generate detailed 404 error response with suggestions and available routes */
static char* generate_detailed_404_error(crest_app_t *app, crest_request_t *req) {
    static char error_json[4096];
    char timestamp[64];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    const char *method_str[] = {"GET", "POST", "PUT", "DELETE", "PATCH", "HEAD", "OPTIONS"};
    
    // Start building JSON response
    snprintf(error_json, sizeof(error_json),
        "{"
        "\"error\":\"Not Found\","
        "\"message\":\"Route not found\","
        "\"details\":{"
        "\"requested_path\":\"%s\","
        "\"requested_method\":\"%s\","
        "\"timestamp\":\"%s\","
        "\"server\":\"Crest/%s\""
        "},"
        "\"suggestions\":["
        "\"Check if the URL is correct and properly encoded\","
        "\"Ensure you're using the correct HTTP method (GET, POST, etc.)\","
        "\"Verify that the endpoint exists in your application\","
        "\"Check for trailing slashes or case sensitivity issues\""
        "],"
        "\"available_routes\":[",
        req->path,
        method_str[req->method],
        timestamp,
        CREST_VERSION);
    
    // Add available routes (limit to first 10)
    size_t routes_to_show = app->route_count > 10 ? 10 : app->route_count;
    for (size_t i = 0; i < routes_to_show; i++) {
        char route_entry[256];
        if (app->routes[i].description) {
            snprintf(route_entry, sizeof(route_entry),
                "%s{\"method\":\"%s\",\"path\":\"%s\",\"description\":\"%s\"}",
                i > 0 ? "," : "",
                method_str[app->routes[i].method],
                app->routes[i].path,
                app->routes[i].description);
        } else {
            snprintf(route_entry, sizeof(route_entry),
                "%s{\"method\":\"%s\",\"path\":\"%s\"}",
                i > 0 ? "," : "",
                method_str[app->routes[i].method],
                app->routes[i].path);
        }
        strcat(error_json, route_entry);
    }
    
    // Add warnings
    strcat(error_json, "],\"warnings\":[");
    strcat(error_json, 
        "\"This endpoint does not exist in the application\","
        "\"Consider checking the API documentation or dashboard\"");
    
    if (app->route_count == 0) {
        strcat(error_json, ",\"No routes have been registered with the application\"");
    }
    
    // Close JSON
    strcat(error_json, "]}");
    
    return error_json;
}

static int create_socket(const char *host, int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("[CREST] socket");
        return -1;
    }
    
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) < 0) {
        perror("[CREST] setsockopt");
        close(server_fd);
        return -1;
    }
    
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    
    if (inet_pton(AF_INET, host, &address.sin_addr) <= 0) {
        address.sin_addr.s_addr = INADDR_ANY;
    }
    
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("[CREST] bind");
        close(server_fd);
        return -1;
    }
    
    if (listen(server_fd, 128) < 0) {
        perror("[CREST] listen");
        close(server_fd);
        return -1;
    }
    
    return server_fd;
}

static void handle_connection(crest_app_t *app, int client_fd) {
    char buffer[8192];
    int bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    
    if (bytes_read <= 0) {
        return;
    }
    
    buffer[bytes_read] = '\0';
    
    crest_request_t *req = parse_request(buffer);
    if (!req) {
        return;
    }
    
    /* Set user_data to app so handlers can access it */
    req->user_data = app;
    
    crest_response_t *res = create_response();
    
    // Run middleware
    bool middleware_passed = true;
    for (size_t i = 0; i < app->middleware_count; i++) {
        if (!app->middleware[i].handler(req, res)) {
            middleware_passed = false;
            break;
        }
    }
    
    if (middleware_passed) {
        // Find and execute route handler
        crest_route_t *route = find_route(app, req->method, req->path);
        
        if (route) {
            /* Extract path parameters if this is a pattern route */
            if (route->is_pattern) {
                match_route_pattern(route->path, req->path, req);
            }
            route->handler(req, res);
        } else {
            crest_response_status(res, CREST_STATUS_NOT_FOUND);
            char *detailed_error = generate_detailed_404_error(app, req);
            crest_response_json(res, detailed_error);
        }
    }
    
    // Set default status if not set
    if (res->status_code == 0) {
        res->status_code = CREST_STATUS_OK;
    }
    
    send_response(client_fd, res);
    
    free_request(req);
    free_response(res);
}

static crest_request_t* parse_request(const char *raw_request) {
    crest_request_t *req = (crest_request_t*)calloc(1, sizeof(crest_request_t));
    if (!req) {
        return NULL;
    }
    
    // Parse request line: METHOD /path HTTP/1.1
    char method[16], path[512], version[16];
    if (sscanf(raw_request, "%15s %511s %15s", method, path, version) != 3) {
        free(req);
        return NULL;
    }
    
    // Parse method
    if (strcmp(method, "GET") == 0) req->method = CREST_GET;
    else if (strcmp(method, "POST") == 0) req->method = CREST_POST;
    else if (strcmp(method, "PUT") == 0) req->method = CREST_PUT;
    else if (strcmp(method, "DELETE") == 0) req->method = CREST_DELETE;
    else if (strcmp(method, "PATCH") == 0) req->method = CREST_PATCH;
    else if (strcmp(method, "HEAD") == 0) req->method = CREST_HEAD;
    else if (strcmp(method, "OPTIONS") == 0) req->method = CREST_OPTIONS;
    
    // Split path and query string
    char *query_start = strchr(path, '?');
    if (query_start) {
        *query_start = '\0';
        query_start++;
        
        // Parse query parameters
        char *query_copy = strdup(query_start);
        char *pair = strtok(query_copy, "&");
        while (pair && req->query_count < CREST_MAX_QUERY_PARAMS) {
            char *eq = strchr(pair, '=');
            if (eq) {
                *eq = '\0';
                req->query_params[req->query_count].key = strdup(pair);
                req->query_params[req->query_count].value = strdup(eq + 1);
                req->query_count++;
            }
            pair = strtok(NULL, "&");
        }
        free(query_copy);
    }
    
    req->path = strdup(path);
    
    // Parse headers
    const char *header_start = strchr(raw_request, '\n');
    if (header_start) {
        header_start++;
        char *headers_copy = strdup(header_start);
        char *line = strtok(headers_copy, "\r\n");
        
        while (line && req->header_count < CREST_MAX_HEADERS) {
            if (strlen(line) == 0) {
                break; // End of headers
            }
            
            char *colon = strchr(line, ':');
            if (colon) {
                *colon = '\0';
                char *value = colon + 1;
                while (*value == ' ') value++; // Skip leading spaces
                
                req->headers[req->header_count].key = strdup(line);
                req->headers[req->header_count].value = strdup(value);
                req->header_count++;
            }
            
            line = strtok(NULL, "\r\n");
        }
        free(headers_copy);
    }
    
    // Parse body (if present)
    const char *body_start = strstr(raw_request, "\r\n\r\n");
    if (body_start) {
        body_start += 4;
        if (strlen(body_start) > 0) {
            req->body = strdup(body_start);
            req->body_len = strlen(body_start);
        }
    }
    
    return req;
}

static crest_response_t* create_response(void) {
    crest_response_t *res = (crest_response_t*)calloc(1, sizeof(crest_response_t));
    return res;
}

static void send_response(int client_fd, crest_response_t *res) {
    if (!res) {
        return;
    }
    
    // Status line
    char status_line[128];
    const char *status_text = "OK";
    
    switch (res->status_code) {
        case 200: status_text = "OK"; break;
        case 201: status_text = "Created"; break;
        case 204: status_text = "No Content"; break;
        case 400: status_text = "Bad Request"; break;
        case 401: status_text = "Unauthorized"; break;
        case 403: status_text = "Forbidden"; break;
        case 404: status_text = "Not Found"; break;
        case 500: status_text = "Internal Server Error"; break;
    }
    
    snprintf(status_line, sizeof(status_line), "HTTP/1.1 %d %s\r\n", res->status_code, status_text);
    send(client_fd, status_line, strlen(status_line), 0);
    
    // Headers
    for (size_t i = 0; i < res->header_count; i++) {
        char header_line[512];
        snprintf(header_line, sizeof(header_line), "%s: %s\r\n", 
                res->headers[i].key, res->headers[i].value);
        send(client_fd, header_line, strlen(header_line), 0);
    }
    
    // Content-Length
    if (res->body) {
        char content_length[64];
        snprintf(content_length, sizeof(content_length), "Content-Length: %zu\r\n", res->body_len);
        send(client_fd, content_length, strlen(content_length), 0);
    }
    
    send(client_fd, "\r\n", 2, 0);
    
    // Body
    if (res->body) {
        send(client_fd, res->body, res->body_len, 0);
    }
}

static void free_request(crest_request_t *req) {
    if (!req) {
        return;
    }
    
    free(req->path);
    free(req->body);
    
    for (size_t i = 0; i < req->header_count; i++) {
        free(req->headers[i].key);
        free(req->headers[i].value);
    }
    
    for (size_t i = 0; i < req->query_count; i++) {
        free(req->query_params[i].key);
        free(req->query_params[i].value);
    }
    
    for (size_t i = 0; i < req->param_count; i++) {
        free(req->path_params[i].key);
        free(req->path_params[i].value);
    }
    
    free(req);
}

static void free_response(crest_response_t *res) {
    if (!res) {
        return;
    }
    
    free(res->body);
    
    for (size_t i = 0; i < res->header_count; i++) {
        free(res->headers[i].key);
        free(res->headers[i].value);
    }
    
    free(res);
}

static crest_route_t* find_route(crest_app_t *app, int method, const char *path) {
    for (size_t i = 0; i < app->route_count; i++) {
        if (app->routes[i].method == method) {
            if (app->routes[i].is_pattern) {
                /* Use pattern matching for routes with parameters or wildcards */
                crest_request_t temp_req = {0};
                if (match_route_pattern(app->routes[i].path, path, &temp_req)) {
                    /* Note: This is a match check only. 
                     * Actual param extraction happens during request processing */
                    return &app->routes[i];
                }
            } else {
                if (strcmp(app->routes[i].path, path) == 0) {
                    return &app->routes[i];
                }
            }
        }
    }
    
    return NULL;
}

static bool match_route_pattern(const char *pattern, const char *path, crest_request_t *req) {
    const char *p = pattern;
    const char *u = path;
    
    while (*p && *u) {
        if (*p == ':') {
            /* Path parameter like :id */
            p++;
            const char *param_start = p;
            
            /* Find end of parameter name */
            while (*p && *p != '/') {
                p++;
            }
            
            /* Extract parameter name */
            size_t param_len = p - param_start;
            if (param_len == 0) {
                return false; /* Invalid pattern */
            }
            
            char *param_name = (char*)malloc(param_len + 1);
            if (!param_name) {
                return false;
            }
            memcpy(param_name, param_start, param_len);
            param_name[param_len] = '\0';
            
            /* Extract parameter value from path */
            const char *value_start = u;
            while (*u && *u != '/') {
                u++;
            }
            
            size_t value_len = u - value_start;
            if (value_len == 0) {
                free(param_name);
                return false; /* Empty parameter value */
            }
            
            char *param_value = (char*)malloc(value_len + 1);
            if (!param_value) {
                free(param_name);
                return false;
            }
            memcpy(param_value, value_start, value_len);
            param_value[value_len] = '\0';
            
            /* Add to request path params */
            if (req && req->param_count < CREST_MAX_PARAMS) {
                req->path_params[req->param_count].key = param_name;
                req->path_params[req->param_count].value = param_value;
                req->param_count++;
            } else {
                free(param_name);
                free(param_value);
            }
        } else if (*p == '*') {
            /* Wildcard - matches everything after this point */
            return true;
        } else if (*p == *u) {
            /* Exact match */
            p++;
            u++;
        } else {
            /* Mismatch */
            return false;
        }
    }
    
    /* Both must be at end for successful match */
    return (*p == '\0' && *u == '\0');
}

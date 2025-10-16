/**
 * @file openapi.c
 * @brief OpenAPI 3.0 specification generator implementation
 * @details Generates production-ready OpenAPI 3.0 JSON from registered routes
 */

#include "crest/openapi.h"
#include "crest/crest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Initial buffer size for OpenAPI spec (will grow if needed) */
#define INITIAL_SPEC_SIZE 16384
#define SPEC_GROW_SIZE 8192

/* Helper to append string to dynamic buffer */
static int append_to_spec(char **spec, size_t *size, size_t *capacity, const char *text) {
    size_t text_len = strlen(text);
    
    while (*size + text_len + 1 > *capacity) {
        *capacity += SPEC_GROW_SIZE;
        char *new_spec = (char*)realloc(*spec, *capacity);
        if (!new_spec) {
            return -1;
        }
        *spec = new_spec;
    }
    
    strcpy(*spec + *size, text);
    *size += text_len;
    return 0;
}

/* Helper to escape JSON strings */
static char* escape_json_string(const char *str) {
    if (!str) return strdup("");
    
    size_t len = strlen(str);
    char *escaped = (char*)malloc(len * 2 + 1); /* Worst case: all chars escaped */
    if (!escaped) return NULL;
    
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        switch (str[i]) {
            case '"':  escaped[j++] = '\\'; escaped[j++] = '"'; break;
            case '\\': escaped[j++] = '\\'; escaped[j++] = '\\'; break;
            case '\n': escaped[j++] = '\\'; escaped[j++] = 'n'; break;
            case '\r': escaped[j++] = '\\'; escaped[j++] = 'r'; break;
            case '\t': escaped[j++] = '\\'; escaped[j++] = 't'; break;
            default:   escaped[j++] = str[i]; break;
        }
    }
    escaped[j] = '\0';
    return escaped;
}

/* Get HTTP method name as string */
static const char* get_method_name(crest_method_t method) {
    switch (method) {
        case CREST_GET:     return "get";
        case CREST_POST:    return "post";
        case CREST_PUT:     return "put";
        case CREST_DELETE:  return "delete";
        case CREST_PATCH:   return "patch";
        case CREST_HEAD:    return "head";
        case CREST_OPTIONS: return "options";
        default:            return "get";
    }
}

/* Generate parameter object for path parameters */
static char* generate_path_parameters(const char *path) {
    char *params = (char*)malloc(4096);
    if (!params) return NULL;
    
    params[0] = '\0';
    const char *p = path;
    int param_count = 0;
    
    while ((p = strchr(p, ':')) != NULL) {
        p++; /* Skip ':' */
        const char *end = p;
        while (*end && *end != '/' && *end != '.') end++;
        
        size_t name_len = end - p;
        if (name_len > 0) {
            char param_name[256];
            snprintf(param_name, sizeof(param_name), "%.*s", (int)name_len, p);
            
            if (param_count > 0) strcat(params, ",");
            
            char param_json[512];
            snprintf(param_json, sizeof(param_json),
                "{\"name\":\"%s\",\"in\":\"path\",\"required\":true,\"schema\":{\"type\":\"string\"},"
                "\"description\":\"Path parameter %s\"}",
                param_name, param_name);
            
            strcat(params, param_json);
            param_count++;
        }
        p = end;
    }
    
    return params;
}

/* Generate request body schema for POST/PUT/PATCH */
static const char* generate_request_body(crest_method_t method) {
    if (method != CREST_POST && method != CREST_PUT && method != CREST_PATCH) {
        return "";
    }
    
    return "\"requestBody\":{"
           "\"required\":true,"
           "\"content\":{"
           "\"application/json\":{"
           "\"schema\":{"
           "\"type\":\"object\","
           "\"properties\":{"
           "\"data\":{\"type\":\"object\",\"description\":\"Request payload\"}"
           "},"
           "\"example\":{\"data\":{\"key\":\"value\"}}"
           "}}}}";
}

/* Generate response schemas */
static const char* generate_responses(crest_method_t method) {
    const char *success_code = (method == CREST_POST) ? "201" : "200";
    
    static char responses[2048];
    snprintf(responses, sizeof(responses),
        "\"responses\":{"
        "\"%s\":{\"description\":\"Successful operation\","
        "\"content\":{\"application/json\":{"
        "\"schema\":{\"type\":\"object\","
        "\"properties\":{"
        "\"status\":{\"type\":\"string\",\"example\":\"success\"},"
        "\"data\":{\"type\":\"object\"}"
        "}}}}},"
        "\"400\":{\"description\":\"Bad request\","
        "\"content\":{\"application/json\":{"
        "\"schema\":{\"type\":\"object\","
        "\"properties\":{"
        "\"error\":{\"type\":\"string\",\"example\":\"Invalid request\"}"
        "}}}}},"
        "\"404\":{\"description\":\"Not found\","
        "\"content\":{\"application/json\":{"
        "\"schema\":{\"type\":\"object\","
        "\"properties\":{"
        "\"error\":{\"type\":\"string\",\"example\":\"Resource not found\"}"
        "}}}}},"
        "\"500\":{\"description\":\"Internal server error\","
        "\"content\":{\"application/json\":{"
        "\"schema\":{\"type\":\"object\","
        "\"properties\":{"
        "\"error\":{\"type\":\"string\",\"example\":\"Server error\"}"
        "}}}}}"
        "}",
        success_code);
    
    return responses;
}

char* crest_generate_openapi_spec(crest_app_t *app) {
    if (!app) return NULL;
    
    size_t capacity = INITIAL_SPEC_SIZE;
    size_t size = 0;
    char *spec = (char*)malloc(capacity);
    if (!spec) return NULL;
    
    spec[0] = '\0';
    
    /* OpenAPI header */
    const char *header = 
        "{\n"
        "  \"openapi\": \"3.0.3\",\n"
        "  \"info\": {\n"
        "    \"title\": \"Crest API\",\n"
        "    \"description\": \"High-performance REST API built with Crest Framework - A modern C/C++ web framework\",\n"
        "    \"version\": \"" CREST_VERSION "\",\n"
        "    \"contact\": {\n"
        "      \"name\": \"Crest Framework\",\n"
        "      \"url\": \"https://github.com/crest-framework\"\n"
        "    },\n"
        "    \"license\": {\n"
        "      \"name\": \"MIT\",\n"
        "      \"url\": \"https://opensource.org/licenses/MIT\"\n"
        "    }\n"
        "  },\n"
        "  \"servers\": [\n"
        "    {\n"
        "      \"url\": \"http://localhost:8000\",\n"
        "      \"description\": \"Local development server\"\n"
        "    },\n"
        "    {\n"
        "      \"url\": \"http://127.0.0.1:8000\",\n"
        "      \"description\": \"Local loopback server\"\n"
        "    }\n"
        "  ],\n"
        "  \"tags\": [\n"
        "    {\n"
        "      \"name\": \"API\",\n"
        "      \"description\": \"Application endpoints\"\n"
        "    },\n"
        "    {\n"
        "      \"name\": \"Dashboard\",\n"
        "      \"description\": \"API documentation and monitoring (reserved routes)\"\n"
        "    }\n"
        "  ],\n"
        "  \"paths\": {\n";
    
    if (append_to_spec(&spec, &size, &capacity, header) != 0) {
        free(spec);
        return NULL;
    }
    
    /* Reserved dashboard routes first */
    const char *dashboard_routes =
        "    \"/docs\": {\n"
        "      \"get\": {\n"
        "        \"tags\": [\"Dashboard\"],\n"
        "        \"summary\": \"Swagger UI - Interactive API Documentation\",\n"
        "        \"description\": \"Full-featured Swagger UI dashboard for testing all API endpoints interactively\",\n"
        "        \"operationId\": \"getSwaggerUI\",\n"
        "        \"responses\": {\n"
        "          \"200\": {\n"
        "            \"description\": \"Swagger UI HTML page\",\n"
        "            \"content\": {\"text/html\": {}}\n"
        "          }\n"
        "        }\n"
        "      }\n"
        "    },\n"
        "    \"/redoc\": {\n"
        "      \"get\": {\n"
        "        \"tags\": [\"Dashboard\"],\n"
        "        \"summary\": \"ReDoc UI - Read-Only API Documentation\",\n"
        "        \"description\": \"Clean, professional API documentation interface (read-only, not for testing)\",\n"
        "        \"operationId\": \"getReDocUI\",\n"
        "        \"responses\": {\n"
        "          \"200\": {\n"
        "            \"description\": \"ReDoc UI HTML page\",\n"
        "            \"content\": {\"text/html\": {}}\n"
        "          }\n"
        "        }\n"
        "      }\n"
        "    },\n"
        "    \"/openapi.json\": {\n"
        "      \"get\": {\n"
        "        \"tags\": [\"Dashboard\"],\n"
        "        \"summary\": \"OpenAPI Specification (JSON)\",\n"
        "        \"description\": \"Machine-readable OpenAPI 3.0 specification for this API\",\n"
        "        \"operationId\": \"getOpenAPISpec\",\n"
        "        \"responses\": {\n"
        "          \"200\": {\n"
        "            \"description\": \"OpenAPI 3.0 JSON specification\",\n"
        "            \"content\": {\"application/json\": {}}\n"
        "          }\n"
        "        }\n"
        "      }\n"
        "    }";
    
    if (append_to_spec(&spec, &size, &capacity, dashboard_routes) != 0) {
        free(spec);
        return NULL;
    }
    
    /* Count non-reserved routes */
    size_t user_route_count = 0;
    for (size_t i = 0; i < app->route_count; i++) {
        crest_route_t *route = &app->routes[i];
        if (strcmp(route->path, "/docs") != 0 && 
            strcmp(route->path, "/redoc") != 0 && 
            strcmp(route->path, "/openapi.json") != 0 &&
            strcmp(route->path, "/dashboard") != 0 &&
            strcmp(route->path, "/api/routes") != 0) {
            user_route_count++;
        }
    }
    
    /* Group routes by path first */
    typedef struct {
        const char *path;
        crest_route_t *routes[10]; /* Max 10 methods per path */
        size_t method_count;
    } path_group_t;
    
    path_group_t path_groups[100]; /* Max 100 unique paths */
    size_t path_count = 0;
    
    /* Collect all user routes and group by path */
    for (size_t i = 0; i < app->route_count; i++) {
        crest_route_t *route = &app->routes[i];
        
        /* Skip reserved dashboard routes */
        if (strcmp(route->path, "/docs") == 0 || 
            strcmp(route->path, "/redoc") == 0 || 
            strcmp(route->path, "/openapi.json") == 0 ||
            strcmp(route->path, "/dashboard") == 0 ||
            strcmp(route->path, "/api/routes") == 0) {
            continue;
        }
        
        /* Find existing path group or create new one */
        size_t group_idx = path_count;
        for (size_t j = 0; j < path_count; j++) {
            if (strcmp(path_groups[j].path, route->path) == 0) {
                group_idx = j;
                break;
            }
        }
        
        if (group_idx == path_count) {
            /* New path group */
            path_groups[path_count].path = route->path;
            path_groups[path_count].method_count = 0;
            path_count++;
        }
        
        /* Add route to path group */
        if (path_groups[group_idx].method_count < 10) {
            path_groups[group_idx].routes[path_groups[group_idx].method_count++] = route;
        }
    }
    
    /* Generate paths for grouped routes */
    for (size_t group_idx = 0; group_idx < path_count; group_idx++) {
        path_group_t *group = &path_groups[group_idx];
        
        char *escaped_path = escape_json_string(group->path);
        if (!escaped_path) continue;
        
        /* Start path object */
        char path_json[32768] = {0};
        snprintf(path_json, sizeof(path_json), ",\n    \"%s\": {", escaped_path);
        
        /* Generate method objects for this path */
        for (size_t method_idx = 0; method_idx < group->method_count; method_idx++) {
            crest_route_t *route = group->routes[method_idx];
            
            char *escaped_desc = route->description ? escape_json_string(route->description) : strdup("API endpoint");
            char *path_params = generate_path_parameters(route->path);
            const char *req_body = generate_request_body(route->method);
            
            if (!escaped_desc || !path_params) {
                free(escaped_desc);
                free(path_params);
                continue;
            }
            
            char method_json[8192];
            int has_params = strlen(path_params) > 0;
            int has_reqbody = strlen(req_body) > 0;
            
            if (has_params && has_reqbody) {
                snprintf(method_json, sizeof(method_json),
                    "\n      \"%s\": {\n"
                    "        \"tags\": [\"API\"],\n"
                    "        \"summary\": \"%s\",\n"
                    "        \"description\": \"%s\",\n"
                    "        \"operationId\": \"%s_%s\",\n"
                    "        \"parameters\": [%s],\n"
                    "        %s,\n"
                    "        %s\n"
                    "      }",
                    get_method_name(route->method),
                    escaped_desc, escaped_desc,
                    get_method_name(route->method), escaped_path,
                    path_params,
                    req_body,
                    generate_responses(route->method));
            } else if (has_reqbody) {
                snprintf(method_json, sizeof(method_json),
                    "\n      \"%s\": {\n"
                    "        \"tags\": [\"API\"],\n"
                    "        \"summary\": \"%s\",\n"
                    "        \"description\": \"%s\",\n"
                    "        \"operationId\": \"%s_%s\",\n"
                    "        %s,\n"
                    "        %s\n"
                    "      }",
                    get_method_name(route->method),
                    escaped_desc, escaped_desc,
                    get_method_name(route->method), escaped_path,
                    req_body,
                    generate_responses(route->method));
            } else if (has_params) {
                snprintf(method_json, sizeof(method_json),
                    "\n      \"%s\": {\n"
                    "        \"tags\": [\"API\"],\n"
                    "        \"summary\": \"%s\",\n"
                    "        \"description\": \"%s\",\n"
                    "        \"operationId\": \"%s_%s\",\n"
                    "        \"parameters\": [%s],\n"
                    "        %s\n"
                    "      }",
                    get_method_name(route->method),
                    escaped_desc, escaped_desc,
                    get_method_name(route->method), escaped_path,
                    path_params,
                    generate_responses(route->method));
            } else {
                snprintf(method_json, sizeof(method_json),
                    "\n      \"%s\": {\n"
                    "        \"tags\": [\"API\"],\n"
                    "        \"summary\": \"%s\",\n"
                    "        \"description\": \"%s\",\n"
                    "        \"operationId\": \"%s_%s\",\n"
                    "        %s\n"
                    "      }",
                    get_method_name(route->method),
                    escaped_desc, escaped_desc,
                    get_method_name(route->method), escaped_path,
                    generate_responses(route->method));
            }
            
            /* Append method to path object */
            if (method_idx > 0) {
                strncat(path_json, ",", sizeof(path_json) - strlen(path_json) - 1);
            }
            strncat(path_json, method_json, sizeof(path_json) - strlen(path_json) - 1);
            
            free(escaped_desc);
            free(path_params);
        }
        
        /* Close path object */
        strncat(path_json, "\n    }", sizeof(path_json) - strlen(path_json) - 1);
        
        /* Append to spec */
        if (append_to_spec(&spec, &size, &capacity, path_json) != 0) {
            free(escaped_path);
            free(spec);
            return NULL;
        }
        
        free(escaped_path);
    }
    
    /* Close paths and add components */
    const char *footer = 
        "\n  },\n"
        "  \"components\": {\n"
        "    \"schemas\": {\n"
        "      \"Error\": {\n"
        "        \"type\": \"object\",\n"
        "        \"properties\": {\n"
        "          \"error\": {\n"
        "            \"type\": \"string\",\n"
        "            \"description\": \"Error message\"\n"
        "          },\n"
        "          \"code\": {\n"
        "            \"type\": \"integer\",\n"
        "            \"description\": \"Error code\"\n"
        "          }\n"
        "        },\n"
        "        \"required\": [\"error\"]\n"
        "      },\n"
        "      \"Success\": {\n"
        "        \"type\": \"object\",\n"
        "        \"properties\": {\n"
        "          \"status\": {\n"
        "            \"type\": \"string\",\n"
        "            \"example\": \"success\"\n"
        "          },\n"
        "          \"data\": {\n"
        "            \"type\": \"object\",\n"
        "            \"description\": \"Response data\"\n"
        "          }\n"
        "        }\n"
        "      }\n"
        "    },\n"
        "    \"securitySchemes\": {\n"
        "      \"bearerAuth\": {\n"
        "        \"type\": \"http\",\n"
        "        \"scheme\": \"bearer\",\n"
        "        \"bearerFormat\": \"JWT\",\n"
        "        \"description\": \"JWT Bearer token authentication\"\n"
        "      },\n"
        "      \"apiKey\": {\n"
        "        \"type\": \"apiKey\",\n"
        "        \"in\": \"header\",\n"
        "        \"name\": \"X-API-Key\",\n"
        "        \"description\": \"API key authentication\"\n"
        "      }\n"
        "    }\n"
        "  }\n"
        "}\n";
    
    if (append_to_spec(&spec, &size, &capacity, footer) != 0) {
        free(spec);
        return NULL;
    }
    
    return spec;
}

void crest_free_openapi_spec(char *spec) {
    if (spec) {
        free(spec);
    }
}

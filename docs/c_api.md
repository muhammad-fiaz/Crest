# Crest C API Documentation

Complete reference for using Crest in C projects.

## Table of Contents

- [Getting Started](#getting-started)
- [Core Functions](#core-functions)
- [Request Handling](#request-handling)
- [Response Functions](#response-functions)
- [Configuration](#configuration)
- [Examples](#examples)

## Getting Started

Include the Crest header in your C project:

```c
#include "crest/crest.h"
```

## Core Functions

### crest_create

Create a new Crest application instance.

```c
crest_app_t* crest_create(void);
```

**Returns:** Pointer to the created application, or NULL on failure.

**Example:**
```c
crest_app_t* app = crest_create();
if (!app) {
    fprintf(stderr, "Failed to create app\n");
    return 1;
}
```

### crest_create_with_config

Create a new Crest application with custom configuration.

```c
crest_app_t* crest_create_with_config(crest_config_t* config);
```

**Parameters:**
- `config`: Configuration structure

**Returns:** Pointer to the created application, or NULL on failure.

**Example:**
```c
crest_config_t config = {
    .title = "My API",
    .description = "API Description",
    .version = "1.0.0",
    .docs_enabled = true
};

crest_app_t* app = crest_create_with_config(&config);
```

### crest_destroy

Destroy a Crest application and free all resources.

```c
void crest_destroy(crest_app_t* app);
```

**Parameters:**
- `app`: Application instance to destroy

**Example:**
```c
crest_destroy(app);
```

### crest_route

Register a route handler.

```c
int crest_route(crest_app_t* app, crest_method_t method, const char* path,
                crest_handler_t handler, const char* description);
```

**Parameters:**
- `app`: Application instance
- `method`: HTTP method (CREST_GET, CREST_POST, etc.)
- `path`: Route path
- `handler`: Handler function
- `description`: Route description for documentation

**Returns:** 0 on success, -1 on error (e.g., duplicate route)

**Example:**
```c
void my_handler(crest_request_t* req, crest_response_t* res) {
    crest_response_json(res, 200, "{\"status\":\"ok\"}");
}

crest_route(app, CREST_GET, "/api/status", my_handler, "Get API status");
```

### crest_run

Start the HTTP server.

```c
int crest_run(crest_app_t* app, const char* host, int port);
```

**Parameters:**
- `app`: Application instance
- `host`: Host address (e.g., "0.0.0.0", "127.0.0.1")
- `port`: Port number

**Returns:** 0 on success, -1 on error

**Example:**
```c
int result = crest_run(app, "0.0.0.0", 8000);
if (result != 0) {
    fprintf(stderr, "Failed to start server\n");
}
```

### crest_stop

Stop the HTTP server.

```c
void crest_stop(crest_app_t* app);
```

**Parameters:**
- `app`: Application instance

## Request Handling

### crest_request_get_path

Get the request path.

```c
const char* crest_request_get_path(crest_request_t* req);
```

**Parameters:**
- `req`: Request object

**Returns:** Path string, or NULL if unavailable

### crest_request_get_method

Get the request HTTP method.

```c
const char* crest_request_get_method(crest_request_t* req);
```

**Parameters:**
- `req`: Request object

**Returns:** Method string (e.g., "GET", "POST"), or NULL if unavailable

### crest_request_get_body

Get the request body.

```c
const char* crest_request_get_body(crest_request_t* req);
```

**Parameters:**
- `req`: Request object

**Returns:** Body string, or NULL if unavailable

### crest_request_get_query

Get a query parameter value.

```c
const char* crest_request_get_query(crest_request_t* req, const char* key);
```

**Parameters:**
- `req`: Request object
- `key`: Query parameter key

**Returns:** Parameter value, or NULL if not found

### crest_request_get_header

Get a header value.

```c
const char* crest_request_get_header(crest_request_t* req, const char* key);
```

**Parameters:**
- `req`: Request object
- `key`: Header key

**Returns:** Header value, or NULL if not found

## Response Functions

### crest_response_json

Send a JSON response.

```c
void crest_response_json(crest_response_t* res, int status, const char* json);
```

**Parameters:**
- `res`: Response object
- `status`: HTTP status code
- `json`: JSON string

**Example:**
```c
crest_response_json(res, 200, "{\"message\":\"Success\"}");
```

### crest_response_text

Send a plain text response.

```c
void crest_response_text(crest_response_t* res, int status, const char* text);
```

**Parameters:**
- `res`: Response object
- `status`: HTTP status code
- `text`: Text content

**Example:**
```c
crest_response_text(res, 200, "Hello, World!");
```

### crest_response_html

Send an HTML response.

```c
void crest_response_html(crest_response_t* res, int status, const char* html);
```

**Parameters:**
- `res`: Response object
- `status`: HTTP status code
- `html`: HTML content

**Example:**
```c
crest_response_html(res, 200, "<h1>Welcome</h1>");
```

### crest_response_set_header

Set a response header.

```c
void crest_response_set_header(crest_response_t* res, const char* key, const char* value);
```

**Parameters:**
- `res`: Response object
- `key`: Header key
- `value`: Header value

## Configuration

### crest_set_docs_enabled

Enable or disable Swagger documentation.

```c
void crest_set_docs_enabled(crest_app_t* app, bool enabled);
```

**Parameters:**
- `app`: Application instance
- `enabled`: true to enable, false to disable

**Example:**
```c
crest_set_docs_enabled(app, false); // Disable in production
```

### crest_set_title

Set the application title for documentation.

```c
void crest_set_title(crest_app_t* app, const char* title);
```

**Parameters:**
- `app`: Application instance
- `title`: Application title

### crest_set_description

Set the application description for documentation.

```c
void crest_set_description(crest_app_t* app, const char* description);
```

**Parameters:**
- `app`: Application instance
- `description`: Application description

### crest_set_proxy

Configure proxy settings.

```c
void crest_set_proxy(crest_app_t* app, const char* proxy_url);
```

**Parameters:**
- `app`: Application instance
- `proxy_url`: Proxy URL

## HTTP Methods

```c
typedef enum {
    CREST_GET,
    CREST_POST,
    CREST_PUT,
    CREST_DELETE,
    CREST_PATCH,
    CREST_HEAD,
    CREST_OPTIONS
} crest_method_t;
```

## HTTP Status Codes

```c
typedef enum {
    CREST_STATUS_OK = 200,
    CREST_STATUS_CREATED = 201,
    CREST_STATUS_BAD_REQUEST = 400,
    CREST_STATUS_NOT_FOUND = 404,
    CREST_STATUS_INTERNAL_ERROR = 500
} crest_status_t;
```

## Examples

### Complete Example

```c
#include "crest/crest.h"
#include <stdio.h>

void handle_root(crest_request_t* req, crest_response_t* res) {
    crest_response_json(res, CREST_STATUS_OK, 
        "{\"message\":\"Welcome to my API\"}");
}

void handle_users(crest_request_t* req, crest_response_t* res) {
    const char* body = crest_request_get_body(req);
    crest_response_json(res, CREST_STATUS_CREATED, 
        "{\"message\":\"User created\"}");
}

int main(void) {
    crest_config_t config = {
        .title = "My C API",
        .description = "RESTful API in C",
        .version = "1.0.0",
        .docs_enabled = true
    };
    
    crest_app_t* app = crest_create_with_config(&config);
    if (!app) {
        return 1;
    }
    
    crest_route(app, CREST_GET, "/", handle_root, "Root endpoint");
    crest_route(app, CREST_POST, "/users", handle_users, "Create user");
    
    printf("Starting server on http://127.0.0.1:8000\n");
    crest_run(app, "127.0.0.1", 8000);
    
    crest_destroy(app);
    return 0;
}
```

## Best Practices

1. **Always check return values** from crest_create and crest_route
2. **Clean up resources** with crest_destroy when done
3. **Use descriptive route descriptions** for better documentation
4. **Disable docs in production** for security
5. **Handle errors gracefully** in your handlers
6. **Validate input** in your handlers before processing

## Thread Safety

Crest handles concurrent requests automatically. Each request is processed in a separate thread, so your handlers should be thread-safe if they access shared resources.

## Memory Management

- Crest manages memory for request and response objects
- You are responsible for managing memory in your handler functions
- Always call crest_destroy to free application resources

---
layout: default
title: API Reference
nav_order: 3
---

# API Reference
{: .no_toc }

Comprehensive reference for all Crest functions, types, and constants
{: .fs-6 .fw-300 }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Core Functions

### Application Lifecycle

#### crest_create

```c
crest_app_t* crest_create(void);
```

Creates a new Crest application instance with default settings.

**Returns:** 
- Pointer to `crest_app_t` application structure
- `NULL` on allocation failure

**Default Configuration:**
- Host: `127.0.0.1` (localhost)
- Port: `3000`
- Timeout: `60` seconds
- Max body size: `10 MB`
- Dashboard: disabled
- CORS: disabled
- Logging: enabled (INFO level)

**Example:**
```c
crest_app_t *app = crest_create();
if (!app) {
    fprintf(stderr, "Failed to create application\n");
    return 1;
}

// Configure and use app...

crest_destroy(app);
```

**Memory:** Always call `crest_destroy()` to free resources.

---

#### crest_create_with_config

```c
crest_app_t* crest_create_with_config(crest_config_t *config);
```

Creates a Crest application with custom configuration.

**Parameters:**
- `config` - Pointer to `crest_config_t` structure with custom settings

**Returns:** Application instance, or `NULL` on error

**Example:**
```c
crest_config_t config = {
    .host = "0.0.0.0",
    .port = 8080,
    .enable_dashboard = true,
    .enable_cors = true,
    .max_body_size = 50 * 1024 * 1024,  // 50 MB
    .timeout = 30
};

crest_app_t *app = crest_create_with_config(&config);
```

**See:** [Configuration Options](#configuration-options)

**Returns:** Pointer to application instance

---

### crest_run

```c
int crest_run(crest_app_t *app, const char *host, int port);
```

Starts the HTTP server.

**Parameters:**
- `app` - Application instance
- `host` - Host to bind (e.g., "0.0.0.0", "localhost")
- `port` - Port number

**Returns:** 0 on success, -1 on error

---

### crest_destroy

```c
void crest_destroy(crest_app_t *app);
```

Frees all resources associated with the application.

**Parameters:**
- `app` - Application instance

---

## Route Registration

### crest_get

```c
void crest_get(crest_app_t *app, const char *path, 
               crest_handler_t handler, const char *description);
```

Registers a GET route.

**Parameters:**
- `app` - Application instance
- `path` - Route path (e.g., "/api/users")
- `handler` - Handler function
- `description` - Optional description for documentation

---

### crest_post

```c
void crest_post(crest_app_t *app, const char *path,
                crest_handler_t handler, const char *description);
```

Registers a POST route.

---

### crest_put

```c
void crest_put(crest_app_t *app, const char *path,
               crest_handler_t handler, const char *description);
```

Registers a PUT route.

---

### crest_delete

```c
void crest_delete(crest_app_t *app, const char *path,
                  crest_handler_t handler, const char *description);
```

Registers a DELETE route.

---

### crest_patch

```c
void crest_patch(crest_app_t *app, const char *path,
                 crest_handler_t handler, const char *description);
```

Registers a PATCH route.

---

## Request Functions

### crest_request_method

```c
crest_method_t crest_request_method(crest_request_t *req);
```

Gets the HTTP method of the request.

**Returns:** HTTP method enum value

---

### crest_request_path

```c
const char* crest_request_path(crest_request_t *req);
```

Gets the request path.

**Returns:** Path string

---

### crest_request_query

```c
const char* crest_request_query(crest_request_t *req, const char *key);
```

Gets a query parameter value.

**Parameters:**
- `req` - Request instance
- `key` - Parameter name

**Returns:** Parameter value, or NULL if not found

---

### crest_request_param

```c
const char* crest_request_param(crest_request_t *req, const char *key);
```

Gets a path parameter value (e.g., from /users/:id).

**Returns:** Parameter value, or NULL if not found

---

### crest_request_header

```c
const char* crest_request_header(crest_request_t *req, const char *key);
```

Gets a request header value.

**Returns:** Header value, or NULL if not found

---

### crest_request_body

```c
const char* crest_request_body(crest_request_t *req);
```

Gets the request body.

**Returns:** Body string, or NULL if no body

---

## Response Functions

### crest_response_status

```c
void crest_response_status(crest_response_t *res, crest_status_t status);
```

Sets the HTTP response status code.

**Parameters:**
- `res` - Response instance
- `status` - HTTP status code

---

### crest_response_header

```c
void crest_response_header(crest_response_t *res, 
                          const char *key, const char *value);
```

Sets a response header.

---

### crest_response_send

```c
void crest_response_send(crest_response_t *res, const char *body);
```

Sends a plain text response.

---

### crest_response_json

```c
void crest_response_json(crest_response_t *res, const char *json);
```

Sends a JSON response with proper Content-Type header.

---

### crest_response_sendf

```c
void crest_response_sendf(crest_response_t *res, const char *format, ...);
```

Sends a formatted response.

---

## Middleware

### crest_use

```c
void crest_use(crest_app_t *app, crest_middleware_fn_t middleware_fn);
```

Adds middleware to the application.

**Parameters:**
- `app` - Application instance
- `middleware_fn` - Middleware function

---

## Dashboard

### crest_enable_dashboard

```c
void crest_enable_dashboard(crest_app_t *app, bool enable);
```

Enables or disables the web dashboard.

---

### crest_set_dashboard_path

```c
void crest_set_dashboard_path(crest_app_t *app, const char *path);
```

Sets the dashboard path (default: "/docs").

---

## Configuration

### crest_config_create

```c
crest_config_t* crest_config_create(void);
```

Creates a default configuration.

---

### crest_config_load

```c
crest_config_t* crest_config_load(const char *filepath);
```

Loads configuration from a file.

---

## Status Codes

| Code | Constant | Description |
|------|----------|-------------|
| 200 | CREST_STATUS_OK | Success |
| 201 | CREST_STATUS_CREATED | Resource created |
| 204 | CREST_STATUS_NO_CONTENT | Success, no content |
| 400 | CREST_STATUS_BAD_REQUEST | Bad request |
| 401 | CREST_STATUS_UNAUTHORIZED | Unauthorized |
| 403 | CREST_STATUS_FORBIDDEN | Forbidden |
| 404 | CREST_STATUS_NOT_FOUND | Not found |
| 500 | CREST_STATUS_INTERNAL_ERROR | Internal error |

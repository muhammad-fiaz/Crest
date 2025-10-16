---
layout: default
title: Quick Start
nav_order: 2
---

# Quick Start Guide
{: .no_toc }

Get up and running with Crest in minutes!
{: .fs-6 .fw-300 }

## Table of contents
{: .no_toc .text-delta }

1. TOC
{:toc}

---

## Installation

### Prerequisites

- C11 or C++17 compatible compiler
- CMake 3.15+ (for CMake builds)
- Git

### Option 1: Using CMake

```bash
# Clone the repository
git clone https://github.com/muhammad-fiaz/crest.git
cd crest

# Build and install
mkdir build && cd build
cmake ..
cmake --build .
sudo cmake --install .
```

### Option 2: Using xmake

```bash
git clone https://github.com/muhammad-fiaz/crest.git
cd crest
xmake
xmake install
```

### Option 3: Using Make

```bash
git clone https://github.com/muhammad-fiaz/crest.git
cd crest
make
sudo make install
```

### Option 4: Using vcpkg

```bash
vcpkg install crest
```

### Option 5: Using Conan

```bash
conan install crest/1.0.0@
```

## Your First Application

Create a file named `main.c`:

```c
#include <crest/crest.h>

void hello_handler(crest_request_t *req, crest_response_t *res) {
    crest_response_json(res, "{\"message\":\"Hello from Crest!\"}");
}

int main(void) {
    // Create application
    crest_app_t *app = crest_create();
    
    // Enable web dashboard
    crest_enable_dashboard(app, true);
    
    // Register route
    crest_get(app, "/", hello_handler, "Welcome endpoint");
    
    // Start server
    crest_run(app, "0.0.0.0", 8080);
    
    // Cleanup
    crest_destroy(app);
    return 0;
}
```

### Compile and Run

```bash
gcc -o myapp main.c -lcrest
./myapp
```

Visit:
- Application: `http://localhost:8080/`
- Dashboard: `http://localhost:8080/docs`

## Basic Concepts

### Routes

Routes map URL paths to handler functions:

```c
crest_get(app, "/users", list_users, "List all users");
crest_post(app, "/users", create_user, "Create a user");
crest_put(app, "/users/:id", update_user, "Update a user");
crest_delete(app, "/users/:id", delete_user, "Delete a user");
```

### Handlers

Handler functions process requests and send responses:

```c
void my_handler(crest_request_t *req, crest_response_t *res) {
    // Read request
    const char *param = crest_request_param(req, "id");
    
    // Send response
    crest_response_json(res, "{\"status\":\"ok\"}");
}
```

### Path Parameters

Capture dynamic values from URLs:

```c
void get_user(crest_request_t *req, crest_response_t *res) {
    const char *id = crest_request_param(req, "id");
    
    char json[256];
    snprintf(json, sizeof(json), "{\"id\":\"%s\"}", id);
    crest_response_json(res, json);
}

crest_get(app, "/users/:id", get_user, "Get user by ID");
```

### Query Parameters

Access URL query parameters:

```c
void search(crest_request_t *req, crest_response_t *res) {
    const char *q = crest_request_query(req, "q");
    const char *limit = crest_request_query(req, "limit");
    
    // Use parameters...
}

// GET /search?q=test&limit=10
```

### Request Body

Read POST/PUT request bodies:

```c
void create_item(crest_request_t *req, crest_response_t *res) {
    const char *body = crest_request_body(req);
    
    // Parse and process body...
    
    crest_response_status(res, CREST_STATUS_CREATED);
    crest_response_json(res, "{\"message\":\"Created\"}");
}
```

## Next Steps

- [API Reference](api) - Complete API documentation
- [Examples](examples) - More code examples
- [Configuration](configuration) - Configuration options
- [Middleware](middleware) - Using middleware
- [Deployment](deployment) - Deploying to production

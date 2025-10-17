# Crest C++ API Documentation

Complete reference for using Crest in C++ projects.

## Table of Contents

- [Getting Started](#getting-started)
- [App Class](#app-class)
- [Request Class](#request-class)
- [Response Class](#response-class)
- [Configuration](#configuration)
- [Exception Handling](#exception-handling)
- [Examples](#examples)

## Getting Started

Include the Crest header in your C++ project:

```cpp
#include "crest/crest.hpp"
```

Use the crest namespace:

```cpp
using namespace crest;
```

## App Class

The main application class for creating and managing your API.

### Constructor

```cpp
App();
App(const Config& config);
```

**Example:**
```cpp
// Default constructor
crest::App app;

// With configuration
crest::Config config;
config.title = "My API";
crest::App app(config);
```

### Route Registration Methods

#### get

Register a GET route.

```cpp
App& get(const std::string& path, Handler handler, 
         const std::string& description = "");
```

**Parameters:**
- `path`: Route path
- `handler`: Lambda or function to handle requests
- `description`: Optional route description

**Returns:** Reference to App for method chaining

**Example:**
```cpp
app.get("/users", [](Request& req, Response& res) {
    res.json(Status::OK, R"({"users":[]})");
}, "Get all users");
```

#### post

Register a POST route.

```cpp
App& post(const std::string& path, Handler handler,
          const std::string& description = "");
```

**Example:**
```cpp
app.post("/users", [](Request& req, Response& res) {
    std::string body = req.body();
    res.json(Status::CREATED, R"({"message":"Created"})");
}, "Create a user");
```

#### put

Register a PUT route.

```cpp
App& put(const std::string& path, Handler handler,
         const std::string& description = "");
```

#### del

Register a DELETE route.

```cpp
App& del(const std::string& path, Handler handler,
         const std::string& description = "");
```

#### patch

Register a PATCH route.

```cpp
App& patch(const std::string& path, Handler handler,
           const std::string& description = "");
```

#### route

Register a route with a specific HTTP method.

```cpp
App& route(Method method, const std::string& path, Handler handler,
           const std::string& description = "");
```

**Example:**
```cpp
app.route(Method::GET, "/status", [](Request& req, Response& res) {
    res.json(200, R"({"status":"ok"})");
});
```

### Server Control

#### run

Start the HTTP server.

```cpp
void run(const std::string& host = "0.0.0.0", int port = 8000);
```

**Parameters:**
- `host`: Host address (default: "0.0.0.0")
- `port`: Port number (default: 8000)

**Throws:** `crest::Exception` on error

**Example:**
```cpp
try {
    app.run("127.0.0.1", 3000);
} catch (const crest::Exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

#### stop

Stop the HTTP server.

```cpp
void stop();
```

### Configuration Methods

#### set_title

Set the application title for documentation.

```cpp
void set_title(const std::string& title);
```

#### set_description

Set the application description for documentation.

```cpp
void set_description(const std::string& description);
```

#### set_docs_enabled

Enable or disable Swagger documentation.

```cpp
void set_docs_enabled(bool enabled);
```

**Example:**
```cpp
#ifdef PRODUCTION
    app.set_docs_enabled(false);
#endif
```

#### set_proxy

Configure proxy settings.

```cpp
void set_proxy(const std::string& proxy_url);
```

## Request Class

Represents an HTTP request.

### Methods

#### path

Get the request path.

```cpp
std::string path() const;
```

**Returns:** Request path

**Example:**
```cpp
std::string p = req.path(); // "/api/users"
```

#### method

Get the HTTP method.

```cpp
std::string method() const;
```

**Returns:** HTTP method (e.g., "GET", "POST")

#### body

Get the request body.

```cpp
std::string body() const;
```

**Returns:** Request body as string

**Example:**
```cpp
std::string data = req.body();
// Parse JSON, etc.
```

#### query

Get a query parameter value.

```cpp
std::string query(const std::string& key) const;
```

**Parameters:**
- `key`: Query parameter key

**Returns:** Parameter value, or empty string if not found

**Example:**
```cpp
std::string page = req.query("page"); // ?page=1
```

#### header

Get a header value.

```cpp
std::string header(const std::string& key) const;
```

**Parameters:**
- `key`: Header key

**Returns:** Header value, or empty string if not found

**Example:**
```cpp
std::string auth = req.header("Authorization");
```

#### queries

Get all query parameters.

```cpp
std::map<std::string, std::string> queries() const;
```

**Returns:** Map of all query parameters

#### headers

Get all headers.

```cpp
std::map<std::string, std::string> headers() const;
```

**Returns:** Map of all headers

## Response Class

Represents an HTTP response.

### Methods

#### json

Send a JSON response.

```cpp
void json(Status status, const std::string& json);
void json(int status, const std::string& json);
```

**Parameters:**
- `status`: HTTP status code (enum or int)
- `json`: JSON string

**Example:**
```cpp
res.json(Status::OK, R"({"message":"Success"})");
res.json(200, R"({"data":[]})");
```

#### text

Send a plain text response.

```cpp
void text(Status status, const std::string& text);
void text(int status, const std::string& text);
```

**Example:**
```cpp
res.text(Status::OK, "Hello, World!");
```

#### html

Send an HTML response.

```cpp
void html(Status status, const std::string& html);
void html(int status, const std::string& html);
```

**Example:**
```cpp
res.html(Status::OK, "<h1>Welcome</h1>");
```

#### set_header

Set a response header.

```cpp
void set_header(const std::string& key, const std::string& value);
```

**Example:**
```cpp
res.set_header("X-Custom-Header", "value");
res.json(200, "{}");
```

## Configuration

### Config Struct

```cpp
struct Config {
    std::string title = "Crest API";
    std::string description = "RESTful API built with Crest";
    std::string version = "0.0.0";
    bool docs_enabled = true;
    std::string docs_path = "/docs";
    std::string openapi_path = "/openapi.json";
    std::string proxy_url;
    int max_connections = 1000;
    int timeout_seconds = 30;
};
```

**Example:**
```cpp
crest::Config config;
config.title = "My API";
config.description = "A powerful API";
config.version = "1.0.0";
config.docs_enabled = true;
config.max_connections = 500;

crest::App app(config);
```

## Enums

### Method

```cpp
enum class Method {
    GET,
    POST,
    PUT,
    DELETE,
    PATCH,
    HEAD,
    OPTIONS
};
```

### Status

```cpp
enum class Status {
    OK = 200,
    CREATED = 201,
    ACCEPTED = 202,
    NO_CONTENT = 204,
    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    INTERNAL_ERROR = 500,
    NOT_IMPLEMENTED = 501,
    SERVICE_UNAVAILABLE = 503
};
```

## Exception Handling

### Exception Class

```cpp
class Exception : public std::exception {
public:
    explicit Exception(const std::string& message, int code = 500);
    const char* what() const noexcept override;
    int code() const noexcept;
};
```

**Example:**
```cpp
try {
    app.run("0.0.0.0", 8000);
} catch (const crest::Exception& e) {
    std::cerr << "Error " << e.code() << ": " << e.what() << std::endl;
}
```

## Examples

### Basic API

```cpp
#include "crest/crest.hpp"
#include <iostream>

int main() {
    crest::App app;
    
    app.get("/", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"message":"Hello, World!"})");
    });
    
    app.run("0.0.0.0", 8000);
    return 0;
}
```

### RESTful CRUD API

```cpp
#include "crest/crest.hpp"

int main() {
    crest::App app;
    
    // List all
    app.get("/users", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"users":[]})");
    }, "Get all users");
    
    // Get one
    app.get("/users/:id", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"id":1,"name":"John"})");
    }, "Get user by ID");
    
    // Create
    app.post("/users", [](crest::Request& req, crest::Response& res) {
        std::string body = req.body();
        res.json(crest::Status::CREATED, R"({"message":"Created"})");
    }, "Create a new user");
    
    // Update
    app.put("/users/:id", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"message":"Updated"})");
    }, "Update user");
    
    // Delete
    app.del("/users/:id", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"message":"Deleted"})");
    }, "Delete user");
    
    app.run("0.0.0.0", 8000);
    return 0;
}
```

### With Configuration

```cpp
#include "crest/crest.hpp"

int main() {
    crest::Config config;
    config.title = "My API";
    config.description = "A powerful RESTful API";
    config.version = "1.0.0";
    config.docs_enabled = true;
    
    crest::App app(config);
    
    app.get("/", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"status":"running"})");
    });
    
    try {
        app.run("127.0.0.1", 3000);
    } catch (const crest::Exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### Method Chaining

```cpp
crest::App app;

app.get("/a", handler1)
   .post("/b", handler2)
   .put("/c", handler3)
   .del("/d", handler4);

app.run();
```

## Best Practices

1. **Use RAII** - App destructor handles cleanup automatically
2. **Capture by reference** in lambdas when needed for performance
3. **Use Status enum** for better code readability
4. **Handle exceptions** around app.run()
5. **Validate input** in your handlers
6. **Use const references** for string parameters
7. **Disable docs in production** for security

## Thread Safety

Request handlers are executed in separate threads. Ensure your handlers are thread-safe when accessing shared resources. Use mutexes or other synchronization primitives as needed.

## Performance Tips

1. **Minimize allocations** in hot paths
2. **Use string_view** for read-only string operations
3. **Reserve capacity** for containers when size is known
4. **Avoid unnecessary copies** - use move semantics
5. **Profile your handlers** to identify bottlenecks

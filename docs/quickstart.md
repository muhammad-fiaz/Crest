# Quick Start Guide

Get up and running with Crest in minutes!

## Your First API

Let's build a simple REST API with Crest.

### C++ Version

Create `main.cpp`:

```cpp
#include "crest/crest.hpp"

int main() {
    // Create app
    crest::App app;
    
    // Define a route
    app.get("/", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"message":"Hello, Crest!"})");
    });
    
    // Start server
    app.run("127.0.0.1", 8000);
    return 0;
}
```

Build and run:

```bash
xmake build
xmake run
```

Visit `http://127.0.0.1:8000` - you should see:

```json
{"message":"Hello, Crest!"}
```

### C Version

Create `main.c`:

```c
#include "crest/crest.h"
#include <stdio.h>

void handle_root(crest_request_t* req, crest_response_t* res) {
    crest_response_json(res, 200, "{\"message\":\"Hello, Crest!\"}");
}

int main(void) {
    crest_app_t* app = crest_create();
    crest_route(app, CREST_GET, "/", handle_root, "Root endpoint");
    
    printf("Server running on http://127.0.0.1:8000\n");
    crest_run(app, "127.0.0.1", 8000);
    
    crest_destroy(app);
    return 0;
}
```

## Building a CRUD API

Let's create a simple user management API.

### C++ Implementation

```cpp
#include "crest/crest.hpp"
#include <map>
#include <mutex>

// Simple in-memory storage
std::map<int, std::string> users;
std::mutex users_mutex;
int next_id = 1;

int main() {
    crest::App app;
    
    // List all users
    app.get("/users", [](crest::Request& req, crest::Response& res) {
        std::lock_guard<std::mutex> lock(users_mutex);
        
        std::string json = "{\"users\":[";
        bool first = true;
        for (const auto& [id, name] : users) {
            if (!first) json += ",";
            json += "{\"id\":" + std::to_string(id) + ",\"name\":\"" + name + "\"}";
            first = false;
        }
        json += "]}";
        
        res.json(200, json);
    }, "Get all users");
    
    // Get single user
    app.get("/users/:id", [](crest::Request& req, crest::Response& res) {
        // In a real implementation, parse :id from path
        std::lock_guard<std::mutex> lock(users_mutex);
        
        if (users.find(1) != users.end()) {
            res.json(200, R"({"id":1,"name":")" + users[1] + "\"}");
        } else {
            res.json(404, R"({"error":"User not found"})");
        }
    }, "Get user by ID");
    
    // Create user
    app.post("/users", [](crest::Request& req, crest::Response& res) {
        std::lock_guard<std::mutex> lock(users_mutex);
        
        // In a real implementation, parse JSON body
        int id = next_id++;
        users[id] = "New User";
        
        std::string json = "{\"id\":" + std::to_string(id) + ",\"name\":\"New User\"}";
        res.json(201, json);
    }, "Create a new user");
    
    // Update user
    app.put("/users/:id", [](crest::Request& req, crest::Response& res) {
        std::lock_guard<std::mutex> lock(users_mutex);
        
        // In a real implementation, parse :id and body
        if (users.find(1) != users.end()) {
            users[1] = "Updated User";
            res.json(200, R"({"message":"User updated"})");
        } else {
            res.json(404, R"({"error":"User not found"})");
        }
    }, "Update user by ID");
    
    // Delete user
    app.del("/users/:id", [](crest::Request& req, crest::Response& res) {
        std::lock_guard<std::mutex> lock(users_mutex);
        
        // In a real implementation, parse :id
        if (users.erase(1) > 0) {
            res.json(200, R"({"message":"User deleted"})");
        } else {
            res.json(404, R"({"error":"User not found"})");
        }
    }, "Delete user by ID");
    
    app.run("127.0.0.1", 8000);
    return 0;
}
```

## Using Configuration

Customize your API with configuration:

```cpp
#include "crest/crest.hpp"

int main() {
    // Create configuration
    crest::Config config;
    config.title = "My API";
    config.description = "A simple REST API";
    config.version = "1.0.0";
    config.docs_enabled = true;
    
    // Create app with config
    crest::App app(config);
    
    // Add routes
    app.get("/", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"status":"running"})");
    });
    
    app.run("0.0.0.0", 8000);
    return 0;
}
```

## Viewing Documentation

Crest automatically generates Swagger UI documentation for your API.

1. Start your server
2. Visit `http://127.0.0.1:8000/docs`
3. See all your routes with descriptions
4. View OpenAPI spec at `http://127.0.0.1:8000/openapi.json`

## Handling Different Response Types

### JSON Response

```cpp
app.get("/json", [](crest::Request& req, crest::Response& res) {
    res.json(200, R"({"type":"json"})");
});
```

### Text Response

```cpp
app.get("/text", [](crest::Request& req, crest::Response& res) {
    res.text(200, "Plain text response");
});
```

### HTML Response

```cpp
app.get("/html", [](crest::Request& req, crest::Response& res) {
    res.html(200, "<h1>HTML Response</h1>");
});
```

## Reading Request Data

### Request Body

```cpp
app.post("/data", [](crest::Request& req, crest::Response& res) {
    std::string body = req.body();
    // Process body...
    res.json(200, R"({"received":true})");
});
```

### Query Parameters

```cpp
app.get("/search", [](crest::Request& req, crest::Response& res) {
    std::string query = req.query("q");
    // Use query parameter...
    res.json(200, R"({"query":")" + query + "\"}");
});
```

### Headers

```cpp
app.get("/auth", [](crest::Request& req, crest::Response& res) {
    std::string auth = req.header("Authorization");
    if (auth.empty()) {
        res.json(401, R"({"error":"Unauthorized"})");
    } else {
        res.json(200, R"({"status":"authorized"})");
    }
});
```

## Error Handling

### C++ Exception Handling

```cpp
int main() {
    try {
        crest::App app;
        
        app.get("/", [](crest::Request& req, crest::Response& res) {
            res.json(200, R"({"status":"ok"})");
        });
        
        app.run("0.0.0.0", 8000);
        
    } catch (const crest::Exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### Route-Level Error Handling

```cpp
app.get("/risky", [](crest::Request& req, crest::Response& res) {
    try {
        // Risky operation...
        res.json(200, R"({"status":"success"})");
    } catch (const std::exception& e) {
        res.json(500, R"({"error":"Internal server error"})");
    }
});
```

## Method Chaining

Chain route definitions for cleaner code:

```cpp
app.get("/a", handler1)
   .post("/b", handler2)
   .put("/c", handler3)
   .del("/d", handler4);
```

## Production Deployment

### Disable Documentation

```cpp
#ifdef PRODUCTION
    app.set_docs_enabled(false);
#endif
```

### Use Environment Variables

```cpp
#include <cstdlib>

int main() {
    crest::App app;
    
    // Get port from environment
    const char* port_env = std::getenv("PORT");
    int port = port_env ? std::stoi(port_env) : 8000;
    
    app.run("0.0.0.0", port);
    return 0;
}
```

## Testing Your API

### Using curl

```bash
# GET request
curl http://127.0.0.1:8000/

# POST request
curl -X POST http://127.0.0.1:8000/users \
  -H "Content-Type: application/json" \
  -d '{"name":"John Doe"}'

# PUT request
curl -X PUT http://127.0.0.1:8000/users/1 \
  -H "Content-Type: application/json" \
  -d '{"name":"Jane Doe"}'

# DELETE request
curl -X DELETE http://127.0.0.1:8000/users/1
```

### Using Python

```python
import requests

# GET
response = requests.get('http://127.0.0.1:8000/')
print(response.json())

# POST
response = requests.post('http://127.0.0.1:8000/users',
    json={'name': 'John Doe'})
print(response.json())
```

## Next Steps

- Explore the [C API Documentation](c_api.md)
- Read the [C++ API Documentation](cpp_api.md)
- Learn about [Configuration Options](configuration.md)
- Check out more [Examples](examples.md)

## Common Patterns

### Health Check Endpoint

```cpp
app.get("/health", [](crest::Request& req, crest::Response& res) {
    res.json(200, R"({"status":"healthy","timestamp":)" + 
        std::to_string(time(nullptr)) + "}");
}, "Health check endpoint");
```

### Version Endpoint

```cpp
app.get("/version", [](crest::Request& req, crest::Response& res) {
    res.json(200, R"({"version":"1.0.0","api":"v1"})");
}, "API version information");
```

### 404 Handler

Crest automatically returns 404 for undefined routes, but you can customize the response in your handlers.

## Tips and Best Practices

1. **Use descriptive route descriptions** - They appear in Swagger UI
2. **Handle errors gracefully** - Always catch exceptions
3. **Validate input** - Check request data before processing
4. **Use appropriate status codes** - Follow HTTP standards
5. **Keep handlers focused** - One responsibility per handler
6. **Test your API** - Write tests for all endpoints
7. **Document your API** - Use the built-in Swagger UI
8. **Secure your API** - Disable docs in production

Happy coding with Crest! 🌊

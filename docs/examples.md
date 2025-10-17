# Examples

Collection of practical examples using Crest.

## Basic Examples

### Hello World

=== "C++"
    ```cpp
    #include "crest/crest.hpp"

    int main() {
        crest::App app;
        
        app.get("/", [](crest::Request& req, crest::Response& res) {
            res.json(200, R"({"message":"Hello, World!"})");
        });
        
        app.run("0.0.0.0", 8000);
        return 0;
    }
    ```

=== "C"
    ```c
    #include "crest/crest.h"

    void handle_root(crest_request_t* req, crest_response_t* res) {
        crest_response_json(res, 200, "{\"message\":\"Hello, World!\"}");
    }

    int main(void) {
        crest_app_t* app = crest_create();
        crest_route(app, CREST_GET, "/", handle_root, "Root");
        crest_run(app, "0.0.0.0", 8000);
        crest_destroy(app);
        return 0;
    }
    ```

### Multiple Routes

```cpp
#include "crest/crest.hpp"

int main() {
    crest::App app;
    
    app.get("/", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"message":"Home"})");
    }, "Home page");
    
    app.get("/about", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"message":"About page"})");
    }, "About page");
    
    app.get("/contact", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"message":"Contact page"})");
    }, "Contact page");
    
    app.run("0.0.0.0", 8000);
    return 0;
}
```

## CRUD API Examples

### User Management API

```cpp
#include "crest/crest.hpp"
#include <map>
#include <mutex>
#include <sstream>

std::map<int, std::string> users;
std::mutex users_mutex;
int next_id = 1;

std::string users_to_json() {
    std::ostringstream oss;
    oss << "{\"users\":[";
    bool first = true;
    for (const auto& [id, name] : users) {
        if (!first) oss << ",";
        oss << "{\"id\":" << id << ",\"name\":\"" << name << "\"}";
        first = false;
    }
    oss << "]}";
    return oss.str();
}

int main() {
    crest::App app;
    
    // List users
    app.get("/users", [](crest::Request& req, crest::Response& res) {
        std::lock_guard<std::mutex> lock(users_mutex);
        res.json(200, users_to_json());
    }, "List all users");
    
    // Create user
    app.post("/users", [](crest::Request& req, crest::Response& res) {
        std::lock_guard<std::mutex> lock(users_mutex);
        int id = next_id++;
        users[id] = "User " + std::to_string(id);
        
        std::ostringstream oss;
        oss << "{\"id\":" << id << ",\"name\":\"" << users[id] << "\"}";
        res.json(201, oss.str());
    }, "Create a user");
    
    // Update user
    app.put("/users/:id", [](crest::Request& req, crest::Response& res) {
        std::lock_guard<std::mutex> lock(users_mutex);
        // Parse ID from path in real implementation
        res.json(200, R"({"message":"User updated"})");
    }, "Update a user");
    
    // Delete user
    app.del("/users/:id", [](crest::Request& req, crest::Response& res) {
        std::lock_guard<std::mutex> lock(users_mutex);
        // Parse ID from path in real implementation
        res.json(200, R"({"message":"User deleted"})");
    }, "Delete a user");
    
    app.run("0.0.0.0", 8000);
    return 0;
}
```

### Blog API

```cpp
#include "crest/crest.hpp"
#include <vector>
#include <string>

struct Post {
    int id;
    std::string title;
    std::string content;
    std::string author;
};

std::vector<Post> posts;
int next_post_id = 1;

int main() {
    crest::App app;
    
    app.get("/posts", [](crest::Request& req, crest::Response& res) {
        // Return all posts
        res.json(200, R"({"posts":[]})");
    }, "Get all blog posts");
    
    app.get("/posts/:id", [](crest::Request& req, crest::Response& res) {
        // Return single post
        res.json(200, R"({"id":1,"title":"Post"})");
    }, "Get a blog post");
    
    app.post("/posts", [](crest::Request& req, crest::Response& res) {
        // Create new post
        res.json(201, R"({"message":"Post created"})");
    }, "Create a blog post");
    
    app.run("0.0.0.0", 8000);
    return 0;
}
```

## Configuration Examples

### Custom Configuration

```cpp
#include "crest/crest.hpp"

int main() {
    crest::Config config;
    config.title = "My Custom API";
    config.description = "A fully customized API";
    config.version = "2.0.0";
    config.docs_enabled = true;
    config.max_connections = 500;
    config.timeout_seconds = 45;
    
    crest::App app(config);
    
    app.get("/", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"status":"running"})");
    });
    
    app.run("0.0.0.0", 8000);
    return 0;
}
```

### Environment-Based Configuration

```cpp
#include "crest/crest.hpp"
#include <cstdlib>
#include <iostream>

std::string getEnv(const char* key, const char* default_val) {
    const char* val = std::getenv(key);
    return val ? std::string(val) : std::string(default_val);
}

int main() {
    std::string env = getEnv("ENVIRONMENT", "development");
    
    crest::Config config;
    config.title = getEnv("API_TITLE", "My API");
    config.version = getEnv("API_VERSION", "1.0.0");
    config.docs_enabled = (env != "production");
    
    crest::App app(config);
    
    app.get("/", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"status":"ok"})");
    });
    
    std::string host = getEnv("HOST", "0.0.0.0");
    int port = std::stoi(getEnv("PORT", "8000"));
    
    std::cout << "Running in " << env << " mode\n";
    app.run(host, port);
    
    return 0;
}
```

## Response Type Examples

### JSON Responses

```cpp
app.get("/json", [](crest::Request& req, crest::Response& res) {
    res.json(200, R"({
        "status": "success",
        "data": {
            "id": 1,
            "name": "Example"
        }
    })");
});
```

### Text Responses

```cpp
app.get("/text", [](crest::Request& req, crest::Response& res) {
    res.text(200, "This is plain text");
});
```

### HTML Responses

```cpp
app.get("/html", [](crest::Request& req, crest::Response& res) {
    res.html(200, R"(
        <!DOCTYPE html>
        <html>
        <head><title>Crest</title></head>
        <body>
            <h1>Welcome to Crest</h1>
            <p>This is an HTML response</p>
        </body>
        </html>
    )");
});
```

## Request Handling Examples

### Reading Request Body

```cpp
app.post("/data", [](crest::Request& req, crest::Response& res) {
    std::string body = req.body();
    
    // Process body (parse JSON, etc.)
    
    res.json(200, R"({"received": true})");
});
```

### Query Parameters

```cpp
app.get("/search", [](crest::Request& req, crest::Response& res) {
    std::string query = req.query("q");
    std::string page = req.query("page");
    
    // Use parameters
    
    res.json(200, R"({"query":")" + query + "\"}");
});
```

### Headers

```cpp
app.get("/auth", [](crest::Request& req, crest::Response& res) {
    std::string auth = req.header("Authorization");
    
    if (auth.empty()) {
        res.json(401, R"({"error":"Unauthorized"})");
        return;
    }
    
    // Validate token
    
    res.json(200, R"({"status":"authorized"})");
});
```

## Error Handling Examples

### Try-Catch Pattern

```cpp
int main() {
    try {
        crest::App app;
        
        app.get("/", [](crest::Request& req, crest::Response& res) {
            res.json(200, R"({"status":"ok"})");
        });
        
        app.run("0.0.0.0", 8000);
        
    } catch (const crest::Exception& e) {
        std::cerr << "Crest error: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
```

### Route-Level Error Handling

```cpp
app.get("/risky", [](crest::Request& req, crest::Response& res) {
    try {
        // Potentially failing operation
        throw std::runtime_error("Something went wrong");
        
    } catch (const std::exception& e) {
        res.json(500, R"({"error":"Internal server error"})");
    }
});
```

## Advanced Examples

### API with Authentication

```cpp
#include "crest/crest.hpp"
#include <string>

bool validate_token(const std::string& token) {
    // Implement token validation
    return token == "secret-token";
}

int main() {
    crest::App app;
    
    app.post("/login", [](crest::Request& req, crest::Response& res) {
        // Validate credentials
        res.json(200, R"({"token":"secret-token"})");
    }, "Login endpoint");
    
    app.get("/protected", [](crest::Request& req, crest::Response& res) {
        std::string auth = req.header("Authorization");
        
        if (!validate_token(auth)) {
            res.json(401, R"({"error":"Unauthorized"})");
            return;
        }
        
        res.json(200, R"({"data":"Protected resource"})");
    }, "Protected endpoint");
    
    app.run("0.0.0.0", 8000);
    return 0;
}
```

### File Upload API

```cpp
app.post("/upload", [](crest::Request& req, crest::Response& res) {
    std::string body = req.body();
    
    // Process file upload
    // Save to disk, etc.
    
    res.json(200, R"({"message":"File uploaded"})");
}, "Upload a file");
```

### Pagination Example

```cpp
app.get("/items", [](crest::Request& req, crest::Response& res) {
    std::string page_str = req.query("page");
    std::string limit_str = req.query("limit");
    
    int page = page_str.empty() ? 1 : std::stoi(page_str);
    int limit = limit_str.empty() ? 10 : std::stoi(limit_str);
    
    // Fetch paginated data
    
    res.json(200, R"({
        "page": )" + std::to_string(page) + R"(,
        "limit": )" + std::to_string(limit) + R"(,
        "items": []
    })");
}, "Get paginated items");
```

## Testing Examples

### Health Check

```cpp
app.get("/health", [](crest::Request& req, crest::Response& res) {
    res.json(200, R"({
        "status": "healthy",
        "timestamp": )" + std::to_string(time(nullptr)) + R"(,
        "uptime": 12345
    })");
}, "Health check endpoint");
```

### Version Info

```cpp
app.get("/version", [](crest::Request& req, crest::Response& res) {
    res.json(200, R"({
        "version": "1.0.0",
        "api_version": "v1",
        "crest_version": ")" + std::string(crest::VERSION) + R"("
    })");
}, "Version information");
```

## Production Examples

### Production-Ready Setup

```cpp
#include "crest/crest.hpp"
#include <csignal>
#include <atomic>

std::atomic<bool> running{true};

void signal_handler(int signal) {
    running = false;
}

int main() {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    
    crest::Config config;
    config.title = "Production API";
    config.version = "1.0.0";
    
#ifdef PRODUCTION
    config.docs_enabled = false;
#else
    config.docs_enabled = true;
#endif
    
    crest::App app(config);
    
    app.get("/", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"status":"running"})");
    });
    
    try {
        app.run("0.0.0.0", 8000);
    } catch (const crest::Exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
```

## More Examples

Check the `examples/` directory in the repository for more complete examples:

- `examples/cpp/main.cpp` - Full C++ example
- `examples/c/main.c` - Full C example

## Running Examples

```bash
# Build examples
xmake build crest_example_cpp
xmake build crest_example_c

# Run C++ example
xmake run crest_example_cpp

# Run C example
xmake run crest_example_c
```

Visit `http://127.0.0.1:8000/docs` to see the generated documentation!

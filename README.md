<div align="center">

<img alt="Logo" src="https://github.com/user-attachments/assets/eaca101f-ca45-442c-9e0b-222530290f4b" />


**A Modern, Fast, and Lightweight REST API Framework for C/C++**

[![Version](https://img.shields.io/github/v/release/muhammad-fiaz/crest)](https://github.com/muhammad-fiaz/crest/releases)
[![License](https://img.shields.io/github/license/muhammad-fiaz/crest)](LICENSE)
[![Last Commit](https://img.shields.io/github/last-commit/muhammad-fiaz/crest)](https://github.com/muhammad-fiaz/crest/commits/main)
[![Issues](https://img.shields.io/github/issues/muhammad-fiaz/crest)](https://github.com/muhammad-fiaz/crest/issues)
[![Pull Requests](https://img.shields.io/github/issues-pr/muhammad-fiaz/crest)](https://github.com/muhammad-fiaz/crest/pulls)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg)](https://github.com/muhammad-fiaz/crest)


[📚 Documentation](https://muhammad-fiaz.github.io/Crest/) • [🚀 Quick Start](#-quick-start) • [🤝 Contributing](CONTRIBUTING.md)

</div>

A production-ready, high-performance RESTful API framework for C and C++. Inspired by modern API frameworks for simplicity and developer experience.

> Crest is Still in Early Development. Expect frequent changes and updates.

## ✨ Features

- 🚀 **High Performance** - Built for speed with minimal overhead and thread pool
- 🔧 **C & C++ Support** - Use with both C and C++ projects
- 📚 **Auto Documentation** - Built-in Swagger UI at `/docs`
- 🎯 **Simple API** - Intuitive, easy-to-use interface
- 🔒 **Production Ready** - Exception handling, validation, and error reporting
- 🌐 **Cross-Platform** - Windows, Linux, and macOS support
- 📦 **Easy Integration** - Install via xmake, Conan, or vcpkg
- ⚡ **Modern C++20** - Leverages latest C++ features
- 🎨 **Responsive UI** - Beautiful, mobile-friendly documentation interface
- 🔌 **Middleware System** - CORS, rate limiting, authentication, logging
- 🔌 **WebSocket Support** - Real-time bidirectional communication
- 💾 **Database Helpers** - Connection pooling, query builder, ORM-like models
- 📄 **File Upload** - Multipart form data parsing with validation
- 🎨 **Template Engine** - Dynamic HTML rendering with filters and conditionals

## 📦 Installation

### Using xmake (Recommended)

```bash
xmake repo -u
xmake require crest
```

In your `xmake.lua`:

```lua
add_requires("crest")

target("your_app")
    set_kind("binary")
    add_packages("crest")
    add_files("src/*.cpp")
```

### Build from Source

```bash
git clone https://github.com/muhammad-fiaz/crest.git
cd crest
xmake build
xmake install
```

### Download Pre-built Binaries

Download the latest packages from [GitHub Releases](https://github.com/muhammad-fiaz/crest/releases) - includes binaries, xmake, Conan, and vcpkg packages.

## 🚀 Quick Start

### C++ Example

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

### C Example

```c
#include "crest/crest.h"

void handle_root(crest_request_t* req, crest_response_t* res) {
    crest_response_json(res, 200, "{\"message\":\"Hello, World!\"}");
}

int main(void) {
    crest_app_t* app = crest_create();
    crest_route(app, CREST_GET, "/", handle_root, "Root endpoint");
    crest_run(app, "0.0.0.0", 8000);
    crest_destroy(app);
    return 0;
}
```

## 📖 Documentation

Full documentation is available at: [https://muhammad-fiaz.github.io/Crest](https://muhammad-fiaz.github.io/Crest)

- [C API Documentation](docs/c_api.md)
- [C++ API Documentation](docs/cpp_api.md)
- [Configuration Guide](docs/configuration.md)
- [Logging System](docs/logging.md)
- [Performance & Concurrency](docs/performance.md)
- [HTTP Status Codes Guide](docs/status_codes.md)
- [Schema Documentation](docs/schemas.md)
- [Middleware System](docs/middleware.md)
- [WebSocket Support](docs/websocket.md)
- [Database Integration](docs/database.md)
- [File Upload Handling](docs/file_upload.md)
- [Template Engine](docs/templates.md)
- [Examples](examples/)

## 🎯 Core Concepts

### Routes

Define routes with HTTP methods:

```cpp
app.get("/users", handler);           // GET request
app.post("/users", handler);          // POST request
app.put("/users/:id", handler);       // PUT request
app.del("/users/:id", handler);       // DELETE request
app.patch("/users/:id", handler);     // PATCH request
```

### Request Handling

```cpp
app.get("/api/data", [](crest::Request& req, crest::Response& res) {
    std::string body = req.body();
    std::string query = req.query("param");
    std::string header = req.header("Authorization");
    
    res.json(200, R"({"status":"success"})");
});
```

### Response Types

```cpp
res.json(200, R"({"key":"value"})");        // JSON response
res.text(200, "Plain text");                 // Text response
res.html(200, "<h1>HTML</h1>");             // HTML response
```

### Schema Definition

```cpp
// Set schemas to match your actual responses
app.set_response_schema(crest::Method::GET, "/user",
    R"({"id": "number", "name": "string", "email": "string"})");

app.set_request_schema(crest::Method::POST, "/user",
    R"({"name": "string", "email": "string"})");
```

### Configuration

```cpp
crest::Config config;
config.title = "My API";
config.description = "API Description";
config.version = "1.0.0";
config.docs_enabled = true;
config.docs_path = "/docs";
config.openapi_path = "/openapi.json";

crest::App app(config);
```

## 🔧 Advanced Features

### Proxy Configuration

```cpp
app.set_proxy("http://proxy.example.com:8080");
```

### Custom Host and Port

```cpp
app.run("127.0.0.1", 3000);
```

### Disable Documentation in Production

```cpp
app.set_docs_enabled(false);
```

### Exception Handling

```cpp
try {
    app.run("0.0.0.0", 8000);
} catch (const crest::Exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

## 📊 Swagger UI

Crest automatically generates interactive API documentation accessible at `/docs`:

- **Responsive Design** - Works on all devices
- **Real-time Updates** - Reflects all registered routes
- **OpenAPI Spec** - Available at `/openapi.json`
- **Error Display** - Shows route conflicts and errors

## 🏗️ Project Structure

```
crest/
├── include/
│   └── crest/
│       ├── crest.h          # C API
│       ├── crest.hpp        # C++ API
│       └── internal/        # Internal headers
├── src/
│   ├── core/               # Core functionality
│   ├── http/               # HTTP handling
│   ├── router/             # Route management
│   ├── server/             # Server implementation
│   ├── middleware/         # Middleware support
│   ├── swagger/            # Documentation generation
│   └── utils/              # Utilities
├── examples/
│   ├── c/                  # C examples
│   └── cpp/                # C++ examples
├── tests/                  # Test suite
├── docs/                   # Documentation
└── xmake.lua              # Build configuration
```

## 🧪 Testing

Run the test suite:

```bash
xmake build crest_tests
xmake run crest_tests
```

## 🤝 Contributing

Contributions are welcome! Please read our [Contributing Guide](CONTRIBUTING.md) for details.

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👤 Author

**Muhammad Fiaz**
- Email: contact@muhammadfiaz.com
- GitHub: [@muhammad-fiaz](https://github.com/muhammad-fiaz)

## 🙏 Acknowledgments

Crest is inspired by modern API frameworks that prioritize simplicity and developer experience.


## 🐛 Bug Reports

Found a bug? Please open an issue on [GitHub](https://github.com/muhammad-fiaz/crest/issues).

---

<div align="center">

[![Star History Chart](https://api.star-history.com/svg?repos=muhammad-fiaz/crest&type=Date&bg=transparent)](https://github.com/muhammad-fiaz/crest/)

**⭐ Star the repository if you find Crest useful!**

</div>

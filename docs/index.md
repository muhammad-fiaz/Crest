# Welcome to Crest 🌊

A production-ready, high-performance RESTful API framework for C and C++.

## Overview

Crest is a modern, lightweight framework for building RESTful APIs in C and C++. Inspired by the simplicity of modern API frameworks, Crest provides an intuitive interface while maintaining the performance and control that C/C++ developers expect.

## Key Features

- **🚀 High Performance** - Built for speed with minimal overhead
- **🔧 Dual Language Support** - Full support for both C and C++ projects
- **📚 Auto Documentation** - Built-in Swagger UI at `/docs`
- **🎯 Simple API** - Intuitive, easy-to-use interface
- **🔒 Production Ready** - Exception handling, validation, and error reporting
- **🌐 Cross-Platform** - Windows, Linux, and macOS support
- **📦 Easy Integration** - Install via xmake, Conan, or vcpkg
- **⚡ Modern C++20** - Leverages latest C++ features
- **🎨 Responsive UI** - Beautiful, mobile-friendly documentation interface

## Quick Example

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
        crest_route(app, CREST_GET, "/", handle_root, "Root endpoint");
        crest_run(app, "0.0.0.0", 8000);
        crest_destroy(app);
        return 0;
    }
    ```

## Why Crest?

### Simple and Intuitive

Crest provides a clean, modern API that feels natural to use. Whether you're building a microservice or a full-featured API, Crest gets out of your way and lets you focus on your application logic.

### Production Ready

Built with production use in mind, Crest includes:

- Comprehensive error handling
- Request validation
- Automatic documentation generation
- Thread-safe request handling
- Configurable timeouts and limits

### Performance Focused

Written in C/C++ with performance as a priority:

- Minimal overhead
- Efficient memory management
- Fast request routing
- Optimized for high concurrency

### Developer Experience

Inspired by modern frameworks for simplicity:

- Automatic Swagger UI generation
- Clear error messages
- Extensive documentation
- Rich examples

## Getting Started

Ready to build your first API with Crest? Check out our [Installation Guide](installation.md) and [Quick Start Tutorial](quickstart.md).

<div class="grid cards" markdown>

-   📥 __Installation__

    ---

    Get Crest installed on your system via xmake, Conan, or vcpkg

    [→ Install now](installation.md)

-   🚀 __Quick Start__

    ---

    Build your first REST API in minutes with our step-by-step guide

    [→ Get started](quickstart.md)

-   📖 __API Reference__

    ---

    Complete C and C++ API documentation with examples

    [→ View API docs](c_api.md)

-   💡 __Examples__

    ---

    Learn from real-world examples and code samples

    [→ Browse examples](examples.md)

-   ⚙️ __Configuration__

    ---

    Configure Crest for your specific needs

    [→ Configure](configuration.md)

-   ⚡ __Advanced Features__

    ---

    Explore middleware, WebSocket, database, and more

    [→ Learn more](middleware.md)

</div>

## Community

- **GitHub**: [muhammad-fiaz/crest](https://github.com/muhammad-fiaz/crest)
- **Issues**: [Report bugs or request features](https://github.com/muhammad-fiaz/crest/issues)
- **Email**: [contact@muhammadfiaz.com](mailto:contact@muhammadfiaz.com)

## License

Crest is open source software licensed under the [MIT License](license.md).

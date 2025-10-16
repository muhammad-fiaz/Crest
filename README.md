<div align="center">

# 🌊 Crest

**A Modern, Fast, and Lightweight REST API Framework for C/C++**

[![GitHub release](https://img.shields.io/github/v/release/muhammad-fiaz/crest)](https://github.com/muhammad-fiaz/crest/releases)
[![C Standard](https://img.shields.io/badge/c-11%2B-brightgreen.svg)](https://en.cppreference.com/w/c/11)
[![C++ Standard](https://img.shields.io/badge/c++-17%2B-brightgreen.svg)](https://en.cppreference.com/w/cpp/17)
[![Build Status](https://github.com/muhammad-fiaz/crest/actions/workflows/build.yml/badge.svg)](https://github.com/muhammad-fiaz/crest/actions)
[![License](https://img.shields.io/github/license/muhammad-fiaz/crest)](LICENSE)

*Inspired by simplicity, designed for performance*

[📚 Documentation](https://muhammad-fiaz.github.io/crest/) • [🚀 Quick Start](#-quick-start) • [🤝 Contributing](CONTRIBUTING.md)

</div>

---

> This Project is Active Development, So it may contain bugs and incomplete features. Please report any issues you find.

## 🎯 What is Crest?

**Crest** is a production-ready REST API framework for C and C++ that combines the simplicity of modern web frameworks with the performance of native code. Build fast, type-safe HTTP APIs with minimal boilerplate.

### Why Crest?

✅ **Simple & Intuitive** - Clean API design inspired by modern frameworks  
✅ **Blazing Fast** - Native C performance with zero overhead  
✅ **Production Ready** - Comprehensive test suite and memory safety  
✅ **Auto Documentation** - Interactive Swagger UI and OpenAPI support  
✅ **Cross-Platform** - Windows, Linux, macOS support  
✅ **Zero Dependencies** - Standalone library with optional extensions  

---

## ✨ Features

- 🚀 **High Performance** - Optimized for speed with minimal memory footprint
- 🎯 **RESTful Routing** - Path parameters, wildcards, and regex support
- 📊 **Interactive Dashboard** - Swagger UI and OpenAPI documentation
- 🔧 **Middleware System** - CORS, logging, authentication support
- 📦 **JSON Support** - RFC 8259 compliant parser and serializer
- 🔌 **Modular Design** - Use only what you need

---

## 🚀 Quick Start

### Prerequisites

- **C Compiler**: GCC 7+, Clang 6+, MSVC 2019+
- **CMake**: 3.15 or higher
- **Platform**: Windows 10+, Linux, macOS

### Installation

```bash
git clone https://github.com/muhammad-fiaz/crest.git
cd crest
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Basic Example

```c
#include <crest/crest.h>

void hello_handler(crest_request_t *req, crest_response_t *res) {
    crest_response_json(res, "{\"message\": \"Hello, World!\"}");
}

int main() {
    crest_app_t *app = crest_create();
    crest_get(app, "/hello", hello_handler, "Hello endpoint");
    crest_listen(app, 8080);
    crest_destroy(app);
    return 0;
}
```

---

## 📖 Examples

Check out the [examples/](examples/) directory for more comprehensive examples including:
- Basic API setup
- Advanced routing with middleware
- JSON handling
- Authentication patterns

---

## 🤝 Contributing

We welcome contributions! Please see our [Contributing Guide](CONTRIBUTING.md) for details.

---

## 📍 Support

- 📧 **Email**: [contact@muhammadfiaz.com](mailto:contact@muhammadfiaz.com)
- 🐛 **Issues**: [GitHub Issues](https://github.com/muhammad-fiaz/crest/issues)
- 💬 **Discussions**: [GitHub Discussions](https://github.com/muhammad-fiaz/crest/discussions)
- 📖 **Documentation**: [https://muhammad-fiaz.github.io/crest/](https://muhammad-fiaz.github.io/crest/)

---

## 📄 License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

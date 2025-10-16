---
layout: default
title: Home
nav_order: 1
description: "Crest - A modern, fast, and lightweight REST API framework for C/C++"
permalink: /
---

# 🌊 Crest Framework

{: .fs-9 }

A modern, fast, and lightweight REST API framework for C/C++ with a beautiful web dashboard
{: .fs-6 .fw-300 }

[Get Started](quickstart){: .btn .btn-primary .fs-5 .mb-4 .mb-md-0 .mr-2 }
[View on GitHub](https://github.com/muhammad-fiaz/crest){: .btn .fs-5 .mb-4 .mb-md-0 }

---

## Features

🚀 **Fast & Lightweight**
{: .label .label-green }
Minimal overhead with maximum performance

🎯 **Simple API**
{: .label .label-blue }
FastAPI-like syntax for C/C++

📊 **Web Dashboard**
{: .label .label-purple }
Beautiful interactive API documentation

🔧 **Highly Modular**
{: .label .label-yellow }
Clean separation of concerns

---

## Quick Example

```c
#include <crest/crest.h>

void hello(crest_request_t *req, crest_response_t *res) {
    crest_response_json(res, "{\"message\":\"Hello, World!\"}");
}

int main(void) {
    crest_app_t *app = crest_create();
    
    crest_enable_dashboard(app, true);
    crest_get(app, "/", hello, "Welcome endpoint");
    
    crest_run(app, "0.0.0.0", 8080);
    
    crest_destroy(app);
    return 0;
}
```

## Installation

### Using CMake

```bash
git clone https://github.com/muhammad-fiaz/crest.git
cd crest
mkdir build && cd build
cmake ..
cmake --build .
sudo cmake --install .
```

### Using vcpkg

```bash
vcpkg install crest
```

### Using Conan

```bash
conan install crest/1.0.0@
```

## Why Crest?

- **Zero Dependencies**: Core library has no external dependencies
- **Cross-Platform**: Works on Windows, Linux, and macOS
- **Multiple Build Systems**: CMake, xmake, Make, vcpkg, Conan
- **C & C++ Compatible**: Use with both C and C++ projects
- **Production Ready**: Battle-tested in real-world applications
- **Well Documented**: Comprehensive documentation and examples

## Community

- [GitHub Issues](https://github.com/muhammad-fiaz/crest/issues)
- [GitHub Discussions](https://github.com/muhammad-fiaz/crest/discussions)
- [Contributing Guide](https://github.com/muhammad-fiaz/crest/blob/main/CONTRIBUTING.md)

## License

Crest is released under the [MIT License](https://github.com/muhammad-fiaz/crest/blob/main/LICENSE).

---

Made with ❤️ by [Muhammad Fiaz](https://github.com/muhammad-fiaz)

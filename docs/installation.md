# Installation Guide

This guide covers multiple ways to install and use Crest in your projects.

## Prerequisites

- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- C17 compatible compiler for C projects
- CMake 3.15+ or xmake 2.8.0+

## Installation Methods

### Using xmake (Recommended)

xmake is the recommended build system for Crest.

#### Install xmake

=== "Linux/macOS"
    ```bash
    curl -fsSL https://xmake.io/shget.text | bash
    ```

=== "Windows"
    ```powershell
    Invoke-Expression (Invoke-Webrequest 'https://xmake.io/psget.text' -UseBasicParsing).Content
    ```

#### Add Crest to Your Project

In your `xmake.lua`:

```lua
add_requires("crest")

target("your_app")
    set_kind("binary")
    add_packages("crest")
    add_files("src/*.cpp")
```

Then build:

```bash
xmake build
```

### Using Conan

#### Install Conan

```bash
pip install conan
```

#### Add Crest Dependency

Create or update `conanfile.txt`:

```ini
[requires]
crest/0.0.0

[generators]
CMakeDeps
CMakeToolchain
```

#### Install Dependencies

```bash
conan install . --build=missing
```

#### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.15)
project(MyApp)

find_package(crest REQUIRED)

add_executable(my_app src/main.cpp)
target_link_libraries(my_app crest::crest)
```

### Using vcpkg

#### Install vcpkg

=== "Linux/macOS"
    ```bash
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    ./bootstrap-vcpkg.sh
    ```

=== "Windows"
    ```powershell
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    .\bootstrap-vcpkg.bat
    ```

#### Install Crest

```bash
./vcpkg install crest
```

#### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.15)
project(MyApp)

find_package(crest CONFIG REQUIRED)

add_executable(my_app src/main.cpp)
target_link_libraries(my_app PRIVATE crest::crest)
```

### Build from Source

#### Clone Repository

```bash
git clone https://github.com/muhammad-fiaz/crest.git
cd crest
```

#### Build with xmake

```bash
xmake config -m release
xmake build
xmake install
```

#### Build with CMake

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
sudo cmake --install .
```

## Platform-Specific Instructions

### Linux

#### Ubuntu/Debian

```bash
# Install dependencies
sudo apt update
sudo apt install build-essential git

# Install xmake
curl -fsSL https://xmake.io/shget.text | bash

# Clone and build
git clone https://github.com/muhammad-fiaz/crest.git
cd crest
xmake build
sudo xmake install
```

#### Fedora/RHEL

```bash
# Install dependencies
sudo dnf install gcc-c++ git

# Install xmake
curl -fsSL https://xmake.io/shget.text | bash

# Clone and build
git clone https://github.com/muhammad-fiaz/crest.git
cd crest
xmake build
sudo xmake install
```

### Windows

#### Using Visual Studio

1. Install Visual Studio 2019 or later with C++ support
2. Install xmake from [xmake.io](https://xmake.io)
3. Clone the repository:
   ```powershell
   git clone https://github.com/muhammad-fiaz/crest.git
   cd crest
   ```
4. Build:
   ```powershell
   xmake config -m release
   xmake build
   xmake install
   ```

#### Using MSYS2/MinGW

```bash
# Install dependencies
pacman -S mingw-w64-x86_64-gcc git

# Install xmake
curl -fsSL https://xmake.io/shget.text | bash

# Clone and build
git clone https://github.com/muhammad-fiaz/crest.git
cd crest
xmake build
xmake install
```

### macOS

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Install xmake
curl -fsSL https://xmake.io/shget.text | bash

# Clone and build
git clone https://github.com/muhammad-fiaz/crest.git
cd crest
xmake build
sudo xmake install
```

## Verify Installation

Create a test file `test.cpp`:

```cpp
#include "crest/crest.hpp"
#include <iostream>

int main() {
    std::cout << "Crest version: " << crest::VERSION << std::endl;
    
    crest::App app;
    app.get("/", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"status":"ok"})");
    });
    
    std::cout << "Starting server on http://127.0.0.1:8000" << std::endl;
    app.run("127.0.0.1", 8000);
    
    return 0;
}
```

Build and run:

=== "xmake"
    ```bash
    xmake build
    xmake run
    ```

=== "CMake"
    ```bash
    mkdir build && cd build
    cmake ..
    cmake --build .
    ./test
    ```

Visit `http://127.0.0.1:8000` to verify the server is running.

## Troubleshooting

### Compiler Not Found

Ensure you have a C++20 compatible compiler installed:

```bash
# Check GCC version (should be 10+)
g++ --version

# Check Clang version (should be 12+)
clang++ --version
```

### xmake Not Found

Add xmake to your PATH:

=== "Linux/macOS"
    ```bash
    export PATH=$HOME/.local/bin:$PATH
    ```

=== "Windows"
    Add `%USERPROFILE%\.local\bin` to your PATH environment variable.

### Link Errors on Windows

Ensure you're linking against the correct Windows libraries:

```lua
if is_plat("windows") then
    add_syslinks("ws2_32", "mswsock")
end
```

### Permission Denied on Linux/macOS

Use `sudo` for system-wide installation:

```bash
sudo xmake install
```

Or install to a user directory:

```bash
xmake install --prefix=$HOME/.local
```

## Next Steps

- Read the [Quick Start Guide](quickstart.md)
- Explore the [C API Documentation](c_api.md)
- Check out the [C++ API Documentation](cpp_api.md)
- View [Examples](examples.md)

## Getting Help

- [GitHub Issues](https://github.com/muhammad-fiaz/crest/issues)
- [Email Support](mailto:contact@muhammadfiaz.com)

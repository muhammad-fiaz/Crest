# Crest Configuration Guide

Complete guide to configuring your Crest application.

## Table of Contents

- [Basic Configuration](#basic-configuration)
- [Application Settings](#application-settings)
- [Server Settings](#server-settings)
- [Documentation Settings](#documentation-settings)
- [Network Settings](#network-settings)
- [Production Configuration](#production-configuration)
- [Environment-Specific Configuration](#environment-specific-configuration)

## Basic Configuration

### C++ Configuration

```cpp
#include "crest/crest.hpp"

crest::Config config;
config.title = "My API";
config.description = "API Description";
config.version = "1.0.0";

crest::App app(config);
```

### C Configuration

```c
#include "crest/crest.h"

crest_config_t config = {
    .title = "My API",
    .description = "API Description",
    .version = "1.0.0",
    .docs_enabled = true
};

crest_app_t* app = crest_create_with_config(&config);
```

## Application Settings

### Title

Set the application title displayed in documentation.

**C++:**
```cpp
config.title = "My Awesome API";
// or
app.set_title("My Awesome API");
```

**C:**
```c
crest_set_title(app, "My Awesome API");
```

### Description

Set the application description displayed in documentation.

**C++:**
```cpp
config.description = "A powerful RESTful API for managing resources";
// or
app.set_description("A powerful RESTful API for managing resources");
```

**C:**
```c
crest_set_description(app, "A powerful RESTful API for managing resources");
```

### Version

Set the API version.

**C++:**
```cpp
config.version = "1.0.0";
```

**C:**
```c
config.version = "1.0.0";
```

## Server Settings

### Host and Port

Configure the server host and port.

**C++:**
```cpp
// Default: 0.0.0.0:8000
app.run("0.0.0.0", 8000);

// Localhost only
app.run("127.0.0.1", 3000);

// Specific interface
app.run("192.168.1.100", 8080);
```

**C:**
```c
crest_run(app, "0.0.0.0", 8000);
```

### Connection Limits

**C++:**
```cpp
config.max_connections = 1000;  // Maximum concurrent connections
```

### Timeout

**C++:**
```cpp
config.timeout_seconds = 30;  // Request timeout in seconds
```

## Documentation Settings

### Enable/Disable Documentation

**C++:**
```cpp
// In config
config.docs_enabled = true;

// Or dynamically
app.set_docs_enabled(true);
```

**C:**
```c
crest_set_docs_enabled(app, true);
```

### Custom Documentation Paths

**C++:**
```cpp
config.docs_path = "/api-docs";        // Default: /docs
config.openapi_path = "/api-spec.json"; // Default: /openapi.json
```

### Documentation Example

```cpp
crest::Config config;
config.title = "E-Commerce API";
config.description = "RESTful API for e-commerce platform";
config.version = "2.1.0";
config.docs_enabled = true;
config.docs_path = "/documentation";
config.openapi_path = "/api-spec.json";

crest::App app(config);
```

## Network Settings

### Proxy Configuration

Configure proxy settings for outbound requests.

**C++:**
```cpp
config.proxy_url = "http://proxy.company.com:8080";
// or
app.set_proxy("http://proxy.company.com:8080");
```

**C:**
```c
crest_set_proxy(app, "http://proxy.company.com:8080");
```

### Proxy with Authentication

```cpp
app.set_proxy("http://username:password@proxy.company.com:8080");
```

## Production Configuration

### Recommended Production Settings

**C++:**
```cpp
crest::Config config;
config.title = "Production API";
config.description = "Production RESTful API";
config.version = "1.0.0";
config.docs_enabled = false;           // Disable docs in production
config.max_connections = 2000;         // Higher connection limit
config.timeout_seconds = 60;           // Longer timeout

crest::App app(config);
app.run("0.0.0.0", 8000);
```

### Security Considerations

1. **Disable Documentation**
   ```cpp
   app.set_docs_enabled(false);
   ```

2. **Bind to Specific Interface**
   ```cpp
   app.run("127.0.0.1", 8000);  // Localhost only
   ```

3. **Use Reverse Proxy**
   - Deploy behind nginx or Apache
   - Let reverse proxy handle SSL/TLS
   - Configure rate limiting at proxy level

## Environment-Specific Configuration

### Using Environment Variables

**C++:**
```cpp
#include <cstdlib>

std::string getEnv(const char* key, const char* default_value) {
    const char* val = std::getenv(key);
    return val ? std::string(val) : std::string(default_value);
}

int main() {
    crest::Config config;
    config.title = getEnv("API_TITLE", "My API");
    config.version = getEnv("API_VERSION", "1.0.0");
    
    bool is_production = getEnv("ENVIRONMENT", "development") == "production";
    config.docs_enabled = !is_production;
    
    crest::App app(config);
    
    std::string host = getEnv("HOST", "0.0.0.0");
    int port = std::stoi(getEnv("PORT", "8000"));
    
    app.run(host, port);
    return 0;
}
```

### Development Configuration

```cpp
crest::Config dev_config;
dev_config.title = "Dev API";
dev_config.docs_enabled = true;
dev_config.max_connections = 100;
dev_config.timeout_seconds = 30;

crest::App app(dev_config);
app.run("127.0.0.1", 8000);
```

### Staging Configuration

```cpp
crest::Config staging_config;
staging_config.title = "Staging API";
staging_config.docs_enabled = true;  // Keep docs for testing
staging_config.max_connections = 500;
staging_config.timeout_seconds = 45;

crest::App app(staging_config);
app.run("0.0.0.0", 8000);
```

### Production Configuration

```cpp
crest::Config prod_config;
prod_config.title = "Production API";
prod_config.docs_enabled = false;  // Disable docs
prod_config.max_connections = 2000;
prod_config.timeout_seconds = 60;

crest::App app(prod_config);
app.run("0.0.0.0", 8000);
```

## Configuration File Example

### JSON Configuration (Custom Implementation)

```json
{
  "app": {
    "title": "My API",
    "description": "RESTful API",
    "version": "1.0.0"
  },
  "server": {
    "host": "0.0.0.0",
    "port": 8000,
    "max_connections": 1000,
    "timeout_seconds": 30
  },
  "docs": {
    "enabled": true,
    "path": "/docs",
    "openapi_path": "/openapi.json"
  },
  "network": {
    "proxy_url": ""
  }
}
```

### Loading Configuration

```cpp
#include "crest/crest.hpp"
#include <fstream>
#include <nlohmann/json.hpp>  // Example JSON library

crest::Config load_config(const std::string& filename) {
    std::ifstream file(filename);
    nlohmann::json j;
    file >> j;
    
    crest::Config config;
    config.title = j["app"]["title"];
    config.description = j["app"]["description"];
    config.version = j["app"]["version"];
    config.docs_enabled = j["docs"]["enabled"];
    config.docs_path = j["docs"]["path"];
    config.openapi_path = j["docs"]["openapi_path"];
    config.max_connections = j["server"]["max_connections"];
    config.timeout_seconds = j["server"]["timeout_seconds"];
    
    if (!j["network"]["proxy_url"].empty()) {
        config.proxy_url = j["network"]["proxy_url"];
    }
    
    return config;
}

int main() {
    crest::Config config = load_config("config.json");
    crest::App app(config);
    
    // Load server settings
    std::ifstream file("config.json");
    nlohmann::json j;
    file >> j;
    
    app.run(j["server"]["host"], j["server"]["port"]);
    return 0;
}
```

## Complete Configuration Example

```cpp
#include "crest/crest.hpp"
#include <iostream>
#include <cstdlib>

int main() {
    // Determine environment
    const char* env = std::getenv("ENVIRONMENT");
    std::string environment = env ? env : "development";
    
    // Create configuration
    crest::Config config;
    config.title = "My API";
    config.description = "Production-ready RESTful API";
    config.version = "1.0.0";
    
    // Environment-specific settings
    if (environment == "production") {
        config.docs_enabled = false;
        config.max_connections = 2000;
        config.timeout_seconds = 60;
    } else if (environment == "staging") {
        config.docs_enabled = true;
        config.max_connections = 500;
        config.timeout_seconds = 45;
    } else {  // development
        config.docs_enabled = true;
        config.max_connections = 100;
        config.timeout_seconds = 30;
    }
    
    // Create app
    crest::App app(config);
    
    // Register routes
    app.get("/", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"status":"ok"})");
    });
    
    // Get host and port from environment
    const char* host_env = std::getenv("HOST");
    const char* port_env = std::getenv("PORT");
    
    std::string host = host_env ? host_env : "0.0.0.0";
    int port = port_env ? std::stoi(port_env) : 8000;
    
    // Start server
    std::cout << "Starting server in " << environment << " mode\n";
    std::cout << "Listening on " << host << ":" << port << "\n";
    
    try {
        app.run(host, port);
    } catch (const crest::Exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
```

## Configuration Best Practices

1. **Use Environment Variables** for deployment-specific settings
2. **Disable Documentation** in production environments
3. **Set Appropriate Timeouts** based on your use case
4. **Configure Connection Limits** based on expected load
5. **Use Configuration Files** for complex setups
6. **Validate Configuration** before starting the server
7. **Document Your Configuration** for team members
8. **Version Your Configuration** files
9. **Use Secrets Management** for sensitive data
10. **Test Configuration** in staging before production

## Troubleshooting

### Port Already in Use

```cpp
try {
    app.run("0.0.0.0", 8000);
} catch (const crest::Exception& e) {
    std::cerr << "Failed to start: " << e.what() << "\n";
    std::cerr << "Try a different port\n";
}
```

### Permission Denied (Port < 1024)

On Unix systems, ports below 1024 require root privileges:

```bash
# Use a higher port
app.run("0.0.0.0", 8000);

# Or run with sudo (not recommended)
sudo ./my_app
```

### Configuration Not Applied

Ensure configuration is set before calling run():

```cpp
crest::App app(config);  // Config applied here
app.set_title("New Title");  // Can still modify
app.run("0.0.0.0", 8000);  // Start with final config
```

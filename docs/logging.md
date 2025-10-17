# Crest Logging System

Complete guide to Crest's colorful console logging system.

## Overview

Crest includes a built-in logging system with colored output, timestamps, and request logging.

## Features

- ✅ Color-coded log levels
- ✅ Timestamps (optional)
- ✅ Request logging
- ✅ Enable/disable logging
- ✅ Cross-platform (Windows, Linux, macOS)
- ✅ No emoji encoding issues

## Log Levels

### INFO
Blue colored informational messages.

```c
crest_log_info("Server configuration loaded");
```

Output:
```
[2024-01-15 10:30:45] [INFO] Server configuration loaded
```

### SUCCESS
Green colored success messages.

```c
crest_log_success("Server started successfully");
```

Output:
```
[2024-01-15 10:30:45] [SUCCESS] Server started successfully
```

### ERROR
Red colored error messages.

```c
crest_log_error("Failed to bind to port");
```

Output:
```
[2024-01-15 10:30:45] [ERROR] Failed to bind to port
```

### WARNING
Yellow colored warning messages.

```c
crest_log_warning("Configuration file not found, using defaults");
```

Output:
```
[2024-01-15 10:30:45] [WARNING] Configuration file not found, using defaults
```

### REQUEST
Color-coded HTTP request logs.

```c
crest_log_request("GET", "/api/users", 200);  // Green
crest_log_request("POST", "/api/users", 404); // Red
```

Output:
```
[2024-01-15 10:30:45] [REQUEST] GET /api/users -> 200
[2024-01-15 10:30:46] [REQUEST] POST /api/users -> 404
```

## Configuration

### Disable Logging

**C:**
```c
crest_log_set_enabled(false);
```

**C++:**
```cpp
crest::App::set_logging_enabled(false);
```

### Disable Timestamps

**C:**
```c
crest_log_set_timestamp(false);
```

**C++:**
```cpp
crest::App::set_timestamp_enabled(false);
```

## Automatic Request Logging

Crest automatically logs all HTTP requests:

```
[2024-01-15 10:30:45] [REQUEST] GET / -> 200
[2024-01-15 10:30:46] [REQUEST] POST /users -> 201
[2024-01-15 10:30:47] [REQUEST] GET /notfound -> 404
```

Color coding:
- **Green**: 2xx status codes (success)
- **Red**: 4xx/5xx status codes (errors)
- **Blue**: Other status codes

## Example Usage

### C Example

```c
#include "crest/crest.h"

int main(void) {
    // Configure logging
    crest_log_set_enabled(true);
    crest_log_set_timestamp(true);
    
    // Create app
    crest_app_t* app = crest_create();
    
    // Logs are automatic
    crest_run(app, "127.0.0.1", 8000);
    
    crest_destroy(app);
    return 0;
}
```

Output:
```
[2024-01-15 10:30:45] [SUCCESS] Crest server running on http://127.0.0.1:8000
[2024-01-15 10:30:45] [INFO] Documentation: http://127.0.0.1:8000/docs
[2024-01-15 10:30:45] [INFO] Playground: http://127.0.0.1:8000/playground
```

### C++ Example

```cpp
#include "crest/crest.hpp"

int main() {
    // Configure logging
    crest::App::set_logging_enabled(true);
    crest::App::set_timestamp_enabled(true);
    
    crest::App app;
    
    app.get("/", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"status":"ok"})");
    });
    
    app.run("127.0.0.1", 8000);
    return 0;
}
```

## Production Configuration

### Disable Logging in Production

```cpp
#ifdef PRODUCTION
    crest::App::set_logging_enabled(false);
#endif
```

### Minimal Logging

```cpp
// Disable timestamps for cleaner logs
crest::App::set_timestamp_enabled(false);
```

Output:
```
[SUCCESS] Crest server running on http://127.0.0.1:8000
[REQUEST] GET / -> 200
```

## Platform-Specific Behavior

### Windows
- Uses Windows Console API for colors
- SetConsoleTextAttribute for color control
- No ANSI escape codes needed

### Linux/macOS
- Uses ANSI escape codes
- Full color support in terminal
- Works with all standard terminals

## Color Reference

| Level | Windows Color | Unix Color | Code |
|-------|--------------|------------|------|
| INFO | Blue | Blue | FOREGROUND_BLUE |
| SUCCESS | Green | Green | FOREGROUND_GREEN |
| ERROR | Red | Red | FOREGROUND_RED |
| WARNING | Yellow | Yellow | FOREGROUND_RED + GREEN |
| REQUEST (2xx) | Green | Green | - |
| REQUEST (4xx/5xx) | Red | Red | - |

## Best Practices

1. **Enable in Development**
   ```cpp
   crest::App::set_logging_enabled(true);
   crest::App::set_timestamp_enabled(true);
   ```

2. **Disable in Production**
   ```cpp
   crest::App::set_logging_enabled(false);
   ```

3. **Use Timestamps for Debugging**
   ```cpp
   crest::App::set_timestamp_enabled(true);
   ```

4. **Monitor Request Logs**
   - Watch for 4xx/5xx errors (red)
   - Track response times
   - Identify traffic patterns

## Troubleshooting

### No Colors on Windows
- Ensure Windows 10+ or Windows Terminal
- Colors work automatically with Windows Console API

### Garbled Output
- Fixed in latest version
- No emoji encoding issues
- Plain text labels only

### Logs Not Appearing
```cpp
// Check if logging is enabled
crest::App::set_logging_enabled(true);
```

## API Reference

### C Functions

```c
void crest_log_info(const char* msg);
void crest_log_success(const char* msg);
void crest_log_error(const char* msg);
void crest_log_warning(const char* msg);
void crest_log_request(const char* method, const char* path, int status);
void crest_log_set_enabled(bool enabled);
void crest_log_set_timestamp(bool enabled);
```

### C++ Methods

```cpp
static void crest::App::set_logging_enabled(bool enabled);
static void crest::App::set_timestamp_enabled(bool enabled);
```

## Examples

### Custom Logging

```c
// In your handler
void my_handler(crest_request_t* req, crest_response_t* res) {
    crest_log_info("Processing request");
    
    // Your logic here
    
    crest_log_success("Request processed successfully");
    crest_response_json(res, 200, "{\"status\":\"ok\"}");
}
```

### Conditional Logging

```cpp
#ifdef DEBUG
    crest::App::set_logging_enabled(true);
    crest::App::set_timestamp_enabled(true);
#else
    crest::App::set_logging_enabled(false);
#endif
```

## Summary

Crest's logging system provides:
- ✅ Colorful, readable console output
- ✅ Automatic request logging
- ✅ Configurable timestamps
- ✅ Enable/disable controls
- ✅ Cross-platform support
- ✅ Production-ready

Perfect for development and debugging! 🌊

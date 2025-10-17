# Crest Schema Documentation

Complete guide to defining and using schemas in Crest for API documentation.

## Overview

Crest supports **custom schema definitions** for request and response bodies. Schemas are displayed in Swagger UI and help document your API's data structures.

**Important**: Schemas must be set manually using `set_request_schema()` and `set_response_schema()`. If not set, Crest uses sensible defaults based on HTTP method.

## Features

- ✅ **Manual Schema Definition** - Set custom schemas for any route
- ✅ **Type Display** - Show types: string, number, boolean, object, array, null
- ✅ **Dynamic Documentation** - Schemas appear in Swagger UI automatically
- ✅ **Request & Response** - Define schemas for both request and response
- ✅ **All HTTP Methods** - Works with GET, POST, PUT, DELETE, PATCH

## Supported Types

| Type | Description | Example Value |
|------|-------------|---------------|
| `string` | Text values | `"hello"`, `"john@example.com"` |
| `number` | Numeric values (int/float) | `42`, `3.14`, `-10` |
| `boolean` | True/false values | `true`, `false` |
| `object` | Nested objects | `{"key": "value"}` |
| `array` | Lists/arrays | `[1, 2, 3]`, `["a", "b"]` |
| `null` | Null value | `null` |

## Setting Schemas

### C++ API

```cpp
// Set response schema
app.set_response_schema(crest::Method::GET, "/user",
    R"({"id": "number", "name": "string", "email": "string"})");

// Set request schema
app.set_request_schema(crest::Method::POST, "/user",
    R"({"name": "string", "email": "string", "age": "number"})");

// Method chaining
app.post("/user", handler, "Create user")
   .set_request_schema(crest::Method::POST, "/user", R"({"name": "string"})")
   .set_response_schema(crest::Method::POST, "/user", R"({"id": "number"})");
```

### C API

```c
// Set response schema
crest_set_response_schema(app, CREST_GET, "/user",
    "{\"id\": \"number\", \"name\": \"string\"}");

// Set request schema
crest_set_request_schema(app, CREST_POST, "/user",
    "{\"name\": \"string\", \"email\": \"string\"}");
```

## Schema Format

Schemas are JSON objects where:
- **Keys** are field names
- **Values** are type strings: `"string"`, `"number"`, `"boolean"`, `"object"`, `"array"`, `"null"`

### Basic Schema
```json
{
  "id": "number",
  "name": "string",
  "active": "boolean"
}
```

### Nested Objects
```json
{
  "user": "object",
  "settings": "object",
  "metadata": "object"
}
```

### Arrays
```json
{
  "tags": "array",
  "items": "array"
}
```

### Mixed Types
```json
{
  "id": "string",
  "count": "number",
  "enabled": "boolean",
  "data": "object",
  "list": "array",
  "optional": "null"
}
```

## Examples

### Example 1: Simple User API

```cpp
crest::App app;

// GET /user - Retrieve user
app.get("/user", [](crest::Request& req, crest::Response& res) {
    res.json(200, R"({
        "id": 123,
        "name": "John Doe",
        "email": "john@example.com",
        "age": 30
    })");
});

app.set_response_schema(crest::Method::GET, "/user",
    R"({"id": "number", "name": "string", "email": "string", "age": "number"})");
```

**Swagger UI Display:**
```
📥 Request Schema
None

📤 Response Schema (200 OK)
{
  "id": "number",
  "name": "string",
  "email": "string",
  "age": "number"
}
```

### Example 2: Create User

```cpp
// POST /user - Create user
app.post("/user", [](crest::Request& req, crest::Response& res) {
    res.json(201, R"({"id": 456, "status": "created"})");
});

app.set_request_schema(crest::Method::POST, "/user",
    R"({"name": "string", "email": "string", "age": "number"})");

app.set_response_schema(crest::Method::POST, "/user",
    R"({"id": "number", "status": "string"})");
```

**Swagger UI Display:**
```
📥 Request Schema
{
  "name": "string",
  "email": "string",
  "age": "number"
}

📤 Response Schema (201 Created)
{
  "id": "number",
  "status": "string"
}
```

### Example 3: Complex Nested Data

```cpp
app.get("/profile", [](crest::Request& req, crest::Response& res) {
    res.json(200, R"({
        "user": {"id": 1, "name": "Alice"},
        "settings": {"theme": "dark", "notifications": true},
        "stats": {"posts": 42, "followers": 1337}
    })");
});

app.set_response_schema(crest::Method::GET, "/profile",
    R"({
        "user": "object",
        "settings": "object",
        "stats": "object"
    })");
```

**Swagger UI Display:**
```
📥 Request Schema
None

📤 Response Schema (200 OK)
{
  "user": "object",
  "settings": "object",
  "stats": "object"
}
```

### Example 4: Array Response

```cpp
app.get("/users", [](crest::Request& req, crest::Response& res) {
    res.json(200, R"([
        {"id": 1, "name": "Alice"},
        {"id": 2, "name": "Bob"}
    ])");
});

app.set_response_schema(crest::Method::GET, "/users",
    R"([{"id": "number", "name": "string"}])");
```

**Swagger UI Display:**
```
📥 Request Schema
None

📤 Response Schema (200 OK)
[
  {
    "id": "number",
    "name": "string"
  }
]
```

### Example 5: All Types

```cpp
app.post("/analytics", [](crest::Request& req, crest::Response& res) {
    res.json(200, R"({
        "event_id": "evt_123",
        "count": 42,
        "percentage": 85.5,
        "active": true,
        "metadata": {"source": "web"},
        "tags": ["analytics", "metrics"],
        "error": null
    })");
});

app.set_response_schema(crest::Method::POST, "/analytics",
    R"({
        "event_id": "string",
        "count": "number",
        "percentage": "number",
        "active": "boolean",
        "metadata": "object",
        "tags": "array",
        "error": "null"
    })");
```

**Swagger UI Display:**
```
📥 Request Schema
{
  "event": "string",
  "user_id": "number",
  "properties": "object"
}

📤 Response Schema (200 OK)
{
  "event_id": "string",
  "count": "number",
  "percentage": "number",
  "active": "boolean",
  "metadata": "object",
  "tags": "array",
  "error": "null"
}
```

## Default Schemas

If you don't set custom schemas, Crest provides sensible defaults:

| Method | Request Schema | Response Schema |
|--------|----------------|-----------------|
| GET | None | `{"data": "string"}` |
| POST | `{"name": "string", "value": "string"}` | `{"id": "number", "status": "string"}` |
| PUT | `{"name": "string", "value": "string"}` | `{"status": "string"}` |
| DELETE | None | `{"status": "string"}` |
| PATCH | `{"field": "string"}` | `{"status": "string"}` |

## Best Practices

### 1. Define Schemas for All Routes
```cpp
// ✅ Good - Clear documentation
app.post("/user", handler)
   .set_request_schema(crest::Method::POST, "/user", R"({"name": "string"})")
   .set_response_schema(crest::Method::POST, "/user", R"({"id": "number"})");

// ❌ Bad - No schema, uses defaults
app.post("/user", handler);
```

### 2. Use Accurate Types
```cpp
// ✅ Good - Correct types
R"({"age": "number", "name": "string", "active": "boolean"})"

// ❌ Bad - Wrong types
R"({"age": "string", "name": "number", "active": "string"})"
```

### 3. Document Nested Objects
```cpp
// ✅ Good - Shows structure
R"({"user": "object", "settings": "object"})"

// ❌ Bad - Too vague
R"({"data": "object"})"
```

### 4. Use Arrays for Lists
```cpp
// ✅ Good - Clear array type
R"({"users": "array", "tags": "array"})"

// ❌ Bad - Unclear
R"({"users": "object"})"
```

### 5. Match Actual Response
```cpp
// ✅ Good - Schema matches response
app.get("/user", [](auto& req, auto& res) {
    res.json(200, R"({"id": 123, "name": "John"})");
});
app.set_response_schema(crest::Method::GET, "/user",
    R"({"id": "number", "name": "string"})");

// ❌ Bad - Schema doesn't match
app.set_response_schema(crest::Method::GET, "/user",
    R"({"email": "string"})");  // Response doesn't have email!
```

## Complete Example

```cpp
#include "crest/crest.hpp"

int main() {
    crest::App app;
    
    // User CRUD API with full schemas
    
    // List users
    app.get("/users", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"([{"id": 1, "name": "Alice"}, {"id": 2, "name": "Bob"}])");
    }, "List all users");
    app.set_response_schema(crest::Method::GET, "/users",
        R"([{"id": "number", "name": "string"}])");
    
    // Get user
    app.get("/user", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"id": 1, "name": "Alice", "email": "alice@example.com"})");
    }, "Get user by ID");
    app.set_response_schema(crest::Method::GET, "/user",
        R"({"id": "number", "name": "string", "email": "string"})");
    
    // Create user
    app.post("/user", [](crest::Request& req, crest::Response& res) {
        res.json(201, R"({"id": 3, "status": "created"})");
    }, "Create new user");
    app.set_request_schema(crest::Method::POST, "/user",
        R"({"name": "string", "email": "string"})");
    app.set_response_schema(crest::Method::POST, "/user",
        R"({"id": "number", "status": "string"})");
    
    // Update user
    app.put("/user", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"updated": true})");
    }, "Update user");
    app.set_request_schema(crest::Method::PUT, "/user",
        R"({"name": "string", "email": "string"})");
    app.set_response_schema(crest::Method::PUT, "/user",
        R"({"updated": "boolean"})");
    
    // Delete user
    app.del("/user", [](crest::Request& req, crest::Response& res) {
        res.json(200, R"({"deleted": true})");
    }, "Delete user");
    app.set_response_schema(crest::Method::DELETE, "/user",
        R"({"deleted": "boolean"})");
    
    app.run("0.0.0.0", 8000);
    return 0;
}
```

## Testing

### Run Schema Example
```bash
xmake run crest_schema_example
# Visit http://localhost:8000/docs
```

### Verify Schemas
1. Open http://localhost:8000/docs
2. Click any endpoint to expand
3. View "📥 Request Schema" section
4. View "📤 Response Schema (200 OK)" section
5. Verify types match your definitions

## Summary

Crest schema system provides:
- ✅ **Manual schema definition** for all routes
- ✅ **Type display**: string, number, boolean, object, array, null
- ✅ **Automatic documentation** in Swagger UI
- ✅ **Request & response** schemas
- ✅ **Default schemas** for convenience
- ✅ **Method chaining** support

**Perfect for documenting your API's data structures!** 🌊

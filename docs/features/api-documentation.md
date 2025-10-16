# API Documentation

Crest provides production-ready API documentation with three endpoints that automatically generate interactive documentation from your registered routes.

## Endpoints

| Endpoint | Type | Description |
|----------|------|-------------|
| `/docs` | 🧠 Swagger UI | Interactive API testing dashboard with full OpenAPI 3.0 support |
| `/redoc` | ⚙️ ReDoc | Clean, read-only API documentation interface |
| `/openapi.json` | 🧾 OpenAPI JSON | Machine-readable OpenAPI 3.0.3 specification |

## Usage

Enable API documentation by calling `crest_enable_dashboard(app, true)`:

```c
#include <crest/crest.h>

int main(void) {
    crest_app_t *app = crest_create();

    // Enable API documentation
    crest_enable_dashboard(app, true);

    // Register your routes
    crest_get(app, "/users", get_users_handler, "Get all users");
    crest_post(app, "/users", create_user_handler, "Create a new user");
    crest_get(app, "/users/:id", get_user_handler, "Get user by ID");
    crest_put(app, "/users/:id", update_user_handler, "Update user");
    crest_delete(app, "/users/:id", delete_user_handler, "Delete user");

    crest_listen(app);
    crest_destroy(app);
    return 0;
}
```

## Features

### Automatic Route Discovery
- **Zero Configuration**: Documentation is generated automatically from your route registrations
- **Path Parameters**: Automatically detects and documents path parameters like `:id`
- **Request Bodies**: Generates schemas for POST/PUT/PATCH endpoints
- **Response Schemas**: Standard response formats with status codes (200, 201, 400, 404, 500)

### Swagger UI (`/docs`)
- **Interactive Testing**: Try out API endpoints directly from the browser
- **Authentication Support**: JWT Bearer token and API key authentication
- **Request/Response Examples**: Pre-filled examples for easy testing
- **Real-time Validation**: Client-side validation of request parameters

### ReDoc (`/redoc`)
- **Clean Interface**: Professional documentation layout
- **Mobile Friendly**: Responsive design that works on all devices
- **Search Functionality**: Find endpoints quickly
- **Print Friendly**: Clean printing for offline reference

### OpenAPI 3.0.3 Specification (`/openapi.json`)
- **Machine Readable**: Perfect for code generation and API clients
- **Industry Standard**: Compatible with all OpenAPI 3.0 tools
- **Complete Schema**: Includes all routes, parameters, and responses
- **Security Schemes**: JWT and API key authentication definitions

## Reserved Routes

When API documentation is enabled, the following routes are automatically reserved and cannot be used by your application:

- `/docs` - Swagger UI interface
- `/redoc` - ReDoc interface
- `/openapi.json` - OpenAPI specification
- `/dashboard` - Legacy dashboard (deprecated)
- `/api/routes` - Route listing endpoint

## Security

API documentation endpoints include CORS headers and are designed to be safely exposed in production environments. The documentation only exposes route information and does not include sensitive configuration details.

## Examples

### Basic API with Documentation

```c
crest_app_t *app = crest_create();

// Enable documentation
crest_enable_dashboard(app, true);

// Your API routes
crest_get(app, "/products", list_products, "List all products");
crest_get(app, "/products/:id", get_product, "Get product by ID");
crest_post(app, "/products", create_product, "Create a new product");
crest_put(app, "/products/:id", update_product, "Update product");
crest_delete(app, "/products/:id", delete_product, "Delete product");

// Start server
crest_listen(app);
```

### Accessing Documentation

Once your server is running:

- **Swagger UI**: Visit `http://localhost:8080/docs`
- **ReDoc**: Visit `http://localhost:8080/redoc`
- **OpenAPI JSON**: Visit `http://localhost:8080/openapi.json`



## Integration

API documentation integrates seamlessly with Crest's features:

- **Middleware**: Works with authentication, CORS, and logging middleware
- **Route Groups**: Documents grouped routes appropriately
- **Custom Responses**: Respects custom response formats
- **Error Handling**: Documents error response schemas

## Performance

API documentation generation is optimized for performance:

- **Lazy Generation**: OpenAPI spec is generated on-demand
- **Caching**: Generated specs can be cached for better performance
- **Minimal Overhead**: Documentation endpoints have negligible impact on API performance
- **Memory Efficient**: Uses dynamic buffer allocation with growth limits</content>
<parameter name="filePath">e:\Projects\Crest\docs\features\api-documentation.md
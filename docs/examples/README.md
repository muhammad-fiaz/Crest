# Crest API Documentation Examples

This directory contains comprehensive examples demonstrating all aspects of Crest's API documentation features. Each example includes code samples, testing commands, expected outputs, and detailed explanations.

## Available Examples

### [basic-api-documentation.md](basic-api-documentation.md)
**Basic API Documentation Setup**
- Simple Crest application with API documentation enabled
- Basic routes and handlers
- Swagger UI, ReDoc, and OpenAPI JSON endpoints
- Testing with cURL and PowerShell
- Expected JSON responses and validation

### [advanced-api-documentation.md](advanced-api-documentation.md)
**Advanced Features and Authentication**
- JWT and API key authentication middleware
- CORS support and request handling
- Comprehensive testing suite with error responses
- Performance testing endpoints
- Rate limiting and pagination examples

### [integration-examples.md](integration-examples.md)
**Real-World Integration Patterns**
- Database integration with SQLite
- Configuration management with environment variables
- Logging and monitoring middleware
- Docker deployment with docker-compose
- Nginx reverse proxy setup

### [testing-examples.md](testing-examples.md)
**Comprehensive Testing Strategies**
- Unit tests with mock objects
- Integration tests with HTTP requests
- API contract testing and validation
- Automated testing pipelines (Makefile, GitHub Actions)
- Performance and load testing
- Memory leak detection

## Quick Start

1. **Enable API Documentation** in your Crest application:
   ```c
   crest_app_t *app = crest_create();
   crest_enable_dashboard(app, true); // Enables /docs, /redoc, /openapi.json
   ```

2. **Access Documentation**:
   - **Swagger UI**: `http://localhost:8080/docs`
   - **ReDoc**: `http://localhost:8080/redoc`
   - **OpenAPI JSON**: `http://localhost:8080/openapi.json`

3. **Test Your API**:
   ```bash
   # Health check
   curl http://localhost:8080/health

   # Get OpenAPI specification
   curl http://localhost:8080/openapi.json

   # View interactive documentation
   open http://localhost:8080/docs
   ```

## Key Features Demonstrated

### 🔧 **Core Functionality**
- Automatic OpenAPI 3.0.3 specification generation
- Swagger UI integration with custom branding
- ReDoc integration with custom branding
- Reserved routes that cannot be overridden by users
- Dynamic route registration and documentation

### 🔐 **Security & Authentication**
- JWT bearer token authentication
- API key authentication
- CORS middleware
- Request validation and error handling

### 📊 **Monitoring & Observability**
- Request/response logging middleware
- Performance timing middleware
- Metrics endpoints
- Health check endpoints

### 🗄️ **Data Integration**
- SQLite database integration
- CRUD operations with proper error handling
- Data validation and sanitization
- Connection management

### ⚙️ **Configuration Management**
- Environment variable configuration
- Runtime configuration validation
- Feature flags and conditional setup
- Configuration endpoint for runtime inspection

### 🐳 **Deployment & DevOps**
- Docker containerization
- Docker Compose orchestration
- Nginx reverse proxy configuration
- Health checks and monitoring
- Production-ready deployment patterns

### 🧪 **Testing & Quality Assurance**
- Unit testing with mock objects
- Integration testing with real HTTP requests
- API contract testing and schema validation
- Automated CI/CD pipelines
- Performance and load testing
- Memory leak detection

## Example Application Structure

```
your-app/
├── src/
│   ├── main.c              # Main application entry point
│   ├── handlers.c          # Route handlers
│   ├── middleware.c        # Custom middleware
│   └── database.c          # Database operations
├── tests/
│   ├── test_unit.c         # Unit tests
│   ├── test_integration.c  # Integration tests
│   └── test_contract.c     # Contract tests
├── Dockerfile              # Container definition
├── docker-compose.yml      # Orchestration
├── Makefile               # Build and test automation
└── README.md              # Documentation
```

## Common Patterns

### Basic Route Registration
```c
crest_get(app, "/users", get_users_handler, "Get all users");
crest_post(app, "/users", create_user_handler, "Create a new user");
crest_get(app, "/users/:id", get_user_handler, "Get user by ID");
```

### Middleware Setup
```c
crest_use(app, "/*", logging_middleware, NULL);
crest_use(app, "/api/*", auth_middleware, NULL);
crest_use(app, "/api/*", cors_middleware, NULL);
```

### Error Handling
```c
if (!valid_input) {
    crest_response_status(res, 400);
    crest_response_json(res, "{\"error\": \"Invalid input\", \"code\": \"VALIDATION_ERROR\"}");
    return;
}
```

### Database Operations
```c
sqlite3_stmt *stmt;
const char *sql = "SELECT * FROM users WHERE id = ?";
sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
sqlite3_bind_int(stmt, 1, user_id);
// Execute and process results
```

## Testing Commands

### Manual Testing
```bash
# Test all endpoints
curl http://localhost:8080/health
curl http://localhost:8080/api/users
curl http://localhost:8080/openapi.json

# Test with authentication
curl -H "Authorization: Bearer <token>" http://localhost:8080/api/profile

# Test error responses
curl http://localhost:8080/api/invalid-endpoint
```

### Automated Testing
```bash
# Run all tests
make test

# Run specific test suites
make test-unit
make test-integration
make test-contract

# Performance testing
ab -n 1000 -c 10 http://localhost:8080/api/test
```

## Production Considerations

- **Security**: Always validate input, use HTTPS, implement proper authentication
- **Performance**: Use connection pooling, implement caching, monitor memory usage
- **Monitoring**: Log requests/responses, implement health checks, use metrics
- **Scalability**: Consider load balancing, database optimization, async processing
- **Documentation**: Keep API docs updated, provide examples, document breaking changes

## Support

For questions about these examples or Crest's API documentation features:

1. Check the main [API Documentation](../features/api-documentation.md) guide
2. Review the [MkDocs documentation](../../mkdocs.yml) structure
3. Test with the provided examples and modify for your use case
4. Check the [README.md](../../README.md) for endpoint references

All examples are production-ready and demonstrate best practices for building REST APIs with comprehensive documentation using Crest.
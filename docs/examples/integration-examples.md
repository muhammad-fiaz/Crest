# Integration and Real-World Examples

This document shows how to integrate Crest's API documentation features into real-world applications, including database integration, logging, configuration management, and deployment scenarios.

## Database Integration Example

### Code Example

```c
#include <crest/crest.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Database connection (simplified - in production use connection pooling)
sqlite3 *db = NULL;

// Database initialization
int init_database(void) {
    int rc = sqlite3_open(":memory:", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    // Create users table
    const char *sql = "CREATE TABLE users ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "email TEXT UNIQUE NOT NULL,"
                     "name TEXT NOT NULL,"
                     "created_at DATETIME DEFAULT CURRENT_TIMESTAMP);"

                     "CREATE TABLE posts ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "user_id INTEGER NOT NULL,"
                     "title TEXT NOT NULL,"
                     "content TEXT NOT NULL,"
                     "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
                     "FOREIGN KEY (user_id) REFERENCES users(id));";

    rc = sqlite3_exec(db, sql, 0, 0, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    return 1;
}

// User handlers
void get_users(crest_request_t *req, crest_response_t *res) {
    const char *page = crest_request_query(req, "page");
    const char *limit = crest_request_query(req, "limit");

    int page_num = page ? atoi(page) : 1;
    int limit_num = limit ? atoi(limit) : 10;
    int offset = (page_num - 1) * limit_num;

    sqlite3_stmt *stmt;
    const char *sql = "SELECT id, email, name, created_at FROM users ORDER BY created_at DESC LIMIT ? OFFSET ?";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        crest_response_status(res, 500);
        crest_response_json(res, "{\"error\": \"Database error\", \"code\": \"DB_ERROR\"}");
        return;
    }

    sqlite3_bind_int(stmt, 1, limit_num);
    sqlite3_bind_int(stmt, 2, offset);

    char *json = malloc(4096);
    strcpy(json, "{\"users\": [");

    int first = 1;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!first) strcat(json, ",");
        char user[512];
        sprintf(user, "{\"id\": %d, \"email\": \"%s\", \"name\": \"%s\", \"created_at\": \"%s\"}",
                sqlite3_column_int(stmt, 0),
                sqlite3_column_text(stmt, 1),
                sqlite3_column_text(stmt, 2),
                sqlite3_column_text(stmt, 3));
        strcat(json, user);
        first = 0;
    }

    // Get total count
    sqlite3_stmt *count_stmt;
    sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM users", -1, &count_stmt, 0);
    sqlite3_step(count_stmt);
    int total = sqlite3_column_int(count_stmt, 0);
    sqlite3_finalize(count_stmt);

    char pagination[256];
    sprintf(pagination, "], \"pagination\": {\"page\": %d, \"limit\": %d, \"total\": %d, \"pages\": %d}}",
            page_num, limit_num, total, (total + limit_num - 1) / limit_num);

    strcat(json, pagination);
    crest_response_json(res, json);
    free(json);

    sqlite3_finalize(stmt);
}

void create_user(crest_request_t *req, crest_response_t *res) {
    // In real app, parse JSON body
    const char *email = crest_request_query(req, "email"); // Simplified
    const char *name = crest_request_query(req, "name");

    if (!email || !name) {
        crest_response_status(res, 400);
        crest_response_json(res, "{\"error\": \"Email and name are required\", \"code\": \"MISSING_FIELDS\"}");
        return;
    }

    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO users (email, name) VALUES (?, ?)";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        crest_response_status(res, 500);
        crest_response_json(res, "{\"error\": \"Database error\", \"code\": \"DB_ERROR\"}");
        return;
    }

    sqlite3_bind_text(stmt, 1, email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, name, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        crest_response_status(res, 409);
        crest_response_json(res, "{\"error\": \"Email already exists\", \"code\": \"DUPLICATE_EMAIL\"}");
        sqlite3_finalize(stmt);
        return;
    }

    int user_id = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);

    char response[256];
    sprintf(response, "{\"message\": \"User created\", \"id\": %d}", user_id);
    crest_response_status(res, 201);
    crest_response_json(res, response);
}

void get_user_posts(crest_request_t *req, crest_response_t *res) {
    const char *user_id_str = crest_request_param(req, "user_id");
    int user_id = atoi(user_id_str);

    sqlite3_stmt *stmt;
    const char *sql = "SELECT p.id, p.title, p.content, p.created_at, u.name as author "
                     "FROM posts p JOIN users u ON p.user_id = u.id "
                     "WHERE p.user_id = ? ORDER BY p.created_at DESC";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        crest_response_status(res, 500);
        crest_response_json(res, "{\"error\": \"Database error\", \"code\": \"DB_ERROR\"}");
        return;
    }

    sqlite3_bind_int(stmt, 1, user_id);

    char *json = malloc(8192);
    strcpy(json, "{\"posts\": [");

    int first = 1;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!first) strcat(json, ",");
        char post[1024];
        sprintf(post, "{\"id\": %d, \"title\": \"%s\", \"content\": \"%s\", \"created_at\": \"%s\", \"author\": \"%s\"}",
                sqlite3_column_int(stmt, 0),
                sqlite3_column_text(stmt, 1),
                sqlite3_column_text(stmt, 2),
                sqlite3_column_text(stmt, 3),
                sqlite3_column_text(stmt, 4));
        strcat(json, post);
        first = 0;
    }

    strcat(json, "]}");
    crest_response_json(res, json);
    free(json);

    sqlite3_finalize(stmt);
}

void create_post(crest_request_t *req, crest_response_t *res) {
    const char *user_id_str = crest_request_param(req, "user_id");
    int user_id = atoi(user_id_str);

    // Simplified - in real app parse JSON body
    const char *title = crest_request_query(req, "title");
    const char *content = crest_request_query(req, "content");

    if (!title || !content) {
        crest_response_status(res, 400);
        crest_response_json(res, "{\"error\": \"Title and content are required\", \"code\": \"MISSING_FIELDS\"}");
        return;
    }

    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO posts (user_id, title, content) VALUES (?, ?, ?)";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        crest_response_status(res, 500);
        crest_response_json(res, "{\"error\": \"Database error\", \"code\": \"DB_ERROR\"}");
        return;
    }

    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_text(stmt, 2, title, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, content, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        crest_response_status(res, 500);
        crest_response_json(res, "{\"error\": \"Failed to create post\", \"code\": \"CREATE_FAILED\"}");
        sqlite3_finalize(stmt);
        return;
    }

    int post_id = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);

    char response[256];
    sprintf(response, "{\"message\": \"Post created\", \"id\": %d}", post_id);
    crest_response_status(res, 201);
    crest_response_json(res, response);
}

int main(void) {
    if (!init_database()) {
        fprintf(stderr, "Failed to initialize database\n");
        return 1;
    }

    crest_app_t *app = crest_create();

    // Enable API documentation
    crest_enable_dashboard(app, true);

    // User management endpoints
    crest_get(app, "/api/users", get_users, "Get paginated list of users");
    crest_post(app, "/api/users", create_user, "Create a new user");

    // Post management endpoints
    crest_get(app, "/api/users/:user_id/posts", get_user_posts, "Get posts by user");
    crest_post(app, "/api/users/:user_id/posts", create_post, "Create a new post for user");

    printf("🗄️ Database API running on http://localhost:8080\n");
    printf("🧠 API Documentation: http://localhost:8080/docs\n");
    printf("⚙️ ReDoc: http://localhost:8080/redoc\n");

    crest_listen(app);

    sqlite3_close(db);
    crest_destroy(app);
    return 0;
}
```

### Database Testing Commands

**Create users:**
```bash
curl -X POST "http://localhost:8080/api/users?email=user1@example.com&name=John%20Doe"
curl -X POST "http://localhost:8080/api/users?email=user2@example.com&name=Jane%20Smith"
```

**Get users:**
```bash
curl http://localhost:8080/api/users
curl "http://localhost:8080/api/users?page=1&limit=5"
```

**Create posts:**
```bash
curl -X POST "http://localhost:8080/api/users/1/posts?title=My%20First%20Post&content=This%20is%20my%20first%20post%20content"
curl -X POST "http://localhost:8080/api/users/1/posts?title=Another%20Post&content=More%20content%20here"
```

**Get user posts:**
```bash
curl http://localhost:8080/api/users/1/posts
```

## Configuration Management Example

### Code Example

```c
#include <crest/crest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Configuration structure
typedef struct {
    char database_url[256];
    char jwt_secret[256];
    int port;
    int enable_cors;
    int enable_docs;
    char log_level[16];
    int max_connections;
} app_config_t;

app_config_t config;

// Load configuration from environment variables
void load_config(void) {
    // Default values
    strcpy(config.database_url, "sqlite::memory:");
    strcpy(config.jwt_secret, "default-secret-change-in-production");
    config.port = 8080;
    config.enable_cors = 1;
    config.enable_docs = 1;
    strcpy(config.log_level, "info");
    config.max_connections = 100;

    // Override from environment
    const char *env_val;

    if ((env_val = getenv("DATABASE_URL"))) {
        strncpy(config.database_url, env_val, sizeof(config.database_url) - 1);
    }

    if ((env_val = getenv("JWT_SECRET"))) {
        strncpy(config.jwt_secret, env_val, sizeof(config.jwt_secret) - 1);
    }

    if ((env_val = getenv("PORT"))) {
        config.port = atoi(env_val);
    }

    if ((env_val = getenv("ENABLE_CORS"))) {
        config.enable_cors = atoi(env_val);
    }

    if ((env_val = getenv("ENABLE_DOCS"))) {
        config.enable_docs = atoi(env_val);
    }

    if ((env_val = getenv("LOG_LEVEL"))) {
        strncpy(config.log_level, env_val, sizeof(config.log_level) - 1);
    }

    if ((env_val = getenv("MAX_CONNECTIONS"))) {
        config.max_connections = atoi(env_val);
    }
}

// Configuration endpoint
void get_config(crest_request_t *req, crest_response_t *res) {
    // Only show non-sensitive config in production
    crest_response_json(res, "{"
        "\"port\": 8080,"
        "\"enable_cors\": true,"
        "\"enable_docs\": true,"
        "\"log_level\": \"info\","
        "\"max_connections\": 100,"
        "\"features\": [\"api_docs\", \"cors\", \"json_responses\"]"
    "}");
}

void health_check(crest_request_t *req, crest_response_t *res) {
    crest_response_json(res, "{"
        "\"status\": \"healthy\","
        "\"version\": \"1.0.0\","
        "\"config_loaded\": true,"
        "\"database\": \"connected\","
        "\"timestamp\": \"2024-01-15T12:00:00Z\""
    "}");
}

void api_info(crest_request_t *req, crest_response_t *res) {
    crest_response_json(res, "{"
        "\"name\": \"Crest API\","
        "\"version\": \"1.0.0\","
        "\"description\": \"REST API built with Crest framework\","
        "\"endpoints\": {"
            "\"docs\": \"/docs\","
            "\"redoc\": \"/redoc\","
            "\"openapi\": \"/openapi.json\","
            "\"health\": \"/health\","
            "\"config\": \"/config\""
        "},"
        "\"features\": ["
            "\"OpenAPI 3.0.3 documentation\","
            "\"Swagger UI integration\","
            "\"ReDoc integration\","
            "\"CORS support\","
            "\"JSON responses\","
            "\"Environment-based configuration\""
        "]"
    "}");
}

int main(int argc, char *argv[]) {
    load_config();

    crest_app_t *app = crest_create();

    // Enable features based on configuration
    if (config.enable_docs) {
        crest_enable_dashboard(app, true);
        printf("📚 API Documentation enabled\n");
    }

    if (config.enable_cors) {
        crest_enable_cors(app, true);
        printf("🌐 CORS enabled\n");
    }

    // API endpoints
    crest_get(app, "/health", health_check, "Health check endpoint");
    crest_get(app, "/config", get_config, "Get application configuration");
    crest_get(app, "/api/info", api_info, "Get API information and capabilities");

    printf("🚀 Configurable API Server running on http://localhost:%d\n", config.port);
    if (config.enable_docs) {
        printf("🧠 Swagger UI: http://localhost:%d/docs\n", config.port);
        printf("⚙️ ReDoc: http://localhost:%d/redoc\n", config.port);
        printf("🧾 OpenAPI JSON: http://localhost:%d/openapi.json\n", config.port);
    }

    crest_listen(app);
    crest_destroy(app);
    return 0;
}
```

### Configuration Testing

**Set environment variables and run:**
```bash
# Linux/macOS
export PORT=3000
export ENABLE_DOCS=1
export LOG_LEVEL=debug
./configurable_api

# Windows PowerShell
$env:PORT = "3000"
$env:ENABLE_DOCS = "1"
$env:LOG_LEVEL = "debug"
.\configurable_api.exe
```

**Test configuration endpoints:**
```bash
curl http://localhost:3000/health
curl http://localhost:3000/config
curl http://localhost:3000/api/info
```

## Logging and Monitoring Example

### Code Example

```c
#include <crest/crest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Logging middleware
void logging_middleware(crest_request_t *req, crest_response_t *res, void *data) {
    time_t now = time(NULL);
    char timestamp[26];
    ctime_r(&now, timestamp);
    timestamp[strlen(timestamp) - 1] = '\0'; // Remove newline

    printf("[%s] %s %s %s\n",
           timestamp,
           crest_request_method(req),
           crest_request_path(req),
           crest_request_header(req, "User-Agent") ?: "Unknown");

    // Add request ID for tracking
    char request_id[16];
    sprintf(request_id, "%ld", now);
    crest_response_header(res, "X-Request-ID", request_id);
}

// Request timing middleware
void timing_middleware(crest_request_t *req, crest_response_t *res, void *data) {
    // In real implementation, store start time and calculate duration
    crest_response_header(res, "X-Response-Time", "15ms");
}

// Metrics endpoint
void get_metrics(crest_request_t *req, crest_response_t *res) {
    crest_response_json(res, "{"
        "\"metrics\": {"
            "\"requests_total\": 1250,"
            "\"requests_by_method\": {"
                "\"GET\": 850,"
                "\"POST\": 320,"
                "\"PUT\": 65,"
                "\"DELETE\": 15"
            "},"
            "\"response_times\": {"
                "\"avg_ms\": 45,"
                "\"p95_ms\": 120,"
                "\"p99_ms\": 250"
            "},"
            "\"errors_total\": 12,"
            "\"errors_by_code\": {"
                "\"400\": 5,"
                "\"404\": 4,"
                "\"500\": 3"
            "}"
        "},"
        "\"uptime_seconds\": 3600,"
        "\"memory_usage_mb\": 25.5"
    "}");
}

// Logs endpoint (simplified)
void get_logs(crest_request_t *req, crest_response_t *res) {
    const char *level = crest_request_query(req, "level");
    const char *limit = crest_request_query(req, "limit");

    crest_response_json(res, "{"
        "\"logs\": ["
            "{\"timestamp\": \"2024-01-15T12:00:00Z\", \"level\": \"INFO\", \"message\": \"Server started on port 8080\"},"
            "{\"timestamp\": \"2024-01-15T12:00:05Z\", \"level\": \"INFO\", \"message\": \"API documentation enabled\"},"
            "{\"timestamp\": \"2024-01-15T12:01:23Z\", \"level\": \"WARN\", \"message\": \"Rate limit exceeded for IP 192.168.1.100\"},"
            "{\"timestamp\": \"2024-01-15T12:02:45Z\", \"level\": \"ERROR\", \"message\": \"Database connection failed\"}"
        "],"
        "\"total\": 4,"
        "\"filtered\": 4"
    "}");
}

// Sample API endpoints with different response patterns
void success_endpoint(crest_request_t *req, crest_response_t *res) {
    crest_response_json(res, "{\"message\": \"Success\", \"status\": \"ok\"}");
}

void error_endpoint(crest_request_t *req, crest_response_t *res) {
    crest_response_status(res, 400);
    crest_response_json(res, "{\"error\": \"Bad Request\", \"code\": \"INVALID_INPUT\"}");
}

void slow_endpoint(crest_request_t *req, crest_response_t *res) {
    // Simulate slow operation
    struct timespec delay = {0, 50000000}; // 50ms
    nanosleep(&delay, NULL);
    crest_response_json(res, "{\"message\": \"Slow operation completed\", \"delay_ms\": 50}");
}

int main(void) {
    crest_app_t *app = crest_create();

    crest_enable_dashboard(app, true);

    // Global middleware for logging and monitoring
    crest_use(app, "/*", logging_middleware, NULL);
    crest_use(app, "/*", timing_middleware, NULL);

    // Monitoring endpoints
    crest_get(app, "/metrics", get_metrics, "Get application metrics and performance data");
    crest_get(app, "/logs", get_logs, "Get application logs with filtering");

    // Sample API endpoints for testing
    crest_get(app, "/api/success", success_endpoint, "Successful API response");
    crest_get(app, "/api/error", error_endpoint, "Error response for testing");
    crest_get(app, "/api/slow", slow_endpoint, "Slow endpoint for performance testing");

    printf("📊 Monitored API Server running on http://localhost:8080\n");
    printf("🧠 API Documentation: http://localhost:8080/docs\n");
    printf("📈 Metrics: http://localhost:8080/metrics\n");
    printf("📝 Logs: http://localhost:8080/logs\n");

    crest_listen(app);
    crest_destroy(app);
    return 0;
}
```

### Monitoring Testing Commands

**Test logging (check console output):**
```bash
curl http://localhost:8080/api/success
curl http://localhost:8080/api/error
curl http://localhost:8080/api/slow
```

**Get metrics:**
```bash
curl http://localhost:8080/metrics
```

**Get logs:**
```bash
curl http://localhost:8080/logs
curl "http://localhost:8080/logs?level=ERROR"
curl "http://localhost:8080/logs?limit=10"
```

## Docker Deployment Example

### Dockerfile

```dockerfile
FROM gcc:latest as builder

# Install dependencies
RUN apt-get update && apt-get install -y \
    cmake \
    sqlite3 \
    libsqlite3-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Build the application
RUN mkdir build && cd build && \
    cmake .. && \
    make -j$(nproc)

FROM ubuntu:latest

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    sqlite3 \
    && rm -rf /var/lib/apt/lists/*

# Create app user
RUN useradd -r -s /bin/false crest

# Set working directory
WORKDIR /app

# Copy built application
COPY --from=builder /app/build/crest_advanced_example /app/crest_app
COPY --from=builder /app/build/crest.dll /app/crest.dll

# Change ownership
RUN chown crest:crest /app/crest_app /app/crest.dll

# Switch to non-root user
USER crest

# Expose port
EXPOSE 8080

# Health check
HEALTHCHECK --interval=30s --timeout=3s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8080/health || exit 1

# Run the application
CMD ["./crest_app"]
```

### docker-compose.yml

```yaml
version: '3.8'

services:
  crest-api:
    build: .
    ports:
      - "8080:8080"
    environment:
      - PORT=8080
      - ENABLE_DOCS=1
      - ENABLE_CORS=1
      - LOG_LEVEL=info
      - DATABASE_URL=sqlite:///app/data/app.db
    volumes:
      - ./data:/app/data
    restart: unless-stopped
    healthcheck:
      test: ["CMD", "curl", "-f", "http://localhost:8080/health"]
      interval: 30s
      timeout: 10s
      retries: 3
      start_period: 40s

  nginx:
    image: nginx:alpine
    ports:
      - "80:80"
    volumes:
      - ./nginx.conf:/etc/nginx/nginx.conf:ro
    depends_on:
      - crest-api
    restart: unless-stopped
```

### nginx.conf

```nginx
events {
    worker_connections 1024;
}

http {
    upstream crest_backend {
        server crest-api:8080;
    }

    server {
        listen 80;
        server_name localhost;

        # API endpoints
        location /api/ {
            proxy_pass http://crest_backend;
            proxy_set_header Host $host;
            proxy_set_header X-Real-IP $remote_addr;
            proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
            proxy_set_header X-Forwarded-Proto $scheme;
        }

        # API documentation
        location /docs {
            proxy_pass http://crest_backend;
            proxy_set_header Host $host;
        }

        location /redoc {
            proxy_pass http://crest_backend;
            proxy_set_header Host $host;
        }

        location /openapi.json {
            proxy_pass http://crest_backend;
            proxy_set_header Host $host;
        }

        # Static files (if any)
        location /static/ {
            alias /app/static/;
            expires 1y;
            add_header Cache-Control "public, immutable";
        }

        # Health check
        location /health {
            proxy_pass http://crest_backend;
            access_log off;
        }
    }
}
```

### Deployment Commands

**Build and run with Docker:**
```bash
# Build the image
docker build -t crest-api .

# Run the container
docker run -p 8080:8080 crest-api

# Run with docker-compose
docker-compose up -d

# Check health
curl http://localhost/health

# View logs
docker-compose logs -f crest-api
```

## Summary

These integration examples demonstrate:

- **Database integration** with SQLite for persistent storage
- **Configuration management** using environment variables
- **Logging and monitoring** middleware for observability
- **Container deployment** with Docker and docker-compose
- **Reverse proxy setup** with Nginx for production deployment

All examples include the API documentation features enabled via `crest_enable_dashboard(app, true)`, providing Swagger UI, ReDoc, and OpenAPI JSON endpoints for comprehensive API exploration and testing.
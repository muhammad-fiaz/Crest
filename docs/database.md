# Database Integration

Crest provides database helpers for easy integration with SQL databases.

## Overview

Features:
- Connection pooling
- Query builder
- ORM-like models
- Transaction support
- Prepared statements

## Connection

```cpp
#include "crest/database.hpp"

// Create connection (implement for your database)
class MySQLConnection : public crest::db::Connection {
    // Implement virtual methods
};

auto conn = std::make_shared<MySQLConnection>();
conn->connect("host=localhost;db=mydb;user=root;password=secret");
```

## Connection Pool

```cpp
crest::db::ConnectionPool::Config pool_config;
pool_config.connection_string = "host=localhost;db=mydb";
pool_config.min_connections = 2;
pool_config.max_connections = 10;
pool_config.timeout_seconds = 30;

crest::db::ConnectionPool pool(pool_config);

// Acquire connection
auto conn = pool.acquire();

// Use connection
auto results = conn->execute("SELECT * FROM users");

// Release back to pool
pool.release(conn);
```

## Query Builder

### SELECT Queries

```cpp
crest::db::QueryBuilder qb;

// Simple select
qb.select({"id", "name", "email"})
  .from("users")
  .where("age", ">", 18)
  .order_by("name", true)
  .limit(10);

std::string query = qb.build();
// SELECT id, name, email FROM users WHERE age > ? ORDER BY name ASC LIMIT 10

auto results = conn->execute(query, qb.get_params());
```

### INSERT Queries

```cpp
crest::db::QueryBuilder qb;

qb.insert_into("users")
  .values({
      {"name", "John Doe"},
      {"email", "john@example.com"},
      {"age", 30}
  });

conn->execute_update(qb.build(), qb.get_params());
```

### UPDATE Queries

```cpp
crest::db::QueryBuilder qb;

qb.update("users")
  .set({
      {"name", "Jane Doe"},
      {"email", "jane@example.com"}
  })
  .where("id", "=", 1);

conn->execute_update(qb.build(), qb.get_params());
```

### DELETE Queries

```cpp
crest::db::QueryBuilder qb;

qb.delete_from("users")
  .where("id", "=", 1);

conn->execute_update(qb.build(), qb.get_params());
```

### Complex Queries

```cpp
crest::db::QueryBuilder qb;

qb.select({"users.id", "users.name", "orders.total"})
  .from("users")
  .join("orders", "users.id = orders.user_id")
  .where("orders.status", "=", "completed")
  .and_where("orders.total", ">", 100)
  .order_by("orders.total", false)
  .limit(20)
  .offset(0);

auto results = conn->execute(qb.build(), qb.get_params());
```

## Models

```cpp
class User : public crest::db::Model {
public:
    int id;
    std::string name;
    std::string email;
    int age;
    
    std::string table_name() const override {
        return "users";
    }
    
    crest::db::Row to_row() const override {
        return {
            {"id", id},
            {"name", name},
            {"email", email},
            {"age", age}
        };
    }
    
    void from_row(const crest::db::Row& row) override {
        id = std::get<int>(row.at("id"));
        name = std::get<std::string>(row.at("name"));
        email = std::get<std::string>(row.at("email"));
        age = std::get<int>(row.at("age"));
    }
};

// Usage
User user;
user.name = "Alice";
user.email = "alice@example.com";
user.age = 25;

// Save (insert or update)
user.save(*conn);

// Delete
user.remove(*conn);

// Find all
auto results = User::find_all(*conn, "users");

// Find by ID
auto row = User::find_by_id(*conn, "users", 1);
```

## Transactions

```cpp
auto conn = pool.acquire();

try {
    conn->begin_transaction();
    
    // Multiple operations
    conn->execute_update("INSERT INTO users (name) VALUES (?)", {"Alice"});
    conn->execute_update("INSERT INTO orders (user_id) VALUES (?)", {1});
    
    conn->commit();
} catch (const std::exception& e) {
    conn->rollback();
    std::cerr << "Transaction failed: " << e.what() << std::endl;
}

pool.release(conn);
```

## API Integration Example

```cpp
#include "crest/crest.hpp"
#include "crest/database.hpp"

int main() {
    crest::App app;
    
    // Setup connection pool
    crest::db::ConnectionPool::Config pool_config;
    pool_config.connection_string = "host=localhost;db=api";
    crest::db::ConnectionPool pool(pool_config);
    
    // Get all users
    app.get("/users", [&pool](crest::Request& req, crest::Response& res) {
        auto conn = pool.acquire();
        
        crest::db::QueryBuilder qb;
        qb.select({}).from("users");
        
        auto results = conn->execute(qb.build());
        pool.release(conn);
        
        // Convert to JSON (simplified)
        res.json(200, R"({"users":[]})");
    });
    
    // Create user
    app.post("/users", [&pool](crest::Request& req, crest::Response& res) {
        auto conn = pool.acquire();
        
        // Parse request body (simplified)
        crest::db::QueryBuilder qb;
        qb.insert_into("users").values({
            {"name", "John"},
            {"email", "john@example.com"}
        });
        
        conn->execute_update(qb.build(), qb.get_params());
        pool.release(conn);
        
        res.json(201, R"({"message":"User created"})");
    });
    
    app.run("0.0.0.0", 8000);
    return 0;
}
```

## Best Practices

- Always use connection pooling in production
- Use prepared statements to prevent SQL injection
- Handle transactions properly with try-catch
- Release connections back to pool
- Set appropriate pool size based on load
- Use query builder for complex queries
- Implement proper error handling
- Close connections on application shutdown

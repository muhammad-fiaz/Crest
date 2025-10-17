/**
 * @file database_example.cpp
 * @brief Database integration example
 */

#include "crest/crest.hpp"
#include "crest/database.hpp"
#include <iostream>

// Example User model
class User : public crest::db::Model {
public:
    int id = 0;
    std::string name;
    std::string email;
    int age = 0;
    
    std::string table_name() const override {
        return "users";
    }
    
    crest::db::Row to_row() const override {
        crest::db::Row row;
        if (id > 0) row["id"] = id;
        row["name"] = name;
        row["email"] = email;
        row["age"] = age;
        return row;
    }
    
    void from_row(const crest::db::Row& row) override {
        if (row.find("id") != row.end()) {
            id = std::get<int>(row.at("id"));
        }
        if (row.find("name") != row.end()) {
            name = std::get<std::string>(row.at("name"));
        }
        if (row.find("email") != row.end()) {
            email = std::get<std::string>(row.at("email"));
        }
        if (row.find("age") != row.end()) {
            age = std::get<int>(row.at("age"));
        }
    }
};

int main() {
    crest::App app;
    
    // Configure connection pool
    crest::db::ConnectionPool::Config pool_config;
    pool_config.connection_string = "host=localhost;db=crest_example;user=root;password=secret";
    pool_config.min_connections = 2;
    pool_config.max_connections = 10;
    pool_config.timeout_seconds = 30;
    
    // Note: You need to implement a concrete Connection class for your database
    // crest::db::ConnectionPool pool(pool_config);
    
    // GET /users - List all users
    app.get("/users", [](crest::Request& req, crest::Response& res) {
        // auto conn = pool.acquire();
        
        crest::db::QueryBuilder qb;
        qb.select({"id", "name", "email", "age"})
          .from("users")
          .order_by("name", true);
        
        std::string query = qb.build();
        std::cout << "Query: " << query << std::endl;
        
        // auto results = conn->execute(query);
        // pool.release(conn);
        
        // Mock response
        res.json(200, R"({
            "users": [
                {"id": 1, "name": "Alice", "email": "alice@example.com", "age": 25},
                {"id": 2, "name": "Bob", "email": "bob@example.com", "age": 30}
            ]
        })");
    });
    
    // GET /users/:id - Get user by ID
    app.get("/users/:id", [](crest::Request& req, crest::Response& res) {
        // auto conn = pool.acquire();
        
        // std::string id = req.path().substr(7); // Extract ID from path
        
        crest::db::QueryBuilder qb;
        qb.select({})
          .from("users")
          .where("id", "=", 1)
          .limit(1);
        
        std::string query = qb.build();
        std::cout << "Query: " << query << std::endl;
        
        // auto results = conn->execute(query, qb.get_params());
        // pool.release(conn);
        
        res.json(200, R"({
            "id": 1,
            "name": "Alice",
            "email": "alice@example.com",
            "age": 25
        })");
    });
    
    // POST /users - Create new user
    app.post("/users", [](crest::Request& req, crest::Response& res) {
        // auto conn = pool.acquire();
        
        // Parse request body (simplified)
        crest::db::QueryBuilder qb;
        qb.insert_into("users")
          .values({
              {"name", "Charlie"},
              {"email", "charlie@example.com"},
              {"age", 28}
          });
        
        std::string query = qb.build();
        std::cout << "Query: " << query << std::endl;
        
        // int affected = conn->execute_update(query, qb.get_params());
        // pool.release(conn);
        
        res.json(201, R"({
            "message": "User created successfully",
            "id": 3
        })");
    });
    
    // PUT /users/:id - Update user
    app.put("/users/:id", [](crest::Request& req, crest::Response& res) {
        // auto conn = pool.acquire();
        
        crest::db::QueryBuilder qb;
        qb.update("users")
          .set({
              {"name", "Alice Smith"},
              {"email", "alice.smith@example.com"}
          })
          .where("id", "=", 1);
        
        std::string query = qb.build();
        std::cout << "Query: " << query << std::endl;
        
        // int affected = conn->execute_update(query, qb.get_params());
        // pool.release(conn);
        
        res.json(200, R"({
            "message": "User updated successfully"
        })");
    });
    
    // DELETE /users/:id - Delete user
    app.del("/users/:id", [](crest::Request& req, crest::Response& res) {
        // auto conn = pool.acquire();
        
        crest::db::QueryBuilder qb;
        qb.delete_from("users")
          .where("id", "=", 1);
        
        std::string query = qb.build();
        std::cout << "Query: " << query << std::endl;
        
        // int affected = conn->execute_update(query, qb.get_params());
        // pool.release(conn);
        
        res.json(200, R"({
            "message": "User deleted successfully"
        })");
    });
    
    // GET /users/search - Search users
    app.get("/users/search", [](crest::Request& req, crest::Response& res) {
        // auto conn = pool.acquire();
        
        std::string name = req.query("name");
        int min_age = 0; // Parse from req.query("min_age")
        
        crest::db::QueryBuilder qb;
        qb.select({})
          .from("users")
          .where("name", "LIKE", "%" + name + "%")
          .and_where("age", ">=", min_age)
          .order_by("age", false)
          .limit(10);
        
        std::string query = qb.build();
        std::cout << "Query: " << query << std::endl;
        
        // auto results = conn->execute(query, qb.get_params());
        // pool.release(conn);
        
        res.json(200, R"({
            "users": []
        })");
    });
    
    // POST /users/bulk - Bulk insert with transaction
    app.post("/users/bulk", [](crest::Request& req, crest::Response& res) {
        // auto conn = pool.acquire();
        
        // try {
        //     conn->begin_transaction();
        //     
        //     // Insert multiple users
        //     for (const auto& user_data : users) {
        //         crest::db::QueryBuilder qb;
        //         qb.insert_into("users").values(user_data);
        //         conn->execute_update(qb.build(), qb.get_params());
        //     }
        //     
        //     conn->commit();
        //     res.json(200, R"({"message":"Users created"})");
        // } catch (const std::exception& e) {
        //     conn->rollback();
        //     res.json(500, R"({"error":"Transaction failed"})");
        // }
        // 
        // pool.release(conn);
        
        res.json(200, R"({
            "message": "Bulk insert completed",
            "count": 5
        })");
    });
    
    // GET /stats - Database statistics
    app.get("/stats", [](crest::Request& req, crest::Response& res) {
        // auto conn = pool.acquire();
        
        crest::db::QueryBuilder qb;
        qb.select({"COUNT(*) as total"})
          .from("users");
        
        std::string query = qb.build();
        std::cout << "Query: " << query << std::endl;
        
        // auto results = conn->execute(query);
        // pool.release(conn);
        
        res.json(200, R"({
            "total_users": 10,
            "active_connections": 3,
            "available_connections": 7
        })");
    });
    
    std::cout << "Database API running on http://0.0.0.0:8000" << std::endl;
    std::cout << "Endpoints:" << std::endl;
    std::cout << "  GET    /users           - List all users" << std::endl;
    std::cout << "  GET    /users/:id       - Get user by ID" << std::endl;
    std::cout << "  POST   /users           - Create new user" << std::endl;
    std::cout << "  PUT    /users/:id       - Update user" << std::endl;
    std::cout << "  DELETE /users/:id       - Delete user" << std::endl;
    std::cout << "  GET    /users/search    - Search users" << std::endl;
    std::cout << "  POST   /users/bulk      - Bulk insert" << std::endl;
    std::cout << "  GET    /stats           - Database stats" << std::endl;
    std::cout << std::endl;
    std::cout << "Note: This example shows query building." << std::endl;
    std::cout << "Implement a Connection class for actual database operations." << std::endl;
    
    app.run("0.0.0.0", 8000);
    return 0;
}

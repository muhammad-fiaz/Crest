/**
 * @file test_database.cpp
 * @brief Test cases for database integration
 */

#include "crest/database.hpp"
#include <cassert>
#include <iostream>

void test_query_builder_select() {
    std::cout << "Testing query builder SELECT..." << std::endl;
    
    crest::db::QueryBuilder qb;
    qb.select({"id", "name", "email"})
      .from("users")
      .where("age", ">", 18)
      .order_by("name", true)
      .limit(10);
    
    std::string query = qb.build();
    
    assert(query.find("SELECT") != std::string::npos);
    assert(query.find("FROM users") != std::string::npos);
    assert(query.find("WHERE") != std::string::npos);
    assert(query.find("ORDER BY") != std::string::npos);
    assert(query.find("LIMIT") != std::string::npos);
    
    std::cout << "  ✓ SELECT query: " << query << std::endl;
}

void test_query_builder_insert() {
    std::cout << "Testing query builder INSERT..." << std::endl;
    
    crest::db::QueryBuilder qb;
    qb.insert_into("users")
      .values({
          {"name", "John"},
          {"email", "john@example.com"},
          {"age", 30}
      });
    
    std::string query = qb.build();
    
    assert(query.find("INSERT INTO users") != std::string::npos);
    assert(query.find("VALUES") != std::string::npos);
    
    std::cout << "  ✓ INSERT query: " << query << std::endl;
}

void test_query_builder_update() {
    std::cout << "Testing query builder UPDATE..." << std::endl;
    
    crest::db::QueryBuilder qb;
    qb.update("users")
      .set({
          {"name", "Jane"},
          {"email", "jane@example.com"}
      })
      .where("id", "=", 1);
    
    std::string query = qb.build();
    
    assert(query.find("UPDATE users") != std::string::npos);
    assert(query.find("SET") != std::string::npos);
    assert(query.find("WHERE") != std::string::npos);
    
    std::cout << "  ✓ UPDATE query: " << query << std::endl;
}

void test_query_builder_delete() {
    std::cout << "Testing query builder DELETE..." << std::endl;
    
    crest::db::QueryBuilder qb;
    qb.delete_from("users")
      .where("id", "=", 1);
    
    std::string query = qb.build();
    
    assert(query.find("DELETE FROM users") != std::string::npos);
    assert(query.find("WHERE") != std::string::npos);
    
    std::cout << "  ✓ DELETE query: " << query << std::endl;
}

void test_connection_pool() {
    std::cout << "Testing connection pool..." << std::endl;
    
    crest::db::ConnectionPool::Config config;
    config.connection_string = "test";
    config.min_connections = 2;
    config.max_connections = 10;
    
    crest::db::ConnectionPool pool(config);
    
    assert(pool.available_count() == 0);
    assert(pool.active_count() == 0);
    
    std::cout << "  ✓ Connection pool created" << std::endl;
}

int main() {
    std::cout << "\n=== Database Tests ===" << std::endl;
    
    test_query_builder_select();
    test_query_builder_insert();
    test_query_builder_update();
    test_query_builder_delete();
    test_connection_pool();
    
    std::cout << "\n✅ All database tests passed!" << std::endl;
    return 0;
}

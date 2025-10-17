/**
 * @file database.hpp
 * @brief Database integration helpers for Crest framework
 * @version 0.0.0
 */

#ifndef CREST_DATABASE_HPP
#define CREST_DATABASE_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <variant>
#include <mutex>

namespace crest {
namespace db {

using Value = std::variant<std::nullptr_t, int, double, std::string, bool>;
using Row = std::map<std::string, Value>;
using ResultSet = std::vector<Row>;

class Connection {
public:
    virtual ~Connection() = default;
    
    virtual bool connect(const std::string& connection_string) = 0;
    virtual void disconnect() = 0;
    virtual bool is_connected() const = 0;
    
    virtual ResultSet execute(const std::string& query) = 0;
    virtual ResultSet execute(const std::string& query, const std::vector<Value>& params) = 0;
    
    virtual int execute_update(const std::string& query) = 0;
    virtual int execute_update(const std::string& query, const std::vector<Value>& params) = 0;
    
    virtual bool begin_transaction() = 0;
    virtual bool commit() = 0;
    virtual bool rollback() = 0;
    
    virtual std::string escape(const std::string& str) = 0;
    virtual std::string last_error() const = 0;
};

class ConnectionPool {
public:
    struct Config {
        std::string connection_string;
        size_t min_connections = 2;
        size_t max_connections = 10;
        int timeout_seconds = 30;
    };
    
    explicit ConnectionPool(const Config& config);
    ~ConnectionPool();
    
    std::shared_ptr<Connection> acquire();
    void release(std::shared_ptr<Connection> conn);
    
    size_t available_count() const;
    size_t active_count() const;

private:
    Config config_;
    std::vector<std::shared_ptr<Connection>> available_;
    std::vector<std::shared_ptr<Connection>> active_;
    mutable std::mutex mutex_;
};

class QueryBuilder {
public:
    QueryBuilder& select(const std::vector<std::string>& columns);
    QueryBuilder& from(const std::string& table);
    QueryBuilder& where(const std::string& condition);
    QueryBuilder& where(const std::string& column, const std::string& op, const Value& value);
    QueryBuilder& and_where(const std::string& condition);
    QueryBuilder& and_where(const std::string& column, const std::string& op, const Value& value);
    QueryBuilder& or_where(const std::string& condition);
    QueryBuilder& or_where(const std::string& column, const std::string& op, const Value& value);
    QueryBuilder& order_by(const std::string& column, bool ascending = true);
    QueryBuilder& limit(int count);
    QueryBuilder& offset(int count);
    QueryBuilder& join(const std::string& table, const std::string& condition);
    QueryBuilder& left_join(const std::string& table, const std::string& condition);
    
    QueryBuilder& insert_into(const std::string& table);
    QueryBuilder& values(const std::map<std::string, Value>& data);
    
    QueryBuilder& update(const std::string& table);
    QueryBuilder& set(const std::map<std::string, Value>& data);
    
    QueryBuilder& delete_from(const std::string& table);
    
    std::string build() const;
    std::vector<Value> get_params() const;

private:
    std::string query_;
    std::vector<Value> params_;
    std::string table_;
    std::vector<std::string> columns_;
    std::vector<std::string> conditions_;
    std::vector<std::string> joins_;
    std::string order_;
    int limit_ = -1;
    int offset_ = -1;
    std::map<std::string, Value> data_;
    enum { SELECT, INSERT, UPDATE, DELETE } type_;
};

class Model {
public:
    virtual ~Model() = default;
    
    virtual std::string table_name() const = 0;
    virtual std::string primary_key() const { return "id"; }
    
    virtual Row to_row() const = 0;
    virtual void from_row(const Row& row) = 0;
    
    bool save(Connection& conn);
    bool remove(Connection& conn);
    
    static ResultSet find_all(Connection& conn, const std::string& table);
    static Row find_by_id(Connection& conn, const std::string& table, const Value& id);
};

} // namespace db
} // namespace crest

#endif /* CREST_DATABASE_HPP */

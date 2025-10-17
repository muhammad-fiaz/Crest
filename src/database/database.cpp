/**
 * @file database.cpp
 * @brief Database implementation
 */

#include "crest/database.hpp"
#include <sstream>
#include <mutex>
#include <algorithm>

namespace crest {
namespace db {

ConnectionPool::ConnectionPool(const Config& config) : config_(config) {}

ConnectionPool::~ConnectionPool() {
    std::lock_guard<std::mutex> lock(mutex_);
    available_.clear();
    active_.clear();
}

std::shared_ptr<Connection> ConnectionPool::acquire() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!available_.empty()) {
        auto conn = available_.back();
        available_.pop_back();
        active_.push_back(conn);
        return conn;
    }
    
    return nullptr;
}

void ConnectionPool::release(std::shared_ptr<Connection> conn) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = std::find(active_.begin(), active_.end(), conn);
    if (it != active_.end()) {
        active_.erase(it);
        available_.push_back(conn);
    }
}

size_t ConnectionPool::available_count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return available_.size();
}

size_t ConnectionPool::active_count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return active_.size();
}

QueryBuilder& QueryBuilder::select(const std::vector<std::string>& columns) {
    type_ = SELECT;
    columns_ = columns;
    return *this;
}

QueryBuilder& QueryBuilder::from(const std::string& table) {
    table_ = table;
    return *this;
}

QueryBuilder& QueryBuilder::where(const std::string& condition) {
    conditions_.push_back(condition);
    return *this;
}

QueryBuilder& QueryBuilder::where(const std::string& column, const std::string& op, const Value& value) {
    conditions_.push_back(column + " " + op + " ?");
    params_.push_back(value);
    return *this;
}

QueryBuilder& QueryBuilder::and_where(const std::string& condition) {
    if (!conditions_.empty()) {
        conditions_.push_back("AND " + condition);
    } else {
        conditions_.push_back(condition);
    }
    return *this;
}

QueryBuilder& QueryBuilder::and_where(const std::string& column, const std::string& op, const Value& value) {
    if (!conditions_.empty()) {
        conditions_.push_back("AND " + column + " " + op + " ?");
    } else {
        conditions_.push_back(column + " " + op + " ?");
    }
    params_.push_back(value);
    return *this;
}

QueryBuilder& QueryBuilder::or_where(const std::string& condition) {
    if (!conditions_.empty()) {
        conditions_.push_back("OR " + condition);
    } else {
        conditions_.push_back(condition);
    }
    return *this;
}

QueryBuilder& QueryBuilder::or_where(const std::string& column, const std::string& op, const Value& value) {
    if (!conditions_.empty()) {
        conditions_.push_back("OR " + column + " " + op + " ?");
    } else {
        conditions_.push_back(column + " " + op + " ?");
    }
    params_.push_back(value);
    return *this;
}

QueryBuilder& QueryBuilder::order_by(const std::string& column, bool ascending) {
    order_ = column + (ascending ? " ASC" : " DESC");
    return *this;
}

QueryBuilder& QueryBuilder::limit(int count) {
    limit_ = count;
    return *this;
}

QueryBuilder& QueryBuilder::offset(int count) {
    offset_ = count;
    return *this;
}

QueryBuilder& QueryBuilder::join(const std::string& table, const std::string& condition) {
    joins_.push_back("JOIN " + table + " ON " + condition);
    return *this;
}

QueryBuilder& QueryBuilder::left_join(const std::string& table, const std::string& condition) {
    joins_.push_back("LEFT JOIN " + table + " ON " + condition);
    return *this;
}

QueryBuilder& QueryBuilder::insert_into(const std::string& table) {
    type_ = INSERT;
    table_ = table;
    return *this;
}

QueryBuilder& QueryBuilder::values(const std::map<std::string, Value>& data) {
    data_ = data;
    return *this;
}

QueryBuilder& QueryBuilder::update(const std::string& table) {
    type_ = UPDATE;
    table_ = table;
    return *this;
}

QueryBuilder& QueryBuilder::set(const std::map<std::string, Value>& data) {
    data_ = data;
    return *this;
}

QueryBuilder& QueryBuilder::delete_from(const std::string& table) {
    type_ = DELETE;
    table_ = table;
    return *this;
}

std::string QueryBuilder::build() const {
    std::ostringstream query;
    
    switch (type_) {
        case SELECT:
            query << "SELECT ";
            if (columns_.empty()) {
                query << "*";
            } else {
                for (size_t i = 0; i < columns_.size(); ++i) {
                    if (i > 0) query << ", ";
                    query << columns_[i];
                }
            }
            query << " FROM " << table_;
            
            for (const auto& join : joins_) {
                query << " " << join;
            }
            
            if (!conditions_.empty()) {
                query << " WHERE ";
                for (size_t i = 0; i < conditions_.size(); ++i) {
                    if (i > 0) query << " ";
                    query << conditions_[i];
                }
            }
            
            if (!order_.empty()) {
                query << " ORDER BY " << order_;
            }
            
            if (limit_ > 0) {
                query << " LIMIT " << limit_;
            }
            
            if (offset_ > 0) {
                query << " OFFSET " << offset_;
            }
            break;
            
        case INSERT:
            query << "INSERT INTO " << table_ << " (";
            {
                size_t i = 0;
                for (const auto& pair : data_) {
                    if (i++ > 0) query << ", ";
                    query << pair.first;
                }
            }
            query << ") VALUES (";
            {
                size_t i = 0;
                for (const auto& pair : data_) {
                    if (i++ > 0) query << ", ";
                    query << "?";
                }
            }
            query << ")";
            break;
            
        case UPDATE:
            query << "UPDATE " << table_ << " SET ";
            {
                size_t i = 0;
                for (const auto& pair : data_) {
                    if (i++ > 0) query << ", ";
                    query << pair.first << " = ?";
                }
            }
            
            if (!conditions_.empty()) {
                query << " WHERE ";
                for (size_t i = 0; i < conditions_.size(); ++i) {
                    if (i > 0) query << " ";
                    query << conditions_[i];
                }
            }
            break;
            
        case DELETE:
            query << "DELETE FROM " << table_;
            
            if (!conditions_.empty()) {
                query << " WHERE ";
                for (size_t i = 0; i < conditions_.size(); ++i) {
                    if (i > 0) query << " ";
                    query << conditions_[i];
                }
            }
            (void)data_;
            break;
    }
    
    return query.str();
}

std::vector<Value> QueryBuilder::get_params() const {
    std::vector<Value> all_params;
    
    if (type_ == INSERT || type_ == UPDATE) {
        for (const auto& pair : data_) {
            all_params.push_back(pair.second);
        }
    }
    
    all_params.insert(all_params.end(), params_.begin(), params_.end());
    return all_params;
}

bool Model::save(Connection& conn) {
    auto row = to_row();
    QueryBuilder qb;
    
    if (row.find(primary_key()) != row.end() && !std::holds_alternative<std::nullptr_t>(row[primary_key()])) {
        qb.update(table_name()).set(row).where(primary_key(), "=", row[primary_key()]);
    } else {
        qb.insert_into(table_name()).values(row);
    }
    
    return conn.execute_update(qb.build(), qb.get_params()) > 0;
}

bool Model::remove(Connection& conn) {
    auto row = to_row();
    if (row.find(primary_key()) == row.end()) {
        return false;
    }
    
    QueryBuilder qb;
    qb.delete_from(table_name()).where(primary_key(), "=", row[primary_key()]);
    
    return conn.execute_update(qb.build(), qb.get_params()) > 0;
}

ResultSet Model::find_all(Connection& conn, const std::string& table) {
    QueryBuilder qb;
    qb.select({}).from(table);
    return conn.execute(qb.build());
}

Row Model::find_by_id(Connection& conn, const std::string& table, const Value& id) {
    QueryBuilder qb;
    qb.select({}).from(table).where("id", "=", id).limit(1);
    auto results = conn.execute(qb.build(), qb.get_params());
    return results.empty() ? Row{} : results[0];
}

} // namespace db
} // namespace crest

/**
 * @file upload.hpp
 * @brief File upload handling for Crest framework
 * @version 0.0.0
 */

#ifndef CREST_UPLOAD_HPP
#define CREST_UPLOAD_HPP

#include "crest.hpp"
#include <string>
#include <vector>
#include <map>
#include <functional>

namespace crest {

struct UploadedFile {
    std::string field_name;
    std::string filename;
    std::string content_type;
    size_t size;
    std::vector<uint8_t> data;
    
    bool save_to(const std::string& path) const;
    std::string to_string() const;
};

class MultipartParser {
public:
    struct Config {
        size_t max_file_size = 10 * 1024 * 1024; // 10MB
        size_t max_files = 10;
        std::vector<std::string> allowed_extensions;
        std::vector<std::string> allowed_mime_types;
    };
    
    explicit MultipartParser(const Config& config = Config());
    
    bool parse(const std::string& body, const std::string& boundary);
    
    std::vector<UploadedFile> get_files() const { return files_; }
    std::map<std::string, std::string> get_fields() const { return fields_; }
    
    UploadedFile get_file(const std::string& field_name) const;
    std::string get_field(const std::string& field_name) const;
    
    bool has_file(const std::string& field_name) const;
    bool has_field(const std::string& field_name) const;
    
    std::string last_error() const { return error_; }

private:
    Config config_;
    std::vector<UploadedFile> files_;
    std::map<std::string, std::string> fields_;
    std::string error_;
    
    bool validate_file(const UploadedFile& file);
    std::string get_extension(const std::string& filename) const;
};

class FileUploadMiddleware {
public:
    using Config = MultipartParser::Config;
    
    explicit FileUploadMiddleware(const Config& config = Config()) : config_(config) {}
    
    void handle(Request& req, Response& res, std::function<void()> next);

private:
    Config config_;
};

} // namespace crest

#endif /* CREST_UPLOAD_HPP */

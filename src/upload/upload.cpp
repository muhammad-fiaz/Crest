/**
 * @file upload.cpp
 * @brief File upload implementation
 */

#include "crest/upload.hpp"
#include <fstream>
#include <algorithm>

namespace crest {

bool UploadedFile::save_to(const std::string& path) const {
    std::ofstream file(path, std::ios::binary);
    if (!file) return false;
    
    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    return file.good();
}

std::string UploadedFile::to_string() const {
    return std::string(data.begin(), data.end());
}

MultipartParser::MultipartParser(const Config& config) : config_(config) {}

bool MultipartParser::parse(const std::string& body, const std::string& boundary) {
    files_.clear();
    fields_.clear();
    error_.clear();
    
    std::string delimiter = "--" + boundary;
    std::string end_delimiter = delimiter + "--";
    
    size_t pos = 0;
    while (pos < body.size()) {
        size_t start = body.find(delimiter, pos);
        if (start == std::string::npos) break;
        
        start += delimiter.size();
        if (body.substr(start, 2) == "--") break;
        
        size_t header_end = body.find("\r\n\r\n", start);
        if (header_end == std::string::npos) break;
        
        std::string headers = body.substr(start, header_end - start);
        size_t content_start = header_end + 4;
        
        size_t content_end = body.find(delimiter, content_start);
        if (content_end == std::string::npos) break;
        
        std::string content = body.substr(content_start, content_end - content_start - 2);
        
        size_t name_pos = headers.find("name=\"");
        if (name_pos == std::string::npos) {
            pos = content_end;
            continue;
        }
        
        name_pos += 6;
        size_t name_end = headers.find("\"", name_pos);
        std::string field_name = headers.substr(name_pos, name_end - name_pos);
        
        size_t filename_pos = headers.find("filename=\"");
        if (filename_pos != std::string::npos) {
            filename_pos += 10;
            size_t filename_end = headers.find("\"", filename_pos);
            std::string filename = headers.substr(filename_pos, filename_end - filename_pos);
            
            std::string content_type = "application/octet-stream";
            size_t type_pos = headers.find("Content-Type: ");
            if (type_pos != std::string::npos) {
                type_pos += 14;
                size_t type_end = headers.find("\r\n", type_pos);
                content_type = headers.substr(type_pos, type_end - type_pos);
            }
            
            UploadedFile file;
            file.field_name = field_name;
            file.filename = filename;
            file.content_type = content_type;
            file.data = std::vector<uint8_t>(content.begin(), content.end());
            file.size = file.data.size();
            
            if (!validate_file(file)) {
                return false;
            }
            
            files_.push_back(file);
        } else {
            fields_[field_name] = content;
        }
        
        pos = content_end;
    }
    
    return true;
}

UploadedFile MultipartParser::get_file(const std::string& field_name) const {
    for (const auto& file : files_) {
        if (file.field_name == field_name) {
            return file;
        }
    }
    return UploadedFile{};
}

std::string MultipartParser::get_field(const std::string& field_name) const {
    auto it = fields_.find(field_name);
    return it != fields_.end() ? it->second : "";
}

bool MultipartParser::has_file(const std::string& field_name) const {
    return std::any_of(files_.begin(), files_.end(), 
        [&](const UploadedFile& f) { return f.field_name == field_name; });
}

bool MultipartParser::has_field(const std::string& field_name) const {
    return fields_.find(field_name) != fields_.end();
}

bool MultipartParser::validate_file(const UploadedFile& file) {
    if (file.size > config_.max_file_size) {
        error_ = "File size exceeds maximum allowed size";
        return false;
    }
    
    if (files_.size() >= config_.max_files) {
        error_ = "Maximum number of files exceeded";
        return false;
    }
    
    if (!config_.allowed_extensions.empty()) {
        std::string ext = get_extension(file.filename);
        if (std::find(config_.allowed_extensions.begin(), config_.allowed_extensions.end(), ext) 
            == config_.allowed_extensions.end()) {
            error_ = "File extension not allowed";
            return false;
        }
    }
    
    if (!config_.allowed_mime_types.empty()) {
        if (std::find(config_.allowed_mime_types.begin(), config_.allowed_mime_types.end(), file.content_type) 
            == config_.allowed_mime_types.end()) {
            error_ = "File MIME type not allowed";
            return false;
        }
    }
    
    return true;
}

std::string MultipartParser::get_extension(const std::string& filename) const {
    size_t pos = filename.find_last_of('.');
    if (pos == std::string::npos) return "";
    return filename.substr(pos);
}

void FileUploadMiddleware::handle(Request& req, Response& res, std::function<void()> next) {
    std::string content_type = req.header("Content-Type");
    
    if (content_type.find("multipart/form-data") == std::string::npos) {
        next();
        return;
    }
    
    size_t boundary_pos = content_type.find("boundary=");
    if (boundary_pos == std::string::npos) {
        res.json(400, "{\"error\":\"Missing boundary in Content-Type\"}");
        return;
    }
    
    std::string boundary = content_type.substr(boundary_pos + 9);
    
    MultipartParser parser(config_);
    if (!parser.parse(req.body(), boundary)) {
        res.json(400, "{\"error\":\"" + parser.last_error() + "\"}");
        return;
    }
    
    next();
}

} // namespace crest

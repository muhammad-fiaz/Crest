# File Upload Handling

Handle file uploads with multipart/form-data parsing and validation.

## Overview

Features:
- Multipart form data parsing
- File size limits
- File type validation
- Multiple file uploads
- Save to disk or process in memory

## Basic Usage

```cpp
#include "crest/crest.hpp"
#include "crest/upload.hpp"

int main() {
    crest::App app;
    
    app.post("/upload", [](crest::Request& req, crest::Response& res) {
        std::string content_type = req.header("Content-Type");
        
        if (content_type.find("multipart/form-data") == std::string::npos) {
            res.json(400, R"({"error":"Expected multipart/form-data"})");
            return;
        }
        
        // Extract boundary
        size_t pos = content_type.find("boundary=");
        std::string boundary = content_type.substr(pos + 9);
        
        // Parse multipart data
        crest::MultipartParser parser;
        if (!parser.parse(req.body(), boundary)) {
            res.json(400, R"({"error":")" + parser.last_error() + R"("})");
            return;
        }
        
        // Get uploaded file
        auto file = parser.get_file("file");
        
        if (file.filename.empty()) {
            res.json(400, R"({"error":"No file uploaded"})");
            return;
        }
        
        // Save file
        if (file.save_to("./uploads/" + file.filename)) {
            res.json(200, R"({"message":"File uploaded successfully"})");
        } else {
            res.json(500, R"({"error":"Failed to save file"})");
        }
    });
    
    app.run("0.0.0.0", 8000);
    return 0;
}
```

## Configuration

```cpp
crest::MultipartParser::Config config;
config.max_file_size = 10 * 1024 * 1024; // 10MB
config.max_files = 5;
config.allowed_extensions = {".jpg", ".png", ".pdf"};
config.allowed_mime_types = {"image/jpeg", "image/png", "application/pdf"};

crest::MultipartParser parser(config);
```

## Multiple Files

```cpp
app.post("/upload-multiple", [](crest::Request& req, crest::Response& res) {
    std::string content_type = req.header("Content-Type");
    size_t pos = content_type.find("boundary=");
    std::string boundary = content_type.substr(pos + 9);
    
    crest::MultipartParser parser;
    parser.parse(req.body(), boundary);
    
    auto files = parser.get_files();
    
    int saved = 0;
    for (const auto& file : files) {
        if (file.save_to("./uploads/" + file.filename)) {
            saved++;
        }
    }
    
    res.json(200, R"({"uploaded":)" + std::to_string(saved) + R"(})");
});
```

## File Information

```cpp
auto file = parser.get_file("avatar");

std::cout << "Field name: " << file.field_name << std::endl;
std::cout << "Filename: " << file.filename << std::endl;
std::cout << "Content-Type: " << file.content_type << std::endl;
std::cout << "Size: " << file.size << " bytes" << std::endl;
```

## Process in Memory

```cpp
auto file = parser.get_file("document");

// Get as string
std::string content = file.to_string();

// Get as binary data
std::vector<uint8_t> data = file.data;

// Process data
// ... your processing logic
```

## Form Fields

```cpp
// Get regular form fields
std::string username = parser.get_field("username");
std::string description = parser.get_field("description");

// Get all fields
auto fields = parser.get_fields();
for (const auto& [key, value] : fields) {
    std::cout << key << ": " << value << std::endl;
}
```

## Validation

```cpp
app.post("/upload", [](crest::Request& req, crest::Response& res) {
    std::string content_type = req.header("Content-Type");
    size_t pos = content_type.find("boundary=");
    std::string boundary = content_type.substr(pos + 9);
    
    crest::MultipartParser::Config config;
    config.max_file_size = 5 * 1024 * 1024; // 5MB
    config.allowed_extensions = {".jpg", ".png", ".gif"};
    config.allowed_mime_types = {"image/jpeg", "image/png", "image/gif"};
    
    crest::MultipartParser parser(config);
    
    if (!parser.parse(req.body(), boundary)) {
        res.json(400, R"({"error":")" + parser.last_error() + R"("})");
        return;
    }
    
    auto file = parser.get_file("image");
    
    // Additional validation
    if (file.size < 1024) {
        res.json(400, R"({"error":"File too small"})");
        return;
    }
    
    if (file.filename.length() > 255) {
        res.json(400, R"({"error":"Filename too long"})");
        return;
    }
    
    file.save_to("./uploads/" + file.filename);
    res.json(200, R"({"message":"Upload successful"})");
});
```

## Image Upload Example

```cpp
#include "crest/crest.hpp"
#include "crest/upload.hpp"
#include <filesystem>

int main() {
    crest::App app;
    
    // Create uploads directory
    std::filesystem::create_directories("./uploads");
    
    app.post("/upload/image", [](crest::Request& req, crest::Response& res) {
        std::string content_type = req.header("Content-Type");
        size_t pos = content_type.find("boundary=");
        std::string boundary = content_type.substr(pos + 9);
        
        crest::MultipartParser::Config config;
        config.max_file_size = 5 * 1024 * 1024;
        config.allowed_extensions = {".jpg", ".jpeg", ".png", ".gif"};
        config.allowed_mime_types = {
            "image/jpeg", 
            "image/png", 
            "image/gif"
        };
        
        crest::MultipartParser parser(config);
        
        if (!parser.parse(req.body(), boundary)) {
            res.json(400, R"({"error":")" + parser.last_error() + R"("})");
            return;
        }
        
        auto file = parser.get_file("image");
        
        if (file.filename.empty()) {
            res.json(400, R"({"error":"No image provided"})");
            return;
        }
        
        // Generate unique filename
        std::string unique_name = std::to_string(std::time(nullptr)) + "_" + file.filename;
        std::string path = "./uploads/" + unique_name;
        
        if (file.save_to(path)) {
            res.json(200, R"({"message":"Image uploaded","path":")" + path + R"("})");
        } else {
            res.json(500, R"({"error":"Failed to save image"})");
        }
    });
    
    app.run("0.0.0.0", 8000);
    return 0;
}
```

## HTML Form Example

```html
<!DOCTYPE html>
<html>
<head>
    <title>File Upload</title>
</head>
<body>
    <h1>Upload File</h1>
    <form action="/upload" method="POST" enctype="multipart/form-data">
        <input type="text" name="username" placeholder="Username" required>
        <input type="file" name="file" required>
        <button type="submit">Upload</button>
    </form>
</body>
</html>
```

## Best Practices

- Always validate file size and type
- Use unique filenames to prevent overwrites
- Store files outside web root for security
- Scan uploaded files for malware
- Set appropriate file permissions
- Clean up old uploads periodically
- Use streaming for large files
- Implement rate limiting for uploads
- Provide progress feedback to users
- Handle upload errors gracefully

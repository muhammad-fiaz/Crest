/**
 * @file upload_example.cpp
 * @brief File upload handling example
 */

#include "crest/crest.hpp"
#include "crest/upload.hpp"
#include <iostream>
#include <filesystem>

int main() {
    crest::App app;
    
    // Create uploads directory
    std::filesystem::create_directories("./uploads");
    std::filesystem::create_directories("./uploads/images");
    std::filesystem::create_directories("./uploads/documents");
    
    // Serve upload form
    app.get("/", [](crest::Request& req, crest::Response& res) {
        res.html(200, R"(
<!DOCTYPE html>
<html>
<head>
    <title>File Upload Example</title>
    <style>
        body { font-family: Arial, sans-serif; max-width: 800px; margin: 50px auto; }
        .form-group { margin: 20px 0; }
        label { display: block; margin-bottom: 5px; font-weight: bold; }
        input[type="file"], input[type="text"] { width: 100%; padding: 10px; }
        button { padding: 10px 20px; background: #007bff; color: white; border: none; cursor: pointer; }
        button:hover { background: #0056b3; }
        .result { margin-top: 20px; padding: 15px; background: #f0f0f0; border-radius: 5px; }
    </style>
</head>
<body>
    <h1>File Upload Example</h1>
    
    <h2>Single File Upload</h2>
    <form action="/upload" method="POST" enctype="multipart/form-data">
        <div class="form-group">
            <label>Name:</label>
            <input type="text" name="username" required />
        </div>
        <div class="form-group">
            <label>File:</label>
            <input type="file" name="file" required />
        </div>
        <button type="submit">Upload</button>
    </form>
    
    <h2>Image Upload</h2>
    <form action="/upload/image" method="POST" enctype="multipart/form-data">
        <div class="form-group">
            <label>Image (JPG, PNG, GIF only):</label>
            <input type="file" name="image" accept="image/*" required />
        </div>
        <button type="submit">Upload Image</button>
    </form>
    
    <h2>Multiple Files Upload</h2>
    <form action="/upload/multiple" method="POST" enctype="multipart/form-data">
        <div class="form-group">
            <label>Files (max 5):</label>
            <input type="file" name="files" multiple required />
        </div>
        <button type="submit">Upload Files</button>
    </form>
    
    <div id="result" class="result" style="display:none;"></div>
    
    <script>
        document.querySelectorAll('form').forEach(form => {
            form.addEventListener('submit', async (e) => {
                e.preventDefault();
                const formData = new FormData(form);
                const result = document.getElementById('result');
                
                try {
                    const response = await fetch(form.action, {
                        method: 'POST',
                        body: formData
                    });
                    const data = await response.json();
                    result.style.display = 'block';
                    result.innerHTML = '<pre>' + JSON.stringify(data, null, 2) + '</pre>';
                } catch (error) {
                    result.style.display = 'block';
                    result.innerHTML = '<p style="color:red;">Error: ' + error.message + '</p>';
                }
            });
        });
    </script>
</body>
</html>
        )");
    });
    
    // Single file upload
    app.post("/upload", [](crest::Request& req, crest::Response& res) {
        std::string content_type = req.header("Content-Type");
        
        if (content_type.find("multipart/form-data") == std::string::npos) {
            res.json(400, R"({"error":"Expected multipart/form-data"})");
            return;
        }
        
        size_t pos = content_type.find("boundary=");
        if (pos == std::string::npos) {
            res.json(400, R"({"error":"Missing boundary"})");
            return;
        }
        
        std::string boundary = content_type.substr(pos + 9);
        
        crest::MultipartParser parser;
        if (!parser.parse(req.body(), boundary)) {
            res.json(400, R"({"error":")" + parser.last_error() + R"("})");
            return;
        }
        
        std::string username = parser.get_field("username");
        auto file = parser.get_file("file");
        
        if (file.filename.empty()) {
            res.json(400, R"({"error":"No file uploaded"})");
            return;
        }
        
        std::string path = "./uploads/" + file.filename;
        if (file.save_to(path)) {
            res.json(200, R"({
                "message":"File uploaded successfully",
                "username":")" + username + R"(",
                "filename":")" + file.filename + R"(",
                "size":)" + std::to_string(file.size) + R"(,
                "type":")" + file.content_type + R"(",
                "path":")" + path + R"("
            })");
        } else {
            res.json(500, R"({"error":"Failed to save file"})");
        }
    });
    
    // Image upload with validation
    app.post("/upload/image", [](crest::Request& req, crest::Response& res) {
        std::string content_type = req.header("Content-Type");
        size_t pos = content_type.find("boundary=");
        std::string boundary = content_type.substr(pos + 9);
        
        crest::MultipartParser::Config config;
        config.max_file_size = 5 * 1024 * 1024; // 5MB
        config.allowed_extensions = {".jpg", ".jpeg", ".png", ".gif"};
        config.allowed_mime_types = {"image/jpeg", "image/png", "image/gif"};
        
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
        std::string path = "./uploads/images/" + unique_name;
        
        if (file.save_to(path)) {
            res.json(200, R"({
                "message":"Image uploaded successfully",
                "filename":")" + unique_name + R"(",
                "size":)" + std::to_string(file.size) + R"(,
                "type":")" + file.content_type + R"(",
                "path":")" + path + R"(",
                "url":"/uploads/images/)" + unique_name + R"("
            })");
        } else {
            res.json(500, R"({"error":"Failed to save image"})");
        }
    });
    
    // Multiple files upload
    app.post("/upload/multiple", [](crest::Request& req, crest::Response& res) {
        std::string content_type = req.header("Content-Type");
        size_t pos = content_type.find("boundary=");
        std::string boundary = content_type.substr(pos + 9);
        
        crest::MultipartParser::Config config;
        config.max_file_size = 10 * 1024 * 1024; // 10MB per file
        config.max_files = 5;
        
        crest::MultipartParser parser(config);
        
        if (!parser.parse(req.body(), boundary)) {
            res.json(400, R"({"error":")" + parser.last_error() + R"("})");
            return;
        }
        
        auto files = parser.get_files();
        
        if (files.empty()) {
            res.json(400, R"({"error":"No files uploaded"})");
            return;
        }
        
        std::string response = R"({"message":"Files uploaded","files":[)";
        int saved = 0;
        
        for (size_t i = 0; i < files.size(); ++i) {
            const auto& file = files[i];
            std::string path = "./uploads/" + file.filename;
            
            if (file.save_to(path)) {
                if (saved > 0) response += ",";
                response += R"({"filename":")" + file.filename + R"(",)";
                response += R"("size":)" + std::to_string(file.size) + R"(,)";
                response += R"("type":")" + file.content_type + R"("})";
                saved++;
            }
        }
        
        response += R"(],"total":)" + std::to_string(saved) + "}";
        res.json(200, response);
    });
    
    // List uploaded files
    app.get("/uploads", [](crest::Request& req, crest::Response& res) {
        std::string response = R"({"files":[)";
        int count = 0;
        
        try {
            for (const auto& entry : std::filesystem::directory_iterator("./uploads")) {
                if (entry.is_regular_file()) {
                    if (count > 0) response += ",";
                    response += R"({"name":")" + entry.path().filename().string() + R"(",)";
                    response += R"("size":)" + std::to_string(entry.file_size()) + "}";
                    count++;
                }
            }
        } catch (const std::exception& e) {
            res.json(500, R"({"error":"Failed to list files"})");
            return;
        }
        
        response += "]}";
        res.json(200, response);
    });
    
    // Delete uploaded file
    app.del("/uploads/:filename", [](crest::Request& req, crest::Response& res) {
        // Extract filename from path (simplified)
        std::string path_str = req.path();
        std::string filename = path_str.substr(10); // Remove "/uploads/"
        
        std::string filepath = "./uploads/" + filename;
        
        try {
            if (std::filesystem::remove(filepath)) {
                res.json(200, R"({"message":"File deleted successfully"})");
            } else {
                res.json(404, R"({"error":"File not found"})");
            }
        } catch (const std::exception& e) {
            res.json(500, R"({"error":"Failed to delete file"})");
        }
    });
    
    std::cout << "File Upload Server running on http://0.0.0.0:8000" << std::endl;
    std::cout << "Open http://localhost:8000 in your browser" << std::endl;
    std::cout << std::endl;
    std::cout << "Endpoints:" << std::endl;
    std::cout << "  GET    /              - Upload form" << std::endl;
    std::cout << "  POST   /upload        - Single file upload" << std::endl;
    std::cout << "  POST   /upload/image  - Image upload (validated)" << std::endl;
    std::cout << "  POST   /upload/multiple - Multiple files upload" << std::endl;
    std::cout << "  GET    /uploads       - List uploaded files" << std::endl;
    std::cout << "  DELETE /uploads/:filename - Delete file" << std::endl;
    
    app.run("0.0.0.0", 8000);
    return 0;
}

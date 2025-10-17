/**
 * @file test_upload.cpp
 * @brief Test cases for file upload handling
 */

#include "crest/upload.hpp"
#include <cassert>
#include <iostream>

void test_multipart_parser_creation() {
    std::cout << "Testing multipart parser creation..." << std::endl;
    
    crest::MultipartParser::Config config;
    config.max_file_size = 10 * 1024 * 1024;
    config.max_files = 5;
    config.allowed_extensions = {".jpg", ".png"};
    
    crest::MultipartParser parser(config);
    
    assert(parser.get_files().empty());
    assert(parser.get_fields().empty());
    
    std::cout << "  ✓ Multipart parser created" << std::endl;
}

void test_uploaded_file() {
    std::cout << "Testing uploaded file structure..." << std::endl;
    
    crest::UploadedFile file;
    file.field_name = "avatar";
    file.filename = "test.jpg";
    file.content_type = "image/jpeg";
    file.data = {'t', 'e', 's', 't'};
    file.size = 4;
    
    assert(file.field_name == "avatar");
    assert(file.filename == "test.jpg");
    assert(file.size == 4);
    
    std::string content = file.to_string();
    assert(content == "test");
    
    std::cout << "  ✓ Uploaded file structure validated" << std::endl;
}

void test_multipart_parser_validation() {
    std::cout << "Testing multipart parser validation..." << std::endl;
    
    crest::MultipartParser::Config config;
    config.max_file_size = 1024;
    config.allowed_extensions = {".txt"};
    
    crest::MultipartParser parser(config);
    
    std::cout << "  ✓ Parser validation configured" << std::endl;
}

int main() {
    std::cout << "\n=== File Upload Tests ===" << std::endl;
    
    test_multipart_parser_creation();
    test_uploaded_file();
    test_multipart_parser_validation();
    
    std::cout << "\n✅ All file upload tests passed!" << std::endl;
    return 0;
}

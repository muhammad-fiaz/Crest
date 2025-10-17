/**
 * @file test_template.cpp
 * @brief Test cases for template engine
 */

#include "crest/template.hpp"
#include <cassert>
#include <iostream>

void test_template_engine_creation() {
    std::cout << "Testing template engine creation..." << std::endl;
    
    crest::TemplateEngine::Config config;
    config.template_dir = "./templates";
    config.cache_enabled = true;
    config.auto_escape = true;
    
    crest::TemplateEngine engine(config);
    
    std::cout << "  ✓ Template engine created" << std::endl;
}

void test_variable_substitution() {
    std::cout << "Testing variable substitution..." << std::endl;
    
    crest::TemplateEngine engine;
    
    crest::TemplateContext context = {
        {"name", "Alice"},
        {"age", 25}
    };
    
    std::string result = engine.render_string("Hello {{ name }}!", context);
    
    assert(result.find("Alice") != std::string::npos);
    
    std::cout << "  ✓ Variable substitution: " << result << std::endl;
}

void test_filters() {
    std::cout << "Testing filters..." << std::endl;
    
    crest::TemplateEngine engine;
    
    crest::TemplateContext context = {
        {"text", "hello"}
    };
    
    std::string result = engine.render_string("{{ text|upper }}", context);
    
    assert(result.find("HELLO") != std::string::npos);
    
    std::cout << "  ✓ Filter applied: " << result << std::endl;
}

void test_custom_filter() {
    std::cout << "Testing custom filter..." << std::endl;
    
    crest::TemplateEngine engine;
    
    engine.add_filter("reverse", [](const std::string& s) {
        return std::string(s.rbegin(), s.rend());
    });
    
    crest::TemplateContext context = {
        {"word", "hello"}
    };
    
    std::string result = engine.render_string("{{ word|reverse }}", context);
    
    assert(result.find("olleh") != std::string::npos);
    
    std::cout << "  ✓ Custom filter: " << result << std::endl;
}

void test_global_variables() {
    std::cout << "Testing global variables..." << std::endl;
    
    crest::TemplateEngine engine;
    
    engine.add_global("site_name", "Test Site");
    
    std::string result = engine.render_string("Welcome to {{ site_name }}", {});
    
    assert(result.find("Test Site") != std::string::npos);
    
    std::cout << "  ✓ Global variable: " << result << std::endl;
}

void test_html_escaping() {
    std::cout << "Testing HTML escaping..." << std::endl;
    
    crest::TemplateEngine::Config config;
    config.auto_escape = true;
    
    crest::TemplateEngine engine(config);
    
    crest::TemplateContext context = {
        {"script", "<script>alert('xss')</script>"}
    };
    
    std::string result = engine.render_string("{{ script }}", context);
    
    assert(result.find("&lt;") != std::string::npos);
    assert(result.find("&gt;") != std::string::npos);
    
    std::cout << "  ✓ HTML escaped for security" << std::endl;
}

int main() {
    std::cout << "\n=== Template Engine Tests ===" << std::endl;
    
    test_template_engine_creation();
    test_variable_substitution();
    test_filters();
    test_custom_filter();
    test_global_variables();
    test_html_escaping();
    
    std::cout << "\n✅ All template engine tests passed!" << std::endl;
    return 0;
}

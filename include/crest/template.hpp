/**
 * @file template.hpp
 * @brief Template engine for Crest framework
 * @version 0.0.0
 */

#ifndef CREST_TEMPLATE_HPP
#define CREST_TEMPLATE_HPP

#include "crest.hpp"
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <variant>

namespace crest {

using TemplateValue = std::variant<std::string, int, double, bool, std::nullptr_t>;
using TemplateContext = std::map<std::string, TemplateValue>;

class TemplateEngine {
public:
    struct Config {
        std::string template_dir = "./templates";
        std::string extension = ".html";
        bool cache_enabled = true;
        bool auto_escape = true;
    };
    
    explicit TemplateEngine(const Config& config = Config());
    
    std::string render(const std::string& template_name, const TemplateContext& context = {});
    std::string render_string(const std::string& template_str, const TemplateContext& context = {});
    
    void add_filter(const std::string& name, std::function<std::string(const std::string&)> filter);
    void add_global(const std::string& name, const TemplateValue& value);
    
    void clear_cache();

private:
    Config config_;
    std::map<std::string, std::string> cache_;
    std::map<std::string, std::function<std::string(const std::string&)>> filters_;
    TemplateContext globals_;
    
    std::string load_template(const std::string& template_name);
    std::string process_template(const std::string& template_str, const TemplateContext& context);
    std::string replace_variables(const std::string& str, const TemplateContext& context);
    std::string process_conditionals(const std::string& str, const TemplateContext& context);
    std::string process_loops(const std::string& str, const TemplateContext& context);
    std::string escape_html(const std::string& str);
    std::string value_to_string(const TemplateValue& value);
};

class TemplateResponse {
public:
    TemplateResponse(Response& res, TemplateEngine& engine) 
        : res_(res), engine_(engine) {}
    
    void render(int status, const std::string& template_name, const TemplateContext& context = {});

private:
    Response& res_;
    TemplateEngine& engine_;
};

} // namespace crest

#endif /* CREST_TEMPLATE_HPP */

/**
 * @file template.cpp
 * @brief Template engine implementation
 */

#include "crest/template.hpp"
#include <fstream>
#include <sstream>
#include <regex>

namespace crest {

TemplateEngine::TemplateEngine(const Config& config) : config_(config) {
    add_filter("upper", [](const std::string& s) {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    });
    
    add_filter("lower", [](const std::string& s) {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    });
    
    add_filter("capitalize", [](const std::string& s) {
        if (s.empty()) return s;
        std::string result = s;
        result[0] = ::toupper(result[0]);
        return result;
    });
}

std::string TemplateEngine::render(const std::string& template_name, const TemplateContext& context) {
    std::string template_str;
    
    if (config_.cache_enabled && cache_.find(template_name) != cache_.end()) {
        template_str = cache_[template_name];
    } else {
        template_str = load_template(template_name);
        if (config_.cache_enabled) {
            cache_[template_name] = template_str;
        }
    }
    
    TemplateContext merged_context = globals_;
    merged_context.insert(context.begin(), context.end());
    
    return process_template(template_str, merged_context);
}

std::string TemplateEngine::render_string(const std::string& template_str, const TemplateContext& context) {
    TemplateContext merged_context = globals_;
    merged_context.insert(context.begin(), context.end());
    return process_template(template_str, merged_context);
}

void TemplateEngine::add_filter(const std::string& name, std::function<std::string(const std::string&)> filter) {
    filters_[name] = filter;
}

void TemplateEngine::add_global(const std::string& name, const TemplateValue& value) {
    globals_[name] = value;
}

void TemplateEngine::clear_cache() {
    cache_.clear();
}

std::string TemplateEngine::load_template(const std::string& template_name) {
    std::string path = config_.template_dir + "/" + template_name;
    if (path.find(config_.extension) == std::string::npos) {
        path += config_.extension;
    }
    
    std::ifstream file(path);
    if (!file) {
        return "Template not found: " + template_name;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string TemplateEngine::process_template(const std::string& template_str, const TemplateContext& context) {
    std::string result = template_str;
    result = process_loops(result, context);
    result = process_conditionals(result, context);
    result = replace_variables(result, context);
    return result;
}

std::string TemplateEngine::replace_variables(const std::string& str, const TemplateContext& context) {
    std::string result = str;
    std::regex var_regex(R"(\{\{\s*(\w+)(?:\|(\w+))?\s*\}\})");
    std::smatch match;
    
    std::string::const_iterator search_start(result.cbegin());
    while (std::regex_search(search_start, result.cend(), match, var_regex)) {
        std::string var_name = match[1].str();
        std::string filter_name = match[2].str();
        
        auto it = context.find(var_name);
        if (it != context.end()) {
            std::string value = value_to_string(it->second);
            
            if (!filter_name.empty() && filters_.find(filter_name) != filters_.end()) {
                value = filters_[filter_name](value);
            }
            
            if (config_.auto_escape) {
                value = escape_html(value);
            }
            
            size_t pos = match.position() + (search_start - result.cbegin());
            result.replace(pos, match.length(), value);
            search_start = result.cbegin() + pos + value.length();
        } else {
            search_start = match.suffix().first;
        }
    }
    
    return result;
}

std::string TemplateEngine::process_conditionals(const std::string& str, const TemplateContext& context) {
    std::string result = str;
    std::regex if_regex(R"(\{%\s*if\s+(\w+)\s*%\}(.*?)\{%\s*endif\s*%\})");
    std::smatch match;
    
    while (std::regex_search(result, match, if_regex)) {
        std::string var_name = match[1].str();
        std::string content = match[2].str();
        
        bool condition = false;
        auto it = context.find(var_name);
        if (it != context.end()) {
            if (std::holds_alternative<bool>(it->second)) {
                condition = std::get<bool>(it->second);
            } else if (std::holds_alternative<int>(it->second)) {
                condition = std::get<int>(it->second) != 0;
            } else if (std::holds_alternative<std::string>(it->second)) {
                condition = !std::get<std::string>(it->second).empty();
            }
        }
        
        result.replace(match.position(), match.length(), condition ? content : "");
    }
    
    return result;
}

std::string TemplateEngine::process_loops(const std::string& str, const TemplateContext& context) {
    return str;
}

std::string TemplateEngine::escape_html(const std::string& str) {
    std::string result;
    result.reserve(str.size());
    
    for (char c : str) {
        switch (c) {
            case '&': result += "&amp;"; break;
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '"': result += "&quot;"; break;
            case '\'': result += "&#39;"; break;
            default: result += c;
        }
    }
    
    return result;
}

std::string TemplateEngine::value_to_string(const TemplateValue& value) {
    if (std::holds_alternative<std::string>(value)) {
        return std::get<std::string>(value);
    } else if (std::holds_alternative<int>(value)) {
        return std::to_string(std::get<int>(value));
    } else if (std::holds_alternative<double>(value)) {
        return std::to_string(std::get<double>(value));
    } else if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value) ? "true" : "false";
    }
    return "";
}

void TemplateResponse::render(int status, const std::string& template_name, const TemplateContext& context) {
    std::string html = engine_.render(template_name, context);
    res_.html(status, html);
}

} // namespace crest

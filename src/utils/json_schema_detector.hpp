/**
 * @file json_schema_detector.hpp
 * @brief Automatic JSON schema detection from response data
 */

#ifndef CREST_JSON_SCHEMA_DETECTOR_HPP
#define CREST_JSON_SCHEMA_DETECTOR_HPP

#include <string>
#include <cstring>

namespace crest {

inline std::string detect_json_type(const char* json, size_t& pos) {
    while (pos < strlen(json) && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n')) pos++;
    
    if (pos >= strlen(json)) return "\"string\"";
    
    char c = json[pos];
    
    if (c == '"') return "\"string\"";
    if (c == 't' || c == 'f') return "\"boolean\"";
    if (c == 'n') return "\"null\"";
    if (c == '[') return "\"array\"";
    if (c == '{') return "\"object\"";
    if ((c >= '0' && c <= '9') || c == '-') return "\"number\"";
    
    return "\"string\"";
}

inline std::string detect_schema_from_json(const char* json) {
    if (!json || !*json) return "{}";
    
    std::string result;
    size_t len = strlen(json);
    size_t pos = 0;
    
    // Skip whitespace
    while (pos < len && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n')) pos++;
    
    if (pos >= len) return "{}";
    
    // Handle array
    if (json[pos] == '[') {
        return "\"array\"";
    }
    
    // Handle object
    if (json[pos] != '{') return "{}";
    
    result += "{";
    pos++; // Skip '{'
    
    bool first = true;
    bool in_string = false;
    std::string current_key;
    
    while (pos < len) {
        char c = json[pos];
        
        // Skip whitespace
        if (!in_string && (c == ' ' || c == '\t' || c == '\n')) {
            pos++;
            continue;
        }
        
        // Handle string quotes
        if (c == '"' && (pos == 0 || json[pos-1] != '\\')) {
            if (!in_string) {
                // Start of key
                in_string = true;
                current_key.clear();
            } else {
                // End of key
                in_string = false;
                
                // Look for colon
                size_t colon_pos = pos + 1;
                while (colon_pos < len && (json[colon_pos] == ' ' || json[colon_pos] == '\t')) colon_pos++;
                
                if (colon_pos < len && json[colon_pos] == ':') {
                    // Found key-value pair
                    if (!first) result += ",";
                    first = false;
                    
                    result += "\"" + current_key + "\":";
                    
                    // Detect value type
                    size_t value_pos = colon_pos + 1;
                    std::string type = detect_json_type(json, value_pos);
                    result += type;
                    
                    // Skip to next comma or end
                    int depth = 0;
                    bool in_val_string = false;
                    while (value_pos < len) {
                        char vc = json[value_pos];
                        if (vc == '"' && (value_pos == 0 || json[value_pos-1] != '\\')) {
                            in_val_string = !in_val_string;
                        }
                        if (!in_val_string) {
                            if (vc == '{' || vc == '[') depth++;
                            else if (vc == '}' || vc == ']') {
                                if (depth == 0) break;
                                depth--;
                            }
                            else if (vc == ',' && depth == 0) break;
                        }
                        value_pos++;
                    }
                    pos = value_pos;
                    continue;
                }
            }
        } else if (in_string) {
            current_key += c;
        }
        
        // Handle end of object
        if (!in_string && c == '}') {
            break;
        }
        
        pos++;
    }
    
    result += "}";
    return result;
}

} // namespace crest

#endif // CREST_JSON_SCHEMA_DETECTOR_HPP

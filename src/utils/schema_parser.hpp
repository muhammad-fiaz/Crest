/**
 * @file schema_parser.hpp
 * @brief JSON schema parser for dynamic type detection
 */

#ifndef CREST_SCHEMA_PARSER_HPP
#define CREST_SCHEMA_PARSER_HPP

#include <string>
#include <sstream>

namespace crest {

inline std::string parse_json_to_schema(const char* json) {
    if (!json || !*json) return "{}";
    
    std::string result;
    std::string input(json);
    size_t pos = 0;
    bool in_string = false;
    bool in_value = false;
    std::string current_key;
    
    while (pos < input.length()) {
        char c = input[pos];
        
        if (c == '"' && (pos == 0 || input[pos-1] != '\\')) {
            in_string = !in_string;
            if (!in_string && !current_key.empty() && in_value) {
                result += "\"string\"";
                current_key.clear();
                in_value = false;
            }
        } else if (!in_string) {
            if (c == ':') {
                in_value = true;
                result += c;
                pos++;
                // Skip whitespace
                while (pos < input.length() && (input[pos] == ' ' || input[pos] == '\t')) pos++;
                if (pos < input.length()) {
                    char next = input[pos];
                    if (next == '"') {
                        // String value - will be handled by quote logic
                    } else if (next == 't' || next == 'f') {
                        // Boolean
                        result += "\"boolean\"";
                        while (pos < input.length() && input[pos] != ',' && input[pos] != '}') pos++;
                        in_value = false;
                        continue;
                    } else if (next == 'n') {
                        // null
                        result += "\"null\"";
                        while (pos < input.length() && input[pos] != ',' && input[pos] != '}') pos++;
                        in_value = false;
                        continue;
                    } else if ((next >= '0' && next <= '9') || next == '-') {
                        // Number
                        result += "\"number\"";
                        while (pos < input.length() && input[pos] != ',' && input[pos] != '}') pos++;
                        in_value = false;
                        continue;
                    } else if (next == '{') {
                        result += "\"object\"";
                        int depth = 0;
                        while (pos < input.length()) {
                            if (input[pos] == '{') depth++;
                            else if (input[pos] == '}') {
                                depth--;
                                if (depth == 0) {
                                    pos++;
                                    break;
                                }
                            }
                            pos++;
                        }
                        in_value = false;
                        continue;
                    } else if (next == '[') {
                        result += "\"array\"";
                        int depth = 0;
                        while (pos < input.length()) {
                            if (input[pos] == '[') depth++;
                            else if (input[pos] == ']') {
                                depth--;
                                if (depth == 0) {
                                    pos++;
                                    break;
                                }
                            }
                            pos++;
                        }
                        in_value = false;
                        continue;
                    }
                }
            } else if (c == '{' || c == '}' || c == ',' || c == '[' || c == ']') {
                result += c;
            } else if (in_string) {
                result += c;
            }
        } else {
            result += c;
        }
        pos++;
    }
    
    return result;
}

} // namespace crest

#endif // CREST_SCHEMA_PARSER_HPP

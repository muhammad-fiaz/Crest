/**
 * @file response.cpp
 * @brief C++ response utilities for schema detection
 */

#include "crest/internal/app_internal.h"
#include "../utils/json_schema_detector.hpp"
#include <cstring>
#include <cstdlib>

extern "C" {

void crest_response_detect_schema(crest_response_t* res, const char* json) {
    if (!res || !json) return;
    
    std::string schema = crest::detect_schema_from_json(json);
    
    free(res->detected_schema);
    res->detected_schema = strdup(schema.c_str());
}

}

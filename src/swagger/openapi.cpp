/**
 * @file openapi.cpp
 * @brief OpenAPI specification generation
 */

#include "crest/internal/app_internal.h"
#include <string>
#include <sstream>

namespace crest {
namespace swagger {

std::string generate_openapi_spec(const crest_app_t* app) {
    std::ostringstream oss;
    
    oss << "{\n";
    oss << "  \"openapi\": \"3.0.0\",\n";
    oss << "  \"info\": {\n";
    oss << "    \"title\": \"" << (app->title ? app->title : "API") << "\",\n";
    oss << "    \"description\": \"" << (app->description ? app->description : "") << "\",\n";
    oss << "    \"version\": \"" << (app->version ? app->version : "0.0.0") << "\"\n";
    oss << "  },\n";
    oss << "  \"servers\": [{\"url\": \"/\"}],\n";
    oss << "  \"paths\": {\n";
    
    for (size_t i = 0; i < app->route_count; i++) {
        const char* method = "";
        switch (app->routes[i].method) {
            case CREST_GET: method = "get"; break;
            case CREST_POST: method = "post"; break;
            case CREST_PUT: method = "put"; break;
            case CREST_DELETE: method = "delete"; break;
            case CREST_PATCH: method = "patch"; break;
            default: method = "get"; break;
        }
        
        if (i > 0) oss << ",\n";
        oss << "    \"" << app->routes[i].path << "\": {\n";
        oss << "      \"" << method << "\": {\n";
        oss << "        \"summary\": \"" << (app->routes[i].description ? app->routes[i].description : "No description") << "\",\n";
        oss << "        \"responses\": {\n";
        oss << "          \"200\": {\"description\": \"Successful response\"}\n";
        oss << "        }\n";
        oss << "      }\n";
        oss << "    }";
    }
    
    oss << "\n  }\n";
    oss << "}\n";
    
    return oss.str();
}

} // namespace swagger
} // namespace crest

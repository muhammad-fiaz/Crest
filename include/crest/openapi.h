/**
 * @file openapi.h
 * @brief OpenAPI 3.0 specification generator for Crest Framework
 * @details Automatically generates OpenAPI 3.0 compliant JSON specifications
 *          from registered routes, enabling Swagger UI and ReDoc integration.
 */

#ifndef CREST_OPENAPI_H
#define CREST_OPENAPI_H

#include "crest/crest.h"
#include "crest/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Generate OpenAPI 3.0 JSON specification from registered routes
 * 
 * @param app The Crest application instance with registered routes
 * @return Dynamically allocated JSON string containing OpenAPI 3.0 spec
 *         Caller is responsible for freeing the returned string with free()
 * @note Returns NULL if app is NULL or memory allocation fails
 */
CREST_API char* crest_generate_openapi_spec(crest_app_t *app);

/**
 * @brief Free OpenAPI spec string allocated by crest_generate_openapi_spec
 * 
 * @param spec The OpenAPI spec string to free
 */
CREST_API void crest_free_openapi_spec(char *spec);

#ifdef __cplusplus
}
#endif

#endif /* CREST_OPENAPI_H */

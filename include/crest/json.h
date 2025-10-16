/**
 * @file json.h
 * @brief JSON utility functions for Crest
 */

#ifndef CREST_JSON_H
#define CREST_JSON_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* DLL export/import macro */
#ifdef _WIN32
    #ifdef CREST_STATIC
        /* Static library - no dll import/export */
        #define CREST_API
    #elif defined(CREST_BUILD_SHARED)
        #define CREST_API __declspec(dllexport)
    #else
        #define CREST_API __declspec(dllimport)
    #endif
#else
    #define CREST_API
#endif

/* JSON value types */
typedef enum {
    CREST_JSON_NULL,
    CREST_JSON_BOOL,
    CREST_JSON_NUMBER,
    CREST_JSON_STRING,
    CREST_JSON_ARRAY,
    CREST_JSON_OBJECT
} crest_json_type_t;

/* Forward declaration */
typedef struct crest_json_value crest_json_value_t;

/**
 * @brief Parse JSON string
 * @param json JSON string
 * @return Parsed JSON value or NULL on error
 */
CREST_API crest_json_value_t* crest_json_parse(const char *json);

/**
 * @brief Create JSON object
 */
CREST_API crest_json_value_t* crest_json_object(void);

/**
 * @brief Create JSON array
 */
crest_json_value_t* crest_json_array(void);

/**
 * @brief Create JSON string
 */
CREST_API crest_json_value_t* crest_json_string(const char *value);

/**
 * @brief Create JSON number
 */
CREST_API crest_json_value_t* crest_json_number(double value);

/**
 * @brief Create JSON boolean
 */
crest_json_value_t* crest_json_bool(bool value);

/**
 * @brief Create JSON null
 */
crest_json_value_t* crest_json_null(void);

/**
 * @brief Set object property
 */
CREST_API void crest_json_set(crest_json_value_t *obj, const char *key, crest_json_value_t *value);

/**
 * @brief Get object property
 */
CREST_API crest_json_value_t* crest_json_get(crest_json_value_t *obj, const char *key);

/**
 * @brief Add item to array
 */
void crest_json_array_add(crest_json_value_t *arr, crest_json_value_t *value);

/**
 * @brief Get array item
 */
crest_json_value_t* crest_json_array_get(crest_json_value_t *arr, size_t index);

/**
 * @brief Get array size
 */
CREST_API size_t crest_json_array_size(crest_json_value_t *arr);

/**
 * @brief Get JSON value type
 */
CREST_API crest_json_type_t crest_json_type(crest_json_value_t *value);

/**
 * @brief Serialize to JSON string
 */
CREST_API char* crest_json_stringify(crest_json_value_t *value);

/**
 * @brief Get string value
 */
CREST_API const char* crest_json_as_string(crest_json_value_t *value);

/**
 * @brief Get number value
 */
CREST_API double crest_json_as_number(crest_json_value_t *value);

/**
 * @brief Get boolean value
 */
CREST_API bool crest_json_as_bool(crest_json_value_t *value);

/**
 * @brief Free JSON value
 */
CREST_API void crest_json_free(crest_json_value_t *value);

#ifdef __cplusplus
}
#endif

#endif /* CREST_JSON_H */

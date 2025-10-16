/**
 * @file string_utils.c
 * @brief String utility functions for Crest
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

char* crest_str_trim(const char *str) {
    if (!str) return NULL;
    
    while (isspace((unsigned char)*str)) str++;
    
    if (*str == 0) return strdup("");
    
    const char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    
    size_t len = end - str + 1;
    char *result = (char*)malloc(len + 1);
    memcpy(result, str, len);
    result[len] = '\0';
    
    return result;
}

char* crest_str_tolower(const char *str) {
    if (!str) return NULL;
    
    char *result = strdup(str);
    for (char *p = result; *p; p++) {
        *p = tolower((unsigned char)*p);
    }
    
    return result;
}

char* crest_str_toupper(const char *str) {
    if (!str) return NULL;
    
    char *result = strdup(str);
    for (char *p = result; *p; p++) {
        *p = toupper((unsigned char)*p);
    }
    
    return result;
}

int crest_str_equals(const char *a, const char *b) {
    if (!a || !b) return 0;
    return strcmp(a, b) == 0;
}

int crest_str_equals_ignore_case(const char *a, const char *b) {
    if (!a || !b) return 0;
    return strcasecmp(a, b) == 0;
}

/**
 * @file string_utils.c
 * @brief String utility functions
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

char* crest_strdup(const char* str) {
    if (!str) return NULL;
    size_t len = strlen(str) + 1;
    char* dup = (char*)malloc(len);
    if (dup) memcpy(dup, str, len);
    return dup;
}

void crest_str_tolower(char* str) {
    if (!str) return;
    for (size_t i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

int crest_str_startswith(const char* str, const char* prefix) {
    if (!str || !prefix) return 0;
    size_t len = strlen(prefix);
    return strncmp(str, prefix, len) == 0;
}

int crest_str_endswith(const char* str, const char* suffix) {
    if (!str || !suffix) return 0;
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    if (suffix_len > str_len) return 0;
    return strcmp(str + str_len - suffix_len, suffix) == 0;
}

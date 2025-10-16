/**
 * @file response.c
 * @brief Response handling implementation
 */

#include "crest/crest.h"
#include "crest/types.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define open _open
#define close _close
#define read _read
#define O_RDONLY _O_RDONLY
#define O_BINARY _O_BINARY
typedef long long ssize_t;
#define strcasecmp _stricmp
#else
#include <unistd.h>
#endif

void crest_response_status(crest_response_t *res, crest_status_t status) {
    if (res) {
        res->status_code = status;
    }
}

void crest_response_header(crest_response_t *res, const char *key, const char *value) {
    if (!res || !key || !value) {
        return;
    }
    
    if (res->header_count >= CREST_MAX_HEADERS) {
        return;
    }
    
    res->headers[res->header_count].key = strdup(key);
    res->headers[res->header_count].value = strdup(value);
    res->header_count++;
}

void crest_response_send(crest_response_t *res, const char *body) {
    if (!res || !body) {
        return;
    }
    
    res->body = strdup(body);
    res->body_len = strlen(body);
    res->sent = true;
    
    // Set default content type if not set
    bool has_content_type = false;
    for (size_t i = 0; i < res->header_count; i++) {
        if (strcasecmp(res->headers[i].key, "Content-Type") == 0) {
            has_content_type = true;
            break;
        }
    }
    
    if (!has_content_type) {
        crest_response_header(res, "Content-Type", "text/plain");
    }
}

void crest_response_json(crest_response_t *res, const char *json) {
    if (!res || !json) {
        return;
    }
    
    crest_response_header(res, "Content-Type", "application/json");
    crest_response_send(res, json);
}

/**
 * @file response.c
 * @brief Response handling implementation
 */

#include "crest/crest.h"
#include "crest/types.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#ifdef _WIN32
#define strcasecmp _stricmp
#include <windows.h>
#include <io.h>
#define open _open
#define close _close
#define read _read
#define O_RDONLY _O_RDONLY
#define O_BINARY _O_BINARY
#else
#include <unistd.h>
#endif

/* ====================================================================
 * MIME TYPE DETECTION
 * ==================================================================== */

static const struct {
    const char *extension;
    const char *mime_type;
} mime_types[] = {
    {".html", "text/html"},
    {".htm", "text/html"},
    {".css", "text/css"},
    {".js", "application/javascript"},
    {".json", "application/json"},
    {".xml", "application/xml"},
    {".txt", "text/plain"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".svg", "image/svg+xml"},
    {".ico", "image/x-icon"},
    {".pdf", "application/pdf"},
    {".zip", "application/zip"},
    {".tar", "application/x-tar"},
    {".gz", "application/gzip"},
    {".mp4", "video/mp4"},
    {".mp3", "audio/mpeg"},
    {".wav", "audio/wav"},
    {".woff", "font/woff"},
    {".woff2", "font/woff2"},
    {".ttf", "font/ttf"},
    {".otf", "font/otf"},
    {NULL, NULL}
};

static const char* get_mime_type(const char *filepath) {
    if (!filepath) return "application/octet-stream";

    const char *ext = strrchr(filepath, '.');
    if (!ext) return "application/octet-stream";

    for (size_t i = 0; mime_types[i].extension; i++) {
        if (strcasecmp(ext, mime_types[i].extension) == 0) {
            return mime_types[i].mime_type;
        }
    }

    return "application/octet-stream";
}

/* ====================================================================
 * SECURITY CHECKS
 * ==================================================================== */

static bool is_path_safe(const char *filepath) {
    if (!filepath) return false;

    /* Check for directory traversal attempts */
    if (strstr(filepath, "..") != NULL) {
        return false;
    }

    /* Check for absolute paths */
    if (filepath[0] == '/' || filepath[0] == '\\') {
        return false;
    }

    /* Check for drive letters on Windows */
#ifdef _WIN32
    if (strlen(filepath) >= 3 && filepath[1] == ':' && (filepath[2] == '/' || filepath[2] == '\\')) {
        return false;
    }
#endif

    return true;
}

/* ====================================================================
 * FILE SERVING IMPLEMENTATION
 * ==================================================================== */

void crest_response_file(crest_response_t *res, const char *filepath) {
    if (!res || !filepath) {
        crest_response_status(res, CREST_STATUS_BAD_REQUEST);
        crest_response_json(res, "{\"error\":\"Invalid file path\"}");
        return;
    }

    /* Security check */
    if (!is_path_safe(filepath)) {
        crest_response_status(res, CREST_STATUS_FORBIDDEN);
        crest_response_json(res, "{\"error\":\"Access denied\"}");
        return;
    }

    /* Open file */
    int fd = open(filepath, O_RDONLY | O_BINARY);
    if (fd < 0) {
        if (errno == ENOENT) {
            crest_response_status(res, CREST_STATUS_NOT_FOUND);
            crest_response_json(res, "{\"error\":\"File not found\"}");
        } else {
            crest_response_status(res, CREST_STATUS_INTERNAL_SERVER_ERROR);
            crest_response_json(res, "{\"error\":\"Failed to open file\"}");
        }
        return;
    }

    /* Get file size */
    struct stat st;
    if (fstat(fd, &st) != 0) {
        close(fd);
        crest_response_status(res, CREST_STATUS_INTERNAL_SERVER_ERROR);
        crest_response_json(res, "{\"error\":\"Failed to get file info\"}");
        return;
    }

    size_t file_size = st.st_size;

    /* Check if file is too large (limit to 100MB for safety) */
    if (file_size > 104857600) {
        close(fd);
        crest_response_status(res, CREST_STATUS_REQUEST_ENTITY_TOO_LARGE);
        crest_response_json(res, "{\"error\":\"File too large\"}");
        return;
    }

    /* Read file content */
    char *content = (char*)malloc(file_size);
    if (!content) {
        close(fd);
        crest_response_status(res, CREST_STATUS_INTERNAL_SERVER_ERROR);
        crest_response_json(res, "{\"error\":\"Memory allocation failed\"}");
        return;
    }

    ssize_t bytes_read = read(fd, content, file_size);
    close(fd);

    if (bytes_read != (ssize_t)file_size) {
        free(content);
        crest_response_status(res, CREST_STATUS_INTERNAL_SERVER_ERROR);
        crest_response_json(res, "{\"error\":\"Failed to read file\"}");
        return;
    }

    /* Set headers */
    const char *mime_type = get_mime_type(filepath);
    crest_response_header(res, "Content-Type", mime_type);
    crest_response_header(res, "Content-Length", ""); // Will be set by server

    /* Set cache headers for static assets */
    if (strstr(mime_type, "text/") || strstr(mime_type, "application/javascript") ||
        strstr(mime_type, "application/json") || strstr(mime_type, "image/")) {
        crest_response_header(res, "Cache-Control", "public, max-age=3600");
    }

    /* Send response */
    res->body = content; // Transfer ownership
    res->body_len = file_size;
    res->sent = true;
}

/* ====================================================================
 * RANGE REQUEST SUPPORT (OPTIONAL ENHANCEMENT)
 * ==================================================================== */

void crest_response_file_range(crest_response_t *res, const char *filepath,
                              size_t start, size_t end) {
    if (!res || !filepath || start >= end) {
        crest_response_status(res, CREST_STATUS_BAD_REQUEST);
        crest_response_json(res, "{\"error\":\"Invalid range request\"}");
        return;
    }

    /* Security check */
    if (!is_path_safe(filepath)) {
        crest_response_status(res, CREST_STATUS_FORBIDDEN);
        crest_response_json(res, "{\"error\":\"Access denied\"}");
        return;
    }

    /* Open file */
    int fd = open(filepath, O_RDONLY | O_BINARY);
    if (fd < 0) {
        crest_response_status(res, CREST_STATUS_NOT_FOUND);
        crest_response_json(res, "{\"error\":\"File not found\"}");
        return;
    }

    /* Get file size */
    struct stat st;
    if (fstat(fd, &st) != 0) {
        close(fd);
        crest_response_status(res, CREST_STATUS_INTERNAL_SERVER_ERROR);
        crest_response_json(res, "{\"error\":\"Failed to get file info\"}");
        return;
    }

    size_t file_size = st.st_size;

    /* Validate range */
    if (end > file_size) {
        end = file_size;
    }
    size_t range_size = end - start;

    /* Seek to start position */
    if (lseek(fd, start, SEEK_SET) < 0) {
        close(fd);
        crest_response_status(res, CREST_STATUS_INTERNAL_SERVER_ERROR);
        crest_response_json(res, "{\"error\":\"Failed to seek file\"}");
        return;
    }

    /* Read range content */
    char *content = (char*)malloc(range_size);
    if (!content) {
        close(fd);
        crest_response_status(res, CREST_STATUS_INTERNAL_SERVER_ERROR);
        crest_response_json(res, "{\"error\":\"Memory allocation failed\"}");
        return;
    }

    ssize_t bytes_read = read(fd, content, range_size);
    close(fd);

    if (bytes_read != (ssize_t)range_size) {
        free(content);
        crest_response_status(res, CREST_STATUS_INTERNAL_SERVER_ERROR);
        crest_response_json(res, "{\"error\":\"Failed to read file range\"}");
        return;
    }

    /* Set headers */
    const char *mime_type = get_mime_type(filepath);
    crest_response_header(res, "Content-Type", mime_type);
    crest_response_header(res, "Content-Length", ""); // Will be set by server
    crest_response_header(res, "Accept-Ranges", "bytes");

    char range_header[128];
    snprintf(range_header, sizeof(range_header), "bytes %zu-%zu/%zu", start, end - 1, file_size);
    crest_response_header(res, "Content-Range", range_header);

    crest_response_status(res, CREST_STATUS_PARTIAL_CONTENT);

    /* Send response */
    res->body = content; // Transfer ownership
    res->body_len = range_size;
    res->sent = true;
}

void crest_response_sendf(crest_response_t *res, const char *format, ...) {
    if (!res || !format) {
        return;
    }
    
    char buffer[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    crest_response_send(res, buffer);
}

/**
 * @file config.c
 * @brief Configuration management with JSON/TOML file support
 */

#include "crest/crest.h"
#include "crest/types.h"
#include "crest/json.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#define open _open
#define close _close
#define read _read
#define O_RDONLY _O_RDONLY
#define O_BINARY _O_BINARY
#define strdup _strdup
typedef long long ssize_t;
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif

/* ====================================================================
 * CONFIG CREATION AND DEFAULTS
 * ==================================================================== */

crest_config_t* crest_config_create(void) {
    crest_config_t *config = (crest_config_t*)calloc(1, sizeof(crest_config_t));
    if (!config) {
        return NULL;
    }

    // Default configuration
    config->host = strdup("127.0.0.1");
    config->port = 3000;
    config->enable_logging = true;
    config->log_level = CREST_LOG_INFO;
    config->enable_cors = false;
    config->enable_dashboard = false;
    config->dashboard_path = strdup("/__crest__/dashboard");
    config->max_body_size = 10485760; // 10 MB
    config->timeout_seconds = 60;
    config->static_dir = NULL;
    config->upload_dir = NULL;
    config->thread_count = 4; // Default 4 threads
    config->rate_limit_max_requests = 100; // 100 requests
    config->rate_limit_window_seconds = 60; // per minute
    config->read_timeout_ms = 30000; // 30 seconds
    config->write_timeout_ms = 30000; // 30 seconds

    return config;
}

/* ====================================================================
 * FILE READING HELPERS
 * ==================================================================== */

static char* read_file_content(const char *filepath, size_t *size) {
    if (!filepath || !size) return NULL;

    int fd = open(filepath, O_RDONLY | O_BINARY);
    if (fd < 0) return NULL;

    /* Get file size */
    struct stat st;
    if (fstat(fd, &st) != 0) {
        close(fd);
        return NULL;
    }

    size_t file_size = st.st_size;
    if (file_size == 0) {
        close(fd);
        *size = 0;
        return strdup("");
    }

    char *content = malloc(file_size + 1);
    if (!content) {
        close(fd);
        return NULL;
    }

    ssize_t bytes_read = read(fd, content, file_size);
    close(fd);

    if (bytes_read != (ssize_t)file_size) {
        free(content);
        return NULL;
    }

    content[file_size] = '\0';
    *size = file_size;
    return content;
}

/* ====================================================================
 * JSON CONFIG PARSING
 * ==================================================================== */

static bool parse_json_config(crest_config_t *config, const char *json_str) {
    crest_json_value_t *root = crest_json_parse(json_str);
    if (!root || crest_json_type(root) != CREST_JSON_OBJECT) {
        return false;
    }

    /* Parse server settings */
    crest_json_value_t *server = crest_json_get(root, "server");
    if (server && crest_json_type(server) == CREST_JSON_OBJECT) {
        /* Host */
        crest_json_value_t *host = crest_json_get(server, "host");
        if (host && crest_json_type(host) == CREST_JSON_STRING) {
            free(config->host);
            config->host = strdup(crest_json_as_string(host));
        }

        /* Port */
        crest_json_value_t *port = crest_json_get(server, "port");
        if (port && crest_json_type(port) == CREST_JSON_NUMBER) {
            config->port = (int)crest_json_as_number(port);
        }

        /* Timeout */
        crest_json_value_t *timeout = crest_json_get(server, "timeout");
        if (timeout && crest_json_type(timeout) == CREST_JSON_NUMBER) {
            config->timeout_seconds = (int)crest_json_as_number(timeout);
        }

        /* Max body size */
        crest_json_value_t *max_body = crest_json_get(server, "max_body_size");
        if (max_body && crest_json_type(max_body) == CREST_JSON_NUMBER) {
            config->max_body_size = (size_t)crest_json_as_number(max_body);
        }

        /* Thread count */
        crest_json_value_t *thread_count = crest_json_get(server, "thread_count");
        if (thread_count && crest_json_type(thread_count) == CREST_JSON_NUMBER) {
            config->thread_count = (int)crest_json_as_number(thread_count);
        }

        /* Rate limiting */
        crest_json_value_t *rate_limit = crest_json_get(server, "rate_limit");
        if (rate_limit && crest_json_type(rate_limit) == CREST_JSON_OBJECT) {
            crest_json_value_t *max_req = crest_json_get(rate_limit, "max_requests");
            if (max_req && crest_json_type(max_req) == CREST_JSON_NUMBER) {
                config->rate_limit_max_requests = (int)crest_json_as_number(max_req);
            }
            crest_json_value_t *window = crest_json_get(rate_limit, "window_seconds");
            if (window && crest_json_type(window) == CREST_JSON_NUMBER) {
                config->rate_limit_window_seconds = (int)crest_json_as_number(window);
            }
        }

        /* Timeouts */
        crest_json_value_t *timeouts = crest_json_get(server, "timeouts");
        if (timeouts && crest_json_type(timeouts) == CREST_JSON_OBJECT) {
            crest_json_value_t *read_timeout = crest_json_get(timeouts, "read_ms");
            if (read_timeout && crest_json_type(read_timeout) == CREST_JSON_NUMBER) {
                config->read_timeout_ms = (int)crest_json_as_number(read_timeout);
            }
            crest_json_value_t *write_timeout = crest_json_get(timeouts, "write_ms");
            if (write_timeout && crest_json_type(write_timeout) == CREST_JSON_NUMBER) {
                config->write_timeout_ms = (int)crest_json_as_number(write_timeout);
            }
        }
    }

    /* Parse middleware settings */
    crest_json_value_t *middleware = crest_json_get(root, "middleware");
    if (middleware && crest_json_type(middleware) == CREST_JSON_OBJECT) {
        /* CORS */
        crest_json_value_t *cors = crest_json_get(middleware, "cors");
        if (cors && crest_json_type(cors) == CREST_JSON_BOOL) {
            config->enable_cors = crest_json_as_bool(cors);
        }

        /* Logging */
        crest_json_value_t *logging = crest_json_get(middleware, "logging");
        if (logging && crest_json_type(logging) == CREST_JSON_BOOL) {
            config->enable_logging = crest_json_as_bool(logging);
        }

        /* Log level */
        crest_json_value_t *log_level = crest_json_get(middleware, "log_level");
        if (log_level && crest_json_type(log_level) == CREST_JSON_STRING) {
            const char *level_str = crest_json_as_string(log_level);
            if (strcmp(level_str, "debug") == 0) {
                config->log_level = CREST_LOG_DEBUG;
            } else if (strcmp(level_str, "info") == 0) {
                config->log_level = CREST_LOG_INFO;
            } else if (strcmp(level_str, "warn") == 0) {
                config->log_level = CREST_LOG_WARN;
            } else if (strcmp(level_str, "error") == 0) {
                config->log_level = CREST_LOG_ERROR;
            }
        }

        /* Dashboard */
        crest_json_value_t *dashboard = crest_json_get(middleware, "dashboard");
        if (dashboard && crest_json_type(dashboard) == CREST_JSON_BOOL) {
            config->enable_dashboard = crest_json_as_bool(dashboard);
        }

        /* Dashboard path */
        crest_json_value_t *dash_path = crest_json_get(middleware, "dashboard_path");
        if (dash_path && crest_json_type(dash_path) == CREST_JSON_STRING) {
            free(config->dashboard_path);
            config->dashboard_path = strdup(crest_json_as_string(dash_path));
        }
    }

    /* Parse paths */
    crest_json_value_t *paths = crest_json_get(root, "paths");
    if (paths && crest_json_type(paths) == CREST_JSON_OBJECT) {
        /* Static directory */
        crest_json_value_t *static_dir = crest_json_get(paths, "static");
        if (static_dir && crest_json_type(static_dir) == CREST_JSON_STRING) {
            free(config->static_dir);
            config->static_dir = strdup(crest_json_as_string(static_dir));
        }

        /* Upload directory */
        crest_json_value_t *upload_dir = crest_json_get(paths, "upload");
        if (upload_dir && crest_json_type(upload_dir) == CREST_JSON_STRING) {
            free(config->upload_dir);
            config->upload_dir = strdup(crest_json_as_string(upload_dir));
        }
    }

    crest_json_free(root);
    return true;
}

/* ====================================================================
 * TOML CONFIG PARSING (SIMPLIFIED)
 * ==================================================================== */

static bool parse_toml_config(crest_config_t *config, const char *toml_str) {
    /* Basic TOML parsing - simplified implementation */
    /* In a full implementation, you'd use a proper TOML parser */
    char *content = strdup(toml_str);
    if (!content) return false;

    char *line = strtok(content, "\n");
    while (line) {
        /* Skip comments and empty lines */
        if (*line == '#' || *line == '\0') {
            line = strtok(NULL, "\n");
            continue;
        }

        /* Parse key-value pairs */
        char *eq = strchr(line, '=');
        if (eq) {
            *eq = '\0';
            char *key = line;
            char *value = eq + 1;

            /* Trim whitespace */
            while (isspace(*key)) key++;
            char *end = key + strlen(key) - 1;
            while (end > key && isspace(*end)) *end-- = '\0';

            while (isspace(*value)) value++;
            end = value + strlen(value) - 1;
            while (end > value && isspace(*end)) *end-- = '\0';

            /* Remove quotes */
            if (*value == '"' || *value == '\'') {
                size_t len = strlen(value);
                if (value[len-1] == *value) {
                    value[len-1] = '\0';
                    value++;
                }
            }

            /* Parse configuration values */
            if (strcmp(key, "host") == 0) {
                free(config->host);
                config->host = strdup(value);
            } else if (strcmp(key, "port") == 0) {
                config->port = atoi(value);
            } else if (strcmp(key, "timeout") == 0) {
                config->timeout_seconds = atoi(value);
            } else if (strcmp(key, "max_body_size") == 0) {
                config->max_body_size = (size_t)atol(value);
            } else if (strcmp(key, "enable_cors") == 0) {
                config->enable_cors = (strcmp(value, "true") == 0);
            } else if (strcmp(key, "enable_logging") == 0) {
                config->enable_logging = (strcmp(value, "true") == 0);
            } else if (strcmp(key, "enable_dashboard") == 0) {
                config->enable_dashboard = (strcmp(value, "true") == 0);
            } else if (strcmp(key, "log_level") == 0) {
                if (strcmp(value, "debug") == 0) config->log_level = CREST_LOG_DEBUG;
                else if (strcmp(value, "info") == 0) config->log_level = CREST_LOG_INFO;
                else if (strcmp(value, "warn") == 0) config->log_level = CREST_LOG_WARN;
                else if (strcmp(value, "error") == 0) config->log_level = CREST_LOG_ERROR;
            } else if (strcmp(key, "dashboard_path") == 0) {
                free(config->dashboard_path);
                config->dashboard_path = strdup(value);
            } else if (strcmp(key, "static_dir") == 0) {
                free(config->static_dir);
                config->static_dir = strdup(value);
            } else if (strcmp(key, "upload_dir") == 0) {
                free(config->upload_dir);
                config->upload_dir = strdup(value);
            }
        }

        line = strtok(NULL, "\n");
    }

    free(content);
    return true;
}

/* ====================================================================
 * CONFIG FILE LOADING
 * ==================================================================== */

crest_config_t* crest_config_load(const char *filepath) {
    if (!filepath) return NULL;

    crest_config_t *config = crest_config_create();
    if (!config) return NULL;

    size_t size;
    char *content = read_file_content(filepath, &size);
    if (!content) {
        return config; /* Return defaults on error */
    }

    bool success = false;

    /* Detect file type by extension */
    const char *ext = strrchr(filepath, '.');
    if (ext) {
        if (strcmp(ext, ".json") == 0) {
            success = parse_json_config(config, content);
        } else if (strcmp(ext, ".toml") == 0) {
            success = parse_toml_config(config, content);
        }
    }

    /* Try JSON if extension doesn't match */
    if (!success && strstr(content, "{") && strstr(content, "}")) {
        success = parse_json_config(config, content);
    }

    /* Try TOML if JSON failed */
    if (!success) {
        success = parse_toml_config(config, content);
    }

    free(content);

    if (!success) {
        crest_log(CREST_LOG_WARN, "Failed to parse config file '%s'", filepath);
    }

    return config;
}

/* ====================================================================
 * CONFIG VALIDATION
 * ==================================================================== */

bool crest_config_validate(const crest_config_t *config) {
    if (!config) return false;

    /* Validate host */
    if (!config->host || strlen(config->host) == 0) {
        return false;
    }

    /* Validate port */
    if (config->port < 1 || config->port > 65535) {
        return false;
    }

    /* Validate timeout */
    if (config->timeout_seconds < 1 || config->timeout_seconds > 3600) {
        return false;
    }

    /* Validate max body size */
    if (config->max_body_size < 1024 || config->max_body_size > 1073741824) { // 1KB to 1GB
        return false;
    }

    /* Validate log level */
    if (config->log_level < CREST_LOG_DEBUG || config->log_level > CREST_LOG_ERROR) {
        return false;
    }

    /* Validate paths */
    if (config->dashboard_path && strlen(config->dashboard_path) == 0) {
        return false;
    }

    return true;
}

/* ====================================================================
 * CONFIG UTILITIES
 * ==================================================================== */

void crest_config_print(const crest_config_t *config) {
    if (!config) return;

    crest_log(CREST_LOG_INFO, "Crest Configuration:");
    crest_log(CREST_LOG_INFO, "  Server:");
    crest_log(CREST_LOG_INFO, "    Host: %s", config->host);
    crest_log(CREST_LOG_INFO, "    Port: %d", config->port);
    crest_log(CREST_LOG_INFO, "    Timeout: %d seconds", config->timeout_seconds);
    crest_log(CREST_LOG_INFO, "    Max Body Size: %zu bytes", config->max_body_size);
    crest_log(CREST_LOG_INFO, "    Thread Count: %d", config->thread_count);
    crest_log(CREST_LOG_INFO, "    Rate Limit: %d requests per %d seconds", config->rate_limit_max_requests, config->rate_limit_window_seconds);
    crest_log(CREST_LOG_INFO, "    Read Timeout: %d ms", config->read_timeout_ms);
    crest_log(CREST_LOG_INFO, "    Write Timeout: %d ms", config->write_timeout_ms);
    crest_log(CREST_LOG_INFO, "  Middleware:");
    crest_log(CREST_LOG_INFO, "    CORS: %s", config->enable_cors ? "enabled" : "disabled");
    crest_log(CREST_LOG_INFO, "    Logging: %s (level %d)", config->enable_logging ? "enabled" : "disabled", config->log_level);
    crest_log(CREST_LOG_INFO, "    Dashboard: %s", config->enable_dashboard ? "enabled" : "disabled");
    if (config->dashboard_path) {
        crest_log(CREST_LOG_INFO, "    Dashboard Path: %s", config->dashboard_path);
    }
    crest_log(CREST_LOG_INFO, "  Paths:");
    if (config->static_dir) {
        crest_log(CREST_LOG_INFO, "    Static Directory: %s", config->static_dir);
    }
    if (config->upload_dir) {
        crest_log(CREST_LOG_INFO, "    Upload Directory: %s", config->upload_dir);
    }
}

/* ====================================================================
 * CONFIG DESTRUCTION
 * ==================================================================== */

void crest_config_destroy(crest_config_t *config) {
    if (!config) {
        return;
    }

    free(config->host);
    free(config->dashboard_path);
    free(config->static_dir);
    free(config->upload_dir);
    free(config);
}

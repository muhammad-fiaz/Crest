/**
 * @file logger.c
 * @brief Logging utilities for Crest
 */

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define CREST_LOG_DEBUG 0
#define CREST_LOG_INFO  1
#define CREST_LOG_WARN  2
#define CREST_LOG_ERROR 3

static int current_log_level = CREST_LOG_INFO;
static int logging_enabled = 1;

#ifdef _WIN32
static HANDLE console_handle = NULL;
static WORD original_attributes = 0;

static void init_console(void) {
    if (!console_handle) {
        console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (GetConsoleScreenBufferInfo(console_handle, &info)) {
            original_attributes = info.wAttributes;
        }
    }
}

static void set_console_color(int level) {
    if (!console_handle) init_console();
    
    WORD color;
    switch (level) {
        case CREST_LOG_DEBUG: color = FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
        case CREST_LOG_INFO:  color = FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
        case CREST_LOG_WARN:  color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
        case CREST_LOG_ERROR: color = FOREGROUND_RED | FOREGROUND_INTENSITY; break;
        default: color = original_attributes; break;
    }
    SetConsoleTextAttribute(console_handle, color);
}

static void reset_console_color(void) {
    if (console_handle) {
        SetConsoleTextAttribute(console_handle, original_attributes);
    }
}
#endif

void crest_log_set_level(int level) {
    current_log_level = level;
}

void crest_log_enable(int enable) {
    logging_enabled = enable;
}

static const char* get_timestamp(void) {
    static char buffer[32];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", t);
    return buffer;
}

void crest_log(int level, const char *format, ...) {
    if (!logging_enabled || level < current_log_level) {
        return;
    }
    
    const char *level_str[] = {"DEBUG", "INFO", "WARN", "ERROR"};
    
#ifdef _WIN32
    // Windows console colors
    set_console_color(level);
    printf("[%s] [%s] ", get_timestamp(), level_str[level]);
    reset_console_color();
#else
    // ANSI escape codes for Unix-like systems
    const char *color[] = {"\033[36m", "\033[32m", "\033[33m", "\033[31m"};
    const char *reset = "\033[0m";
    printf("%s[%s] [%s]%s ", color[level], get_timestamp(), level_str[level], reset);
#endif
    
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    
    printf("\n");
}

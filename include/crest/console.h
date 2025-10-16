/**
 * @file console.h
 * @brief Cross-platform console color and formatting utilities
 */

#ifndef CREST_CONSOLE_H
#define CREST_CONSOLE_H

#include "crest/crest.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Console color definitions */
#ifdef _WIN32
#include <windows.h>

#define CREST_COLOR_BLACK     0
#define CREST_COLOR_BLUE      (FOREGROUND_BLUE)
#define CREST_COLOR_GREEN     (FOREGROUND_GREEN)
#define CREST_COLOR_CYAN      (FOREGROUND_GREEN | FOREGROUND_BLUE)
#define CREST_COLOR_RED       (FOREGROUND_RED)
#define CREST_COLOR_MAGENTA   (FOREGROUND_RED | FOREGROUND_BLUE)
#define CREST_COLOR_YELLOW    (FOREGROUND_RED | FOREGROUND_GREEN)
#define CREST_COLOR_WHITE     (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define CREST_COLOR_GRAY      (FOREGROUND_INTENSITY)
#define CREST_COLOR_BRIGHT_BLUE     (FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define CREST_COLOR_BRIGHT_GREEN    (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define CREST_COLOR_BRIGHT_CYAN     (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define CREST_COLOR_BRIGHT_RED      (FOREGROUND_RED | FOREGROUND_INTENSITY)
#define CREST_COLOR_BRIGHT_MAGENTA  (FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY)
#define CREST_COLOR_BRIGHT_YELLOW   (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define CREST_COLOR_BRIGHT_WHITE    (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)

#else

#define CREST_COLOR_BLACK     "\033[30m"
#define CREST_COLOR_BLUE      "\033[34m"
#define CREST_COLOR_GREEN     "\033[32m"
#define CREST_COLOR_CYAN      "\033[36m"
#define CREST_COLOR_RED       "\033[31m"
#define CREST_COLOR_MAGENTA   "\033[35m"
#define CREST_COLOR_YELLOW    "\033[33m"
#define CREST_COLOR_WHITE     "\033[37m"
#define CREST_COLOR_GRAY      "\033[90m"
#define CREST_COLOR_BRIGHT_BLUE     "\033[94m"
#define CREST_COLOR_BRIGHT_GREEN    "\033[92m"
#define CREST_COLOR_BRIGHT_CYAN     "\033[96m"
#define CREST_COLOR_BRIGHT_RED      "\033[91m"
#define CREST_COLOR_BRIGHT_MAGENTA  "\033[95m"
#define CREST_COLOR_BRIGHT_YELLOW   "\033[93m"
#define CREST_COLOR_BRIGHT_WHITE    "\033[97m"
#define CREST_COLOR_RESET     "\033[0m"

#endif

/**
 * Initialize console color support
 * Must be called before using any color functions
 */
CREST_API void crest_console_init(void);

/**
 * Set console text color
 * @param color Color code (use CREST_COLOR_* constants)
 */
CREST_API void crest_console_set_color(int color);

/**
 * Reset console color to default
 */
CREST_API void crest_console_reset_color(void);

/**
 * Print colored text to console
 * @param color Color code (use CREST_COLOR_* constants)
 * @param text Text to print
 */
CREST_API void crest_console_print_color(int color, const char *text);

/**
 * Print colored text with newline
 * @param color Color code (use CREST_COLOR_* constants)
 * @param text Text to print
 */
CREST_API void crest_console_println_color(int color, const char *text);

#ifdef __cplusplus
}
#endif

#endif /* CREST_CONSOLE_H */
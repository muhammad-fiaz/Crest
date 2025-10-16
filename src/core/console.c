/**
 * @file console.c
 * @brief Cross-platform console color and formatting utilities implementation
 */

#include "crest/console.h"
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>

static HANDLE hConsole = NULL;
static WORD originalConsoleAttrs = 0;

void crest_console_init(void) {
    if (!hConsole) {
        hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE) {
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
                originalConsoleAttrs = csbi.wAttributes;
            }
        }
    }
}

void crest_console_set_color(int color) {
    if (hConsole && hConsole != INVALID_HANDLE_VALUE) {
        SetConsoleTextAttribute(hConsole, (WORD)color);
    }
}

void crest_console_reset_color(void) {
    if (hConsole && hConsole != INVALID_HANDLE_VALUE) {
        SetConsoleTextAttribute(hConsole, originalConsoleAttrs);
    }
}

void crest_console_print_color(int color, const char *text) {
    crest_console_set_color(color);
    printf("%s", text);
    crest_console_reset_color();
}

void crest_console_println_color(int color, const char *text) {
    crest_console_print_color(color, text);
    printf("\n");
}

#else

void crest_console_init(void) {
    // No initialization needed for ANSI escape sequences
}

void crest_console_set_color(int color) {
    printf("%s", (const char*)color);
}

void crest_console_reset_color(void) {
    printf("%s", CREST_COLOR_RESET);
}

void crest_console_print_color(int color, const char *text) {
    printf("%s%s%s", (const char*)color, text, CREST_COLOR_RESET);
}

void crest_console_println_color(int color, const char *text) {
    crest_console_print_color(color, text);
    printf("\n");
}

#endif
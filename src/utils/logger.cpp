/**
 * @file logger.cpp
 * @brief Console logging for Crest
 */

#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <cstring>
#include <ctime>

#ifdef CREST_WINDOWS
    #include <windows.h>
#else
    #define RESET "\033[0m"
    #define RED "\033[31m"
    #define GREEN "\033[32m"
    #define YELLOW "\033[33m"
    #define BLUE "\033[34m"
    #define MAGENTA "\033[35m"
    #define CYAN "\033[36m"
#endif

static bool log_enabled = true;
static bool timestamp_enabled = true;

static void print_timestamp() {
    if (!timestamp_enabled) return;
    
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    printf("[%04d-%02d-%02d %02d:%02d:%02d] ", 
           t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
           t->tm_hour, t->tm_min, t->tm_sec);
}

extern "C" {

void crest_log_set_enabled(bool enabled) {
    log_enabled = enabled;
}

void crest_log_set_timestamp(bool enabled) {
    timestamp_enabled = enabled;
}

void crest_log_info(const char* msg) {
    if (!log_enabled) return;
    
    #ifdef CREST_WINDOWS
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        print_timestamp();
        SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        printf("[INFO] %s\n", msg);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    #else
        print_timestamp();
        printf("%s[INFO] %s%s\n", BLUE, msg, RESET);
    #endif
}

void crest_log_success(const char* msg) {
    if (!log_enabled) return;
    
    #ifdef CREST_WINDOWS
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        print_timestamp();
        SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        printf("[SUCCESS] %s\n", msg);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    #else
        print_timestamp();
        printf("%s[SUCCESS] %s%s\n", GREEN, msg, RESET);
    #endif
}

void crest_log_error(const char* msg) {
    if (!log_enabled) return;
    
    #ifdef CREST_WINDOWS
        HANDLE hConsole = GetStdHandle(STD_ERROR_HANDLE);
        print_timestamp();
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
        fprintf(stderr, "[ERROR] %s\n", msg);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    #else
        print_timestamp();
        fprintf(stderr, "%s[ERROR] %s%s\n", RED, msg, RESET);
    #endif
}

void crest_log_warning(const char* msg) {
    if (!log_enabled) return;
    
    #ifdef CREST_WINDOWS
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        print_timestamp();
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        printf("[WARNING] %s\n", msg);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    #else
        print_timestamp();
        printf("%s[WARNING] %s%s\n", YELLOW, msg, RESET);
    #endif
}

void crest_log_request(const char* method, const char* path, int status) {
    if (!log_enabled) return;
    
    #ifdef CREST_WINDOWS
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        print_timestamp();
        if (status >= 200 && status < 300) {
            SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
        } else if (status >= 400) {
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
        } else {
            SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
        }
        printf("[REQUEST] %s %s -> %d\n", method, path, status);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    #else
        const char* color = (status >= 200 && status < 300) ? GREEN : (status >= 400) ? RED : BLUE;
        print_timestamp();
        printf("%s[REQUEST] %s %s -> %d%s\n", color, method, path, status, RESET);
    #endif
}

} // extern "C"

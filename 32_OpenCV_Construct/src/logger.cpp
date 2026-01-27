#define _GNU_SOURCE
#include "logger.h"
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>

#ifdef _WIN32
    #include <io.h>
    #include <windows.h>
    #define localtime_r(t, tm) localtime_s(tm, t)
    #define isatty(fd) _isatty(fd)
    #define STDOUT_FILENO 1
#else
    #include <unistd.h>
    #include <pthread.h>
#endif

// 配置开关
#define LOGGER_ENABLE_COLOR 1
#define LOGGER_ENABLE_FILE  1
#define LOGGER_ENABLE_MUTEX 1

#if LOGGER_ENABLE_COLOR
static const char* g_log_colors[] = {
    "\033[37m",  // DEBUG - white
    "\033[32m",  // INFO  - green
    "\033[33m",  // WARN  - yellow
    "\033[31m",  // ERROR - red
    "\033[35m"   // FATAL - magenta
};
#define COLOR_RESET "\033[0m"
#else
#define COLOR_RESET ""
#endif

static const char* g_log_names[] = {
    "DEBUG", "INFO ", "WARN ", "ERROR", "FATAL"
};

// 全局状态
static LogLevel_t g_level = LOG_LEVEL_INFO;
static FILE* g_fp = NULL;
static bool g_use_color = true;
static void (*g_fatal_cb)(void) = NULL;

#if LOGGER_ENABLE_MUTEX
#ifdef _WIN32
static CRITICAL_SECTION g_mtx;
static volatile long g_mtx_init = 0;
#else
static pthread_mutex_t g_mtx = PTHREAD_MUTEX_INITIALIZER;
#endif
#endif

// 锁操作
#if LOGGER_ENABLE_MUTEX
static void lock_log(void) {
#ifdef _WIN32
    if (!g_mtx_init) {
        if (InterlockedCompareExchange(&g_mtx_init, 1, 0) == 0) {
            InitializeCriticalSection(&g_mtx);
            InterlockedExchange(&g_mtx_init, 2);
        } else {
            while (g_mtx_init == 1) Sleep(1);
        }
    }
    EnterCriticalSection(&g_mtx);
#else
    pthread_mutex_lock(&g_mtx);
#endif
}
static void unlock_log(void) {
#ifdef _WIN32
    LeaveCriticalSection(&g_mtx);
#else
    pthread_mutex_unlock(&g_mtx);
#endif
}
#else
#define lock_log()
#define unlock_log()
#endif

static bool should_color(void) {
#if LOGGER_ENABLE_COLOR
    return g_use_color && isatty(STDOUT_FILENO);
#else
    return false;
#endif
}

// ======== 核心实现 ========
void log_set_level(LogLevel_t level) {
    if (level < LOG_LEVEL_COUNT) g_level = level;
}

void log_set_color(bool enable) {
    g_use_color = enable;
}

void log_set_output_file(const char* path) {
#if LOGGER_ENABLE_FILE
    lock_log();
    if (g_fp && g_fp != stdout && g_fp != stderr) fclose(g_fp);
    if (path) {
        g_fp = fopen(path, "a");
        if (!g_fp) {
            fprintf(stderr, "[LOGGER] Failed to open log file: %s\n", path);
            g_fp = stdout;
        }
    } else {
        g_fp = stdout;
    }
    unlock_log();
#else
    (void)path;
#endif
}

void log_set_fatal_callback(void (*cb)(void)) {
    g_fatal_cb = cb;
}

void _log_print(LogLevel_t level, const char* mod,
                const char* file, int line, const char* fmt, ...) {
    if (level < g_level) return;

    lock_log();

    time_t now = time(NULL);
    struct tm tm_info;
    localtime_r(&now, &tm_info);

    FILE* out = g_fp ? g_fp : stdout;
    bool use_color = should_color() && (out == stdout || out == stderr);

    char prefix[256];
    const char* color_start = "";
    const char* color_end = "";

#if LOGGER_ENABLE_COLOR
    if (use_color) {
        color_start = g_log_colors[level];
        color_end = COLOR_RESET;
    }
#endif

    // 完整时间戳：2026-01-20 16:45:30
    if (mod && mod[0]) {
        snprintf(prefix, sizeof(prefix),
                 "%s[%04d-%02d-%02d %02d:%02d:%02d] [%s] [%s] [%s:%d] ",
                 color_start,
                 tm_info.tm_year + 1900,
                 tm_info.tm_mon + 1,
                 tm_info.tm_mday,
                 tm_info.tm_hour,
                 tm_info.tm_min,
                 tm_info.tm_sec,
                 g_log_names[level],
                 mod,
                 file, line);
    } else {
        snprintf(prefix, sizeof(prefix),
                 "%s[%04d-%02d-%02d %02d:%02d:%02d] [%s] [%s:%d] ",
                 color_start,
                 tm_info.tm_year + 1900,
                 tm_info.tm_mon + 1,
                 tm_info.tm_mday,
                 tm_info.tm_hour,
                 tm_info.tm_min,
                 tm_info.tm_sec,
                 g_log_names[level],
                 file, line);
    }

    fputs(prefix, out);

    va_list args;
    va_start(args, fmt);
    vfprintf(out, fmt, args);
    va_end(args);

    fprintf(out, "%s\n", color_end);
    fflush(out);

    unlock_log();

    if (level == LOG_LEVEL_FATAL) {
        if (g_fatal_cb) g_fatal_cb();
        exit(EXIT_FAILURE);
    }
}
#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL,
    LOG_LEVEL_COUNT
} LogLevel_t;

// ======== 公共 API ========
void log_set_level(LogLevel_t level);
void log_set_color(bool enable);
void log_set_output_file(const char* filepath);
void log_set_fatal_callback(void (*callback)(void));

// 主日志函数（内部使用）
void _log_print(LogLevel_t level, const char* module,
                const char* file, int line, const char* fmt, ...);

// ======== 用户宏 ========
#ifndef LOG_MODULE_NAME
#define LOG_MODULE_NAME NULL
#endif

#define LOG_DEBUG(fmt, ...)   _log_print(LOG_LEVEL_DEBUG, LOG_MODULE_NAME, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)    _log_print(LOG_LEVEL_INFO,  LOG_MODULE_NAME, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)    _log_print(LOG_LEVEL_WARN,  LOG_MODULE_NAME, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)   _log_print(LOG_LEVEL_ERROR, LOG_MODULE_NAME, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_FATAL(fmt, ...)   _log_print(LOG_LEVEL_FATAL, LOG_MODULE_NAME, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // LOGGER_H_
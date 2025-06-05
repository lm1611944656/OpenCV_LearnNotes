/*************************************************************************
* 
*   Copyright (C), 2017-2027, BPG. Co., Ltd.
* 
*   文件名称: imgclassification.h
*   软件模块: 图像分类
*   版 本 号: 1.0
*   生成日期: 2025-05-29
*   作    者: lium
*   功    能: 图像分类头文件定义
* 
************************************************************************/
#ifndef LOGGING_H
#define LOGGING_H

// a logging wrapper so it can be easily replaced
#include <stdio.h>

// log level from low to high
// 0: no log
// 1: error
// 2: error, warning
// 3: error, warning, info
// 4: error, warning, info, debug
static unsigned char g_log_level = 4;

// a printf wrapper so the msg can be formatted with %d %s, etc.

#define NN_LOG_ERROR(...)          \
    do                             \
    {                              \
        if (g_log_level >= 1)      \
        {                          \
            printf("[NN_ERROR] "); \
            printf(__VA_ARGS__);   \
            printf("\n");          \
        }                          \
    } while (0)

#define NN_LOG_WARNING(...)          \
    do                               \
    {                                \
        if (g_log_level >= 2)        \
        {                            \
            printf("[NN_WARNING] "); \
            printf(__VA_ARGS__);     \
            printf("\n");            \
        }                            \
    } while (0)

#define NN_LOG_INFO(...)          \
    do                            \
    {                             \
        if (g_log_level >= 3)     \
        {                         \
            printf("[NN_INFO] "); \
            printf(__VA_ARGS__);  \
            printf("\n");         \
        }                         \
    } while (0)

#define NN_LOG_DEBUG(...)          \
    do                             \
    {                              \
        if (g_log_level >= 4)      \
        {                          \
            printf("[NN_DEBUG] "); \
            printf(__VA_ARGS__);   \
            printf("\n");          \
        }                          \
    } while (0)

#endif // LOGGING_H

/*************************************************************************
* 改动历史纪录：
* Revision 1.0, 2025-05-29, lium
* describe: 初始创建.
*************************************************************************/



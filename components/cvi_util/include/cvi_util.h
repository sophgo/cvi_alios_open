#ifndef __CVI_UTIL_H__
#define __CVI_UTIL_H__

#include <stdio.h>
#include <stdarg.h>

#include "cvi_type.h"

/* Log macro define */
#define LOG_UTIL_LEVEL 2

#define UTIL_ERRO 0
#define UTIL_WARN 1
#define UTIL_INFO 2
#define UTIL_DBUG 3

#define LOG_UTIL(level, fmt, ...)                                    \
    do {                                                                \
        if (level <= LOG_UTIL_LEVEL) {                               \
            switch (level) {                                            \
            case UTIL_ERRO:                                          \
                printf("[UTIL_ERRO]");                               \
                break;                                                  \
            case UTIL_WARN:                                          \
                printf("[UTIL_WARN]");                               \
                break;                                                  \
            case UTIL_INFO:                                          \
                printf("[UTIL_INFO]");                               \
                break;                                                  \
            case UTIL_DBUG:                                          \
                printf("[UTIL_DBUG]");                               \
                break;                                                  \
            default:                                                    \
                break;                                                  \
            }                                                           \
            printf("(%s) %d: " fmt, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                               \
    } while (0)

#endif
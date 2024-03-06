#ifndef __CVITDL_TYPES_H__
#define __CVITDL_TYPES_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* 整型 */
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long long s64;
typedef unsigned long long u64;

/* 浮点数 */
typedef float f32;
typedef double f64;

/* 字符类型 */
typedef char c8;  // 字符（可能是有符号或无符号，平台依赖）

/* 指针类型 */
typedef void* pvoid;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CVITDL_TYPES_H__ */
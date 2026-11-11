/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __METAL_METAL_LOG__H__
#define __METAL_METAL_LOG__H__

#ifdef __cplusplus
extern "C" {
#endif

enum metal_log_level {
    METAL_LOG_EMERGENCY,
    METAL_LOG_ALERT,
    METAL_LOG_CRITICAL,
    METAL_LOG_ERROR,
    METAL_LOG_WARNING,
    METAL_LOG_NOTICE,
    METAL_LOG_INFO,
    METAL_LOG_DEBUG,
};

typedef void (*metal_log_handler)(enum metal_log_level level,
                                  const char *format, ...);

void metal_set_log_handler(metal_log_handler handler);
metal_log_handler metal_get_log_handler(void);
void metal_set_log_level(enum metal_log_level level);
enum metal_log_level metal_get_log_level(void);
void metal_default_log_handler(enum metal_log_level level,
                               const char *format, ...);

#define metal_fmt(fmt) fmt

#define metal_log(level, fmt, ...) ({ \
    if (_metal.common.log_handler && level <= _metal.common.log_level) \
        _metal.common.log_handler(level, metal_fmt(fmt), ## __VA_ARGS__); \
})

#define metal_err(fmt, args...) metal_log(METAL_LOG_ERROR, fmt, ##args)
#define metal_warn(fmt, args...) metal_log(METAL_LOG_WARNING, fmt, ##args)
#define metal_info(fmt, args...) metal_log(METAL_LOG_INFO, fmt, ##args)
#define metal_dbg(fmt, args...) metal_log(METAL_LOG_DEBUG, fmt, ##args)

#ifdef __cplusplus
}
#endif

#endif /* __METAL_METAL_LOG__H__ */

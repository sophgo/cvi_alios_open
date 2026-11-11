/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * AliOS adaptation: system initialization and logging.
 */

#ifndef METAL_INTERNAL
#define METAL_INTERNAL
#endif

#include <metal/sys.h>
#include <metal/log.h>
#include <metal/list.h>
#include <stdio.h>
#include <stdarg.h>

struct metal_state _metal;

void metal_default_log_handler(enum metal_log_level level,
                               const char *format, ...)
{
    va_list ap;
    const char *level_str[] = {
        "EMERGENCY", "ALERT", "CRITICAL", "ERROR",
        "WARNING", "NOTICE", "INFO", "DEBUG"
    };

    if (level > METAL_LOG_DEBUG)
        return;

    printf("[metal %s] ", level_str[level]);

    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);

    printf("\n");
}

void metal_set_log_handler(metal_log_handler handler)
{
    _metal.common.log_handler = handler;
}

metal_log_handler metal_get_log_handler(void)
{
    return _metal.common.log_handler;
}

void metal_set_log_level(enum metal_log_level level)
{
    _metal.common.log_level = level;
}

enum metal_log_level metal_get_log_level(void)
{
    return _metal.common.log_level;
}

int metal_sys_init(const struct metal_init_params *params)
{
    /* Initialize common state */
    metal_list_init(&_metal.common.bus_list);
    metal_list_init(&_metal.common.generic_shmem_list);
    metal_list_init(&_metal.common.generic_device_list);

    if (params) {
        _metal.common.log_handler = params->log_handler;
        _metal.common.log_level = params->log_level;
    }

    if (!_metal.common.log_handler)
        _metal.common.log_handler = metal_default_log_handler;

    return 0;
}

void metal_sys_finish(void)
{
    /* Nothing special to do for bare-metal */
}

int metal_init(const struct metal_init_params *params)
{
    int ret;

    _metal.common.ref_count++;
    if (_metal.common.ref_count > 1)
        return 0;

    ret = metal_sys_init(params);
    if (ret)
        return ret;

    return 0;
}

void metal_finish(void)
{
    if (_metal.common.ref_count == 0)
        return;

    _metal.common.ref_count--;
    if (_metal.common.ref_count == 0)
        metal_sys_finish();
}

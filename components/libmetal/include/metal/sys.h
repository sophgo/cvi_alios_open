/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __METAL_SYS__H__
#define __METAL_SYS__H__

#include <stdlib.h>

#include <metal/log.h>
#include <metal/list.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long metal_phys_addr_t;
typedef int metal_irq_t;

#define METAL_BAD_OFFSET   ((unsigned long)-1)
#define METAL_BAD_PHYS     ((metal_phys_addr_t)-1)
#define METAL_BAD_VA       ((void *)-1)
#define METAL_BAD_IRQ      ((metal_irq_t)-1)

struct metal_init_params {
    metal_log_handler       log_handler;
    enum metal_log_level    log_level;
};

struct metal_common_state {
    int                     ref_count;
    enum metal_log_level    log_level;
    metal_log_handler       log_handler;
    struct metal_list       bus_list;
    struct metal_list       generic_shmem_list;
    struct metal_list       generic_device_list;
};

struct metal_state;

#include <metal/system/alios/sys.h>

#ifndef METAL_INIT_DEFAULTS
#define METAL_INIT_DEFAULTS \
{ \
    .log_handler = metal_default_log_handler, \
    .log_level   = METAL_LOG_INFO, \
}
#endif

extern struct metal_state _metal;

int metal_init(const struct metal_init_params *params);
void metal_finish(void);

#ifdef METAL_INTERNAL

int metal_sys_init(const struct metal_init_params *params);
void metal_sys_finish(void);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __METAL_SYS__H__ */

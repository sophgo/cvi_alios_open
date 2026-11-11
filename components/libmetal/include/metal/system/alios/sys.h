/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * AliOS system definition for libmetal.
 */

#ifndef __METAL_SYS__H__
#error "Do not include this file directly, include <metal/sys.h> instead"
#endif

#ifndef __METAL_ALIOS_SYS__H__
#define __METAL_ALIOS_SYS__H__

#include <limits.h>
#include <metal/errno.h>
#include <metal/cpu.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef METAL_MAX_DEVICE_REGIONS
#define METAL_MAX_DEVICE_REGIONS 1
#endif

struct metal_state {
    struct metal_common_state common;
};

#define metal_yield() metal_cpu_yield()

#ifdef METAL_INTERNAL

void sys_irq_restore_enable(unsigned int flags);
unsigned int sys_irq_save_disable(void);
void sys_irq_enable(unsigned int vector);
void sys_irq_disable(unsigned int vector);

#endif /* METAL_INTERNAL */

#ifdef __cplusplus
}
#endif

#endif /* __METAL_ALIOS_SYS__H__ */

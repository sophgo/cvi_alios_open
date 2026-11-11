/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * AliOS adaptation: use aos_malloc/aos_free for memory allocation.
 */

#ifndef __METAL_ALLOC__H__
#error "Do not include this file directly, include <metal/alloc.h> instead"
#endif

#ifndef __METAL_ALIOS_ALLOC__H__
#define __METAL_ALIOS_ALLOC__H__

#include <aos/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline void *__metal_allocate_memory(unsigned int size)
{
    return aos_malloc(size);
}

static inline void __metal_free_memory(void *ptr)
{
    aos_free(ptr);
}

#ifdef __cplusplus
}
#endif

#endif /* __METAL_ALIOS_ALLOC__H__ */

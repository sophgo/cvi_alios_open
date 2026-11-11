/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __METAL_ALLOC__H__
#define __METAL_ALLOC__H__

#include <metal/system/alios/alloc.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline void *metal_allocate_memory(unsigned int size)
{
    return __metal_allocate_memory(size);
}

static inline void metal_free_memory(void *ptr)
{
    __metal_free_memory(ptr);
}

#ifdef __cplusplus
}
#endif

#endif /* __METAL_ALLOC__H__ */

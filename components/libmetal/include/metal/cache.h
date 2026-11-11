/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __METAL_CACHE__H__
#define __METAL_CACHE__H__

#include <metal/system/alios/cache.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline void metal_cache_flush(void *addr, unsigned int len)
{
    __metal_cache_flush(addr, len);
}

static inline void metal_cache_invalidate(void *addr, unsigned int len)
{
    __metal_cache_invalidate(addr, len);
}

#ifdef __cplusplus
}
#endif

#endif /* __METAL_CACHE__H__ */

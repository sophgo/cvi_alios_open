/*
 * Copyright (c) 2018, Linaro Limited. and Contributors. All rights reserved.
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * AliOS adaptation: cache flush/invalidate using hardware cache operations.
 */

#ifndef __METAL_CACHE__H__
#error "Do not include this file directly, include <metal/cache.h> instead"
#endif

#ifndef __METAL_ALIOS_CACHE__H__
#define __METAL_ALIOS_CACHE__H__

#ifdef __cplusplus
extern "C" {
#endif

void metal_machine_cache_flush(void *addr, unsigned int len);
void metal_machine_cache_invalidate(void *addr, unsigned int len);

static inline void __metal_cache_flush(void *addr, unsigned int len)
{
    metal_machine_cache_flush(addr, len);
}

static inline void __metal_cache_invalidate(void *addr, unsigned int len)
{
    metal_machine_cache_invalidate(addr, len);
}

#ifdef __cplusplus
}
#endif

#endif /* __METAL_ALIOS_CACHE__H__ */

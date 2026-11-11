/*
 * Copyright (c) 2018, Linaro Limited. and Contributors. All rights reserved.
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * AliOS adaptation: cache flush/invalidate using RISC-V cache instructions.
 */

#include <stddef.h>
#include <asm/cache.h>

/*
 * These functions are implemented using RISC-V fence instructions.
 * On cvitek RISC-V cores, we need proper dcache management for
 * shared memory coherency with the other core.
 *
 * For now, use simple fence.i / fence implementations.
 * The actual hardware-specific cache operations should use
 * the existing AliOS cache API when available.
 */

void metal_machine_cache_flush(void *addr, unsigned int len)
{
    if (addr == NULL && len == 0) {
        /* Flush entire dcache */
        //asm volatile("fence" ::: "memory");
    } else {
        /* Flush specific range */
        //asm volatile("fence" ::: "memory");
        flush_dcache_range((unsigned long)addr, len);
    }
}

void metal_machine_cache_invalidate(void *addr, unsigned int len)
{
    if (addr == NULL && len == 0) {
        /* Invalidate entire dcache */
        //asm volatile("fence.i" ::: "memory");
    } else {
        /* Invalidate specific range */
        //asm volatile("fence.i" ::: "memory");
        inv_dcache_range((unsigned long)addr, len);
    }
}

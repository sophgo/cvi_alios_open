/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * AliOS adaptation: spinlock uses atomic_flag directly (no OS dependency).
 */

#ifndef __METAL_SPINLOCK__H__
#define __METAL_SPINLOCK__H__

#include <metal/atomic.h>
#include <metal/cpu.h>

#ifdef __cplusplus
extern "C" {
#endif

struct metal_spinlock {
    atomic_flag v;
};

#define METAL_SPINLOCK_INIT     {ATOMIC_FLAG_INIT}

static inline void metal_spinlock_init(struct metal_spinlock *slock)
{
    atomic_flag_clear(&slock->v);
}

static inline void metal_spinlock_acquire(struct metal_spinlock *slock)
{
    while (atomic_flag_test_and_set(&slock->v)) {
        metal_cpu_yield();
    }
}

static inline void metal_spinlock_release(struct metal_spinlock *slock)
{
    atomic_flag_clear(&slock->v);
}

#ifdef __cplusplus
}
#endif

#endif /* __METAL_SPINLOCK__H__ */

/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * AliOS adaptation: mutex backed by aos_mutex_t (a real blocking mutex).
 *
 * IMPORTANT: the previous implementation used an atomic_int busy-spin
 * (while (!cas_weak) ;).  On a single-core preemptive RTOS that is a
 * deadlock hazard: if the lock holder is preempted or yields (e.g.
 * rpmsg_send waiting for a TX buffer calls metal_sleep_usec -> aos_msleep
 * while holding rdev->lock), any other task that tries to acquire the same
 * lock busy-spins and the holder never gets the CPU again.  Using aos_mutex
 * (which blocks/yields) fixes that.
 */

#ifndef __METAL_MUTEX__H__
#define __METAL_MUTEX__H__

#include <metal/atomic.h>
#include <aos/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    aos_mutex_t m;
} metal_mutex_t;

#define METAL_MUTEX_UNLOCKED 0
#define METAL_MUTEX_LOCKED   1

/* Zero-initialized; aos_mutex_new() is invoked by metal_mutex_init() before
 * any use (open_amp calls metal_mutex_init for every lock it creates). */
#define METAL_MUTEX_INIT(m) { 0 }
#define METAL_MUTEX_DEFINE(m) metal_mutex_t m = METAL_MUTEX_INIT(m)

static inline void metal_mutex_init(metal_mutex_t *mutex)
{
    aos_mutex_new(&mutex->m);
}

static inline void metal_mutex_deinit(metal_mutex_t *mutex)
{
    aos_mutex_free(&mutex->m);
}

static inline int metal_mutex_try_acquire(metal_mutex_t *mutex)
{
    return (aos_mutex_lock(&mutex->m, AOS_NO_WAIT) == 0) ? 1 : 0;
}

static inline void metal_mutex_acquire(metal_mutex_t *mutex)
{
    (void)aos_mutex_lock(&mutex->m, AOS_WAIT_FOREVER);
}

static inline void metal_mutex_release(metal_mutex_t *mutex)
{
    aos_mutex_unlock(&mutex->m);
}

static inline int metal_mutex_is_acquired(metal_mutex_t *mutex)
{
    /* Not meaningfully queryable for an aos_mutex; unused by open_amp. */
    (void)mutex;
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* __METAL_MUTEX__H__ */

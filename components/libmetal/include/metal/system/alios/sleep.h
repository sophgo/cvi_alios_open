/*
 * Copyright (c) 2018, Linaro Limited. and Contributors. All rights reserved.
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * AliOS adaptation: sleep using aos_msleep.
 */

#ifndef __METAL_SLEEP__H__
#error "Do not include this file directly, include <metal/sleep.h> instead"
#endif

#ifndef __METAL_ALIOS_SLEEP__H__
#define __METAL_ALIOS_SLEEP__H__

#include <aos/kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline int __metal_sleep_usec(unsigned int usec)
{
    aos_msleep(usec / 1000);
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif /* __METAL_ALIOS_SLEEP__H__ */

/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __METAL_SLEEP__H__
#define __METAL_SLEEP__H__

#include <metal/system/alios/sleep.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline int metal_sleep_usec(unsigned int usec)
{
    return __metal_sleep_usec(usec);
}

#ifdef __cplusplus
}
#endif

#endif /* __METAL_SLEEP__H__ */

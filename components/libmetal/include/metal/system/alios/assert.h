/*
 * Copyright (c) 2018, Xilinx Inc. and Contributors. All rights reserved.
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * AliOS adaptation: use assert() from the standard library.
 */

#ifndef __METAL_ASSERT__H__
#error "Do not include this file directly, include <metal/assert.h> instead"
#endif

#ifndef __METAL_ALIOS_ASSERT__H__
#define __METAL_ALIOS_ASSERT__H__

#include <assert.h>

#define metal_sys_assert(cond) assert(cond)

#endif /* __METAL_ALIOS_ASSERT__H__ */

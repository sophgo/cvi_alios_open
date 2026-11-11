/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __METAL_GCC_COMPILER__H__
#define __METAL_GCC_COMPILER__H__

#ifdef __cplusplus
extern "C" {
#endif

#define restrict __restrict__
#define metal_align(n) __attribute__((aligned(n)))
#define metal_weak __attribute__((weak))

#if defined(__STRICT_ANSI__)
#define metal_asm __asm__
#else
#define metal_asm asm
#endif

#define METAL_PACKED_BEGIN
#define METAL_PACKED_END __attribute__((__packed__))

#ifndef __deprecated
#define __deprecated    __attribute__((deprecated))
#endif

#ifdef __cplusplus
}
#endif

#endif /* __METAL_GCC_COMPILER__H__ */

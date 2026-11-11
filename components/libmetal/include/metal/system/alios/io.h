/*
 * Copyright (c) 2017, Xilinx Inc. and Contributors. All rights reserved.
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * AliOS adaptation: I/O region for bare-metal (no MMU, phys=virt).
 */

#ifndef __METAL_IO__H__
#error "Do not include this file directly, include <metal/io.h> instead"
#endif

#ifndef __METAL_ALIOS_IO__H__
#define __METAL_ALIOS_IO__H__

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef METAL_INTERNAL

void metal_sys_io_mem_map(struct metal_io_region *io);
void *metal_machine_io_mem_map(void *va, metal_phys_addr_t pa,
                               size_t size, unsigned int flags);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __METAL_ALIOS_IO__H__ */

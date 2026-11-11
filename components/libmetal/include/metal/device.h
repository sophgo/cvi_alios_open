/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __METAL_DEVICE__H__
#define __METAL_DEVICE__H__

#include <metal/io.h>
#include <metal/list.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef METAL_MAX_DEVICE_REGIONS
#define METAL_MAX_DEVICE_REGIONS 1
#endif

struct metal_device {
    struct metal_list node;
    const char *name;
    metal_phys_addr_t regions[METAL_MAX_DEVICE_REGIONS];
    struct metal_io_region regions_io[METAL_MAX_DEVICE_REGIONS];
    int num_regions;
};

#ifdef __cplusplus
}
#endif

#endif /* __METAL_DEVICE__H__ */

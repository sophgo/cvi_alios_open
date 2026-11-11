/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __METAL_SHMEM__H__
#define __METAL_SHMEM__H__

#include <metal/io.h>
#include <metal/list.h>

#ifdef __cplusplus
extern "C" {
#endif

struct metal_shmem {
    struct metal_list node;
    struct metal_io_region io;
};

#ifdef __cplusplus
}
#endif

#endif /* __METAL_SHMEM__H__ */

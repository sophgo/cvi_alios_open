/*
 * Copyright (c) 2015, Xilinx Inc. and Contributors. All rights reserved.
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __METAL_IRQ__H__
#define __METAL_IRQ__H__

#include <metal/sys.h>

#ifdef __cplusplus
extern "C" {
#endif

int metal_irq_register(metal_irq_t irq,
                       void (*handler)(metal_irq_t irq,
                                       void *dev_id),
                       void *dev_id,
                       struct metal_device *dev);
void metal_irq_unregister(metal_irq_t irq,
                          void (*handler)(metal_irq_t irq,
                                         void *dev_id),
                          void *dev_id,
                          struct metal_device *dev);

#ifdef __cplusplus
}
#endif

#endif /* __METAL_IRQ__H__ */

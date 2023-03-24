/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */

#ifndef DEVICE_INPUT_IMPL_H
#define DEVICE_INPUT_IMPL_H

#include <stdio.h>
#include <stdlib.h>

#include <devices/driver.h>
#include <devices/device.h>
#include <devices/input.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief input dirver ops */
typedef struct input_driver {
    driver_t drv;
    /** set event callback */
    int (*set_event)(rvm_dev_t *dev, rvm_hal_input_event event_cb, void *priv);
    /** read input data */
    int (*read)(rvm_dev_t *dev, void *data, uint32_t size, unsigned int timeout_ms);
} input_driver_t;

#ifdef __cplusplus
}
#endif

#endif

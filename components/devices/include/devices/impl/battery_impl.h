/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */

#ifndef HAL_BATTERY_H
#define HAL_BATTERY_H

#include <stdint.h>

#include <devices/driver.h>
#include <devices/battery.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef struct battery_driver {
    driver_t    drv;
    int         (*fetch)(rvm_dev_t *dev, rvm_hal_battery_attr_t attr);
    int         (*getvalue)(rvm_dev_t *dev, rvm_hal_battery_attr_t attr, void *value, size_t size);
    int         (*event_cb)(rvm_dev_t *dev, rvm_hal_battery_event_t event);
} battery_driver_t;

#ifdef __cplusplus
}
#endif

#endif

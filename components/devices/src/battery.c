/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dev_internal.h>
#include <devices/impl/battery_impl.h>

#define BATTERY_DRIVER(dev)  ((battery_driver_t*)(dev->drv))
#define BATTERY_VAILD(dev) do { \
    if (device_valid(dev, "battery") != 0) \
        return -1; \
} while(0)

int rvm_hal_battery_fetch(rvm_dev_t *dev, rvm_hal_battery_attr_t attr)
{
    int ret;

    BATTERY_VAILD(dev);

    device_lock(dev);
    ret = BATTERY_DRIVER(dev)->fetch(dev, attr);
    device_unlock(dev);

    return ret < 0 ? -EIO : 0;
}

int rvm_hal_battery_getvalue(rvm_dev_t *dev, rvm_hal_battery_attr_t attr, void *value, size_t size)
{
    int ret;

    BATTERY_VAILD(dev);

    device_lock(dev);
    ret = BATTERY_DRIVER(dev)->getvalue(dev, attr, value, size);
    device_unlock(dev);

    return ret < 0 ? -EIO : 0;
}

int rvm_hal_battery_event_cb(rvm_hal_battery_event_t event)
{
    if (event == REMOVED) {
        //TODO

    }

    return 0;
}

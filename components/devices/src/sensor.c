/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <dev_internal.h>
#include <devices/impl/sensor_impl.h>

#define SENSOR_DRIVER(dev)  ((sensor_driver_t*)(dev->drv))
#define SENSOR_VAILD(dev) do { \
    if (device_valid(dev, "sensor") != 0) \
        return -1; \
} while(0)

int rvm_hal_sensor_fetch(rvm_dev_t *dev)
{
    int ret;

    SENSOR_VAILD(dev);

    device_lock(dev);
    ret = SENSOR_DRIVER(dev)->fetch(dev);
    device_unlock(dev);

    return ret < 0 ? -EIO : 0;
}

int rvm_hal_sensor_getvalue(rvm_dev_t *dev, void *value, size_t size)
{
    int ret;

    SENSOR_VAILD(dev);

    device_lock(dev);
    ret = SENSOR_DRIVER(dev)->getvalue(dev, value, size);
    device_unlock(dev);

    return ret < 0 ? -EIO : 0;
}

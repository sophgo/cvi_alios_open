/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <devices/hal/iic_impl.h>

#define IIC_DRIVER(dev)  ((iic_driver_t*)(dev->drv))
#define IIC_VAILD(dev) do { \
    if (device_valid(dev, "iic") != 0) \
        return -1; \
} while(0)

void iic_config_default(iic_config_t *config)
{
    config->mode      = MODE_MASTER;
    config->speed     = BUS_SPEED_STANDARD;
    config->addr_mode = ADDR_7BIT;
}

int iic_config(aos_dev_t *dev, iic_config_t *config)
{
    int ret;

    IIC_VAILD(dev);

    device_lock(dev);
    ret = IIC_DRIVER(dev)->config(dev, config);
    device_unlock(dev);

    return ret;
}

int iic_master_send(aos_dev_t *dev, uint16_t dev_addr, const void *data, uint32_t size, uint32_t timeout)
{
    if (size == 0 || NULL == data) {
        return -EINVAL;
    }

    int ret;

    IIC_VAILD(dev);

    device_lock(dev);
    ret = IIC_DRIVER(dev)->master_send(dev, dev_addr, data, size, timeout);
    device_unlock(dev);

    return ret;
}

int iic_master_recv(aos_dev_t *dev, uint16_t dev_addr, void *data, uint32_t size, uint32_t timeout)
{
    int ret;

    IIC_VAILD(dev);

    device_lock(dev);
    ret = IIC_DRIVER(dev)->master_recv(dev, dev_addr, data, size, timeout);
    device_unlock(dev);

    return ret;
}

int iic_slave_send(aos_dev_t *dev, const void *data, uint32_t size, uint32_t timeout)
{
    int ret;

    IIC_VAILD(dev);

    device_lock(dev);
    ret = IIC_DRIVER(dev)->slave_send(dev, data, size, timeout);
    device_unlock(dev);

    return ret;
}

int iic_slave_recv(aos_dev_t *dev, void *data, uint32_t size, uint32_t timeout)
{
    int ret;

    IIC_VAILD(dev);

    device_lock(dev);
    ret = IIC_DRIVER(dev)->slave_recv(dev, data, size, timeout);
    device_unlock(dev);

    return ret;
}

int iic_mem_write(aos_dev_t *dev, uint16_t dev_addr, uint16_t mem_addr, uint16_t mem_addr_size,\
                  const void *data, uint32_t size, uint32_t timeout)
{
    int ret;

    IIC_VAILD(dev);

    device_lock(dev);
    ret = IIC_DRIVER(dev)->mem_write(dev, dev_addr, mem_addr, mem_addr_size, data, size, timeout);
    device_unlock(dev);

    return ret;
}

int iic_mem_read(aos_dev_t *dev, uint16_t dev_addr, uint16_t mem_addr, uint16_t mem_addr_size,\
                 void *data, uint32_t size, uint32_t timeout)
{
    int ret;

    IIC_VAILD(dev);

    device_lock(dev);
    ret = IIC_DRIVER(dev)->mem_read(dev, dev_addr, mem_addr, mem_addr_size, data, size, timeout);
    device_unlock(dev);

    return ret;
}
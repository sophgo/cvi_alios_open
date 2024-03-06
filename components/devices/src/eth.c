/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */

#if defined(CONFIG_SAL) || defined(CONFIG_TCPIP)
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dev_internal.h>
#include <devices/ethernet.h>
#include <devices/impl/ethernet_impl.h>
#include <devices/impl/net_impl.h>

#define ETH_DRIVER(dev)  ((eth_driver_t *)(((netdev_driver_t *)dev->drv)->link_ops))
#define ETH_VALID(dev) do { \
        if (device_valid(dev, "eth") != 0) \
            return -1; \
    } while(0)

int rvm_hal_eth_reset(rvm_dev_t *dev)
{
    int ret;

    ETH_VALID(dev);

    device_lock(dev);
    ret = ETH_DRIVER(dev)->reset(dev);
    device_unlock(dev);

    return ret;
}

int rvm_hal_eth_start(rvm_dev_t *dev)
{
    int ret;

    ETH_VALID(dev);

    device_lock(dev);
    ret = ETH_DRIVER(dev)->start(dev);
    device_unlock(dev);

    return ret;
}

int rvm_hal_eth_stop(rvm_dev_t *dev)
{
    int ret;

    ETH_VALID(dev);

    device_lock(dev);
    ret = ETH_DRIVER(dev)->reset(dev);
    device_unlock(dev);

    return ret;
}

int rvm_hal_eth_mac_control(rvm_dev_t *dev, eth_config_t *config)
{
    int ret;

    ETH_VALID(dev);

    device_lock(dev);
    ret = ETH_DRIVER(dev)->mac_control(dev, config);
    device_unlock(dev);

    return ret;
}

int rvm_hal_eth_set_packet_filter(rvm_dev_t *dev, int type)
{
    int ret;

    ETH_VALID(dev);

    device_lock(dev);
    ret = ETH_DRIVER(dev)->set_packet_filter(dev, type);
    device_unlock(dev);

    return ret;
}

#endif


/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lwip/apps/dhcps.h>
#include <lwip/netifapi.h>
#include <lwip/dns.h>

#include <devices/netdrv.h>
#include <devices/ethernet.h>
#include <devices/impl/net_impl.h>
#include <devices/impl/ethernet_impl.h>


#if !defined(MIN)
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define TAG "esp32_drv"

typedef struct {
    rvm_dev_t device;

    void *priv;
} esp32_dev_t;

extern struct netif rndis_netif;

static rvm_dev_t *esp32_dev_init(driver_t *drv, void *config, int id)
{
    printf("Entry %s \n", __func__);
    rvm_dev_t *dev = rvm_hal_device_new(drv, sizeof(esp32_dev_t), id);

    return dev;
}

#define esp32_dev_uninit rvm_hal_device_free

static int esp32_dev_open(rvm_dev_t *dev)
{
    printf("Entry %s \n", __func__);
    return 0;
}

static int esp32_dev_close(rvm_dev_t *dev)
{
    printf("Entry %s \n", __func__);
    return 0;
}

/*****************************************
 * common netif driver interface
 ******************************************/
static int esp32_set_mac_addr(rvm_dev_t *dev, const uint8_t *mac)
{
    LOGE(TAG, "%s not support", __func__);
    return 0;
}

static int esp32_get_mac_addr(rvm_dev_t *dev, uint8_t *mac)
{
    printf("Entry %s \n", __func__);
    mac[0] = 0xf4;
    mac[1] = 0x12;
    mac[2] = 0xfa;
    mac[3] = 0xe5;
    mac[4] = 0x23;
    mac[5] = 0xf0;
    return 0;
}

static void net_status_callback(struct netif *netif)
{
    printf("Entry %s \n", __func__);
    if (!ip_addr_isany(ip_2_ip4(&netif->ip_addr))) {
        event_publish(EVENT_NET_GOT_IP, NULL);
    }
}

static int esp32_start_dhcp(rvm_dev_t *dev)
{
    printf("Entry %s\n", __func__);
    struct netif *netif = &rndis_netif;
    aos_check_return_einval(netif);

    if (!netif_is_link_up(netif)) {
        printf("netif_is_link_up failed \n");
        return -1;
    }

    netif_set_ipaddr(netif, NULL);
    netif_set_netmask(netif, NULL);
    netif_set_gw(netif, NULL);

    netif_set_status_callback(netif, net_status_callback);

    return netifapi_dhcp_start(netif);
}

static int esp32_stop_dhcp(rvm_dev_t *dev)
{
    printf("Entry %s \n", __func__);
    struct netif *netif = &rndis_netif;
    aos_check_return_einval(netif);

    netifapi_dhcp_stop(netif);

    return 0;
}

static int esp32_set_ipaddr(rvm_dev_t *dev, const ip_addr_t *ipaddr, const ip_addr_t *netmask, const ip_addr_t *gw)
{
    printf("Entry %s \n", __func__);
    return -1;
}

static int esp32_get_ipaddr(rvm_dev_t *dev, ip_addr_t *ipaddr, ip_addr_t *netmask_addr, ip_addr_t *gw_addr)
{
    printf("Entry %s \n", __func__);
    struct netif *netif = &rndis_netif;
    aos_check_return_einval(netif && ipaddr && netmask_addr && gw_addr);

    ip_addr_copy(*(ip4_addr_t *)ip_2_ip4(ipaddr), *netif_ip_addr4(netif));
    ip_addr_copy(*(ip4_addr_t *)ip_2_ip4(gw_addr), *netif_ip_gw4(netif));
    ip_addr_copy(*(ip4_addr_t *)ip_2_ip4(netmask_addr), *netif_ip_netmask4(netif));

    return 0;
}

static int esp32_subscribe(rvm_dev_t *dev, uint32_t event, event_callback_t cb, void *param)
{
    printf("Entry %s \n", __func__);
    if (cb) {
        event_subscribe(event, cb, param);
    }

    return 0;
}

int esp32_set_dns_server(rvm_dev_t *dev, ip_addr_t ipaddr[], uint32_t num)
{
    LOGE(TAG, "%s not support", __func__);
    return 0;
}

static int esp32_get_dns_server(rvm_dev_t *dev, ip_addr_t ipaddr[], uint32_t num)
{
    LOGE(TAG, "%s not support", __func__);
    return 0;
}

static int esp32_set_hostname(rvm_dev_t *dev, const char *name)
{
    LOGE(TAG, "%s not support", __func__);
    return 0;
}

static const char *esp32_get_hostname(rvm_dev_t *dev)
{
    LOGE(TAG, "%s not support", __func__);
    return NULL;
}

/*****************************************
 * esp32 driver interface
 ******************************************/

static int esp32_mac_control(rvm_dev_t *dev, eth_config_t *config)
{
    printf("Entry %s \n", __func__);
    return 0;
}

static int esp32_set_packet_filter(rvm_dev_t *dev, int type)
{
    printf("Entry %s \n", __func__);
    return 0;
}

static int esp32_start(rvm_dev_t *dev)
{
    printf("Entry %s \n", __func__);
    return 0;
}

static int esp32_stop(rvm_dev_t *dev)
{
    printf("Entry %s \n", __func__);
    return 0;
}

static int esp32_reset(rvm_dev_t *dev)
{
    printf("Entry %s \n", __func__);
    return 0;
}

static net_ops_t esp32_net_driver = {
    .get_mac_addr   = esp32_get_mac_addr,
    .set_mac_addr   = esp32_set_mac_addr,
    .set_dns_server = esp32_set_dns_server,
    .get_dns_server = esp32_get_dns_server,
    .set_hostname   = esp32_set_hostname,
    .get_hostname   = esp32_get_hostname,
    .start_dhcp     = esp32_start_dhcp,
    .stop_dhcp      = esp32_stop_dhcp,
    .set_ipaddr     = esp32_set_ipaddr,
    .get_ipaddr     = esp32_get_ipaddr,
    .subscribe      = esp32_subscribe,
    .ping           = NULL,
};

static eth_driver_t eth_driver = {
    .mac_control       = esp32_mac_control,
    .set_packet_filter = esp32_set_packet_filter,
    .start             = esp32_start,
    .stop              = esp32_stop,
    .reset             = esp32_reset,
};

static netdev_driver_t neteth_driver = {
    .drv = {
        .name   = "eth",
        .init   = esp32_dev_init,
        .uninit = esp32_dev_uninit,
        .open   = esp32_dev_open,
        .close  = esp32_dev_close,
    },
    .link_type = NETDEV_TYPE_ETH,
    .net_ops =  &esp32_net_driver,
    .link_ops = &eth_driver,
};

extern void esp32_rndis_lwip_init(void);
extern void esp32_rndis_net_init(void);

void drv_esp32_rndis_register()
{
    int ret;

    esp32_rndis_lwip_init();

    ret = rvm_driver_register(&neteth_driver.drv, NULL, 0);

    if (ret < 0) {
        LOGI(TAG, "esp32 eth device register error");
    }

    esp32_rndis_net_init();
}
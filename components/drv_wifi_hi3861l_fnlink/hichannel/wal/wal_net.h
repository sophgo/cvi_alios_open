/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: Header file for oal_util.h.
 * Author: Hisilicon
 * Create: 2018-08-04
 */

#ifndef __WAL_NET_H__
#define __WAL_NET_H__

//#include <linux/netdevice.h>
//#include "oal_net.h"
#include <lwip/pbuf.h>
#include "oal_netbuf.h"
//#include "oam_ext_if.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* �궨�� */
#define ETHER_ADDR_LEN          6
#define WLAN_MAC_ADDR_LEN       6
#define ether_is_multicast(_a) (*(_a) & 0x01)
hi_u32 hi_channel_send_data_to_dev(hi_u8 *buf, hi_u32 len, struct pbuf *p);
hi_u32 hi_channel_send_msg_to_dev(hi_u8 *buf, hi_u32 len);
int wal_irq_config(void);


#if 0
/* inline �������� */
static inline oal_net_device_stru *wal_net_alloc_netdev(hi_u32 sizeof_priv, const hi_char *netdev_name, hi_void *set_up)
{
    oal_net_device_stru *tmp_netdev = HI_NULL;

    if ((netdev_name == HI_NULL) || (set_up == HI_NULL)) {
        return HI_NULL;
    }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
    tmp_netdev = alloc_netdev(sizeof_priv, netdev_name, NET_NAME_UNKNOWN, set_up);
#else
    tmp_netdev = alloc_netdev(sizeof_priv, netdev_name, set_up);
#endif
    if (tmp_netdev == HI_NULL) {
        oam_error_log0("oal_net_alloc_netdev:: netdev is NULL");
    }
    return tmp_netdev;
}

static inline hi_void wal_net_free_netdev(oal_net_device_stru *netdev)
{
    if (netdev == HI_NULL) {
        return;
    }
    free_netdev(netdev);
}

/* �������� */
hi_s32 netdev_register(hi_void);
hi_void netdev_unregister(hi_void);
struct pbuf * wal_rx_data_proc(oal_netbuf_stru *netbuf);
#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __WAL_NET_H__ */

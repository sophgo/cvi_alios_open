/*
 * Copyright (C) 2017-2019 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     test_eth_phy_loopback.c
 * @brief    ethernet phy loopback test case
 * @version  V1.0
 * @date     21 March 2019
 ******************************************************************************/

// // #include <csi_config.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <drv_errno.h>
#include "drv_eth.h"
#include "drv_eth_phy.h"
#include "drv_eth_mac.h"
#include "phy.h"
#include "aos/kernel.h"

#define LOG_LEVEL 0
#include <syslog.h>

#define PHY_OFFLINE 0
#define PHY_ONLINE 1

static eth_mac_handle_t g_mac_handle;
static eth_phy_handle_t g_phy_handle;
uint8_t g_phy_online_stat = PHY_OFFLINE;
uint8_t g_phy_init_finish = 0;

uint8_t g_mac_addr[6] = {0x00, 0xD8, 0x61, 0xc5, 0x35, 0x19};
/*
#define CSI_ETH_MAC_EVENT_RX_FRAME      (1UL << 0)  ///< Frame Received
#define CSI_ETH_MAC_EVENT_TX_FRAME      (1UL << 1)  ///< Frame Transmitted
#define CSI_ETH_MAC_EVENT_WAKEUP        (1UL << 2)  ///< Wake-up (on Magic Packet)
#define CSI_ETH_MAC_EVENT_TIMER_ALCSI   (1UL << 3)  ///< Timer Alarm
#define CSI_ETH_MAC_EVENT_LINK_CHANGE
*/
extern aos_sem_t eth_rx_sem;
static void eth_mac_signal_event(uint32_t idx, uint32_t event)
{
    if(idx == 0) {
        if(event & CSI_ETH_MAC_EVENT_RX_FRAME) {
            if(eth_rx_sem.hdl)
                aos_sem_signal(&eth_rx_sem);
        }
    }
}

static int32_t fn_phy_read(uint8_t phy_addr, uint8_t reg_addr, uint16_t *data)
{
    return csi_eth_mac_phy_read(g_mac_handle, phy_addr, reg_addr, data);
}

static int32_t fn_phy_write(uint8_t phy_addr, uint8_t reg_addr, uint16_t data)
{
    return csi_eth_mac_phy_write(g_mac_handle, phy_addr, reg_addr, data);
}

int32_t csi_eth_mac_start(void)
{
    int32_t ret;

    /* adjust mac link parameter */
    ret = csi_eth_mac_control(g_mac_handle, DRV_ETH_MAC_ADJUST_LINK, (uint32_t)g_phy_handle);

    if (ret != 0) {
        LOG_E("Failed to adjust link\n");
        return -1;
    }
    /* start TX */
    ret = csi_eth_mac_control(g_mac_handle, CSI_ETH_MAC_CONTROL_TX, 1);

    if (ret != 0) {
        LOG_E("Failed to start TX\n");
        return -1;
    }
    /* start RX */
    ret = csi_eth_mac_control(g_mac_handle, CSI_ETH_MAC_CONTROL_RX, 1);

    if (ret != 0) {
        LOG_E("Failed to start RX\n");
        return -1;
    }

    return 0;
}

int csi_eth_mac_phy_init(void)
{
    eth_mac_addr_t addr;
    int32_t ret;
    uint8_t mac[6] = {0};

    /* initialize MAC & PHY */
    g_mac_handle = csi_eth_mac_initialize(0, (eth_event_cb_t)eth_mac_signal_event);

    if (g_mac_handle == NULL) {
        LOG_E("Failed to init mac\n");
        return -1;
    }

    g_phy_handle = csi_eth_phy_initialize(fn_phy_read, fn_phy_write);

    if (g_phy_handle == NULL) {
        LOG_E("Failed to init phy\n");
        return -1;
    }

    /* set mac address */
    extern int get_net_mac_kv(uint8_t mac[6]);
    ret = get_net_mac_kv(mac);
    if (0 == ret) {
        memcpy(g_mac_addr, mac, 6);
    } 
    memcpy(addr.b, g_mac_addr, sizeof(g_mac_addr));
    ret = csi_eth_mac_set_macaddr(g_mac_handle, &addr);

    if (ret != 0) {
        LOG_E("Failed to set mac address\n");
        return -1;
    }

    /* Start up the PHY */
    ret = csi_eth_phy_power_control(g_phy_handle, CSI_ETH_POWER_FULL);
    g_phy_init_finish = 1;
    
    if (ret != 0) {
        LOG_E("Failed to start phy, state:0x%x\n", ret);
        return -1;
    }
    g_phy_online_stat = PHY_ONLINE;
    
    ret = csi_eth_mac_start();

    if (ret != 0) {
        LOG_E("Failed to start mac\n");
        return -1;
    }
    
    LOG_I("PHY MAC init done\n");
    return 0;
}
void csi_eth_phy_update_link_task(void *arg)
{
    int32_t ret =0;
    eth_phy_dev_t *dev = (eth_phy_dev_t *)g_phy_handle;
    
    while(1) {
        if(g_phy_init_finish == 1) {
            /* Start up the PHY */
            ret = csi_eth_phy_power_control(g_phy_handle, CSI_ETH_POWER_FULL);
            if ((ret != 0)||(dev->link_state == ETH_LINK_DOWN)) {
                g_phy_online_stat = PHY_OFFLINE;
            } else if((dev->link_state == ETH_LINK_UP) && (g_phy_online_stat == PHY_OFFLINE)) {                
                ret = csi_eth_mac_start();
                if(ret == 0) {
                    g_phy_online_stat = PHY_ONLINE;          
                }
            } 
        }

        aos_msleep(400);  
    }
}

void csi_send_packet(uint8_t *buf, uint32_t len, uint32_t flags)
{
    int32_t ret =0;

    if(buf == NULL) return;
    ret = csi_eth_mac_send_frame(g_mac_handle, buf, len, flags); 
    if(ret != 0) {
        LOG_E("csi_send_packet send failed\r\n");
    }
    
    return;
}
int32_t csi_receive_packet(uint8_t *buf, uint32_t len)
{
    int32_t ac_len = 0;

    if(buf == NULL) return -1;
    ac_len = csi_eth_mac_read_frame(g_mac_handle, buf, len);
    
    return ac_len;
}

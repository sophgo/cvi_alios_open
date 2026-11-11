/*
 * Copyright (C) 2017-2019 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     test_eth_phy_loopback.c
 * @brief    ethernet phy loopback test case
 * @version  V1.0
 * @date     21 March 2019
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <drv_errno.h>
#include "drv_eth.h"
#include "drv_eth_phy.h"
#include "drv_eth_mac.h"
#include "phy.h"
#include "aos/kernel.h"
//#define ETH_PHY_LOOKBACK

static eth_mac_handle_t g_test_mac_handle;
static eth_phy_handle_t g_test_phy_handle;

#define SLT_RTN_INFO_LEN	64
typedef struct slt_rtn
{
	int ret;
	char info[SLT_RTN_INFO_LEN];
}slt_rtn_t;

slt_rtn_t  g_gmac_rtn;

typedef enum {
    ERR_MAC_INIT = 1,
	ERR_PHY_INIT,
	ERR_MAC_SET,
	ERR_PHY_START,
	ERR_MAC_LINK,
	ERR_PHY_LOOPBACK,
	ERR_TX_START,
    ERR_RX_START,
    ERR_RX_DATE_ERROR,
    ERR_RX_ERROR,
	ERR_GMAC_UNINIT
}EM_GMAC_ERR;

static uint8_t g_test_mac_addr[6] = {0x00, 0x11, 0x22, 0x60, 0x94, 0x2d};

#define FRAME_DATA_LEN  80

static uint8_t g_frame_data[FRAME_DATA_LEN] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x11, 0x22, 0x60, 0x94, 0x2d, 0x08, 0x06,
    0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x01,
    0x00, 0x11, 0x22, 0x60, 0x94, 0x2d, 0xc0, 0xa8, 0x67, 0x9f,
    0x3b, 0xc8, 0x50, 0x10, 0xfa, 0xf0, 0xc0, 0xa8, 0x67, 0x73,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// static uint8_t g_received_data[FRAME_DATA_LEN];

static uint8_t g_tx_perf_test_data[1512] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x11, 0x22, 0x60, 0x94, 0x2d, 0x08, 0x06,
    0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x01,
    0x00, 0x11, 0x22, 0x60, 0x94, 0x2d, 0xc0, 0xa8, 0x67, 0x9f,
    0x3b, 0xc8, 0x50, 0x10, 0xfa, 0xf0, 0xc0, 0xa8, 0x67, 0x73,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// static bool g_loopback_supported = true;

static void eth_mac_signal_event(uint32_t idx, uint32_t event)
{
    /* printf("mac signal\n"); */
}

static int32_t test_fn_phy_read(uint8_t phy_addr, uint8_t reg_addr, uint16_t *data)
{
    return csi_eth_mac_phy_read(g_test_mac_handle, phy_addr, reg_addr, data);
}

static int32_t test_fn_phy_write(uint8_t phy_addr, uint8_t reg_addr, uint16_t data)
{
    return csi_eth_mac_phy_write(g_test_mac_handle, phy_addr, reg_addr, data);
}

// static void print_data(uint8_t *buf, int32_t length)
// {
//     int32_t i;

//     for (i = 0; i < length; i++) {
//         printf("0x%02X ", buf[i]);

//         if ((i + 1) % 16 == 0) {
//             printf("\n");
//         }
//     }

//     printf("\n");
// }

#define TX_PERF_TEST_CNT    20000
static void __eth_test_tx_perf(void)
{
    int32_t cost_us;
    int32_t cost_ms;
    int32_t total_len;
    volatile int i;

    struct timespec ts_begin, ts_end;
    uint32_t gettime_adjust;

    csi_eth_mac_send_frame(g_test_mac_handle,
                           g_frame_data, sizeof(g_frame_data), 0);
    aos_msleep(1000);
    printf("Testing TX bandwidth...\n");
    gettime_adjust = clock_gettime_adjust() / 1000;

    clock_gettime(CLOCK_MONOTONIC, &ts_begin);

    for (i = 0; i < TX_PERF_TEST_CNT; i++) {
        csi_eth_mac_send_frame(g_test_mac_handle,
                    g_tx_perf_test_data, sizeof(g_tx_perf_test_data), 0);
    }

    total_len = TX_PERF_TEST_CNT * sizeof(g_tx_perf_test_data);

    clock_gettime(CLOCK_MONOTONIC, &ts_end);

    cost_us = (ts_end.tv_sec * 1000000 + ts_end.tv_nsec / 1000) -
              (ts_begin.tv_sec * 1000000 + ts_begin.tv_nsec / 1000) -
              gettime_adjust / 1000;

    cost_ms = cost_us / 1000;
    if (cost_ms <= 0) {
        printf("Unbelievable, cost 0ms, stop test\n");
        return;
    }

    printf("send %d Bytes, cost %dus, speed: %dKB/s(= %d MB/s = %d Mbps)\n",
            total_len, cost_us, total_len/cost_ms,
            (total_len/cost_ms)/1000,
            ((total_len/cost_ms)/1000)*8);
}
void _eth_mac_phy_force_config(eth_phy_handle_t phy_handle)
{
    eth_phy_dev_t *phydev = (eth_phy_dev_t *)phy_handle;
    eth_phy_priv_t *phy_priv = phydev->priv;

    if(phydev->interface == PHY_IF_MODE_GMII) {
        phy_priv->link_info.speed = CSI_ETH_SPEED_1G;
    } else {
        phy_priv->link_info.speed = CSI_ETH_SPEED_100M;
    }
    phy_priv->link_info.duplex = CSI_ETH_DUPLEX_FULL;
    phydev->link_state = ETH_LINK_UP;

    return;
}
static int __eth_test_phy_loopback(void)
{
    eth_mac_addr_t addr;
    // int32_t length;
    int32_t ret;

    printf("PHY Loopback Test\n");

	memset(&g_gmac_rtn, 0, sizeof(g_gmac_rtn));

    /* initialize MAC & PHY */
    g_test_mac_handle = csi_eth_mac_initialize(0, (eth_event_cb_t)eth_mac_signal_event);
    if (g_test_mac_handle == NULL) {
        snprintf(g_gmac_rtn.info, SLT_RTN_INFO_LEN, "Failed to init mac.\r\n");
        g_gmac_rtn.ret = -ERR_MAC_INIT;
        goto ERR;
    }

    g_test_phy_handle = csi_eth_phy_initialize(test_fn_phy_read, test_fn_phy_write);
    if (g_test_phy_handle == NULL) {
        snprintf(g_gmac_rtn.info, SLT_RTN_INFO_LEN, "Failed to init phy.\r\n");
        g_gmac_rtn.ret = -ERR_PHY_INIT;
        goto ERR;
    }

    /* set mac address */
    memcpy(addr.b, g_test_mac_addr, sizeof(g_test_mac_addr));
    ret = csi_eth_mac_set_macaddr(g_test_mac_handle, &addr);
    if (ret != 0) {
        snprintf(g_gmac_rtn.info, SLT_RTN_INFO_LEN, "Failed to set mac address.\r\n");
        g_gmac_rtn.ret = -ERR_MAC_SET;
        goto ERR;
    }

#ifndef ETH_PHY_LOOKBACK
    /* Start up the PHY */
    ret = csi_eth_phy_power_control(g_test_phy_handle, CSI_ETH_POWER_FULL);
    if (ret != 0) {
        snprintf(g_gmac_rtn.info, SLT_RTN_INFO_LEN, "Failed to start phy.\r\n");
        g_gmac_rtn.ret = -ERR_PHY_START;
        goto ERR;
    }
#else
    /*force phy up and force mac config*/
    _eth_mac_phy_force_config((uint32_t)g_test_phy_handle);
#endif

    /* adjust mac link parameter */
    ret = csi_eth_mac_control(g_test_mac_handle, DRV_ETH_MAC_ADJUST_LINK, (uint32_t)g_test_phy_handle);
    if (ret != 0) {
        snprintf(g_gmac_rtn.info, SLT_RTN_INFO_LEN, "Failed to adjust link.\r\n");
        g_gmac_rtn.ret = -ERR_MAC_LINK;
        goto ERR;
    }

#ifdef ETH_PHY_LOOKBACK
    /* phy loopback */
    ret = csi_eth_phy_set_mode(g_test_phy_handle, CSI_ETH_PHY_LOOPBACK);
    if (ret != 0) {
        if (GET_ERROR_NUM(ret) == DRV_ERROR_UNSUPPORTED) {
            printf("loopback NOT supported by phy driver\n");
            g_loopback_supported = false;
        } else {
            snprintf(g_gmac_rtn.info, SLT_RTN_INFO_LEN, "Failed to set phy loopback.\r\n");
            g_gmac_rtn.ret = -ERR_PHY_LOOPBACK;
            goto ERR;
        }
    }
#endif

    /* start TX */
    ret = csi_eth_mac_control(g_test_mac_handle, CSI_ETH_MAC_CONTROL_TX, 1);
    if (ret != 0) {
        snprintf(g_gmac_rtn.info, SLT_RTN_INFO_LEN, "Failed to start TX\r\n");
        g_gmac_rtn.ret = -ERR_TX_START;
        goto ERR;
    }

    /* start RX */
    ret = csi_eth_mac_control(g_test_mac_handle, CSI_ETH_MAC_CONTROL_RX, 1);
    if (ret != 0) {
        snprintf(g_gmac_rtn.info, SLT_RTN_INFO_LEN, "Failed to start RX\r\n");
        g_gmac_rtn.ret = -ERR_RX_START;
        goto ERR;
    }

#ifndef ETH_PHY_LOOKBACK
    __eth_test_tx_perf();
#else
    /* send data */
    int32_t count = 0;
    csi_eth_mac_delete_vlanid(g_test_mac_handle);

    while (count++ < 5) {
        //printf("count: %d\n", count);
        csi_eth_mac_send_frame(g_test_mac_handle, g_frame_data, sizeof(g_frame_data), 0);
        /* waiting for interrupt done */
        memset(g_received_data,0,sizeof(g_received_data));
        aos_msleep(10);
        /* receive data */
        length = csi_eth_mac_read_frame(g_test_mac_handle, g_received_data, sizeof(g_received_data));
        if (length > 0) {
            if (g_loopback_supported) {
                if (memcmp(g_received_data, g_frame_data, FRAME_DATA_LEN) == 0) {
                    //printf("*** PHY Loopback Test OK ***\n");
                } else {
                    printf("received data (%d bytes):\n", length);
                    print_data(g_received_data, length);
                    ret = -ERR_RX_DATE_ERROR;
                }
            } else {
                printf("received data (%d bytes):\n", length);
                print_data(g_received_data, length);
            }
        } else {
            ret = -ERR_RX_ERROR;
        }

    }

    if(ret == -ERR_RX_DATE_ERROR) {
        snprintf(g_gmac_rtn.info, SLT_RTN_INFO_LEN, "phy loopback: eth receive packet date is wrong\r\n");
        g_gmac_rtn.ret = -ERR_RX_START;
    }

    if(ret == -ERR_RX_ERROR) {
        snprintf(g_gmac_rtn.info, SLT_RTN_INFO_LEN, "phy loopback: eth cannot receive packet\r\n");
        g_gmac_rtn.ret = -ERR_RX_ERROR;
    }
#endif

ERR:
    /* uninitialize MAC */
    ret = csi_eth_mac_uninitialize(g_test_mac_handle);
    if (ret != 0) {
        snprintf(g_gmac_rtn.info, SLT_RTN_INFO_LEN, "gmac uninit failed\r\n");
        g_gmac_rtn.ret = -ERR_GMAC_UNINIT;
        return g_gmac_rtn.ret;
    }

    return g_gmac_rtn.ret;
}

int test_gmac(void)
{
    int32_t ret;
    ret = __eth_test_phy_loopback();
    if(ret != 0) {
        printf("*****fail(%d).%s*****\r\n", g_gmac_rtn.ret, g_gmac_rtn.info);
    }
    return ret;
}
slt_rtn_t * test_gmac_slt(void)
{
    (void)__eth_test_phy_loopback();
    return &g_gmac_rtn;
}

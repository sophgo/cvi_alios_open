/*
 * Copyright (C) 2017-2019 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     marvell.c
 * @brief    CSI Source File for marvell phy Driver
 * @version  V1.0
 * @date     21 March 2019
 ******************************************************************************/

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <aos/kernel.h>

/* LOG_LEVEL: 0: Err; 1: Err&Warn; 2: Err&Warn&Info; 3: Err&Warn&Info&Debug */
#define LOG_LEVEL 0
#include <syslog.h>

#include <soc.h>
#include <drv_common.h>
#include <drv_irq.h>
#include <drv_eth_mac.h>
#include <drv_eth_phy.h>
#include "phy.h"

extern void udelay(uint32_t us);

/* 88E1xxx PHY Status Register */
#define M88E1XXX_PHY_STATUS	        17
#define M88E1XXX_PHYSTAT_SPEED      0xc000
#define M88E1XXX_PHYSTAT_GBIT       0x8000
#define M88E1XXX_PHYSTAT_100        0x4000
#define M88E1XXX_PHYSTAT_DUPLEX     0x2000
#define M88E1XXX_PHYSTAT_SPDDONE    0x0800  /* speed and duplex resolved */
#define M88E1XXX_PHYSTAT_LINK       0x0400

/*
 * 88E1111 PHY Registers
 */

/* 88E1111 Extended PHY Specific Control Register */
#define M88E1111_PHY_EXT_CTRL               20
#define M88E1111_RGMII_RX_ADD_DELAY         (1 << 7)
#define M88E1111_RGMII_TX_ADD_DELAY         (1 << 1)

/* 88E1111 Extended PHY Specific Status Register */
#define M88E1111_PHY_EXT_STATUS             27
#define M88E1111_HWCFG_MODE_MASK            0xf
#define M88E1111_HWCFG_MODE_COPPER_RGMII    0xb
#define M88E1111_HWCFG_MODE_FIBER_RGMII     0x3
#define M88E1111_HWCFG_FIBER_COPPER_AUTO    0x8000
#define M88E1111_HWCFG_FIBER_COPPER_RES     0x2000

static inline bool phy_if_mode_is_rgmii(phy_if_mode_t interface)
{
    return interface >= PHY_IF_MODE_RGMII && interface <= PHY_IF_MODE_RGMII_TXID;
}

/* Marvell 88E1111 */

/**
  \brief       Configure the 88E1111 before make it start up.
  \param[in]   handle  phy handle
  \return      error code
*/
int32_t m88e1111_config(eth_phy_handle_t handle)
{
    ETHPHY_NULL_PARAM_CHK(handle);

    eth_phy_dev_t *dev = (eth_phy_dev_t *)handle;
    eth_phy_priv_t *priv = dev->priv;
    phy_if_mode_t interface = dev->interface;
    uint8_t phy_addr = dev->phy_addr;
    int32_t ret;
    uint16_t data;

    if (phy_if_mode_is_rgmii(interface)) {

        ret = eth_phy_read(priv, phy_addr, M88E1111_PHY_EXT_CTRL, &data);

        if (ret != 0) {
            return ret;
        }

        if ((interface == PHY_IF_MODE_RGMII) ||
            (interface == PHY_IF_MODE_RGMII_ID)) {
            data |= (M88E1111_RGMII_RX_ADD_DELAY | M88E1111_RGMII_TX_ADD_DELAY);
        } else if (interface == PHY_IF_MODE_RGMII_RXID) {
            data &= ~M88E1111_RGMII_TX_ADD_DELAY;
            data |= M88E1111_RGMII_RX_ADD_DELAY;
        } else if (interface == PHY_IF_MODE_RGMII_TXID) {
            data &= ~M88E1111_RGMII_RX_ADD_DELAY;
            data |= M88E1111_RGMII_TX_ADD_DELAY;
        }

        ret = eth_phy_write(priv, phy_addr, M88E1111_PHY_EXT_CTRL, data);

        if (ret != 0) {
            return ret;
        }

        ret = eth_phy_read(priv, phy_addr, M88E1111_PHY_EXT_STATUS, &data);

        if (ret != 0) {
            return ret;
        }

        data &= ~(M88E1111_HWCFG_MODE_MASK);

        if (data & M88E1111_HWCFG_FIBER_COPPER_RES) {
            data |= M88E1111_HWCFG_MODE_FIBER_RGMII;
        } else {
            data |= M88E1111_HWCFG_MODE_COPPER_RGMII;
        }

        ret = eth_phy_write(priv, phy_addr, M88E1111_PHY_EXT_STATUS, data);

        if (ret != 0) {
            return ret;
        }
    }

    /* soft reset */
    ret = eth_phy_reset(handle);

    if (ret != 0) {
        return ret;
    }

    ret = generic_phy_config_aneg(dev);

    if (ret != 0) {
        LOG_E("config aneg failed\n");
        return ret;
    }

    return 0;
}

/**
  \brief       Parse 88E1xxx's speed and duplex from status register.
  \param[in]   dev  phy device pointer
  \return      error code
*/
static int32_t m88e1xxx_parse_status(eth_phy_dev_t *dev)
{
    ETHPHY_NULL_PARAM_CHK(dev);
    ETHPHY_NULL_PARAM_CHK(dev->priv);

    eth_phy_priv_t *priv = dev->priv;
    uint8_t phy_addr = dev->phy_addr;
    unsigned int speed;
    uint16_t mii_reg;
    int32_t ret;

    ret = eth_phy_read(priv, phy_addr, M88E1XXX_PHY_STATUS, &mii_reg);

    if (ret != 0) {
        return ret;
    }

    if ((mii_reg & M88E1XXX_PHYSTAT_LINK) &&
        !(mii_reg & M88E1XXX_PHYSTAT_SPDDONE)) {
        int i = 0;

        while (!(mii_reg & M88E1XXX_PHYSTAT_SPDDONE)) {
            if (i > PHY_ANEG_TIMEOUT) {
                printf("time out\n");
                dev->link_state = ETH_LINK_DOWN;
                return ERR_ETHPHY(DRV_ERROR_TIMEOUT);
            }

            i++;
            aos_msleep(1);

            ret = eth_phy_read(priv, phy_addr, M88E1XXX_PHY_STATUS, &mii_reg);

            if (ret != 0) {
                return ret;
            }
        }
    } else {
        if (mii_reg & M88E1XXX_PHYSTAT_LINK) {
            dev->link_state = ETH_LINK_UP;
        } else {
            dev->link_state = ETH_LINK_DOWN;
        }
    }

    if (mii_reg & M88E1XXX_PHYSTAT_DUPLEX) {
        priv->link_info.duplex = CSI_ETH_DUPLEX_FULL;
    } else {
        priv->link_info.duplex = CSI_ETH_DUPLEX_HALF;
    }

    speed = mii_reg & M88E1XXX_PHYSTAT_SPEED;

    switch (speed) {
        case M88E1XXX_PHYSTAT_GBIT:
            priv->link_info.speed = CSI_ETH_SPEED_1G;
            break;

        case M88E1XXX_PHYSTAT_100:
            priv->link_info.speed = CSI_ETH_SPEED_100M;
            break;

        default:
            priv->link_info.speed = CSI_ETH_SPEED_10M;
            break;
    }

    return 0;
}

/**
  \brief       Start up the 88E1111.
  \param[in]   handle  phy handle
  \return      error code
*/
int32_t m88e1111_start(eth_phy_handle_t handle)
{
    ETHPHY_NULL_PARAM_CHK(handle);

    eth_phy_dev_t *dev = (eth_phy_dev_t *)handle;
    int32_t ret;

    ret = generic_phy_update_link(dev);

    if (ret) {
        return ret;
    }

    return m88e1xxx_parse_status(dev);
}

/**
  \brief       Halt the 88E1111.
  \param[in]   handle  phy handle
  \return      error code
*/
int32_t m88e1111_stop(eth_phy_handle_t handle)
{
    return 0;
}

/**
  \brief       Enable the 88E1111's loopback mode.
  \param[in]   handle  phy handle
  \return      error code
*/
int32_t m88e1111_loopback_line(eth_phy_handle_t handle)
{
    ETHPHY_NULL_PARAM_CHK(handle);
    eth_phy_dev_t *dev = (eth_phy_dev_t *)handle;
    ETHPHY_NULL_PARAM_CHK(dev->priv);

    eth_phy_priv_t *priv = dev->priv;
    uint8_t phy_addr = dev->phy_addr;

    uint16_t data;
    eth_phy_read(priv, phy_addr, M88E1111_PHY_EXT_CTRL, &data);
    data |= (0x7 << 4); /* 1000Mbps */    
    data |= (0x1 << 14); /* line loopback */
    eth_phy_write(priv, phy_addr, M88E1111_PHY_EXT_CTRL, data);

    eth_phy_reset(handle);
    /* waiting for loopback mode done */
    aos_msleep(3000);

    return 0;
}

int32_t m88e1111_loopback(eth_phy_handle_t handle)
{
    ETHPHY_NULL_PARAM_CHK(handle);
    eth_phy_dev_t *dev = (eth_phy_dev_t *)handle;
    ETHPHY_NULL_PARAM_CHK(dev->priv);

    eth_phy_priv_t *priv = dev->priv;
    uint8_t phy_addr = dev->phy_addr;
    int32_t ret;    
    uint16_t date =0xdead;

    if (dev->interface != PHY_IF_MODE_GMII) {
        LOG_I("*** m88e1111: internal loopback only supported for GMII ***\n");
        return ERR_ETHPHY(DRV_ERROR_UNSUPPORTED);
    }
#if 0
    uint16_t data;
    eth_phy_read(priv, phy_addr, M88E1111_PHY_EXT_CTRL, &data);
    data &= ~(0x7 << 4);
    data |= (0x5 << 4); /* 100Mbps */
    eth_phy_write(priv, phy_addr, M88E1111_PHY_EXT_CTRL, data);

    eth_phy_reset(handle);
#endif
    ret = eth_phy_read(priv, phy_addr, MII_BMCR, &date);
    date |= BMCR_LOOPBACK;
    /* enable the 88E1111's MAC Interface Loopback mode */
    ret = eth_phy_write(priv, phy_addr, MII_BMCR, date);

    if (ret != 0) {
        LOG_E("set loopback failed\n");
        return ret;
    }
    /* waiting for loopback mode done */
    aos_msleep(1000);

    return 0;
}

/**
  \brief       Update the 88E1111's link state.
  \param[in]   handle  phy handle
  \return      error code
*/
int32_t m88e1111_update_link(eth_phy_handle_t handle)
{
    ETHPHY_NULL_PARAM_CHK(handle);
    eth_phy_dev_t *dev = (eth_phy_dev_t *)handle;
    return m88e1xxx_parse_status(dev);;
}

eth_phy_dev_t M88E1111_device = {
    .phy_id = 0x01410cc0,
    .mask = 0xffffff0,
    .features = PHY_GBIT_FEATURES,
    .config = &m88e1111_config,
    .start = &m88e1111_start,
    .stop = &m88e1111_stop,
    .loopback = &m88e1111_loopback,
    .update_link = &m88e1111_update_link,
};

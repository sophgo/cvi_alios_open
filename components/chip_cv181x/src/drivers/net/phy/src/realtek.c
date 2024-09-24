#include <stdio.h>
#include <aos/kernel.h>
#include <assert.h>
#include <string.h>
#include <soc.h>
#include <drv_common.h>
#include <drv_irq.h>
#include <drv_eth_mac.h>
#include <drv_eth_phy.h>
#include "phy.h"
#define LOG_LEVEL 0
#include <syslog.h>

static int32_t rtl8201_config_init(eth_phy_handle_t handle)
{
    ETHPHY_NULL_PARAM_CHK(handle);

    eth_phy_dev_t *dev = (eth_phy_dev_t *)handle;
    phy_if_mode_t interface = dev->interface;
    int32_t ret;

    if(interface != PHY_IF_MODE_MII) {
        printf("rtl8201 and MAC not support mode %d\r\n",interface);
        return -1;
    }
    /* soft reset */
    ret = eth_phy_reset(handle);

    if (ret != 0) {
        return ret;
    }

    ret = generic_phy_config_aneg(dev);

    if (ret != 0) {
        LOG_I("config aneg failed\n");
        return ret;
    }

    ret = generic_phy_restart_aneg(dev);

    if (ret != 0) {
        LOG_I("restart aneg failed\n");
        return ret;
    }

    return 0;
}
static int32_t rtl8201_parse_status(eth_phy_dev_t *dev)
{
    ETHPHY_NULL_PARAM_CHK(dev);
    ETHPHY_NULL_PARAM_CHK(dev->priv);

    eth_phy_priv_t *priv = dev->priv;
    uint8_t phy_addr = dev->phy_addr;
    unsigned int speed;
    uint16_t mii_reg;
    int32_t ret;

    ret = eth_phy_read(priv, phy_addr, MII_BMCR, &mii_reg);

    if (ret != 0) {
        return ret;
    }

    if (mii_reg & BMCR_FULLDPLX) {
        priv->link_info.duplex = CSI_ETH_DUPLEX_FULL;
    } else {
        priv->link_info.duplex = CSI_ETH_DUPLEX_HALF;
    }

    speed = mii_reg & BMCR_SPEED100;

    switch (speed) {
        case BMCR_SPEED100:
            priv->link_info.speed = CSI_ETH_SPEED_100M;
            break;

        default:
            priv->link_info.speed = CSI_ETH_SPEED_10M;
            break;
    }

    return 0;
}

static int32_t rtl8201_start(eth_phy_handle_t handle)
{
    ETHPHY_NULL_PARAM_CHK(handle);

    eth_phy_dev_t *dev = (eth_phy_dev_t *)handle;
    int32_t ret;

    ret = generic_phy_update_link(dev);

    if (ret) {
        return ret;
    }
    return rtl8201_parse_status(dev);
}

static int32_t rtl8201_updatelink(eth_phy_handle_t handle)
{
    ETHPHY_NULL_PARAM_CHK(handle);

    eth_phy_dev_t *dev = (eth_phy_dev_t *)handle;
    int32_t ret;

    ret = generic_phy_update_link(dev);

    if (ret) {
        return ret;
    }

    return 0;
}

static int32_t rtl8201_loopback(eth_phy_handle_t handle)
{
    ETHPHY_NULL_PARAM_CHK(handle);
    eth_phy_dev_t *dev = (eth_phy_dev_t *)handle;
    ETHPHY_NULL_PARAM_CHK(dev->priv);

    eth_phy_priv_t *priv = dev->priv;
    uint8_t phy_addr = dev->phy_addr;
    int32_t ret;    
    uint16_t date =0xdead;

    if (dev->interface != PHY_IF_MODE_MII) {
        LOG_I("*** rtl8201: internal loopback only supported for GMII ***\n");
        return ERR_ETHPHY(DRV_ERROR_UNSUPPORTED);
    }

    ret = eth_phy_read(priv, phy_addr, MII_BMCR, &date);
    printf("loopback date 0x%x\r\n",date);
    date |= BMCR_LOOPBACK;
    /* enable the 8201's MAC Interface Loopback mode */
    ret = eth_phy_write(priv, phy_addr, MII_BMCR, date);

    if (ret != 0) {
        LOG_E("set loopback failed\n");
        return ret;
    }
    /* waiting for loopback mode done */
    aos_msleep(3000);

    return 0;
}

eth_phy_dev_t RTL8201F_device = {
    .phy_id = 0x001cc816,
    .mask = 0xffffff0,
    .features = PHY_BASIC_FEATURES,
    .config = &rtl8201_config_init,
    .start = &rtl8201_start,    
    .loopback = &rtl8201_loopback,
    .update_link = &rtl8201_updatelink,    
};

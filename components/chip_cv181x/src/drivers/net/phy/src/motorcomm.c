/*
 * drivers/net/phy/motorcomm.c
 *
 * Driver for Motorcomm PHYs
 *
 * Author: Leilei Zhao <leilei.zhao@motorcomm.com>
 *
 * Copyright (c) 2019 Motorcomm, Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * Support : Motorcomm Phys:
 *		Giga phys: yt8511, yt8521
 *		100/10 Phys : yt8512, yt8512b, yt8510
 *		Automotive 100Mb Phys : yt8010
 *		Automotive 100/10 hyper range Phys: yt8510
 */

#include <stdio.h>
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

#define MOTORCOMM_PHY_ID_MASK	0x00000fff

#define PHY_ID_YT8010		0x00000309
#define PHY_ID_YT8510		0x00000109
#define PHY_ID_YT8511		0x0000010a
#define PHY_ID_YT8512		0x00000118
#define PHY_ID_YT8512B		0x00000128
#define PHY_ID_YT8521		0x0000011a

#define REG_PHY_SPEC_STATUS		0x11
#define REG_DEBUG_ADDR_OFFSET		0x1e
#define REG_DEBUG_DATA			0x1f

#define YT8512_EXTREG_AFE_PLL		0x50
#define YT8512_EXTREG_EXTEND_COMBO	0x4000
#define YT8512_EXTREG_LED0		0x40c0
#define YT8512_EXTREG_LED1		0x40c3

#define YT8512_EXTREG_SLEEP_CONTROL1	0x2027

#define YT_SOFTWARE_RESET		0x8000

#define YT8512_CONFIG_PLL_REFCLK_SEL_EN	0x0040
#define YT8512_CONTROL1_RMII_EN		0x0001
#define YT8512_LED0_ACT_BLK_IND		0x1000
#define YT8512_LED0_DIS_LED_AN_TRY	0x0001
#define YT8512_LED0_BT_BLK_EN		0x0002
#define YT8512_LED0_HT_BLK_EN		0x0004
#define YT8512_LED0_COL_BLK_EN		0x0008
#define YT8512_LED0_BT_ON_EN		0x0010
#define YT8512_LED1_BT_ON_EN		0x0010
#define YT8512_LED1_TXACT_BLK_EN	0x0100
#define YT8512_LED1_RXACT_BLK_EN	0x0200
#define YT8512_SPEED_MODE		0xc000
#define YT8512_DUPLEX			0x2000

#define YT8512_SPEED_MODE_BIT		14
#define YT8512_DUPLEX_BIT		13
#define YT8512_EN_SLEEP_SW_BIT		15

#define YT8521_EXTREG_SLEEP_CONTROL1	0x27
#define YT8521_EN_SLEEP_SW_BIT		15

#define YT8521_SPEED_MODE		0xc000
#define YT8521_DUPLEX			0x2000
#define YT8521_SPEED_MODE_BIT		14
#define YT8521_DUPLEX_BIT		13
#define YT8521_LINK_STATUS_BIT		10

#define BIT(x) ((1 << x))
static uint16_t ytphy_read_ext(eth_phy_handle_t handle, uint32_t regnum)
{
    ETHPHY_NULL_PARAM_CHK(handle);
    eth_phy_dev_t *dev = (eth_phy_dev_t *)handle;
    ETHPHY_NULL_PARAM_CHK(dev->priv);

    eth_phy_priv_t *priv = dev->priv;
    uint8_t phy_addr = dev->phy_addr;

	int ret;
	uint16_t val;

	ret = eth_phy_write(priv, phy_addr, REG_DEBUG_ADDR_OFFSET, regnum);
	if (ret < 0)
		return ret;

	ret = eth_phy_read(priv, phy_addr, REG_DEBUG_DATA, &val);
	if (ret < 0)
		return ret;

	return val;
}

static uint16_t ytphy_write_ext(eth_phy_handle_t handle, uint32_t regnum, uint16_t val)
{
    ETHPHY_NULL_PARAM_CHK(handle);
    eth_phy_dev_t *dev = (eth_phy_dev_t *)handle;
    ETHPHY_NULL_PARAM_CHK(dev->priv);

    eth_phy_priv_t *priv = dev->priv;
    uint8_t phy_addr = dev->phy_addr;

	int ret;

	ret = eth_phy_write(priv, phy_addr, REG_DEBUG_ADDR_OFFSET, regnum);
	if (ret < 0)
		return ret;

	ret = eth_phy_write(priv, phy_addr, REG_DEBUG_DATA, val);

	return ret;
}

// static int yt8512_clk_init(eth_phy_handle_t handle)
// {
//     ETHPHY_NULL_PARAM_CHK(handle);
//     eth_phy_dev_t *dev = (eth_phy_dev_t *)handle;
//     ETHPHY_NULL_PARAM_CHK(dev->priv);

//     eth_phy_priv_t *priv = dev->priv;
//     uint8_t phy_addr = dev->phy_addr;
    

// 	int ret;
// 	uint16_t val;

// 	val = ytphy_read_ext(handle, YT8512_EXTREG_AFE_PLL);
// 	if (val < 0)
// 		return val;

// 	val |= YT8512_CONFIG_PLL_REFCLK_SEL_EN;

// 	ret = ytphy_write_ext(handle, YT8512_EXTREG_AFE_PLL, val);
// 	if (ret < 0)
// 		return ret;

// 	val = ytphy_read_ext(handle, YT8512_EXTREG_EXTEND_COMBO);
// 	if (val < 0)
// 		return val;

// 	val |= YT8512_CONTROL1_RMII_EN;

// 	ret = ytphy_write_ext(handle, YT8512_EXTREG_EXTEND_COMBO, val);
// 	if (ret < 0)
// 		return ret;

//     val = 0;
// 	ret = eth_phy_read(priv, phy_addr, MII_BMCR, &val);
// 	if (ret < 0)
// 		return ret;

// 	val |= YT_SOFTWARE_RESET;
// 	ret = eth_phy_write(priv, phy_addr, MII_BMCR, val);

// 	return ret;
// }

// static int yt8512_led_init(eth_phy_handle_t handle)
// {
// 	int ret;
// 	int val;
// 	int mask;

// 	val = ytphy_read_ext(handle, YT8512_EXTREG_LED0);
// 	if (val < 0)
// 		return val;

// 	val |= YT8512_LED0_ACT_BLK_IND;

// 	mask = YT8512_LED0_DIS_LED_AN_TRY | YT8512_LED0_BT_BLK_EN |
// 		YT8512_LED0_HT_BLK_EN | YT8512_LED0_COL_BLK_EN |
// 		YT8512_LED0_BT_ON_EN;
// 	val &= ~mask;

// 	ret = ytphy_write_ext(handle, YT8512_EXTREG_LED0, val);
// 	if (ret < 0)
// 		return ret;

// 	val = ytphy_read_ext(handle, YT8512_EXTREG_LED1);
// 	if (val < 0)
// 		return val;

// 	val |= YT8512_LED1_BT_ON_EN;

// 	mask = YT8512_LED1_TXACT_BLK_EN | YT8512_LED1_RXACT_BLK_EN;
// 	val &= ~mask;

// 	ret = ytphy_write_ext(handle, YT8512_LED1_BT_ON_EN, val);

// 	return ret;
// }

int genphy_config_init(eth_phy_handle_t handle)
{

    ETHPHY_NULL_PARAM_CHK(handle);
    eth_phy_dev_t *dev = (eth_phy_dev_t *)handle;
    ETHPHY_NULL_PARAM_CHK(dev->priv);

    eth_phy_priv_t *priv = dev->priv;
    uint8_t phy_addr = dev->phy_addr;
    
	int ret;
	uint16_t val;
	uint32_t features;

	features = (SUPPORTED_TP | SUPPORTED_MII
			| SUPPORTED_AUI | SUPPORTED_FIBRE |
			SUPPORTED_BNC | SUPPORTED_Pause | SUPPORTED_Asym_Pause);

	/* Do we support autonegotiation? */
	ret = eth_phy_read(priv, phy_addr, MII_BMSR, &val);
	if (ret < 0)
		return ret;
    
	if (val & BMSR_ANEGCAPABLE)
		features |= SUPPORTED_Autoneg;

	if (val & BMSR_100FULL)
		features |= SUPPORTED_100baseT_Full;
	if (val & BMSR_100HALF)
		features |= SUPPORTED_100baseT_Half;
	if (val & BMSR_10FULL)
		features |= SUPPORTED_10baseT_Full;
	if (val & BMSR_10HALF)
		features |= SUPPORTED_10baseT_Half;

	if (val & BMSR_ESTATEN) {
		ret = eth_phy_read(priv, phy_addr, MII_ESTATUS, &val);
		if (ret < 0)
			return ret;

		if (val & ESTATUS_1000_TFULL)
			features |= SUPPORTED_1000baseT_Full;
		if (val & ESTATUS_1000_THALF)
			features |= SUPPORTED_1000baseT_Half;
	}

	dev->supported &= features;
	dev->advertising &= features;

	return 0;
}

static int yt8512_config_init(eth_phy_handle_t handle)
{
	int ret;
	int val;
    eth_phy_dev_t *dev = (eth_phy_dev_t *)handle;

	ret = genphy_config_init(handle);
	if (ret < 0)
		return ret;
 
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
#if 0
    ret = yt8512_clk_init(handle);
	if (ret < 0)
		return ret;
	ret = yt8512_led_init(handle);
#endif
	/* disable auto sleep */
	val = ytphy_read_ext(handle, YT8512_EXTREG_SLEEP_CONTROL1);
	if (val < 0)
		return val;

	val &= (~BIT(YT8512_EN_SLEEP_SW_BIT));

	ret = ytphy_write_ext(handle, YT8512_EXTREG_SLEEP_CONTROL1, val);
	if (ret < 0)
		return ret;

	return ret;
}
static int yt8512_read_status(eth_phy_handle_t handle)
{
    ETHPHY_NULL_PARAM_CHK(handle);
    eth_phy_dev_t *dev = (eth_phy_dev_t *)handle;
    ETHPHY_NULL_PARAM_CHK(dev->priv);

    eth_phy_priv_t *priv = dev->priv;
    uint8_t phy_addr = dev->phy_addr;

	int ret;
	uint16_t val;
	int speed = CSI_ETH_SPEED_100M;
	int speed_mode, duplex;

	ret = generic_phy_update_link(dev);
	if (ret) {
            return ret;
    }

	ret = eth_phy_read(priv, phy_addr, REG_PHY_SPEC_STATUS, &val);
	if (ret < 0)
		return ret;

	duplex = (val & YT8512_DUPLEX) >> YT8512_DUPLEX_BIT;
	speed_mode = (val & YT8512_SPEED_MODE) >> YT8512_SPEED_MODE_BIT;
	switch (speed_mode) {
	case 0:
		speed = CSI_ETH_SPEED_10M;
		break;
	case 1:
		speed = CSI_ETH_SPEED_100M;
		break;
	case 2:
	case 3:
	default:
		//speed = SPEED_UNKNOWN;
		break;
	}

	priv->link_info.speed = speed;
	priv->link_info.duplex = duplex;

	return 0;
}

     
eth_phy_dev_t YT8512B_device = {
    .phy_id = PHY_ID_YT8512B,
    .mask = MOTORCOMM_PHY_ID_MASK,
    .features = PHY_BASIC_FEATURES,
    .config = &yt8512_config_init,
    .start = &yt8512_read_status,
    //.loopback = &yt8512_loopback,
    //.update_link = &yt8512_update_link,
};


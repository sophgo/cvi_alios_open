/*
 * Copyright (C) 2017-2019 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     phy.c
 * @brief    CSI Source File for generic PHY Driver
 * @version  V1.0
 * @date     21 March 2019
 ******************************************************************************/

/* LOG_LEVEL: 0: Err; 1: Err&Warn; 2: Err&Warn&Info; 3: Err&Warn&Info&Debug */
#define LOG_LEVEL 0
#define CONFIG_ETH_PHY_NUM 2
#include <syslog.h>

#include <soc.h>
#include "phy.h"
#include "aos/kernel.h"
#include <mmio.h>

extern int32_t target_eth_phy_init(int32_t idx, uint32_t *phy_addr, phy_if_mode_t *interface);

extern void udelay(uint32_t us);

eth_phy_priv_t  phy_priv_list[CONFIG_ETH_PHY_NUM];

/* PHY devices */
//extern eth_phy_dev_t M88E1111_device;
//extern eth_phy_dev_t RTL8201F_device;
extern eth_phy_dev_t cv181x_device;

/* registered phy devices */
static eth_phy_dev_t *const eth_phy_devices[] = {
    //&M88E1111_device,
    //&RTL8201F_device,
    &cv181x_device,
    NULL /* Must be the last item */
};

int32_t eth_phy_read(eth_phy_priv_t *priv, uint8_t phy_addr, uint8_t reg_addr, uint16_t *data)
{
    ETHPHY_NULL_PARAM_CHK(priv);
    ETHPHY_NULL_PARAM_CHK(priv->phy_read);
    return priv->phy_read(phy_addr, reg_addr, data);
}

int32_t eth_phy_write(eth_phy_priv_t *priv, uint8_t phy_addr, uint8_t reg_addr, uint16_t data)
{
    ETHPHY_NULL_PARAM_CHK(priv);
    ETHPHY_NULL_PARAM_CHK(priv->phy_write);
    return priv->phy_write(phy_addr, reg_addr, data);
}

static eth_phy_dev_t *eth_get_phy_device(eth_phy_priv_t *priv, uint8_t phy_addr, uint32_t phy_id)
{
    eth_phy_dev_t *p = eth_phy_devices[0];
    int32_t i = 0;

    while (p != NULL) {
        if ((p->phy_id & p->mask) == (phy_id & p->mask)) {
            p->phy_addr = phy_addr;
            p->advertising = p->supported = p->features;
            return p;
        }

        i++;
        p = eth_phy_devices[i];
    }

    return NULL;
}

/**
  \brief       Read the specified phy_addr for PHY ID.
  \param[in]   priv  phy private data
  \param[in]   phy_addr  5-bit device address
  \param[out]  phy_id  Pointer where the PHY ID is written to
  \return      error code
*/
static int32_t eth_read_phy_id(eth_phy_priv_t *priv, uint8_t phy_addr, uint32_t *phy_id)
{
    int32_t ret;
    uint16_t data;
    uint32_t id;

    ret = eth_phy_read(priv, phy_addr, MII_PHYSID1, &data);

    if (ret != 0) {
        return ret;
    }

    id = data;
    id = (id & 0xffff) << 16;

    ret = eth_phy_read(priv, phy_addr, MII_PHYSID2, &data);

    if (ret != 0) {
        return ret;
    }

    id |= (data & 0xffff);

    if (phy_id != NULL) {
        *phy_id = id;
    }

    return 0;
}

static eth_phy_dev_t * eth_get_phy_by_mask(eth_phy_priv_t *priv, uint32_t phy_mask,
                        phy_if_mode_t interface)
{
    uint32_t phy_id = 0xffffffff;
    while (phy_mask) {
		int32_t addr = ffs(phy_mask) - 1;
		int32_t r = eth_read_phy_id(priv, addr, &phy_id);
		/* If the PHY ID is mostly f's, we didn't find anything */
		if (r == 0 && (phy_id & 0x1fffffff) != 0x1fffffff)
			return eth_get_phy_device(priv, addr, phy_id);
		phy_mask &= ~(1 << addr);
	}
	return NULL;
}

static void eth_config(void)
{
    unsigned int val;

    val = mmio_read_32(ETH_PHY_BASE) & ETH_PHY_INIT_MASK;
    mmio_write_32(ETH_PHY_BASE, (val | ETH_PHY_SHUTDOWN) & ETH_PHY_RESET);
    mdelay(1);
    mmio_write_32(ETH_PHY_BASE, val & ETH_PHY_POWERUP & ETH_PHY_RESET);
    mdelay(20);
    mmio_write_32(ETH_PHY_BASE, (val & ETH_PHY_POWERUP) | ETH_PHY_RESET_N);
    mdelay(1);
}

static eth_phy_dev_t *eth_connect_phy(eth_phy_priv_t *priv, uint32_t phy_mask,
                        phy_if_mode_t interface)
{
    int32_t i;
    eth_phy_dev_t *phydev = NULL;

    /* config eth internal phy on ASIC board */
    eth_config();

#ifdef CONFIG_PHY_ADDR
	phy_mask = 1 << CONFIG_PHY_ADDR;
#endif

    for (i = 0; i < 5; i++) {
		phydev = eth_get_phy_by_mask(priv, phy_mask, interface);
		if (phydev)
			return phydev;
	}

	printf("\n PHY: ");
	while (phy_mask) {
		int32_t addr = ffs(phy_mask) - 1;
		printf("%d ", addr);
		phy_mask &= ~(1 << addr);
	}
	printf("not found\n");

	return NULL;
}

/**
  \brief       Soft reset the PHY
  \param[in]   handle  phy handle
  \return      error code
*/
int32_t eth_phy_reset(eth_phy_handle_t handle)
{
    ETHPHY_NULL_PARAM_CHK(handle);
    eth_phy_dev_t *dev = (eth_phy_dev_t *)handle;
    ETHPHY_NULL_PARAM_CHK(dev->priv);
    uint16_t data;
    int32_t ret;
    int32_t timeout = 600;  /* in ms */
    eth_phy_priv_t *priv = dev->priv;
    uint32_t phy_addr = dev->phy_addr;

    /* Soft reset */
    ret = eth_phy_read(priv, phy_addr, MII_BMCR, &data);
    ret = eth_phy_write(priv, phy_addr, MII_BMCR, data | BMCR_RESET);

    if (ret != 0) {
        pr_err("PHY soft reset failed\n");
        return ret;
    }

#ifdef CONFIG_PHY_RESET_DELAY
	udelay(CONFIG_PHY_RESET_DELAY);	/* Intel LXT971A needs this */
#endif
    /*
     * Wait up to 0.6s for the reset sequence to finish. According to
     * IEEE 802.3, Section 2, Subsection 22.2.4.1.1 a PHY reset may take
     * up to 0.5 s.
     */
    ret = eth_phy_read(priv, phy_addr, MII_BMCR, &data);
    while ((data & BMCR_RESET) && timeout--) {
        ret = eth_phy_read(priv, phy_addr, MII_BMCR, &data);

        if (ret != 0) {
            return ret;
        }

        aos_msleep(1);
    }

    if (data & BMCR_RESET) {
        pr_err("PHY reset timed out\n");
        return -1;
    }

    return 0;
}

/**
  \brief       Configure the PHY
  \param[in]   handle  phy handle
  \return      error code
*/
int32_t eth_phy_config(eth_phy_handle_t handle)
{
    eth_phy_dev_t *dev = (eth_phy_dev_t *)handle;

    if (dev->config) {
        return dev->config(handle);
    }

    return 0;
}

/**
  \brief       Start up the PHY
  \param[in]   handle  phy handle
  \return      error code
*/
int32_t eth_phy_start(eth_phy_handle_t handle)
{
    ETHPHY_NULL_PARAM_CHK(handle);
    eth_phy_dev_t *dev = (eth_phy_dev_t *)handle;

    if (dev->start) {
        return dev->start(handle);
    }

    return 0;
}

/**
  \brief       Stop the PHY
  \param[in]   handle  phy handle
  \return      error code
*/
int32_t eth_phy_stop(eth_phy_handle_t handle)
{
    ETHPHY_NULL_PARAM_CHK(handle);
    eth_phy_dev_t *dev = (eth_phy_dev_t *)handle;

    if (dev->start) {
        return dev->stop(handle);
    }

    return 0;
}

/* generic PHY functions */

/**
  \brief       Restart auto-negotiation
  \param[in]   phy_dev  phy device pointer
  \return      error code
*/
int32_t generic_phy_restart_aneg(eth_phy_dev_t *phy_dev)
{
    ETHPHY_NULL_PARAM_CHK(phy_dev);
    ETHPHY_NULL_PARAM_CHK(phy_dev->priv);

    eth_phy_priv_t *priv = phy_dev->priv;
    uint8_t phy_addr = phy_dev->phy_addr;
    uint16_t ctl;
    int32_t ret;

    ret = eth_phy_read(priv, phy_addr, MII_BMCR, &ctl);

    if (ret != 0) {
        return ret;
    }

    ctl |= (BMCR_ANENABLE | BMCR_ANRESTART);

    /* Don't isolate the PHY if we're negotiating */
    ctl &= ~(BMCR_ISOLATE);

    ret = eth_phy_write(priv, phy_addr, MII_BMCR, ctl);

    if (ret != 0) {
        return ret;
    }

    return 0;
}

/**
  \brief       Configure advertisement parameters.
  \param[in]   phy_dev  phy device pointer
  \return      < 0 (errno) on error,
               = 0 if advertisement NOT changed,
               > 0 if advertisement changed.
*/
static int32_t genphy_config_advert(eth_phy_dev_t *phy_dev)
{
    ETHPHY_NULL_PARAM_CHK(phy_dev->priv);

    eth_phy_priv_t *priv = phy_dev->priv;
    uint8_t phy_addr = phy_dev->phy_addr;
    uint32_t advertise;
    uint16_t oldadv, adv, bmsr;
    int32_t changed = 0;
    int32_t ret;

	/* Only allow advertising what this PHY supports */
	phy_dev->advertising &= phy_dev->supported;
	advertise = phy_dev->advertising;

	/* Setup standard advertisement */
    ret = eth_phy_read(priv, phy_addr, MII_ADVERTISE, &adv);
    if (ret != 0) {
        return ret;
    }
	oldadv = adv;

	if (adv < 0)
		return adv;

	adv &= ~(ADVERTISE_ALL | ADVERTISE_100BASE4 | ADVERTISE_PAUSE_CAP |
		 ADVERTISE_PAUSE_ASYM);
	if (advertise & ADVERTISED_10baseT_Half)
		adv |= ADVERTISE_10HALF;
	if (advertise & ADVERTISED_10baseT_Full)
		adv |= ADVERTISE_10FULL;
	if (advertise & ADVERTISED_100baseT_Half)
		adv |= ADVERTISE_100HALF;
	if (advertise & ADVERTISED_100baseT_Full)
		adv |= ADVERTISE_100FULL;
	if (advertise & ADVERTISED_Pause)
		adv |= ADVERTISE_PAUSE_CAP;
	if (advertise & ADVERTISED_Asym_Pause)
		adv |= ADVERTISE_PAUSE_ASYM;
	if (advertise & ADVERTISED_1000baseX_Half)
		adv |= ADVERTISE_1000XHALF;
	if (advertise & ADVERTISED_1000baseX_Full)
		adv |= ADVERTISE_1000XFULL;

	if (adv != oldadv) {
        ret = eth_phy_write(priv, phy_addr, MII_ADVERTISE, adv);

        if (ret != 0) {
            return ret;
        }
		changed = 1;
	}

    ret = eth_phy_read(priv, phy_addr, MII_BMSR, &bmsr);

    if (ret != 0 || bmsr < 0) {
        return ret;
    }

	/* Per 802.3-2008, Section 22.2.4.2.16 Extended status all
	 * 1000Mbits/sec capable PHYs shall have the BMSR_ESTATEN bit set to a
	 * logical 1.
	 */
	if (!(bmsr & BMSR_ESTATEN))
		return changed;

	/* Configure gigabit if it's supported */
    ret = eth_phy_read(priv, phy_addr, MII_CTRL1000, &adv);

    if (ret != 0 || adv < 0) {
        return ret;
    }

    oldadv = adv;

	adv &= ~(ADVERTISE_1000FULL | ADVERTISE_1000HALF);

	if (phy_dev->supported & (SUPPORTED_1000baseT_Half |
				SUPPORTED_1000baseT_Full)) {
		if (advertise & SUPPORTED_1000baseT_Half)
			adv |= ADVERTISE_1000HALF;
		if (advertise & SUPPORTED_1000baseT_Full)
			adv |= ADVERTISE_1000FULL;
	}

	if (adv != oldadv)
		changed = 1;

    ret = eth_phy_write(priv, phy_addr, MII_CTRL1000, adv);

    if (ret != 0) {
        return ret;
    }

	return changed;
}

/**
  \brief       Set up the forced speed/duplex
  \param[in]   phy_dev  phy device pointer
  \return      error code
*/
static int32_t genphy_setup_forced(eth_phy_dev_t *phy_dev)
{
    ETHPHY_NULL_PARAM_CHK(phy_dev->priv);

    eth_phy_priv_t *priv = phy_dev->priv;
    uint8_t phy_addr = phy_dev->phy_addr;
    int32_t ctl = BMCR_ANRESTART;
    int32_t ret;

	if (CSI_ETH_SPEED_1G == priv->link_info.speed)
		ctl |= BMCR_SPEED1000;
	else if (CSI_ETH_SPEED_100M == priv->link_info.speed)
		ctl |= BMCR_SPEED100;
    else//CSI_ETH_SPEED_10M == priv->link_info.speed
		ctl |= BMCR_SPEED100;

	if (CSI_ETH_DUPLEX_FULL == priv->link_info.duplex)
		ctl |= BMCR_FULLDPLX;

    ret = eth_phy_write(priv, phy_addr, MII_BMCR, ctl);

	return ret;
}

/**
 * genphy_restart_aneg - Enable and Restart Autonegotiation
 * @phy_dev: target phy_device struct
 */
int genphy_restart_aneg(eth_phy_dev_t *phy_dev)
{
	int32_t ret;
    uint16_t ctl;
    ret = eth_phy_read(phy_dev->priv, phy_dev->phy_addr, MII_BMCR, &ctl);

	if (ret != 0 || ctl < 0)
		return ret;

	ctl |= (BMCR_ANENABLE | BMCR_ANRESTART);

	/* Don't isolate the PHY if we're negotiating */
	ctl &= ~(BMCR_ISOLATE);

    ret = eth_phy_write(phy_dev->priv, phy_dev->phy_addr, MII_BMCR, ctl);

	return ret;
}

/**
  \brief       Configure auto-negotiation
  \param[in]   phy_dev  phy device pointer
  \return      error code
*/
int32_t genphy_config_aneg(eth_phy_dev_t *phy_dev)
{
    ETHPHY_NULL_PARAM_CHK(phy_dev->priv);

    eth_phy_priv_t *priv = phy_dev->priv;
    uint8_t phy_addr = phy_dev->phy_addr;
    int32_t result;
    uint16_t ctl;
    int32_t ret;

	if (CSI_ETH_AUTONEG_ENABLE != priv->link_info.autoneg)
		return genphy_setup_forced(phy_dev);

	result = genphy_config_advert(phy_dev);

	if (result < 0) /* error */
		return result;

	if (result == 0) {
		/* Advertisment hasn't changed, but maybe aneg was never on to
		 * begin with?  Or maybe phy was isolated? */
        ret = eth_phy_read(priv, phy_addr, MII_BMCR, &ctl);
        if (ret != 0 || ctl < 0)
            return ret;

		if (!(ctl & BMCR_ANENABLE) || (ctl & BMCR_ISOLATE))
			result = 1; /* do restart aneg */
	}

	/* Only restart aneg if we are advertising something different
	 * than we were before.	 */
	if (result > 0)
		result = genphy_restart_aneg(phy_dev);

	return result;
}

/**
  \brief       Configure phy
  \param[in]   phy_dev  phy device pointer
  \return      error code
*/
int32_t genphy_config(eth_phy_dev_t *phy_dev)
{
    ETHPHY_NULL_PARAM_CHK(phy_dev->priv);

    eth_phy_priv_t *priv = phy_dev->priv;
    uint8_t phy_addr = phy_dev->phy_addr;
    int32_t ret;
    uint16_t val;
	uint32_t features;

	features = (SUPPORTED_TP | SUPPORTED_MII
			| SUPPORTED_AUI | SUPPORTED_FIBRE |
			SUPPORTED_BNC);

	/* Do we support autonegotiation? */
    ret = eth_phy_read(priv, phy_addr, MII_BMSR, &val);
	if (ret != 0 || val < 0)
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
	    if (ret != 0 || val < 0)
		    return val;

		if (val & ESTATUS_1000_TFULL)
			features |= SUPPORTED_1000baseT_Full;
		if (val & ESTATUS_1000_THALF)
			features |= SUPPORTED_1000baseT_Half;
		if (val & ESTATUS_1000_XFULL)
			features |= SUPPORTED_1000baseX_Full;
		if (val & ESTATUS_1000_XHALF)
			features |= SUPPORTED_1000baseX_Half;
	}

	phy_dev->supported &= features;
	phy_dev->advertising &= features;

	genphy_config_aneg(phy_dev);

	return 0;
}

/**
  \brief       Update link status
  \param[in]   phy_dev  phy device pointer
  \return      error code
*/
int32_t genphy_update_link(eth_phy_dev_t *phy_dev)
{
    uint8_t phy_addr = phy_dev->phy_addr;
	uint16_t mii_reg;
    int32_t ret;

	/*
	 * Wait if the link is up, and autonegotiation is in progress
	 * (ie - we're capable and it's not done)
	 */
    ret = eth_phy_read(phy_dev->priv, phy_addr, MII_BMSR, &mii_reg);

    if (ret != 0) {
        return ret;
    }

	/*
	 * If we already saw the link up, and it hasn't gone down, then
	 * we don't need to wait for autoneg again
	 */
	if (phy_dev->link_state && mii_reg & BMSR_LSTATUS)
		return 0;

	if ((phy_dev->priv->link_info.autoneg == CSI_ETH_AUTONEG_ENABLE) &&
	    !(mii_reg & BMSR_ANEGCOMPLETE)) {
		int i = 0;

		pr_info("%s waiting for PHY auto negotiation to complete...\n",
			phy_dev->name);
		while (!(mii_reg & BMSR_ANEGCOMPLETE)) {
			/*
			 * Timeout reached ?
			 */
			if (i > PHY_ANEG_TIMEOUT) {
				pr_err("TIMEOUT!\n");
                phy_dev->link_state = ETH_LINK_DOWN;
				return -1;
			}

			// if ((i++ % 1000) == 0)
			// 	printf(".");
            i++;

			udelay(1000);	/* 1 ms */

            ret = eth_phy_read(phy_dev->priv, phy_addr, MII_BMSR, &mii_reg);
            if (ret != 0) {
                return ret;
            }
		}
		pr_info("auto negotiation Done!\n");
		phy_dev->link_state = ETH_LINK_UP;
	} else {

		/* Read the link a second time to clear the latched state */
        ret = eth_phy_read(phy_dev->priv, phy_addr, MII_BMSR, &mii_reg);

        if (ret != 0) {
            return ret;
        }

		if (mii_reg & BMSR_LSTATUS)
			phy_dev->link_state = ETH_LINK_UP;
		else
			phy_dev->link_state = ETH_LINK_DOWN;
	}

	return 0;
}

/**
  \brief       Update PHY's link state
  \param[in]   handle  phy handle
  \return      error code
*/
int32_t eth_phy_update_link(eth_phy_handle_t handle)
{
    ETHPHY_NULL_PARAM_CHK(handle);
    eth_phy_dev_t *dev = (eth_phy_dev_t *)handle;

    if (dev->update_link) {
        return dev->update_link(handle);
    } else {
        return genphy_update_link(dev);
    }
}

/* CSI PHY functions */

/**
  \brief       Get driver version.
  \param[in]   handle  ethernet phy handle
  \return      driver version
*/
csi_drv_version_t csi_eth_phy_get_version(eth_phy_handle_t handle)
{
    csi_drv_version_t ver;
    ver.api = CSI_ETH_PHY_API_VERSION;
    ver.drv = 0;
    return ver;
}

/**
  \brief       Initialize Ethernet PHY Device.
  \param[in]   fn_read
  \param[in]   fn_write
  \return      ethernet phy handle
*/
eth_phy_handle_t csi_eth_phy_initialize(csi_eth_phy_read_t fn_read, csi_eth_phy_write_t fn_write)
{
    ETHPHY_NULL_PARAM_CHK_RE_NULL(fn_read);
    ETHPHY_NULL_PARAM_CHK_RE_NULL(fn_write);
    int32_t idx = 0;    /* phy index */
    eth_phy_dev_t *phy_dev;
    eth_phy_priv_t *priv;
    uint32_t phy_mask = 0xffffffff;
    phy_if_mode_t interface = 0;

    if (idx >= CONFIG_ETH_PHY_NUM) {
        return NULL;
    }

    priv = &phy_priv_list[0];
    priv->phy_read = fn_read;
    priv->phy_write = fn_write;
    priv->link_info.autoneg = CSI_ETH_AUTONEG_ENABLE;

    /* connect phy device */
    phy_dev = eth_connect_phy(priv, phy_mask, interface);

    if (phy_dev == NULL) {
        pr_err("No phy device found!\n");
        return NULL;
    }
    pr_info("connect phy id: 0x%X\n", phy_dev->phy_id);

    phy_dev->priv = priv;

    /* Reset PHY */
    if (eth_phy_reset(phy_dev) != 0) {
        pr_err("phy reset failed\n");
        return NULL;
    }

    phy_dev->supported &= PHY_GBIT_FEATURES;
    phy_dev->advertising = phy_dev->supported;

    /* Config PHY */
    eth_phy_config(phy_dev);

    return phy_dev;
}

/**
  \brief       De-initialize Ethernet PHY Device.
  \param[in]   handle  ethernet phy handle
  \return      error code
*/
int32_t csi_eth_phy_uninitialize(eth_phy_handle_t handle)
{
    return 0;
}

/**
  \brief       Control Ethernet PHY Device Power.
  \param[in]   handle  ethernet phy handle
  \param[in]   state  Power state
  \return      error code
*/
int32_t csi_eth_phy_power_control(eth_phy_handle_t handle, eth_power_state_t state)
{
    if (state == CSI_ETH_POWER_FULL) {
        return eth_phy_start(handle);
    } else if (state == CSI_ETH_POWER_OFF) {
        return eth_phy_stop(handle);
    }

    return 0;
}

/**
  \brief       Set Ethernet Media Interface.
  \param[in]   handle  ethernet phy handle
  \param[in]   interface  Media Interface type
  \return      error code
*/
int32_t csi_eth_phy_set_interface(eth_phy_handle_t handle, uint32_t interface)
{
    return 0;
}

/**
  \brief       Set Ethernet PHY Device Operation mode.
  \param[in]   handle  phy handle
  \param[in]   mode  Operation Mode
  \return      error code
*/
int32_t csi_eth_phy_set_mode(eth_phy_handle_t handle, uint32_t mode)
{
    ETHPHY_NULL_PARAM_CHK(handle);
    eth_phy_dev_t *dev = (eth_phy_dev_t *)handle;

    if (mode & CSI_ETH_PHY_LOOPBACK) {
        if (dev->loopback) {
            return dev->loopback(handle);
        }
    }

    return 0;
}

/**
  \brief       Get Ethernet PHY Device Link state.
  \param[in]   handle  ethernet phy handle
  \return      current link status \ref eth_link_state_t
*/
eth_link_state_t csi_eth_phy_get_linkstate(eth_phy_handle_t handle)
{
    eth_phy_dev_t *phy_dev = (eth_phy_dev_t *)handle;
    uint8_t phy_addr;
    uint16_t mii_reg;
    uint32_t ret;

    if (phy_dev == NULL) {
        LOG_E("DRV_ERROR_PARAMETER");
        return ETH_LINK_DOWN;
    }

    phy_addr = phy_dev->phy_addr;

	/*
	 * Wait if the link is up, and autonegotiation is in progress
	 * (ie - we're capable and it's not done)
	 */
    ret = eth_phy_read(phy_dev->priv, phy_addr, MII_BMSR, &mii_reg);

    if (ret != 0) {
        return ETH_LINK_DOWN;
    }

	/*
	 * If we already saw the link up, and it hasn't gone down, then
	 * we don't need to wait for autoneg again
	 */
    if (mii_reg & BMSR_LSTATUS)
        phy_dev->link_state = ETH_LINK_UP;
    else
        phy_dev->link_state = ETH_LINK_DOWN;

    return phy_dev->link_state;
}

/**
  \brief       Get Ethernet PHY Device Link information.
  \param[in]   handle  ethernet phy handle
  \return      current link parameters \ref eth_link_info_t
*/
eth_link_info_t csi_eth_phy_get_linkinfo(eth_phy_handle_t handle)
{
    eth_phy_dev_t *dev = (eth_phy_dev_t *)handle;
    eth_phy_priv_t *priv;
    eth_link_info_t ret = {0};

    if (dev == NULL) {
        LOG_E("DRV_ERROR_PARAMETER");
        return ret;
    }

    priv = dev->priv;

    if (priv != NULL) {
        return priv->link_info;
    } else {
        LOG_E("No Link Info Found");
        return ret;
    }
}

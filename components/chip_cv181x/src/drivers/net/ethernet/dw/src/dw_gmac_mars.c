/*
* Copyright (C) Cvitek Co., Ltd. 2019-2022. All rights reserved.
*/

/******************************************************************************
 * @file     dw_gmac_mars.c
 * @brief    CSI Source File for DW GMAC Driver
 * @version  V1.0
 * @date     21 July 2022
 ******************************************************************************/

/* LOG_LEVEL: 0: Err; 1: Err&Warn; 2: Err&Warn&Info; 3: Err&Warn&Info&Debug */
#define LOG_LEVEL 0
#include <syslog.h>

#include <string.h>
#include <soc.h>
#include <drv/common.h>
#include <drv/tick.h>
#include <drv/cvi_irq.h>
// #include <drv/eth_mac.h>
// #include <drv/eth_phy.h>
// #include <drv/eth.h>
#include <dw_gmac_mars.h>
#include "errno.h"
#include "drv/porting.h"
#include <aos/kernel.h>
#include <posix/timer.h>

#define roundup(x, y) (					\
{							\
	const typeof(y) __y = y;			\
	(((x) + (__y - 1)) / __y) * __y;		\
}							\
)

#define CONFIG_GMAC_NUM 2

static gmac_dev_t gmac_instance[CONFIG_GMAC_NUM];

extern void mdelay(uint32_t ms);
extern void udelay(uint32_t us);

void dw_gmac_irqhandler(unsigned int irqn, void *arg);

#if 0
static void dw_dump_register(char *tag, struct dw_gmac_dma_regs *dma_p, struct dw_gmac_mac_regs *mac_p)
{
	printf("===========dump %s============\n", tag);
	printf("dma reg:%#x %#x %#x %#x %#x %#x %#x %#x %#x\n",
			dma_p->busmode, dma_p->txpolldemand, dma_p->rxpolldemand,
			dma_p->rxdesclistaddr, dma_p->txdesclistaddr, dma_p->status,
			dma_p->opmode, dma_p->intenable, dma_p->axibus);
	printf("mac reg:%#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x %#x\n",
			mac_p->conf, mac_p->framefilt, mac_p->hashtablehigh,
			mac_p->hashtablelow, mac_p->miiaddr, mac_p->miidata,
			mac_p->flowcontrol, mac_p->vlantag, mac_p->version,
			mac_p->intreg, mac_p->intmask, mac_p->macaddr0hi, mac_p->macaddr0lo);
}
#endif

static int32_t designware_read_hwaddr(eth_mac_handle_t handle)
{
    gmac_dev_t *mac_dev = (gmac_dev_t *)handle;
    struct dw_gmac_mac_regs *mac_reg = mac_dev->priv->mac_regs_p;
	uint32_t macid_lo, macid_hi;
    uint8_t mac_id[6] = {0};

	macid_hi = mac_reg->macaddr0hi;
	macid_lo = mac_reg->macaddr0lo;

    mac_id[0] = macid_lo & 0xff;
    mac_id[1] = (macid_lo >> 8) & 0xff;
    mac_id[2] = (macid_lo >> 16) & 0xff;
    mac_id[3] = (macid_lo >> 24) & 0xff;
    mac_id[4] = macid_hi & 0xff;
    mac_id[5] = (macid_hi >> 8) & 0xff;

    memcpy(mac_dev->mac_addr, mac_id, sizeof(mac_id));

    return 0;
}

static int32_t designware_write_hwaddr(eth_mac_handle_t handle)
{
    gmac_dev_t *mac_dev = (gmac_dev_t *)handle;
    struct dw_gmac_mac_regs *mac_reg = mac_dev->priv->mac_regs_p;

	uint32_t macid_lo, macid_hi;
    uint8_t *mac_id = mac_dev->mac_addr;

	macid_lo = mac_id[0] + (mac_id[1] << 8) + (mac_id[2] << 16) +
		   (mac_id[3] << 24);
	macid_hi = mac_id[4] + (mac_id[5] << 8);

	mac_reg->macaddr0hi = macid_hi;
	mac_reg->macaddr0lo = macid_lo;

	return 0;
}

static void tx_descs_init(eth_mac_handle_t handle)
{
    gmac_dev_t *mac_dev = (gmac_dev_t *)handle;
    struct dw_gmac_priv *priv = mac_dev->priv;
    struct dw_gmac_dma_regs *dma_reg = priv->dma_regs_p;
	struct dmamacdescr *desc_table_p = &priv->tx_mac_descrtable[0];
	char *txbuffs = &priv->txbuffs[0];
	struct dmamacdescr *desc_p;
	uint32_t idx;

	for (idx = 0; idx < CONFIG_TX_DESCR_NUM; idx++) {
		desc_p = &desc_table_p[idx];
		desc_p->dmamac_addr = (unsigned long)&txbuffs[idx * CONFIG_ETH_BUFSIZE];
		desc_p->dmamac_next = (unsigned long)&desc_table_p[idx + 1];

#if defined(CONFIG_DW_ALTDESCRIPTOR)
		desc_p->txrx_status &= ~(DESC_TXSTS_TXINT | DESC_TXSTS_TXLAST |
				DESC_TXSTS_TXFIRST | DESC_TXSTS_TXCRCDIS |
				DESC_TXSTS_TXCHECKINSCTRL |
				DESC_TXSTS_TXRINGEND | DESC_TXSTS_TXPADDIS);

		desc_p->txrx_status |= DESC_TXSTS_TXCHAIN;
		desc_p->dmamac_cntl = 0;
		desc_p->txrx_status &= ~(DESC_TXSTS_MSK | DESC_TXSTS_OWNBYDMA);
#else
		desc_p->dmamac_cntl = DESC_TXCTRL_TXCHAIN;
		desc_p->txrx_status = 0;
#endif
	}

	/* Correcting the last pointer of the chain */
	desc_p->dmamac_next = (unsigned long)&desc_table_p[0];

	/* Flush all Tx buffer descriptors at once */
	soc_dcache_clean_invalid_range((unsigned long)priv->tx_mac_descrtable, sizeof(priv->tx_mac_descrtable));

	dma_reg->txdesclistaddr = (unsigned long)&desc_table_p[0];

	priv->tx_currdescnum = 0;
}

static void rx_descs_init(eth_mac_handle_t handle)
{
    gmac_dev_t *mac_dev = (gmac_dev_t *)handle;
    struct dw_gmac_priv *priv = mac_dev->priv;
    struct dw_gmac_dma_regs *dma_reg = priv->dma_regs_p;
	struct dmamacdescr *desc_table_p = &priv->rx_mac_descrtable[0];
	char *rxbuffs = &priv->rxbuffs[0];
	struct dmamacdescr *desc_p;
	uint32_t idx;

	/* Before passing buffers to GMAC we need to make sure zeros
	 * written there right after "priv" structure allocation were
	 * flushed into RAM.
	 * Otherwise there's a chance to get some of them flushed in RAM when
	 * GMAC is already pushing data to RAM via DMA. This way incoming from
	 * GMAC data will be corrupted. */
	soc_dcache_clean_invalid_range((unsigned long)rxbuffs, RX_TOTAL_BUFSIZE);

	for (idx = 0; idx < CONFIG_RX_DESCR_NUM; idx++) {
		desc_p = &desc_table_p[idx];
		desc_p->dmamac_addr = (unsigned long)&rxbuffs[idx * CONFIG_ETH_BUFSIZE];
		desc_p->dmamac_next = (unsigned long)&desc_table_p[idx + 1];

		desc_p->dmamac_cntl =
			(MAC_MAX_FRAME_SZ & DESC_RXCTRL_SIZE1MASK) |
				      DESC_RXCTRL_RXCHAIN;

		desc_p->txrx_status = DESC_RXSTS_OWNBYDMA;
	}

	/* Correcting the last pointer of the chain */
	desc_p->dmamac_next = (unsigned long)&desc_table_p[0];

	/* Flush all Rx buffer descriptors at once */
	soc_dcache_clean_invalid_range((unsigned long)priv->rx_mac_descrtable, sizeof(priv->rx_mac_descrtable));

	dma_reg->rxdesclistaddr = (unsigned long)&desc_table_p[0];

	priv->rx_currdescnum = 0;
}

static int32_t designware_adjust_link(eth_mac_handle_t handle)
{
    gmac_dev_t *mac_dev = (gmac_dev_t *)handle;
    struct dw_gmac_priv *priv = mac_dev->priv;
    struct dw_gmac_mac_regs *mac_reg = priv->mac_regs_p;
    eth_link_info_t *link_info = &mac_dev->phy_dev->priv->link_info;
    eth_link_state_t link_state = mac_dev->phy_dev->link_state;

	uint32_t conf = mac_reg->conf | FRAMEBURSTENABLE | DISABLERXOWN;

	if (!link_state) {
		pr_err("eth No link.\n");
		return 0;
	}

	if (link_info->speed != CSI_ETH_SPEED_1G)
		conf |= MII_PORTSELECT;
	else
		conf &= ~MII_PORTSELECT;

	if (link_info->speed == CSI_ETH_SPEED_100M)
		conf |= FES_100;

	if (link_info->duplex)
		conf |= FULLDPLXMODE;

	mac_reg->conf = conf;

	pr_info("Speed: %s, duplex: %s\n",
            (link_info->speed) ? "100M" : "10M",
	       (link_info->duplex) ? "full" : "half");

	return 0;
}

static int32_t designware_eth_init(eth_mac_handle_t handle)
{
    gmac_dev_t *mac_dev = (gmac_dev_t *)handle;
    struct dw_gmac_mac_regs *mac_reg = mac_dev->priv->mac_regs_p;
    struct dw_gmac_dma_regs *dma_reg = mac_dev->priv->dma_regs_p;

    uint32_t start;

    dma_reg->busmode |= DMAMAC_SRST;

	start = aos_now_ms();
	while (dma_reg->busmode & DMAMAC_SRST) {
        if ((aos_now_ms() - start) >= CONFIG_MACRESET_TIMEOUT) {
            pr_err("DMA reset timeout\n");
			return -ETIMEDOUT;
        }

		mdelay(100);
	};

	/*
	 * Soft reset above clears HW address registers.
	 * So we have to set it here once again.
	 */
    // designware_read_hwaddr(handle);
	// designware_write_hwaddr(handle);

	rx_descs_init(handle);
	tx_descs_init(handle);

	dma_reg->busmode = (FIXEDBURST | PRIORXTX_41 | DMA_PBL);

    // mac_reg->framefilt = 0x10;
    // mac_reg->flowcontrol = 0x8;
    // dma_reg->wdtforri = 0xff;
    // dma_reg->axibus = 0x0012100F;

#ifndef CONFIG_DW_MAC_FORCE_THRESHOLD_MODE
	dma_reg->opmode |= (FLUSHTXFIFO | STOREFORWARD);
#else
	dma_reg->opmode |= FLUSHTXFIFO;
#endif

    dma_reg->opmode |= (RXSTART | TXSTART);
    dma_reg->opmode = 0x2202906;
    dma_reg->busmode = 0x3900800;
    mac_reg->conf = 0x41cc00;
    dma_reg->intenable = 0x10040;

#ifdef CONFIG_DW_AXI_BURST_LEN
	dma_reg->axibus = (CONFIG_DW_AXI_BURST_LEN & 0x1FF >> 1);
#endif

    /* Start up the PHY */
    /* adjust link */
	return 0;
}

static int32_t designware_eth_enable(eth_mac_handle_t handle, int32_t control)
{
    gmac_dev_t *mac_dev = (gmac_dev_t *)handle;
    struct dw_gmac_mac_regs *mac_reg = mac_dev->priv->mac_regs_p;
    eth_link_state_t link_state = mac_dev->phy_dev->link_state;

	if (link_state == ETH_LINK_DOWN)
		return -1;

    switch (control) {
    case CSI_ETH_MAC_CONTROL_TX:
        mac_reg->conf |= TXENABLE;
        break;
    case CSI_ETH_MAC_CONTROL_RX:
        mac_reg->conf |= RXENABLE;
        break;
    default:
        break;
    }

	return 0;
}
static int32_t designware_eth_disable(eth_mac_handle_t handle, int32_t arg)
{
    gmac_dev_t *mac_dev = (gmac_dev_t *)handle;
    struct dw_gmac_mac_regs *mac_reg = mac_dev->priv->mac_regs_p;

    switch (arg) {
    case CSI_ETH_MAC_CONTROL_TX:
        mac_reg->conf &= ~TXENABLE;
        break;
    case CSI_ETH_MAC_CONTROL_RX:
        mac_reg->conf &= ~RXENABLE;
        break;
    default:
        break;
    }

	return 0;
}

static int32_t designware_eth_start(eth_mac_handle_t handle)
{
    int32_t ret;

	ret = designware_eth_init(handle);
	if (ret)
		return ret;

	return 0;
}

static void designware_eth_stop(eth_mac_handle_t handle)
{
    gmac_dev_t *mac_dev = (gmac_dev_t *)handle;
    struct dw_gmac_mac_regs *mac_reg = mac_dev->priv->mac_regs_p;
    struct dw_gmac_dma_regs *dma_reg = mac_dev->priv->dma_regs_p;

	mac_reg->conf &= ~(RXENABLE | TXENABLE);
	dma_reg->opmode &= ~(RXSTART | TXSTART);

	//phy_shutdown(priv->phydev);
}

static int32_t designware_eth_send(eth_mac_handle_t handle, const uint8_t *frame, uint32_t length)
{
    gmac_dev_t *mac_dev = (gmac_dev_t *)handle;
    struct dw_gmac_priv *priv = mac_dev->priv;
    struct dw_gmac_dma_regs *dma_reg = mac_dev->priv->dma_regs_p;
	uint32_t desc_num = priv->tx_currdescnum;
	struct dmamacdescr *desc_p = &priv->tx_mac_descrtable[desc_num];
	uint64_t desc_start = (uint64_t)desc_p;
	uint64_t desc_end = desc_start +
		roundup(sizeof(*desc_p), DW_GMAC_DMA_ALIGN);
	uint64_t data_start = desc_p->dmamac_addr;
	uint64_t data_end = data_start + roundup(length, DW_GMAC_DMA_ALIGN);
	uint32_t count = 0;

	/*
	 * Strictly we only need to invalidate the "txrx_status" field
	 * for the following check, but on some platforms we cannot
	 * invalidate only 4 bytes, so we flush the entire descriptor,
	 * which is 16 bytes in total. This is safe because the
	 * individual descriptors in the array are each aligned to
	 * DW_GMAC_DMA_ALIGN and padded appropriately.
	 */

	/* Check if the descriptor is owned by CPU */
	while (1) {
		soc_dcache_invalid_range(desc_start, desc_end - desc_start);
		if (!(desc_p->txrx_status & DESC_TXSTS_OWNBYDMA)) {
			break;
		}
		if (count > 1000) {
			pr_err("desc onwer is DMA\n");
			return -1;
		}
		count++;
		mdelay(1);
	}

	memcpy((void *)data_start, frame, length);

	/* Flush data to be sent */
	soc_dcache_clean_invalid_range(data_start, data_end - data_start);

#if defined(CONFIG_DW_ALTDESCRIPTOR)
	desc_p->txrx_status |= DESC_TXSTS_TXFIRST | DESC_TXSTS_TXLAST;
    desc_p->dmamac_cntl &= ~DESC_TXCTRL_SIZE1MASK;
	desc_p->dmamac_cntl |= (length << DESC_TXCTRL_SIZE1SHFT) &
			       DESC_TXCTRL_SIZE1MASK;

	desc_p->txrx_status &= ~(DESC_TXSTS_MSK);
	desc_p->txrx_status |= DESC_TXSTS_OWNBYDMA;
#else
	desc_p->dmamac_cntl &= ~DESC_TXCTRL_SIZE1MASK;
	desc_p->dmamac_cntl |= ((length << DESC_TXCTRL_SIZE1SHFT) &
			       DESC_TXCTRL_SIZE1MASK) | DESC_TXCTRL_TXLAST |
			       DESC_TXCTRL_TXFIRST;

	desc_p->txrx_status = DESC_TXSTS_OWNBYDMA;
#endif

	/* Flush modified buffer descriptor */
	soc_dcache_clean_invalid_range(desc_start, desc_end - desc_start);

	/* Test the wrap-around condition. */
	if (++desc_num >= CONFIG_TX_DESCR_NUM)
		desc_num = 0;

	priv->tx_currdescnum = desc_num;

	/* Start the transmission */
	dma_reg->txpolldemand = POLL_DATA;

	return 0;
}

static int32_t designware_eth_recv(eth_mac_handle_t handle, uint8_t **packetp)
{
    gmac_dev_t *mac_dev = (gmac_dev_t *)handle;
    struct dw_gmac_priv *priv = mac_dev->priv;
    uint32_t status, desc_num = priv->rx_currdescnum;
	struct dmamacdescr *desc_p = &priv->rx_mac_descrtable[desc_num];
	int32_t length = -1;
	uint64_t desc_start = (uint64_t)desc_p;
	uint64_t desc_end = desc_start +
		roundup(sizeof(*desc_p), DW_GMAC_DMA_ALIGN);
	uint64_t data_start = desc_p->dmamac_addr;
	uint64_t data_end;

	/* Invalidate entire buffer descriptor */
	soc_dcache_invalid_range(desc_start, desc_end - desc_start);
	status = desc_p->txrx_status;
	/* Check  if the owner is the CPU */
	if (!(status & DESC_RXSTS_OWNBYDMA)) {
		length = (status & DESC_RXSTS_FRMLENMSK) >>
			 DESC_RXSTS_FRMLENSHFT;
		/* Invalidate received data */
		data_end = data_start + roundup(length, DW_GMAC_DMA_ALIGN);
		soc_dcache_invalid_range(data_start, data_end - data_start);
		*packetp = (uint8_t *)((uint64_t)desc_p->dmamac_addr);
	}

	return length;
}

static int32_t designware_free_pkt(eth_mac_handle_t handle)
{
    gmac_dev_t *mac_dev = (gmac_dev_t *)handle;
    struct dw_gmac_priv *priv = mac_dev->priv;
    uint32_t desc_num = priv->rx_currdescnum;
	struct dmamacdescr *desc_p = &priv->rx_mac_descrtable[desc_num];
	uint64_t desc_start = (uint64_t)desc_p;
	uint64_t desc_end = desc_start +
		roundup(sizeof(*desc_p), DW_GMAC_DMA_ALIGN);

	/*
	 * Make the current descriptor valid again and go to
	 * the next one
	 */
	desc_p->txrx_status |= DESC_RXSTS_OWNBYDMA;

	/* Flush only status field - others weren't changed */
	soc_dcache_clean_invalid_range(desc_start, desc_end - desc_start);

	/* Test the wrap-around condition. */
	if (++desc_num >= CONFIG_RX_DESCR_NUM)
		desc_num = 0;
	priv->rx_currdescnum = desc_num;

	return 0;
}

#if 0
static int designware_mdio_reset(struct mii_dev *bus)
{
	struct udevice *dev = bus->priv;
	struct dw_eth_dev *priv = dev_get_priv(dev);
	struct dw_eth_pdata *pdata = dev_get_platdata(dev);
	int ret;

	if (!dm_gpio_is_valid(&priv->reset_gpio))
		return 0;

	/* reset the phy */
	ret = dm_gpio_set_value(&priv->reset_gpio, 0);
	if (ret)
		return ret;

	udelay(pdata->reset_delays[0]);

	ret = dm_gpio_set_value(&priv->reset_gpio, 1);
	if (ret)
		return ret;

	udelay(pdata->reset_delays[1]);

	ret = dm_gpio_set_value(&priv->reset_gpio, 0);
	if (ret)
		return ret;

	udelay(pdata->reset_delays[2]);

	return 0;
}
#endif

/**
  \brief       Connect phy device to mac device.
  \param[in]   handle_mac  mac handle
  \param[in]   handle_phy  phy handle
*/
void csi_eth_mac_connect_phy(eth_mac_handle_t handle_mac, eth_phy_handle_t handle_phy)
{
    GMAC_NULL_PARAM_CHK_NORETVAL(handle_mac);
    GMAC_NULL_PARAM_CHK_NORETVAL(handle_phy);
    gmac_dev_t *mac_dev = (gmac_dev_t *)handle_mac;
    eth_phy_dev_t *phy_dev = (eth_phy_dev_t *)handle_phy;

    mac_dev->phy_dev = phy_dev;
}

/**
  \brief       Read Ethernet PHY Register through Management Interface.
  \param[in]   handle  ethernet handle
  \param[in]   phy_addr  5-bit device address
  \param[in]   reg_addr  5-bit register address
  \param[out]  data      Pointer where the result is written to
  \return      error code
*/
int32_t csi_eth_mac_phy_read(eth_mac_handle_t handle, uint8_t phy_addr, uint8_t reg_addr, uint16_t *data)
{
    GMAC_NULL_PARAM_CHK(handle);
    GMAC_NULL_PARAM_CHK(data);
    gmac_dev_t *mac_dev = (gmac_dev_t *)handle;
    struct dw_gmac_priv *priv = mac_dev->priv;
    struct dw_gmac_mac_regs *mac_reg = priv->mac_regs_p;
    uint16_t miiaddr;
    int32_t start;

	miiaddr = ((phy_addr << MIIADDRSHIFT) & MII_ADDRMSK) |
		  ((reg_addr << MIIREGSHIFT) & MII_REGMSK);

	mac_reg->miiaddr = (miiaddr | MII_CLKRANGE_150_250M | MII_BUSY);

	start = aos_now_ms();
	while ((aos_now_ms() - start) < CONFIG_MDIO_TIMEOUT) {
		if (!(mac_reg->miiaddr & MII_BUSY)) {
            *data = mac_reg->miidata;
            return 0;
        }
		udelay(10);
	};

	return -1;
}

/**
  \brief       Write Ethernet PHY Register through Management Interface.
  \param[in]   handle  ethernet handle
  \param[in]   phy_addr  5-bit device address
  \param[in]   reg_addr  5-bit register address
  \param[in]   data      16-bit data to write
  \return      error code
*/
int32_t csi_eth_mac_phy_write(eth_mac_handle_t handle, uint8_t phy_addr, uint8_t reg_addr, uint16_t data)
{
    GMAC_NULL_PARAM_CHK(handle);
    gmac_dev_t *mac_dev = (gmac_dev_t *)handle;
    struct dw_gmac_priv *priv = mac_dev->priv;
    struct dw_gmac_mac_regs *mac_reg = priv->mac_regs_p;
    uint16_t miiaddr;
    int32_t start;

    mac_reg->miidata = data;
	miiaddr = ((phy_addr << MIIADDRSHIFT) & MII_ADDRMSK) |
		  ((reg_addr << MIIREGSHIFT) & MII_REGMSK) | MII_WRITE;

	mac_reg->miiaddr = (miiaddr | MII_CLKRANGE_150_250M | MII_BUSY);

    start = aos_now_ms();
	while ((aos_now_ms() - start) < CONFIG_MDIO_TIMEOUT) {
		if (!(mac_reg->miiaddr & MII_BUSY)) {
			return 0;
        }
		udelay(10);
	};

    return -1;
}

/**
  \brief       This function is used to initialize Ethernet device and register an event callback.
  \param[in]   idx device id
  \param[in]   cb  callback to handle ethernet event
  \return      return ethernet handle if success
 */
eth_mac_handle_t csi_eth_mac_initialize(int32_t idx, eth_event_cb_t cb_event)
{
    //GMAC_NULL_PARAM_CHK(cb_event);

    gmac_dev_t *mac_dev = &gmac_instance[idx];
    csi_error_t ret = CSI_OK;
    struct dw_gmac_priv *priv, *priv_unalign;

    ret = target_get(DEV_DW_MAC_TAG, idx, &mac_dev->dev);

    if (ret) {
        pr_err("target_get fail\n");
		return NULL;
    }

    mac_dev->base = (unsigned long)HANDLE_REG_BASE(mac_dev);
    mac_dev->irq = (uint8_t)HANDLE_IRQ_NUM(mac_dev);
    mac_dev->cb_event = cb_event;

	priv = memalign(DW_GMAC_DMA_ALIGN, sizeof(struct dw_gmac_priv),
                            (void **)&priv_unalign);
    if (!priv) {
        pr_err("malloc fail\n");
        return NULL;
    }
    memset(priv_unalign, 0, sizeof(struct dw_gmac_priv) +
            DW_GMAC_DMA_ALIGN);

	priv->mac_regs_p = (struct dw_gmac_mac_regs *)mac_dev->base;
	priv->dma_regs_p = (struct dw_gmac_dma_regs *)(mac_dev->base +
                        DW_DMA_BASE_OFFSET);
	//priv->interface = pdata->phy_interface;
	//priv->max_speed = pdata->max_speed;

    mac_dev->priv_unalign = priv_unalign;
    mac_dev->priv = priv;

    request_irq((uint32_t)(mac_dev->irq), &dw_gmac_irqhandler, 0x1, "dwmac int", mac_dev);

    return (eth_mac_handle_t)mac_dev;
}

/**
  \brief       This function is used to de-initialize Ethernet device.
  \param[in]   handle  ethernet handle
  \return      error code
 */
int32_t csi_eth_mac_uninitialize(eth_mac_handle_t handle)
{
    GMAC_NULL_PARAM_CHK(handle);
    gmac_dev_t *mac_dev = (gmac_dev_t *)handle;

    // csi_irq_disable((uint32_t)(mac_dev->irq));
    // csi_irq_detach((uint32_t)(mac_dev->irq));

    free(mac_dev->priv_unalign);

    return 0;
}

/**
  \brief       Get Ethernet MAC Address.
  \param[in]   handle  ethernet handle
  \param[in]   mac  Pointer to address
  \return      error code
*/
int32_t csi_eth_mac_get_macaddr(eth_mac_handle_t handle, eth_mac_addr_t *mac)
{
    GMAC_NULL_PARAM_CHK(handle);
    GMAC_NULL_PARAM_CHK(mac);

    gmac_dev_t *mac_dev = (gmac_dev_t *)handle;
    
    designware_read_hwaddr(handle);

    memcpy(mac->b, mac_dev->mac_addr, sizeof(mac_dev->mac_addr));

    return 0;
}

/**
  \brief       Set Ethernet MAC Address.
  \param[in]   handle  ethernet handle
  \param[in]   mac  Pointer to address
  \return      error code
*/
int32_t csi_eth_mac_set_macaddr(eth_mac_handle_t handle, const eth_mac_addr_t *mac)
{
    GMAC_NULL_PARAM_CHK(handle);
    GMAC_NULL_PARAM_CHK(mac);

    gmac_dev_t *mac_dev = (gmac_dev_t *)handle;
    memcpy(mac_dev->mac_addr, mac->b, sizeof(mac->b));

    designware_write_hwaddr(handle);

    return 0;
}

/**
  \brief       Send Ethernet frame.
  \param[in]   handle  ethernet handle
  \param[in]   frame  Pointer to frame buffer with data to send
  \param[in]   len    Frame buffer length in bytes
  \param[in]   flags  Frame transmit flags (see CSI_ETH_MAC_TX_FRAME_...)
  \return      error code
*/
int32_t csi_eth_mac_send_frame(eth_mac_handle_t handle, const uint8_t *frame, uint32_t len, uint32_t flags)
{
    GMAC_NULL_PARAM_CHK(handle);
    GMAC_NULL_PARAM_CHK(frame);

    return designware_eth_send(handle, frame, len);
}

/**
  \brief       Read data of received Ethernet frame.
  \param[in]   handle  ethernet handle
  \param[in]   frame  Pointer to frame buffer for data to read into
  \param[in]   len    Frame buffer length in bytes
  \return      number of data bytes read or execution status
                 - value >= 0: number of data bytes read
                 - value < 0: error occurred, value is execution status as defined with execution_status
*/
int32_t csi_eth_mac_read_frame(eth_mac_handle_t handle, uint8_t *frame, uint32_t len)
{
    GMAC_NULL_PARAM_CHK(handle);
    GMAC_NULL_PARAM_CHK(frame);

    uint8_t *packet = NULL;
    int32_t actual_length;

    actual_length = designware_eth_recv(handle, &packet);

    if (actual_length < 0) {
        return -1;
    }

    /* process received packet */
    actual_length = (actual_length > len) ? len : actual_length;

    if (packet != NULL) {
        memcpy(frame, packet, actual_length);
    }

    designware_free_pkt(handle);

    return actual_length;
}

/**
  \brief       Request data of received Ethernet frame.
               drv_eth_mac_request_frame() and drv_eth_mac_release_frame()
               must be called in pairs.
  \param[in]   handle  ethernet handle
  \param[in]   frame  Pointer to frame buffer pointer
  \return      number of data bytes read or execution status
                 - value >= 0: number of data bytes read
                 - value < 0: error occurred
*/
int32_t drv_eth_mac_request_frame(eth_mac_handle_t handle, uint8_t **frame)
{
    return 0;
}

/**
  \brief       Release current Ethernet frame.
               drv_eth_mac_request_frame() and drv_eth_mac_release_frame()
               must be called in pairs.
  \param[in]   handle  ethernet handle
  \return      error code
*/
int32_t drv_eth_mac_release_frame(eth_mac_handle_t handle)
{
    return 0;
}

/**
  \brief       Start Ethernet mac.
  \param[in]   none
  \return      error code
*/
int32_t csi_eth_mac_start(void)
{
    return 0;
}

/**
  \brief       Control Ethernet Interface.
  \param[in]   handle  ethernet handle
  \param[in]   control  Operation
  \param[in]   arg      Argument of operation (optional)
  \return      error code
*/
int32_t csi_eth_mac_control(eth_mac_handle_t handle, uint32_t control, uint32_t arg)
{
    GMAC_NULL_PARAM_CHK(handle);

    gmac_dev_t *mac_dev = (gmac_dev_t *)handle;
    int32_t ret = 0;

    GMAC_NULL_PARAM_CHK(mac_dev->phy_dev);

    switch (control) {
        case CSI_ETH_MAC_CONFIGURE:
            if (arg) {
                /* startup mac */
                ret = designware_eth_start(handle);
            } else {
                /* stop mac */
                designware_eth_stop(handle);
            }
            break;

        case DRV_ETH_MAC_ADJUST_LINK:
            ret = designware_adjust_link(handle);
            break;

        case CSI_ETH_MAC_CONTROL_TX:
            if (arg) {
                /* enable TX */
                ret = designware_eth_enable(handle, CSI_ETH_MAC_CONTROL_TX);
            } else {
                /* disable TX */
                ret = designware_eth_disable(handle, CSI_ETH_MAC_CONTROL_TX);
            }
            break;

        case CSI_ETH_MAC_CONTROL_RX:
            if (arg) {
                /* enable RX */
                ret = designware_eth_enable(handle, CSI_ETH_MAC_CONTROL_RX);
            } else {
                /* disable RX */
                ret = designware_eth_disable(handle, CSI_ETH_MAC_CONTROL_RX);
            }
            break;

        case DRV_ETH_MAC_CONTROL_IRQ:
            if (arg) {
                /* enable interrupt */
            } else {
                /* disable interrupt */
            }
            break;

        default:
            break;
    };

    return ret;
}

#if 1
/**
  \brief       the interrupt service function.
  \param[in]   index of gmac instance.
*/
void dw_gmac_irqhandler(unsigned int irqn, void *arg)
{
    gmac_dev_t *mac_dev = (gmac_dev_t *)arg;
    struct dw_gmac_dma_regs *dma_reg = mac_dev->priv->dma_regs_p;
    uint32_t dma_status;
    uint32_t event = 0;

    /* no ephy or ephy link down */
    if (!mac_dev->phy_dev || !mac_dev->phy_dev->link_state)
        return;

    /* read and clear dma interrupt */
    dma_status = dma_reg->status;
    // pr_info("dma status: 0x%X\n", dma_status);

    /* Clear the interrupt by writing a logic 1 to the CSR5[15-0] */
	dma_reg->status = dma_status & 0x1ffff;

    if (dma_status & DMA_STATUS_RI) {
        event |= CSI_ETH_MAC_EVENT_RX_FRAME;
    }

    if (dma_status & DMA_STATUS_TI) {
        // event |= CSI_ETH_MAC_EVENT_TX_FRAME;
    }

    if (dma_status & DMA_STATUS_ERI) {
        ;
    }

    if (mac_dev->cb_event && (event != 0)) {
        mac_dev->cb_event(mac_dev->dev.idx, event);
    }
}
#endif

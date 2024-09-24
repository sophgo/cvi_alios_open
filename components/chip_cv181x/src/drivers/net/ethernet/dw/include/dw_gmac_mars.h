/*
* Copyright (C) Cvitek Co., Ltd. 2019-2022. All rights reserved.
*/
/******************************************************************************
 * @file     dw_gmac_mars.h
 * @brief    header file for DW GMAC Driver
 * @version  V1.0
 * @date     21 July 2022
 ******************************************************************************/
#ifndef _DW_GMAC_182x_H_
#define _DW_GMAC_182x_H_

#include <soc.h>
#include <csi_core.h>
#include <drv/eth_mac.h>
#include <drv/eth_phy.h>
#include <drv/eth.h>
#include <drv/common.h>
#include <phy.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DW_GMAC_DMA_ALIGN 128

#ifndef _DW_ETH_H
#define _DW_ETH_H

#define GMAC_NULL_PARAM_CHK(para)   CSI_PARAM_CHK(para, -1)
#define GMAC_NULL_PARAM_CHK_NORETVAL(para)   CSI_PARAM_CHK_NORETVAL(para)

#define CONFIG_SYS_HZ 1000
#define CONFIG_TX_DESCR_NUM	16
#define CONFIG_RX_DESCR_NUM	16
#define CONFIG_ETH_BUFSIZE	2048
#define TX_TOTAL_BUFSIZE	(CONFIG_ETH_BUFSIZE * CONFIG_TX_DESCR_NUM)
#define RX_TOTAL_BUFSIZE	(CONFIG_ETH_BUFSIZE * CONFIG_RX_DESCR_NUM)

#define CONFIG_MACRESET_TIMEOUT	(3 * CONFIG_SYS_HZ)
#define CONFIG_MDIO_TIMEOUT	(3 * CONFIG_SYS_HZ)

struct dw_gmac_mac_regs {
	__IOM uint32_t conf;		/* 0x00 */
	__IOM uint32_t framefilt;		/* 0x04 */
	__IOM uint32_t hashtablehigh;	/* 0x08 */
	__IOM uint32_t hashtablelow;	/* 0x0c */
	__IOM uint32_t miiaddr;		/* 0x10 */
	__IOM uint32_t miidata;		/* 0x14 */
	__IOM uint32_t flowcontrol;	/* 0x18 */
	__IOM uint32_t vlantag;		/* 0x1c */
	__IOM uint32_t version;		/* 0x20 */
	__IOM uint32_t reserved_1[5];
	__IOM uint32_t intreg;		/* 0x38 */
	__IOM uint32_t intmask;		/* 0x3c */
	__IOM uint32_t macaddr0hi;		/* 0x40 */
	__IOM uint32_t macaddr0lo;		/* 0x44 */
};

/* MAC configuration register definitions */
#define FRAMEBURSTENABLE	(1 << 21)
#define MII_PORTSELECT		(1 << 15)
#define FES_100			(1 << 14)
#define DISABLERXOWN		(1 << 13)
#define FULLDPLXMODE		(1 << 11)
#define RXENABLE		(1 << 2)
#define TXENABLE		(1 << 3)

/* MII address register definitions */
#define MII_BUSY		(1 << 0)
#define MII_WRITE		(1 << 1)
#define MII_CLKRANGE_60_100M	(0)
#define MII_CLKRANGE_100_150M	(0x4)
#define MII_CLKRANGE_20_35M	(0x8)
#define MII_CLKRANGE_35_60M	(0xC)
#define MII_CLKRANGE_150_250M	(0x10)
#define MII_CLKRANGE_250_300M	(0x14)

#define MIIADDRSHIFT		(11)
#define MIIREGSHIFT		(6)
#define MII_REGMSK		(0x1F << 6)
#define MII_ADDRMSK		(0x1F << 11)


struct dw_gmac_dma_regs {
	__IOM uint32_t busmode;		/* 0x00 */
	__IOM uint32_t txpolldemand;	/* 0x04 */
	__IOM uint32_t rxpolldemand;	/* 0x08 */
	__IOM uint32_t rxdesclistaddr;	/* 0x0c */
	__IOM uint32_t txdesclistaddr;	/* 0x10 */
	__IOM uint32_t status;		/* 0x14 */
	__IOM uint32_t opmode;		/* 0x18 */
	__IOM uint32_t intenable;		/* 0x1c */
	__IOM uint32_t discardedcount;		/* 0x20 */
	__IOM uint32_t wdtforri;		/* 0x24 */
	//__IOM uint32_t reserved1[2];
	__IOM uint32_t axibus;		/* 0x28 */
	__IOM uint32_t reserved2[7];
	__IOM uint32_t currhosttxdesc;	/* 0x48 */
	__IOM uint32_t currhostrxdesc;	/* 0x4c */
	__IOM uint32_t currhosttxbuffaddr;	/* 0x50 */
	__IOM uint32_t currhostrxbuffaddr;	/* 0x54 */
};

#define DW_DMA_BASE_OFFSET	(0x1000)

/* Default DMA Burst length */
#ifndef CONFIG_DW_GMAC_DEFAULT_DMA_PBL
#define CONFIG_DW_GMAC_DEFAULT_DMA_PBL 8
#endif

/* Bus mode register definitions */
#define FIXEDBURST		(1 << 16)
#define PRIORXTX_41		(3 << 14)
#define PRIORXTX_31		(2 << 14)
#define PRIORXTX_21		(1 << 14)
#define PRIORXTX_11		(0 << 14)
#define DMA_PBL			(CONFIG_DW_GMAC_DEFAULT_DMA_PBL<<8)
#define RXHIGHPRIO		(1 << 1)
#define DMAMAC_SRST		(1 << 0)

/* Poll demand definitions */
#define POLL_DATA		(0xFFFFFFFF)

/* Operation mode definitions */
#define STOREFORWARD		(1 << 21)
#define FLUSHTXFIFO		(1 << 20)
#define TXSTART			(1 << 13)
#define TXSECONDFRAME		(1 << 2)
#define RXSTART			(1 << 1)

/* Status definitions */
#define DMA_STATUS_ERI	0x00004000	/* Early Receive Interrupt */
#define DMA_STATUS_RI	0x00000040	/* Receive Interrupt */
#define DMA_STATUS_TI	0x00000001	/* Transmit Interrupt */

/* Descriptior related definitions */
#define MAC_MAX_FRAME_SZ	(1600)

struct dmamacdescr {
	unsigned int txrx_status;
	unsigned int dmamac_cntl;
	unsigned int dmamac_addr;
	unsigned int dmamac_next;
} __attribute__((aligned(DW_GMAC_DMA_ALIGN)));

/*
 * txrx_status definitions
 */

/* tx status bits definitions */
#if defined(CONFIG_DW_ALTDESCRIPTOR)

#define DESC_TXSTS_OWNBYDMA		(1 << 31)
#define DESC_TXSTS_TXINT		(1 << 30)
#define DESC_TXSTS_TXLAST		(1 << 29)
#define DESC_TXSTS_TXFIRST		(1 << 28)
#define DESC_TXSTS_TXCRCDIS		(1 << 27)

#define DESC_TXSTS_TXPADDIS		(1 << 26)
#define DESC_TXSTS_TXCHECKINSCTRL	(3 << 22)
#define DESC_TXSTS_TXRINGEND		(1 << 21)
#define DESC_TXSTS_TXCHAIN		(1 << 20)
#define DESC_TXSTS_MSK			(0x1FFFF << 0)

#else

#define DESC_TXSTS_OWNBYDMA		(1 << 31)
#define DESC_TXSTS_MSK			(0x1FFFF << 0)

#endif

/* rx status bits definitions */
#define DESC_RXSTS_OWNBYDMA		(1 << 31)
#define DESC_RXSTS_DAFILTERFAIL		(1 << 30)
#define DESC_RXSTS_FRMLENMSK		(0x3FFF << 16)
#define DESC_RXSTS_FRMLENSHFT		(16)

#define DESC_RXSTS_ERROR		(1 << 15)
#define DESC_RXSTS_RXTRUNCATED		(1 << 14)
#define DESC_RXSTS_SAFILTERFAIL		(1 << 13)
#define DESC_RXSTS_RXIPC_GIANTFRAME	(1 << 12)
#define DESC_RXSTS_RXDAMAGED		(1 << 11)
#define DESC_RXSTS_RXVLANTAG		(1 << 10)
#define DESC_RXSTS_RXFIRST		(1 << 9)
#define DESC_RXSTS_RXLAST		(1 << 8)
#define DESC_RXSTS_RXIPC_GIANT		(1 << 7)
#define DESC_RXSTS_RXCOLLISION		(1 << 6)
#define DESC_RXSTS_RXFRAMEETHER		(1 << 5)
#define DESC_RXSTS_RXWATCHDOG		(1 << 4)
#define DESC_RXSTS_RXMIIERROR		(1 << 3)
#define DESC_RXSTS_RXDRIBBLING		(1 << 2)
#define DESC_RXSTS_RXCRC		(1 << 1)

/*
 * dmamac_cntl definitions
 */

/* tx control bits definitions */
#if defined(CONFIG_DW_ALTDESCRIPTOR)

#define DESC_TXCTRL_SIZE1MASK		(0x1FFF << 0)
#define DESC_TXCTRL_SIZE1SHFT		(0)
#define DESC_TXCTRL_SIZE2MASK		(0x1FFF << 16)
#define DESC_TXCTRL_SIZE2SHFT		(16)

#else

#define DESC_TXCTRL_TXINT		(1 << 31)
#define DESC_TXCTRL_TXLAST		(1 << 30)
#define DESC_TXCTRL_TXFIRST		(1 << 29)
#define DESC_TXCTRL_TXCHECKINSCTRL	(3 << 27)
#define DESC_TXCTRL_TXCRCDIS		(1 << 26)
#define DESC_TXCTRL_TXRINGEND		(1 << 25)
#define DESC_TXCTRL_TXCHAIN		(1 << 24)

#define DESC_TXCTRL_SIZE1MASK		(0x7FF << 0)
#define DESC_TXCTRL_SIZE1SHFT		(0)
#define DESC_TXCTRL_SIZE2MASK		(0x7FF << 11)
#define DESC_TXCTRL_SIZE2SHFT		(11)

#endif

/* rx control bits definitions */
#if defined(CONFIG_DW_ALTDESCRIPTOR)

#define DESC_RXCTRL_RXINTDIS		(1 << 31)
#define DESC_RXCTRL_RXRINGEND		(1 << 15)
#define DESC_RXCTRL_RXCHAIN		(1 << 14)

#define DESC_RXCTRL_SIZE1MASK		(0x1FFF << 0)
#define DESC_RXCTRL_SIZE1SHFT		(0)
#define DESC_RXCTRL_SIZE2MASK		(0x1FFF << 16)
#define DESC_RXCTRL_SIZE2SHFT		(16)

#else

#define DESC_RXCTRL_RXINTDIS		(1 << 31)
#define DESC_RXCTRL_RXRINGEND		(1 << 25)
#define DESC_RXCTRL_RXCHAIN		(1 << 24)

#define DESC_RXCTRL_SIZE1MASK		(0x7FF << 0)
#define DESC_RXCTRL_SIZE1SHFT		(0)
#define DESC_RXCTRL_SIZE2MASK		(0x7FF << 11)
#define DESC_RXCTRL_SIZE2SHFT		(11)

#endif

struct dw_gmac_priv {
	struct dmamacdescr tx_mac_descrtable[CONFIG_TX_DESCR_NUM] __aligned(DW_GMAC_DMA_ALIGN);
	struct dmamacdescr rx_mac_descrtable[CONFIG_RX_DESCR_NUM] __aligned(DW_GMAC_DMA_ALIGN);
	char txbuffs[TX_TOTAL_BUFSIZE] __aligned(DW_GMAC_DMA_ALIGN);
	char rxbuffs[RX_TOTAL_BUFSIZE] __aligned(DW_GMAC_DMA_ALIGN);

	uint32_t interface;
	uint32_t max_speed;
	uint32_t tx_currdescnum;
	uint32_t rx_currdescnum;

	struct dw_gmac_mac_regs *mac_regs_p;
	struct dw_gmac_dma_regs *dma_regs_p;

	//struct gpio_desc reset_gpio;
};

#ifdef CONFIG_DM_ETH
int designware_eth_ofdata_to_platdata(struct udevice *dev);
int designware_eth_probe(struct udevice *dev);
extern const struct eth_ops designware_eth_ops;

struct dw_eth_pdata {
	struct eth_pdata eth_pdata;
	u32 reset_delays[3];
};

int designware_eth_init(struct dw_eth_dev *priv, u8 *enetaddr);
int designware_eth_enable(struct dw_eth_dev *priv);
int designware_eth_send(struct udevice *dev, void *packet, int length);
int designware_eth_recv(struct udevice *dev, int flags, uchar **packetp);
int designware_eth_free_pkt(struct udevice *dev, uchar *packet,
				   int length);
void designware_eth_stop(struct udevice *dev);
int designware_eth_write_hwaddr(struct udevice *dev);
#endif

#endif

typedef struct {
    csi_dev_t			dev;
    eth_phy_dev_t       *phy_dev;
    unsigned long       base;
    uint8_t             irq;
    eth_event_cb_t      cb_event;
    uint8_t             mac_addr[6];
    struct dw_gmac_priv *priv_unalign;
    struct dw_gmac_priv *priv;
} gmac_dev_t;


static inline void *memalign(uint32_t align, uint32_t size, void **mem_unalign)
{
    void *mem;
    uint32_t offset;

    *mem_unalign = (void *)malloc(size + align);

    if (!*mem_unalign) {
        return NULL;
    }

    offset = *(uint32_t *)mem_unalign % align;

    if (offset == 0) {
        mem = (struct eqos_priv *)*mem_unalign;
    } else {
        mem = (struct eqos_priv *)(*mem_unalign + (align - offset));
    }
	return mem;
}

#ifdef __cplusplus
}
#endif

#endif /* _DW_GMAC_182x_H_ */

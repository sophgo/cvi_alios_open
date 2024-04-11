#ifndef __USB_DWC2_PARAM_H__
#define __USB_DWC2_PARAM_H__

#define MAX_EPS_CHANNELS	16
/**
 * struct dwc2_hw_params - Autodetected parameters.
 *
 * These parameters are the various parameters read from hardware
 * registers during initialization. They typically contain the best
 * supported or maximum value that can be configured in the
 * corresponding dwc2_core_params value.
 *
 * The values that are not in dwc2_core_params are documented below.
 *
 * @op_mode:             Mode of Operation
 *                       0 - HNP- and SRP-Capable OTG (Host & Device)
 *                       1 - SRP-Capable OTG (Host & Device)
 *                       2 - Non-HNP and Non-SRP Capable OTG (Host & Device)
 *                       3 - SRP-Capable Device
 *                       4 - Non-OTG Device
 *                       5 - SRP-Capable Host
 *                       6 - Non-OTG Host
 * @arch:                Architecture
 *                       0 - Slave only
 *                       1 - External DMA
 *                       2 - Internal DMA
 * @ipg_isoc_en:        This feature indicates that the controller supports
 *                      the worst-case scenario of Rx followed by Rx
 *                      Interpacket Gap (IPG) (32 bitTimes) as per the utmi
 *                      specification for any token following ISOC OUT token.
 *                       0 - Don't support
 *                       1 - Support
 * @power_optimized:    Are power optimizations enabled?
 * @num_dev_ep:         Number of device endpoints available
 * @num_dev_in_eps:     Number of device IN endpoints available
 * @num_dev_perio_in_ep: Number of device periodic IN endpoints
 *                       available
 * @dev_token_q_depth:  Device Mode IN Token Sequence Learning Queue
 *                      Depth
 *                       0 to 30
 * @host_perio_tx_q_depth:
 *                      Host Mode Periodic Request Queue Depth
 *                       2, 4 or 8
 * @nperio_tx_q_depth:
 *                      Non-Periodic Request Queue Depth
 *                       2, 4 or 8
 * @hs_phy_type:         High-speed PHY interface type
 *                       0 - High-speed interface not supported
 *                       1 - UTMI+
 *                       2 - ULPI
 *                       3 - UTMI+ and ULPI
 * @fs_phy_type:         Full-speed PHY interface type
 *                       0 - Full speed interface not supported
 *                       1 - Dedicated full speed interface
 *                       2 - FS pins shared with UTMI+ pins
 *                       3 - FS pins shared with ULPI pins
 * @total_fifo_size:    Total internal RAM for FIFOs (bytes)
 * @hibernation:	Is hibernation enabled?
 * @utmi_phy_data_width: UTMI+ PHY data width
 *                       0 - 8 bits
 *                       1 - 16 bits
 *                       2 - 8 or 16 bits
 * @snpsid:             Value from SNPSID register
 * @dev_ep_dirs:        Direction of device endpoints (GHWCFG1)
 * @g_tx_fifo_size:	Power-on values of TxFIFO sizes
 * @dma_desc_enable:    When DMA mode is enabled, specifies whether to use
 *                      address DMA mode or descriptor DMA mode for accessing
 *                      the data FIFOs. The driver will automatically detect the
 *                      value for this if none is specified.
 *                       0 - Address DMA
 *                       1 - Descriptor DMA (default, if available)
 * @enable_dynamic_fifo: 0 - Use coreConsultant-specified FIFO size parameters
 *                       1 - Allow dynamic FIFO sizing (default, if available)
 * @en_multiple_tx_fifo: Specifies whether dedicated per-endpoint transmit FIFOs
 *                      are enabled for non-periodic IN endpoints in device
 *                      mode.
 * @host_nperio_tx_fifo_size: Number of 4-byte words in the non-periodic Tx FIFO
 *                      in host mode when dynamic FIFO sizing is enabled
 *                       16 to 32768
 *                      Actual maximum value is autodetected and also
 *                      the default.
 * @host_perio_tx_fifo_size: Number of 4-byte words in the periodic Tx FIFO in
 *                      host mode when dynamic FIFO sizing is enabled
 *                       16 to 32768
 *                      Actual maximum value is autodetected and also
 *                      the default.
 * @max_transfer_size:  The maximum transfer size supported, in bytes
 *                       2047 to 65,535
 *                      Actual maximum value is autodetected and also
 *                      the default.
 * @max_packet_count:   The maximum number of packets in a transfer
 *                       15 to 511
 *                      Actual maximum value is autodetected and also
 *                      the default.
 * @host_channels:      The number of host channel registers to use
 *                       1 to 16
 *                      Actual maximum value is autodetected and also
 *                      the default.
 * @dev_nperio_tx_fifo_size: Number of 4-byte words in the non-periodic Tx FIFO
 *			     in device mode when dynamic FIFO sizing is enabled
 *			     16 to 32768
 *			     Actual maximum value is autodetected and also
 *			     the default.
 * @i2c_enable:         Specifies whether to use the I2Cinterface for a full
 *                      speed PHY. This parameter is only applicable if phy_type
 *                      is FS.
 *                       0 - No (default)
 *                       1 - Yes
 * @acg_enable:		For enabling Active Clock Gating in the controller
 *                       0 - Disable
 *                       1 - Enable
 * @lpm_mode:		For enabling Link Power Management in the controller
 *                       0 - Disable
 *                       1 - Enable
 * @rx_fifo_size:	Number of 4-byte words in the  Rx FIFO when dynamic
 *			FIFO sizing is enabled 16 to 32768
 *			Actual maximum value is autodetected and also
 *			the default.
 * @service_interval_mode: For enabling service interval based scheduling in the
 *                         controller.
 *                           0 - Disable
 *                           1 - Enable
 */
struct dwc2_hw_params {
	unsigned op_mode:3;
	unsigned arch:2;
	unsigned dma_desc_enable:1;
	unsigned enable_dynamic_fifo:1;
	unsigned en_multiple_tx_fifo:1;
	unsigned rx_fifo_size:16;
	unsigned host_nperio_tx_fifo_size:16;
	unsigned dev_nperio_tx_fifo_size:16;
	unsigned host_perio_tx_fifo_size:16;
	unsigned nperio_tx_q_depth:3;
	unsigned host_perio_tx_q_depth:3;
	unsigned dev_token_q_depth:5;
	unsigned max_transfer_size:26;
	unsigned max_packet_count:11;
	unsigned host_channels:5;
	unsigned hs_phy_type:2;
	unsigned fs_phy_type:2;
	unsigned i2c_enable:1;
	unsigned acg_enable:1;
	unsigned num_dev_ep:4;
	unsigned num_dev_in_eps : 4;
	unsigned num_dev_perio_in_ep:4;
	unsigned total_fifo_size:16;
	unsigned power_optimized:1;
	unsigned hibernation:1;
	unsigned utmi_phy_data_width:2;
	unsigned lpm_mode:1;
	unsigned ipg_isoc_en:1;
	unsigned service_interval_mode:1;
	uint32_t snpsid;
	uint32_t dev_ep_dirs;
	uint32_t g_tx_fifo_size[MAX_EPS_CHANNELS];
};

#define HSOTG_REG(x)	(x)

// #define GOTGCTL				HSOTG_REG(0x000)
#define GOTGCTL_CHIRPEN			BIT(27)
#define GOTGCTL_MULT_VALID_BC_MASK	(0x1f << 22)
#define GOTGCTL_MULT_VALID_BC_SHIFT	22
#define GOTGCTL_CURMODE_HOST		BIT(21)
#define GOTGCTL_OTGVER			BIT(20)
#define GOTGCTL_BSESVLD			BIT(19)
#define GOTGCTL_ASESVLD			BIT(18)
#define GOTGCTL_DBNC_SHORT		BIT(17)
#define GOTGCTL_CONID_B			BIT(16)
#define GOTGCTL_DBNCE_FLTR_BYPASS	BIT(15)
#define GOTGCTL_DEVHNPEN		BIT(11)
#define GOTGCTL_HSTSETHNPEN		BIT(10)
#define GOTGCTL_HNPREQ			BIT(9)
#define GOTGCTL_HSTNEGSCS		BIT(8)
#define GOTGCTL_BVALOVAL		BIT(7)
#define GOTGCTL_BVALOEN			BIT(6)
#define GOTGCTL_AVALOVAL		BIT(5)
#define GOTGCTL_AVALOEN			BIT(4)
#define GOTGCTL_VBVALOVAL		BIT(3)
#define GOTGCTL_VBVALOEN		BIT(2)
#define GOTGCTL_SESREQ			BIT(1)
#define GOTGCTL_SESREQSCS		BIT(0)

// #define GOTGINT				HSOTG_REG(0x004)
#define GOTGINT_DBNCE_DONE		BIT(19)
#define GOTGINT_A_DEV_TOUT_CHG		BIT(18)
#define GOTGINT_HST_NEG_DET		BIT(17)
#define GOTGINT_HST_NEG_SUC_STS_CHNG	BIT(9)
#define GOTGINT_SES_REQ_SUC_STS_CHNG	BIT(8)
#define GOTGINT_SES_END_DET		BIT(2)

// #define GAHBCFG				HSOTG_REG(0x008)
#define GAHBCFG_AHB_SINGLE		BIT(23)
#define GAHBCFG_NOTI_ALL_DMA_WRIT	BIT(22)
#define GAHBCFG_REM_MEM_SUPP		BIT(21)
#define GAHBCFG_P_TXF_EMP_LVL		BIT(8)
#define GAHBCFG_NP_TXF_EMP_LVL		BIT(7)
#define GAHBCFG_DMA_EN			BIT(5)
#define GAHBCFG_HBSTLEN_MASK		(0xf << 1)
#define GAHBCFG_HBSTLEN_SHIFT		1
#define GAHBCFG_HBSTLEN_SINGLE		0
#define GAHBCFG_HBSTLEN_INCR		1
#define GAHBCFG_HBSTLEN_INCR4		3
#define GAHBCFG_HBSTLEN_INCR8		5
#define GAHBCFG_HBSTLEN_INCR16		7
#define GAHBCFG_GLBL_INTR_EN		BIT(0)
#define GAHBCFG_CTRL_MASK		(GAHBCFG_P_TXF_EMP_LVL | \
					 GAHBCFG_NP_TXF_EMP_LVL | \
					 GAHBCFG_DMA_EN | \
					 GAHBCFG_GLBL_INTR_EN)

// #define GUSBCFG				HSOTG_REG(0x00C)
#define GUSBCFG_FORCEDEVMODE		BIT(30)
#define GUSBCFG_FORCEHOSTMODE		BIT(29)
#define GUSBCFG_TXENDDELAY		BIT(28)
#define GUSBCFG_ICTRAFFICPULLREMOVE	BIT(27)
#define GUSBCFG_ICUSBCAP		BIT(26)
#define GUSBCFG_ULPI_INT_PROT_DIS	BIT(25)
#define GUSBCFG_INDICATORPASSTHROUGH	BIT(24)
#define GUSBCFG_INDICATORCOMPLEMENT	BIT(23)
#define GUSBCFG_TERMSELDLPULSE		BIT(22)
#define GUSBCFG_ULPI_INT_VBUS_IND	BIT(21)
#define GUSBCFG_ULPI_EXT_VBUS_DRV	BIT(20)
#define GUSBCFG_ULPI_CLK_SUSP_M		BIT(19)
#define GUSBCFG_ULPI_AUTO_RES		BIT(18)
#define GUSBCFG_ULPI_FS_LS		BIT(17)
#define GUSBCFG_OTG_UTMI_FS_SEL		BIT(16)
#define GUSBCFG_PHY_LP_CLK_SEL		BIT(15)
#define GUSBCFG_USBTRDTIM_MASK		(0xf << 10)
#define GUSBCFG_USBTRDTIM_SHIFT		10
#define GUSBCFG_HNPCAP			BIT(9)
#define GUSBCFG_SRPCAP			BIT(8)
#define GUSBCFG_DDRSEL			BIT(7)
#define GUSBCFG_PHYSEL			BIT(6)
#define GUSBCFG_FSINTF			BIT(5)
#define GUSBCFG_ULPI_UTMI_SEL		BIT(4)
#define GUSBCFG_PHYIF16			BIT(3)
#define GUSBCFG_PHYIF8			(0 << 3)
#define GUSBCFG_TOUTCAL_MASK		(0x7 << 0)
#define GUSBCFG_TOUTCAL_SHIFT		0
#define GUSBCFG_TOUTCAL_LIMIT		0x7
#define GUSBCFG_TOUTCAL(_x)		((_x) << 0)

// #define GRSTCTL				HSOTG_REG(0x010)
#define GRSTCTL_AHBIDLE			BIT(31)
#define GRSTCTL_DMAREQ			BIT(30)
#define GRSTCTL_CSFTRST_DONE		BIT(29)
#define GRSTCTL_TXFNUM_MASK		(0x1f << 6)
#define GRSTCTL_TXFNUM_SHIFT		6
#define GRSTCTL_TXFNUM_LIMIT		0x1f
#define GRSTCTL_TXFNUM(_x)		((_x) << 6)
#define GRSTCTL_TXFFLSH			BIT(5)
#define GRSTCTL_RXFFLSH			BIT(4)
#define GRSTCTL_IN_TKNQ_FLSH		BIT(3)
#define GRSTCTL_FRMCNTRRST		BIT(2)
#define GRSTCTL_HSFTRST			BIT(1)
#define GRSTCTL_CSFTRST			BIT(0)

// #define GINTSTS				HSOTG_REG(0x014)
// #define GINTMSK				HSOTG_REG(0x018)
#define GINTSTS_WKUPINT			BIT(31)
#define GINTSTS_SESSREQINT		BIT(30)
#define GINTSTS_DISCONNINT		BIT(29)
#define GINTSTS_CONIDSTSCHNG		BIT(28)
#define GINTSTS_LPMTRANRCVD		BIT(27)
#define GINTSTS_PTXFEMP			BIT(26)
#define GINTSTS_HCHINT			BIT(25)
#define GINTSTS_PRTINT			BIT(24)
#define GINTSTS_RESETDET		BIT(23)
#define GINTSTS_FET_SUSP		BIT(22)
#define GINTSTS_INCOMPL_IP		BIT(21)
#define GINTSTS_INCOMPL_SOOUT		BIT(21)
#define GINTSTS_INCOMPL_SOIN		BIT(20)
#define GINTSTS_OEPINT			BIT(19)
#define GINTSTS_IEPINT			BIT(18)
#define GINTSTS_EPMIS			BIT(17)
#define GINTSTS_RESTOREDONE		BIT(16)
#define GINTSTS_EOPF			BIT(15)
#define GINTSTS_ISOUTDROP		BIT(14)
#define GINTSTS_ENUMDONE		BIT(13)
#define GINTSTS_USBRST			BIT(12)
#define GINTSTS_USBSUSP			BIT(11)
#define GINTSTS_ERLYSUSP		BIT(10)
#define GINTSTS_I2CINT			BIT(9)
#define GINTSTS_ULPI_CK_INT		BIT(8)
#define GINTSTS_GOUTNAKEFF		BIT(7)
#define GINTSTS_GINNAKEFF		BIT(6)
#define GINTSTS_NPTXFEMP		BIT(5)
#define GINTSTS_RXFLVL			BIT(4)
#define GINTSTS_SOF			BIT(3)
#define GINTSTS_OTGINT			BIT(2)
#define GINTSTS_MODEMIS			BIT(1)
#define GINTSTS_CURMODE_HOST		BIT(0)

// #define GRXSTSR				HSOTG_REG(0x01C)
// #define GRXSTSP				HSOTG_REG(0x020)
#define GRXSTS_FN_MASK			(0x7f << 25)
#define GRXSTS_FN_SHIFT			25
#define GRXSTS_PKTSTS_MASK		(0xf << 17)
#define GRXSTS_PKTSTS_SHIFT		17
#define GRXSTS_PKTSTS_GLOBALOUTNAK	1
#define GRXSTS_PKTSTS_OUTRX		2
#define GRXSTS_PKTSTS_HCHIN		2
#define GRXSTS_PKTSTS_OUTDONE		3
#define GRXSTS_PKTSTS_HCHIN_XFER_COMP	3
#define GRXSTS_PKTSTS_SETUPDONE		4
#define GRXSTS_PKTSTS_DATATOGGLEERR	5
#define GRXSTS_PKTSTS_SETUPRX		6
#define GRXSTS_PKTSTS_HCHHALTED		7
#define GRXSTS_HCHNUM_MASK		(0xf << 0)
#define GRXSTS_HCHNUM_SHIFT		0
#define GRXSTS_DPID_MASK		(0x3 << 15)
#define GRXSTS_DPID_SHIFT		15
#define GRXSTS_BYTECNT_MASK		(0x7ff << 4)
#define GRXSTS_BYTECNT_SHIFT		4
#define GRXSTS_EPNUM_MASK		(0xf << 0)
#define GRXSTS_EPNUM_SHIFT		0

// #define GRXFSIZ				HSOTG_REG(0x024)
#define GRXFSIZ_DEPTH_MASK		(0xffff << 0)
#define GRXFSIZ_DEPTH_SHIFT		0

// #define GNPTXFSIZ			HSOTG_REG(0x028)
/* Use FIFOSIZE_* constants to access this register */

// #define GNPTXSTS			HSOTG_REG(0x02C)
#define GNPTXSTS_NP_TXQ_TOP_MASK		(0x7f << 24)
#define GNPTXSTS_NP_TXQ_TOP_SHIFT		24
#define GNPTXSTS_NP_TXQ_SPC_AVAIL_MASK		(0xff << 16)
#define GNPTXSTS_NP_TXQ_SPC_AVAIL_SHIFT		16
#define GNPTXSTS_NP_TXQ_SPC_AVAIL_GET(_v)	(((_v) >> 16) & 0xff)
#define GNPTXSTS_NP_TXF_SPC_AVAIL_MASK		(0xffff << 0)
#define GNPTXSTS_NP_TXF_SPC_AVAIL_SHIFT		0
#define GNPTXSTS_NP_TXF_SPC_AVAIL_GET(_v)	(((_v) >> 0) & 0xffff)

// #define GI2CCTL				HSOTG_REG(0x0030)
#define GI2CCTL_BSYDNE			BIT(31)
#define GI2CCTL_RW			BIT(30)
#define GI2CCTL_I2CDATSE0		BIT(28)
#define GI2CCTL_I2CDEVADDR_MASK		(0x3 << 26)
#define GI2CCTL_I2CDEVADDR_SHIFT	26
#define GI2CCTL_I2CSUSPCTL		BIT(25)
#define GI2CCTL_ACK			BIT(24)
#define GI2CCTL_I2CEN			BIT(23)
#define GI2CCTL_ADDR_MASK		(0x7f << 16)
#define GI2CCTL_ADDR_SHIFT		16
#define GI2CCTL_REGADDR_MASK		(0xff << 8)
#define GI2CCTL_REGADDR_SHIFT		8
#define GI2CCTL_RWDATA_MASK		(0xff << 0)
#define GI2CCTL_RWDATA_SHIFT		0

// #define GPVNDCTL			HSOTG_REG(0x0034)
// #define GGPIO				HSOTG_REG(0x0038)
#define GGPIO_STM32_OTG_GCCFG_PWRDWN	BIT(16)
#define GGPIO_STM32_OTG_GCCFG_VBDEN	BIT(21)
#define GGPIO_STM32_OTG_GCCFG_IDEN	BIT(22)

// #define GUID				HSOTG_REG(0x003c)
// #define GSNPSID				HSOTG_REG(0x0040)
// #define GHWCFG1				HSOTG_REG(0x0044)
#define GSNPSID_ID_MASK			GENMASK(31, 16)

// #define GHWCFG2				HSOTG_REG(0x0048)
#define GHWCFG2_OTG_ENABLE_IC_USB		BIT(31)
#define GHWCFG2_DEV_TOKEN_Q_DEPTH_MASK		(0x1f << 26)
#define GHWCFG2_DEV_TOKEN_Q_DEPTH_SHIFT		26
#define GHWCFG2_HOST_PERIO_TX_Q_DEPTH_MASK	(0x3 << 24)
#define GHWCFG2_HOST_PERIO_TX_Q_DEPTH_SHIFT	24
#define GHWCFG2_NONPERIO_TX_Q_DEPTH_MASK	(0x3 << 22)
#define GHWCFG2_NONPERIO_TX_Q_DEPTH_SHIFT	22
#define GHWCFG2_MULTI_PROC_INT			BIT(20)
#define GHWCFG2_DYNAMIC_FIFO			BIT(19)
#define GHWCFG2_PERIO_EP_SUPPORTED		BIT(18)
#define GHWCFG2_NUM_HOST_CHAN_MASK		(0xf << 14)
#define GHWCFG2_NUM_HOST_CHAN_SHIFT		14
#define GHWCFG2_NUM_DEV_EP_MASK			(0xf << 10)
#define GHWCFG2_NUM_DEV_EP_SHIFT		10
#define GHWCFG2_FS_PHY_TYPE_MASK		(0x3 << 8)
#define GHWCFG2_FS_PHY_TYPE_SHIFT		8
#define GHWCFG2_FS_PHY_TYPE_NOT_SUPPORTED	0
#define GHWCFG2_FS_PHY_TYPE_DEDICATED		1
#define GHWCFG2_FS_PHY_TYPE_SHARED_UTMI		2
#define GHWCFG2_FS_PHY_TYPE_SHARED_ULPI		3
#define GHWCFG2_HS_PHY_TYPE_MASK		(0x3 << 6)
#define GHWCFG2_HS_PHY_TYPE_SHIFT		6
#define GHWCFG2_HS_PHY_TYPE_NOT_SUPPORTED	0
#define GHWCFG2_HS_PHY_TYPE_UTMI		1
#define GHWCFG2_HS_PHY_TYPE_ULPI		2
#define GHWCFG2_HS_PHY_TYPE_UTMI_ULPI		3
#define GHWCFG2_POINT2POINT			BIT(5)
#define GHWCFG2_ARCHITECTURE_MASK		(0x3 << 3)
#define GHWCFG2_ARCHITECTURE_SHIFT		3
#define GHWCFG2_SLAVE_ONLY_ARCH			0
#define GHWCFG2_EXT_DMA_ARCH			1
#define GHWCFG2_INT_DMA_ARCH			2
#define GHWCFG2_OP_MODE_MASK			(0x7 << 0)
#define GHWCFG2_OP_MODE_SHIFT			0
#define GHWCFG2_OP_MODE_HNP_SRP_CAPABLE		0
#define GHWCFG2_OP_MODE_SRP_ONLY_CAPABLE	1
#define GHWCFG2_OP_MODE_NO_HNP_SRP_CAPABLE	2
#define GHWCFG2_OP_MODE_SRP_CAPABLE_DEVICE	3
#define GHWCFG2_OP_MODE_NO_SRP_CAPABLE_DEVICE	4
#define GHWCFG2_OP_MODE_SRP_CAPABLE_HOST	5
#define GHWCFG2_OP_MODE_NO_SRP_CAPABLE_HOST	6
#define GHWCFG2_OP_MODE_UNDEFINED		7

// #define GHWCFG3				HSOTG_REG(0x004c)
#define GHWCFG3_DFIFO_DEPTH_MASK		(0xffff << 16)
#define GHWCFG3_DFIFO_DEPTH_SHIFT		16
#define GHWCFG3_OTG_LPM_EN			BIT(15)
#define GHWCFG3_BC_SUPPORT			BIT(14)
#define GHWCFG3_OTG_ENABLE_HSIC			BIT(13)
#define GHWCFG3_ADP_SUPP			BIT(12)
#define GHWCFG3_SYNCH_RESET_TYPE		BIT(11)
#define GHWCFG3_OPTIONAL_FEATURES		BIT(10)
#define GHWCFG3_VENDOR_CTRL_IF			BIT(9)
#define GHWCFG3_I2C				BIT(8)
#define GHWCFG3_OTG_FUNC			BIT(7)
#define GHWCFG3_PACKET_SIZE_CNTR_WIDTH_MASK	(0x7 << 4)
#define GHWCFG3_PACKET_SIZE_CNTR_WIDTH_SHIFT	4
#define GHWCFG3_XFER_SIZE_CNTR_WIDTH_MASK	(0xf << 0)
#define GHWCFG3_XFER_SIZE_CNTR_WIDTH_SHIFT	0

// #define GHWCFG4				HSOTG_REG(0x0050)
#define GHWCFG4_DESC_DMA_DYN			BIT(31)
#define GHWCFG4_DESC_DMA			BIT(30)
#define GHWCFG4_NUM_IN_EPS_MASK			(0xf << 26)
#define GHWCFG4_NUM_IN_EPS_SHIFT		26
#define GHWCFG4_DED_FIFO_EN			BIT(25)
#define GHWCFG4_DED_FIFO_SHIFT		25
#define GHWCFG4_SESSION_END_FILT_EN		BIT(24)
#define GHWCFG4_B_VALID_FILT_EN			BIT(23)
#define GHWCFG4_A_VALID_FILT_EN			BIT(22)
#define GHWCFG4_VBUS_VALID_FILT_EN		BIT(21)
#define GHWCFG4_IDDIG_FILT_EN			BIT(20)
#define GHWCFG4_NUM_DEV_MODE_CTRL_EP_MASK	(0xf << 16)
#define GHWCFG4_NUM_DEV_MODE_CTRL_EP_SHIFT	16
#define GHWCFG4_UTMI_PHY_DATA_WIDTH_MASK	(0x3 << 14)
#define GHWCFG4_UTMI_PHY_DATA_WIDTH_SHIFT	14
#define GHWCFG4_UTMI_PHY_DATA_WIDTH_8		0
#define GHWCFG4_UTMI_PHY_DATA_WIDTH_16		1
#define GHWCFG4_UTMI_PHY_DATA_WIDTH_8_OR_16	2
#define GHWCFG4_ACG_SUPPORTED			BIT(12)
#define GHWCFG4_IPG_ISOC_SUPPORTED		BIT(11)
#define GHWCFG4_SERVICE_INTERVAL_SUPPORTED      BIT(10)
#define GHWCFG4_XHIBER				BIT(7)
#define GHWCFG4_HIBER				BIT(6)
#define GHWCFG4_MIN_AHB_FREQ			BIT(5)
#define GHWCFG4_POWER_OPTIMIZ			BIT(4)
#define GHWCFG4_NUM_DEV_PERIO_IN_EP_MASK	(0xf << 0)
#define GHWCFG4_NUM_DEV_PERIO_IN_EP_SHIFT	0

// #define GLPMCFG				HSOTG_REG(0x0054)
#define GLPMCFG_INVSELHSIC		BIT(31)
#define GLPMCFG_HSICCON			BIT(30)
#define GLPMCFG_RSTRSLPSTS		BIT(29)
#define GLPMCFG_ENBESL			BIT(28)
#define GLPMCFG_LPM_RETRYCNT_STS_MASK	(0x7 << 25)
#define GLPMCFG_LPM_RETRYCNT_STS_SHIFT	25
#define GLPMCFG_SNDLPM			BIT(24)
#define GLPMCFG_RETRY_CNT_MASK		(0x7 << 21)
#define GLPMCFG_RETRY_CNT_SHIFT		21
#define GLPMCFG_LPM_REJECT_CTRL_CONTROL	BIT(21)
#define GLPMCFG_LPM_ACCEPT_CTRL_ISOC	BIT(22)
#define GLPMCFG_LPM_CHNL_INDX_MASK	(0xf << 17)
#define GLPMCFG_LPM_CHNL_INDX_SHIFT	17
#define GLPMCFG_L1RESUMEOK		BIT(16)
#define GLPMCFG_SLPSTS			BIT(15)
#define GLPMCFG_COREL1RES_MASK		(0x3 << 13)
#define GLPMCFG_COREL1RES_SHIFT		13
#define GLPMCFG_HIRD_THRES_MASK		(0x1f << 8)
#define GLPMCFG_HIRD_THRES_SHIFT	8
#define GLPMCFG_HIRD_THRES_EN		(0x10 << 8)
#define GLPMCFG_ENBLSLPM		BIT(7)
#define GLPMCFG_BREMOTEWAKE		BIT(6)
#define GLPMCFG_HIRD_MASK		(0xf << 2)
#define GLPMCFG_HIRD_SHIFT		2
#define GLPMCFG_APPL1RES		BIT(1)
#define GLPMCFG_LPMCAP			BIT(0)

// #define GPWRDN				HSOTG_REG(0x0058)
#define GPWRDN_MULT_VAL_ID_BC_MASK	(0x1f << 24)
#define GPWRDN_MULT_VAL_ID_BC_SHIFT	24
#define GPWRDN_ADP_INT			BIT(23)
#define GPWRDN_BSESSVLD			BIT(22)
#define GPWRDN_IDSTS			BIT(21)
#define GPWRDN_LINESTATE_MASK		(0x3 << 19)
#define GPWRDN_LINESTATE_SHIFT		19
#define GPWRDN_STS_CHGINT_MSK		BIT(18)
#define GPWRDN_STS_CHGINT		BIT(17)
#define GPWRDN_SRP_DET_MSK		BIT(16)
#define GPWRDN_SRP_DET			BIT(15)
#define GPWRDN_CONNECT_DET_MSK		BIT(14)
#define GPWRDN_CONNECT_DET		BIT(13)
#define GPWRDN_DISCONN_DET_MSK		BIT(12)
#define GPWRDN_DISCONN_DET		BIT(11)
#define GPWRDN_RST_DET_MSK		BIT(10)
#define GPWRDN_RST_DET			BIT(9)
#define GPWRDN_LNSTSCHG_MSK		BIT(8)
#define GPWRDN_LNSTSCHG			BIT(7)
#define GPWRDN_DIS_VBUS			BIT(6)
#define GPWRDN_PWRDNSWTCH		BIT(5)
#define GPWRDN_PWRDNRSTN		BIT(4)
#define GPWRDN_PWRDNCLMP		BIT(3)
#define GPWRDN_RESTORE			BIT(2)
#define GPWRDN_PMUACTV			BIT(1)
#define GPWRDN_PMUINTSEL		BIT(0)

// #define GDFIFOCFG			HSOTG_REG(0x005c)
#define GDFIFOCFG_EPINFOBASE_MASK	(0xffff << 16)
#define GDFIFOCFG_EPINFOBASE_SHIFT	16
#define GDFIFOCFG_GDFIFOCFG_MASK	(0xffff << 0)
#define GDFIFOCFG_GDFIFOCFG_SHIFT	0

// #define ADPCTL				HSOTG_REG(0x0060)
#define ADPCTL_AR_MASK			(0x3 << 27)
#define ADPCTL_AR_SHIFT			27
#define ADPCTL_ADP_TMOUT_INT_MSK	BIT(26)
#define ADPCTL_ADP_SNS_INT_MSK		BIT(25)
#define ADPCTL_ADP_PRB_INT_MSK		BIT(24)
#define ADPCTL_ADP_TMOUT_INT		BIT(23)
#define ADPCTL_ADP_SNS_INT		BIT(22)
#define ADPCTL_ADP_PRB_INT		BIT(21)
#define ADPCTL_ADPENA			BIT(20)
#define ADPCTL_ADPRES			BIT(19)
#define ADPCTL_ENASNS			BIT(18)
#define ADPCTL_ENAPRB			BIT(17)
#define ADPCTL_RTIM_MASK		(0x7ff << 6)
#define ADPCTL_RTIM_SHIFT		6
#define ADPCTL_PRB_PER_MASK		(0x3 << 4)
#define ADPCTL_PRB_PER_SHIFT		4
#define ADPCTL_PRB_DELTA_MASK		(0x3 << 2)
#define ADPCTL_PRB_DELTA_SHIFT		2
#define ADPCTL_PRB_DSCHRG_MASK		(0x3 << 0)
#define ADPCTL_PRB_DSCHRG_SHIFT		0

// #define GREFCLK				    HSOTG_REG(0x0064)
#define GREFCLK_REFCLKPER_MASK		    (0x1ffff << 15)
#define GREFCLK_REFCLKPER_SHIFT		    15
#define GREFCLK_REF_CLK_MODE		    BIT(14)
#define GREFCLK_SOF_CNT_WKUP_ALERT_MASK	    (0x3ff)
#define GREFCLK_SOF_CNT_WKUP_ALERT_SHIFT    0

// #define GINTMSK2			HSOTG_REG(0x0068)
#define GINTMSK2_WKUP_ALERT_INT_MSK	BIT(0)

// #define GINTSTS2			HSOTG_REG(0x006c)
#define GINTSTS2_WKUP_ALERT_INT		BIT(0)

// #define HPTXFSIZ			HSOTG_REG(0x100)
/* Use FIFOSIZE_* constants to access this register */

// #define DPTXFSIZN(_a)			HSOTG_REG(0x104 + (((_a) - 1) * 4))
/* Use FIFOSIZE_* constants to access this register */

/* These apply to the GNPTXFSIZ, HPTXFSIZ and DPTXFSIZN registers */
#define FIFOSIZE_DEPTH_MASK		(0xffff << 16)
#define FIFOSIZE_DEPTH_SHIFT		16
#define FIFOSIZE_STARTADDR_MASK		(0xffff << 0)
#define FIFOSIZE_STARTADDR_SHIFT	0
#define FIFOSIZE_DEPTH_GET(_x)		(((_x) >> 16) & 0xffff)



int dwc2_get_hwparams(void);

#endif
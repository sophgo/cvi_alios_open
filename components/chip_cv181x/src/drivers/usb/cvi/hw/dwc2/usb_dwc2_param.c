
#include "usbd_core.h"
#include "usb_dwc2_reg.h"

#include "usb_dwc2_param.h"

#define DWC2_PRINT_HW_PARAMS(hw, field) printf(#field ":%#x\n", (hw)->field)

struct dwc2_hw_params g_dwc2_hw_params;

static void dwc2_print_hwparams(void)
{
	return;
	struct dwc2_hw_params *hw = &g_dwc2_hw_params;
	DWC2_PRINT_HW_PARAMS(hw, op_mode);
	DWC2_PRINT_HW_PARAMS(hw, arch);
	DWC2_PRINT_HW_PARAMS(hw, dma_desc_enable);
	DWC2_PRINT_HW_PARAMS(hw, enable_dynamic_fifo);
	DWC2_PRINT_HW_PARAMS(hw, en_multiple_tx_fifo);
	DWC2_PRINT_HW_PARAMS(hw, rx_fifo_size);
	DWC2_PRINT_HW_PARAMS(hw, host_nperio_tx_fifo_size);
	DWC2_PRINT_HW_PARAMS(hw, dev_nperio_tx_fifo_size);
	DWC2_PRINT_HW_PARAMS(hw, host_perio_tx_fifo_size);
	DWC2_PRINT_HW_PARAMS(hw, nperio_tx_q_depth);
	DWC2_PRINT_HW_PARAMS(hw, host_perio_tx_q_depth);
	DWC2_PRINT_HW_PARAMS(hw, dev_token_q_depth);
	DWC2_PRINT_HW_PARAMS(hw, max_transfer_size);
	DWC2_PRINT_HW_PARAMS(hw, max_packet_count);
	DWC2_PRINT_HW_PARAMS(hw, host_channels);
	DWC2_PRINT_HW_PARAMS(hw, hs_phy_type);
	DWC2_PRINT_HW_PARAMS(hw, fs_phy_type);
	DWC2_PRINT_HW_PARAMS(hw, i2c_enable);
	DWC2_PRINT_HW_PARAMS(hw, acg_enable);
	DWC2_PRINT_HW_PARAMS(hw, num_dev_ep);
	DWC2_PRINT_HW_PARAMS(hw, num_dev_in_eps);
	DWC2_PRINT_HW_PARAMS(hw, num_dev_perio_in_ep);
	DWC2_PRINT_HW_PARAMS(hw, total_fifo_size);
	DWC2_PRINT_HW_PARAMS(hw, power_optimized);
	DWC2_PRINT_HW_PARAMS(hw, hibernation);
	DWC2_PRINT_HW_PARAMS(hw, utmi_phy_data_width);
	DWC2_PRINT_HW_PARAMS(hw, lpm_mode);
	DWC2_PRINT_HW_PARAMS(hw, ipg_isoc_en);
	DWC2_PRINT_HW_PARAMS(hw, service_interval_mode);
	DWC2_PRINT_HW_PARAMS(hw, snpsid);
	DWC2_PRINT_HW_PARAMS(hw, dev_ep_dirs);
}

int dwc2_get_hwparams(void)
{
	struct dwc2_hw_params *hw = &g_dwc2_hw_params;
	unsigned int width;
	u32 hwcfg1, hwcfg2, hwcfg3, hwcfg4;
	u32 grxfsiz;

	hwcfg1 = USB_OTG_GLB->GHWCFG1;
	hwcfg2 = USB_OTG_GLB->GHWCFG2;
	hwcfg3 = USB_OTG_GLB->GHWCFG3;
	hwcfg4 = USB_OTG_GLB->GHWCFG4;
	grxfsiz = USB_OTG_GLB->GRXFSIZ;

	/* hwcfg1 */
	hw->dev_ep_dirs = hwcfg1;

	/* hwcfg2 */
	hw->op_mode = (hwcfg2 & GHWCFG2_OP_MODE_MASK) >>
		      GHWCFG2_OP_MODE_SHIFT;
	hw->arch = (hwcfg2 & GHWCFG2_ARCHITECTURE_MASK) >>
		   GHWCFG2_ARCHITECTURE_SHIFT;
	hw->enable_dynamic_fifo = !!(hwcfg2 & GHWCFG2_DYNAMIC_FIFO);
	hw->host_channels = 1 + ((hwcfg2 & GHWCFG2_NUM_HOST_CHAN_MASK) >>
				GHWCFG2_NUM_HOST_CHAN_SHIFT);
	hw->hs_phy_type = (hwcfg2 & GHWCFG2_HS_PHY_TYPE_MASK) >>
			  GHWCFG2_HS_PHY_TYPE_SHIFT;
	hw->fs_phy_type = (hwcfg2 & GHWCFG2_FS_PHY_TYPE_MASK) >>
			  GHWCFG2_FS_PHY_TYPE_SHIFT;
	hw->num_dev_ep = (hwcfg2 & GHWCFG2_NUM_DEV_EP_MASK) >>
			 GHWCFG2_NUM_DEV_EP_SHIFT;
	hw->nperio_tx_q_depth =
		(hwcfg2 & GHWCFG2_NONPERIO_TX_Q_DEPTH_MASK) >>
		GHWCFG2_NONPERIO_TX_Q_DEPTH_SHIFT << 1;
	hw->host_perio_tx_q_depth =
		(hwcfg2 & GHWCFG2_HOST_PERIO_TX_Q_DEPTH_MASK) >>
		GHWCFG2_HOST_PERIO_TX_Q_DEPTH_SHIFT << 1;
	hw->dev_token_q_depth =
		(hwcfg2 & GHWCFG2_DEV_TOKEN_Q_DEPTH_MASK) >>
		GHWCFG2_DEV_TOKEN_Q_DEPTH_SHIFT;

	/* hwcfg3 */
	width = (hwcfg3 & GHWCFG3_XFER_SIZE_CNTR_WIDTH_MASK) >>
		GHWCFG3_XFER_SIZE_CNTR_WIDTH_SHIFT;
	hw->max_transfer_size = (1 << (width + 11)) - 1;
	width = (hwcfg3 & GHWCFG3_PACKET_SIZE_CNTR_WIDTH_MASK) >>
		GHWCFG3_PACKET_SIZE_CNTR_WIDTH_SHIFT;
	hw->max_packet_count = (1 << (width + 4)) - 1;
	hw->i2c_enable = !!(hwcfg3 & GHWCFG3_I2C);
	hw->total_fifo_size = (hwcfg3 & GHWCFG3_DFIFO_DEPTH_MASK) >>
			      GHWCFG3_DFIFO_DEPTH_SHIFT;
	hw->lpm_mode = !!(hwcfg3 & GHWCFG3_OTG_LPM_EN);

	/* hwcfg4 */
	hw->en_multiple_tx_fifo = !!(hwcfg4 & GHWCFG4_DED_FIFO_EN);
	hw->num_dev_perio_in_ep = (hwcfg4 & GHWCFG4_NUM_DEV_PERIO_IN_EP_MASK) >>
				  GHWCFG4_NUM_DEV_PERIO_IN_EP_SHIFT;
	hw->num_dev_in_eps = (hwcfg4 & GHWCFG4_NUM_IN_EPS_MASK) >>
			     GHWCFG4_NUM_IN_EPS_SHIFT;
	hw->dma_desc_enable = !!(hwcfg4 & GHWCFG4_DESC_DMA);
	hw->power_optimized = !!(hwcfg4 & GHWCFG4_POWER_OPTIMIZ);
	hw->hibernation = !!(hwcfg4 & GHWCFG4_HIBER);
	hw->utmi_phy_data_width = (hwcfg4 & GHWCFG4_UTMI_PHY_DATA_WIDTH_MASK) >>
				  GHWCFG4_UTMI_PHY_DATA_WIDTH_SHIFT;
	hw->acg_enable = !!(hwcfg4 & GHWCFG4_ACG_SUPPORTED);
	hw->ipg_isoc_en = !!(hwcfg4 & GHWCFG4_IPG_ISOC_SUPPORTED);
	hw->service_interval_mode = !!(hwcfg4 &
				       GHWCFG4_SERVICE_INTERVAL_SUPPORTED);

	/* fifo sizes */
	hw->rx_fifo_size = (grxfsiz & GRXFSIZ_DEPTH_MASK) >>
				GRXFSIZ_DEPTH_SHIFT;
	/*
	 * Host specific hardware parameters. Reading these parameters
	 * requires the controller to be in host mode. The mode will
	 * be forced, if necessary, to read these values.
	 */
	// dwc2_get_host_hwparams(hsotg);
	// dwc2_get_dev_hwparams(hsotg);

	dwc2_print_hwparams();

	return 0;
}


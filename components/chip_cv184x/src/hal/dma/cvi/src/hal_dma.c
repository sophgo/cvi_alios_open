/*
* Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
*/

#include <cvi_dma_ll.h>

void hal_dma_dwc_clk_set(int enable)
{
	uint32_t clk_state;

	clk_state = mmio_read_32((long unsigned int)DMA_CLK_EN_REG);

	if(enable)
		clk_state |= 1 << CLK_SDMA_AXI_BIT;
	else
		clk_state &= ~(1 << CLK_SDMA_AXI_BIT);

	mmio_write_32((long unsigned int)DMA_CLK_EN_REG, clk_state);
}

void hal_dma_off(dw_dma_t *dma)
{
	dma_writeq(dma, CFG, 0); /* disable dmac and interrupt */
	while(dma_readq(dma, CFG) & 3)
		barrier();
}

void hal_dma_on(dw_dma_t *dma)
{
	dma_writeq(dma, CFG, DW_CFG_DMA_EN | DW_CFG_DMA_INT_EN);
}

void hal_dma_write_cfg(dw_dma_channel_t *dwc, uint64_t cfg)
{
	channel_writeq(dwc, CFG, cfg);
}

void hal_dma_enable_irq(dw_dma_channel_t *dwc, uint64_t int_status_reg)
{
	channel_writeq(dwc, INTSTATUS_ENABLEREG, int_status_reg);
	channel_writeq(dwc, INTSIGNAL_ENABLEREG, int_status_reg);
}

void hal_dma_write_llp(dw_dma_channel_t *dwc, uint64_t llp)
{
	channel_writeq(dwc, LLP, llp);
}

uint64_t hal_dma_get_ch_en_status(struct dw_dma *dma)
{
	return dma_readq(dma, CH_EN);
}

void hal_dma_ch_on(struct dw_dma *dma, uint8_t ch_mask)
{
	uint64_t ch_en;

	ch_en = dma_readq(dma, CH_EN);
	ch_en |= (ch_mask << DW_DMAC_CH_EN_WE_OFFSET) | ch_mask;
	dma_writeq(dma, CH_EN, ch_en);
}

void hal_dma_ch_off(struct dw_dma *dma, uint8_t ch_mask)
{
	uint64_t dma_ch_en;

	dma_ch_en = dma_readq(dma, CH_EN);
	dma_ch_en |= (ch_mask << DW_DMAC_CH_EN_WE_OFFSET);
	dma_ch_en &= ~ch_mask;
	dma_writeq(dma, CH_EN, dma_ch_en);
	while (dma_readq(dma, CH_EN) & ch_mask)
		barrier();
}

uint64_t hal_dma_get_intstatus(dw_dma_t *dma)
{
	return dma_readq(dma, INTSTATUS);
}

void hal_dma_clear_comm_intstatus(dw_dma_t *dma)
{
	dma_writeq(dma, COMM_INTCLEAR, 0x10f);
}

/* return dwc status */
uint64_t hal_dma_dwc_read_clear_intstatus(dw_dma_channel_t *dwc)
{
	uint64_t dwc_status;

	dwc_status = channel_readq(dwc, INTSTATUS);
	dma_dbg("channel, intstatus = %lx\r\n", dwc_status);
	channel_writeq(dwc, INTCLEARREG, dwc_status);

	return dwc_status;
}

void hal_dma_dwc_clear_intstatus(dw_dma_channel_t *dwc)
{
	channel_writeq(dwc, INTCLEARREG, 0xffffffff);
}

void hal_dma_turn_off_chans(dw_dma_t *dma)
{
	dma_set_bit(dma, CH_EN, (uint64_t)DW_DMA_CHAN_MASK << DW_DMAC_CH_EN_WE_OFFSET);
	dma_set_bit(dma, CH_EN, (uint64_t)DW_DMA_CHAN_MASK << DW_DMAC_CH_PAUSE_EN_OFFSET);
	dma_set_bit(dma, CH_EN, (uint64_t)DW_DMA_CHAN_MASK << DW_DMAC_CH_ABORT_EN_OFFSET);
	dma_clear_bit(dma, CH_EN, (uint64_t)DW_DMA_CHAN_MASK);
	dma_clear_bit(dma, CH_EN, (uint64_t)DW_DMA_CHAN_MASK << DW_DMAC_CH_PAUSE_OFFSET);
	dma_clear_bit(dma, CH_EN, (uint64_t)DW_DMA_CHAN_MASK << DW_DMAC_CH_ABORT_OFFSET);
	dma_clear_bit(dma, CH_EN, (uint64_t)DW_DMA_CHAN_MASK << DW_DMAC_CH_EN_WE_OFFSET);
	dma_clear_bit(dma, CH_EN, (uint64_t)DW_DMA_CHAN_MASK << DW_DMAC_CH_PAUSE_EN_OFFSET);
	dma_clear_bit(dma, CH_EN, (uint64_t)DW_DMA_CHAN_MASK << DW_DMAC_CH_ABORT_EN_OFFSET);
}

void hal_dma_reset(dw_dma_t *dma)
{
	dma_writeq(dma, RESET, 1);
}

void hal_sdma_dma_int_mux_set_c906b(void)
{
	*(volatile uint32_t *)SDMA_DMA_INT_MUX = SDMA_DMA_INT_MUX_C906L;
}

/* ---------------------- show debug info -------------------------- */
void hal_print_dma_reg(struct dw_dma *dw)
{
	dma_log("CFG: 0x%lx, CH_EN=0x%lx, INT_STATUS=0x%lx, COMM_INTSTATUS=0x%lx\r\n",
	dma_readq(dw, CFG), dma_readq(dw, CH_EN), dma_readq(dw, INTSTATUS),  dma_readq(dw, COMM_INTSTATUS));
}

void hal_print_ch_info(struct dw_dma *dw, int ch_id)
{
	struct dw_dma_channel *dwc = &dw->chans[ch_id];

	dma_dbg("SAR=%p\r\n", &(_dwc_regs(dwc)->SAR));

	dma_log("Ch%d - SAR:0x%lx DAR:0x%lx ch_status:0x%lx int_status:0x%lx\r\n", ch_id,
		channel_readq(dwc, SAR), channel_readq(dwc, DAR),
		channel_readq(dwc, STATUS), channel_readq(dwc, INTSTATUS));

	dma_log("\tLLP:0x%lx\r\n", channel_readq(dwc, LLP));
}





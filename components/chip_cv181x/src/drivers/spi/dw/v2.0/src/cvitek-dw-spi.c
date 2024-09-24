/*
* Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
*/
#include "cvitek-dw-spi.h"
#include "drv/spi.h"

/* Restart the controller, disable all interrupts, clean rx fifo */
void spi_hw_init(struct dw_spi *dws)
{
	/*
	 * Try to detect the FIFO depth if not set by interface driver,
	 * the depth could be from 2 to 256 from HW spec
	 */
	if (!dws->fifo_len) {
		uint32_t fifo;

		for (fifo = 1; fifo < 256; fifo++) {
			dw_writel(dws, DW_SPI_TXFTLR, fifo);
			if (fifo != dw_readl(dws, DW_SPI_TXFTLR))
				break;
		}
		dw_writel(dws, DW_SPI_TXFTLR, 0);

		dws->fifo_len = (fifo == 1) ? 0 : fifo;

		printf("Detected FIFO size: %u bytes\n", dws->fifo_len);
	}
}

uint32_t min3(uint32_t a, uint32_t b, uint32_t c)
{
	uint32_t tmp;

	tmp = (a < b) ? a : b;
	return (tmp < c) ? tmp : c;
}

static inline void cpu_relax(void)
{
	asm volatile("" ::: "memory");
}

static inline uint32_t tx_max(struct dw_spi *dws)
{
	uint32_t tx_left, tx_room, rxtx_gap, temp;
	cpu_relax();
	tx_left = (dws->tx_end - dws->tx) / dws->n_bytes;
	tx_room = dws->fifo_len - dw_readl(dws, DW_SPI_TXFLR);

	/*
	 * Another concern is about the tx/rx mismatch, we
	 * though to use (dws->fifo_len - rxflr - txflr) as
	 * one maximum value for tx, but it doesn't cover the
	 * data which is out of tx/rx fifo and inside the
	 * shift registers. So a control from sw point of
	 * view is taken.
	 */

	spi_debug("tx left: %#x, tx room: %#x\n", tx_left, tx_room);
	if (dws->rx != NULL && dws->tx != NULL) {
		cpu_relax();
		rxtx_gap =  ((dws->rx_end - dws->rx) - (dws->tx_end - dws->tx)) / dws->n_bytes;
		temp = min3(tx_left, tx_room, (uint32_t) (dws->fifo_len - rxtx_gap));
	} else {
		temp = tx_left < tx_room ? tx_left : tx_room;
	}

	spi_debug("temp: %#x\n", temp);
	return temp;
}

void dw_writer(struct dw_spi *dws)
{
	uint32_t max;
	uint16_t txw = 0;

	max = tx_max(dws);
	spi_debug("max: %#x \n", max);
	while (max) {
		/* Set the tx word if the transfer's original "tx" is not null */
		if (dws->tx_end - dws->len) {
			if (dws->n_bytes == 1)
				txw = *(uint8_t *)(dws->tx);
			else
				txw = *(uint16_t *)(dws->tx);
		}
		dw_writel(dws, DW_SPI_DR, txw);
		dws->tx += dws->n_bytes;
		max--;
	}
}

static inline uint32_t rx_max(struct dw_spi *dws)
{
	uint32_t temp;
	uint32_t rx_left = (dws->rx_end - dws->rx) / dws->n_bytes;
	uint32_t data_in_fifo = dw_readl(dws, DW_SPI_RXFLR);

	temp = (rx_left < data_in_fifo ? rx_left : data_in_fifo);
	spi_debug("temp: %u\n", temp);
	return temp;
}

void dw_reader(struct dw_spi *dws)
{
	uint32_t max;
	uint16_t rxw;

	max = rx_max(dws);
	spi_debug("max: %#x \n", max);
	while (max) {
		rxw = dw_readl(dws, DW_SPI_DR);
		/* Care rx only if the transfer's original "rx" is not null */
		if (dws->rx_end - dws->len) {
			if (dws->n_bytes == 1)
				*(uint8_t *)(dws->rx) = rxw;
			else
				*(uint16_t *)(dws->rx) = rxw;
		}
		dws->rx += dws->n_bytes;
		max--;
	}
}

int poll_transfer(struct dw_spi *dws)
{
	uint32_t end = 0;
	do {
		if (dws->tx != NULL) {
			dw_writer(dws);
			cpu_relax();
			end = dws->tx_end - dws->tx;
		}

		if (dws->rx != NULL) {
			dw_reader(dws);
			cpu_relax();
			end = dws->tx_end - dws->tx;
		}

		if (dws->rx != NULL && dws->tx != NULL) {
			cpu_relax();
			end = dws->rx_end - dws->rx;
		}

	} while (end);
	while(!(spi_get_status(dws) & 0x4));
	return 0;
}


void set_tran_mode(struct dw_spi *dws)
{
	uint32_t reg = dw_readl(dws, DW_SPI_CTRLR0);
	uint8_t tmode;

	if (dws->rx && dws->tx)
		tmode = SPI_TMOD_TR;
	else if (dws->rx)
		tmode = SPI_TMOD_RO;
	else
		tmode = SPI_TMOD_TO;

	reg &= ~SPI_TMOD_MASK;
	reg |= (tmode << SPI_TMOD_OFFSET);

	dw_writel(dws, DW_SPI_CTRLR0, reg);
}

void dw_spi_set_controller_mode(struct dw_spi *dws, uint8_t enable_master)
{
	/* do not support to switch controller mode, it is default master mode */
}

void dw_spi_set_cs(struct dw_spi *dws, bool enable, uint32_t index)
{
	uint32_t reg = dw_readl(dws, DW_SPI_SER);

	if (enable)
		dw_writel(dws, DW_SPI_SER, reg | BIT(index));
	else
		dw_writel(dws, DW_SPI_SER, reg & ~BIT(index));
}

void dw_spi_set_polarity_and_phase(struct dw_spi *dws, csi_spi_cp_format_t format)
{
	uint32_t reg = dw_readl(dws, DW_SPI_CTRLR0);
	reg &= ~(0x3 << 6);

	switch (format) {
		case SPI_FORMAT_CPOL0_CPHA0:
			reg |= (SPI_MODE_0 << 6);
			break;

		case SPI_FORMAT_CPOL0_CPHA1:
			reg |= (SPI_MODE_1 << 6);
			break;

		case SPI_FORMAT_CPOL1_CPHA0:
			reg |= (SPI_MODE_2 << 6);
			break;

		case SPI_FORMAT_CPOL1_CPHA1:
			reg |= (SPI_MODE_3 << 6);
			break;

		default:
			reg = dw_readl(dws, DW_SPI_CTRLR0);
			break;
	}
	spi_debug("set phase and polarity: %#x\n", reg);
	dw_writel(dws, DW_SPI_CTRLR0, reg);
}

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))
void dw_spi_set_clock(struct dw_spi *dws, uint32_t clock_in, uint32_t clock_out)
{
	uint16_t div;

	spi_debug("clock_in: %u, clock_out: %u\n", clock_in, clock_in);

	div = (DIV_ROUND_UP(clock_in, clock_out) + 1) & 0xfffe;
	spi_debug("clk div value is: 0x%x\n", div);
	spi_set_clk(dws, div);
}

int dw_spi_set_data_frame_len(struct dw_spi *dws, uint32_t size)
{
	uint32_t temp = dw_readl(dws, DW_SPI_CTRLR0);
	temp &= ~0xf;

	if (size == 8) {
		dws->n_bytes = 1;
	} else if (size == 16) {
		dws->n_bytes = 2;
	} else {
		printf("do not support %u bit data!\n", size);
		return -1;
	}
	temp |= (size - 1);
	dw_writel(dws, DW_SPI_CTRLR0, temp);
	spi_debug("set data frame len: %#x\n", temp);
	return 0;
}

void dw_spi_show_regs(struct dw_spi *dws)
{
	spi_debug("CTRLR0: \t0x%08x\n", dw_readl(dws, DW_SPI_CTRLR0));
	spi_debug("CTRLR1: \t0x%08x\n", dw_readl(dws, DW_SPI_CTRLR1));
	spi_debug("SSIENR: \t0x%08x\n", dw_readl(dws, DW_SPI_SSIENR));
	spi_debug("SER: \t0x%08x\n", dw_readl(dws, DW_SPI_SER));
	spi_debug("BAUDR: \t0x%08x\n", dw_readl(dws, DW_SPI_BAUDR));
	spi_debug("TXFTLR: \t0x%08x\n", dw_readl(dws, DW_SPI_TXFTLR));
	spi_debug("RXFTLR: \t0x%08x\n", dw_readl(dws, DW_SPI_RXFTLR));
	spi_debug("TXFLR: \t0x%08x\n", dw_readl(dws, DW_SPI_TXFLR));
	spi_debug("RXFLR: \t\t0x%08x\n", dw_readl(dws, DW_SPI_RXFLR));
	spi_debug("SR: \t0x%08x\n", dw_readl(dws, DW_SPI_SR));
	spi_debug("IMR: \t0x%08x\n", dw_readl(dws, DW_SPI_IMR));
	spi_debug("ISR: \t0x%08x\n", dw_readl(dws, DW_SPI_ISR));
	spi_debug("DMACR: \t0x%08x\n", dw_readl(dws, DW_SPI_DMACR));
	spi_debug("DMATDLR: \t0x%08x\n", dw_readl(dws, DW_SPI_DMATDLR));
	spi_debug("DMARDLR: \t0x%08x\n", dw_readl(dws, DW_SPI_DMARDLR));
}

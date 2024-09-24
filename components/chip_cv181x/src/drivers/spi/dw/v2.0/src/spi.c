/*
* Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
*/
#include <string.h>
#include <drv/spi.h>
#include <drv/cvi_irq.h>
#include <drv/tick.h>
#include <drv/porting.h>
#include <drv/common.h>
#include "cvitek-dw-spi.h"
#include "drv/spi.h"
#include "io.h"
#include <drv/dma.h>

struct dw_spi g_dws[MAX_SPI_NUM]= {0};
static int dw_spi_irq(int irq, void *args);

csi_error_t csi_spi_init(csi_spi_t *spi, uint32_t idx)
{
	int ret;
	CSI_PARAM_CHK(spi, CSI_ERROR);
	struct dw_spi *dws = &g_dws[idx];
	memset(dws, 0x0, sizeof(struct dw_spi));

	if (target_get(DEV_DW_SPI_TAG, idx, &spi->dev) != CSI_OK) {
		printf("get dev failed!\n");
		return CSI_ERROR;
	}

	spi->state.writeable = 1U;
	spi->state.readable  = 1U;
	spi->state.error     = 0U;
	spi->send            = NULL;
	spi->receive         = NULL;
	spi->send_receive    = NULL;
	spi->rx_dma          = NULL;
	spi->tx_dma          = NULL;
	spi->rx_data         = NULL;
	spi->tx_data         = NULL;
	spi->callback        = NULL;
	spi->arg             = NULL;
	spi->priv            = (void *)dws;

	csi_dev_t *dev = &spi->dev;
	dws->regs = (void *)dev->reg_base;
	dws->irq = dev->irq_num;
	dws->index = dev->idx;

	dws->tx              = NULL;
	dws->tx_end          = NULL;
	dws->rx              = NULL;
	dws->rx_end          = NULL;

	spi_reset_chip(dws);
	spi_hw_init(dws);
	spi_enable_chip(dws, 0);
	spi_mask_intr(dws, 0xff);
	request_irq((uint32_t)spi->dev.irq_num, dw_spi_irq, 0, "SPI_IRQ", (void *)spi);

	return ret;
}

void csi_spi_uninit(csi_spi_t *spi)
{
	CSI_PARAM_CHK_NORETVAL(spi);
	struct dw_spi *dws = (struct dw_spi *)spi->priv;

	spi_shutdown_chip(dws);
	spi->send            = NULL;
	spi->receive         = NULL;
	spi->send_receive    = NULL;
	spi->rx_data         = NULL;
	spi->tx_data         = NULL;
	spi->callback        = NULL;
	spi->arg             = NULL;

	free(spi->rx_dma);
	free(spi->tx_dma);
}

csi_error_t csi_spi_attach_callback(csi_spi_t *spi, void *callback, void *arg)
{
	CSI_PARAM_CHK(spi, CSI_ERROR);
	CSI_PARAM_CHK(callback, CSI_ERROR);

	spi->callback     = callback;
	spi->arg          = arg;
	spi->send         = NULL;
	spi->receive      = NULL;
	spi->send_receive = NULL;

	return CSI_OK;
}

void csi_spi_detach_callback(csi_spi_t *spi)
{
	CSI_PARAM_CHK_NORETVAL(spi);

	spi->callback     = NULL;
	spi->arg          = NULL;
	spi->send         = NULL;
	spi->receive      = NULL;
	spi->send_receive = NULL;
}

csi_error_t csi_spi_mode(csi_spi_t *spi, csi_spi_mode_t mode)
{
	CSI_PARAM_CHK(spi, CSI_ERROR);
	struct dw_spi *dws = (struct dw_spi *)spi->priv;
	csi_error_t   ret = CSI_OK;

	dw_spi_set_controller_mode(dws, mode);

	return ret;
}

csi_error_t csi_spi_cp_format(csi_spi_t *spi, csi_spi_cp_format_t format)
{
	CSI_PARAM_CHK(spi, CSI_ERROR);
	struct dw_spi *dws = (struct dw_spi *)spi->priv;
	csi_error_t   ret = CSI_OK;

	dw_spi_set_polarity_and_phase(dws, format);

	return ret;
}

uint32_t csi_spi_baud(csi_spi_t *spi, uint32_t baud)
{
	CSI_PARAM_CHK(spi,  CSI_ERROR);
	CSI_PARAM_CHK(baud, CSI_ERROR);
	struct dw_spi *dws = (struct dw_spi *)spi->priv;

	/*need to ensure the clk */
	dw_spi_set_clock(dws, SPI_REF_CLK, baud);
	return baud;
}

csi_error_t csi_spi_frame_len(csi_spi_t *spi, csi_spi_frame_len_t length)
{
	CSI_PARAM_CHK(spi, CSI_ERROR);
	struct dw_spi *dws = (struct dw_spi *)spi->priv;
	csi_error_t   ret = CSI_OK;

	ret = dw_spi_set_data_frame_len(dws, (uint32_t)length);

	return ret;
}

void csi_spi_select_slave(csi_spi_t *spi, uint32_t slave_num)
{
	CSI_PARAM_CHK_NORETVAL(spi);
	struct dw_spi *dws = (struct dw_spi *)spi->priv;

	dw_spi_set_cs(dws, 1, slave_num);
}

static int interrupt_transfer(csi_spi_t *spi)
{
	struct dw_spi *dws = (struct dw_spi *)spi->priv;
	uint16_t irq_status = dw_readl(dws, DW_SPI_ISR);


	/* Error handling */
	if (irq_status & (SPI_INT_TXOI | SPI_INT_RXOI | SPI_INT_RXUI)) {
		dw_readl(dws, DW_SPI_ICR);
		spi_reset_chip(dws);
		printf("interrupt_transfer: fifo overrun/underrun");
		return 0;
	}

	if (dws->rx) {
		dw_reader(dws);
		if (dws->rx_end == dws->rx) {
			spi_mask_intr(dws, SPI_INT_TXEI);

			if (spi->callback && (dws->rx_end == dws->rx))
				spi->callback(spi, SPI_EVENT_RECEIVE_COMPLETE, spi->arg);
			return 0;
		}
	}

	if (irq_status & SPI_INT_TXEI) {
		spi_mask_intr(dws, SPI_INT_TXEI);
		dw_writer(dws);
		while (dw_readl(dws, DW_SPI_SR) & 0x1);
		/* Enable TX irq always, it will be disabled when RX finished */
		spi_umask_intr(dws, SPI_INT_TXEI);

		if (spi->callback && (dws->tx == dws->tx_end)) {
			spi->callback(spi, SPI_EVENT_SEND_COMPLETE, spi->arg);
			spi_mask_intr(dws, SPI_INT_TXEI);
		}

		if (dws->tx == dws->tx_end)
			spi_mask_intr(dws, SPI_INT_TXEI);
	}
	return 0;
}

static int dw_spi_irq(int irq, void *args)
{
	csi_spi_t *spi = (csi_spi_t *)args;
	struct dw_spi *dws = (struct dw_spi *)spi->priv;
	uint32_t irq_status = dw_readl(dws, DW_SPI_ISR) & 0x3f;

	if (!irq_status)
		return 0;

	return dws->transfer_handler(spi);
}

static int dma_transfer(csi_spi_t *spi)
{
	csi_dma_ch_t        *tx_dma_ch, *rx_dma_ch;
	csi_dma_ch_config_t tx_config, rx_config;
	uint8_t             dma_data_width;

	memset(&tx_config, 0, sizeof(csi_dma_ch_config_t));
	memset(&rx_config, 0, sizeof(csi_dma_ch_config_t));
	struct dw_spi *dws = (struct dw_spi *)spi->priv;

	if (dws->n_bytes == 2)
		dma_data_width = DMA_DATA_WIDTH_16_BITS;
	else
		dma_data_width = DMA_DATA_WIDTH_8_BITS;

	if (dws->tx) {
		tx_dma_ch   = (csi_dma_ch_t *)spi->tx_dma;
		/* configure tx dma channel */
		tx_config.src_tw = DMA_DATA_WIDTH_32_BITS;
		tx_config.dst_tw = dma_data_width;
		tx_config.src_inc = DMA_ADDR_INC;
		tx_config.dst_inc = DMA_ADDR_CONSTANT;
		tx_config.group_len = 8;
		tx_config.trans_dir = DMA_MEM2PERH;
		tx_config.handshake = 5; /* dma channel 5 */
		csi_dma_ch_config(tx_dma_ch, &tx_config);
		dw_writel(dws, DW_SPI_DMATDLR, 8);
		spi_enable_dma(dws, 1, 1);
		soc_dcache_clean_invalid_range((unsigned long)dws->tx, dws->len);
	}

	if (dws->rx) {
		/* configure rx dma channel */
		rx_dma_ch   = (csi_dma_ch_t *)spi->rx_dma;
		rx_config.src_tw = dma_data_width;
		rx_config.dst_tw = DMA_DATA_WIDTH_32_BITS;
		rx_config.src_inc = DMA_ADDR_CONSTANT;
		rx_config.dst_inc = DMA_ADDR_INC;
		rx_config.group_len = 8;
		rx_config.trans_dir = DMA_PERH2MEM;
		rx_config.handshake = 4;
		csi_dma_ch_config(rx_dma_ch, &rx_config);
		dw_writel(dws, DW_SPI_DMARDLR, 8 - 1);
		spi_enable_dma(dws, 0, 1);
		soc_dcache_clean_invalid_range((unsigned long)dws->rx, dws->len);
	}
	/* rx must be started before tx due to spi instinct */
	if (dws->rx)
		csi_dma_ch_start(spi->rx_dma, (void *)(dws->regs + DW_SPI_DR), dws->rx, dws->len);

	if (dws->tx)
		csi_dma_ch_start(spi->tx_dma, (void *)dws->tx, (void *)(dws->regs + DW_SPI_DR), dws->len);

	return 0;
}

csi_error_t dw_spi_allocate_txrx_td(csi_spi_t *spi)
{
	struct dw_spi *dws = spi->priv;
	csi_error_t ret = CSI_OK;

	if (dws->tx != NULL) {
		spi->tx_dma = (csi_dma_ch_t *)malloc(sizeof(csi_dma_ch_t));
		if (spi->tx_dma == NULL) {
			printf("malloc tx_dma failed!\n");
			return -1;
		}
		spi->tx_dma->parent = spi;
		ret = csi_dma_ch_alloc(spi->tx_dma, -1, -1);
		if (ret != CSI_OK) {
			printf("dma allocate tx channel failed!\n");
			return ret;
		}
	}

	if (dws->rx != NULL) {
		spi->rx_dma = (csi_dma_ch_t *)malloc(sizeof(csi_dma_ch_t));
		if (spi->rx_dma == NULL) {
			printf("malloc tx_dma failed!\n");
			return -1;
		}
		spi->rx_dma->parent = spi;
		ret = csi_dma_ch_alloc(spi->rx_dma, -1, -1);
		if (ret != CSI_OK) {
			printf("dma allocate rx channel failed!\n");
			return ret;
		}
	}
	return ret;
}

static int dw_spi_transfer_one(csi_spi_t *spi, const void *tx_buf,
		void *rx_buf, uint32_t len, enum transfer_type  tran_type)
{
	struct dw_spi *dws = (struct dw_spi *)spi->priv;
	uint8_t imask = 0;
	uint16_t txlevel = 0;

	dws->tx = NULL;
	dws->tx_end = NULL;
	dws->rx = NULL;
	dws->rx_end = NULL;

	if (tx_buf != NULL) {
		dws->tx = tx_buf;
		dws->tx_end = dws->tx + len;
	}

	if (rx_buf != NULL) {
		dws->rx = rx_buf;
		dws->rx_end = dws->rx + len;
	}
	dws->len = len;

	spi_enable_chip(dws, 0);

	/* For poll mode just disable all interrupts */
	spi_mask_intr(dws, 0xff);

	/* set tran mode */
	set_tran_mode(dws);

	if (tran_type == IRQ_TRAN) {
		/*
		 * Interrupt mode
		 * we only need set the TXEI IRQ, as TX/RX always happen syncronizely
		 */
		if ((dws->fifo_len / 2) < (dws->len / dws->n_bytes))
			txlevel = dws->fifo_len / 2 < dws->len;
		else
			txlevel = dws->len / dws->n_bytes;

		dw_writel(dws, DW_SPI_TXFTLR, txlevel);
		/* Set the interrupt umask */
		imask |= SPI_INT_TXEI | SPI_INT_TXOI | SPI_INT_RXUI | SPI_INT_RXOI;
		spi_umask_intr(dws, imask);
		dws->transfer_handler = interrupt_transfer;
	}

	if (tran_type == DMA_TRAN) {
		/*
		 * if user use spi by hal interfarce, there is no need to allocate
		 * dma transfer descriptor because it was done in csi_spi_link_dma.
		 */
		//dw_spi_allocate_txrx_td(spi);
		dma_transfer(spi);
	}

	/* enable spi */
	spi_enable_chip(dws, 1);
	dw_spi_show_regs(dws);

	if (tran_type == POLL_TRAN)
		return poll_transfer(dws);

	return 0;
}

static csi_error_t wait_ready_until_timeout(csi_spi_t *spi, uint32_t timeout)
{
	uint32_t timestart = 0U;
	int ret;
	struct dw_spi *dw = (struct dw_spi *)spi->priv;

	timestart = csi_tick_get_ms();

	while (dw_readl(dw, DW_SPI_SR) & 0x1) {
		if ((csi_tick_get_ms() - timestart) > timeout) {
			ret = CSI_TIMEOUT;
			break;
		}
	}
	return ret;
}

int32_t csi_spi_send(csi_spi_t *spi, const void *data, uint32_t size, uint32_t timeout)
{
	CSI_PARAM_CHK(spi,  CSI_ERROR);
	CSI_PARAM_CHK(data, CSI_ERROR);
	CSI_PARAM_CHK(size, CSI_ERROR);
	int32_t  ret   = CSI_OK;

	ret = dw_spi_transfer_one(spi, data, NULL, size, POLL_TRAN);

	return ret;
}

int32_t csi_spi_send_async(csi_spi_t *spi, const void *data, uint32_t size)
{
	CSI_PARAM_CHK(spi,  CSI_ERROR);
	CSI_PARAM_CHK(data, CSI_ERROR);
	CSI_PARAM_CHK(size, CSI_ERROR);
	int32_t  ret   = CSI_OK;

	ret = dw_spi_transfer_one(spi, data, NULL, size, IRQ_TRAN);

	return ret;
}


int32_t csi_spi_receive(csi_spi_t *spi, void *data, uint32_t size, uint32_t timeout)
{
	CSI_PARAM_CHK(spi,  CSI_ERROR);
	CSI_PARAM_CHK(data, CSI_ERROR);
	CSI_PARAM_CHK(size, CSI_ERROR);
	int32_t  ret = CSI_OK;

	ret = dw_spi_transfer_one(spi, NULL, data, size, POLL_TRAN);

	return ret;
}

int32_t csi_spi_receive_async(csi_spi_t *spi, void *data, uint32_t size)
{
	CSI_PARAM_CHK(spi,  CSI_ERROR);
	CSI_PARAM_CHK(data, CSI_ERROR);
	CSI_PARAM_CHK(size, CSI_ERROR);
	int32_t  ret = CSI_OK;

	ret = dw_spi_transfer_one(spi, NULL, data, size, IRQ_TRAN);

	return ret;
}

int32_t csi_spi_send_dma(csi_spi_t *spi, const void *data, uint32_t size)
{
	CSI_PARAM_CHK(spi,  CSI_ERROR);
	CSI_PARAM_CHK(data, CSI_ERROR);
	CSI_PARAM_CHK(size, CSI_ERROR);
	int32_t  ret   = CSI_OK;

	if (size > 16)
		ret = dw_spi_transfer_one(spi, data, NULL, size, DMA_TRAN);
	else
		ret = dw_spi_transfer_one(spi, data, NULL, size, POLL_TRAN);

	return ret;
}


int32_t csi_spi_receive_dma(csi_spi_t *spi, void *data, uint32_t size)
{
	CSI_PARAM_CHK(spi,  CSI_ERROR);
	CSI_PARAM_CHK(data, CSI_ERROR);
	CSI_PARAM_CHK(size, CSI_ERROR);
	int32_t  ret = CSI_OK;

	if (size > 16)
		ret = dw_spi_transfer_one(spi, data, NULL, size, DMA_TRAN);
	else
		ret = dw_spi_transfer_one(spi, data, NULL, size, POLL_TRAN);

	return ret;
}

int32_t csi_spi_send_receive(csi_spi_t *spi, const void *data_out,
		void *data_in, uint32_t size, uint32_t timeout)
{
	CSI_PARAM_CHK(spi,  CSI_ERROR);
	CSI_PARAM_CHK(data_out, CSI_ERROR);
	CSI_PARAM_CHK(data_in, CSI_ERROR);
	CSI_PARAM_CHK(size, CSI_ERROR);
	int32_t  ret   = CSI_OK;

	ret = dw_spi_transfer_one(spi, data_out, data_in, size, POLL_TRAN);

	return ret;
}

int32_t csi_spi_send_receive_asyn(csi_spi_t *spi, const void *data_out,
		void *data_in, uint32_t size, uint32_t timeout)
{
	CSI_PARAM_CHK(spi,  CSI_ERROR);
	CSI_PARAM_CHK(data_out, CSI_ERROR);
	CSI_PARAM_CHK(data_in, CSI_ERROR);
	CSI_PARAM_CHK(size, CSI_ERROR);
	int32_t  ret   = CSI_OK;

	ret = dw_spi_transfer_one(spi, data_out, data_in, size, IRQ_TRAN);

	return ret;
}

int32_t csi_spi_send_receive_dma(csi_spi_t *spi, const void *data_out,
				 void *data_in, uint32_t size)
{
	CSI_PARAM_CHK(spi,  CSI_ERROR);
	CSI_PARAM_CHK(data_out, CSI_ERROR);
	CSI_PARAM_CHK(data_in, CSI_ERROR);
	CSI_PARAM_CHK(size, CSI_ERROR);
	int32_t  ret   = CSI_OK;

	if (size > 16)
		ret = dw_spi_transfer_one(spi, data_out, data_in, size, DMA_TRAN);
	else
		ret = dw_spi_transfer_one(spi, data_out, data_in, size, POLL_TRAN);

	return ret;
}

static void dw_spi_dma_event_cb(csi_dma_ch_t *dma, csi_dma_event_t event, void *arg)
{
	csi_spi_t *spi = (csi_spi_t *)dma->parent;
	uint32_t mode;

	struct dw_spi *dws = spi->priv;
	/*  00 -- send and receive ==> SPI_EVENT_SEND_RECEIVE_COMPLETE
	 *  01 -- send             ==> SPI_EVENT_SEND_COMPLETE
	 *  02 -- receive          ==> SPI_EVENT_RECEIVE_COMPLETE
	 */
	mode = (dw_readl(dws, DW_SPI_CTRLR0) >> 8 & 0x3);
	if (event == DMA_EVENT_TRANSFER_DONE) {
		csi_dma_ch_stop(dma);
		/* process end of transmit */
		if ((spi->tx_dma != NULL) && (spi->tx_dma->ch_id == dma->ch_id)) {
			if (wait_ready_until_timeout(spi, 200) != CSI_OK)
				mode = SPI_EVENT_ERROR;
		}

		/* disable all the dma op */
		dw_writel(dws, DW_SPI_DMACR, 0);
		if (spi->callback)
			spi->callback(spi, mode, spi->arg);
	}
}

csi_error_t csi_spi_link_dma(csi_spi_t *spi, csi_dma_ch_t *tx_dma, csi_dma_ch_t *rx_dma)
{
	CSI_PARAM_CHK(spi, CSI_ERROR);
	csi_error_t ret = CSI_OK;

	ret = dw_spi_allocate_txrx_td(spi);
	if (ret != CSI_OK) {
		printf("allocate tx or rx td failed!\n");
		return ret;
	}

	if (spi->tx_dma != NULL) {
		spi->send = csi_spi_send_dma;
		csi_dma_ch_attach_callback(spi->tx_dma, dw_spi_dma_event_cb, NULL);
	}

	if (spi->rx_dma != NULL) {
		spi->receive = csi_spi_receive_dma;
		csi_dma_ch_attach_callback(spi->rx_dma, dw_spi_dma_event_cb, NULL);
	}

	if ((spi->tx_dma != NULL) && (spi->rx_dma != NULL))
		spi->send_receive = csi_spi_send_receive_dma;

	return ret;
}


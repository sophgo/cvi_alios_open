/*
* Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
*/
#include "cvitek_spi_nor.h"
#include "drv/common.h"
#include <csi_core.h>
#ifndef CONFIG_KERNEL_NONE
#include "aos/kernel.h"
#endif
#include "soc.h"

#ifdef CONFIG_DMA_SUPPORT
#define DMA_POLL_TIMEOUT_US 100000
#endif

int check_irq_status(struct cvi_spif *spif, uint8_t status, uint32_t timeout)
{
	uint32_t wait = 0;
	uint8_t reg;
	do {
		reg = readb(spif->io_base + REG_SPI_INT_STS);
		if (reg & status)
			return 0;
		udelay(10);
		wait++;
	} while(wait < timeout);

	printf("[%s, %d]:check status timeout\n", __func__, __LINE__);
	return -1;
}

static int cvi_spif_wait_for_tran_done(struct cvi_spif *spif)
{
	uint8_t stat;

	/* 300 ms */
	if (check_irq_status(spif, BIT_SPI_INT_TRAN_DONE, 300 * 1000 / 10)) {
		printf("there is no interrupt for writing fifo, init status:0x%x\n", readb(spif->io_base + REG_SPI_INT_STS));
		return -1;
	}

	stat = readb(spif->io_base + REG_SPI_INT_STS);
	stat &= ~BIT_SPI_INT_TRAN_DONE;
	writeb(stat, spif->io_base + REG_SPI_INT_STS);

	stat = readb(spif->io_base + REG_SPI_INT_STS);
	stat &= ~BIT_SPI_INT_RD_FIFO;
	writeb(stat, spif->io_base + REG_SPI_INT_STS);

	return 0;
}

static uint8_t cvi_spi_data_out_tran(struct cvi_spif *spif, const uint8_t *src_buf,
		uint32_t data_bytes, uint8_t bus_width)
{
	uint32_t tran_csr = 0;
	uint32_t xfer_size, off;
	int i;
	int ret = 0;
	uint32_t wait;

	if (data_bytes > 65535) {
		printf("data out overflow, should be less than 65535 bytes(%d)\n", data_bytes);
		return -1;
	}

	writel(0, spif->io_base + REG_SPI_INT_STS);
	writel(BIT_SPI_INT_TRAN_DONE, spif->io_base + REG_SPI_INT_EN);
	/* init tran_csr */
	tran_csr = readw(spif->io_base + REG_SPI_TRAN_CSR);
	tran_csr &= ~(BIT_SPI_TRAN_CSR_TRAN_MODE_MASK
			| BIT_SPI_TRAN_CSR_ADDR_BYTES_MASK
			| BIT_SPI_TRAN_CSR_FIFO_TRG_LVL_MASK
			| BIT_SPI_TRAN_CSR_BUS_WIDTH_MASK
			| BIT_SPI_TRAN_CSR_WITH_CMD);
	tran_csr |= BIT_SPI_TRAN_CSR_FIFO_TRG_LVL_8_BYTE;
	tran_csr |= BIT_SPI_TRAN_CSR_TRAN_MODE_TX;

	/* flush fifo before filling fifo */
	writel(0, spif->io_base + REG_SPI_FIFO_PT);

	/* issue tran */
	writew(data_bytes, spif->io_base + REG_SPI_TRAN_NUM);
	tran_csr |= BIT_SPI_TRAN_CSR_GO_BUSY;
	tran_csr |= (bus_width / 2) << 4;

	writew(tran_csr, spif->io_base + REG_SPI_TRAN_CSR);
	/* 300 ms */
	if (check_irq_status(spif, BIT_SPI_INT_WR_FIFO, 300 * 1000 / 10)) {
		printf("there is no interrupt for writing fifo, init status:0x%x\n", readb(spif->io_base + REG_SPI_INT_STS));
		return -1;
	}

	/* fill data */
	off = 0;
	while (off < data_bytes) {
		if (data_bytes - off >= SPI_MAX_FIFO_DEPTH)
			xfer_size = SPI_MAX_FIFO_DEPTH;
		else
			xfer_size = data_bytes - off;

		wait = 0;
		while (((readb(spif->io_base + REG_SPI_FIFO_PT) & 0xf) != 0)) {
			wait++;
			udelay(10);
			if (wait > 30000) { // 300ms
				printf("wait to write FIFO timeout\n");
				return -1;
			}
		}

		for (i = 0; i < xfer_size; i++)
			writeb(*(src_buf + off + i), spif->io_base + REG_SPI_FIFO_PORT);

		off += xfer_size;
	}

	/* wait tran done */
	ret = cvi_spif_wait_for_tran_done(spif);
	if (ret)
		return ret;

	writel(0, spif->io_base + REG_SPI_FIFO_PT);

	/* clear interrupts */
	writeb(readb(spif->io_base + REG_SPI_INT_STS) & ~BIT_SPI_INT_TRAN_DONE, spif->io_base + REG_SPI_INT_STS);
	writeb(readb(spif->io_base + REG_SPI_INT_STS) & ~BIT_SPI_INT_WR_FIFO, spif->io_base + REG_SPI_INT_STS);

	return 0;
}

static int cvi_spi_data_in_tran(struct cvi_spif *spif, uint8_t *dst_buf,
		int data_bytes, uint8_t bus_width)
{
	uint32_t tran_csr = 0;
	int i, xfer_size, off;
	int ret;

	if (data_bytes > 65535) {
		return -1;
	}

	writel(0, spif->io_base + REG_SPI_INT_STS);
	writel(BIT_SPI_INT_TRAN_DONE, spif->io_base + REG_SPI_INT_EN);

	/* init tran_csr */
	tran_csr = readw(spif->io_base + REG_SPI_TRAN_CSR);
	tran_csr &= ~(BIT_SPI_TRAN_CSR_TRAN_MODE_MASK
			| BIT_SPI_TRAN_CSR_ADDR_BYTES_MASK
			| BIT_SPI_TRAN_CSR_FIFO_TRG_LVL_MASK
			| BIT_SPI_TRAN_CSR_BUS_WIDTH_MASK
			| BIT_SPI_TRAN_CSR_WITH_CMD);
	tran_csr |= BIT_SPI_TRAN_CSR_FIFO_TRG_LVL_8_BYTE;
	tran_csr |= BIT_SPI_TRAN_CSR_TRAN_MODE_RX;

	writel(0, spif->io_base + REG_SPI_FIFO_PT);

	/* issue tran */
	writew(data_bytes, spif->io_base + REG_SPI_TRAN_NUM);
	tran_csr |= BIT_SPI_TRAN_CSR_GO_BUSY;
	tran_csr |= (bus_width / 2) << 4;
	writew(tran_csr, spif->io_base + REG_SPI_TRAN_CSR);

	if (check_irq_status(spif, (BIT_SPI_INT_RD_FIFO | BIT_SPI_INT_TRAN_DONE), 300 * 10000 / 10)) {
		printf("there is no rd fifo and tran done interrupt, val:0x%x\n", readb(spif->io_base + REG_SPI_INT_STS));
		return -1;
	}

	/* get data */
	off = 0;
	while (off < data_bytes) {
		if (data_bytes - off >= SPI_MAX_FIFO_DEPTH)
			xfer_size = SPI_MAX_FIFO_DEPTH;
		else
			xfer_size = data_bytes - off;

		while ((readb(spif->io_base + REG_SPI_FIFO_PT) & 0xF) < xfer_size);
		for (i = 0; i < xfer_size; i++)
			*(dst_buf + off + i) = readb(spif->io_base + REG_SPI_FIFO_PORT);

		off += xfer_size;
	}
	ret = cvi_spif_wait_for_tran_done(spif);
	if (ret)
		return ret;

	writeb(0, spif->io_base + REG_SPI_FIFO_PT); // flush unwanted data

	/* clear interrupts */
	writeb(readb(spif->io_base + REG_SPI_INT_STS) & ~BIT_SPI_INT_TRAN_DONE, spif->io_base + REG_SPI_INT_STS);
	writeb(readb(spif->io_base + REG_SPI_INT_STS) & ~BIT_SPI_INT_RD_FIFO, spif->io_base + REG_SPI_INT_STS);
	return 0;
}

static unsigned int match_value_for_read(uint8_t opcode)
{
	int i;
	uint32_t val;
	const struct dmmr_reg_t dmmr_reg_set[] = {
		{0x03, 0x0, 0x003B81},
		{0x0B, 0x8, 0x003B89},
		{0x3B, 0x8, 0x003B91},
		{0xBB, 0x4, 0x003B99},
		{0x6B, 0x8, 0x003BA1},
		{0xEB, 0x6, 0x003BA9},
		{0x13, 0x0, 0x303C81},
		{0x0C, 0x8, 0x303C89},
		{0x3C, 0x8, 0x303C91},
		{0xBC, 0x4, 0x303C99},
		{0x6C, 0x8, 0x303CA1},
		{0xEC, 0x6, 0x303CA9},
		{0x0, 0x0, 0x0}
	};

	for (i = 0; i < sizeof(dmmr_reg_set) / sizeof(struct dmmr_reg_t); i++) {
		if (opcode == dmmr_reg_set[i].read_cmd) {
			val = dmmr_reg_set[i].reg_set & (~SPI_TRAN_CSR_DUMMY_MASK);
			val |= (dmmr_reg_set[i].dummy_clock & 0xf) << 16;
			return val;
		}
	}
	return dmmr_reg_set[0].reg_set;
}

static void cvi_spif_set_memory_mode_on(struct cvi_spif *spif,
		spi_tran_conf_t *read)
{
	uint32_t reg = match_value_for_read(read->cmd.opcode);
	if ((reg >> 16 & 0xf) != read->dummy.clks) {
		reg &= ~SPI_TRAN_CSR_DUMMY_MASK;
		reg |= (read->dummy.clks & 0xf) << 16;
	}

	writel(0, spif->io_base + REG_SPI_DMMR);
	spif->org_tran_csr = readl(spif->io_base + REG_SPI_TRAN_CSR);

	/* support only 1 ce */
	writel(0, spif->io_base + REG_SPI_CE_CTRL);
	writel(reg, spif->io_base + REG_SPI_TRAN_CSR);
	writel(1, spif->io_base + REG_SPI_DMMR);
}

static void cvi_spif_set_memory_mode_off(struct cvi_spif *spif)
{
	writel(0, spif->io_base + REG_SPI_DMMR);
	writel(0x3, spif->io_base + REG_SPI_CE_CTRL);
	writel(spif->org_tran_csr, spif->io_base + REG_SPI_TRAN_CSR);
}

#ifdef CONFIG_DMA_SUPPORT

/*
 * Poll DMA channel enable register to wait for transfer completion.
 * When CH_EN bit clears, all AXI transactions are done and data is in DDR.
 */
static int dma_poll_wait_done(csi_dma_ch_t *ch_hd, uint32_t timeout_us)
{
	uint32_t waited = 0;
	uint8_t ch_mask = 1 << ch_hd->ch_id;

	while (waited < timeout_us) {
		if ((readl((void *)(0x4330000 + 0x18)) & ch_mask) == 0)
			return 0;
		udelay(1);
		waited++;
	}
	return -1;
}

/*
 * cvi_spif_direct_read_execute - DMA read from DMMR-mapped flash to DDR.
 * @spif: SPI flash controller context
 * @buf:  destination buffer in DDR
 * @from: source address (XIP-mapped flash address, DMMR must be ON)
 * @len:  byte count
 * @read: read operation config (for DMMR register setup)
 *
 * The scheduler is suspended ONLY across the narrow DMMR-critical window:
 *   DMMR=1 -> DMA start -> DMA done -> DMMR=0
 *
 * DMA channel alloc/config are done BEFORE suspending (they may block on a
 * mutex held by another task; blocking with the scheduler suspended would
 * deadlock).  fence/dcache_invalid/stop/free are done AFTER resuming (they
 * are slow and must not starve VI or other real-time tasks).
 */
static int cvi_spif_direct_read_execute(struct cvi_spif *spif, void *buf,
		void *from, uint32_t len, spi_tran_conf_t *read)
{
	csi_dma_ch_t ch_hd;
	csi_dma_ch_config_t *dma_config = &spif->dma_chan_config;
	int ret;

	memset(dma_config, 0x0, sizeof(csi_dma_ch_config_t));
	memset(&ch_hd, 0x0, sizeof(csi_dma_ch_t));

	dma_config->src_inc = DMA_ADDR_INC;
	dma_config->dst_inc = DMA_ADDR_INC;
	dma_config->src_tw = DMA_DATA_WIDTH_32_BITS;
	dma_config->dst_tw = DMA_DATA_WIDTH_32_BITS;
	dma_config->trans_dir = DMA_MEM2MEM;
	dma_config->group_len = 0x8000;

	/* Phase 1: alloc/config - may block, must be outside sched_suspend */
	ret = csi_dma_ch_alloc(&ch_hd, -1, -1);
	if (ret) {
		printf("request dma channel failed!, ret:%d\n", ret);
		return ret;
	}

	ret = csi_dma_ch_config(&ch_hd, dma_config);
	if (ret) {
		printf(" dma channel config failed!\n");
		csi_dma_ch_free(&ch_hd);
		return ret;
	}

	/* Phase 2: DMMR-critical window - no blocking allowed */
	aos_kernel_sched_suspend();
	cvi_spif_set_memory_mode_on(spif, read);
	csi_dma_ch_start(&ch_hd, from, buf, len);
	ret = dma_poll_wait_done(&ch_hd, DMA_POLL_TIMEOUT_US);
	cvi_spif_set_memory_mode_off(spif);
	aos_kernel_sched_resume();

	if (ret) {
		printf("DMA poll timeout!\n");
		csi_dma_ch_stop(&ch_hd);
		csi_dma_ch_free(&ch_hd);
		return ret;
	}

	/* Phase 3: post-DMA cleanup - outside sched_suspend */
	__asm__ volatile("fence iorw, iorw" ::: "memory");
	/*
	 * Invalidate ONLY the destination buffer.
	 * DMA has written fresh data for [buf, buf+len) to DDR; any
	 * CPU-cached lines covering that range are now stale.
	 *
	 * csi_dcache_invalid() (global) must NOT be used here: it discards
	 * ALL dirty cache lines without writeback, silently losing in-flight
	 * stack/heap writes of every other task and causing corruption at the
	 * next context switch.
	 */
	csi_dcache_invalid_range((uint64_t *)buf, (int64_t)len);

	csi_dma_ch_stop(&ch_hd);
	csi_dma_ch_free(&ch_hd);
	return 0;
}
#endif

void memcpy_fromio(void *to, const volatile void *from, long count)
{
	/* Optimize co-aligned transfers.  Everything else gets handled
	   a byte at a time. */

	if (count >= 8 && ((uint64_t)to & 7) == ((uint64_t)from & 7)) {
		count -= 8;
		do {
			*(uint64_t *)to = readq(from);
			count -= 8;
			to += 8;
			from += 8;
		} while (count >= 0);
		count += 8;
	}

	if (count >= 4 && ((uint64_t)to & 3) == ((uint64_t)from & 3)) {
		count -= 4;
		do {
			*(uint32_t *)to = readl(from);
			count -= 4;
			to += 4;
			from += 4;
		} while (count >= 0);
		count += 4;
	}

	if (count >= 2 && ((uint64_t)to & 1) == ((uint64_t)from & 1)) {
		count -= 2;
		do {
			*(uint16_t *)to = readw(from);
			count -= 2;
			to += 2;
			from += 2;
		} while (count >= 0);
		count += 2;
	}

	while (count > 0) {
		*(uint8_t *) to = readb(from);
		count--;
		to++;
		from++;
	}
}

int cvi_spif_clk_setup(struct cvi_spif *spif, uint32_t sck_div)
{
	uint32_t reg, sck_div_backup;

	writel(0, spif->io_base + REG_SPI_DMMR);

	/* set positive edge sampling if SCK <= 30MHz */
	if (sck_div >= 4) {
		reg = readl(spif->io_base + REG_SPI_DLY_CTRL);
		reg &= ~BIT(14);
	} else { /* set negative edge sampling otherwise */
		reg = readl(spif->io_base + REG_SPI_DLY_CTRL);
		reg |= BIT(14);
	}
	writel(reg, spif->io_base + REG_SPI_DLY_CTRL);

	/* set spinor clock divider */
	reg = readl(spif->io_base + REG_SPI_CTRL);
	sck_div_backup = reg & BIT_SPI_CTRL_SCK_DIV_MASK;
	reg &= ~BIT_SPI_CTRL_SCK_DIV_MASK;
	reg |= sck_div;
	writel(reg, spif->io_base + REG_SPI_CTRL);
	return sck_div_backup;
}

int cvi_spif_read_reg(struct spi_nor *nor, uint8_t opcode, uint8_t *buf, int len)
{
	uint8_t bus_width;
	uint32_t sck_div_orig;
	struct cvi_spif *spif = nor->priv;
	uint8_t cmd[5] = {0};

	/* always 1 bit bus width for cmd */
	bus_width = 1;

	/* set clock to 30MHz for no-addr cmd */
	sck_div_orig = cvi_spif_clk_setup(spif, 4);

	writel(0, spif->io_base + REG_SPI_DMMR);
	writel(0x2, spif->io_base + REG_SPI_CE_CTRL);

	cmd[0] = opcode;
	/* 0x4b (Read Unique ID, cmd+4 dummy) and 0x5a (RDSFDP, cmd+3 addr+1
	 * dummy) both send 5 bytes before reading, so share the same path.
	 */
	if (cmd[0] == 0x4b || cmd[0] == CVI_SPINOR_OP_RDSFDP)
		cvi_spi_data_out_tran(spif, cmd, 5, bus_width);
	else
		cvi_spi_data_out_tran(spif, &cmd[0], 1, bus_width);

	cvi_spi_data_in_tran(spif, buf, len, bus_width);

	writel(0x3, spif->io_base + REG_SPI_CE_CTRL);
	/* restore higher speed */
	cvi_spif_clk_setup(spif, sck_div_orig);

	return 0;
}

int cvi_spif_write_reg(struct spi_nor *nor, uint8_t opcode, uint8_t *buf, int len)
{
	uint8_t bus_width;
	uint32_t sck_div_orig;
	struct cvi_spif *spif = nor->priv;

	/* always 1 bit bus width */
	bus_width = 1;

	/* set clock to 30MHz for no-addr cmd */
	sck_div_orig = cvi_spif_clk_setup(spif, 4);

	writel(0x2, spif->io_base + REG_SPI_CE_CTRL);

	cvi_spi_data_out_tran(spif, &opcode, 1, bus_width);

	if (len)
		cvi_spi_data_out_tran(spif, buf, len, bus_width);

	writel(0x3, spif->io_base + REG_SPI_CE_CTRL);

	/* restore higher speed */
	cvi_spif_clk_setup(spif, sck_div_orig);

	return 0;
}

int cvi_spif_read(struct spi_nor *nor, uint64_t from, uint32_t len, void *buf)
{
	struct cvi_spif *spif = nor->priv;
	spi_tran_conf_t *read = &nor->read_op;

#ifdef CONFIG_DMA_SUPPORT
    /*
     * DMA path requires buf to be 64-byte (cache-line) aligned.
     * csi_dcache_invalid_range() invalidates whole cache lines; if buf
     * shares a cache line with adjacent data, that adjacent data's dirty
     * cache line would be discarded without writeback, causing corruption.
     * Fall through to the safe CPU memcpy path for unaligned buffers and length not 64-byte aligned.
     */
    if (len > 1024) {
        if (((uintptr_t)buf & 0x3f) == 0 && (len & 0x3f) == 0) {
            int ret = cvi_spif_direct_read_execute(spif, buf, spif->io_base + from, len, read);
            return ret ? ret : (int)len;
        } else {
            printf("buf or length is not 64-byte aligned, use CPU memcpy path instead of DMA path\n");
        }
    }
#endif

	/*
	 * CPU memcpy path (len <= 1024): suspend the scheduler across the
	 * entire DMMR window so no other task can clear DMMR mid-copy.
	 * This path is fast (<< 1 ms), so the scheduling pause is negligible.
	 */
#ifndef CONFIG_KERNEL_NONE
	aos_kernel_sched_suspend();
#endif
	cvi_spif_set_memory_mode_on(spif, read);
	memcpy_fromio(buf, spif->io_base + from, len);
	cvi_spif_set_memory_mode_off(spif);
#ifndef CONFIG_KERNEL_NONE
	aos_kernel_sched_resume();
#endif

	return (int)len;
}

void spi_flash_addr(uint32_t addr, uint8_t *cmd, uint8_t len)
{
	int i;

	for (i = len - 1; i >= 0; i--) {
		cmd[i] = addr & 0xff;
		addr >>= 8;
	}
}

static int cvi_spif_nor_write(struct spi_nor *nor, spi_tran_conf_t *write_op, uint32_t to)
{
	uint8_t cmd[SPI_FLASH_BUF_LEN];
	struct cvi_spif *spif = nor->priv;
	uint8_t bus_width;

	cmd[0] = write_op->cmd.opcode;

	spi_flash_addr(to, cmd + 1, write_op->addr.nbytes);
	/* cmd */
	bus_width = 1;
	cvi_spi_data_out_tran(spif, cmd, 1, bus_width);

	/* addr */
	bus_width = write_op->addr.buswidth;
	cvi_spi_data_out_tran(spif, cmd + 1, write_op->addr.nbytes, bus_width);

	/* data */
	if (write_op->data.nbytes) {
		bus_width = write_op->data.buswidth;
		cvi_spi_data_out_tran(spif, write_op->data.buf.out, write_op->data.nbytes, bus_width);
	}
	return 0;
}

int cvi_spif_write(struct spi_nor *nor, uint32_t to, uint32_t len, const void *buf)
{
	struct cvi_spif *spif = nor->priv;
	spi_tran_conf_t *write_op = &nor->write_op;

	write_op->data.buf.out = buf;
	write_op->data.nbytes = len;

	writel(0, spif->io_base + REG_SPI_DMMR);

	writel(0x2, spif->io_base + REG_SPI_CE_CTRL);

	cvi_spif_nor_write(nor, write_op, to);

	writel(0x3, spif->io_base + REG_SPI_CE_CTRL);

	return (int)len;
}

extern int cvi_spif_erase(struct spi_nor *nor, uint32_t to, uint32_t len);

int cvi_spif_setup_flash(struct cvi_spif *spif)
{
	spif->nor.read  = cvi_spif_read;
	spif->nor.write = cvi_spif_write;
	spif->nor.read_reg  = cvi_spif_read_reg;
	spif->nor.write_reg = cvi_spif_write_reg;
	spif->nor.erase  = cvi_spif_erase;
	spif->nor.priv  = spif;
	return 0;
}


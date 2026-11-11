#include <drv/tick.h>
#include <drv/spinand.h>
#include <cvi_spinand.h>

#define CVI_DEVICE_TX_RX_TIMEOUT        0x2000U
#define BASE_SPI_NAND   (void*)0x04060000

csi_dma_ch_t ch_hd_g;

static void cvi_spinand_dma_ch_callback(csi_dma_ch_t *dma_ch, csi_dma_event_t event, void *arg)
{
	if (event != DMA_EVENT_TRANSFER_DONE) {
        spinand_printf(SPINAND_ERR, "dma error event=0x%x\n", event);
	}

	csi_dma_ch_stop(dma_ch);
	csi_dma_ch_detach_callback(dma_ch);
	csi_dma_ch_free(dma_ch);
}

static int cvi_spinand_dma_setup(void *buf, uint32_t len, uint32_t rw)
{
	csi_dma_ch_config_t dma_config;
	int ret;
	memset(&dma_config, 0x0, sizeof(csi_dma_ch_config_t));
	memset(&ch_hd_g, 0x0, sizeof(csi_dma_ch_t));
    if(rw){
        dma_config.src_inc = DMA_ADDR_INC;
        dma_config.dst_inc = DMA_ADDR_CONSTANT;
        dma_config.trans_dir = DMA_MEM2PERH;
    }else{
        dma_config.src_inc = DMA_ADDR_CONSTANT;
        dma_config.dst_inc = DMA_ADDR_INC;
        dma_config.trans_dir = DMA_PERH2MEM;
    }
	dma_config.src_tw = DMA_DATA_WIDTH_32_BITS;
	dma_config.dst_tw = DMA_DATA_WIDTH_32_BITS;
    dma_config.handshake = 7;
	/* 32K */
	dma_config.group_len = 4;
	ret = csi_dma_ch_alloc(&ch_hd_g, 7, 0);
	if (ret) {
		spinand_printf(SPINAND_ERR, "request dma channel failed!, ret:%d\n", ret);
		goto ch_free;
	}
	ret = csi_dma_ch_config(&ch_hd_g, &dma_config);
	if (ret) {
		spinand_printf(SPINAND_ERR, " dma channel config failed!\n");
		goto ch_free;
    }
	ret = csi_dma_ch_attach_callback(&ch_hd_g, cvi_spinand_dma_ch_callback, NULL);
	if (ret) {
		spinand_printf(SPINAND_ERR, "attach dma channel failed!\n");
		goto detach;
	}
    if(rw){
        csi_dma_ch_start(&ch_hd_g, buf, BASE_SPI_NAND+0x800, len);
    }else{
        csi_dma_ch_start(&ch_hd_g, BASE_SPI_NAND+0xc00, buf, len);
    }
    return ret;
//     while(!g_qspi_xfrer_stat){

//     }

	csi_dma_ch_stop(&ch_hd_g);
detach:
	csi_dma_ch_detach_callback(&ch_hd_g);
ch_free:
	csi_dma_ch_free(&ch_hd_g);

    return ret;
}

void debug_printf(csi_spinand_t *spinand, spi_mem_op_t *op, spinand_reg_t *spinand_reg_config)
{
    spinand_msg_prio_e level_temp = SPINAND_PRINTK_LEVEL;
    SPINAND_PRINTK_LEVEL = SPINAND_DBG;
    spinand_printf(SPINAND_DBG, "=================================================\n");
    spinand_printf(SPINAND_DBG, "op->cmd.buswidth = 0x%x\n", op->cmd.buswidth);
    spinand_printf(SPINAND_DBG, "op->cmd.opcode = 0x%x\n", op->cmd.opcode);
    spinand_printf(SPINAND_DBG, "op->addr.buswidth = 0x%x\n", op->addr.buswidth);
    spinand_printf(SPINAND_DBG, "op->addr.nbytes = 0x%x\n", op->addr.nbytes);
    spinand_printf(SPINAND_DBG, "op->addr.val = 0x%lx\n", op->addr.val);
    spinand_printf(SPINAND_DBG, "op->dummy.nbytes = 0x%x\n", op->dummy.nbytes);
    spinand_printf(SPINAND_DBG, "op->dummy.buswidth = 0x%x\n", op->dummy.buswidth);
    spinand_printf(SPINAND_DBG, "op->data.buswidth = 0x%x\n", op->data.buswidth);
    spinand_printf(SPINAND_DBG, "op->data.nbytes = 0x%x\n", op->data.nbytes);
    spinand_printf(SPINAND_DBG, "op->data.dir = 0x%x\n", op->data.dir);
    spinand_printf(SPINAND_DBG, "spinand->scractbuf[0] = 0x%x\n", spinand->scractbuf[0]);
    spinand_printf(SPINAND_DBG, "spinand->scractbuf[1] = 0x%x\n", spinand->scractbuf[1]);
    spinand_printf(SPINAND_DBG, "spinand->scractbuf[2] = 0x%x\n", spinand->scractbuf[2]);
    spinand_printf(SPINAND_DBG, "spinand->scractbuf[3] = 0x%x\n", spinand->scractbuf[3]);
    spinand_printf(SPINAND_DBG, "spinand_reg_config->reg_trx_size.raw = 0x%x\n", spinand_reg_config->reg_trx_size.raw);
    spinand_printf(SPINAND_DBG, "spinand_reg_config->reg_mode_ctrl0.raw = 0x%x\n", spinand_reg_config->reg_mode_ctrl0.raw);
    spinand_printf(SPINAND_DBG, "spinand_reg_config->reg_cont0.raw = 0x%x\n", spinand_reg_config->reg_cont0.raw);
    spinand_printf(SPINAND_DBG, "=================================================\n\n");
    SPINAND_PRINTK_LEVEL = level_temp;
}

static void cvi_spinand_setup_intr(void)
{
	spinand_setbit_32(BASE_SPI_NAND + REG_SPI_NAND_INT_EN, 0x1f1);
	spinand_setbit_32(BASE_SPI_NAND + REG_SPI_NAND_INT_CLR, BITS_SPI_NAND_INT_CLR_ALL);
	spinand_setbit_32(BASE_SPI_NAND + REG_SPI_NAND_INT_MASK, 0);
}

csi_error_t cvi_spinand_op(csi_spinand_t *spinand, spi_mem_op_t *op)
{
    uint32_t rx_data = 0;
    spinand_reg_t spinand_reg_config = {0};
    uint32_t trx_addr = 0;

    if(op->data.nbytes > 4){
        cvi_spinand_dma_setup(op->data.buf.out, op->data.nbytes, (op->data.dir == SPI_MEM_DATA_IN) ? 0 : 1);
        spinand_reg_config.reg_mode_ctrl0.bit.reg_trx_dma_en = 1;
    }

    if(op->cmd.opcode == SPI_NAND_CMD_SET_FEATURE){
        spinand_reg_config.reg_trx_size.bit.reg_trx_cmd_cont_size = op->addr.nbytes + op->data.nbytes;
        spinand_reg_config.reg_trx_size.bit.reg_trx_dummy_size = op->dummy.nbytes;
        spinand_reg_config.reg_cont0.bit.reg_trx_cmd_cont0 = ((spinand->scractbuf[0] << 8) | op->addr.val);
    }else{
        spinand_reg_config.reg_trx_size.bit.reg_trx_cmd_cont_size = op->addr.nbytes;
        spinand_reg_config.reg_trx_size.bit.reg_trx_dummy_size = op->dummy.nbytes;
        spinand_reg_config.reg_trx_size.bit.reg_trx_data_size = op->data.nbytes;
        trx_addr = (op->addr.val&0xff);
        for(uint8_t i=1; i<op->addr.nbytes; i++){
            trx_addr = trx_addr<<8;
            trx_addr |= ((op->addr.val>>(8*i))&0xff);
        }

        spinand_reg_config.reg_cont0.bit.reg_trx_cmd_cont0 = trx_addr;
    }

    if(op->data.buswidth == 4){
        spinand_reg_config.reg_mode_ctrl0.bit.reg_io_size_mode = SPI_NAND_CTRL3_IO_TYPE_X4_MODE;
    }else{

    }
    spinand_reg_config.reg_mode_ctrl0.bit.reg_trx_rw = (op->data.dir == SPI_MEM_DATA_IN) ? 0 : 1;

    spinand_reg_config.reg_cont0.bit.reg_trx_cmd_idx = op->cmd.opcode;

	spinand_writel(BASE_SPI_NAND + REG_SPI_NAND_TRX_CTRL2, spinand_reg_config.reg_trx_size.raw);
	spinand_writel(BASE_SPI_NAND + REG_SPI_NAND_TRX_CTRL3, spinand_reg_config.reg_mode_ctrl0.raw);
	spinand_writel(BASE_SPI_NAND + REG_SPI_NAND_TRX_CMD0, spinand_reg_config.reg_cont0.raw);
    cvi_spinand_setup_intr();

	spinand_setbit_32(BASE_SPI_NAND + REG_SPI_NAND_TRX_CTRL0, BIT_REG_TRX_START);

    uint32_t time_end = csi_tick_get_ms() + CVI_DEVICE_TX_RX_TIMEOUT;
    do{
        if((spinand_readl(BASE_SPI_NAND + REG_SPI_NAND_INT) & BIT_REG_TRX_DONE_INT) != 0){
            if((op->data.nbytes > 0) && (op->data.nbytes <= 4) && (op->data.dir == SPI_MEM_DATA_IN)){
                rx_data = spinand_readl(BASE_SPI_NAND + REG_SPI_NAND_RX_DATA);
                memcpy(op->data.buf.in, &rx_data, op->data.nbytes);
            }
            spinand_writel(BASE_SPI_NAND + REG_SPI_NAND_INT_CLR, spinand_readl(BASE_SPI_NAND + REG_SPI_NAND_INT_CLR) | BIT_REG_TRX_DONE_INT);
            spinand_printf(SPINAND_DBG, "exit\n");
            return CSI_OK;
        }
    }while(csi_tick_get_ms() < time_end);
    spinand_printf(SPINAND_ERR, "error timeout\n");
    debug_printf(spinand, op, &spinand_reg_config);

    return CSI_TIMEOUT;
}
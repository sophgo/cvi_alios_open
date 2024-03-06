/* SPDX-License-Identifier: GPL-2.0-or-later
 * CV181x I2s driver on CVITEK CV181x
 *
 * Copyright 2022 CVITEK
 *
 * Author: Ruilong.Chen
 *
 */


#include <stdio.h>
#include <string.h>
#include <csi_config.h>
#include <sys_clk.h>
#include <k_err.h>

#include <drv/i2s.h>
#include <drv/cvi_irq.h>
#include "dw_i2s.h"
#include "ringbuffer.h"
#include <ulog/ulog.h>
#include <drv/dma.h>
#include <alsa/pcm.h>
#include <aos/kernel.h>
#include <semaphore.h>
#include <pthread.h>
#include <aos/cli.h>
#include "debug/dbg.h"

#define TAG "i2s"
//#define AUDIO_USING_NONCACHE_MEM 1

static struct i2stx_info g_i2stx_pri;
static struct i2stx_info g_i2srx_pri;
static struct i2stx_info g_i2s1rx_pri;
static struct i2stx_info g_i2s2tx_pri;
unsigned char *test_data_src = NULL;

csi_dma_ch_config_t i2s3_dma_config = {
    .src_inc = DMA_ADDR_INC,
    .dst_inc = DMA_ADDR_CONSTANT,
    .src_tw = DMA_DATA_WIDTH_16_BITS,
    .dst_tw = DMA_DATA_WIDTH_32_BITS,
    .trans_dir = DMA_MEM2PERH,
    .group_len = (INIT_FIFO_THRESHOLD+1)*4,//32U,//16U,
    .handshake = 2,

};

csi_dma_ch_config_t i2s0_dma_config = {
    .src_inc = DMA_ADDR_CONSTANT,
    .dst_inc = DMA_ADDR_INC,
    .src_tw = DMA_DATA_WIDTH_32_BITS,
    .dst_tw = DMA_DATA_WIDTH_16_BITS,
    .trans_dir = DMA_PERH2MEM,
    .group_len = (INIT_FIFO_THRESHOLD+1)*4,
    .handshake = 3,
};

csi_dma_ch_config_t i2s1_dma_config = {
    .src_inc = DMA_ADDR_CONSTANT,
    .dst_inc = DMA_ADDR_INC,
    .src_tw = DMA_DATA_WIDTH_32_BITS,
    .dst_tw = DMA_DATA_WIDTH_16_BITS,
    .trans_dir = DMA_PERH2MEM,
    .group_len = (INIT_FIFO_THRESHOLD+1)*4,
    .handshake = 6,
};
typedef struct _csi_i2s_status_tag{
	aos_mutex_t mutex;
	aos_event_t evt;
	int wake_cnt;
}csi_i2s_status_t;

static bool i2s0_start_dma = false;
static bool i2s1_start_dma = false;
static bool i2s2_start_dma = false;
static bool i2s3_start_dma = false;

#ifndef AUDIO_USING_NONCACHE_MEM
	#define audio_dcache_clean_invalid_range soc_dcache_clean_invalid_range
#else
	#define audio_dcache_clean_invalid_range(...)
#endif

//for underrun
static uint64_t *pZero = NULL;
static uint8_t *pAlignZero = NULL;
static uint32_t xrun = 0;
static uint32_t res_data = 0;


static uint8_t *get_buffer_alig_64(uint32_t len, uint64_t **first_addr)
{
	uint8_t  *addr;
	uint64_t tmp;

	tmp = (uintptr_t)aos_zalloc(len + (1 << 6) - 1);
	if(!tmp){
		return NULL;
	}
	*first_addr = (uint64_t *)tmp;
	addr = (uint8_t *)((tmp+ (1 << 6) - 1) & ~((1 << 6) - 1));
	return addr;
}
/*
 * set_default_sound_i2s_values for i2s parameters
 *
 */
int set_default_sound_i2s3_values(unsigned int i2s_no, u32 rate, u8 channels)
{
    g_i2stx_pri.mclk_out_en = 0;
    g_i2stx_pri.clk_src = AUD_CLK_FROM_PLL;
    g_i2stx_pri.channels = 2;
    g_i2stx_pri.id = i2s_no;
    g_i2stx_pri.base_address = i2s_get_base(i2s_no);
    g_i2stx_pri.sys_base_address = i2s_get_sys_base();
    g_i2stx_pri.role = MASTER_MODE;
    g_i2stx_pri.slot_no = 2;
    //dac only support 2ch.
    g_i2stx_pri.channels = channels;
    g_i2stx_pri.slot_no = channels;
    g_i2stx_pri.role = MASTER_MODE;
    g_i2stx_pri.inv = FMT_IB_IF; // FMT_IB_IF;

    g_i2stx_pri.aud_mode = I2S_MODE;
    g_i2stx_pri.mclk_div = 1; /* 1 means bypass*/

    g_i2stx_pri.samplingrate = rate;
    g_i2stx_pri.bitspersample = 16; /* bit resolution */
    g_i2stx_pri.sync_div = WSS_16_CLKCYCLE; /* WS clock cycle (L+R). 0x40: 64 bit, 0x30: 48 bit, 0x20: 32 bit*/
    g_i2stx_pri.bclk_div = 0x01; /* 1 means bypass*/
    g_i2stx_pri.fifo_threshold = INIT_FIFO_THRESHOLD;
    g_i2stx_pri.fifo_high_threshold = INIT_FIFO_HIGH_THRESHOLD;

    return 0;
}


int set_default_sound_i2s0_rx_values(unsigned int i2s_no, u32 rate, u8 channels)
{
    debug("%s\n", __func__);
    g_i2srx_pri.mclk_out_en = 0;
    g_i2srx_pri.clk_src = AUD_CLK_FROM_PLL;
    g_i2srx_pri.channels = 2;
    g_i2srx_pri.id = i2s_no;
    g_i2srx_pri.base_address = i2s_get_base(i2s_no);
    g_i2srx_pri.sys_base_address = i2s_get_sys_base();

    g_i2srx_pri.mclk_out_en = 1;
    g_i2srx_pri.role = SLAVE_MODE;
    //adc only support 2ch.
    g_i2srx_pri.channels = channels;
    g_i2srx_pri.slot_no = channels;
    g_i2srx_pri.inv = FMT_IB_NF; // FMT_IB_NF;

    g_i2srx_pri.aud_mode = I2S_MODE;
    g_i2srx_pri.mclk_div = 1; /* 1 means bypass*/

    g_i2srx_pri.samplingrate = rate;
    g_i2srx_pri.bitspersample = 16; /* bit resolution */
    g_i2srx_pri.sync_div = WSS_16_CLKCYCLE; /* WS clock cycle (L+R). 0x40: 64 bit, 0x30: 48 bit, 0x20: 32 bit*/
    g_i2srx_pri.bclk_div = 0x01; /* 1 means bypass*/
    g_i2srx_pri.fifo_threshold = INIT_FIFO_THRESHOLD;
    g_i2srx_pri.fifo_high_threshold = INIT_FIFO_HIGH_THRESHOLD;

    return 0;
}

int set_default_sound_i2s1_rx_values(unsigned int i2s_no, u32 rate, u8 channels)
{
    debug("%s\n", __func__);
    g_i2s1rx_pri.mclk_out_en = 0;
    g_i2s1rx_pri.clk_src = AUD_CLK_FROM_PLL;
    g_i2s1rx_pri.channels = 2;
    g_i2s1rx_pri.id = i2s_no;
    g_i2s1rx_pri.base_address = i2s_get_base(i2s_no);
    g_i2s1rx_pri.sys_base_address = i2s_get_sys_base();

    g_i2s1rx_pri.mclk_out_en = 1;
    g_i2s1rx_pri.role = SLAVE_MODE;
    //adc only support 2ch.
    g_i2s1rx_pri.channels = channels;
    g_i2s1rx_pri.slot_no = channels;
    g_i2s1rx_pri.inv = FMT_IB_NF; // FMT_IB_NF;

    g_i2s1rx_pri.aud_mode = I2S_MODE;
    g_i2s1rx_pri.mclk_div = 1; /* 1 means bypass*/

    g_i2s1rx_pri.samplingrate = rate;
    g_i2s1rx_pri.bitspersample = 16; /* bit resolution */
    g_i2s1rx_pri.sync_div = WSS_16_CLKCYCLE; /* WS clock cycle (L+R). 0x40: 64 bit, 0x30: 48 bit, 0x20: 32 bit*/
    g_i2s1rx_pri.bclk_div = 0x01; /* 1 means bypass*/
    g_i2s1rx_pri.fifo_threshold = INIT_FIFO_THRESHOLD;
    g_i2s1rx_pri.fifo_high_threshold = INIT_FIFO_HIGH_THRESHOLD;

    return 0;
}

struct i2stx_info * i2s_get_i2stx_info(csi_i2s_t *i2s) {

	int i2s_no = i2s_get_no(i2s->dev.reg_base);
	switch(i2s_no) {
		case I2S0:
			return &g_i2srx_pri;
			break;
		case I2S1:
			return &g_i2s1rx_pri;
			break;
		case I2S2:
			return &g_i2s2tx_pri;
			break;
		case I2S3:
			return &g_i2stx_pri;
			break;
		default:
			return &g_i2srx_pri;
	}

	return &g_i2srx_pri;
}


void i2s_stop(csi_i2s_t *i2s)
{
	struct i2stx_info *pi2s_tx = i2s_get_i2stx_info(i2s);

	i2s_switch(I2S_OFF, pi2s_tx->base_address);

}


static int32_t i2s_send_polling(csi_i2s_t *i2s, const void *byte_data, uint32_t nbytes)
{
	struct i2stx_info *pi2s_tx = i2s_get_i2stx_info(i2s);
	struct i2s_tdm_regs *i2s_reg;
	u16 *send_data = (u16 *)byte_data;
	uint32_t sample_num = nbytes/2;

	int32_t write_size = 0;
	u32 overrun = 0;
	u32 fifo_depth = 0;
	u32 remainder = 0;
	u32 integer = 0;
	i2s_reg = pi2s_tx->base_address;
	fifo_depth = pi2s_tx->fifo_threshold + 1;
	integer = sample_num/fifo_depth + 1;
	remainder = sample_num%fifo_depth;
//i2s_send_polling fifo_depth = 16 , sample_num = 18468(1154)(4), send_data = 0x820391a8, i2s_reg = 0x4130000
//	printf("%s fifo_depth = %d , sample_num = %d(%d)(%d), send_data = %p, i2s_reg = %p\n", __func__, fifo_depth, sample_num, sample_num/fifo_depth, sample_num%fifo_depth, send_data, i2s_reg);

	while(integer) {
		overrun = readl(&i2s_reg->i2s_int);
		if (((overrun & I2S_INT_TXDA) == I2S_INT_TXDA) || ((overrun & I2S_INT_TXDA_RAW) == I2S_INT_TXDA_RAW)) {

			if (integer > 1) {
				for (int i = 0; i < fifo_depth; i++) {
					mmio_write_32((uintptr_t)&i2s_reg->tx_wr_port_ch0, *send_data);
					send_data++;
				}
				write_size += fifo_depth;
			} else if (integer == 1) {
				//printf("write remainder data. \n");
				for (int i = 0; i < remainder; i++) {
					mmio_write_32((uintptr_t)&i2s_reg->tx_wr_port_ch0, *send_data);
					send_data++;
				}
				write_size += remainder;
			}
			integer--;
	        // Write 1 to clear.
	        mmio_write_32((uintptr_t)&i2s_reg->i2s_int, overrun | I2S_INT_TXDA | I2S_INT_TXDA_RAW);
		}else {
			//printf("%s can't write fifo , overrun = 0x%x, sample_num = %d\n", __func__, overrun, sample_num);
		}

#if 1
		if ((overrun & I2S_INT_TXFO) || (overrun & I2S_INT_TXFO_RAW)) {
			printf("o\n");
			//printf("WARNING!!! I2S%d TX FIFO exception occur int_status=0x%x\n", i2s_no, overrun);
			i2s_switch(I2S_OFF, i2s_reg);
			mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, (readl(&i2s_reg->i2s_clk_ctrl0) | AUD_ENABLE));
			i2s_sw_reset(i2s_reg);
			i2s_switch(I2S_ON, i2s_reg);
		} else if ((overrun & I2S_INT_TXFU) || (overrun & I2S_INT_TXFU_RAW)) {
			printf("u\n");
			//printf("WARNING!!! I2S%d TX FIFO exception occur int_status=0x%x\n", i2s_no, overrun);
			i2s_switch(I2S_OFF, i2s_reg);
			mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, (readl(&i2s_reg->i2s_clk_ctrl0) | AUD_ENABLE));
			i2s_sw_reset(i2s_reg);
			i2s_switch(I2S_ON, i2s_reg);

		}
#endif
	}

//	printf("%s end write_byte_size = %d \n", __func__, 2*write_size);
	return nbytes;//write_size;
}


static int32_t i2s_receive_polling(csi_i2s_t *i2s, void *data, uint32_t nbytes)
{

    struct i2stx_info *pi2s_rx = i2s_get_i2stx_info(i2s);
    struct i2s_tdm_regs *i2s_reg;
    int32_t received_size = (int32_t)nbytes;
    uint16_t i = 0U;
    uint16_t *readdata = (uint16_t *)data;
    uint32_t sample_num = nbytes / 2U;
    uint32_t fifo_depth;
    u32 overrun = 0;
    u32 read_byte_size = 0;

    i2s_reg = pi2s_rx->base_address;
    fifo_depth = pi2s_rx->fifo_threshold + 1;
    //printf("%s fifo_depth=%d, nbytes=%d, sample_num=%d, data=%p (%d)(%d)\n", __func__, fifo_depth, nbytes, sample_num, data, integer, remainder);
    while (sample_num) {
		overrun = readl(&i2s_reg->i2s_int);
		if (((overrun & I2S_INT_RXDA_RAW) == I2S_INT_RXDA_RAW) || ((overrun & I2S_INT_RXDA) == I2S_INT_RXDA)) {
			if(sample_num >= fifo_depth) {
		        for (i = 0U; i < fifo_depth; i++) {
		            *(readdata++) = (uint16_t)readl(&i2s_reg->rx_rd_port_ch0);
		        }
				read_byte_size += fifo_depth;
				sample_num -= fifo_depth;
			}else {
				//printf("readl remainder data. \n");
				int tmp = sample_num;
				for (i = 0; i < tmp; i++) {
					*(readdata++) = (uint16_t)readl(&i2s_reg->rx_rd_port_ch0);
				}
				read_byte_size += tmp;
				sample_num -= tmp;
			}
			// Write 1 to clear.
			mmio_write_32((uintptr_t)&i2s_reg->i2s_int, overrun | I2S_INT_RXDA | I2S_INT_RXDA_RAW);

	        if (sample_num == 0U) {
	            break;
	        }
		} else {
			//printf("%s can't read fifo , overrun = 0x%x, sample_num = %d\n", __func__, overrun, sample_num);
		}

#if 1
		if ((overrun & I2S_INT_RXFO) || (overrun & I2S_INT_RXFO_RAW)) {
			printf("o\n");
			i2s_switch(I2S_OFF, i2s_reg);
			mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, (readl(&i2s_reg->i2s_clk_ctrl0) | AUD_ENABLE));
			i2s_sw_reset(i2s_reg);
			i2s_switch(I2S_ON, i2s_reg);
		} else if ((overrun & I2S_INT_RXFU) || (overrun & I2S_INT_RXFU_RAW)) {
			printf("u\n");
			i2s_switch(I2S_OFF, i2s_reg);
			mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, (readl(&i2s_reg->i2s_clk_ctrl0) | AUD_ENABLE));
			i2s_sw_reset(i2s_reg);
			i2s_switch(I2S_ON, i2s_reg);

		}
#endif

    }
//	printf("%s end read_byte_size = %d , nbytes = %d \n", __func__, read_byte_size*2, nbytes);
    return received_size;
}

#ifdef CONFIG_XIP
#define I2S_CODE_IN_RAM __attribute__((section(".ram.code")))
#else
#define I2S_CODE_IN_RAM
#endif
I2S_CODE_IN_RAM int i2s_check_fifo_status(csi_i2s_t *i2s)
{
	struct i2stx_info *pi2s_tx = i2s_get_i2stx_info(i2s);
	struct i2s_tdm_regs *i2s_reg;
	volatile u32 overrun = 0;

	i2s_reg = pi2s_tx->base_address;

	overrun = readl(&i2s_reg->i2s_int);
#if 1
	if ((overrun & I2S_INT_TXFO) || (overrun & I2S_INT_TXFO_RAW)) {
		printf("o\n");
		//printf("WARNING!!! I2S%d TX FIFO exception occur int_status=0x%x\n", i2s_no, overrun);
		i2s_switch(I2S_OFF, i2s_reg);
		mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, (readl(&i2s_reg->i2s_clk_ctrl0) | AUD_ENABLE));
		i2s_sw_reset(i2s_reg);
		i2s_switch(I2S_ON, i2s_reg);
	} else if ((overrun & I2S_INT_TXFU) || (overrun & I2S_INT_TXFU_RAW)) {
		printf("u\n");
		//printf("WARNING!!! I2S%d TX FIFO exception occur int_status=0x%x\n", i2s_no, overrun);
		i2s_switch(I2S_OFF, i2s_reg);
		mmio_write_32((uintptr_t)&i2s_reg->i2s_clk_ctrl0, (readl(&i2s_reg->i2s_clk_ctrl0) | AUD_ENABLE));
		i2s_sw_reset(i2s_reg);
		i2s_switch(I2S_ON, i2s_reg);
	}
#endif

	return 0;//write_size;
}

I2S_CODE_IN_RAM void dw_i2s_dma_event_cb(csi_dma_ch_t *dma, csi_dma_event_t event, void *arg)
{
    CSI_PARAM_CHK_NORETVAL(dma);
    csi_i2s_t *i2s = (csi_i2s_t *)dma->parent;
	csi_i2s_status_t *p_status = (csi_i2s_status_t *)i2s->priv;
	csi_dma_ch_stop(dma);
	//i2s_check_status(i2s);
	struct i2stx_info *pi2s_tx = i2s_get_i2stx_info(i2s);
    if (event == DMA_EVENT_TRANSFER_ERROR) {/* DMA transfer ERROR */
        if (i2s->tx_dma->ch_id == dma->ch_id) {
            /* to do tx error action */
            i2s->state.error = 1U;

            if (i2s->callback) {
                i2s->callback(i2s, I2S_EVENT_ERROR, i2s->arg);
            }
        } else if (i2s->rx_dma->ch_id == dma->ch_id) {
            /* to do rx error action */
            i2s->state.error = 1U;

            if (i2s->callback) {
                i2s->callback(i2s, I2S_EVENT_ERROR, i2s->arg);
            }
        }

    } else if (event == DMA_EVENT_TRANSFER_DONE) {/* DMA transfer complete */
        if ((i2s->tx_dma != NULL) && (i2s->tx_dma->ch_id == dma->ch_id)) {
            /* to do tx action */
            uint32_t read_len = i2s->tx_period;
            void * src = NULL;

            if(i2s->tx_buf->data_len < 2 * i2s->tx_period)
            {
                if (i2s->callback) {
                    //printf("i2s%d,audio playing underrun...\n", i2s_get_no(i2s->dev.reg_base));
                    i2s->callback(i2s, I2S_EVENT_TX_BUFFER_EMPTY, i2s->arg);
                }
                if(!xrun && (i2s->tx_buf->data_len > i2s->tx_period)) {
                    src = (void *)(i2s->tx_buf->buffer + (i2s->tx_buf->read + i2s->tx_period) % i2s->tx_buf->size);
                    read_len = i2s->tx_buf->data_len - i2s->tx_period;
                    res_data = i2s->tx_buf->data_len;
                    xrun++;
                } else {
                    if(res_data == i2s->tx_buf->data_len)
                    {
                        xrun++;
                        if(xrun > (i2s->tx_buf->size / i2s->tx_period))
                        {
                            ringbuffer_reset(i2s->tx_buf);
                            //aos_debug_printf("play underrun reset buffer\r\n");
                            i2s3_start_dma = false;
                            xrun = 0;
                            res_data = 0;
                            return;
                        }
                    } else {
                        xrun = 1;
                        res_data = i2s->tx_buf->data_len;
                    }
                    src = (void *)pAlignZero;
                    //aos_debug_printf("src:%x, len:%d, xrun:%d, data_len:%d\r\n", src, read_len, xrun, i2s->tx_buf->data_len);
                }
            } else {
                i2s->tx_buf->read = (i2s->tx_buf->read + read_len) % i2s->tx_buf->size;
                i2s->tx_buf->data_len -= read_len;
                src = (void *)(i2s->tx_buf->buffer + i2s->tx_buf->read);
                xrun = 0;
            }
            audio_dcache_clean_invalid_range((unsigned long)(i2s->tx_buf->buffer), i2s->tx_buf->size);
            csi_dma_ch_start(i2s->tx_dma, src, (void *) & (pi2s_tx->base_address->tx_wr_port_ch0), read_len);
            if (i2s->callback) {
                i2s->callback(i2s, I2S_EVENT_SEND_COMPLETE, i2s->arg);
                aos_event_set(&p_status->evt, PCM_EVT_WRITE, AOS_EVENT_OR);
            }
        } else {
            /* to do rx action */
            uint32_t write_len = i2s->rx_period;
            if(ringbuffer_avail(i2s->rx_buf) < write_len) {
                if (i2s->callback)
                    i2s->callback(i2s, I2S_EVENT_RX_BUFFER_FULL, i2s->arg);
                //printf("i2s%d,audio capture overrun...\n", i2s_get_no(i2s->dev.reg_base));
            }else {
                i2s->rx_buf->data_len += write_len;
                i2s->rx_buf->write = (i2s->rx_buf->write + write_len) % i2s->rx_buf->size;
                //i2s->rx_buf->data_len = i2s->rx_buf->data_len % i2s->rx_buf->size;
                if (i2s->callback && i2s->rx_buf->data_len != 0) {
                    i2s->callback(i2s, I2S_EVENT_RECEIVE_COMPLETE, i2s->arg);
                    //printf("i2s%d,audio capture...\n", i2s_get_no(i2s->dev.reg_base));
                    if(p_status) {
                        p_status->wake_cnt++;
                        aos_event_set(&p_status->evt, PCM_EVT_READ, AOS_EVENT_OR);
                    }
                }
            }
            audio_dcache_clean_invalid_range((unsigned long)(i2s->rx_buf->buffer), i2s->rx_buf->size);
           	csi_dma_ch_start(i2s->rx_dma, (void *) & (pi2s_tx->base_address->rx_rd_port_ch0), i2s->rx_buf->buffer + i2s->rx_buf->write, i2s->rx_period);
        }
    }
}

/**
  \brief       I2S init
  \param[in]   i2s i2s handle to operate.
  \param[in]   idx i2s interface idx
  \return      error code
*/
csi_error_t csi_i2s_init(csi_i2s_t *i2s, uint32_t idx)
{
	csi_error_t ret = CSI_OK;
	CSI_PARAM_CHK(i2s, CSI_ERROR);
	//i2s_debug_cmd_register();
	switch(idx) {
		case I2S0:
			i2s->dev.reg_base = (unsigned long)i2s_get_base(I2S0);
			set_default_sound_i2s0_rx_values(I2S0, 16000, 2);
		//	ret = i2s_init(&g_i2srx_pri);
			break;
		case I2S1:
            i2s->dev.reg_base = (unsigned long)i2s_get_base(I2S1);
            set_default_sound_i2s1_rx_values(I2S1, 16000, 2);
			break;
		case I2S2:
			break;
		case I2S3:
			i2s->dev.reg_base = (unsigned long)i2s_get_base(I2S3);
			set_default_sound_i2s3_values(I2S3, 16000, 2);
		//	ret = i2s_init(&g_i2stx_pri);
			break;
		default:
			break;
	}

    i2s->rx_dma = NULL;
    i2s->tx_dma = NULL;
    i2s->rx_buf = NULL;
    i2s->tx_buf = NULL;
    i2s->state.error = 0U;
    i2s->state.readable = 0U;
    i2s->state.writeable = 0U;
    i2s->priv = (void *)0U;
    i2s->callback = NULL;

	csi_i2s_status_t *p_status = calloc(sizeof(csi_i2s_status_t), 1);
	if (!p_status) {
	    return -1;
	}
	memset(p_status,0,sizeof(csi_i2s_status_t));
	aos_mutex_new(&p_status->mutex);
	if (aos_mutex_new(&p_status->mutex) != 0) {
		printf("csi_i2s_init failed mutex\n");
        return -1;
    }
	aos_event_new(&p_status->evt, 0);
	i2s->priv = (void *)p_status;

    return ret;
}

/**
  \brief       I2S uninit
  \param[in]   i2s i2s handle to operate.
  \return      none
*/
void csi_i2s_uninit(csi_i2s_t *i2s)
{
    CSI_PARAM_CHK_NORETVAL(i2s);
    i2s->rx_dma = NULL;
    i2s->tx_dma = NULL;
    i2s->rx_buf = NULL;
    i2s->tx_buf = NULL;
    i2s->state.error = 0U;
    i2s->state.readable = 0U;
    i2s->state.writeable = 0U;
	if(i2s->priv) {
		csi_i2s_status_t *p_status = (csi_i2s_status_t *)i2s->priv;
		aos_mutex_free(&p_status->mutex);
		aos_event_free(&p_status->evt);
		free(p_status);
		i2s->priv = NULL;
	}
}

/**
  \brief       I2S enable
  \param[in]   i2s i2s handle to operate.
  \param[in]   en  1 enable, 0 disable
  \return      none
*/
void csi_i2s_enable(csi_i2s_t *i2s, bool enable)
{
    CSI_PARAM_CHK_NORETVAL(i2s);
    if (enable) {
        i2s_switch(I2S_ON, (struct i2s_tdm_regs*)i2s->dev.reg_base);
    } else {
		i2s_switch(I2S_OFF, (struct i2s_tdm_regs*)i2s->dev.reg_base);
    }
}

/**
  \brief       I2S config
  \param[in]   i2s i2s handle to operate.
  \param[in]   config i2s config param
  \return      error code
*/
csi_error_t csi_i2s_format(csi_i2s_t *i2s, csi_i2s_format_t *format)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    CSI_PARAM_CHK(format, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    struct i2stx_info *pi2s_tx = i2s_get_i2stx_info(i2s);
    int i2s_no = i2s_get_no(i2s->dev.reg_base);

    switch (format->mode) {
        case I2S_MODE_MASTER:
            pi2s_tx->role = MASTER_MODE;
            break;
        case I2S_MODE_SLAVE:
            pi2s_tx->role = SLAVE_MODE;
            break;
        default:
            ret = CSI_ERROR;
            break;
    }

    switch (format->protocol) {
        case I2S_PROTOCOL_I2S:
            pi2s_tx->aud_mode = I2S_MODE;
            break;
        case I2S_PROTOCOL_MSB_JUSTIFIED:
            pi2s_tx->aud_mode = RJ_MODE;
            break;
        case I2S_PROTOCOL_LSB_JUSTIFIED:
            pi2s_tx->aud_mode = LJ_MODE;
            break;
        case I2S_PROTOCOL_PCM:
            ret = CSI_UNSUPPORTED;
            break;
        default:
            ret = CSI_ERROR;
            break;
    }

    switch (format->width) {
        case I2S_SAMPLE_WIDTH_16BIT:
            pi2s_tx->bitspersample = 16;
            pi2s_tx->sync_div = WSS_16_CLKCYCLE;
            break;
        case I2S_SAMPLE_WIDTH_24BIT:
            pi2s_tx->bitspersample = 24;
            pi2s_tx->sync_div = WSS_24_CLKCYCLE;
            break;

        case I2S_SAMPLE_WIDTH_32BIT:
			pi2s_tx->bitspersample = 32;
			pi2s_tx->sync_div = WSS_32_CLKCYCLE;
            break;

        default:
            ret = CSI_UNSUPPORTED;
            break;
    }

    if (format->mode == I2S_MODE_MASTER) {
        switch (format->rate) {
            case I2S_SAMPLE_RATE_8000:
                pi2s_tx->samplingrate = 8000;
                break;
            case I2S_SAMPLE_RATE_16000:
                pi2s_tx->samplingrate = 16000;
                break;
            case I2S_SAMPLE_RATE_32000:
                pi2s_tx->samplingrate = 32000;
                break;
            case I2S_SAMPLE_RATE_44100:
                pi2s_tx->samplingrate = 44100;
                break;
            case I2S_SAMPLE_RATE_48000:
                pi2s_tx->samplingrate = 48000;
                break;
            case I2S_SAMPLE_RATE_96000:
                pi2s_tx->samplingrate = 96000;
                break;

            default:
                ret = CSI_UNSUPPORTED;
                break;
        }
        printf("%s format->rate = %d\n", __func__, pi2s_tx->samplingrate);
    } else {
    }

    if (format->polarity) {
 		switch(i2s_no) {
			case I2S0:
				pi2s_tx->inv = FMT_IB_NF;
				break;
			case I2S1:
				pi2s_tx->inv = FMT_IB_NF;
				break;
			case I2S2:
				pi2s_tx->inv = FMT_IB_NF;
				break;
			case I2S3:
				pi2s_tx->inv = FMT_IB_IF;
				break;
			default:
				break;
		}

    } else {
		switch(i2s_no) {
			case I2S0:
				pi2s_tx->inv = FMT_IB_NF;
				break;
			case I2S1:
				pi2s_tx->inv = FMT_IB_NF;
				break;
			case I2S2:
				pi2s_tx->inv = FMT_IB_NF;
				break;
			case I2S3:
				pi2s_tx->inv = FMT_IB_IF;
				break;
			default:
				break;
		}

    }
    //printf("csi_i2s_format samplingrate:%d id:%d i2s_no:%d\n",
    //    pi2s_tx->samplingrate,pi2s_tx->id,i2s_no);

    ret = i2s_init(pi2s_tx);
    if(i2s_no == I2S0 || i2s_no == I2S3) {
        struct i2s_tdm_regs *i2s_reg = i2s_get_base(I2S3);
        i2s_set_clk_source(i2s_reg,pi2s_tx->clk_src);
        i2s0_set_clk_sample_rate(i2s_reg,format->rate);
    }

    return ret;
}

/**
  \brief       Set the i2s tx mono
  \param[in]   i2s i2s handle to operate.
  \param[in]   Mono channel selection.
  \param[in]   bool mono mode enable
  \return      error code
*/
csi_error_t csi_i2s_tx_select_sound_channel(csi_i2s_t *i2s, csi_i2s_sound_channel_t ch)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    struct i2stx_info *pi2s_tx = i2s_get_i2stx_info(i2s);
    struct i2s_tdm_regs *i2s_reg;
    u32 data_format = 0;
    u32 slot_setting2 = 0;
    i2s_reg = pi2s_tx->base_address;
    //_/wj_i2s_regs_t *i2s_base;
    //_/i2s_base = (wj_i2s_regs_t *)HANDLE_REG_BASE(i2s);

    switch (ch) {
        case I2S_LEFT_CHANNEL:
            //_/wj_i2s_transmit_mode_source_is_single_channel(i2s_base);
            //_/wj_i2s_transmit_mode_channel_polarity_low_for_left(i2s_base);
            data_format = readl(&i2s_reg->data_format);
            data_format |= (1<<5);
            slot_setting2 = 0x1;
            mmio_write_32((uintptr_t)&i2s_reg->slot_setting2, slot_setting2);
            mmio_write_32((uintptr_t)&i2s_reg->data_format, data_format);
            break;

        case I2S_RIGHT_CHANNEL:
            //_/wj_i2s_transmit_mode_source_is_single_channel(i2s_base);
           //_/ wj_i2s_transmit_mode_channel_polarity_high_for_left(i2s_base);
            data_format = readl(&i2s_reg->data_format);
            data_format |= (1<<5);
            slot_setting2 = 0x2;
            mmio_write_32((uintptr_t)&i2s_reg->slot_setting2, slot_setting2);
            mmio_write_32((uintptr_t)&i2s_reg->data_format, data_format);
            break;

        case I2S_LEFT_RIGHT_CHANNEL:
            //_/wj_i2s_transmit_mode_source_is_dual_channel(i2s_base);
            slot_setting2 = 0x3;
            mmio_write_32((uintptr_t)&i2s_reg->slot_setting2, slot_setting2);
            break;

        default:
            break;
    }

    return ret;
}

/**
  \brief       Set the i2s rx mono
  \param[in]   i2s i2s handle to operate.
  \param[in]   Mono channel selection.
  \param[in]   bool mono mode enable
  \return      error code
*/
csi_error_t csi_i2s_rx_select_sound_channel(csi_i2s_t *i2s, csi_i2s_sound_channel_t ch)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    struct i2stx_info *pi2s_tx = i2s_get_i2stx_info(i2s);
    struct i2s_tdm_regs *i2s_reg;
    u32 data_format = 0;
    u32 slot_setting2 = 0;
    i2s_reg = pi2s_tx->base_address;
    //_/wj_i2s_regs_t *i2s_base;
    //_/i2s_base = (wj_i2s_regs_t *)HANDLE_REG_BASE(i2s);

    switch (ch) {
        case I2S_LEFT_CHANNEL:
            //_/wj_i2s_transmit_mode_source_is_single_channel(i2s_base);
            //_/wj_i2s_transmit_mode_channel_polarity_low_for_left(i2s_base);
            data_format = readl(&i2s_reg->data_format);
            data_format |= (1<<4);
            slot_setting2 = 0x1;
            mmio_write_32((uintptr_t)&i2s_reg->slot_setting2, slot_setting2);
            mmio_write_32((uintptr_t)&i2s_reg->data_format, data_format);
            break;

        case I2S_RIGHT_CHANNEL:
            //_/wj_i2s_transmit_mode_source_is_single_channel(i2s_base);
            //_/wj_i2s_transmit_mode_channel_polarity_high_for_left(i2s_base);
            data_format = readl(&i2s_reg->data_format);
            data_format |= (1<<4);
            slot_setting2 = 0x2;
            mmio_write_32((uintptr_t)&i2s_reg->slot_setting2, slot_setting2);
            mmio_write_32((uintptr_t)&i2s_reg->data_format, data_format);
            break;

        case I2S_LEFT_RIGHT_CHANNEL:
            //_/wj_i2s_transmit_mode_source_is_dual_channel(i2s_base);
            slot_setting2 = 0x3;
            mmio_write_32((uintptr_t)&i2s_reg->slot_setting2, slot_setting2);
            break;

        default:
            break;
    }

    return ret;
}

/**
  \brief       link DMA channel to i2s device
  \param[in]   i2s  i2s handle to operate.
  \param[in]   rx_dma the DMA channel  for receive, when it is NULL means to unused dma.
  \return      error code
*/
csi_error_t csi_i2s_rx_link_dma(csi_i2s_t *i2s, csi_dma_ch_t *rx_dma)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    if (rx_dma != NULL) {
        rx_dma->parent = i2s;
        switch(i2s->dev.idx){
            case 0:
                ret = csi_dma_ch_alloc(rx_dma, 3, 0);
                if (ret == CSI_OK) {
                    ret = csi_dma_ch_config(rx_dma, &i2s0_dma_config);
                    csi_dma_ch_attach_callback(rx_dma, dw_i2s_dma_event_cb, NULL);
                    i2s->rx_dma = rx_dma;
                } else {
                    printf("%s csi_dma_ch_alloc error \n", __func__);
                    rx_dma->parent = NULL;
                    ret = CSI_ERROR;
                }
                break;
            case 1:
                ret = csi_dma_ch_alloc(rx_dma, 6, 0);
                if (ret == CSI_OK) {
                    ret = csi_dma_ch_config(rx_dma, &i2s1_dma_config);
                    csi_dma_ch_attach_callback(rx_dma, dw_i2s_dma_event_cb, NULL);
                    i2s->rx_dma = rx_dma;
                     //printf("%s csi_dma_ch_alloc success \n", __func__);
                } else {
                    printf("%s csi_dma_ch_alloc error \n", __func__);
                    rx_dma->parent = NULL;
                    ret = CSI_ERROR;
                }
                break;
        }
    } else {
        if (i2s->rx_dma) {
            csi_dma_ch_detach_callback(i2s->rx_dma);
            csi_dma_ch_free(i2s->rx_dma);
            i2s->rx_dma = NULL;
        } else {
            ret = CSI_ERROR;
        }
    }

    return ret;
}

/**
  \brief       link DMA channel to i2s device
  \param[in]   i2s  i2s handle to operate.
  \param[in]   rx_dma the DMA channel  for receive, when it is NULL means to unused dma.
  \return      error code
*/
csi_error_t csi_i2s_tx_link_dma(csi_i2s_t *i2s, csi_dma_ch_t *tx_dma)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    if (tx_dma != NULL) {
        tx_dma->parent = i2s;
        ret = csi_dma_ch_alloc(tx_dma, 2, 0);

        if (ret == CSI_OK) {
            ret = csi_dma_ch_config(tx_dma, &i2s3_dma_config);
            csi_dma_ch_attach_callback(tx_dma, dw_i2s_dma_event_cb, NULL);
            i2s->tx_dma = tx_dma;
        } else {
            printf("%s csi_dma_ch_alloc error \n", __func__);
            tx_dma->parent = NULL;
            ret = CSI_ERROR;
        }
    } else {
        if (i2s->tx_dma) {
            csi_dma_ch_detach_callback(i2s->tx_dma);
            csi_dma_ch_free(i2s->tx_dma);
            i2s->tx_dma = NULL;
        } else {
            printf("%s error \n", __func__);
            ret = CSI_ERROR;
        }
    }

    return ret;
}

/**
  \brief       I2S rx cache config
  \param[in]   i2s i2s handle to operate.
  \param[in]   buffer i2s rx cache
  \return      none
*/
void csi_i2s_rx_set_buffer(csi_i2s_t *i2s, csi_ringbuf_t *buffer)
{
    CSI_PARAM_CHK_NORETVAL(i2s);
    CSI_PARAM_CHK_NORETVAL(buffer);
    i2s->rx_buf = buffer;
}

/**
  \brief       I2S tx cache config
  \param[in]   i2s i2s handle to operate.
  \param[in]   buffer i2s tx cache
  \return      none
*/
void csi_i2s_tx_set_buffer(csi_i2s_t *i2s, csi_ringbuf_t *buffer)
{
    CSI_PARAM_CHK_NORETVAL(i2s);
    CSI_PARAM_CHK_NORETVAL(buffer);
    i2s->tx_buf = buffer;
}

/**
  \brief       I2S rx set period.The value of period is to report a receive completion event
  \            after each period value data is received.
  \param[in]   i2s i2s handle to operate.
  \param[in]   buffer i2s rx cache
  \return      none
*/
csi_error_t csi_i2s_rx_set_period(csi_i2s_t *i2s, uint32_t period)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    if (period == 0U) {
        ret = CSI_ERROR;
    } else {
        if ((i2s->rx_buf->size % period) != 0U) {
            ret = CSI_ERROR;
        } else {
            i2s->rx_period = period;
        }
    }

    return ret;
}

/**
  \brief       I2S tx set period.The value of period is to report a receive completion event
  \            after each period value data is send.
  \param[in]   i2s i2s handle to operate.
  \param[in]   buffer i2s tx cache
  \return      none
*/
csi_error_t csi_i2s_tx_set_period(csi_i2s_t *i2s, uint32_t period)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    printf("%s ch->period = %d \n", __func__, period);

    if (period == 0U) {
        ret = CSI_ERROR;
    } else {
        if ((i2s->tx_buf->size % period) != 0U) {
            printf("%s failed error ch->period = %d i2s->tx_buf->size = %d\n", __func__, period, i2s->tx_buf->size);
            ret = CSI_ERROR;
        } else {
			printf("%s ch->period = %d \n", __func__, period);
            i2s->tx_period = period;
        }
    }

    return ret;
}

/**
  \brief  Get rx ringbuffer cache free space
   \param[in]   i2s i2s handle to operate.
  \return buffer free space (bytes)
*/
uint32_t csi_i2s_rx_buffer_avail(csi_i2s_t *i2s)
{
    CSI_PARAM_CHK(i2s, 0U);
	CSI_PARAM_CHK(i2s->rx_buf, 0U);
    return ringbuffer_avail(i2s->rx_buf);
}



uint32_t csi_i2s_rx_buffer_remain(csi_i2s_t *i2s)
{
	CSI_PARAM_CHK(i2s, 0U);
	CSI_PARAM_CHK(i2s->rx_buf, 0U);
    return ringbuffer_len(i2s->rx_buf);
}


/**
  \brief  Reset the rx ringbuffer, discard all data in the cache
  \param[in]   i2s i2s handle to operate.
  \return      error code
*/
csi_error_t csi_i2s_rx_buffer_reset(csi_i2s_t *i2s)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    csi_error_t ret = CSI_ERROR;
    ringbuffer_reset(i2s->rx_buf);

    if (i2s->rx_buf->buffer != NULL) {
        memset(i2s->rx_buf->buffer, 0, i2s->rx_buf->size);
    }

    if ((i2s->rx_buf->read == 0U) && (i2s->rx_buf->write == 0U)) {
        ret = CSI_OK;
    }

    return ret;
}

/**
  \brief  Get tx ringbuffer cache free space
   \param[in]   i2s i2s handle to operate.
  \return buffer free space (bytes)
*/
uint32_t csi_i2s_tx_buffer_avail(csi_i2s_t *i2s)
{
    CSI_PARAM_CHK(i2s, 0U);
    return ringbuffer_avail(i2s->tx_buf);
}

/**
  \brief        Get tx csi_ringbuf buffer used space
  \param[in]    i2s    I2s handle to operate
  \return       Buffer used space (bytes)
*/
uint32_t csi_i2s_tx_buffer_remain(csi_i2s_t *i2s)
{
    CSI_PARAM_CHK(i2s, 0U);
	CSI_PARAM_CHK(i2s->tx_buf, 0U);
    return ringbuffer_len(i2s->tx_buf);
}


/**
  \brief  Reset the tx ringbuffer, discard all data in the cache
  \param[in]   i2s i2s handle to operate.
  \return      error code
*/
csi_error_t csi_i2s_tx_buffer_reset(csi_i2s_t *i2s)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    csi_error_t ret = CSI_ERROR;
    ringbuffer_reset(i2s->tx_buf);
    if (i2s->tx_buf->buffer != NULL) {
        memset(i2s->tx_buf->buffer, 0, i2s->tx_buf->size);
    }

    if ((i2s->tx_buf->read == 0U) && (i2s->tx_buf->write == 0U)) {
        ret = CSI_OK;
    }

    return ret;
}
/**
  \brief  Receive an amount of data to cache in blocking mode.
  \param[in]   i2s   operate handle.
  \param[in]   data  save receive data.
  \param[in]   size  receive data size.
  \param[in]   timeout  is the number of queries, not time
  \return      The size of data receive successfully
*/
int32_t csi_i2s_receive(csi_i2s_t *i2s, void *data, uint32_t size)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    int32_t received_size = 0;
    uint8_t *read_data = (void *)data;
    int32_t read_size = 0;
	int ret = 0;
	unsigned int actl_flags = 0;
    int32_t avail_size = 0;
	csi_i2s_status_t *p_status = (csi_i2s_status_t *)i2s->priv;

    if (i2s->rx_dma == NULL) {
        received_size = i2s_receive_polling(i2s, read_data, size);
        if (received_size == (int32_t)size) {
            read_size = (int32_t)size;
        } else {
            read_size = CSI_ERROR;
        }

    } else {
		aos_mutex_lock(&p_status->mutex, AOS_WAIT_FOREVER);
        while (1) {
            avail_size = ringbuffer_len(i2s->rx_buf);
            read_size += (int32_t)ringbuffer_out(i2s->rx_buf, (void *)(read_data + (uint32_t)read_size), (size - (uint32_t)read_size));
            if ((size - (uint32_t)read_size) <= 0U) {
                break;
            }
			if(p_status) {
				ret = aos_event_get(&p_status->evt, PCM_EVT_READ | PCM_EVT_XRUN, AOS_EVENT_OR_CLEAR, &actl_flags, 10*1000);
				if(ret == RHINO_SUCCESS) {
					if ((actl_flags | PCM_EVT_XRUN) == PCM_EVT_XRUN) {
						LOGW(TAG,"pcm read PCM_EVT_XRUN\r\n");
					}
				}else {
					LOGE(TAG,"aos_event_get error ret:%d  avail_size:%d, read_size:%d\r\n",ret, avail_size, read_size);
				}
			}
        }
		aos_mutex_unlock(&p_status->mutex);
    }

    return read_size;
}

/**
  \brief Write data to the cache.
  \With asynchronous sending,
  \the data is first written to the cache and then output through the I2S interface.
  \This function does not block, and the return value is the number
  \of data that was successfully written to the cache.
  \param[in]   i2s   operate handle.
  \param[in]   data  send data.
  \param[in]   size  receive data size.
  \return      The data size that write to cache
*/
uint32_t csi_i2s_send_async(csi_i2s_t *i2s, const void *data, uint32_t size)
{
    CSI_PARAM_CHK(i2s, 0U);
    CSI_PARAM_CHK(data, 0U);
    uint32_t write_len;

    uint32_t result = csi_irq_save();
    write_len = ringbuffer_in(i2s->tx_buf, data, size);
    csi_irq_restore(result);

    if ((uint8_t *)i2s->priv) { ///< if dma is stop, then start it
        //i2s->priv = (void *)0U;
    }

    return write_len;
}
/**
  \brief  Transmits an amount of data to cache in blocking mode.
  \param[in]   i2s   operate handle.
  \param[in]   data  send data.
  \param[in]   size  receive data byte_size.
  \param[in]   timeout  is the number of queries, not time
  \return      The num of data witch is send successful
*/
int32_t csi_i2s_send(csi_i2s_t *i2s, const void *data, uint32_t size)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    int32_t sent_size = 0;
    int32_t write_size = 0;
    int32_t avail_size = 0;
    int ret = 0;
    unsigned int actl_flags = 0;
    csi_i2s_status_t *p_status = (csi_i2s_status_t *)i2s->priv;
    struct i2stx_info *pi2s_tx = i2s_get_i2stx_info(i2s);
    uint8_t *send_data = (void *)data;

    if (i2s->tx_dma == NULL) {

        sent_size = i2s_send_polling(i2s, send_data, size);
        if (sent_size == (int32_t)size) {
            write_size = sent_size;
        } else {
            write_size = CSI_ERROR;
        }

    } else {
		aos_mutex_lock(&p_status->mutex, AOS_WAIT_FOREVER);
        while (1) {
            write_size += (int32_t)ringbuffer_in(i2s->tx_buf, (void *)(send_data + (uint32_t)write_size), (size - (uint32_t)write_size));
            if (!i2s3_start_dma) {
                avail_size = ringbuffer_len(i2s->tx_buf);
                if (avail_size >= i2s->tx_period) {
                    printf("%s avail_size = %d , i2s->tx_buf->buffer addr = %p , i2s->tx_buf->read = %d, i2s->tx_period = %d\n",
                               __func__, avail_size, i2s->tx_buf->buffer, i2s->tx_buf->read, i2s->tx_period);
                    audio_dcache_clean_invalid_range((unsigned long)(i2s->tx_buf->buffer), i2s->tx_buf->size);
                	csi_dma_ch_start(i2s->tx_dma, i2s->tx_buf->buffer + i2s->tx_buf->read, (void *) &(pi2s_tx->base_address->tx_wr_port_ch0), i2s->tx_period);
                	i2s3_start_dma = true;
                }
            }
            if ((size - (uint32_t)write_size) <= 0U) {
                break;
            }
			if(p_status) {
				ret = aos_event_get(&p_status->evt, PCM_EVT_WRITE | PCM_EVT_XRUN, AOS_EVENT_OR_CLEAR, &actl_flags, 10*1000);
				if(ret == RHINO_SUCCESS) {
					if ((actl_flags | PCM_EVT_XRUN) == PCM_EVT_XRUN) {
						LOGW(TAG,"pcm write PCM_EVT_XRUN\r\n");
					}
				}else {
					LOGE(TAG,"aos_event_get error ret:%d \r\n",ret);
				}
			}
        }
		aos_mutex_unlock(&p_status->mutex);
    }

    return write_size;
}

/**
  \brief Read data from the cache.
  \Using asynchronous receive, i2s writes the received data to the cache.
  \This function reads data from the cache, returns the number of successful reads,
  \and returns 0 if there is no data in the cache.
  \param[in]   i2s   operate handle.
  \param[in]   data  the buf save receive data.
  \param[in]   size  receive data size.
  \return      The size of data read successfully
*/
uint32_t csi_i2s_receive_async(csi_i2s_t *i2s, void *data, uint32_t size)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    int32_t received_size = 0;
    uint8_t *read_data = (void *)data;
    int32_t read_size = 0;
    int ret = 0;
    unsigned int actl_flags = 0;
    int32_t avail_size = 0;
    csi_i2s_status_t *p_status = (csi_i2s_status_t *)i2s->priv;

    if (i2s->rx_dma == NULL) {
        received_size = i2s_receive_polling(i2s, read_data, size);
        if (received_size == (int32_t)size) {
            read_size = (int32_t)size;
        } else {
            read_size = CSI_ERROR;
        }

    } else {
        aos_mutex_lock(&p_status->mutex, AOS_WAIT_FOREVER);
        while (1) {
            avail_size = ringbuffer_len(i2s->rx_buf);
            if (avail_size < i2s->rx_period) {
                return 0;
            }
            read_size += (int32_t)ringbuffer_out(i2s->rx_buf, (void *)(read_data + (uint32_t)read_size), (size - (uint32_t)read_size));
            if ((size - (uint32_t)read_size) <= 0U) {
                break;
            }
            if(p_status) {
                ret = aos_event_get(&p_status->evt, PCM_EVT_READ | PCM_EVT_XRUN, AOS_EVENT_OR_CLEAR, &actl_flags, 10*1000);
                if(ret == RHINO_SUCCESS) {
                    if ((actl_flags | PCM_EVT_XRUN) == PCM_EVT_XRUN) {
                        LOGW(TAG,"pcm read PCM_EVT_XRUN\r\n");
                    }
                }else {
                    LOGE(TAG,"aos_event_get error ret:%d  avail_size:%d, read_size:%d\r\n",ret, avail_size, read_size);
                }
            }
        }
        aos_mutex_unlock(&p_status->mutex);
    }

    return read_size;
}

/**
  \brief       start i2s pause asynchronous send
  \param[in]   i2s  operate handle.
  \return      error code
*/
extern void csi_dma_ch_pause(csi_dma_ch_t *dma_ch);
csi_error_t csi_i2s_send_pause(csi_i2s_t *i2s)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    //csi_dma_ch_stop(i2s->tx_dma);
    //i2s->state.writeable = 0U;
    csi_dma_ch_pause(i2s->tx_dma);
    return ret;
}

/**
  \brief       start i2s resume asynchronous send
  \param[in]   i2s  operate handle.
  \return      error code
*/
extern void csi_dma_ch_resume(csi_dma_ch_t *dma_ch);
csi_error_t csi_i2s_send_resume(csi_i2s_t *i2s)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    //i2s->state.writeable = 1U;
    csi_dma_ch_resume(i2s->tx_dma);
    return ret;
}


/**
  \brief       start i2s asynchronous send
  \param[in]   i2s  operate handle.
  \return      error code
*/
csi_error_t csi_i2s_send_start(csi_i2s_t *i2s)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    struct i2stx_info *pi2s_tx = i2s_get_i2stx_info(i2s);

    i2s_transfer_tx_data(pi2s_tx);
    memset(i2s->tx_buf->buffer, 0, i2s->tx_buf->size);
    pAlignZero = get_buffer_alig_64(i2s->tx_period, &pZero);
    i2s->state.writeable = 1U;

    return ret;
}

/**
  \brief       start i2s asynchronous receive
  \param[in]   i2s  operate handle.
  \return      error code
*/
csi_error_t csi_i2s_receive_start(csi_i2s_t *i2s)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    int i2s_no = i2s_get_no(i2s->dev.reg_base);
    struct i2stx_info *pi2s_tx = i2s_get_i2stx_info(i2s);
	switch(i2s_no) {
		case I2S0:
			i2s0_start_dma = true;
			break;
		case I2S1:
			i2s1_start_dma = true;
			break;
		case I2S2:
			i2s2_start_dma = true;
			break;
		case I2S3:
			i2s3_start_dma = true;
			break;
		default:
			break;
	}
    i2s_receive_rx_data(pi2s_tx);
    audio_dcache_clean_invalid_range((unsigned long)(i2s->rx_buf->buffer), i2s->rx_buf->size);
    csi_dma_ch_start(i2s->rx_dma, (void *) &(pi2s_tx->base_address->rx_rd_port_ch0), i2s->rx_buf->buffer, i2s->rx_period);
	ringbuffer_reset(i2s->rx_buf);
	memset(i2s->rx_buf->buffer, 0, i2s->rx_buf->size);
    i2s->state.readable = 1U;
    return ret;
}

/**
  \brief       stop i2s asynchronous send
  \param[in]   i2s  operate handle.
*/
void csi_i2s_send_stop(csi_i2s_t *i2s)
{
    int i2s_no = i2s_get_no(i2s->dev.reg_base);
    CSI_PARAM_CHK_NORETVAL(i2s);

    switch(i2s_no) {
		case I2S0:
			i2s0_start_dma = false;
			break;
		case I2S1:
			i2s1_start_dma = false;
			break;
		case I2S2:
			i2s2_start_dma = false;
			break;
		case I2S3:
			i2s3_start_dma = false;
			break;
		default:
			break;
    }

    csi_dma_ch_stop(i2s->tx_dma);
    ringbuffer_reset(i2s->tx_buf);
    memset(i2s->tx_buf->buffer, 0, i2s->tx_buf->size);
    i2s_stop(i2s);
    if(pZero){
        free(pZero);
        pZero = NULL;
        pAlignZero = NULL;
    }
    i2s->state.writeable = 0U;
}

/**
  \brief       stop i2s asynchronous receive
  \param[in]   i2s  operate handle.
  \return      error code
*/
void csi_i2s_receive_stop(csi_i2s_t *i2s)
{
    int i2s_no = i2s_get_no(i2s->dev.reg_base);
    CSI_PARAM_CHK_NORETVAL(i2s);
	switch(i2s_no) {
		case I2S0:
			i2s0_start_dma = false;
			break;
		case I2S1:
			i2s1_start_dma = false;
			break;
		case I2S2:
			i2s2_start_dma = false;
			break;
		case I2S3:
			i2s3_start_dma = false;
			break;
		default:
			break;
	}

	csi_dma_ch_stop(i2s->rx_dma);
	ringbuffer_reset(i2s->rx_buf);
	memset(i2s->rx_buf->buffer, 0, i2s->rx_buf->size);
	i2s_stop(i2s);
    i2s->rx_buf->write = 0;
    i2s->state.readable = 0U;
}

/**
  \brief       attach the callback handler to i2s
  \param[in]   i2s  operate handle.
  \param[in]   cb    callback function
  \param[in]   arg   user can define it by himself as callback's param
  \return      error code
*/
csi_error_t csi_i2s_attach_callback(csi_i2s_t *i2s, void *callback, void *arg)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    i2s->callback = callback;
    i2s->arg = arg;
    return ret;
}

/**
  \brief       detach the callback handler
  \param[in]   i2s  operate handle.
  \return      none
*/
void csi_i2s_detach_callback(csi_i2s_t *i2s)
{
    CSI_PARAM_CHK_NORETVAL(i2s);
    i2s->callback = NULL;
    i2s->arg = NULL;
}

/**
  \brief       Get i2s status.
  \param[in]   i2s i2s handle to operate.
  \param[out]  state i2s state.
  \return      i2s error code
*/
csi_error_t csi_i2s_get_state(csi_i2s_t *i2s, csi_state_t *state)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    CSI_PARAM_CHK(state, CSI_ERROR);
    *state = i2s->state;
    return CSI_OK;
}

#ifdef CONFIG_PM
csi_error_t csi_i2s_enable_pm(csi_i2s_t *i2s)
{
    return CSI_UNSUPPORTED;
}

void csi_i2s_disable_pm(csi_i2s_t *i2s)
{

}
#endif

static void i2s_dump_register(struct i2s_tdm_regs *i2s_reg)
{
    printf("%s start addr:%p \n", __func__, i2s_reg);
    printf("BLK_MODE_SETTING = 0x%x \n",i2s_reg->blk_mode_setting);
    printf("FRAME_SETTING = 0x%x \n",i2s_reg->frame_setting);
    printf("SLOT_SETTING1 = 0x%x \n",i2s_reg->slot_setting1);
    printf("SLOT_SETTING2 = 0x%x \n",i2s_reg->slot_setting2);
    printf("DATA_FORMAT = 0x%x \n",i2s_reg->data_format);
    printf("BLK_CFG = 0x%x \n",i2s_reg->blk_cfg);
    printf("I2S_ENABLE = 0x%x \n",i2s_reg->i2s_enable);
    printf("I2S_INT_EN = 0x%x \n",i2s_reg->i2s_int_en);
    printf("FIFO_THRESHOLD = 0x%x \n",i2s_reg->fifo_threshold);
    printf("I2S_LRCK_MASTER = 0x%x \n",i2s_reg->i2s_lrck_master);
    printf("I2S_CLK_CTRL0 = 0x%x \n",i2s_reg->i2s_clk_ctrl0);
    printf("I2S_CLK_CTRL1 = 0x%x \n",i2s_reg->i2s_clk_ctrl1);
    printf("I2S_PCM_SYNTH = 0x%x \n",i2s_reg->i2s_pcm_synth);
}

#ifdef AUDIO_DEBUG_CLI_SUPPORT

int i2s_show_info(unsigned int idx)
{
	struct i2stx_info *pst_i2s_info;
	if(idx > 3) {
		printf("invaid iis_%d\n",idx);
		return -1;
	}

	switch(idx) {
		case 0:pst_i2s_info = &g_i2srx_pri;break;
		case 1:pst_i2s_info = &g_i2s1rx_pri;break;
		case 2:pst_i2s_info = &g_i2s2tx_pri;break;
		case 3:pst_i2s_info = &g_i2stx_pri;break;
		default:
			printf("invaid iis_%d\n",idx);
		return -2;
	}

	printf("i2s_%d info:\n", idx);
	printf("mclk_out_en: %d\n", pst_i2s_info->mclk_out_en);
	printf("samplingrate: %d\n", pst_i2s_info->samplingrate);
	printf("channels: %d\n", pst_i2s_info->channels);
	printf("id: %d\n", pst_i2s_info->id);
	printf("role: %d\n", pst_i2s_info->role);
	printf("aud_mode: %d\n", pst_i2s_info->aud_mode);
	printf("mclk_div: %d\n", pst_i2s_info->mclk_div);
	printf("bclk_div: %d\n", pst_i2s_info->bclk_div);
	printf("sync_div: %d\n", pst_i2s_info->sync_div);
	printf("fifo_threshold: %d\n", pst_i2s_info->fifo_threshold);
	printf("fifo_high_threshold: %d\n", pst_i2s_info->fifo_high_threshold);

	if(!pst_i2s_info->base_address)
		pst_i2s_info->base_address = i2s_get_base(idx);
	if(!pst_i2s_info->sys_base_address)
		pst_i2s_info->sys_base_address = i2s_get_sys_base();


	if(pst_i2s_info->base_address) {
		i2s_dump_register(pst_i2s_info->base_address);
	}

	return 0;
}

void audio_debug_show_i2s_info(char *buf, int32_t len, int32_t argc, char **argv)
{
	int idx = 0;
	if(argc == 2) {
		idx = (int)atoi(argv[1]);
	}else {
		printf("usage:%s 0-3\n",argv[0]);
		return;
	}

	if(i2s_show_info(idx) < 0) {
		printf("usage:%s 0-3\n",argv[0]);
		return;
	}
	return;
}

const static struct cli_command i2s_cmd = { "i2s_info", "i2s info", audio_debug_show_i2s_info };

static int register_flag = 0;
int i2s_debug_cmd_register(void)
{
	if(register_flag)
		return 0;
	register_flag = 1;
	return aos_cli_register_command(&i2s_cmd);
}
#else
int i2s_debug_cmd_register(void)
{
	return 0;
}
#endif

/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     wj_spdif.c
 * @brief    source file for spdif ll driver
 * @version  V1.0
 * @date     21. Oct 2020
 ******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <csi_config.h>
#include <sys_clk.h>
#include <drv/spdif.h>
#include <drv/cvi_irq.h>
#include <wj_spdif_ll.h>

extern uint16_t spdif_tx_hs_num[];
extern uint16_t spdif_rx_hs_num[];

DEFINE_DESC_BUF(spdif_send_buf, 4);
DEFINE_DESC_BUF(spdif_receive_buf, 4);

csi_error_t csi_spdif_init(csi_spdif_t *spdif, uint32_t idx)
{
    CSI_PARAM_CHK(spdif, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    ret = target_get(DEV_WJ_SPDIF_TAG, idx, &spdif->dev);

    if (ret == CSI_OK) {
        wj_spdif_regs_t *spdif_base;
        spdif_base = (wj_spdif_regs_t *)HANDLE_REG_BASE(spdif);
        wj_spdif_enable(spdif_base);
        spdif_base->RX_CTL_REG = 0U;
    }

    spdif->rx_dma = NULL;
    spdif->tx_dma = NULL;
    spdif->rx_buf = NULL;
    spdif->tx_buf = NULL;
    spdif->state.error = 0U;
    spdif->state.readable = 0U;
    spdif->state.writeable = 0U;
    spdif->priv = (void *)0U;
    spdif->callback = NULL;
    return ret;
}

void csi_spdif_uninit(csi_spdif_t *spdif)
{
    CSI_PARAM_CHK_NORETVAL(spdif);
    wj_spdif_regs_t *spdif_base;
    spdif_base = (wj_spdif_regs_t *)HANDLE_REG_BASE(spdif);
    wj_spdif_disable(spdif_base);
    // todo
    spdif->rx_dma = NULL;
    spdif->tx_dma = NULL;
    spdif->rx_buf = NULL;
    spdif->tx_buf = NULL;
    spdif->state.error = 0U;
    spdif->state.readable = 0U;
    spdif->state.writeable = 0U;
}

csi_error_t csi_spdif_format(csi_spdif_t *spdif, csi_spdif_format_t *format)
{
    CSI_PARAM_CHK(spdif, CSI_ERROR);
    CSI_PARAM_CHK(format, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    uint32_t div;
    wj_spdif_regs_t *spdif_base;
    spdif_base = (wj_spdif_regs_t *)HANDLE_REG_BASE(spdif);

    switch (format->rate) {
        case SPDIF_SAMPLE_RATE_22050:
            ret = CSI_UNSUPPORTED;
            break;

        case SPDIF_SAMPLE_RATE_24000:
            ret = CSI_UNSUPPORTED;
            break;

        case SPDIF_SAMPLE_RATE_32000:
            div = (soc_get_audio_clk(0) / 2048000 / 2) - 1;
            wj_spdif_set_tx_div(spdif_base, div);
            wj_spdif_set_tx_div_not_bypass(spdif_base);
            wj_spdif_set_tx_sample_rate_32khz(spdif_base);
            wj_spdif_set_tx_channel_b_sample_rate_32khz(spdif_base);
            break;

        case SPDIF_SAMPLE_RATE_44100:
            div = (soc_get_audio_clk(0) / 2822400 / 2) - 1;
            wj_spdif_set_tx_div(spdif_base, div);
            wj_spdif_set_tx_div_not_bypass(spdif_base);
            wj_spdif_set_tx_sample_rate_44_1khz(spdif_base);
            wj_spdif_set_tx_channel_b_sample_rate_44_1khz(spdif_base);
            break;

        case SPDIF_SAMPLE_RATE_48000:
            //div = (soc_get_audio_clk(0) / 3072000 / 2) - 1;
            //wj_spdif_set_tx_div(spdif_base, div);
            wj_spdif_set_tx_div(spdif_base, 1);
            wj_spdif_set_tx_div_not_bypass(spdif_base);
            wj_spdif_set_tx_sample_rate_48khz(spdif_base);
            wj_spdif_set_tx_channel_b_sample_rate_48khz(spdif_base);
            break;

        case SPDIF_SAMPLE_RATE_88200:
            div = (soc_get_audio_clk(0) / 5644800 / 2) - 1;
            wj_spdif_set_tx_div(spdif_base, div);
            wj_spdif_set_tx_div_not_bypass(spdif_base);
            wj_spdif_set_tx_sample_rate_88_2khz(spdif_base);
            wj_spdif_set_tx_channel_b_sample_rate_88_2khz(spdif_base);
            break;

        case SPDIF_SAMPLE_RATE_96000:
            div = (soc_get_audio_clk(0) / 6144000 / 2) - 1;
            wj_spdif_set_tx_div(spdif_base, div);
            wj_spdif_set_tx_div_not_bypass(spdif_base);
            wj_spdif_set_tx_sample_rate_96khz(spdif_base);
            wj_spdif_set_tx_channel_b_sample_rate_96khz(spdif_base);
            break;

        case SPDIF_SAMPLE_RATE_176400:
            ret = CSI_UNSUPPORTED;
            break;

        case SPDIF_SAMPLE_RATE_192000:
            div = (soc_get_audio_clk(0) / 12288000 / 2 - 1);
            wj_spdif_set_tx_div(spdif_base, div);
            wj_spdif_set_tx_div_not_bypass(spdif_base);
            wj_spdif_set_tx_sample_rate_192khz(spdif_base);
            wj_spdif_set_tx_channel_b_sample_rate_192khz(spdif_base);
            break;

        default:
            break;
    }

    switch (format->width) {
        case SPDIF_SAMPLE_WIDTH_16BIT:
            wj_spdif_tx_datamode_16bit_oneworld(spdif_base);
            wj_spdif_rx_datamode_16bit_oneworld(spdif_base);
            break;

        case SPDIF_SAMPLE_WIDTH_20BIT:
            wj_spdif_tx_datamode_20bit(spdif_base);
            wj_spdif_rx_datamode_20bit(spdif_base);
            break;

        case SPDIF_SAMPLE_WIDTH_24BIT:
            wj_spdif_tx_datamode_24bit(spdif_base);
            wj_spdif_rx_datamode_24bit(spdif_base);
            break;

        default:
            break;
    }

    return ret;
}

void csi_spdif_tx_set_buffer(csi_spdif_t *spdif, ringbuffer_t *buffer)
{
    CSI_PARAM_CHK_NORETVAL(spdif);
    CSI_PARAM_CHK_NORETVAL(buffer);
    spdif->tx_buf = buffer;
}

void csi_spdif_rx_set_buffer(csi_spdif_t *spdif, ringbuffer_t *buffer)
{
    CSI_PARAM_CHK_NORETVAL(spdif);
    CSI_PARAM_CHK_NORETVAL(buffer);
    spdif->rx_buf = buffer;
}

csi_error_t csi_spdif_tx_set_period(csi_spdif_t *spdif, uint32_t period)
{
    CSI_PARAM_CHK(spdif, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    if (period == 0U) {
        ret = CSI_ERROR;
    } else {
        if ((spdif->tx_buf->size % period) != 0U) {
            ret = CSI_ERROR;
        } else {
            spdif->tx_period = period;
        }
    }

    return ret;
}

csi_error_t csi_spdif_rx_set_period(csi_spdif_t *spdif, uint32_t period)
{
    CSI_PARAM_CHK(spdif, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    if (period == 0U) {
        ret = CSI_ERROR;
    } else {
        if ((spdif->rx_buf->size % period) != 0U) {
            ret = CSI_ERROR;
        } else {
            spdif->rx_period = period;
        }
    }

    return ret;
}

uint32_t csi_spdif_tx_buffer_avail(csi_spdif_t *spdif)
{
    CSI_PARAM_CHK(spdif, CSI_ERROR);
    return ringbuffer_avail(spdif->tx_buf);
}

uint32_t csi_spdif_rx_buffer_avail(csi_spdif_t *spdif)
{
    CSI_PARAM_CHK(spdif, CSI_ERROR);
    return ringbuffer_avail(spdif->rx_buf);
}

uint32_t csi_spdif_tx_buffer_remain(csi_spdif_t *spdif)
{
    CSI_PARAM_CHK(spdif, CSI_ERROR);
    return ringbuffer_len(spdif->tx_buf);
}

uint32_t csi_spdif_rx_buffer_remain(csi_spdif_t *spdif)
{
    CSI_PARAM_CHK(spdif, CSI_ERROR);
    return ringbuffer_len(spdif->rx_buf);
}

csi_error_t csi_spdif_tx_buffer_reset(csi_spdif_t *spdif)
{
    CSI_PARAM_CHK(spdif, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    ringbuffer_reset(spdif->tx_buf);
    return ret;
}

csi_error_t csi_spdif_rx_buffer_reset(csi_spdif_t *spdif)
{
    CSI_PARAM_CHK(spdif, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    ringbuffer_reset(spdif->rx_buf);
    return ret;
}

static void wj_spdif_dma_event_cb(csi_dma_ch_t *dma, csi_dma_event_t event, void *arg)
{
    CSI_PARAM_CHK_NORETVAL(dma);
    csi_spdif_t *spdif = (csi_spdif_t *)dma->parent;

    if (event == DMA_EVENT_TRANSFER_ERROR) {/* DMA transfer ERROR */
        if (spdif->tx_dma->ch_id == dma->ch_id) {
            /* to do tx error action */
            spdif->state.error = 1U;

            if (spdif->callback) {
                spdif->callback(spdif, SPDIF_EVENT_ERROR, spdif->arg);
            }
        } else if (spdif->rx_dma->ch_id == dma->ch_id) {
            /* to do rx error action */
            spdif->state.error = 1U;

            if (spdif->callback) {
                spdif->callback(spdif, SPDIF_EVENT_ERROR, spdif->arg);
            }
        }

    } else if (event == DMA_EVENT_TRANSFER_BLOCK_DONE) {
        if ((spdif->tx_dma != NULL) && (spdif->tx_dma->ch_id == dma->ch_id)) {
            /* to do tx action */
            uint32_t read_len = spdif->tx_period;
            spdif->tx_buf->read = (spdif->tx_buf->read + read_len) % spdif->tx_buf->size;
            spdif->tx_buf->data_len -= read_len;
            soc_dcache_clean_invalid_range((unsigned long)(spdif->tx_buf->buffer), spdif->tx_buf->size);

            if (spdif->callback) {
                spdif->callback(spdif, SPDIF_EVENT_SEND_COMPLETE, spdif->arg);
            }
        } else {
            /* to do rx action */
            uint32_t write_len = spdif->rx_period;
            spdif->rx_buf->write = (spdif->rx_buf->write + write_len) % spdif->rx_buf->size;
            spdif->rx_buf->data_len += write_len;
            soc_dcache_clean_invalid_range((unsigned long)(spdif->rx_buf->buffer), spdif->rx_buf->size);

            if (spdif->callback) {
                spdif->callback(spdif, SPDIF_EVENT_RECEIVE_COMPLETE, spdif->arg);
            }
        }

    }
}

csi_error_t csi_spdif_tx_link_dma(csi_spdif_t *spdif, csi_dma_ch_t *tx_dma)
{
    CSI_PARAM_CHK(spdif, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    if (tx_dma != NULL) {
        tx_dma->parent = spdif;
        ret = csi_dma_ch_alloc(tx_dma, -1, -1);

        if (ret == CSI_OK) {
            csi_dma_ch_attach_callback(tx_dma, wj_spdif_dma_event_cb, NULL);
            spdif->tx_dma = tx_dma;
        } else {
            tx_dma->parent = NULL;
            ret = CSI_ERROR;
        }
    } else {
        if (spdif->tx_dma) {
            csi_dma_ch_detach_callback(spdif->tx_dma);
            csi_dma_ch_free(spdif->tx_dma);
            spdif->tx_dma = NULL;
        } else {
            ret = CSI_ERROR;
        }
    }

    return ret;
}


csi_error_t csi_spdif_rx_link_dma(csi_spdif_t *spdif, csi_dma_ch_t *rx_dma)
{
    CSI_PARAM_CHK(spdif, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    if (rx_dma != NULL) {
        rx_dma->parent = spdif;
        ret = csi_dma_ch_alloc(rx_dma, -1, -1);

        if (ret == CSI_OK) {
            csi_dma_ch_attach_callback(rx_dma, wj_spdif_dma_event_cb, NULL);
            spdif->rx_dma = rx_dma;
        } else {
            rx_dma->parent = NULL;
            ret = CSI_ERROR;
        }
    } else {
        if (spdif->rx_dma) {
            csi_dma_ch_detach_callback(spdif->rx_dma);
            csi_dma_ch_free(spdif->rx_dma);
            spdif->rx_dma = NULL;
        } else {
            ret = CSI_ERROR;
        }
    }

    return ret;
}

csi_error_t csi_spdif_send_start(csi_spdif_t *spdif)
{
    CSI_PARAM_CHK(spdif, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    csi_dma_ch_config_t config;
    memset(&config, 0, sizeof(csi_dma_ch_config_t));
    wj_spdif_regs_t *spdif_base = (wj_spdif_regs_t *)spdif->dev.reg_base;
    csi_dma_ch_t *dma_ch = (csi_dma_ch_t *)spdif->tx_dma;
    wj_spdif_disable_tx(spdif_base);
    wj_spdif_set_tx_dma_enable(spdif_base);
    wj_spdif_set_tx_dma_th_level(spdif_base, 8U);

    config.src_inc = DMA_ADDR_INC;
    config.dst_inc = DMA_ADDR_CONSTANT;
    config.src_tw = DMA_DATA_WIDTH_32_BITS;
    config.dst_tw = DMA_DATA_WIDTH_32_BITS;

    config.group_len = 32U;
    config.trans_dir = DMA_MEM2PERH;
    config.src_reload_en = 0U;
    config.dst_reload_en = 0U;
    config.half_int_en = 0U;
    /* config for etb */
    config.handshake = spdif_tx_hs_num[spdif->dev.idx];

    /* config for dma cyclic mode*/
    config.cyclic_en = 1U;
    config.cyclic_config.lli_buf = spdif_send_buf;
    config.cyclic_config.buffer = spdif->tx_buf->buffer;
    config.cyclic_config.buffer_bytes = spdif->tx_buf->size;
    config.cyclic_config.period_bytes = spdif->tx_period;
    config.cyclic_config.dstaddr = (void *) & (spdif_base->TX_FIFO_DR);

    ret = csi_dma_ch_config(dma_ch, &config);

    wj_spdif_enable_tx(spdif_base);
    csi_dma_ch_start(dma_ch, NULL, NULL, 0);
    spdif->state.writeable = 1U;
    return ret;
}

csi_error_t csi_spdif_send_pause(csi_spdif_t *spdif)
{
    CSI_PARAM_CHK(spdif, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    extern void dma_ch_pause(csi_dma_ch_t *dma_ch);
    dma_ch_pause(spdif->tx_dma);
    spdif->state.writeable = 0U;
    return ret;
}

csi_error_t csi_spdif_send_resume(csi_spdif_t *spdif)
{
    CSI_PARAM_CHK(spdif, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    extern void dma_ch_resume(csi_dma_ch_t *dma_ch);
    dma_ch_resume(spdif->tx_dma);
    spdif->state.writeable = 1U;
    return ret;
}

uint32_t csi_spdif_send_async(csi_spdif_t *spdif, const void *data, uint32_t size)
{
    CSI_PARAM_CHK(spdif, 0U);
    CSI_PARAM_CHK(data, 0U);
    uint32_t write_len;
    write_len = ringbuffer_in(spdif->tx_buf, data, size);
    return write_len;
}

csi_error_t csi_spdif_receive_start(csi_spdif_t *spdif)
{
    CSI_PARAM_CHK(spdif, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    csi_dma_ch_config_t config;
    memset(&config, 0, sizeof(csi_dma_ch_config_t));
    wj_spdif_regs_t *spdif_base = (wj_spdif_regs_t *)spdif->dev.reg_base;
    csi_dma_ch_t *dma_ch = (csi_dma_ch_t *)spdif->rx_dma;

    wj_spdif_disable_rx(spdif_base);
    wj_spdif_set_rx_dma_enable(spdif_base);
    wj_spdif_set_rx_dma_th_level(spdif_base, 8U);

    config.src_inc = DMA_ADDR_CONSTANT;
    config.dst_inc = DMA_ADDR_INC;
    config.src_tw = DMA_DATA_WIDTH_32_BITS;
    config.dst_tw = DMA_DATA_WIDTH_32_BITS;
    /* config for wj_dma */
    config.group_len = 32U;
    config.trans_dir = DMA_PERH2MEM;
    config.src_reload_en = 0U;
    config.dst_reload_en = 0U;
    config.half_int_en = 0U;
    /* config for etb */
    config.handshake = spdif_rx_hs_num[spdif->dev.idx];

    /* config for dma cyclic mode*/
    config.cyclic_en = 1U;
    config.cyclic_config.lli_buf = spdif_receive_buf;
    config.cyclic_config.buffer = spdif->rx_buf->buffer;
    config.cyclic_config.buffer_bytes = spdif->rx_buf->size;
    config.cyclic_config.period_bytes = spdif->rx_period;
    config.cyclic_config.srcaddr = (void *) & (spdif_base->RX_FIFO_DR);

    ret = csi_dma_ch_config(dma_ch, &config);

    wj_spdif_enable_rx(spdif_base);
    csi_dma_ch_start(dma_ch, NULL, NULL, 0);
    spdif->state.readable = 1U;
    return ret;
}

void csi_spdif_receive_stop(csi_spdif_t *spdif)
{
    CSI_PARAM_CHK_NORETVAL(spdif);
    wj_spdif_regs_t *spdif_base = (wj_spdif_regs_t *)spdif->dev.reg_base;
    csi_dma_ch_stop(spdif->rx_dma);
    ringbuffer_reset(spdif->rx_buf);
    wj_spdif_disable_rx(spdif_base);
    spdif->state.readable = 0U;
}

uint32_t csi_spdif_receive_async(csi_spdif_t *spdif, const void *data, uint32_t size)
{
    CSI_PARAM_CHK(spdif, 0U);
    CSI_PARAM_CHK(data, 0U);
    uint32_t read_len;
    read_len = ringbuffer_out(spdif->rx_buf, (void *)data, size);
    return read_len;
}

csi_error_t csi_spdif_attach_callback(csi_spdif_t *spdif, void *callback, void *arg)
{
    CSI_PARAM_CHK(spdif, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    spdif->callback = callback;
    spdif->arg = arg;
    return ret;
}

void csi_spdif_detach_callback(csi_spdif_t *spdif)
{
    CSI_PARAM_CHK_NORETVAL(spdif);
    spdif->callback = NULL;
    spdif->arg = NULL;
}

csi_error_t csi_spdif_get_state(csi_spdif_t *spdif, csi_state_t *state)
{
    CSI_PARAM_CHK(spdif, CSI_ERROR);
    CSI_PARAM_CHK(state, CSI_ERROR);
    *state = spdif->state;
    return CSI_OK;
}

#ifdef CONFIG_PM
csi_error_t csi_spdif_enable_pm(csi_spdif_t *spdif)
{
    return CSI_UNSUPPORTED;
}

void csi_spdif_disable_pm(csi_spdif_t *spdif)
{

}
#endif

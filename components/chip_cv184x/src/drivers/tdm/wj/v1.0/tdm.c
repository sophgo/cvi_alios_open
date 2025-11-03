/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     tdm.c
 * @brief
 * @version
 * @date     2020-10-20
 ******************************************************************************/

#include <string.h>
#include <csi_config.h>
#include <sys_clk.h>
#include <drv/tdm.h>
#include <drv/cvi_irq.h>
#include <drv/tick.h>
#include <drv/porting.h>
#include <drv/list.h>
#include "wj_tdm_ll.h"

extern uint16_t tdm_rx_hs_num[];

DEFINE_DESC_BUF(tdm_ch0_receive_buf, 6U);
DEFINE_DESC_BUF(tdm_ch1_receive_buf, 6U);
DEFINE_DESC_BUF(tdm_ch2_receive_buf, 6U);
DEFINE_DESC_BUF(tdm_ch3_receive_buf, 6U);
DEFINE_DESC_BUF(tdm_ch4_receive_buf, 6U);
DEFINE_DESC_BUF(tdm_ch5_receive_buf, 6U);
DEFINE_DESC_BUF(tdm_ch6_receive_buf, 6U);
DEFINE_DESC_BUF(tdm_ch7_receive_buf, 6U);

static void wj_tdm_dma_event_cb(csi_dma_ch_t *dma, csi_dma_event_t event, void *arg)
{
    CSI_PARAM_CHK_NORETVAL(dma);
    drv_tdm_ch_t *ch = (drv_tdm_ch_t *)dma->parent;
    if (event == DMA_EVENT_TRANSFER_ERROR) { /* DMA transfer ERROR */
        if (ch->rx_dma->ch_id == dma->ch_id) {
            /* to do rx error action */
            ch->state.error = 1U;

            if (ch->callback) {
                ch->callback(ch, TDM_EVENT_ERROR, ch->arg);
            }
        }

    } else if (event == DMA_EVENT_TRANSFER_BLOCK_DONE) {
        if ((ch->rx_dma != NULL) && (ch->rx_dma->ch_id == dma->ch_id)) {
            /* to do rx action */
            uint32_t write_len = ch->rx_period;
            ch->rx_buf->write = (ch->rx_buf->write + write_len) % ch->rx_buf->size;
            ch->rx_buf->data_len += write_len;

            uint32_t next_write_pos = (ch->rx_buf->write + write_len) % ch->rx_buf->size;
            soc_dcache_clean_invalid_range((unsigned long)(ch->rx_buf->buffer + next_write_pos), ch->rx_period);

            if (ch->callback && ch->rx_buf->data_len != ch->rx_buf->size) {
                ch->callback(ch, TDM_EVENT_RECEIVE_COMPLETE, ch->arg);
            } else {
                ch->callback(ch, TDM_EVENT_RX_BUFFER_FULL, ch->arg);
            }
        }

    }
}

csi_error_t drv_tdm_init(drv_tdm_t *tdm, uint32_t idx)
{
    CSI_PARAM_CHK(tdm, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    slist_init(&tdm->head);

    ret = target_get(DEV_WJ_TDM_TAG, idx, &tdm->dev);

    return ret;
}

void drv_tdm_uninit(drv_tdm_t *tdm)
{
    CSI_PARAM_CHK_NORETVAL(tdm);
    wj_tdm_regs_t *tdm_base;
    tdm_base = (wj_tdm_regs_t *)HANDLE_REG_BASE(tdm);

    wj_tdm_disable(tdm_base);
    slist_init(&tdm->head);
}

void drv_tdm_enable(drv_tdm_t *tdm, bool enable)
{
    CSI_PARAM_CHK_NORETVAL(tdm);
    wj_tdm_regs_t *tdm_base;
    tdm_base = (wj_tdm_regs_t *)HANDLE_REG_BASE(tdm);

    if (enable) {
        wj_tdm_enable(tdm_base);
    } else {
        wj_tdm_disable(tdm_base);
    }
}

csi_error_t drv_tdm_format(drv_tdm_t *tdm, drv_tdm_format_t *format)
{
    CSI_PARAM_CHK(tdm, CSI_ERROR);
    CSI_PARAM_CHK(format, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    wj_tdm_regs_t *tdm_base;
    tdm_base = (wj_tdm_regs_t *)HANDLE_REG_BASE(tdm);

    wj_tdm_disable(tdm_base);

    switch (format->mode) {
        case TDM_MODE_MASTER:
            wj_tdm_master_mode(tdm_base);
            break;

        case TDM_MODE_SLAVE:
            wj_tdm_slave_mode(tdm_base);
            break;

        default:
            ret = CSI_ERROR;
            break;
    }

    wj_tdm_valid_data_edge(tdm_base, (uint32_t)format->sample_edge);

    switch (format->width) {
        case TDM_SAMPLE_WIDTH_16BIT:
            wj_tdm_set_16bits_mode(tdm_base);
            break;

        case TDM_SAMPLE_WIDTH_24BIT:
            wj_tdm_set_24bits_mode(tdm_base);
            break;

        case TDM_SAMPLE_WIDTH_32BIT:
            wj_tdm_set_32bits_mode(tdm_base);
            break;

        default:
            ret = CSI_UNSUPPORTED;
            break;
    }

    if (format->polarity) {
        wj_tdm_polarity_right_first(tdm_base);
    } else {
        wj_tdm_polarity_left_first(tdm_base);
    }

    wj_tdm_div0_level(tdm_base, soc_get_audio_clk(0U) / (format->rate * format->sclk_nfs));

    return ret;
}

csi_error_t drv_tdm_ch_rx_link_dma(drv_tdm_ch_t *ch, csi_dma_ch_t *rx_dma)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    if (rx_dma != NULL) {
        rx_dma->parent = ch->tdm;
        ret = csi_dma_ch_alloc(rx_dma, -1, -1);
        rx_dma->parent = ch;

        if (ret == CSI_OK) {
            csi_dma_ch_attach_callback(rx_dma, wj_tdm_dma_event_cb, NULL);
            ch->rx_dma = rx_dma;
        } else {
            rx_dma->parent = NULL;
            ret = CSI_ERROR;
        }
    } else {
        if (ch->rx_dma) {
            csi_dma_ch_detach_callback(ch->rx_dma);
            csi_dma_ch_free(ch->rx_dma);
            ch->rx_dma = NULL;
        } else {
            ret = CSI_ERROR;
        }
    }

    return ret;
}

void drv_tdm_ch_rx_set_buffer(drv_tdm_ch_t *ch, ringbuffer_t *buffer)
{
    CSI_PARAM_CHK_NORETVAL(ch);
    CSI_PARAM_CHK_NORETVAL(buffer);
    ch->rx_buf = buffer;    
}

csi_error_t drv_tdm_ch_rx_set_period(drv_tdm_ch_t *ch, uint32_t period)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    if (period == 0U) {
        ret = CSI_ERROR;
    } else {
        if ((ch->rx_buf->size % period) != 0U) {
            ret = CSI_ERROR;
        } else {
            ch->rx_period = period;
        }
    }

    return ret;   
}

uint32_t drv_tdm_ch_rx_buffer_avail(drv_tdm_ch_t *ch)
{
    CSI_PARAM_CHK(ch, 0U);
    return ringbuffer_avail(ch->rx_buf);   
}

uint32_t drv_tdm_ch_rx_buffer_remain(drv_tdm_ch_t *ch)
{
    CSI_PARAM_CHK(ch, 0U);
    return ringbuffer_len(ch->rx_buf);
}

csi_error_t drv_tdm_ch_rx_buffer_reset(drv_tdm_ch_t *ch)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_ERROR;
    ringbuffer_reset(ch->rx_buf);
    return ret;
}

uint32_t drv_tdm_ch_receive_async(drv_tdm_ch_t *ch, const void *data, uint32_t size)
{
    CSI_PARAM_CHK(ch, 0U);
    CSI_PARAM_CHK(data, 0U);
    uint32_t read_len;
    read_len = ringbuffer_out(ch->rx_buf, (void *)data, size);
    return read_len;
}

csi_error_t drv_tdm_ch_receive_start(drv_tdm_ch_t *ch)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    csi_dma_ch_config_t config;
    memset(&config, 0U, sizeof(csi_dma_ch_config_t));
    wj_tdm_regs_t *tdm_base;
    tdm_base = (wj_tdm_regs_t *)HANDLE_REG_BASE(ch->tdm);
    csi_dma_ch_t *dma_ch = (csi_dma_ch_t *)ch->rx_dma;

    wj_tdm_dma_enable(tdm_base);
    wj_tdm_set_dma_data_level(tdm_base, 0U);

    config.src_inc = DMA_ADDR_CONSTANT;
    config.dst_inc = DMA_ADDR_INC;
    config.src_tw = DMA_DATA_WIDTH_32_BITS;
    config.dst_tw = DMA_DATA_WIDTH_32_BITS;
    /* config for wj_dma */
    config.group_len = 1U;
    config.trans_dir = DMA_PERH2MEM;
    config.src_reload_en = 0U;
    config.dst_reload_en = 0U;
    config.half_int_en = 0U;
    /* config for etb */
    config.handshake = tdm_rx_hs_num[ch->tdm->dev.idx];

    /* config for dma cyclic mode*/
    config.cyclic_en = 1U;
    config.cyclic_config.buffer = ch->rx_buf->buffer;
    config.cyclic_config.buffer_bytes = ch->rx_buf->size;
    config.cyclic_config.period_bytes = ch->rx_period;

    switch (ch->ch_idx) {
        case 0U:
            config.cyclic_config.lli_buf = tdm_ch0_receive_buf;
            config.cyclic_config.srcaddr = (void *) & (tdm_base->LDR1);
            break;

        case 1U:
            config.cyclic_config.lli_buf = tdm_ch1_receive_buf;
            config.cyclic_config.srcaddr = (void *) & (tdm_base->RDR1);
            break;

        case 2U:
            config.cyclic_config.lli_buf = tdm_ch2_receive_buf;
            config.cyclic_config.srcaddr = (void *) & (tdm_base->LDR2);
            break;

        case 3U:
            config.cyclic_config.lli_buf = tdm_ch3_receive_buf;
            config.cyclic_config.srcaddr = (void *) & (tdm_base->RDR2);
            break;

        case 4U:
            config.cyclic_config.lli_buf = tdm_ch4_receive_buf;
            config.cyclic_config.srcaddr = (void *) & (tdm_base->LDR3);
            break;

        case 5U:
            config.cyclic_config.lli_buf = tdm_ch5_receive_buf;
            config.cyclic_config.srcaddr = (void *) & (tdm_base->RDR3);
            break;

        case 6U:
            config.cyclic_config.lli_buf = tdm_ch6_receive_buf;
            config.cyclic_config.srcaddr = (void *) & (tdm_base->LDR4);
            break;

        case 7U:
            config.cyclic_config.lli_buf = tdm_ch7_receive_buf;
            config.cyclic_config.srcaddr = (void *) & (tdm_base->RDR4);
            break;

        default:
            ret = CSI_ERROR;
            break;    
    }

    ret = csi_dma_ch_config(dma_ch, &config);

    soc_dcache_clean_invalid_range((unsigned long)(ch->rx_buf->buffer), ch->rx_buf->size);
    csi_dma_ch_start(ch->rx_dma, NULL, NULL, 0U);
    ch->state.readable = 1U;
    return ret;
}

csi_error_t drv_tdm_all_ch_receive_start(drv_tdm_t *tdm)
{
    CSI_PARAM_CHK(tdm, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    drv_tdm_ch_t *alloced_ch;
    slist_for_each_entry(&tdm->head, alloced_ch, drv_tdm_ch_t, next) {
        csi_dma_ch_config_t config;
        memset(&config, 0U, sizeof(csi_dma_ch_config_t));
        wj_tdm_regs_t *tdm_base;
        tdm_base = (wj_tdm_regs_t *)HANDLE_REG_BASE(alloced_ch->tdm);
        csi_dma_ch_t *dma_ch = (csi_dma_ch_t *)alloced_ch->rx_dma;

        wj_tdm_set_dma_data_level(tdm_base, 4U);

        config.src_inc = DMA_ADDR_CONSTANT;
        config.dst_inc = DMA_ADDR_INC;
        config.src_tw = DMA_DATA_WIDTH_32_BITS;
        config.dst_tw = DMA_DATA_WIDTH_32_BITS;
        /* config for wj_dma */
        config.group_len = 1U;
        config.trans_dir = DMA_PERH2MEM;
        config.src_reload_en = 0U;
        config.dst_reload_en = 0U;
        config.half_int_en = 0U;
        /* config for etb */
        config.handshake = tdm_rx_hs_num[alloced_ch->tdm->dev.idx];

        /* config for dma cyclic mode*/
        config.cyclic_en = 1U;
        config.cyclic_config.buffer = alloced_ch->rx_buf->buffer;
        config.cyclic_config.buffer_bytes = alloced_ch->rx_buf->size;
        config.cyclic_config.period_bytes = alloced_ch->rx_period;

        switch (alloced_ch->ch_idx) {
            case 0U:
                config.cyclic_config.lli_buf = tdm_ch0_receive_buf;
                config.cyclic_config.srcaddr = (void *) & (tdm_base->LDR1);
                break;

            case 1U:
                config.cyclic_config.lli_buf = tdm_ch1_receive_buf;
                config.cyclic_config.srcaddr = (void *) & (tdm_base->RDR1);
                break;

            case 2U:
                config.cyclic_config.lli_buf = tdm_ch2_receive_buf;
                config.cyclic_config.srcaddr = (void *) & (tdm_base->LDR2);
                break;

            case 3U:
                config.cyclic_config.lli_buf = tdm_ch3_receive_buf;
                config.cyclic_config.srcaddr = (void *) & (tdm_base->RDR2);
                break;

            case 4U:
                config.cyclic_config.lli_buf = tdm_ch4_receive_buf;
                config.cyclic_config.srcaddr = (void *) & (tdm_base->LDR3);
                break;

            case 5U:
                config.cyclic_config.lli_buf = tdm_ch5_receive_buf;
                config.cyclic_config.srcaddr = (void *) & (tdm_base->RDR3);
                break;

            case 6U:
                config.cyclic_config.lli_buf = tdm_ch6_receive_buf;
                config.cyclic_config.srcaddr = (void *) & (tdm_base->LDR4);
                break;

            case 7U:
                config.cyclic_config.lli_buf = tdm_ch7_receive_buf;
                config.cyclic_config.srcaddr = (void *) & (tdm_base->RDR4);
                break;

            default:
                ret = CSI_ERROR;
                break;
        }

        if (ret != CSI_OK) {
            break;
        }

        ret = csi_dma_ch_config(dma_ch, &config);

        if (ret != CSI_OK) {
            break;
        }

        soc_dcache_clean_invalid_range((unsigned long)(alloced_ch->rx_buf->buffer), alloced_ch->rx_buf->size);
        csi_dma_ch_start(alloced_ch->rx_dma, NULL, NULL, 0U);
        alloced_ch->state.readable = 1U;
    }
    return ret;
}

void drv_tdm_ch_receive_stop(drv_tdm_ch_t *ch)
{
    CSI_PARAM_CHK_NORETVAL(ch);
    csi_dma_ch_stop(ch->rx_dma);
    ringbuffer_reset(ch->rx_buf);
    ch->state.readable = 0U;    
}

void drv_tdm_all_ch_receive_stop(drv_tdm_t *tdm)
{
    CSI_PARAM_CHK_NORETVAL(tdm);
    drv_tdm_ch_t *alloced_ch;
    slist_for_each_entry(&tdm->head, alloced_ch, drv_tdm_ch_t, next) {
        csi_dma_ch_stop(alloced_ch->rx_dma);
        ringbuffer_reset(alloced_ch->rx_buf);
        alloced_ch->state.readable = 0U;
    }    
}

csi_error_t drv_tdm_ch_attach_callback(drv_tdm_ch_t *ch, void *callback, void *arg)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    ch->callback = callback;
    ch->arg = arg;
    return ret;
}

void drv_tdm_ch_detach_callback(drv_tdm_ch_t *ch)
{
    CSI_PARAM_CHK_NORETVAL(ch);
    ch->callback = NULL;
    ch->arg = NULL;   
}

csi_error_t drv_tdm_ch_alloc(drv_tdm_t *tdm, drv_tdm_ch_t *ch, uint32_t ch_idx)
{
    CSI_PARAM_CHK(tdm, CSI_ERROR);
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    drv_tdm_ch_t *alloced_ch;
    slist_for_each_entry(&tdm->head, alloced_ch, drv_tdm_ch_t, next) {
        if (alloced_ch->ch_idx == ch_idx) {
            ret = CSI_ERROR;
            break;
        }
    }

    if (ret == CSI_OK) {
        slist_add(&ch->next, &tdm->head);
        ch->ch_idx = ch_idx;
        ch->tdm = tdm;
        ch->rx_dma = NULL;
        ch->rx_buf = NULL;
        ch->state.error = 0U;
        ch->state.readable = 0U;
        ch->priv = (void *)0U;
        ch->callback = NULL;

        wj_tdm_regs_t *tdm_base = (wj_tdm_regs_t *)HANDLE_REG_BASE(tdm);
        wj_tdm_disable(tdm_base);

        switch (ch_idx / 2U) {
            case 0U:
                wj_tdm_set_ch_num(tdm_base, 0U);
                break;

            case 1U:
                wj_tdm_set_ch_num(tdm_base, 1U);
                break;

            case 2U:
                wj_tdm_set_ch_num(tdm_base, 2U);
                break;

            case 3U:
                wj_tdm_set_ch_num(tdm_base, 3U);
                break;
            
            default:
                ret = CSI_ERROR;
            break;
        }

    }

    return ret;   
}

csi_error_t drv_tdm_ch_free(drv_tdm_t *tdm, drv_tdm_ch_t *ch)
{
    CSI_PARAM_CHK(tdm, CSI_ERROR);
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_ERROR;
    drv_tdm_ch_t *alloced_ch;
    slist_for_each_entry(&tdm->head, alloced_ch, drv_tdm_ch_t, next) {
        if (alloced_ch->ch_idx == ch->ch_idx) {
            ch->rx_dma = NULL;
            ch->rx_buf = NULL;
            ch->state.error = 0U;
            ch->state.readable = 0U;
            ch->priv = (void *)0U;
            ch->callback = NULL;
            slist_remove(&ch->next, &tdm->head);
            ret = CSI_OK;
            break;
        }
    }
    return ret;    
}


csi_error_t drv_tdm_ch_get_state(drv_tdm_ch_t *ch, csi_state_t *state)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    CSI_PARAM_CHK(state, CSI_ERROR);
    *state = ch->state;
    return CSI_OK;
}

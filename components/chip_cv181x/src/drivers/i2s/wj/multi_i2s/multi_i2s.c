/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */
#include <stdio.h>
#include <string.h>
#include <csi_config.h>
#include <sys_clk.h>
#include <drv/multi_i2s.h>
#include <drv/cvi_irq.h>
#include <wj_multi_i2s_ll.h>
#include <drv/list.h>

extern uint16_t i2s_tx_hs_num[];
extern uint16_t i2s_rx_hs_num[];

DEFINE_DESC_BUF(i2s_ch0_send_buf, 6);
DEFINE_DESC_BUF(i2s_ch1_send_buf, 6);
DEFINE_DESC_BUF(i2s_ch2_send_buf, 6);
DEFINE_DESC_BUF(i2s_ch3_send_buf, 6);
DEFINE_DESC_BUF(i2s_ch4_send_buf, 6);

DEFINE_DESC_BUF(i2s_ch0_receive_buf, 6);
DEFINE_DESC_BUF(i2s_ch1_receive_buf, 6);
DEFINE_DESC_BUF(i2s_ch2_receive_buf, 6);
DEFINE_DESC_BUF(i2s_ch3_receive_buf, 6);
DEFINE_DESC_BUF(i2s_ch4_receive_buf, 6);

#ifdef CONFIG_XIP
#define I2S_CODE_IN_RAM __attribute__((section(".ram.code")))
#else
#define I2S_CODE_IN_RAM
#endif

I2S_CODE_IN_RAM static void wj_i2s_dma_event_cb(csi_dma_ch_t *dma, csi_dma_event_t event, void *arg)
{
    CSI_PARAM_CHK_NORETVAL(dma);
    drv_multi_i2s_ch_t *ch = (drv_multi_i2s_ch_t *)dma->parent;

    if (event == DMA_EVENT_TRANSFER_ERROR) {/* DMA transfer ERROR */
        if (ch->tx_dma->ch_id == dma->ch_id) {
            /* to do tx error action */
            ch->state.error = 1U;

            if (ch->callback) {
                ch->callback(ch, MULTI_I2S_EVENT_ERROR, ch->arg);
            }
        } else if (ch->rx_dma->ch_id == dma->ch_id) {
            /* to do rx error action */
            ch->state.error = 1U;

            if (ch->callback) {
                ch->callback(ch, MULTI_I2S_EVENT_ERROR, ch->arg);
            }
        }

    } else if (event == DMA_EVENT_TRANSFER_BLOCK_DONE) {
        if ((ch->tx_dma != NULL) && (ch->tx_dma->ch_id == dma->ch_id)) {
            /* to do tx action */

            if (ch->tx_buf->data_len == 0U) {
                ch->tx_buf->data_len = 0U;

                if (ch->callback) {
                    memset(ch->tx_buf->buffer, 0U, ch->tx_buf->size);     ///< tx buffer empty
                    soc_dcache_clean_invalid_range((unsigned long)(ch->tx_buf->buffer), ch->tx_buf->size);
                    ch->callback(ch, MULTI_I2S_EVENT_TX_BUFFER_EMPTY, ch->arg);
                    ch->tx_buf->read = 0U;
                }

            } else {
                uint32_t read_len = ch->tx_period;
                ch->tx_buf->read = (ch->tx_buf->read + read_len) % ch->tx_buf->size;
                ch->tx_buf->data_len -= read_len;

                uint32_t next_read_pos = (ch->tx_buf->read + read_len) % ch->tx_buf->size;
                soc_dcache_clean_invalid_range((unsigned long)(ch->tx_buf->buffer + next_read_pos), ch->tx_period);

                if (ch->callback) {
                    ch->callback(ch, MULTI_I2S_EVENT_SEND_COMPLETE, ch->arg);
                }
            }
        } else {
            /* to do rx action */
            uint32_t write_len = ch->rx_period;
            ch->rx_buf->write = (ch->rx_buf->write + write_len) % ch->rx_buf->size;
            ch->rx_buf->data_len += write_len;

            uint32_t next_write_pos = (ch->rx_buf->write + write_len) % ch->rx_buf->size;
            soc_dcache_clean_invalid_range((unsigned long)(ch->rx_buf->buffer + next_write_pos), ch->rx_period);

            if (ch->callback && ch->rx_buf->data_len != ch->rx_buf->size) {
                ch->callback(ch, MULTI_I2S_EVENT_RECEIVE_COMPLETE, ch->arg);
            } else {
                ch->callback(ch, MULTI_I2S_EVENT_RX_BUFFER_FULL, ch->arg);
            }
        }

    }
}

csi_error_t drv_mulit_i2s_init(drv_multi_i2s_t *i2s, uint32_t idx)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    slist_init(&i2s->head);
    ret = target_get(DEV_WJ_I2S_TAG, idx, &i2s->dev);
    return ret;
}

void drv_mulit_i2s_uninit(drv_multi_i2s_t *i2s)
{
    CSI_PARAM_CHK_NORETVAL(i2s);
    wj_i2s_multi_regs_t *i2s_base;
    i2s_base = (wj_i2s_multi_regs_t *)HANDLE_REG_BASE(i2s);
    wj_i2s_disable(i2s_base);
    slist_init(&i2s->head);
}

void drv_mulit_i2s_enable(drv_multi_i2s_t *i2s, bool enable)
{
    CSI_PARAM_CHK_NORETVAL(i2s);
    wj_i2s_multi_regs_t *i2s_base;
    i2s_base = (wj_i2s_multi_regs_t *)HANDLE_REG_BASE(i2s);

    if (enable) {
        wj_i2s_enable(i2s_base);
    } else {
        wj_i2s_disable(i2s_base);
    }
}

csi_error_t drv_mulit_i2s_format(drv_multi_i2s_t *i2s, drv_multi_i2s_format_t *format)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    CSI_PARAM_CHK(format, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    uint32_t div;
    wj_i2s_multi_regs_t *i2s_base;
    i2s_base = (wj_i2s_multi_regs_t *)HANDLE_REG_BASE(i2s);

    switch (format->mode) {
        case MULTI_I2S_MODE_MASTER:
            wj_i2s_receive_mode_master(i2s_base);
            wj_i2s_transmit_mode_master(i2s_base);
            break;

        case MULTI_I2S_MODE_SLAVE:
            wj_i2s_receive_mode_slave_sclk_src_clk(i2s_base);
            wj_i2s_transmit_mode_slave_sclk_src_clk(i2s_base);
            wj_i2s_receive_mode_slave(i2s_base);
            wj_i2s_transmit_mode_slave(i2s_base);
            break;

        default:
            ret = CSI_ERROR;
            break;
    }

    switch (format->protocol) {
        case MULTI_I2S_PROTOCOL_I2S:
            wj_i2s_receive_mode_format_i2s(i2s_base);
            wj_i2s_transmit_mode_format_i2s(i2s_base);
            break;

        case MULTI_I2S_PROTOCOL_MSB_JUSTIFIED:
            wj_i2s_receive_mode_format_left_justified(i2s_base);
            wj_i2s_transmit_mode_format_left_justified(i2s_base);
            break;

        case MULTI_I2S_PROTOCOL_LSB_JUSTIFIED:
            wj_i2s_receive_mode_format_right_justified(i2s_base);
            wj_i2s_transmit_mode_format_right_justified(i2s_base);
            break;

        case MULTI_I2S_PROTOCOL_PCM:
            wj_i2s_receive_mode_format_pcm(i2s_base);
            wj_i2s_transmit_mode_format_pcm(i2s_base);
            break;

        default:
            ret = CSI_ERROR;
            break;
    }

    switch (format->width) {
        case MULTI_I2S_SAMPLE_WIDTH_16BIT:
            wj_i2s_data_width_mode(i2s_base, WJ_I2S_DATA_WIDTH_16BIT);
            break;

        case MULTI_I2S_SAMPLE_WIDTH_24BIT:
            wj_i2s_data_width_mode(i2s_base, WJ_I2S_DATA_WIDTH_24BIT);
            break;

        case MULTI_I2S_SAMPLE_WIDTH_32BIT:
            wj_i2s_data_width_mode(i2s_base, WJ_I2S_DATA_WIDTH_32BIT);
            break;

        default:
            ret = CSI_UNSUPPORTED;
            break;
    }

    if (format->mode == MULTI_I2S_MODE_MASTER) {
        uint32_t div0 = 0;
        div = soc_get_i2s_freq((uint32_t)i2s->dev.idx) / format->mclk_nfs;
        div0 = (div + div % format->rate) / format->rate;

        wj_i2s_set_div_sclk(i2s_base, div0);
        wj_i2s_set_div_ref_clk(i2s_base, 0);

    } else {
        wj_i2s_receive_mode_audio_input_rate_detected_hardware(i2s_base);   ///< if i2s is slave work mode, hardware automatically detects the audio input rate
    }

    if (format->polarity) {
        wj_i2s_transmit_mode_channel_polarity_high_for_left(i2s_base);
        wj_i2s_receive_mode_channel_polarity_high_for_left(i2s_base);
    } else {
        wj_i2s_transmit_mode_channel_polarity_low_for_left(i2s_base);
        wj_i2s_receive_mode_channel_polarity_low_for_left(i2s_base);
    }

    switch (format->sclk_nfs) {
        case MULTI_I2S_SCLK_16FS:
            wj_i2s_receive_mode_sclk_sel_16_fs(i2s_base);
            break;

        case MULTI_I2S_SCLK_32FS:
            wj_i2s_receive_mode_sclk_sel_32_fs(i2s_base);
            break;

        case MULTI_I2S_SCLK_48FS:
            wj_i2s_receive_mode_sclk_sel_48_fs(i2s_base);
            format->mclk_nfs = MULTI_I2S_MCLK_384FS;           ///< if SCLK_SEL=01 then mclk_o=384*fs，select MCLK_SEL:384fs
            break;

        case MULTI_I2S_SCLK_64FS:
            wj_i2s_receive_mode_sclk_sel_64_fs(i2s_base);
            break;

        default:
            break;
    }

    if (format->mclk_nfs == MULTI_I2S_MCLK_384FS) {
        wj_i2s_receive_mode_mclk_sel_384_fs(i2s_base);
    } else {
        wj_i2s_receive_mode_mclk_sel_256_fs(i2s_base);
    }

    return ret;
}

csi_error_t drv_mulit_i2s_all_ch_send_pause(drv_multi_i2s_t *i2s)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    csi_error_t ret = CSI_ERROR;
    drv_multi_i2s_ch_t *alloced_ch;
    slist_for_each_entry(&i2s->head, alloced_ch, drv_multi_i2s_ch_t, next) {
        dma_ch_pause(alloced_ch->tx_dma);
        ret = CSI_OK;
    }
    return ret;
}

csi_error_t drv_mulit_i2s_all_ch_send_resume(drv_multi_i2s_t *i2s)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    csi_error_t ret = CSI_ERROR;
    drv_multi_i2s_ch_t *alloced_ch;
    slist_for_each_entry(&i2s->head, alloced_ch, drv_multi_i2s_ch_t, next) {
        dma_ch_resume(alloced_ch->tx_dma);
        ret = CSI_OK;
    }
    return ret;
}

csi_error_t drv_mulit_i2s_all_ch_send_start(drv_multi_i2s_t *i2s)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    drv_multi_i2s_ch_t *alloced_ch;
    slist_for_each_entry(&i2s->head, alloced_ch, drv_multi_i2s_ch_t, next) {
        csi_dma_ch_config_t config;
        memset(&config, 0, sizeof(csi_dma_ch_config_t));
        wj_i2s_multi_regs_t *i2s_base;
        i2s_base = (wj_i2s_multi_regs_t *)HANDLE_REG_BASE(alloced_ch->multi_i2s);
        csi_dma_ch_t *dma_ch = (csi_dma_ch_t *)alloced_ch->tx_dma;

        wj_i2s_clear_irq(i2s_base, alloced_ch->ch_idx, 0x7FU);
        wj_i2s_set_transmit_mode(i2s_base, 1U);
        wj_i2s_set_transmit_dma_enable(i2s_base);
        wj_i2s_set_transmit_dma_data_num_level(i2s_base, 16U);

        config.src_inc = DMA_ADDR_INC;
        config.dst_inc = DMA_ADDR_CONSTANT;
        config.src_tw = DMA_DATA_WIDTH_32_BITS;
        config.dst_tw = DMA_DATA_WIDTH_32_BITS;

        config.group_len = 64U;
        config.trans_dir = DMA_MEM2PERH;
        config.src_reload_en = 0U;
        config.dst_reload_en = 0U;
        config.half_int_en = 0U;
        /* config for etb */
        config.handshake = i2s_tx_hs_num[alloced_ch->multi_i2s->dev.idx];

        /* config for dma cyclic mode*/
        config.cyclic_en = 1U;
        config.cyclic_config.buffer = alloced_ch->tx_buf->buffer;
        config.cyclic_config.buffer_bytes = alloced_ch->tx_buf->size;
        config.cyclic_config.period_bytes = alloced_ch->tx_period;

        switch (alloced_ch->ch_idx) {
            case 0U:
                config.cyclic_config.lli_buf = i2s_ch0_send_buf;
                config.cyclic_config.dstaddr = (void *) & (i2s_base->I2S_DR);
                break;

            case 1U:
                config.cyclic_config.lli_buf = i2s_ch1_send_buf;
                config.cyclic_config.dstaddr = (void *) & (i2s_base->I2S_DR1);
                break;

            case 2U:
                config.cyclic_config.lli_buf = i2s_ch2_send_buf;
                config.cyclic_config.dstaddr = (void *) & (i2s_base->I2S_DR2);
                break;

            case 3U:
                config.cyclic_config.lli_buf = i2s_ch3_send_buf;
                config.cyclic_config.dstaddr = (void *) & (i2s_base->I2S_DR3);
                break;

            case 4U:
                config.cyclic_config.lli_buf = i2s_ch4_send_buf;
                config.cyclic_config.dstaddr = (void *) & (i2s_base->I2S_DR4);
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

        soc_dcache_clean_invalid_range((unsigned long)(alloced_ch->tx_buf->buffer), alloced_ch->tx_buf->size);
        csi_dma_ch_start(dma_ch, NULL, NULL, 0);
        alloced_ch->state.writeable = 1U;
    }
    return ret;
}

csi_error_t drv_mulit_i2s_all_ch_receive_start(drv_multi_i2s_t *i2s)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    drv_multi_i2s_ch_t *alloced_ch;
    slist_for_each_entry(&i2s->head, alloced_ch, drv_multi_i2s_ch_t, next) {
        csi_dma_ch_config_t config;
        memset(&config, 0, sizeof(csi_dma_ch_config_t));
        wj_i2s_multi_regs_t *i2s_base;
        i2s_base = (wj_i2s_multi_regs_t *)HANDLE_REG_BASE(alloced_ch->multi_i2s);
        csi_dma_ch_t *dma_ch = (csi_dma_ch_t *)alloced_ch->rx_dma;

        wj_i2s_clear_irq(i2s_base, alloced_ch->ch_idx, 0x7FU);
        wj_i2s_set_receive_mode(i2s_base, 1U);
        wj_i2s_set_receive_dma_enable(i2s_base);
        wj_i2s_set_receive_dma_data_num_level(i2s_base, 16U);

        config.src_inc = DMA_ADDR_CONSTANT;
        config.dst_inc = DMA_ADDR_INC;
        config.src_tw = DMA_DATA_WIDTH_32_BITS;
        config.dst_tw = DMA_DATA_WIDTH_32_BITS;
        /* config for wj_dma */
        config.group_len = 64U;
        config.trans_dir = DMA_PERH2MEM;
        config.src_reload_en = 0U;
        config.dst_reload_en = 0U;
        config.half_int_en = 0U;
        /* config for etb */
        config.handshake = i2s_rx_hs_num[alloced_ch->multi_i2s->dev.idx];

        /* config for dma cyclic mode*/
        config.cyclic_en = 1U;
        config.cyclic_config.buffer = alloced_ch->rx_buf->buffer;
        config.cyclic_config.buffer_bytes = alloced_ch->rx_buf->size;
        config.cyclic_config.period_bytes = alloced_ch->rx_period;

        switch (alloced_ch->ch_idx) {
            case 0U:
                config.cyclic_config.lli_buf = i2s_ch0_receive_buf;
                config.cyclic_config.srcaddr = (void *) & (i2s_base->I2S_DR);
                break;

            case 1U:
                config.cyclic_config.lli_buf = i2s_ch1_receive_buf;
                config.cyclic_config.srcaddr = (void *) & (i2s_base->I2S_DR1);
                break;

            case 2U:
                config.cyclic_config.lli_buf = i2s_ch2_receive_buf;
                config.cyclic_config.srcaddr = (void *) & (i2s_base->I2S_DR2);
                break;

            case 3U:
                config.cyclic_config.lli_buf = i2s_ch3_receive_buf;
                config.cyclic_config.srcaddr = (void *) & (i2s_base->I2S_DR3);
                break;

            case 4U:
                config.cyclic_config.lli_buf = i2s_ch4_receive_buf;
                config.cyclic_config.srcaddr = (void *) & (i2s_base->I2S_DR4);
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
        csi_dma_ch_start(alloced_ch->rx_dma, NULL, NULL, 0);
        alloced_ch->state.readable = 1U;
    }
    return ret;
}

void drv_mulit_i2s_all_ch_send_stop(drv_multi_i2s_t *i2s)
{
    CSI_PARAM_CHK_NORETVAL(i2s);
    drv_multi_i2s_ch_t *alloced_ch;
    slist_for_each_entry(&i2s->head, alloced_ch, drv_multi_i2s_ch_t, next) {
        csi_dma_ch_stop(alloced_ch->tx_dma);
        ringbuffer_reset(alloced_ch->tx_buf);
        alloced_ch->state.writeable = 0U;
    }
}

void drv_mulit_i2s_all_ch_receive_stop(drv_multi_i2s_t *i2s)
{
    CSI_PARAM_CHK_NORETVAL(i2s);
    drv_multi_i2s_ch_t *alloced_ch;
    slist_for_each_entry(&i2s->head, alloced_ch, drv_multi_i2s_ch_t, next) {
        csi_dma_ch_stop(alloced_ch->rx_dma);
        ringbuffer_reset(alloced_ch->rx_buf);
        alloced_ch->state.readable = 0U;
    }
}

csi_error_t drv_mulit_i2s_ch_get_state(drv_multi_i2s_ch_t *ch, csi_state_t *state)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    CSI_PARAM_CHK(state, CSI_ERROR);
    *state = ch->state;
    return CSI_OK;
}

csi_error_t drv_mulit_i2s_ch_alloc(drv_multi_i2s_t *i2s, drv_multi_i2s_ch_t *ch, uint32_t ch_idx)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    drv_multi_i2s_ch_t *alloced_ch;
    slist_for_each_entry(&i2s->head, alloced_ch, drv_multi_i2s_ch_t, next) {
        if (alloced_ch->ch_idx == ch_idx) {
            ret = CSI_ERROR;
            break;
        }
    }

    if (ret == CSI_OK) {
        slist_add(&ch->next, &i2s->head);
        ch->ch_idx = ch_idx;
        ch->multi_i2s = i2s;
        ch->tx_buf = NULL;
        ch->rx_buf = NULL;
        ch->tx_dma = NULL;
        ch->rx_dma = NULL;
#if 0//debug ap alsa
        wj_i2s_multi_regs_t *i2s_base = (wj_i2s_multi_regs_t *)HANDLE_REG_BASE(i2s);
        wj_i2s_clear_irq(i2s_base, ch_idx, 0x7FU);    ///< clear ch all irq
        wj_i2s_enable_ch(i2s_base, ch_idx);
#endif
    }

    return ret;
}

csi_error_t drv_mulit_i2s_ch_attach_callback(drv_multi_i2s_ch_t *ch, void *callback, void *arg)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    ch->callback = callback;
    ch->arg = arg;
    return ret;
}

void drv_mulit_i2s_ch_detach_callback(drv_multi_i2s_ch_t *ch)
{
    CSI_PARAM_CHK_NORETVAL(ch);
    ch->callback = NULL;
    ch->arg = NULL;
}

csi_error_t drv_mulit_i2s_ch_free(drv_multi_i2s_t *i2s, drv_multi_i2s_ch_t *ch)
{
    CSI_PARAM_CHK(i2s, CSI_ERROR);
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_ERROR;
    drv_multi_i2s_ch_t *alloced_ch;
    slist_for_each_entry(&i2s->head, alloced_ch, drv_multi_i2s_ch_t, next) {
        if (alloced_ch->ch_idx == ch->ch_idx) {
            wj_i2s_multi_regs_t *i2s_base = (wj_i2s_multi_regs_t *)HANDLE_REG_BASE(i2s);
            ch->tx_buf = NULL;
            ch->rx_buf = NULL;
            ch->tx_dma = NULL;
            ch->rx_dma = NULL;
            wj_i2s_disable_ch(i2s_base, ch->ch_idx);
            slist_remove(&ch->next, &i2s->head);
            ret = CSI_OK;
            break;
        }
    }
    return ret;
}

csi_error_t drv_mulit_i2s_ch_rx_link_dma(drv_multi_i2s_ch_t *ch, csi_dma_ch_t *rx_dma)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    if (rx_dma != NULL) {
        rx_dma->parent = ch->multi_i2s;
        ret = csi_dma_ch_alloc(rx_dma, -1, -1);
        rx_dma->parent = ch;

        if (ret == CSI_OK) {
            csi_dma_ch_attach_callback(rx_dma, wj_i2s_dma_event_cb, NULL);
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

csi_error_t drv_mulit_i2s_ch_tx_link_dma(drv_multi_i2s_ch_t *ch, csi_dma_ch_t *tx_dma)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    if (tx_dma != NULL) {
        tx_dma->parent = ch->multi_i2s;
        ret = csi_dma_ch_alloc(tx_dma, -1, -1);
        tx_dma->parent = ch;

        if (ret == CSI_OK) {
            csi_dma_ch_attach_callback(tx_dma, wj_i2s_dma_event_cb, NULL);
            ch->tx_dma = tx_dma;
        } else {
            tx_dma->parent = NULL;
            ret = CSI_ERROR;
        }
    } else {
        if (ch->tx_dma) {
            csi_dma_ch_detach_callback(ch->tx_dma);
            csi_dma_ch_free(ch->tx_dma);
            ch->tx_dma = NULL;
        } else {
            ret = CSI_ERROR;
        }
    }

    return ret;
}

void drv_mulit_i2s_ch_rx_set_buffer(drv_multi_i2s_ch_t *ch, ringbuffer_t *buffer)
{
    CSI_PARAM_CHK_NORETVAL(ch);
    CSI_PARAM_CHK_NORETVAL(buffer);
    ch->rx_buf = buffer;
}

void drv_mulit_i2s_ch_tx_set_buffer(drv_multi_i2s_ch_t *ch, ringbuffer_t *buffer)
{
    CSI_PARAM_CHK_NORETVAL(ch);
    CSI_PARAM_CHK_NORETVAL(buffer);
    ch->tx_buf = buffer;
}

csi_error_t drv_mulit_i2s_ch_rx_set_period(drv_multi_i2s_ch_t *ch, uint32_t period)
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

csi_error_t drv_mulit_i2s_ch_tx_set_period(drv_multi_i2s_ch_t *ch, uint32_t period)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    if (period == 0U) {
        ret = CSI_ERROR;
    } else {
        if ((ch->tx_buf->size % period) != 0U) {
            ret = CSI_ERROR;
        } else {
            ch->tx_period = period;
        }
    }

    return ret;
}

uint32_t drv_mulit_i2s_ch_rx_buffer_avail(drv_multi_i2s_ch_t *ch)
{
    CSI_PARAM_CHK(ch, 0U);
    return ringbuffer_avail(ch->rx_buf);
}

uint32_t drv_mulit_i2s_ch_rx_buffer_remain(drv_multi_i2s_ch_t *ch)
{
    CSI_PARAM_CHK(ch, 0U);
    return ringbuffer_len(ch->rx_buf);
}

csi_error_t drv_mulit_i2s_ch_rx_buffer_reset(drv_multi_i2s_ch_t *ch)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_ERROR;
    ringbuffer_reset(ch->rx_buf);
    return ret;
}

uint32_t drv_mulit_i2s_ch_tx_buffer_avail(drv_multi_i2s_ch_t *ch)
{
    CSI_PARAM_CHK(ch, 0U);
    return ringbuffer_avail(ch->tx_buf);
}

uint32_t drv_mulit_i2s_ch_tx_buffer_remain(drv_multi_i2s_ch_t *ch)
{
    CSI_PARAM_CHK(ch, 0U);
    return ringbuffer_len(ch->tx_buf);
}

csi_error_t drv_mulit_i2s_ch_tx_buffer_reset(drv_multi_i2s_ch_t *ch)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_ERROR;
    ringbuffer_reset(ch->tx_buf);
    return ret;
}

uint32_t drv_mulit_i2s_ch_send_async(drv_multi_i2s_ch_t *ch, const void *data, uint32_t size)
{
    CSI_PARAM_CHK(ch, 0U);
    CSI_PARAM_CHK(data, 0U);
    uint32_t write_len;
    uint32_t result = csi_irq_save();
    write_len = ringbuffer_in(ch->tx_buf, data, size);
    csi_irq_restore(result);
    return write_len;
}

uint32_t drv_mulit_i2s_ch_receive_async(drv_multi_i2s_ch_t *ch, void *data, uint32_t size)
{
    CSI_PARAM_CHK(ch, 0U);
    CSI_PARAM_CHK(data, 0U);
    uint32_t read_len;
    uint32_t result = csi_irq_save();
    read_len = ringbuffer_out(ch->rx_buf, (void *)data, size);
    csi_irq_restore(result);
    return read_len;
}

csi_error_t drv_mulit_i2s_ch_send_pause(drv_multi_i2s_ch_t *ch)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    CSI_PARAM_CHK(ch->tx_dma, CSI_ERROR);
    dma_ch_pause(ch->tx_dma);
    return CSI_OK;
}

csi_error_t drv_mulit_i2s_ch_send_resume(drv_multi_i2s_ch_t *ch)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    CSI_PARAM_CHK(ch->tx_dma, CSI_ERROR);
    dma_ch_resume(ch->tx_dma);
    return CSI_OK;
}

csi_error_t drv_mulit_i2s_ch_send_start(drv_multi_i2s_ch_t *ch)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    csi_dma_ch_config_t config;
    memset(&config, 0, sizeof(csi_dma_ch_config_t));
    wj_i2s_multi_regs_t *i2s_base;
    i2s_base = (wj_i2s_multi_regs_t *)HANDLE_REG_BASE(ch->multi_i2s);
    csi_dma_ch_t *dma_ch = (csi_dma_ch_t *)ch->tx_dma;

    wj_i2s_clear_irq(i2s_base, ch->ch_idx, 0x7FU);
    wj_i2s_set_transmit_mode(i2s_base, 1U);
    wj_i2s_set_transmit_dma_enable(i2s_base);
    wj_i2s_set_transmit_dma_data_num_level(i2s_base, 16U);

    config.src_inc = DMA_ADDR_INC;
    config.dst_inc = DMA_ADDR_CONSTANT;
    config.src_tw = DMA_DATA_WIDTH_32_BITS;
    config.dst_tw = DMA_DATA_WIDTH_32_BITS;

    config.group_len = 64U;
    config.trans_dir = DMA_MEM2PERH;
    config.src_reload_en = 0U;
    config.dst_reload_en = 0U;
    config.half_int_en = 0U;
    /* config for etb */
    config.handshake = i2s_tx_hs_num[ch->multi_i2s->dev.idx];

    /* config for dma cyclic mode*/
    config.cyclic_en = 1U;
    config.cyclic_config.buffer = ch->tx_buf->buffer;
    config.cyclic_config.buffer_bytes = ch->tx_buf->size;
    config.cyclic_config.period_bytes = ch->tx_period;

    switch (ch->ch_idx) {
        case 0U:
            config.cyclic_config.lli_buf = i2s_ch0_send_buf;
            config.cyclic_config.dstaddr = (void *) & (i2s_base->I2S_DR);
            break;

        case 1U:
            config.cyclic_config.lli_buf = i2s_ch1_send_buf;
            config.cyclic_config.dstaddr = (void *) & (i2s_base->I2S_DR1);
            break;

        case 2U:
            config.cyclic_config.lli_buf = i2s_ch2_send_buf;
            config.cyclic_config.dstaddr = (void *) & (i2s_base->I2S_DR2);
            break;

        case 3U:
            config.cyclic_config.lli_buf = i2s_ch3_send_buf;
            config.cyclic_config.dstaddr = (void *) & (i2s_base->I2S_DR3);
            break;

        case 4U:
            config.cyclic_config.lli_buf = i2s_ch4_send_buf;
            config.cyclic_config.dstaddr = (void *) & (i2s_base->I2S_DR4);
            break;

        default:
            ret = CSI_ERROR;
            break;
    }

    ret = csi_dma_ch_config(dma_ch, &config);

    soc_dcache_clean_invalid_range((unsigned long)(ch->tx_buf->buffer), ch->tx_buf->size);
    csi_dma_ch_start(ch->tx_dma, NULL, NULL, 0); //enable
    ch->state.writeable = 1U;
    return ret;
}

csi_error_t drv_mulit_i2s_ch_receive_start(drv_multi_i2s_ch_t *ch)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    csi_dma_ch_config_t config;
    memset(&config, 0, sizeof(csi_dma_ch_config_t));
    wj_i2s_multi_regs_t *i2s_base;
    i2s_base = (wj_i2s_multi_regs_t *)HANDLE_REG_BASE(ch->multi_i2s);
    csi_dma_ch_t *dma_ch = (csi_dma_ch_t *)ch->rx_dma;

    wj_i2s_clear_irq(i2s_base, ch->ch_idx, 0x7FU);
    wj_i2s_set_receive_mode(i2s_base, 1U);
    wj_i2s_set_receive_dma_enable(i2s_base);
    wj_i2s_set_receive_dma_data_num_level(i2s_base, 16U);

    config.src_inc = DMA_ADDR_CONSTANT;
    config.dst_inc = DMA_ADDR_INC;
    config.src_tw = DMA_DATA_WIDTH_32_BITS;
    config.dst_tw = DMA_DATA_WIDTH_32_BITS;
    /* config for wj_dma */
    config.group_len = 64U;
    config.trans_dir = DMA_PERH2MEM;
    config.src_reload_en = 0U;
    config.dst_reload_en = 0U;
    config.half_int_en = 0U;
    /* config for etb */
    config.handshake = i2s_rx_hs_num[ch->multi_i2s->dev.idx];

    /* config for dma cyclic mode*/
    config.cyclic_en = 1U;
    config.cyclic_config.buffer = ch->rx_buf->buffer;
    config.cyclic_config.buffer_bytes = ch->rx_buf->size;
    config.cyclic_config.period_bytes = ch->rx_period;

    switch (ch->ch_idx) {
        case 0U:
            config.cyclic_config.lli_buf = i2s_ch0_receive_buf;
            config.cyclic_config.srcaddr = (void *) & (i2s_base->I2S_DR);
            break;

        case 1U:
            config.cyclic_config.lli_buf = i2s_ch1_receive_buf;
            config.cyclic_config.srcaddr = (void *) & (i2s_base->I2S_DR1);
            break;

        case 2U:
            config.cyclic_config.lli_buf = i2s_ch2_receive_buf;
            config.cyclic_config.srcaddr = (void *) & (i2s_base->I2S_DR2);
            break;

        case 3U:
            config.cyclic_config.lli_buf = i2s_ch3_receive_buf;
            config.cyclic_config.srcaddr = (void *) & (i2s_base->I2S_DR3);
            break;

        case 4U:
            config.cyclic_config.lli_buf = i2s_ch4_receive_buf;
            config.cyclic_config.srcaddr = (void *) & (i2s_base->I2S_DR4);
            break;

        default:
            ret = CSI_ERROR;
            break;
    }

    ret = csi_dma_ch_config(dma_ch, &config);

    soc_dcache_clean_invalid_range((unsigned long)(ch->rx_buf->buffer), ch->rx_buf->size);
    csi_dma_ch_start(ch->rx_dma, NULL, NULL, 0);
    ch->state.readable = 1U;
    return ret;
}

void drv_mulit_i2s_ch_send_stop(drv_multi_i2s_ch_t *ch)
{
    CSI_PARAM_CHK_NORETVAL(ch);
    csi_dma_ch_stop(ch->tx_dma);
    ringbuffer_reset(ch->tx_buf);
    ch->state.writeable = 0U;
}

void drv_mulit_i2s_ch_receive_stop(drv_multi_i2s_ch_t *ch)
{
    CSI_PARAM_CHK_NORETVAL(ch);
    csi_dma_ch_stop(ch->rx_dma);
    ringbuffer_reset(ch->rx_buf);
    ch->state.readable = 0U;
}

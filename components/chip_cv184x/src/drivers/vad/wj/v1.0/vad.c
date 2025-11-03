/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

#include <string.h>
#include <sys_clk.h>
#include <drv/vad.h>
#include <drv/cvi_irq.h>
#include <drv/dma.h>
#include <drv/tick.h>
#include <drv/list.h>
#include <drv/porting.h>
#include <wj_vad_ll.h>

extern uint16_t vad_rx_hs_num[];

volatile uint32_t dma_flag = 0U;

static void wj_vad_irq_hander(void *arg);
static void wj_vad_dma_event_cb(csi_dma_ch_t *dma, csi_dma_event_t event, void *arg);

csi_error_t drv_vad_init(drv_vad_t *vad, uint32_t idx)
{
    CSI_PARAM_CHK(vad, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    ret = target_get(DEV_WJ_VAD_TAG, idx, &vad->dev);

    if (ret == CSI_OK) {
        wj_vad_regs_t *vad_base = (wj_vad_regs_t *)HANDLE_REG_BASE(vad);
        vad->callback = NULL;
        wj_vad_disable_dai(vad_base);
        wj_vad_disable(vad_base);
    }

    return ret;
}

void drv_vad_uninit(drv_vad_t *vad)
{
    CSI_PARAM_CHK_NORETVAL(vad);
    wj_vad_regs_t *vad_base = (wj_vad_regs_t *)HANDLE_REG_BASE(vad);
    wj_vad_disable_dai(vad_base);
    wj_vad_disable(vad_base);
    vad->callback = NULL;
    csi_irq_disable((uint32_t)vad->dev.irq_num);
    csi_irq_detach((uint32_t)vad->dev.irq_num);
}

void drv_vad_set_algorithm_parameters(drv_vad_t *vad, uint32_t zcr_l, uint32_t zcr_h, uint32_t ste_l, uint32_t ste_h)
{
    CSI_PARAM_CHK_NORETVAL(vad);
    wj_vad_regs_t *vad_base = (wj_vad_regs_t *)HANDLE_REG_BASE(vad);
    wj_vad_set_zcr_high_threshold(vad_base, zcr_h);
    wj_vad_set_zcr_low_threshold(vad_base, zcr_l);
    wj_vad_set_ste_high_threshold(vad_base, ste_h);
    wj_vad_set_ste_low_threshold(vad_base, ste_l);
}

csi_error_t drv_vad_format(drv_vad_t *vad, drv_vad_format_t *format)
{
    CSI_PARAM_CHK(vad, CSI_ERROR);
    CSI_PARAM_CHK(format, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    wj_vad_regs_t *vad_base = (wj_vad_regs_t *)HANDLE_REG_BASE(vad);

    if (format->dai_mode == VAD_DAI_MODE_I2S) {
        wj_vad_dai_funcmode_i2s(vad_base);

        if (format->vad_i2s_format.i2s_mode == VAD_I2S_MODE_MASTER) {
            wj_vad_dai_mode_i2s_master_mode(vad_base);
        } else if (format->vad_i2s_format.i2s_mode == VAD_I2S_MODE_SLAVE) {
            wj_vad_dai_mode_i2s_slave_mode(vad_base);
        } else {
            ret = CSI_ERROR;
        }


        switch (format->vad_i2s_format.i2s_protocol) {
            case VAD_I2S_PROTOCOL_I2S:
                wj_vad_dai_mode_i2s(vad_base);
                break;

            case VAD_I2S_PROTOCOL_MSB_JUSTIFIED:
                wj_vad_dai_mode_left_justified(vad_base);
                break;

            case VAD_I2S_PROTOCOL_LSB_JUSTIFIED:
                wj_vad_dai_mode_right_justified(vad_base);
                break;

            case VAD_I2S_PROTOCOL_PCM:
                wj_vad_dai_mode_pcm(vad_base);
                break;

            default:
                ret = CSI_ERROR;
                break;
        }

        switch (format->vad_i2s_format.i2s_width) {
            case VAD_I2S_SAMPLE_WIDTH_8BIT:
                wj_vad_dai_mode_i2s_width_8bit(vad_base);
                wj_vad_dai_mode_i2s_256fs_mclk(vad_base);
                break;

            case VAD_I2S_SAMPLE_WIDTH_16BIT:
                wj_vad_dai_mode_i2s_width_16bit(vad_base);
                wj_vad_dai_mode_i2s_256fs_mclk(vad_base);
                break;

            case VAD_I2S_SAMPLE_WIDTH_24BIT:
                wj_vad_dai_mode_i2s_width_24bit(vad_base);
                wj_vad_dai_mode_i2s_384fs_mclk(vad_base);
                break;

            case VAD_I2S_SAMPLE_WIDTH_32BIT:
                wj_vad_dai_mode_i2s_width_32bit(vad_base);
                wj_vad_dai_mode_i2s_256fs_mclk(vad_base);
                break;

            default:
                ret = CSI_ERROR;
                break;
        }

    } else if (format->dai_mode == VAD_DAI_MODE_PDM) {
        wj_vad_dai_funcmode_pdm(vad_base);

        if (format->vad_pdm_format.pdm_mode == VAD_PDM_MODE_LOW_POWER) {
            wj_vad_dai_mode_pdm_low_power_mode(vad_base);
            wj_vad_set_dai_mode_pdm_low_power_gain(vad_base, format->vad_pdm_format.pdm_gain);
        } else if (format->vad_pdm_format.pdm_mode == VAD_PDM_MODE_STANDARD) {
            wj_vad_dai_mode_pdm_standard_mode(vad_base);
            wj_vad_set_dai_mode_pdm_standard_gain(vad_base, format->vad_pdm_format.pdm_gain);
        } else {
            ret = CSI_ERROR;
        }
    } else {
        ret = CSI_ERROR;
    }

    return ret;
}

csi_error_t drv_vad_attach_callback(drv_vad_t *vad, void *callback, void *arg)
{
    CSI_PARAM_CHK(vad, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    vad->callback = callback;
    vad->arg = arg;
    wj_vad_regs_t *vad_base = (wj_vad_regs_t *)HANDLE_REG_BASE(vad);
    wj_vad_enable_vad_trig_irq(vad_base);
    csi_irq_attach((uint32_t)vad->dev.irq_num, &wj_vad_irq_hander, &vad->dev);
    csi_irq_enable((uint32_t)vad->dev.irq_num);
    return ret;
}

void drv_vad_detach_callback(drv_vad_t *vad)
{
    CSI_PARAM_CHK_NORETVAL(vad);
    vad->callback = NULL;
    vad->arg = NULL;
    csi_irq_disable((uint32_t)vad->dev.irq_num);
    csi_irq_detach((uint32_t)vad->dev.irq_num);
}

csi_error_t drv_vad_start_trig_mode(drv_vad_t *vad)
{
    CSI_PARAM_CHK(vad, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    wj_vad_regs_t *vad_base = (wj_vad_regs_t *)HANDLE_REG_BASE(vad);

    wj_vad_set_cache_buffer_size(vad_base, 7U);
    wj_vad_dai_mono_enable(vad_base);
    wj_vad_enable_dai(vad_base);
    mdelay(300);///< delay for dai startup
    wj_vad_set_vad_nume(vad_base, 1U);
    wj_vad_set_vad_deno(vad_base, 2U);
    wj_vad_enable(vad_base);
    return ret;
}

csi_error_t drv_vad_start_recorde_mode(drv_vad_t *vad)
{
    CSI_PARAM_CHK(vad, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    wj_vad_regs_t *vad_base = (wj_vad_regs_t *)HANDLE_REG_BASE(vad);

    wj_vad_set_cache_buffer_size(vad_base, 10U);
    wj_vad_dai_mono_enable(vad_base);
    wj_vad_enable_dai(vad_base);
    wj_vad_disable(vad_base);
    wj_vad_data_trans_enable(vad_base);
    mdelay(300);///< delay for dai startup
    // wj_vad_set_vad_nume(vad_base, 1U);
    // wj_vad_set_vad_deno(vad_base, 2U);
    // wj_vad_enable(vad_base);
    return ret;
}

void drv_vad_restart_trig_mode(drv_vad_t *vad)
{
    CSI_PARAM_CHK_NORETVAL(vad);
    wj_vad_regs_t *vad_base = (wj_vad_regs_t *)HANDLE_REG_BASE(vad);
    wj_vad_disable(vad_base);
    wj_vad_clear_vad_trig_irq(vad_base);
    wj_vad_data_trans_disable(vad_base);
    wj_vad_enable(vad_base);
}

void drv_vad_stop_trig_mode(drv_vad_t *vad)
{
    CSI_PARAM_CHK_NORETVAL(vad);
    wj_vad_regs_t *vad_base = (wj_vad_regs_t *)HANDLE_REG_BASE(vad);
    wj_vad_clear_vad_trig_irq(vad_base);
    wj_vad_disable_dai(vad_base);
    wj_vad_disable(vad_base);
}

csi_error_t drv_vad_ch_get_state(drv_vad_ch_t *ch, csi_state_t *state)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    CSI_PARAM_CHK(state, CSI_ERROR);
    *state = ch->state;
    return CSI_OK;
}

csi_error_t drv_vad_ch_alloc(drv_vad_t *vad, drv_vad_ch_t *ch, uint32_t ch_idx)
{
    CSI_PARAM_CHK(vad, CSI_ERROR);
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    drv_vad_ch_t *alloced_ch;
    slist_for_each_entry(&vad->head, alloced_ch, drv_vad_ch_t, next) {
        if (alloced_ch->ch_idx == ch_idx) {
            ret = CSI_ERROR;
            break;
        }
    }

    if (ret == CSI_OK) {
        slist_add(&ch->next, &vad->head);
        ch->ch_idx = ch_idx;
        ch->vad = vad;

        wj_vad_regs_t *vad_base = (wj_vad_regs_t *)HANDLE_REG_BASE(vad);
        wj_vad_enable_ch(vad_base, ch_idx);
    }

    return ret;
}

void drv_vad_ch_free(drv_vad_t *vad, drv_vad_ch_t *ch)
{
    CSI_PARAM_CHK_NORETVAL(vad);
    CSI_PARAM_CHK_NORETVAL(ch);
    drv_vad_ch_t *alloced_ch;
    slist_for_each_entry(&vad->head, alloced_ch, drv_vad_ch_t, next) {
        if (alloced_ch->ch_idx == ch->ch_idx) {
            wj_vad_regs_t *vad_base = (wj_vad_regs_t *)HANDLE_REG_BASE(vad);
            wj_vad_disable_ch(vad_base, ch->ch_idx);
            slist_remove(&ch->next, &vad->head);
            break;
        }
    }
}

csi_error_t drv_vad_ch_rx_link_dma(drv_vad_ch_t *ch, csi_dma_ch_t *dma)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    if (dma != NULL) {
        dma->parent = ch->vad;
        ret = csi_dma_ch_alloc(dma, -1, -1);
        dma->parent = ch;

        if (ret == CSI_OK) {
            csi_dma_ch_attach_callback(dma, wj_vad_dma_event_cb, NULL);
            ch->dma = dma;
        } else {
            dma->parent = NULL;
            ret = CSI_ERROR;
        }
    } else {
        if (ch->dma) {
            csi_dma_ch_detach_callback(ch->dma);
            csi_dma_ch_free(ch->dma);
            ch->dma = NULL;
        } else {
            ret = CSI_ERROR;
        }
    }

    return ret;
}

uint32_t drv_vad_ch_receive_async(drv_vad_ch_t *ch, void *data, uint32_t size)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    csi_dma_ch_config_t config;
    memset(&config, 0, sizeof(csi_dma_ch_config_t));
    wj_vad_regs_t *vad_base = (wj_vad_regs_t *)HANDLE_REG_BASE(ch->vad);
    csi_dma_ch_t *dma_ch = (csi_dma_ch_t *)ch->dma;

    ch->data = (void *)data;
    ch->size = size;
    ch->state.writeable = 0U;

    wj_vad_set_dma_level(vad_base, 4U); ///< size is 1920

    config.src_inc = DMA_ADDR_CONSTANT;
    config.dst_inc = DMA_ADDR_INC;
    config.src_tw = DMA_DATA_WIDTH_32_BITS;
    config.dst_tw = DMA_DATA_WIDTH_32_BITS;
    /* config for wj_dma */
    config.group_len = 1U;
    config.trans_dir = DMA_PERH2MEM;
    /* config for etb */
    config.handshake = vad_rx_hs_num[ch->ch_idx];

    csi_dma_ch_config(dma_ch, &config);

    // soc_dcache_clean_invalid_range((unsigned long)ch->data, 1920U);

    switch (ch->ch_idx) {
        case 0U:
            csi_dma_ch_start(ch->dma, (uint32_t *) & (vad_base->FIFO0_L_READ), ch->data, 1920U * 4U);
            break;

        case 1U:
            csi_dma_ch_start(ch->dma, (uint32_t *) & (vad_base->FIFO0_R_READ), ch->data, 1920U);
            break;

        case 2U:
            csi_dma_ch_start(ch->dma, (uint32_t *) & (vad_base->FIFO1_L_READ), ch->data, 1920U);
            break;

        case 3U:
            csi_dma_ch_start(ch->dma, (uint32_t *) & (vad_base->FIFO1_R_READ), ch->data, 1920U);
            break;

        case 4U:
            csi_dma_ch_start(ch->dma, (uint32_t *) & (vad_base->FIFO2_L_READ), ch->data, 1920U);
            break;

        case 5U:
            csi_dma_ch_start(ch->dma, (uint32_t *) & (vad_base->FIFO2_R_READ), ch->data, 1920U);
            break;

        case 6U:
            csi_dma_ch_start(ch->dma, (uint32_t *) & (vad_base->FIFO3_L_READ), ch->data, 1920U);
            break;

        case 7U:
            csi_dma_ch_start(ch->dma, (uint32_t *) & (vad_base->FIFO3_R_READ), ch->data, 1920U);
            break;

        default:
            break;
    }

    return ret;
}

uint32_t drv_vad_ch_receive(drv_vad_ch_t *ch, void *data, uint32_t size, uint32_t timeout)
{
    CSI_PARAM_CHK(ch, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);

    csi_error_t ret = CSI_OK;

    csi_dma_ch_config_t config;
    memset(&config, 0, sizeof(csi_dma_ch_config_t));
    wj_vad_regs_t *vad_base = (wj_vad_regs_t *)HANDLE_REG_BASE(ch->vad);
    csi_dma_ch_t *dma_ch = (csi_dma_ch_t *)ch->dma;

    ch->data = (void *)data;
    ch->size = size;
    ch->state.writeable = 0U;

    wj_vad_set_dma_level(vad_base, 4U);

    config.src_inc = DMA_ADDR_CONSTANT;
    config.dst_inc = DMA_ADDR_INC;
    config.src_tw = DMA_DATA_WIDTH_32_BITS;
    config.dst_tw = DMA_DATA_WIDTH_32_BITS;
    /* config for wj_dma */
    config.group_len = 16U;
    config.trans_dir = DMA_PERH2MEM;
    /* config for etb */
    config.handshake = vad_rx_hs_num[ch->ch_idx];

    csi_dma_ch_config(dma_ch, &config);

    uint32_t dma_transfer_num = size / 1920;
    unsigned long data_pos = (unsigned long )ch->data;
    while (dma_transfer_num != 0) {
        
        switch (ch->ch_idx) {
            case 0U:
                csi_dma_ch_start(ch->dma, (uint32_t *) & (vad_base->FIFO0_L_READ), (void *)data_pos, 1920);
                break;

            case 1U:
                csi_dma_ch_start(ch->dma, (uint32_t *) & (vad_base->FIFO0_R_READ), ch->data, 1920U);
                break;

            case 2U:
                csi_dma_ch_start(ch->dma, (uint32_t *) & (vad_base->FIFO1_L_READ), ch->data, 1920U);
                break;

            case 3U:
                csi_dma_ch_start(ch->dma, (uint32_t *) & (vad_base->FIFO1_R_READ), ch->data, 1920U);
                break;

            case 4U:
                csi_dma_ch_start(ch->dma, (uint32_t *) & (vad_base->FIFO2_L_READ), ch->data, 1920U);
                break;

            case 5U:
                csi_dma_ch_start(ch->dma, (uint32_t *) & (vad_base->FIFO2_R_READ), ch->data, 1920U);
                break;

            case 6U:
                csi_dma_ch_start(ch->dma, (uint32_t *) & (vad_base->FIFO3_L_READ), ch->data, 1920U);
                break;

            case 7U:
                csi_dma_ch_start(ch->dma, (uint32_t *) & (vad_base->FIFO3_R_READ), ch->data, 1920U);
                break;

            default:
                break;
        }
        while(!dma_flag);
        dma_flag = 0U;
        dma_transfer_num --;
        data_pos = data_pos + 1920U;
        //printf("recording: %lx\r\n", data_pos);
    }
    soc_dcache_clean_invalid_range((unsigned long)ch->data, size);
    wj_vad_data_trans_disable(vad_base);
    return ret;
    // uint32_t *receive_data = (void *)data;
    // wj_vad_regs_t *vad_base = (wj_vad_regs_t *)HANDLE_REG_BASE(ch->vad);

    // uint8_t data_num = 0;
    // uint32_t received_num = 0;

    // while (received_num < size) {

    //     data_num = wj_vad_get_ch_fifo(vad_base, ch->ch_idx);

    //     while (data_num > 0) {
    //         *(receive_data++) = wj_vad_read_ch_fifo(vad_base, ch->ch_idx);
    //         data_num --;
    //         received_num ++;
    //     }
    // }


    return 0;
}

static void wj_vad_irq_hander(void *arg)
{
    CSI_PARAM_CHK_NORETVAL(arg);
    drv_vad_t *vad = (drv_vad_t *)arg;
    wj_vad_regs_t *vad_base = (wj_vad_regs_t *)HANDLE_REG_BASE(vad);
    uint32_t irq_status = wj_vad_get_vad_trig_irq_status(vad_base);
    wj_vad_clear_vad_trig_irq(vad_base);

    if (irq_status & WJ_VAD_INTR_MFLAG_EN) {
        if (vad->callback) {
            vad->callback(vad, 0U, VAD_EVENT_WAKE_UP, vad->arg);
        }

        //drv_vad_restart_trig_mode(vad);
    }
}

static void wj_vad_dma_event_cb(csi_dma_ch_t *dma, csi_dma_event_t event, void *arg)
{
    CSI_PARAM_CHK_NORETVAL(dma);
    drv_vad_ch_t *vad_ch = (drv_vad_ch_t *)dma->parent;
    //wj_vad_regs_t *vad_base = (wj_vad_regs_t *)HANDLE_REG_BASE(vad_ch->vad);

    if (event == DMA_EVENT_TRANSFER_ERROR) {/* DMA transfer ERROR */
        if (vad_ch->dma->ch_id == dma->ch_id) {
            /* to do tx error action */
            vad_ch->state.error = 1U;

            if (vad_ch->vad->callback) {
                vad_ch->vad->callback(vad_ch->vad, vad_ch->ch_idx, VAD_EVENT_ERROR, vad_ch->vad->arg);
            }
        }
    } else if (event == DMA_EVENT_TRANSFER_DONE) {/* DMA transfer complete */
        if ((vad_ch->dma != NULL) && (vad_ch->dma->ch_id == dma->ch_id)) {
            dma_flag = 1;
            // if (vad_ch->vad->callback) {
            //     soc_dcache_clean_invalid_range((unsigned long)vad_ch->data, 1920 * 4U);
            //     vad_ch->vad->callback(vad_ch->vad, vad_ch->ch_idx, VAD_EVENT_RECEIVE_COMPLETE, vad_ch->vad->arg);
            // }
        }
    }
}
/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     wj_adc.c
 * @brief    CSI Source File for ADC Driver
 * @version  V1.0
 * @date     2020-03-05
 ******************************************************************************/

#include <string.h>

#include <drv/adc.h>
#include <drv/cvi_irq.h>
#include <drv/tick.h>

#include "wj_adc_ll.h"

#define DEFUALT_WJ_ADC_OPERATION_TIMEOUT    1000000U
#define MAX_CHANNEL_NUM                     16U

#define VOID_P_ADD(p, val) do{ uint8_t *temp = (uint8_t *)p; temp += val; p = (void *)temp; }while(0);
#define VOID_P_DEC(p, val) do{ uint8_t *temp = (uint8_t *)p; temp -= val; p = (void *)temp; }while(0);
static void wj_adc_channel_conv_end_irq(csi_adc_t *adc)
{
    wj_adc_regs_t  *adc_base;
    adc_base = (wj_adc_regs_t *)HANDLE_REG_BASE(adc);

    if (adc->num > 0U) {
        // TODO: support other bit width
        *adc->data++ = wj_adc_get_data(adc_base);
        adc->num--;
    }

    if (adc->num == 0U) {
        if (adc->callback) {
            wj_adc_dis_int_one_channel_done(adc_base);
            adc->callback(adc, ADC_EVENT_CONVERT_COMPLETE, adc->arg);
            adc->state.readable = 1U;
        }
    }
}

static void wj_adc_irqhandler(void *args)
{
    csi_adc_t *adc;
    wj_adc_regs_t *adc_base;
    volatile uint32_t intr_status;

    adc         = (csi_adc_t *)args;
    adc_base    = (wj_adc_regs_t *)HANDLE_REG_BASE(adc);
    intr_status = wj_adc_get_int_flag(adc_base);
    wj_adc_clear_int_flag(adc_base, intr_status);

    /* handle end of single channel conversion completed */
    if (intr_status & WJ_ADC_INT_ONE_CONVERT_DONE_EN) {
        wj_adc_channel_conv_end_irq(adc);
    }

    if (intr_status & WJ_ADC_INT_ONE_CONNNEL_DONE_EN) {
        wj_adc_channel_conv_end_irq(adc);
    }

    /* handle overwrite irq */
    if (intr_status & WJ_ADC_INT_ONE_OVERFLOW_EN) {
        if (adc->callback) {
            wj_adc_dis_int_overflow(adc_base);
            adc->callback(adc, ADC_EVENT_ERROR, adc->arg);
        }
    }


}

void wj_adc_dma_event_cb(csi_dma_ch_t *dma, csi_dma_event_t event, void *arg)
{
    csi_adc_t      *adc;

    adc = (csi_adc_t *)dma->parent;

    if (event == DMA_EVENT_TRANSFER_DONE) {
        csi_dma_ch_stop(dma);
        adc->num = 0U;
        adc->state.readable = 1U;

        if (adc->callback) {
            soc_dcache_clean_invalid_range((unsigned long)adc->data, adc->num * 4U);
            adc->callback(adc, ADC_EVENT_CONVERT_COMPLETE, adc->arg);
        }
    }
}

static csi_error_t wj_adc_start_intr(csi_adc_t *adc)
{
    wj_adc_regs_t *adc_base;

    adc_base = (wj_adc_regs_t *)HANDLE_REG_BASE(adc);
    csi_irq_enable((uint32_t)adc->dev.irq_num);
    wj_adc_en_int_one_channel_done(adc_base);

    wj_adc_en_start(adc_base);
    return CSI_OK;
}

static csi_error_t wj_adc_start_dma(csi_adc_t *adc)
{
    csi_dma_ch_config_t config;
    wj_adc_regs_t       *adc_base;

    adc_base = (wj_adc_regs_t *)HANDLE_REG_BASE(adc);

    /* configure dma channel */
    config.src_inc = DMA_ADDR_CONSTANT;
    config.dst_inc = DMA_ADDR_INC;
    config.src_tw  = DMA_DATA_WIDTH_32_BITS;
    config.dst_tw  = DMA_DATA_WIDTH_32_BITS;

    config.group_len = 4U;
    config.trans_dir = DMA_PERH2MEM;
    config.handshake = 81U;
    csi_dma_ch_config(adc->dma, &config);
    csi_irq_disable((uint32_t)adc->dev.irq_num);
    soc_dcache_clean_invalid_range((unsigned long)adc->data, adc->num * 4U);
    /* set rx mode*/

    csi_dma_ch_start(adc->dma, (void *) & (adc_base->ADC_DATA), adc->data, (adc->num) * 4U);
    wj_adc_en_start(adc_base);
    return CSI_OK;
}

static csi_error_t wj_adc_stop_intr(csi_adc_t *adc)
{
    wj_adc_regs_t *adc_base;
    csi_error_t   ret = CSI_OK;
    uint32_t      timestart;

    adc_base = (wj_adc_regs_t *)HANDLE_REG_BASE(adc);
    wj_adc_dis_int_one_convert_done(adc_base);
    timestart = csi_tick_get_ms();

    if (wj_adc_get_work_mode(adc_base) == WJ_ADC_CONFIG0_WORK_MODE_SCAN) {

        wj_adc_en_stop(adc_base);

        while (wj_adc_get_idle(adc_base) != WJ_ADC_CTRL_ADC_DATA_IDLE_Msk) {
            if ((csi_tick_get_ms() - timestart) > DEFUALT_WJ_ADC_OPERATION_TIMEOUT) {
                ret = CSI_TIMEOUT;
                break;
            }
        }
    }

    return ret;
}

static csi_error_t wj_adc_stop_dma(csi_adc_t *adc)
{
    wj_adc_regs_t *adc_base;
    csi_error_t   ret = CSI_OK;
    uint32_t      timestart;

    adc_base = (wj_adc_regs_t *)HANDLE_REG_BASE(adc);

    timestart = csi_tick_get_ms();

    if (wj_adc_get_work_mode(adc_base) == WJ_ADC_CONFIG0_WORK_MODE_SCAN) {
        wj_adc_en_stop(adc_base);

        while (wj_adc_get_idle(adc_base) != WJ_ADC_CTRL_ADC_DATA_IDLE_Msk) {
            if ((csi_tick_get_ms() - timestart) > DEFUALT_WJ_ADC_OPERATION_TIMEOUT) {
                ret = CSI_TIMEOUT;
                break;
            }
        }
    }

    return ret;
}

csi_error_t csi_adc_init(csi_adc_t *adc, uint32_t idx)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    wj_adc_regs_t *adc_base;

    // This variable used to fix hardware bug
    // It save ADC_OSVCNU register value, avoid multiple modification
    // This value will only set once at csi_adc_start()
    adc->priv = 0U;

    if (target_get(DEV_WJ_ADC_TAG, idx, &adc->dev) != CSI_OK) {
        ret = CSI_ERROR;
    } else {
        adc_base = (wj_adc_regs_t *)HANDLE_REG_BASE(adc);

        adc->state.writeable = 1U;
        adc->state.readable  = 1U;
        adc->state.error     = 0U;
        adc->callback        = NULL;
        adc->arg             = NULL;
        adc->data            = NULL;
        adc->dma             = NULL;
        adc->start           = NULL;
        adc->stop            = NULL;

        wj_adc_dis(adc_base);
        wj_adc_en(adc_base);
        mdelay(1U);
        wj_adc_reset(adc_base);
    }

    return ret;
}

void csi_adc_uninit(csi_adc_t *adc)
{
    CSI_PARAM_CHK_NORETVAL(adc);

    wj_adc_regs_t *adc_base;

    adc_base = (wj_adc_regs_t *)HANDLE_REG_BASE(adc);

    wj_adc_reset_sel_channel(adc_base, WJ_ADC_CHANNEL_SEL_Msk);
    wj_adc_dis(adc_base);
}

csi_error_t csi_adc_set_buffer(csi_adc_t *adc, uint32_t *data, uint32_t num)
{
    csi_error_t ret = CSI_OK;
    CSI_PARAM_CHK(adc, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);
    CSI_PARAM_CHK(num, CSI_ERROR);

    if (num == 0U) {
        ret = CSI_ERROR;
    } else {
        adc->data = data;
        adc->num = num;
        ret = CSI_OK;
    }

    return ret;
}

csi_error_t csi_adc_start(csi_adc_t *adc)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);

    wj_adc_regs_t *adc_base;
    csi_error_t ret = CSI_OK;
    adc_base = (wj_adc_regs_t *)HANDLE_REG_BASE(adc);

    do {
        /* rx buffer not full */
        if (adc->state.readable == 0U) {
            ret = CSI_BUSY;
            break;
        }

        /* last conversion existed */
        if (adc->state.writeable == 0U) {
            ret = CSI_BUSY;
            break;
        }

        if (wj_adc_get_work_mode(adc_base) == WJ_ADC_CONFIG0_WORK_MODE_SCAN) {
            adc->state.writeable = 0U;
        }

        wj_adc_en_start(adc_base);
    } while (0);

    return ret;
}

csi_error_t csi_adc_stop(csi_adc_t *adc)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);

    wj_adc_regs_t *adc_base;
    csi_error_t   ret = CSI_OK;

    adc_base = (wj_adc_regs_t *)HANDLE_REG_BASE(adc);
    wj_adc_en_stop(adc_base);

    /* Read the data register to clear the remaining data */
    wj_adc_get_data(adc_base);

    adc->state.readable  = 1U;
    adc->state.writeable = 1U;
    adc->data = NULL;
    adc->num = 0U;

    return ret;
}

csi_error_t csi_adc_channel_enable(csi_adc_t *adc, uint8_t ch_id, bool is_enable)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    wj_adc_regs_t *adc_base = (wj_adc_regs_t *)HANDLE_REG_BASE(adc);

    if (is_enable) {
        if ((uint32_t)adc->priv < MAX_CHANNEL_NUM) {
            VOID_P_ADD(adc->priv, 1U);
            wj_adc_set_sel_channel(adc_base, ((uint32_t)1U << ch_id));
        } else {
            ret = CSI_ERROR;
        }
    } else {
        if (adc->priv > 0) {
            wj_adc_reset_sel_channel(adc_base, ((uint32_t)1U << ch_id));
            VOID_P_DEC(adc->priv, 1U);
        } else {
            ret = CSI_ERROR;
        }
    }

    return ret;
}

csi_error_t csi_adc_channel_sampling_time(csi_adc_t *adc, uint8_t ch_id, uint16_t clock_num)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);
    return CSI_UNSUPPORTED;
}

csi_error_t csi_adc_sampling_time(csi_adc_t *adc, uint16_t clock_num)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);

    csi_error_t ret = CSI_OK;

    wj_adc_regs_t *adc_base = (wj_adc_regs_t *)HANDLE_REG_BASE(adc);
    wj_adc_set_sample_cycle(adc_base, (uint32_t)clock_num);

    return ret;
}


uint32_t csi_adc_freq_div(csi_adc_t *adc, uint32_t div)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);
    uint32_t ret;
    wj_adc_regs_t *adc_base = (wj_adc_regs_t *)HANDLE_REG_BASE(adc);

    if (div <= WJ_ADC_CONFIG0_CLK_DIV_1_32) {
        wj_adc_set_clk_div(adc_base, div);
        ret = soc_get_adc_freq((uint32_t)adc->dev.idx) / (2U * ((div >> WJ_ADC_CONFIG0_CLK_DIV_Pos) + 1U));
    } else {
        ret = 0U;
    }

    return ret;
}

int32_t csi_adc_read(csi_adc_t *adc)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);

    uint32_t timestart;
    int32_t  ret = CSI_OK;

    wj_adc_regs_t *adc_base = (wj_adc_regs_t *)HANDLE_REG_BASE(adc);

    timestart = csi_tick_get_ms();

    while (wj_adc_get_data_ready(adc_base) == 0U) {
        if ((csi_tick_get_ms() - timestart) > DEFUALT_WJ_ADC_OPERATION_TIMEOUT) {
            ret = CSI_TIMEOUT;
            break;
        }
    }

    if (ret == CSI_OK) {
        ret = (int32_t)wj_adc_get_data(adc_base);
    }

    return ret;
}

csi_error_t csi_adc_get_state(csi_adc_t *adc, csi_state_t *state)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);
    *state = adc->state;
    return CSI_OK;
}

uint32_t csi_adc_get_freq(csi_adc_t *adc)
{
    CSI_PARAM_CHK(adc, 0U);

    uint32_t div;

    wj_adc_regs_t *adc_base = (wj_adc_regs_t *)HANDLE_REG_BASE(adc);

    div = wj_adc_get_clk_div(adc_base);

    return soc_get_adc_freq((uint32_t)adc->dev.idx) / (2U * (div + 1U));
}

csi_error_t csi_adc_attach_callback(csi_adc_t *adc, void *callback, void *arg)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);
    CSI_PARAM_CHK(callback, CSI_ERROR);

    adc->callback = callback;
    adc->arg      = arg;
    adc->start    = wj_adc_start_intr;
    adc->stop     = wj_adc_stop_intr;

    csi_irq_attach((uint32_t)adc->dev.irq_num, &wj_adc_irqhandler, &adc->dev);
    csi_irq_enable((uint32_t)adc->dev.irq_num);
    return CSI_OK;

}

void csi_adc_detach_callback(csi_adc_t *adc)
{
    CSI_PARAM_CHK_NORETVAL(adc);

    adc->callback  = NULL;
    adc->arg       = NULL;
    adc->start     = NULL;
    adc->stop      = NULL;
    csi_irq_disable((uint32_t)adc->dev.irq_num);
}

csi_error_t csi_adc_start_async(csi_adc_t *adc)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);

    wj_adc_regs_t *adc_base;
    csi_error_t   ret = CSI_OK;

    adc_base = (wj_adc_regs_t *)HANDLE_REG_BASE(adc);

    do {
        if ((adc->data == NULL) || (adc->num == 0U)) {
            ret = CSI_ERROR;
            break;
        }

        /* rx buffer not full */
        if (adc->state.readable == 0U) {
            ret = CSI_BUSY;
            break;
        }

        /* last conversion existed */
        if (adc->state.writeable == 0U) {
            ret = CSI_ERROR;
            break;
        }

        if (adc->start) {
            adc->start(adc);

            if (wj_adc_get_work_mode(adc_base) == WJ_ADC_CONFIG0_WORK_MODE_SCAN) {
                adc->state.writeable = 0U;
                adc->state.readable  = 0U;
            }
        }
    } while (0);

    return ret;
}

csi_error_t csi_adc_stop_async(csi_adc_t *adc)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);

    csi_error_t ret = CSI_OK;

    if (adc->stop) {
        adc->stop(adc);
        adc->state.readable  = 1U;
        adc->state.writeable = 1U;
        adc->data = NULL;
        adc->num = 0U;
    } else {
        ret = CSI_ERROR;
    }

    return ret;
}

csi_error_t csi_adc_continue_mode(csi_adc_t *adc, bool is_enable)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);

    wj_adc_regs_t *adc_base;

    adc_base = (wj_adc_regs_t *)HANDLE_REG_BASE(adc);

    if (is_enable) {
        wj_adc_set_work_mode(adc_base, WJ_ADC_CONFIG0_WORK_MODE_SCAN);
    } else {
        wj_adc_set_work_mode(adc_base, WJ_ADC_CONFIG0_WORK_MODE_SINGLE);
    }

    wj_adc_en_wait_mode(adc_base);
    return CSI_OK;
}

csi_error_t csi_adc_link_dma(csi_adc_t *adc, csi_dma_ch_t *dma)
{
    CSI_PARAM_CHK(adc, CSI_ERROR);

    csi_error_t ret = CSI_OK;

    if (dma != NULL) {
        dma->parent = adc;
        ret = csi_dma_ch_alloc(dma, -1, -1);

        if (ret == CSI_OK) {
            csi_dma_ch_attach_callback(dma, wj_adc_dma_event_cb, NULL);
            adc->dma   = dma;
            adc->start = wj_adc_start_dma;
            adc->stop  = wj_adc_stop_dma;
        } else {
            dma->parent = NULL;
        }
    } else {
        if (adc->dma) {
            csi_dma_ch_free(adc->dma);
            csi_dma_ch_detach_callback(adc->dma);
            adc->dma = NULL;
        }

        if (adc->callback != NULL) {
            adc->start = wj_adc_start_intr;
            adc->stop  = wj_adc_stop_intr;
        } else {
            adc->start = NULL;
            adc->stop  = NULL;
        }
    }

    return ret;
}

#ifdef CONFIG_PM
csi_error_t dw_adc_pm_action(csi_dev_t *dev, csi_pm_dev_action_t action)
{
    CSI_PARAM_CHK(dev, CSI_ERROR);

    csi_error_t ret = CSI_OK;
    csi_pm_dev_t *pm_dev = &dev->pm_dev;
    wj_adc_regs_t *adc_base = (wj_adc_regs_t *)dev->reg_base;

    switch (action) {
        case PM_DEV_SUSPEND:
            csi_pm_dev_save_regs(pm_dev->reten_mem, (uint32_t *)dev->reg_base, 1U);
            csi_pm_dev_save_regs(pm_dev->reten_mem + 1U, (uint32_t *)(dev->reg_base + 4U), 4U);
            csi_pm_dev_save_regs(pm_dev->reten_mem + 5U, (uint32_t *)(dev->reg_base + 28U), 3U);
            break;

        case PM_DEV_RESUME:
            adc_base->ADC_CTRL &= ~(1U);
            csi_pm_dev_restore_regs(pm_dev->reten_mem + 1U, (uint32_t *)(dev->reg_base + 4U), 4U);
            csi_pm_dev_restore_regs(pm_dev->reten_mem + 5U, (uint32_t *)(dev->reg_base + 28U), 3U);
            csi_pm_dev_restore_regs(pm_dev->reten_mem, (uint32_t *)dev->reg_base, 1U);
            break;

        default:
            ret = CSI_ERROR;
            break;
    }

    return ret;
}

csi_error_t csi_adc_enable_pm(csi_adc_t *adc)
{
    return csi_pm_dev_register(&adc->dev, dw_adc_pm_action, 32U, 0U);
}

void csi_adc_disable_pm(csi_adc_t *adc)
{
    csi_pm_dev_unregister(&adc->dev);
}
#endif


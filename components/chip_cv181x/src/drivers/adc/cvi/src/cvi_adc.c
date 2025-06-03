/*
* Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
*/

#include <drv/common.h>
#include <drv/pin.h>
#include <drv/cvi_irq.h>
#include <drv/tick.h>
#include "cvi_adc.h"
#include "mmio.h"
#include <stdint.h>
#include <soc.h>
#include <csi_core.h>
#include <csi_config.h>

#define ADC_FREQ_DEFAULT 25000000
#define DEFUALT_CVI_ADC_OPERATION_TIMEOUT    1000000U   // 1 second
#define MAX_CHANNEL_NUM                      3U

#define EFUSE_ADC_TRIM_REG 0x18
#define TOP_ADC_TRIM_MASK 0xf0000000
#define TOP_ADC_TRIM_OFFSET 28
#define RTC_ADC_TRIM_MASK 0x0f000000
#define RTC_ADC_TRIM_OFFSET 24
#define EFUSE_BASE      0x03050000
#define EFUSE_FTSN0     0x100

#define TRIM	  0x34

// static void cvi_adc_all_channel_done_irq(cvi_adc_t *adc)
// {
//     unsigned long reg_base;
//     reg_base = HANDLE_REG_BASE(adc);

//     if (adc->num > 0U) {
//         // TODO: support other bit width
//         // *adc->data++ = cvi_adc_get_data(reg_base);
//         adc->num--;
//     }

//     if (adc->num == 0U) {
//         if (adc->callback) {
//             cvi_adc_dis_int_all_channel_done(reg_base);
//             adc->callback(adc, ADC_EVENT_CONVERT_COMPLETE, adc->arg);
//             adc->state.readable = 1U;
//         }
//     }
// }
static inline void __iomem *get_top_domain_adc(void)
{
	return (void __iomem *)0x030F0000;
}

static inline void __iomem *get_rtc_domain_adc(void)
{
	return (void __iomem *)0x0502C000;
}

void cvi_adc_trim(cvi_adc_t *adc)
{
	u32 top_trim, rtc_trim;
	u32 efuse_value;

	//unsigned reg_base = GET_DEV_REG_BASE(adc);

	efuse_value = mmio_read_32(EFUSE_BASE + EFUSE_FTSN0 + (EFUSE_ADC_TRIM_REG));
	printf("0x%x\n", efuse_value);

	top_trim = (efuse_value & TOP_ADC_TRIM_MASK) >> TOP_ADC_TRIM_OFFSET;
	rtc_trim = (efuse_value & RTC_ADC_TRIM_MASK) >> RTC_ADC_TRIM_OFFSET;

	printf("Setting top_trim: 0x%x, rtc_trim: 0x%x\n", top_trim,
		 rtc_trim);

	writel(top_trim, get_top_domain_adc() + TRIM);
	writel(rtc_trim, get_rtc_domain_adc() + TRIM);
	printf("Getting top_trim: 0x%x, rtc_trim: 0x%x\n",
		 readl(get_top_domain_adc() + TRIM) & 0xf,
		 readl(get_rtc_domain_adc() + TRIM) & 0xf);

	pr_err("adc trim ok!\n");
}

static void cvi_adc_irqhandler(unsigned int irqn, void *args)
{
	uint8_t i;
    cvi_adc_t *adc;

    adc      = (cvi_adc_t *)args;

    adc_clear_int_flag(HANDLE_REG_BASE(adc));

	for (i=0; i<adc->num; i++)
		adc->data[i] = cvi_adc_read(adc);

    if (adc->callback) {
        adc->callback(adc, 0, adc->arg);
    }

    // cvi_adc_start(adc);
}

static cvi_error_t cvi_adc_start_intr(cvi_adc_t *adc)
{
    unsigned long reg_base = HANDLE_REG_BASE(adc);
    csi_irq_enable((uint32_t)adc->dev.irq_num);
    adc_en_int_all_channel_done(reg_base);
    adc_start(reg_base);
    return CVI_OK;
}

static cvi_error_t cvi_adc_stop_intr(cvi_adc_t *adc)
{
    unsigned long reg_base;
    cvi_error_t   ret = CVI_OK;

    reg_base = HANDLE_REG_BASE(adc);
    adc_dis_int_all_channel_done(reg_base);
    adc_stop(reg_base);

#if 0
    uint32_t timestart = csi_tick_get_ms();

    if (cvi_adc_get_work_mode(reg_base) == CVI_ADC_CONFIG0_WORK_MODE_SCAN) {

        cvi_adc_en_stop(reg_base);

        while (cvi_adc_get_idle(reg_base) != CVI_ADC_CTRL_ADC_DATA_IDLE_Msk) {
            if ((csi_tick_get_ms() - timestart) > DEFUALT_CVI_ADC_OPERATION_TIMEOUT) {
                ret = CSI_TIMEOUT;
                break;
            }
        }
    }
#endif

    return ret;
}

// cvi_error_t cvi_adc_link_dma(cvi_adc_t *adc, cvi_dma_ch_t *dma)
// {
//     return CVI_OK;
// }

cvi_error_t cvi_adc_init(cvi_adc_t *adc)
{
	unsigned long reg_base = GET_DEV_REG_BASE(adc);
	uint8_t irqn = GET_DEV_IRQ_NUM(adc);

	adc->state.writeable = 1U;
	adc->state.readable  = 1U;
	adc->state.error     = 0U;
	adc->num             = 0U;
	adc->callback        = NULL;
	adc->arg             = NULL;
	adc->data            = NULL;
	// adc->dma             = NULL;
	adc->start           = NULL;
	adc->stop            = NULL;
    adc->ch_id           = 0;

	if (reg_base)
	{
		pr_err("adc init ok! reg_base: 0x%x, bank: %d, irq_num: %d\n", reg_base, GET_DEV_IDX(adc), irqn);
        cvi_adc_sampling_time(adc, 0xf); // set smpling cycle 640ns
		return CVI_OK;
	}
	else
	{
		pr_err("adc init failed! reg_base: 0x%x, bank: %d, irq_num: %d\n", reg_base, GET_DEV_IDX(adc), irqn);
		return CVI_ERROR;
	}

	return CVI_OK;
}

void cvi_adc_uninit(cvi_adc_t *adc)
{
    unsigned long reg_base = GET_DEV_REG_BASE(adc);
    adc_reset_sel_channel(reg_base, ((uint32_t)adc->ch_id << (ADC_CTRL_ADC_SEL_Pos + 1)));
    adc->ch_id = 0;
    // Avoid shutdown the channel used by linux
#if 0
    adc_stop(reg_base);
#endif
}

cvi_error_t cvi_adc_set_buffer(cvi_adc_t *adc, uint32_t *data, uint32_t num)
{
    adc->data = data;
    adc->num = num;
    return CVI_OK;
}

cvi_error_t cvi_adc_start(cvi_adc_t *adc)
{
    cvi_error_t   ret = CVI_OK;
    unsigned long reg_base = GET_DEV_REG_BASE(adc);

    do {
        /* rx buffer not full */
        if (adc->state.readable == 0U) {
            ret = CVI_BUSY;
            break;
        }

        /* last conversion existed */
        if (adc->state.writeable == 0U) {
            ret = CVI_BUSY;
            break;
        }

        adc_start(reg_base);

    } while(0);

    return ret;
}

cvi_error_t cvi_adc_start_async(cvi_adc_t *adc)
{
    cvi_error_t   ret = CVI_OK;

    do {
        if ((adc->data == NULL) || (adc->num == 0U)) {
            ret = CVI_ERROR;
            break;
        }

        /* rx buffer not full */
        if (adc->state.readable == 0U) {
            ret = CVI_BUSY;
            break;
        }

        /* last conversion existed */
        if (adc->state.writeable == 0U) {
            ret = CVI_ERROR;
            break;
        }

        if (adc->start) {
            adc->start(adc);

            // if (wj_adc_get_work_mode(adc_base) == WJ_ADC_CONFIG0_WORK_MODE_SCAN) {
            //     adc->state.writeable = 0U;
            //     adc->state.readable  = 0U;
            // }
        }
    } while (0);

    return ret;
}

cvi_error_t cvi_adc_stop(cvi_adc_t *adc)
{
    adc_stop(GET_DEV_REG_BASE(adc));

    adc->state.readable  = 1U;
    adc->state.writeable = 1U;
    adc->data = NULL;
    adc->num = 0U;

    return CVI_OK;
}

cvi_error_t cvi_adc_stop_async(cvi_adc_t *adc)
{
    cvi_error_t ret = CVI_OK;

    if (adc->stop) {
        adc->stop(adc);
        adc->state.readable  = 1U;
        adc->state.writeable = 1U;
        adc->data = NULL;
        adc->num = 0U;
    } else {
        ret = CVI_ERROR;
    }

    return ret;
}

cvi_error_t cvi_adc_channel_enable(cvi_adc_t *adc, uint8_t ch_id, bool is_enable)
{
    cvi_error_t ret = CVI_OK;

    unsigned reg_base = GET_DEV_REG_BASE(adc);

#if 0
    #if CONFIG_BOARD_CV181XC
    if (adc->dev.idx == 0 && ch_id != 1) {
        pr_err("invalid ch_id\n");
        return CVI_ERROR;
    }
    #endif
#endif

    // channel is used by linux
    if ((adc_get_sel_channel(reg_base) & ((uint32_t)ch_id << (ADC_CTRL_ADC_SEL_Pos + 1)))
        && (ch_id != adc->ch_id)) {
        pr_err("adc chip: %d, ch: %d is used!", adc->dev.idx, ch_id);
        // ignore rtc adc channel
        if (adc->dev.idx != 1)
            return CVI_ERROR;
    }

    if (ch_id < 1 || ch_id > 3 || (adc->dev.idx == 1 && ch_id > 2)) {
        pr_err("invalid ch_id\n");
        ret = CVI_ERROR;
    } else {
        adc->ch_id = ch_id;
        if (is_enable)
            adc_set_sel_channel(reg_base, ((uint32_t)ch_id << (ADC_CTRL_ADC_SEL_Pos + 1)));
        else
            adc_reset_sel_channel(reg_base, ((uint32_t)ch_id << (ADC_CTRL_ADC_SEL_Pos + 1)));
    }

    return ret;
}

// cvi_error_t cvi_adc_channel_sampling_time(cvi_adc_t *adc, uint8_t ch_id, uint16_t clock_num)
// {
//     return CVI_OK;
// }

cvi_error_t cvi_adc_sampling_time(cvi_adc_t *adc, uint16_t clock_num)
{
    if (clock_num > 0xF)
        return CVI_ERROR;

    adc_set_sample_cycle(GET_DEV_REG_BASE(adc), clock_num);
    return CVI_OK;
}

cvi_error_t cvi_adc_continue_mode(cvi_adc_t *adc, bool is_enable)
{
    return CVI_UNSUPPORTED;
}

uint32_t cvi_adc_freq_div(cvi_adc_t *adc, uint32_t div)
{
    uint32_t ret;
    unsigned long reg_base = GET_DEV_REG_BASE(adc);

    if ((div << ADC_CYC_SET_ADC_CYC_CLK_DIV_Pos) <= ADC_CYC_SET_ADC_CYC_CLK_DIV_16) {
        adc_set_clk_div(reg_base, div);
        ret = adc_get_clk_div(reg_base);
    } else {
        ret = 0U;
    }

    return ret;
}

int32_t cvi_adc_read(cvi_adc_t *adc)
{
    int32_t ret = CVI_OK;

    unsigned long reg_base = GET_DEV_REG_BASE(adc);

    uint32_t timestart = csi_tick_get_ms();

    while (adc_get_data_ready(reg_base) == 0U) {
        if ((csi_tick_get_ms() - timestart) > DEFUALT_CVI_ADC_OPERATION_TIMEOUT) {
            ret = CVI_TIMEOUT;
            break;
        }
    }

    if (ret == CVI_OK) {
        ret = (int32_t)adc_get_channel_data_ch(reg_base, adc->ch_id);
    }

    return ret;
}

cvi_error_t cvi_adc_get_state(cvi_adc_t *adc, cvi_state_t *state)
{
    *state = adc->state;
    return CVI_OK;
}

uint32_t cvi_adc_get_freq(cvi_adc_t *adc)
{

    uint32_t div;

    div = adc_get_clk_div(GET_DEV_REG_BASE(adc)) >> ADC_CYC_SET_ADC_CYC_CLK_DIV_Pos;

    // return soc_get_adc_freq((uint32_t)adc->dev.idx) / (2U * (div + 1U));
    // return soc_get_adc_freq((uint32_t)adc->dev.idx) / (div + 1U);
    return ADC_FREQ_DEFAULT / (div + 1U);
}

cvi_error_t cvi_adc_configure_irq(cvi_adc_t *adc, void *callback, void *arg)
{
    adc->callback = callback;
    adc->arg      = arg;
    adc->start    = cvi_adc_start_intr;
    adc->stop     = cvi_adc_stop_intr;

	if (callback)
	{
		adc_int_en(GET_DEV_REG_BASE(adc));
		request_irq((uint32_t)(adc->dev.irq_num), &cvi_adc_irqhandler, 0, "adc int", adc);
	}
	else
	{
		adc_int_dis(GET_DEV_REG_BASE(adc));
		csi_irq_disable((uint32_t)adc->dev.irq_num);
	}

    return CVI_OK;
}

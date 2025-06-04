/*
 * Copyright (C) 2017-2019 Alibaba Group Holding Limited
 */


/******************************************************************************
 * @file     ck_codec_register.c
 * @brief    CSI Source File for codec Driver
 * @version  V1.0
 * @date     25. September 2019
 ******************************************************************************/

#include "stdlib.h"

#include <stdbool.h>
#include <csi_core.h>
#include <drv/codec.h>
#include "ck_codec.h"
#include "drv/ringbuffer.h"

extern csi_dev_t ck_codec_instance_v2[];

#define ERR_CODEC(errno) (CSI_DRV_ERRNO_CODEC_BASE | errno)
#define CODEC_BUSY_TIMEOUT    0xffffffff
#define CODEC_NULL_PARAM_CHK(para) HANDLE_PARAM_CHK(para, ERR_CODEC(DRV_ERROR_PARAMETER))

#define REG_READ(val, addr) do{val = addr; val = addr;}while(0);

/*if sb en, some power is off*/
void ck_codec_sb_power_ctrl(csi_dev_t *priv, int en)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    uint32_t val = 0;

    if (en) {
        val = 0x07;
    }

    addr->CR_VIC = val;
}

void ck_codec_tx_feed(csi_dev_t *priv)
{
    uint32_t tx_cnt = ck_codec_tx_fifo_cnt(priv);
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;;
    int j = 0;
    if(tx_cnt < 4) {
        j = 4 - tx_cnt;
    }
    for (int i = 0; i < j; i++) {
        addr->PARA_TX_FIFO = 0;
    }
}

void ck_codec_inerrupt_ctrl(csi_dev_t *priv, uint32_t mask, int en)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    uint32_t val = 0;
    REG_READ(val, addr->INTR_ERR_CTRL);

    val &= ~mask;

    if (en) {
        val |= mask;
    }

    addr->INTR_ERR_CTRL = val;
}

void ck_codec_inerrupt_disable_all(csi_dev_t *priv)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;

    addr->INTR_ERR_CTRL = 0;
}

void ck_codec_mclk_freq_select(csi_dev_t *priv, ck_codec_mclk_freq_t freq)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    uint32_t val = freq;

    addr->CR_CK = val;
}

void ck_codec_adc_bit_width_and_mode_set(csi_dev_t *priv)
{
    /*must set 0, 16bit and Parallel mode*/
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    addr->AICR_ADC = 0;
}

void ck_codec_tx_threshold_val(csi_dev_t *priv, uint32_t val)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    uint32_t reg = 0;
    REG_READ(reg, addr->FIFO_TH_CTRL);
    reg &= ~(0x7);
    val -= 1;
    reg |= val & 0x7;

    /*Don't fixes me, repair fifo error*/
    addr->FIFO_TH_CTRL = 0x33;
}

void ck_codec_rx_threshold_val(csi_dev_t *priv, uint32_t val)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    uint32_t reg = 0;
    REG_READ(reg, addr->FIFO_TH_CTRL);
    reg &= ~(0x7 << 4);
    val -= 1;
    reg |= (val & 0x7) << 4;

    /*Don't fixes me, repair fifo error*/
    addr->FIFO_TH_CTRL = 0x33;
}

void ck_codec_adc_channel_enable(csi_dev_t *priv, int ad_idx, int en)
{
    ad_idx /= 2;
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    uint32_t val = 0;
    REG_READ(val, addr->AICR_ADC_2);
    val |= (0x01 << ad_idx);

    if (en) {
        val &= ~(0x01 << ad_idx);
    }

    addr->AICR_ADC_2 = val;
    //addr->AICR_ADC_2 = 0;
}

void ck_codec_adc_sample_rate_config(csi_dev_t *priv, uint32_t rate)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    uint32_t val = 0;
    REG_READ(val, addr->FCR_ADC);
    val = rate;

    addr->FCR_ADC = val;
    // FIXME: 修复直流偏移问题
    /*
    FCR_ADC寄存器
    ADC12：bit4
    ADC34：bit5
    ADC56：bit6
    ADC78：bit7
    */
    addr->FCR_ADC |= (0xF << 4);
}

/*mode：0 disable, 1~3 enable, func mode*/
void ck_codec_adc_wnf_ctrl(csi_dev_t *priv, int ad_idx, int mode)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    uint32_t val = 0;
    REG_READ(val, addr->CR_WNF);
    ad_idx /= 2;
    ad_idx *= 2;
    val &= ~(0x03 << ad_idx);

    if (mode) {
        val |= (mode << ad_idx);
    }

    addr->CR_WNF = val;
}

/*ad idx 0~7*/
void ck_codec_mic_set_bias(csi_dev_t *priv, int ad_idx, ck_codec_mic_bias_t bias)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    uint32_t val = 0;
    REG_READ(val, addr->CR_MIC[ad_idx]);
    val &= ~(MICBIAS1_V_Msk);

    if (bias == CK_CODEC_MIC_BIAS_1800MV) {
        val |= MICBIAS1_V_Msk;
    }

    addr->CR_MIC[ad_idx] = val;
}

void ck_codec_disable_dmic(csi_dev_t *priv)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    addr->CR_DMIC12 = 0xC0;  //05: DMIC12 is active, c0:inactive
    addr->CR_DMIC34 = 0xC0;  //05: DMIC12 is active, c0:inactive
    addr->CR_DMIC56 = 0xC0;  //05: DMIC12 is active, c0:inactive
    addr->CR_DMIC78 = 0xC0;  //05: DMIC12 is active, c0:inactive
}

/*val 0~31 -> 0~ -31db*/
void ck_codec_adc_mixer_set_gain(csi_dev_t *priv, uint32_t gain)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;;
    addr->GCR_MIXADCL = gain;
    addr->GCR_MIXADCR = gain;
}

uint32_t ck_codec_adc_mixer_get_gain(csi_dev_t *priv)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;;
    return addr->GCR_MIXADCL;
}

/*ad_idx 0~7*/
void ck_codec_adc_mode_set(csi_dev_t *priv, int ad_idx, ck_codec_adc_mode_t mode)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    uint32_t val = 0;
    REG_READ(val, addr->CR_MIC[ad_idx]);

    val &= ~(MICDIFF1_Msk);
    val |= mode << MICDIFF1_Pos;

    addr->CR_MIC[ad_idx] = val;
}

void ck_codec_mic_bias_en(csi_dev_t *priv, int ad_idx, int en)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    uint32_t val = 0;
    REG_READ(val, addr->CR_MIC[ad_idx]);

    val &= ~(1 << SB_MICBIAS1_Pos);

    if (en == 0) {
        val |= 1 << SB_MICBIAS1_Pos;
    }

    addr->CR_MIC[ad_idx] = val;
}

void ck_codec_adc_sb_enable(csi_dev_t *priv, int ad_idx, int en)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    int offset = ad_idx % 2;
    ad_idx /= 2;
    uint32_t val = 0;
    REG_READ(val, addr->CR_ADC[ad_idx]);

    val &= ~(0x01 << (offset + 4));

    if (en) {
        val |= (0x01 << (offset + 4));
    }

    addr->CR_ADC[ad_idx] = val;
}

void ck_codec_adc_mute_en(csi_dev_t *priv, int ad_idx, int en)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    int reg_idx = ad_idx / 2;
    uint32_t val = 0;
    REG_READ(val, addr->CR_ADC[reg_idx]);

    val &= ~(0x01 << 7);

    if (en) {
        val |= (0x01 << 7);
    }

    addr->CR_ADC[reg_idx] = val;
}

void ck_codec_path_adc_en(csi_dev_t *priv, int adc_idx, int en)
{
    if (adc_idx > 7) {
        return;
    }

    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    uint32_t val = 0;
    REG_READ(val, addr->PATH_EN);

    val &= ~(0x01 << adc_idx);

    if (en) {
        val |= (0x01 << adc_idx);
    }

    addr->PATH_EN = val;
}

/*Microphone boost stage gain programming value, val 0~20, analog gain*/
/*0~3对应0db 4~7对应4db 8~11对应8db 12~15对应12db 16~19对应20db 20对应20db*/
void ck_codec_adc_set_boost_gain(csi_dev_t *priv, int32_t adc_idx, uint32_t gain)
{
    if (gain > 20) {
        gain = 20;
    }

    uint32_t reg_idx = adc_idx / 2;
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    uint32_t val = 0;
    REG_READ(val, addr->GCR_MIC[reg_idx]);

    gain = gain / 4;
    if (adc_idx % 2) {
        val &= ~(0x07 << 3);
        gain = gain << 3;
    } else {
        val &= ~0x07;
    }

    val |= gain;

    addr->GCR_MIC[reg_idx] = val;
}

uint32_t ck_codec_adc_get_boost_gain(csi_dev_t *priv, int32_t adc_idx)
{
    uint32_t reg_idx = adc_idx / 2;
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    uint32_t val = 0;
    REG_READ(val, addr->GCR_MIC[reg_idx]);

    if (adc_idx % 2) {
        val &= 0x07 << 3;
        val = val >> 3;
    } else {
        val &= 0x07;
    }
    val = val * 4;
    return val;
}

/*adc digital filters digital gain programming value for channel*/
void ck_codec_adc_set_digital_gain(csi_dev_t *priv, int32_t adc_idx, uint32_t gain)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;

    if (gain > 31) {
        gain = 31;
    }

    addr->GCR_ADC[adc_idx] = gain;
}

uint32_t ck_codec_adc_get_digital_gain(csi_dev_t *priv, int32_t adc_idx)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    return addr->GCR_ADC[adc_idx];
}

/*********************DAC Registers********************/

void ck_codec_dac_bit_width_and_mode_set(csi_dev_t *priv, uint32_t bit_width)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    uint32_t val = 0;
    REG_READ(val, addr->AICR_DAC);
    val &= ~(0x03 << 6);
    val |= bit_width;
    val &= ~(0x03);
    val &= !(0x01 << 4);

    addr->AICR_DAC = val;
}

/*
DAC audio interface activation
0: DAC audio interface active
1: DAC audio interface in power-down mode
*/
void ck_codec_dac_power_down_ctrl(csi_dev_t *priv, int en)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    uint32_t val = 0;
    REG_READ(val, addr->AICR_DAC);

    val &= ~(0x01 << 4);

    if (en) {
        val |= 0x01 << 4;
    }

    addr->AICR_DAC = val;
}

void ck_codec_dac_sample_rate_config(csi_dev_t *priv, uint32_t rate)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    uint32_t val = 0;
    REG_READ(val, addr->FCR_DAC);
    val = rate;

    addr->FCR_DAC = val;
}

void ck_codec_dac_mute_en(csi_dev_t *priv, int32_t en)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    uint32_t val = 0;
    REG_READ(val, addr->CR_DAC);

    val &= ~(1 << 7);

    if (en) {
        val |= (1 << 7);
    }

    addr->CR_DAC = val;
}

void ck_codec_dac_mono_enable(csi_dev_t *priv, int32_t en)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    uint32_t val = 0;
    REG_READ(val, addr->CR_DAC);

    val &= ~(1 << 5);

    if (en) {
        val |= (1 << 5);
    }

    addr->CR_DAC = val;
}

/*if en dac power off*/
void ck_codec_dac_sb_en(csi_dev_t *priv, int32_t en)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    uint32_t val = 0;
    REG_READ(val, addr->CR_DAC);

    val &= ~(1 << 4);

    if (en) {
        val |= (1 << 4);
    }

    addr->CR_DAC = val;
}

/*val 0~31 -> 0~ -31db*/
void ck_codec_dac_set_mix_left_gain(csi_dev_t *priv, uint32_t val)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    addr->GCR_MIXDACL = val;
}

uint32_t ck_codec_dac_get_mix_left_gain(csi_dev_t *priv)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    return addr->GCR_MIXDACL;
}

/*val 0~31 -> 0~ -31db*/
void ck_codec_dac_set_mix_right_gain(csi_dev_t *priv, uint32_t val)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    addr->GCR_MIXDACR = val;
}

uint32_t ck_codec_dac_get_mix_right_gain(csi_dev_t *priv)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    return addr->GCR_MIXDACR;
}

/*val 0~31 = 0~-31db, 63~32 = 1~32db*/
void ck_codec_dac_set_digital_left_gan(csi_dev_t *priv, uint32_t val)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    addr->GCR_DACL = val;

}

uint32_t ck_codec_dac_get_digital_left_gan(csi_dev_t *priv)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    return addr->GCR_DACL;
}

/*val 0~31,seemingly no effect*/
void ck_codec_dac_set_digital_right_gan(csi_dev_t *priv, uint32_t val)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    addr->GCR_DACR = val;
}

uint32_t ck_codec_dac_get_digital_right_gan(csi_dev_t *priv)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    return addr->GCR_DACR;
}

/*val 0~31 = 0~-31db*/
void ck_codec_dac_set_analog_left_gan(csi_dev_t *priv, uint32_t val)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    addr->GCR_HPL = val;
}

uint32_t ck_codec_dac_get_analog_left_gan(csi_dev_t *priv)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    return addr->GCR_HPL;
}

/*val 0~31 = 0~-31db*/
void ck_codec_dac_set_analog_right_gan(csi_dev_t *priv, uint32_t val)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    addr->GCR_HPR = val;
}

uint32_t ck_codec_dac_get_analog_right_gan(csi_dev_t *priv)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    return addr->GCR_HPR;
}

uint32_t ck_codec_tx_fifo_cnt(csi_dev_t *priv)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    return addr->TX_FIFO_CNT & 0xf;
}
void ck_codec_path_dac_en(csi_dev_t *priv, int en)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    uint32_t val = 0;
    REG_READ(val, addr->PATH_EN);

    val &= ~(0x01 << 8);

    if (en) {
        val |= (0x01 << 8);
    }

    addr->PATH_EN = val;
}

void ck_codec_dac_headphone_mute(csi_dev_t *priv, int en)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    uint32_t val = 0;
    REG_READ(val, addr->CR_HP);
    val &= ~(0x01 << 7);

    if (en) {
        val |= 0x01 << 7;
    }

    addr->CR_HP = val;
}

void ck_codec_dac_headphone_sb_enable(csi_dev_t *priv, int en)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    uint32_t val = 0;
    REG_READ(val, addr->CR_HP);
    val &= ~((0x01 << 5) | (0x01 << 4));

    if (en) {
        val |= 0x01 << 4 ;
    }

    addr->CR_HP = val;
}

/*val 0~15, The larger the number, the smaller the gain*/
void ck_codec_dac_headphone_set_right_gain(csi_dev_t *priv, uint32_t val)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;

    addr->GCR_HPR = val;
}

/*val 0~15, The larger the number, the smaller the gain*/
void ck_codec_dac_headphone_set_left_gain(csi_dev_t *priv, uint32_t val)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;

    addr->GCR_HPL = val;
}

/*val 0~15, The larger the number, the smaller the gain*/
void ck_codec_dac_headphone_get_right_gain(csi_dev_t *priv, uint32_t *val)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;

    *val = addr->GCR_HPR;
}

/*val 0~15, The larger the number, the smaller the gain*/
void ck_codec_dac_headphone_get_left_gain(csi_dev_t *priv, uint32_t *val)
{
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;

    *val = addr->GCR_HPL;
}

void ck_codec_gasket_reset()
{
    *(volatile uint32_t *)0x8b000068 = 0x3FFE;
    *(volatile uint32_t *)0x8b000068 = 0x3FFF;
}

#define CODEC_ERROR_IRQ_MSAK (0x1ff << 18)
#define CODEC_TX_FIFO_EMPTY_IRQ_MSAK (0x1 << 16)

volatile uint32_t g_debug_tx_fifo_empty_cnt = 0;
void ck_codec_irqhandler( csi_dev_t *pt)
{
    uint32_t base =  pt->reg_base;
    ck_codec_reg_t *addr = (ck_codec_reg_t *)base;
    uint32_t error_mask = 0;

    REG_READ(error_mask, addr->INTR_ERR_STA);
    addr->INTR_ERR_CLR = error_mask;

    if (error_mask & CODEC_TX_FIFO_EMPTY_IRQ_MSAK) {
        g_debug_tx_fifo_empty_cnt++;
        ck_codec_dma_all_restart();
    }
}

static volatile int codec_tx_error = 0;
static volatile int codec_rx_error = 0;
static volatile int codec_gasket_err_int = 0;
static volatile uint32_t codec_gasket_err_record[2000];

void ck_codec_gasket_error_irqhandler(int32_t idx)
{

    csi_dev_t *priv = &ck_codec_instance_v2[0];
    ck_codec_reg_t *addr = (ck_codec_reg_t *)priv->reg_base;
    uint32_t error_mask = 0;
    REG_READ(error_mask, addr->INTR_ERR_STA);

    if ((error_mask & CODEC_ERROR_IRQ_MSAK) == 0) {
        return;
    }

    if (codec_gasket_err_int < 2000) {
        codec_gasket_err_record[codec_gasket_err_int] = error_mask;
        codec_gasket_err_int++;
    } else {
        codec_gasket_err_int  = 0;
    }

    if (error_mask & (1 << 26)) {
        codec_tx_error++;
    } else if (error_mask & (0xff << 18)) {
        codec_rx_error++;
    }

    addr->INTR_ERR_CLR = error_mask & (0x1ff << 18);
    ck_codec_dma_all_restart();
}

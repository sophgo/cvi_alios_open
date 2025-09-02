/*
* Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
*/
#ifndef __CVI_PWM_H__
#define __CVI_PWM_H__

#include <stdio.h>
#include <soc.h>
#include <csi_core.h>

#ifdef __cplusplus
extern "C" {
#endif

/* #define CSI_DRV_DEBUG */

#ifndef pr_err
#define pr_err(x, args...) printf("[%s|%d] - " x, __func__, __LINE__, ##args)
#endif

#ifdef CSI_DRV_DEBUG
#ifndef pr_debug
#define pr_debug(x, args...) printf("[%s|%d] - " x, __func__, __LINE__, ##args)
#endif

#ifndef pr_warn
#define pr_warn(x, args...) printf("[%s|%d] - " x, __func__, __LINE__, ##args)
#endif

#else

#ifndef pr_debug
#define pr_debug(x, args...)
#endif

#ifndef pr_warn
#define pr_warn(x, args...)
#endif

#endif

#define     __IM     volatile const       /*! Defines 'read only' structure member permissions */
#define     __OM     volatile             /*! Defines 'write only' structure member permissions */
#define     __IOM    volatile             /*! Defines 'read / write' structure member permissions */


#define CVI_PWM0_BASE               0x03060000
#define CVI_PWM1_BASE               0x03061000
#define CVI_PWM2_BASE               0x03062000


typedef enum {
    PWM_CHANNEL_0    = 0U,
    PWM_CHANNEL_1,
    PWM_CHANNEL_2,
    PWM_CHANNEL_3,
    PWM_CHANNEL_4,
    PWM_CHANNEL_5,
    PWM_CHANNEL_6,
    PWM_CHANNEL_7,
    PWM_CHANNEL_8,
    PWM_CHANNEL_9,
    PWM_CHANNEL_10,
    PWM_CHANNEL_11,
    PWM_CHANNEL_12,
    PWM_CHANNEL_13,
    PWM_CHANNEL_14,
    PWM_CHANNEL_15,
    PWM_CHANNEL_NUM
} cvi_pwm_channel_t;


struct cvi_pwm_regs_t {
    uint32_t HLPERIOD0;
    uint32_t PERIOD0;
    uint32_t HLPERIOD1;
    uint32_t PERIOD1;
    uint32_t HLPERIOD2;
    uint32_t PERIOD2;
    uint32_t HLPERIOD3;
    uint32_t PERIOD3;
    uint32_t HLPERIOD4;
    uint32_t PERIOD4;
    uint32_t HLPERIOD5;
    uint32_t PERIOD5;

    uint32_t CAP_FREQ0NUM;
    uint32_t CAP_FREQ0DATA;
    uint32_t CAP_FREQ1NUM;
    uint32_t CAP_FREQ1DATA;
    uint32_t CAP_FREQ2NUM;
    uint32_t CAP_FREQ2DATA;
    uint32_t CAP_FREQ3NUM;
    uint32_t CAP_FREQ3DATA;
    uint32_t CAP_FREQ4NUM;
    uint32_t CAP_FREQ4DATA;
    uint32_t CAP_FREQ5NUM;
    uint32_t CAP_FREQ5DATA;

    uint32_t POLARITY;
    uint32_t PWMSTART;
    uint32_t PWMDONE;
    uint32_t PWMUPDATE;

    uint32_t PCOUNT0;
    uint32_t PCOUNT1;
    uint32_t PCOUNT2;
    uint32_t PCOUNT3;
    uint32_t PCOUNT4;
    uint32_t PCOUNT5;

    uint32_t PULSECOUNT0;
    uint32_t PULSECOUNT1;
    uint32_t PULSECOUNT2;
    uint32_t PULSECOUNT3;
    uint32_t PULSECOUNT4;
    uint32_t PULSECOUNT5;

    uint32_t PULSECNT0;
    uint32_t PULSECNT1;
    uint32_t PULSECNT2;
    uint32_t PULSECNT3;
    uint32_t PULSECNT4;
    uint32_t PULSECNT5;

    uint32_t SHIFTCOUNT0;
    uint32_t SHIFTCOUNT1;
    uint32_t SHIFTCOUNT2;
    uint32_t SHIFTCOUNT3;
    uint32_t SHIFTCOUNT4;
    uint32_t SHIFTCOUNT5;

    uint32_t SHIFTSTART;
    uint32_t CAP_FREQEN;

    uint32_t CAP_FREQ0_HCOUNT;
    uint32_t CAP_FREQ0_LCOUNT;
    uint32_t CAP_FREQ1_HCOUNT;
    uint32_t CAP_FREQ1_LCOUNT;
    uint32_t CAP_FREQ2_HCOUNT;
    uint32_t CAP_FREQ2_LCOUNT;
    uint32_t CAP_FREQ3_HCOUNT;
    uint32_t CAP_FREQ3_LCOUNT;
    uint32_t CAP_FREQ4_HCOUNT;
    uint32_t CAP_FREQ4_LCOUNT;
    uint32_t CAP_FREQ5_HCOUNT;
    uint32_t CAP_FREQ5_LCOUNT;

    uint32_t CAP_FREQ0DONE_NUM;
    uint32_t CAP_FREQ1DONE_NUM;
    uint32_t CAP_FREQ2DONE_NUM;
    uint32_t CAP_FREQ3DONE_NUM;
    uint32_t CAP_FREQ4DONE_NUM;
    uint32_t CAP_FREQ5DONE_NUM;

    uint32_t PWM_OE;
    uint32_t PWM_REV0;
    uint32_t PWM_VER;
    uint32_t MASK_PERIOD0;
    uint32_t MASK_PERIOD1;
    uint32_t MASK_PERIOD2;
    uint32_t MASK_PERIOD3;
    uint32_t MASK_PERIOD4;
    uint32_t MASK_PERIOD5;

    uint32_t MASK_CNT0;
    uint32_t MASK_CNT1;
    uint32_t MASK_CNT2;
    uint32_t MASK_CNT3;
    uint32_t MASK_CNT4;
    uint32_t MASK_CNT5;

    uint32_t FREQ_DB_CNT0;
    uint32_t FREQ_DB_CNT1;
    uint32_t FREQ_DB_CNT2;
    uint32_t FREQ_DB_CNT3;
    uint32_t FREQ_DB_CNT4;
    uint32_t FREQ_DB_CNT5;

    uint32_t PWM0_START_POINT;
    uint32_t PWM0_END_POINT;
    uint32_t PWM1_START_POINT;
    uint32_t PWM1_END_POINT;
    uint32_t PWM2_START_POINT;
    uint32_t PWM2_END_POINT;
    uint32_t PWM3_START_POINT;
    uint32_t PWM3_END_POINT;
    uint32_t PWM4_START_POINT;
    uint32_t PWM4_END_POINT;
    uint32_t PWM5_START_POINT;
    uint32_t PWM5_END_POINT;

    uint32_t PWM2ADC_CNT_H0;
    uint32_t PWM2ADC_CNT_L0;
    uint32_t PWM2ADC_CNT_H1;
    uint32_t PWM2ADC_CNT_L1;
    uint32_t PWM2ADC_CNT_H2;
    uint32_t PWM2ADC_CNT_L2;
    uint32_t PWM2ADC_CNT_H3;
    uint32_t PWM2ADC_CNT_L3;
    uint32_t PWM2ADC_CNT_H4;
    uint32_t PWM2ADC_CNT_L4;
    uint32_t PWM2ADC_CNT_H5;
    uint32_t PWM2ADC_CNT_L5;

    uint32_t PWM_START_TOGGLE_MODE;
    uint32_t PWM_END_TOGGLE_MODE;
    uint32_t PWM_OUTPUT_DISABLE;

    uint32_t PWM_UPDATE_CNT0;
    uint32_t PWM_UPDATE_CNT1;
    uint32_t PWM_UPDATE_CNT2;
    uint32_t PWM_UPDATE_CNT3;
    uint32_t PWM_UPDATE_CNT4;
    uint32_t PWM_UPDATE_CNT5;

    uint32_t PWM_INTR_MASK;
    uint32_t PWM_INTR_STATUS;
    uint32_t PWM_INTR_CLR;

};


static struct cvi_pwm_regs_t cv184x_pwm_reg = {
    .HLPERIOD0 = 0x0,
    .PERIOD0 = 0x4,
    .HLPERIOD1 = 0x8,
    .PERIOD1 = 0xc,
    .HLPERIOD2 = 0x10,
    .PERIOD2 = 0x14,
    .HLPERIOD3 = 0x18,
    .PERIOD3 = 0x1c,
    .HLPERIOD4 = 0x20,
    .PERIOD4 = 0x24,
    .HLPERIOD5 = 0x28,
    .PERIOD5 = 0x2c,

    .CAP_FREQ0NUM = 0x30,
    .CAP_FREQ0DATA = 0x34,
    .CAP_FREQ1NUM = 0x38,
    .CAP_FREQ1DATA = 0x3c,
    .CAP_FREQ2NUM = 0x40,
    .CAP_FREQ2DATA = 0x44,
    .CAP_FREQ3NUM = 0x48,
    .CAP_FREQ3DATA = 0x4c,
    .CAP_FREQ4NUM = 0x50,
    .CAP_FREQ4DATA = 0x54,
    .CAP_FREQ5NUM = 0x58,
    .CAP_FREQ5DATA = 0x5c,

    .POLARITY = 0x60,
    .PWMSTART = 0x64,
    .PWMDONE = 0x68,
    .PWMUPDATE = 0x6c,
    .PCOUNT0 = 0x70,
    .PCOUNT1 = 0x74,
    .PCOUNT2 = 0x78,
    .PCOUNT3 = 0x7c,
    .PCOUNT4 = 0x80,
    .PCOUNT5 = 0x84,

    .PULSECOUNT0 = 0x88,
    .PULSECOUNT1 = 0x8c,
    .PULSECOUNT2 = 0x90,
    .PULSECOUNT3 = 0x94,
    .PULSECOUNT4 = 0x98,
    .PULSECOUNT5 = 0x9c,

    .PULSECNT0 = 0xa0,
    .PULSECNT1 = 0xa4,
    .PULSECNT2 = 0xa8,
    .PULSECNT3 = 0xac,
    .PULSECNT4 = 0xb0,
    .PULSECNT5 = 0xb4,

    .SHIFTCOUNT0 = 0xb8,
    .SHIFTCOUNT1 = 0xbc,
    .SHIFTCOUNT2 = 0xc0,
    .SHIFTCOUNT3 = 0xc4,
    .SHIFTCOUNT4 = 0xc8,
    .SHIFTCOUNT5 = 0xcc,

    .SHIFTSTART = 0xd0,
    .CAP_FREQEN = 0xd4,

    .CAP_FREQ0_HCOUNT = 0xd8,
    .CAP_FREQ0_LCOUNT = 0xdc,
    .CAP_FREQ1_HCOUNT = 0xe0,
    .CAP_FREQ1_LCOUNT = 0xe4,
    .CAP_FREQ2_HCOUNT = 0xe8,
    .CAP_FREQ2_LCOUNT = 0xec,
    .CAP_FREQ3_HCOUNT = 0xf0,
    .CAP_FREQ3_LCOUNT = 0xf4,
    .CAP_FREQ4_HCOUNT = 0xf8,
    .CAP_FREQ4_LCOUNT = 0xfc,
    .CAP_FREQ5_HCOUNT = 0x100,
    .CAP_FREQ5_LCOUNT = 0x104,
    .CAP_FREQ0DONE_NUM = 0x108,
    .CAP_FREQ1DONE_NUM = 0x10c,
    .CAP_FREQ2DONE_NUM = 0x110,
    .CAP_FREQ3DONE_NUM = 0x114,
    .CAP_FREQ4DONE_NUM = 0x118,
    .CAP_FREQ5DONE_NUM = 0x11c,

    .PWM_OE = 0x120,
    .PWM_REV0 = 0x124,
    .PWM_VER = 0x128,
    .MASK_PERIOD0 = 0x12c,
    .MASK_PERIOD1 = 0x130,
    .MASK_PERIOD2 = 0x134,
    .MASK_PERIOD3 = 0x138,
    .MASK_PERIOD4 = 0x13c,
    .MASK_PERIOD5 = 0x140,

    .MASK_CNT0 = 0x144,
    .MASK_CNT1 = 0x148,
    .MASK_CNT2 = 0x14c,
    .MASK_CNT3 = 0x150,
    .MASK_CNT4 = 0x154,
    .MASK_CNT5 = 0x158,

    .FREQ_DB_CNT0 = 0x15c,
    .FREQ_DB_CNT1 = 0x160,
    .FREQ_DB_CNT2 = 0x164,
    .FREQ_DB_CNT3 = 0x168,
    .FREQ_DB_CNT4 = 0x16c,
    .FREQ_DB_CNT5 = 0x170,

    .PWM0_START_POINT = 0x174,
    .PWM0_END_POINT = 0x178,
    .PWM1_START_POINT = 0x17c,
    .PWM1_END_POINT = 0x180,
    .PWM2_START_POINT = 0x184,
    .PWM2_END_POINT = 0x188,
    .PWM3_START_POINT = 0x18c,
    .PWM3_END_POINT = 0x190,
    .PWM4_START_POINT = 0x194,
    .PWM4_END_POINT = 0x198,
    .PWM5_START_POINT = 0x19c,
    .PWM5_END_POINT = 0x1a0,

    .PWM2ADC_CNT_H0 = 0x1a4,
    .PWM2ADC_CNT_L0 = 0x1a8,
    .PWM2ADC_CNT_H1 = 0x1ac,
    .PWM2ADC_CNT_L1 = 0x1b0,
    .PWM2ADC_CNT_H2 = 0x1b4,
    .PWM2ADC_CNT_L2 = 0x1b8,
    .PWM2ADC_CNT_H3 = 0x1bc,
    .PWM2ADC_CNT_L3 = 0x1c0,
    .PWM2ADC_CNT_H4 = 0x1c4,
    .PWM2ADC_CNT_L4 = 0x1c8,
    .PWM2ADC_CNT_H5 = 0x1cc,
    .PWM2ADC_CNT_L5 = 0x1d0,

    .PWM_START_TOGGLE_MODE = 0x1d4,
    .PWM_END_TOGGLE_MODE = 0x1d8,
    .PWM_OUTPUT_DISABLE = 0x1dc,
    .PWM_UPDATE_CNT0 = 0x1e0,
    .PWM_UPDATE_CNT1 = 0x1e4,
    .PWM_UPDATE_CNT2 = 0x1e8,
    .PWM_UPDATE_CNT3 = 0x1ec,
    .PWM_UPDATE_CNT4 = 0x1f0,
    .PWM_UPDATE_CNT5 = 0x1f4,

    .PWM_INTR_MASK = 0xf00,
    .PWM_INTR_STATUS = 0xf04,
    .PWM_INTR_CLR = 0xf08,

};

static struct cvi_pwm_regs_t *cvi_pwm_reg = &cv184x_pwm_reg;

#define PWM_HLPERIOD0(reg_base)     *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->HLPERIOD0))
#define PWM_PERIOD0(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PERIOD0))
#define PWM_HLPERIOD1(reg_base)     *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->HLPERIOD1))
#define PWM_PERIOD1(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PERIOD1))
#define PWM_HLPERIOD2(reg_base)     *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->HLPERIOD2))
#define PWM_PERIOD2(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PERIOD2))
#define PWM_HLPERIOD3(reg_base)     *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->HLPERIOD3))
#define PWM_PERIOD3(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PERIOD3))
#define PWM_HLPERIOD4(reg_base)     *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->HLPERIOD4))
#define PWM_PERIOD4(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PERIOD4))
#define PWM_HLPERIOD5(reg_base)     *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->HLPERIOD5))
#define PWM_PERIOD5(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PERIOD5))

#define PWM_HLPERIODX(reg_base, _ch_)     *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->HLPERIOD0 + (_ch_ << 3)))
#define PWM_PERIODX(reg_base, _ch_)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PERIOD0 + (_ch_ << 3)))

#define PWM0_START_POINT(reg_base)     *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PWM0_START_POINT))
#define PWM0_END_POINT(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PWM0_END_POINT))
#define PWM1_START_POINT(reg_base)     *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PWM1_START_POINT))
#define PWM1_END_POINT(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PWM1_END_POINT))
#define PWM2_START_POINT(reg_base)     *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PWM2_START_POINT))
#define PWM2_END_POINT(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PWM2_END_POINT))
#define PWM3_START_POINT(reg_base)     *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PWM3_START_POINT))
#define PWM3_END_POINT(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PWM3_END_POINT))
#define PWM4_START_POINT(reg_base)     *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PWM4_START_POINT))
#define PWM4_END_POINT(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PWM4_END_POINT))
#define PWM5_START_POINT(reg_base)     *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PWM5_START_POINT))
#define PWM5_END_POINT(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PWM5_END_POINT))

#define PWM_START_POINTX(reg_base, _ch_)     *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PWM0_START_POINT + (_ch_ << 3)))
#define PWM_END_POINTX(reg_base, _ch_)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PWM0_END_POINT + (_ch_ << 3)))

#define CAP_FREQNUM(reg_base, _ch_)     *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->CAP_FREQ0NUM + _ch_ * 8))
#define CAP_FREQDATA(reg_base, _ch_)       *((__IM uint32_t *)(reg_base + cvi_pwm_reg->CAP_FREQ0DATA + _ch_ * 8))


#define PWM_POLARITY(reg_base)      *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->POLARITY))
#define PWM_PWMSTART(reg_base)      *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PWMSTART))
#define PWM_PWMDONE(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PWMDONE))
#define PWM_PWMUPDATE(reg_base)     *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PWMUPDATE))

#define PWM_PCOUNT0(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PCOUNT0))
#define PWM_PCOUNT1(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PCOUNT1))
#define PWM_PCOUNT2(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PCOUNT2))
#define PWM_PCOUNT3(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PCOUNT3))
#define PWM_PCOUNT4(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PCOUNT4))
#define PWM_PCOUNT5(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PCOUNT5))

#define PWM_PULSECOUNT0(reg_base)       *((__IM uint32_t *)(reg_base + cvi_pwm_reg->PULSECOUNT0))
#define PWM_PULSECOUNT1(reg_base)       *((__IM uint32_t *)(reg_base + cvi_pwm_reg->PULSECOUNT1))
#define PWM_PULSECOUNT2(reg_base)       *((__IM uint32_t *)(reg_base + cvi_pwm_reg->PULSECOUNT2))
#define PWM_PULSECOUNT3(reg_base)       *((__IM uint32_t *)(reg_base + cvi_pwm_reg->PULSECOUNT3))
#define PWM_PULSECOUNT4(reg_base)       *((__IM uint32_t *)(reg_base + cvi_pwm_reg->PULSECOUNT4))
#define PWM_PULSECOUNT5(reg_base)       *((__IM uint32_t *)(reg_base + cvi_pwm_reg->PULSECOUNT5))

#define PWM_SHIFTCOUNT0(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->SHIFTCOUNT0))
#define PWM_SHIFTCOUNT1(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->SHIFTCOUNT1))
#define PWM_SHIFTCOUNT2(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->SHIFTCOUNT2))
#define PWM_SHIFTCOUNT3(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->SHIFTCOUNT3))
#define PWM_SHIFTCOUNT4(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->SHIFTCOUNT4))
#define PWM_SHIFTCOUNT5(reg_base)       *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->SHIFTCOUNT5))

#define PWM_SHIFTSTART(reg_base)        *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->SHIFTSTART))

#define CAP_FREQEN(reg_base)        *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->CAP_FREQEN))
#define CAP_FREQDONE_NUM(reg_base, _ch_)  *((__IM uint32_t *)(reg_base + cvi_pwm_reg->CAP_FREQ0DONE_NUM + _ch_ * 4))

#define PWM_PWM_OE(reg_base)        *((__IOM uint32_t *)(reg_base + cvi_pwm_reg->PWM_OE))


/*! PWM Configure Register, 	offset: 0x00 */
#define CVI_PWM_HIGH_PERIOD_Pos                         (0U)
#define CVI_PWM_HIGH_PERIOD_Msk                         (0xffffffff)

#define CVI_PWM_PERIOD_Pos                              (0U)
#define CVI_PWM_PERIOD_Msk                              (0xffffffff)

#define CVI_PWM_POLARITY_CH_Pos(_ch_)                     (_ch_)
#define CVI_PWM_POLARITY_CH_Msk(_ch_)                     (1U << CVI_PWM_POLARITY_CH_Pos(_ch_))
#define CVI_PWM_POLARITY_CH_HIGH(_ch_)                    CVI_PWM_POLARITY_CH_Msk(_ch_)

#define CVI_PWM_START_CH_Pos(_ch_)                        (_ch_)
#define CVI_PWM_START_CH_Msk(_ch_)                        (1U << CVI_PWM_START_CH_Pos(_ch_))
#define CVI_PWM_START_CH_EN(_ch_)                         CVI_PWM_START_CH_Msk(_ch_)

#define CVI_PWM_OUTPUT_CH_Pos(_ch_)                       (_ch_)
#define CVI_PWM_OUTPUT_CH_Msk(_ch_)                       (1U << CVI_PWM_OUTPUT_CH_Pos(_ch_))
#define CVI_PWM_OUTPUT_CH_EN(_ch_)                        CVI_PWM_OUTPUT_CH_Msk(_ch_)

#define CVI_CAP_FREQNUM_CH_Pos                             (0U)
#define CVI_CAP_FREQNUM_CH_Msk                             (0xffffffff)

#define CVI_CAP_FREQEN_Pos(_ch_)                           (_ch_)
#define CVI_CAP_FREQEN_Msk(_ch_)                           (1U << CVI_CAP_FREQEN_Pos(_ch_))
#define CVI_CAP_FREQEN(_ch_)                                CVI_CAP_FREQEN_Msk(_ch_)

#define CVI_CAP_FREQDONE_NUM_Poa                               (0U)
#define CVI_CAP_FREQDONE_NUM_Msk                               (0xffffffff)

#define CVI_CAP_FREQDATA_pos                                 (0U)
#define CVI_CAP_FREQDATA_msk                                 (0xffffffff)

static inline void cvi_pwm_set_high_period_ch(unsigned long reg_base, uint32_t ch, uint64_t value)
{
    pr_debug("write reg %p value %llu\n", &PWM_HLPERIODX(reg_base, ch), value);
    PWM_HLPERIODX(reg_base, ch) = value;
    PWM_START_POINTX(reg_base, ch) = 0;
    PWM_END_POINTX(reg_base, ch) = value;
}

static inline void cvi_pwm_set_period_ch(unsigned long reg_base, uint32_t ch, uint64_t value)
{
    pr_debug("write reg %p value %llu\n", &PWM_PERIODX(reg_base, ch), value);
    PWM_PERIODX(reg_base, ch) = value;
}

static inline void cvi_pwm_set_polarity_high_ch(unsigned long reg_base, uint32_t ch)
{
    PWM_POLARITY(reg_base) |= CVI_PWM_POLARITY_CH_HIGH(ch);
}

static inline void cvi_pwm_set_polarity_low_ch(unsigned long reg_base, uint32_t ch)
{
    pr_debug("write reg %p value 0x%x\n", &PWM_POLARITY(reg_base), ~CVI_PWM_POLARITY_CH_HIGH(ch));
    PWM_POLARITY(reg_base) &= ~CVI_PWM_POLARITY_CH_HIGH(ch);
}

static inline uint32_t cvi_pwm_get_polarity(unsigned long reg_base, uint32_t ch)
{
    return (PWM_POLARITY(reg_base) & CVI_PWM_POLARITY_CH_Msk(ch));
}

static inline void cvi_pwm_start_en_ch(unsigned long reg_base, uint32_t ch)
{
    PWM_PWMUPDATE(reg_base) = 0xffffffff;
    PWM_PWMSTART(reg_base) |= CVI_PWM_START_CH_EN(ch);
}

static inline void cvi_pwm_start_dis_ch(unsigned long reg_base, uint32_t ch)
{
    PWM_PWMSTART(reg_base) &= ~CVI_PWM_START_CH_EN(ch);
}

static inline void cvi_pwm_output_en_ch(unsigned long reg_base, uint32_t ch)
{
    pr_debug("write reg %p value 0x%x\n", &PWM_PWM_OE(reg_base), CVI_PWM_OUTPUT_CH_EN(ch));
    PWM_PWM_OE(reg_base) |= CVI_PWM_OUTPUT_CH_EN(ch);
}

static inline void cvi_pwm_input_en_ch(unsigned long reg_base, uint32_t ch)
{
    PWM_PWM_OE(reg_base) &= ~CVI_PWM_OUTPUT_CH_EN(ch);
}

static inline void cvi_cap_set_freqnum_ch(unsigned long reg_base, uint32_t ch, uint32_t value)
{
    CAP_FREQNUM(reg_base, ch) = value;
}

static inline void cvi_cap_freq_en_ch(unsigned long reg_base, uint32_t ch)
{
    CAP_FREQEN(reg_base) |= CVI_CAP_FREQEN(ch);
}

static inline void cvi_cap_freq_dis_ch(unsigned long reg_base, uint32_t ch)
{
    CAP_FREQEN(reg_base) &= ~CVI_CAP_FREQEN(ch);
}

static inline uint32_t cvi_cap_get_freq_done_num_ch(unsigned long reg_base, uint32_t ch)
{
    return CAP_FREQDONE_NUM(reg_base, ch);
}

static inline uint32_t cvi_cap_get_freq_data_ch(unsigned long reg_base, uint32_t ch)
{
    return CAP_FREQDATA(reg_base, ch);
}

/**
 * Check channel is exceed max channel num
*/
#define CVI_PWM_CHECK_CHANNEL_NUM(_CH_)                          ((_CH_ >= PWM_CHANNEL_NUM) ? CSI_ERROR : CSI_OK)


#ifdef __cplusplus
}
#endif

#endif

/*
 * Copyright (C) 2017-2019 Alibaba Group Holding Limited
 */
/******************************************************************************
 * @file     yun_pmu.c
 * @brief    source file for pmu.
 * @version  V1.0
 * @date     10. Aug 2019
 * @vendor   csky
 * @chip     pangu
 ******************************************************************************/

#include <csi_config.h>
#include <csi_core.h>
#include <soc.h>
#include <pmu_regs.h>
#include <yun_pmu.h>
#include <drv/io.h>

#define ERR_PMU(errno) (errno)
#define PMU_NULL_PARAM_CHK(para)
#define ATTRIBUTE_DATA __attribute__((section(".tcm1")))

#define RUN_ADDR    0x18000000
#define STORAGE_ADDR    0x80a0000
typedef struct {
    uint8_t idx;
} ck_pmu_priv_t;

typedef enum {
    WAIT_MODE = 0,
    DOZE_MODE,
    STOP_MODE,
    STANDBY_MODE,
    SLEEP_MODE
} lpm_mode_e;

extern int32_t arch_do_cpu_resume(void);
extern int32_t arch_resume_context(void);

void yun_set_pmu_clk_src(clk_src_t src)
{
    if (src == ILS_CLK) {
        YUN_PMU->PMU_CLKSRCSEL |= PMU_CLKSRCSEL_EILS_SEL_ILS;
    } else if (src == ELS_CLK) {
        YUN_PMU->PMU_CLKSRCSEL &= ~PMU_CLKSRCSEL_EILS_SEL_ILS;
    }
}

void drv_set_aon_clk_src(clk_src_t src)
{
    if (src == ELS_CLK) {
        YUN_PMU->PMU_CLKSRCSEL &= ~PMU_CLKSRCSEL_EILS_SEL_Msk;
    } else if (src == ILS_CLK) {
        YUN_PMU->PMU_CLKSRCSEL |= PMU_CLKSRCSEL_EILS_SEL_Msk;
    }
}

void drv_reset_cpu1(void)
{
    YUN_PMU->PMU_CPU12SWRST |= PMU_CPU12SWRST_CPU1_RST_EN;
    YUN_PMU->PMU_CPU12SWRST &= ~PMU_CPU12SWRST_CPU1_RST_EN;
}

void drv_reset_cpu2(void)
{
    YUN_PMU->PMU_CPU12SWRST |= PMU_CPU12SWRST_CPU2_RST_EN;
    YUN_PMU->PMU_CPU12SWRST &= ~PMU_CPU12SWRST_CPU2_RST_EN;
}

void yun_enable_lpm_mode(void)
{
    YUN_PMU->PMU_LPCR |= PMU_LPCR_LPEN;
}

void yun_enter_hw_vad0(void)
{
    YUN_PMU->PMU_LPCR &= ~PMU_LPCR_CHANGE_VOLT_ENTER_HW_VAD1;
}

void yun_enter_hw_vad1(void)
{
    YUN_PMU->PMU_LPCR |= PMU_LPCR_CHANGE_VOLT_ENTER_HW_VAD1;
}

uint32_t yun_is_boot_from_lpm(void)
{
    return ((YUN_PMU->PMU_LPCR & PMU_LPCR_BOOT_FROM_LP) == PMU_LPCR_BOOT_FROM_LP);
}

void yun_set_wakeup_source(uint8_t wakeupn)
{
    YUN_PMU->PMU_WKUPMASK |= (1 << wakeupn);
}

void yun_clear_wakeup_source(uint8_t wakeupn)
{
    YUN_PMU->PMU_WKUPMASK &= ~(1 << wakeupn);
}

yun_reset_status_e yun_get_reset_status(void)
{
    return 32 - __FF1(YUN_PMU->PMU_RSTSTA);
}

void yun_enable_ils_clk(void)
{
    YUN_PMU->PMU_RCCTRL |= PMU_RCCTRL_ILS_CLK_EN;
}

void yun_disable_ils_clk(void)
{
    YUN_PMU->PMU_RCCTRL &= ~PMU_RCCTRL_ILS_CLK_EN;
}

void yun_enable_els_clk(void)
{
    YUN_PMU->PMU_RCCTRL |= PMU_RCCTRL_ELS_CLK_EN;
}

void yun_disable_els_clk(void)
{
    YUN_PMU->PMU_RCCTRL &= ~PMU_RCCTRL_ELS_CLK_EN;
}

void yun_set_cpu0_clk_src(clk_src_t src)
{
    if (src == PLL_CLK) {
        YUN_CPR0->CPR0_CPU0CLK_SEL &= ~CPR0_CPU0CLK_SEL_CPU0CLK_SEL_EHS;
    } else if (src == EHS_CLK) {
        YUN_CPR0->CPR0_CPU0CLK_SEL |= CPR0_CPU0CLK_SEL_CPU0CLK_SEL_EHS;
    }
}

void yun_set_cpu2_clk_src(clk_src_t src)
{
    if (src == PLL_CLK) {
        YUN_CPR1->CPR1_CPU2CLK_SEL &= ~CPR1_CPU2CLK_SEL_CPU2CLK_SEL_EHS;
    } else if (src == EHS_CLK) {
        YUN_CPR1->CPR1_CPU2CLK_SEL |= CPR1_CPU2CLK_SEL_CPU2CLK_SEL_EHS;
    }
}

void drv_clk_enable(clk_module_t module)
{
    uint32_t val;
    uint8_t bit_mask = 0x1U;

    if (module == AONTIM_CLK || module == RTC_CLK) {
        bit_mask = 0x11U;
    }

    if (module == TIM1_CLK) {
        *(volatile uint32_t *)0x30000010 &= ~(0x01 << 19);
    }

    val = getreg32((volatile uint32_t *)module);
    val |= bit_mask;
    putreg32(val, (volatile uint32_t *)module);
}

void drv_clk_disable(clk_module_t module)
{
    uint32_t val;
    uint8_t bit_mask = 0x1U;

    if (module == AONTIM_CLK || module == RTC_CLK) {
        bit_mask = 0x11U;
    }

    val = getreg32((volatile uint32_t *)module);
    val &= ~bit_mask;
    putreg32(val, (volatile uint32_t *)module);
}

void resume_context_from_stop_mode(void)
{
    /* config SDRAM on clk 144M */
    *(volatile uint32_t *)0x31000014 = 0x1;
    *(volatile uint32_t *)0x1a000054 = 0x8;
    *(volatile uint32_t *)0x1a000000 = 0x1c0f68;
    *(volatile uint32_t *)0x1a000004 = 0x029e949f;
    *(volatile uint32_t *)0x1a00000c = 0x3009;
    *(volatile uint32_t *)0x30000210 = 0x1f;
    *(volatile uint32_t *)0x3b800004 = 0x100;
}

int32_t drv_get_boot_type(void)
{
    return yun_is_boot_from_lpm();
}

void yun_pll_power_down(int en)
{
    pmu_reg_t *reg = YUN_PMU;

    if (en) {
        reg->PMU_PLLCTRL |= (1 << 18) | (1 << 11);
    } else {
        reg->PMU_PLLCTRL &= ~((1 << 18) | (1 << 11));
    }
}

static void pll_set_doubling(uint16_t val)
{
    pmu_reg_t *reg = YUN_PMU;
    uint32_t temp = reg->PMU_PLLCTRL;
    temp &= ~(0xfff << 20);
    temp |= (val & 0xfff) << 20;

    reg->PMU_PLLCTRL = temp;
}

static void pll_set_prescale(uint8_t val)
{
    pmu_reg_t *reg = YUN_PMU;
    uint32_t temp = reg->PMU_PLLCTRL;
    temp &= ~(0x3f << 12);
    temp |= (val & 0x3f) << 12;

    reg->PMU_PLLCTRL = temp;
}

static void ATTRIBUTE_DATA pll_set_div1(uint8_t val)
{
    pmu_reg_t *reg = YUN_PMU;
    uint32_t temp = reg->PMU_PLLCTRL;
    temp &= ~(0x7 << 4);
    temp |= (val & 0x7) << 4;

    reg->PMU_PLLCTRL = temp;
}

static void pll_set_div2(uint8_t val)
{
    pmu_reg_t *reg = YUN_PMU;
    uint32_t temp = reg->PMU_PLLCTRL;
    temp &= ~(0x7 << 8);
    temp |= (val & 0x7) << 8;

    reg->PMU_PLLCTRL = temp;
}

static void pll_bypass(int en)
{
    pmu_reg_t *reg = YUN_PMU;

    if (en) {
        reg->PMU_PLLCTRL |= 1 << 2;
    } else {
        reg->PMU_PLLCTRL &= ~(1 << 2);
    }
}

static void pll_change_sw_trigger()
{
    pmu_reg_t *reg = YUN_PMU;
    reg->PMU_LPCR |= 0x4;
}

void pll_frac_bypass(int en)
{
    pmu_reg_t *reg = YUN_PMU;

    if (en) {
        reg->PMU_PLLCTRL_FRAC |= 1 << 2;
    } else {
        reg->PMU_PLLCTRL_FRAC &= ~(1 << 2);
    }
}

static void pll_frac_set_prescale(uint8_t val)
{
    pmu_reg_t *reg = YUN_PMU;
    uint32_t temp = reg->PMU_PLLCTRL_FRAC;
    temp &= ~(0x3f << 12);
    temp |= (val & 0x3f) << 12;

    reg->PMU_PLLCTRL_FRAC = temp;
}

static void pll_frac_set_doubling(uint16_t val)
{
    pmu_reg_t *reg = YUN_PMU;
    uint32_t temp = reg->PMU_PLLCTRL_FRAC;

    temp &= ~(0xfff << 20);
    temp |= (val & 0xfff) << 20;

    reg->PMU_PLLCTRL_FRAC = temp;
}

static void pll_frac_set_div1(uint8_t val)
{
    pmu_reg_t *reg = YUN_PMU;

    uint32_t temp = reg->PMU_PLLCTRL_FRAC;
    temp &= ~(0x7 << 4);
    temp |= (val & 0x7) << 4;

    reg->PMU_PLLCTRL = temp;
}

static void pll_frac_set_div2(uint8_t val)
{
    pmu_reg_t *reg = YUN_PMU;
    uint32_t temp = reg->PMU_PLLCTRL_FRAC;
    temp &= ~(0x7 << 8);
    temp |= (val & 0x7) << 8;

    reg->PMU_PLLCTRL = temp;
}

static void pll_set_frac(uint32_t val)
{
    pmu_reg_t *reg = YUN_PMU;

    reg->PMU_FRACPLLFRAC = val & 0xffffff;
}

static uint32_t get_pll_frac_prescal()
{
    pmu_reg_t *reg = YUN_PMU;
    uint32_t temp = reg->PMU_PLLCTRL_FRAC;
    temp &= 0x3f << 12;
    temp >>= 12;

    return temp;
}

static uint32_t get_pll_frac_freq_doubling()
{
    pmu_reg_t *reg = YUN_PMU;
    uint32_t temp = reg->PMU_PLLCTRL_FRAC;

    temp &= 0xfff << 20;
    temp >>= 20;

    return temp;
}

static uint32_t get_pll_frac_div1()
{
    pmu_reg_t *reg = YUN_PMU;
    uint32_t temp = reg->PMU_PLLCTRL_FRAC;
    temp &= 0x7 << 4;
    temp >>= 4;

    return temp;
}

static uint32_t get_pll_frac_div2()
{
    pmu_reg_t *reg = YUN_PMU;
    uint32_t temp = reg->PMU_PLLCTRL_FRAC;
    temp &= 0x7 << 8;
    temp >>= 8;

    return temp;
}

static uint32_t get_pll_frca()
{
    pmu_reg_t *reg = YUN_PMU;
    uint32_t temp = reg->PMU_FRACPLLFRAC;
    temp &= 0xffffff;

    return temp;
}

#define PLL_FRAC_CONSTANT 0X1000000

static uint32_t get_pll_frac_freq()
{
    pmu_reg_t *reg = YUN_PMU;

    if (reg->PMU_PLLCTRL_FRAC & (1 << 2)) {
        return EHS_VALUE;
    }

    uint32_t freq = 0;
    uint32_t prescal = get_pll_frac_prescal();
    uint32_t doubling = get_pll_frac_freq_doubling();
    uint32_t div1 = get_pll_frac_div1();
    uint32_t div2 = get_pll_frac_div2();

    double frca = EHS_VALUE * (double)get_pll_frca();
    frca /= PLL_FRAC_CONSTANT;
    freq = (EHS_VALUE * doubling + frca) / prescal / div1 / div2;

    return freq;
}

static uint32_t get_pll_prescal()
{
    pmu_reg_t *reg = YUN_PMU;
    uint32_t temp = reg->PMU_PLLCTRL;
    temp &= 0x3f << 12;
    temp >>= 12;

    return temp;
}

static uint32_t get_pll_freq_doubling()
{
    pmu_reg_t *reg = YUN_PMU;
    uint32_t temp = reg->PMU_PLLCTRL;

    temp &= 0xfff << 20;
    temp >>= 20;

    return temp;
}

static uint32_t get_pll_div1()
{
    pmu_reg_t *reg = YUN_PMU;
    uint32_t temp = reg->PMU_PLLCTRL;
    temp &= 0x7 << 4;
    temp >>= 4;

    return temp;
}

static uint32_t get_pll_div2()
{
    pmu_reg_t *reg = YUN_PMU;
    uint32_t temp = reg->PMU_PLLCTRL;
    temp &= 0x7 << 8;
    temp >>= 8;

    return temp;
}

static uint32_t get_pll_freq()
{
    pmu_reg_t *reg = YUN_PMU;

    if (reg->PMU_PLLINTRCTRL & (1 << 2)) {
        return EHS_VALUE;
    }

    uint32_t freq = 0;
    uint32_t prescal = get_pll_prescal();
    uint32_t doubling = get_pll_freq_doubling();
    uint32_t div1 = get_pll_div1();
    uint32_t div2 = get_pll_div2();

    freq = EHS_VALUE * doubling / prescal / div1 / div2;

    return freq;
}

static uint32_t get_pll_cpr0_div()
{
    pmu_reg_t *reg = YUN_PMU;

    if (reg->PMU_SYSCLKDIVEN & 1) {
        uint32_t temp = reg->PMU_SYSCLKDIVEN;
        temp &= 0xf << 4;
        temp >>= 4;
        return temp + 2;
    }

    return 1;
}

static uint32_t get_pll_cpr1_div()
{
    pmu_reg_t *reg = YUN_PMU;

    if (reg->PMU_SYSCLKDIVEN & (1 << 8)) {
        uint32_t temp = reg->PMU_SYSCLKDIVEN;
        temp &= 0xf << 12;
        temp >>= 12;
        return temp + 2;
    }

    return 1;
}

static uint32_t get_ahb0_sub1_clk_div()
{
    cpr0_reg_t *reg = YUN_CPR0;
    uint32_t temp = reg->CPR0_SYSCLK0_DIV_CTL;
    temp &= 0x3;
    temp *= 2;

    if (temp == 0) {
        temp = 1;
    }

    return temp;
}

static uint32_t get_ahb0_sub2_clk_div()
{
    cpr0_reg_t *reg = YUN_CPR0;
    uint32_t temp = reg->CPR0_SYSCLK0_DIV_CTL;
    temp &= 0x3 << 4;
    temp >>= 4;
    temp *= 2;

    if (temp == 0) {
        temp = 1;
    }

    return temp;
}

static uint32_t get_apb0_clk_div()
{
    cpr0_reg_t *reg = YUN_CPR0;
    uint32_t temp = reg->CPR0_SYSCLK0_DIV_CTL;
    temp &= 0xf << 8;
    temp >>= 8;
    temp += 1;

    return temp;
}

static uint32_t get_apb1_clk_div()
{
    cpr1_reg_t *reg = YUN_CPR1;
    uint32_t temp = reg->CPR1_SYSCLK1_DIV_CTL & 0xf;
    temp += 1;

    return temp;
}

static void pll_cpr0_clk_enable(int en)
{
    pmu_reg_t *reg = YUN_PMU;

    if (en) {
        reg->PMU_SYSCLKDIVEN |= 1;
    } else {
        reg->PMU_SYSCLKDIVEN &= ~1;
    }
}

static void pll_cpr1_clk_enable(int en)
{
    pmu_reg_t *reg = YUN_PMU;

    if (en) {
        reg->PMU_SYSCLKDIVEN |= 1 << 8;
    } else {
        reg->PMU_SYSCLKDIVEN &= ~(1 << 8);
    }
}

/*value 2~17*/
static void pll_cpr0_clk_div(uint8_t val)
{
    if (val < 2) {
        val = 2;
    }

    val -= 2;
    pmu_reg_t *reg = YUN_PMU;
    uint32_t temp = reg->PMU_SYSCLKDIVEN;

    val &= 0xf;
    temp &= ~(0xf << 4);
    temp |= val << 4;

    reg->PMU_SYSCLKDIVEN = temp;
}

/*value 2~17*/
static void pll_cpr1_clk_div(uint8_t val)
{
    if (val < 2) {
        val = 2;
    }

    val -= 2;
    pmu_reg_t *reg = YUN_PMU;
    uint32_t temp = reg->PMU_SYSCLKDIVEN;

    val &= 0xf;
    temp &= ~(0xf << 12);
    temp |= val << 12;

    reg->PMU_SYSCLKDIVEN = temp;
}

static void cpr0_ahb0_sub1_clk_div(uint8_t val)
{
    cpr0_reg_t *reg = YUN_CPR0;
    uint32_t temp = reg->CPR0_SYSCLK0_DIV_CTL;
    temp &= ~0x3;
    temp |= val & 0x3;

    reg->CPR0_SYSCLK0_DIV_CTL = temp;
}

static void cpr0_ahb0_sub2_clk_div(uint8_t val)
{
    cpr0_reg_t *reg = YUN_CPR0;
    uint32_t temp = reg->CPR0_SYSCLK0_DIV_CTL;
    temp &= ~(0x3 << 4);
    temp |= (val & 0x3) << 4;

    reg->CPR0_SYSCLK0_DIV_CTL = temp;
}

/*value 2~9*/
static void cpr0_apb0_clk_div(uint8_t val)
{
    cpr0_reg_t *reg = YUN_CPR0;
    uint32_t temp = reg->CPR0_SYSCLK0_DIV_CTL;
    temp &= ~(0xf << 8);
    temp |= (val & 0xf) << 8;

    reg->CPR0_SYSCLK0_DIV_CTL = temp;
}

/*value 2~9*/
static void cpr1_apb1_clk_div(uint8_t val)
{
    cpr1_reg_t *reg = YUN_CPR1;
    uint32_t temp = reg->CPR1_SYSCLK1_DIV_CTL;
    temp &= ~(0xf << 8);
    temp |= (val & 0xf) << 8;

    reg->CPR1_SYSCLK1_DIV_CTL = temp;
}

static uint32_t get_cpu0_cpu1_freq()
{
    cpr0_reg_t *reg = YUN_CPR0;
    uint32_t div = get_pll_cpr0_div();

    if (reg->CPR0_CPU0CLK_SEL) {
        return EHS_VALUE;
    }

    return get_pll_freq() / div;
}

static uint32_t get_cpu2_freq()
{
    cpr1_reg_t *reg = YUN_CPR1;
    uint32_t div = get_pll_cpr1_div();

    if (reg->CPR1_CPU2CLK_SEL) {
        return EHS_VALUE;
    }

    return get_pll_freq() / div;
}

void drv_lclk_select_src(clk_src_t src)
{
    pmu_reg_t *reg = YUN_PMU;

    if (src == ELS_CLK) {
        reg->PMU_CLKSRCSEL = 1;
    } else {
        reg->PMU_CLKSRCSEL = 0;
    }
}

boot_reson_t drv_get_boot_reason(void)
{
    pmu_reg_t *reg = YUN_PMU;
    uint32_t val = reg->PMU_RSTSTA;
    reg->PMU_RSTSTA = val;

    if (val == 8) {
        return CPU0_RESET;
    } else if (val == 4) {
        return WDG_RESET;
    } else if (val == 2) {
        return MCU_RESET;
    } else if (val == 1) {
        return POWER_RESET;
    }

    return val;
}

void yun_pll_config(pll_config_t *param)
{
    pmu_reg_t *reg = YUN_PMU;

    if (param->bypass_ehs_en) {
        pll_bypass(1);
        return;
    }

    pll_bypass(0);
    pll_set_prescale(param->prescale);
    pll_set_doubling(param->freq_doubling);
    pll_set_div1(param->pll_clk_div1);
    pll_set_div2(param->pll_clk_div2);
    reg->PMU_QSPI_CNT = 0x1e;
    pll_change_sw_trigger();

    while ((reg->PMU_PLLCTRL & 1) == 0);
}

void yun_set_cpu0_cpu1_clk_div(uint8_t val)
{
    pll_cpr0_clk_div(val);
}

void yun_set_cpu2_clk_div(uint8_t val)
{
    pll_cpr1_clk_div(val);
}

void yun_cpu0_cpu1_clk_enable(int en)
{
    pll_cpr0_clk_enable(en);
}

void yun_cpu2_clk_enable(int en)
{
    pll_cpr1_clk_enable(en);
}

void yun_qspi_clock_en(int en)
{
    pmu_reg_t *reg = YUN_PMU;

    if (en) {
        reg->PMU_SYSCLKDIVEN |= 1 << 16;
    } else {
        reg->PMU_SYSCLKDIVEN &= ~(1 << 16);
    }
}

void yun_qspi_clk_div(uint8_t val)
{
    if (val < 2) {
        val = 2;
    }

    val -= 2;
    pmu_reg_t *reg = YUN_PMU;
    uint32_t temp = reg->PMU_SYSCLKDIVEN;

    val &= 0x7;
    temp &= ~(0x7 << 20);
    temp |= val << 20;

    reg->PMU_SYSCLKDIVEN = temp;
}

int yun_get_cpu_freq(int idx)
{
    if (idx == 0 || idx == 1) {
        return get_cpu0_cpu1_freq();
    } else if (idx == 2) {
        return get_cpu2_freq();
    }

    return 0;
}

void yun_set_ahb0_sub1_clk_div(uint8_t val)
{
    cpr0_ahb0_sub1_clk_div(val);
}

uint32_t yun_get_ahb0_sub1_freq()
{
    uint32_t div = get_ahb0_sub1_clk_div();
    return get_cpu0_cpu1_freq() / div;
}

void yun_set_ahb0_sub2_clk_div(uint8_t val)
{
    cpr0_ahb0_sub2_clk_div(val);
}

uint32_t yun_get_ahb0_sub2_freq()
{
    uint32_t div = get_ahb0_sub2_clk_div();
    return get_cpu0_cpu1_freq() / div;
}

void yun_set_apb0_clk_div(uint8_t val)
{
    cpr0_apb0_clk_div(val);
}

uint32_t yun_get_apb0_freq()
{
    uint32_t div = get_apb0_clk_div();
    return get_cpu0_cpu1_freq() / div;
}

void yun_set_apb1_clk_div(uint8_t val)
{
    cpr1_apb1_clk_div(val);
}

uint32_t yun_get_apb1_freq()
{
    uint32_t div = get_apb1_clk_div();

    return get_cpu2_freq() / div;
}

void yun_audio_clk_config(pll_frac_config_t *param)
{
    pmu_reg_t *reg = YUN_PMU;

    if (param->bypass_ehs_en) {
        pll_frac_bypass(1);
        return;
    }

    pll_frac_bypass(0);
    pll_frac_set_prescale(param->prescale);
    pll_frac_set_doubling(param->freq_doubling);
    pll_frac_set_div1(param->pll_clk_div1);
    pll_frac_set_div2(param->pll_clk_div2);
    pll_set_frac(param->frac_val);

    reg->PMU_QSPI_CNT = 0x1e;
    pll_change_sw_trigger();

    while ((reg->PMU_PLLCTRL_FRAC & 1) == 0);
}

uint32_t yun_get_audio_freq()
{
    return get_pll_frac_freq();
}

int32_t drv_set_cpu_freq(int idx, cpu_freq_t freq)
{
    if (idx == 0 || idx == 1) {
        if (freq != CPU_BYPASS_EHS) {
            uint32_t div = get_pll_freq() / freq;
            yun_set_cpu0_clk_src(PLL_CLK);
            yun_set_cpu0_cpu1_clk_div(div);
        } else {
            yun_set_cpu0_clk_src(EHS_CLK);
        }

        yun_set_apb0_clk_div(1);
    } else {
        return -1;
    }

    return 0;
}

ATTRIBUTE_DATA inline  void yun_sdram_auto_flash_enable(int en)
{
#define PREG32(addr) *((volatile unsigned int *)addr)

    if (en) {
        csi_dcache_clean_invalid();
        PREG32(0x1a00000c) |= 0x2;

        while ((PREG32(0x1a00000c) & (1 << 11)) == 0);
    } else {
        PREG32(0x1a00000c) &= (~0x2);

        while ((PREG32(0x1a00000c) & (1 << 11)) == 1);
    }
}

ATTRIBUTE_DATA static void delay(int ms)
{
    volatile int i, j;

    for (i = 0; i < ms; i++) {
        for (j = 0; j < i; j++) {
            ;
        }
    }
}

ATTRIBUTE_DATA int yun_enter_lpm(void)
{
#define PMU_WKUPMASK 0x30000018
#define PMU_LPCR     0x30000014
#define PMU_DLC_IFR  0x30000210
    PREG32(PMU_WKUPMASK) = 0xF;
    PREG32(PMU_LPCR) = 0x1;
    PREG32(PMU_DLC_IFR) = 0x1f;
    delay(1000);
    PREG32(0xe000e280) = 0x40;
    PREG32(0x90000200) = 0x1;
    delay(1000);

    uint32_t state = csi_irq_save();

    csi_dcache_clean_invalid();

    PREG32(0x1a00000c) |= 0x2;

    while ((PREG32(0x1a00000c) & (1 << 11)) == 0);

    asm("wait");

    PREG32(0x1a00000c) &= (~0x2);

    while ((PREG32(0x1a00000c) & (1 << 11)) == 1);

    PREG32(0x31000014) = 0x1;
    PREG32(0x1a000100) = 0x0;
    PREG32(0x1a000054) = 0x8;
    PREG32(0x1a000000) = 0x1c0f68;
    PREG32(0x1a000004) = 0x029e949f;
    PREG32(0x1a00000c) = 0x3009;
    csi_irq_restore(state);

    while (PREG32(0x1a00000c) & 1) {;}

    return 0;
}


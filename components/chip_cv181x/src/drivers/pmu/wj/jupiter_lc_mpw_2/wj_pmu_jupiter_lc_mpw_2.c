/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     wj_pmu_jupiter_lc_mpw_2.c
 * @brief    CSI Source File for PMU Driver
 * @version  V1.0
 * @date     19. Feb 2020
 ******************************************************************************/
#include <csi_config.h>
#include <stdio.h>
#include <string.h>
#ifdef CONFIG_PM
#include <drv/io.h>
#include <drv/pm.h>
#include <drv/cvi_irq.h>
#include <drv/timer.h>
#include <soc.h>
#include <wj_pmu_jupiter_lc_mpw_2.h>


#define CONFIG_VIC_WAKEUP_BIT       31U
#define CSKY_SOC_WAKEUP_REG0        (WJ_IOC_BASE + 0x18)
#define CSKY_SOC_WAKEUP_LEVEL       (WJ_IOC_BASE + 0x78)

#define WAKEUP_IO       PA4

typedef enum {
    WAIT_MODE = 0U,
    DOZE_MODE,
    STOP_MODE,
    STANDBY_MODE,
    SLEEP_MODE
} lpm_mode_e;

extern int32_t arch_do_cpu_save(void);
extern int32_t arch_do_cpu_resume(void);
extern int32_t arch_resume_context(void);

extern csi_timer_t tick_timer;

#define CONFIG_PMU_REGISTER_NUM_SAVE  19U
static uint32_t pmu_regs_saved[CONFIG_PMU_REGISTER_NUM_SAVE];

#define CONFIG_CSITIMER_REGISTER_NUM_SAVE    2U
static uint32_t csitimer_regs_saved[CONFIG_CSITIMER_REGISTER_NUM_SAVE];

#define CONFIG_CLIC_REGISTER_NUM_SAVE    (CONFIG_IRQ_NUM + 1U)
static uint32_t clic_regs_saved[CONFIG_CLIC_REGISTER_NUM_SAVE];

#define CONFIG_CPU_REGISTER_NUM_SAVE    60U
uint32_t g_arch_cpu_saved[CONFIG_CPU_REGISTER_NUM_SAVE];

#define CONFIG_IOCTRL_REGISTER_NUM_SAVE 19U
uint32_t io_regs_saved[CONFIG_IOCTRL_REGISTER_NUM_SAVE];
/* Driver Capabilities */

static void do_prepare_sleep_action(void)
{
    uint8_t i;
    wj_pmu_reg_t *pbase = (wj_pmu_reg_t *)WJ_PMU_BASE;

    pmu_regs_saved[0] = pbase->DFCC;
    pmu_regs_saved[1] = pbase->PCR;
    pmu_regs_saved[2] = pbase->LC_WIME;
    pmu_regs_saved[3] = pbase->SRC_CLKMD;
    pmu_regs_saved[4] = pbase->CLK_RATIO;

    /* save clic register */
    clic_regs_saved[0] = CLIC->CLICCFG;

    for (i = 0U; i < CONFIG_IRQ_NUM; i++) {
        clic_regs_saved[i + 1U] = *((volatile uint32_t *)&CLIC->CLICINT[i]);
    }

    /* save the timer0 register */
    volatile uint32_t *ibase = (uint32_t *)(DW_TIMER0_BASE);
    csitimer_regs_saved[0] = *((volatile uint32_t *)ibase);
    csitimer_regs_saved[1] = *((volatile uint32_t *)(ibase + 8U));

    /* save the ioctrl register */
    for (i = 0U; i < sizeof(io_regs_saved) / 4U; i++) {
        if (i < 17U) {
            io_regs_saved[i] = *((volatile uint32_t *)ibase + i);
        } else if (i == 17U) {
            io_regs_saved[i] = *((volatile uint32_t *)(DW_GPIOB_BASE + 0x8U));
        }
    }
}

static void do_wakeup_sleep_action(void)
{
    uint8_t i;

    wj_pmu_reg_t *pbase = (wj_pmu_reg_t *)WJ_PMU_BASE;

    pbase->CLK_RATIO = pmu_regs_saved[4];

    pbase->DFCC = 0x1FU;
    while (pbase->DFCC);

    pbase->PCR = pmu_regs_saved[1];
    pbase->LC_WIME = pmu_regs_saved[2];

    if (pmu_regs_saved[3] & 0xf000U) {
        pbase->SRC_CLKMD = pmu_regs_saved[3] & 0xff00U;
        pbase->MODE_CHG_EN = 2U;

        while (pbase->MODE_CHG_EN);
        while ((pbase->CLKSTBR & 0x10U) != 0x10U);
    }

    pbase->SRC_CLKMD |= pmu_regs_saved[3] & 0xffU;
    pbase->MODE_CHG_EN = 2U;

    while (pbase->MODE_CHG_EN);

    /* resume clic register */
    CLIC->CLICCFG = clic_regs_saved[0];

    for (i = 0U; i < CONFIG_IRQ_NUM; i++) {
        *((volatile uint32_t *)&CLIC->CLICINT[i]) = clic_regs_saved[i + 1U];
    }

    /* resume the ioctrl register */
    volatile uint32_t *ibase = (uint32_t *)(DW_GPIOB_BASE + 0x100U);

    for (i = 0U; i < sizeof(io_regs_saved) / 4U; i++) {
        if (i < 17U) {
            *((volatile uint32_t *)ibase + i) = io_regs_saved[i];
        } else if (i == 17U) {
            *((volatile uint32_t *)(DW_GPIOB_BASE + 0x8)) = io_regs_saved[i];
        }
    }

    /* resume the timer0 register */
    volatile uint32_t *time_ibase = (uint32_t *)(DW_TIMER0_BASE);
    *((volatile uint32_t *)time_ibase) = csitimer_regs_saved[0];
    *((volatile uint32_t *)(time_ibase + 8U)) = csitimer_regs_saved[1];
}

static void pmu_action(csi_pm_dev_action_t action)
{
    switch (action) {
        case PM_DEV_SUSPEND:
            do_prepare_sleep_action();
            break;

        case PM_DEV_RESUME:
            do_wakeup_sleep_action();
            break;

        default:
            return;
    }
}

static void set_resume_func(wj_pmu_reg_t *pmu_base, uint32_t *func)
{
    pmu_base->USER_DEFINE[1] = (uint32_t)func;
}

void soc_sleep(lpm_mode_e mode)
{
    wj_pmu_reg_t *pmu_base = (wj_pmu_reg_t *)WJ_PMU_BASE;

    set_resume_func(pmu_base, (uint32_t *)&arch_resume_context);

    /* clear pend IRQ */
    for (uint8_t i = 0; i < 64; i++) {
        CLIC->CLICINT[i].IE = 0;
        CLIC->CLICINT[i].IP = 0;
    }

    csi_irq_enable((uint32_t)WJ_WAKEUP_IRQn);
    __disable_irq();
    if (mode == WAIT_MODE) {
        wj_pmu_lp_wait_clk_on_mode_enable(pmu_base);
        wj_pmu_allow_lp_mode(pmu_base);
        __WFI();
    } else if (mode == DOZE_MODE) {
        wj_pmu_lp_wait_clk_off_mode_enable(pmu_base);
        wj_pmu_allow_lp_mode(pmu_base);
        __WFI();
    } else if (mode == STOP_MODE) {
        wj_pmu_lp_retention_mode_enable(pmu_base);
        wj_pmu_allow_lp_mode(pmu_base);
        arch_do_cpu_save();
    } else if (mode == STANDBY_MODE) {
        soc_dcache_clean_invalid();
        wj_set_isram_dsram_ret1n_bypass(pmu_base);
        wj_set_bkup_sram_ret1n_bypass(pmu_base);
        wj_pmu_lp_standby_mode_enable(pmu_base);
        wj_pmu_allow_lp_mode(pmu_base);
        __STOP();
    }
}

/**
  \brief       choose the pmu mode to enter
  \param[in]   handle  pmu handle to operate.
  \param[in]   mode    \ref pmu_mode_e
  \return      error code
*/
csi_error_t soc_pm_enter_sleep(csi_pm_mode_t mode)
{
    csi_error_t ret = CSI_OK;

    switch (mode) {
        case PM_MODE_RUN:
            break;

        case PM_MODE_SLEEP_1:
            pmu_action(PM_DEV_SUSPEND);
            soc_sleep(WAIT_MODE);
            pmu_action(PM_DEV_RESUME);
            break;

        case PM_MODE_SLEEP_2:
            pmu_action(PM_DEV_SUSPEND);
            soc_sleep(DOZE_MODE);
            pmu_action(PM_DEV_RESUME);
            break;

        case PM_MODE_DEEP_SLEEP_1:
            pmu_action(PM_DEV_SUSPEND);
            soc_sleep(STOP_MODE);
            pmu_action(PM_DEV_RESUME);
            break;

        case PM_MODE_DEEP_SLEEP_2:
            pmu_action(PM_DEV_SUSPEND);
            soc_sleep(STANDBY_MODE);
            pmu_action(PM_DEV_RESUME);
            break;

        default:
            ret = CSI_ERROR;
    }

    __disable_irq();
    return ret;
}
/**
  \brief       Config the wakeup source.
  \param[in]   wakeup_num wakeup source num
  \param[in]   enable  flag control the wakeup source is enable or not
  \return      error code
*/
csi_error_t soc_pm_config_wakeup_source(uint32_t wakeup_num, bool enable)
{
    csi_error_t ret = CSI_OK;

    if (wakeup_num >= 35U) {
        ret = CSI_ERROR;
    } else {

        wj_pmu_reg_t *pmu_base = (wj_pmu_reg_t *)WJ_PMU_BASE;

        if (enable) {
            csi_vic_set_wakeup_irq(CONFIG_VIC_WAKEUP_BIT);

            if (wakeup_num == IOCTL_Wakeupn) {
                putreg32(1U << WAKEUP_IO, (uint32_t *)CSKY_SOC_WAKEUP_REG0);
                putreg32(0xffffffffU, (uint32_t *)CSKY_SOC_WAKEUP_LEVEL);
            }

            if (wakeup_num < 32U) {
                wj_set_lc_wakeup_source(pmu_base, wakeup_num);
                csi_irq_enable((uint32_t)WJ_WAKEUP_IRQn);
            }
        } else {
            csi_vic_clear_wakeup_irq(CONFIG_VIC_WAKEUP_BIT);

            if (wakeup_num < 32U) {
                wj_clear_lc_wakeup_source(pmu_base, wakeup_num);
				csi_irq_disable((uint32_t)WJ_WAKEUP_IRQn);
            }
        }

    }

    return ret;
}
#endif

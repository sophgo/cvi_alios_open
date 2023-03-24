/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     wj_pmu_cetus.c
 * @brief    CSI Source File for PMU Driver
 * @version  V1.0
 * @date     19. Feb 2020
 ******************************************************************************/
#include <csi_config.h>
#include <stdio.h>
#include <string.h>
#ifdef CONFIG_TEE_CA
#include <drv_tee.h>
#endif
#ifdef CONFIG_PM
#include <drv/io.h>
#include <drv/pm.h>
#include <soc.h>
#include <wj_pmu_cetus.h>


#define CONFIG_VIC_WAKEUP_BIT       31U
#define CSKY_SOC_WAKEUP_REG0        (WJ_IOC_BASE + 0x18)
#define CSKY_SOC_WAKEUP_LEVEL       (WJ_IOC_BASE + 0x78)

#define WAKEUP_IO       PA4
#ifdef CONFIG_CHIP_CETUS
#undef CONFIG_TEE_CA
#endif

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

#ifndef CONFIG_TEE_CA
#define CONFIG_PMU_REGISTER_NUM_SAVE  19U
static uint32_t pmu_regs_saved[CONFIG_PMU_REGISTER_NUM_SAVE];
#endif
#define CONFIG_CORETIM_REGISTER_NUM_SAVE    2U
static uint32_t cortim_regs_saved[CONFIG_CORETIM_REGISTER_NUM_SAVE];
#define CONFIG_VIC_REGISTER_NUM_SAVE    3U
static uint32_t vic_regs_saved[CONFIG_VIC_REGISTER_NUM_SAVE];

#define CONFIG_CACHE_REGISTER_NUM_SAVE    6U
static uint32_t cache_regs_saved[CONFIG_CACHE_REGISTER_NUM_SAVE];

#define CONFIG_CPU_REGISTER_NUM_SAVE    60U
uint32_t g_arch_cpu_saved[CONFIG_CPU_REGISTER_NUM_SAVE];

#define CONFIG_IOCTRL_REGISTER_NUM_SAVE 19U
uint32_t io_regs_saved[CONFIG_IOCTRL_REGISTER_NUM_SAVE];
/* Driver Capabilities */

static void do_prepare_sleep_action(void)
{
    uint8_t i;
#ifndef CONFIG_TEE_CA
    wj_pmu_reg_t *pbase = (wj_pmu_reg_t *)WJ_PMU_BASE;

    pmu_regs_saved[0] = pbase->DFCC;
    pmu_regs_saved[1] = pbase->PCR;
    pmu_regs_saved[2] = pbase->WIME0;
    pmu_regs_saved[3] = pbase->SRC_CLKMD;
    pmu_regs_saved[4] = pbase->PDU_DCU_MODE;
    pmu_regs_saved[5] = pbase->MCLK_RATIO;

#endif

    /* save cache register */
    volatile uint32_t *cbase = (uint32_t *)(CACHE_BASE);

    for (i = 0U; i < sizeof(cache_regs_saved) / 4U; i++) {
        cache_regs_saved[i] = *((volatile uint32_t *)cbase + i);
    }

    /* save vic register */
    volatile uint32_t *ibase = (uint32_t *)(DW_GPIO_BASE + 0x100U);
    vic_regs_saved[0] = VIC->ISER[0];
    vic_regs_saved[1] = VIC->ISPR[0];
    vic_regs_saved[2] = VIC->IPTR;

    /* save the coretim register */
    cortim_regs_saved[0] = CORET->LOAD;
    cortim_regs_saved[1] = CORET->CTRL;

    /* save the ioctrl register */
    for (i = 0U; i < sizeof(io_regs_saved) / 4U; i++) {
        if (i < 17U) {
            io_regs_saved[i] = *((volatile uint32_t *)ibase + i);
        } else if (i == 17U) {
            io_regs_saved[i] = *((volatile uint32_t *)(DW_GPIO_BASE + 0x8U));
        }
    }
}

static void do_wakeup_sleep_action(void)
{
    uint8_t i;
#ifndef CONFIG_TEE_CA
    wj_pmu_reg_t *pbase = (wj_pmu_reg_t *)WJ_PMU_BASE;

    pbase->MCLK_RATIO = pmu_regs_saved[5] & 0xfU;
    pbase->MCLK_RATIO = pmu_regs_saved[5];

    while (pbase->MCLK_RATIO & 0x10U);

    if (pbase->PDU_DCU_MODE & 0xd80U) {
        pbase->PDU_DCU_MODE = pmu_regs_saved[4];
        pbase->DFCC = 1U;

        while (pbase->DFCC & 0x1U);
    }

    pbase->PCR = pmu_regs_saved[1];
    pbase->WIME0 = pmu_regs_saved[2];
    pbase->SRC_CLKMD = pmu_regs_saved[3] & 0x3f00U;
    pbase->MODE_CHG_EN = 2U;

    while (pbase->MODE_CHG_EN);

    while ((pbase->CLKSTBR & 0x8U) != 0x8U);

    pbase->SRC_CLKMD = pmu_regs_saved[3] & 0xff00U;
    pbase->MODE_CHG_EN = 2U;

    while (pbase->MODE_CHG_EN);

    while ((pbase->CLKSTBR & 0x10U) != 0x10U);

    pbase->SRC_CLKMD = pmu_regs_saved[3];
    pbase->MODE_CHG_EN = 2U;

    while (pbase->MODE_CHG_EN);

#endif

    volatile uint32_t *cbase = (uint32_t *)(CACHE_BASE);

    for (i = 2U; i < sizeof(cache_regs_saved) / 4U; i++) {
        *((volatile uint32_t *)cbase + i) = cache_regs_saved[i];
    }

    *((volatile uint32_t *)cbase + 0U) = cache_regs_saved[0];
    *((volatile uint32_t *)cbase + 1U) = 1U;

    /* resume vic register */
    volatile uint32_t *ibase = (uint32_t *)(DW_GPIO_BASE + 0x100U);
    VIC->ISER[0U] = vic_regs_saved[0];
    VIC->ISPR[0U] = vic_regs_saved[1];
    VIC->IPTR = vic_regs_saved[2];

    /* resume the ioctrl register */
    for (i = 0U; i < sizeof(io_regs_saved) / 4U; i++) {
        if (i < 17U) {
            *((volatile uint32_t *)ibase + i) = io_regs_saved[i];
        } else if (i == 17U) {
            *((volatile uint32_t *)(DW_GPIO_BASE + 0x8)) = io_regs_saved[i];
        }
    }

    /* resume the coretim register */
    CORET->LOAD = cortim_regs_saved[0];
    CORET->CTRL = cortim_regs_saved[1];

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

#ifdef CONFIG_TEE_CA
void resume_context_from_stop_mode(void)
{
    arch_do_cpu_resume();
}
#endif

#ifndef CONFIG_TEE_TA
static void set_resume_func(wj_pmu_reg_t *pmu_base, uint32_t *func)
{
    pmu_base->USER_DEFINE[0] = (uint32_t)func;
}
#endif

__attribute__((section(".ram.code"))) void soc_sleep(lpm_mode_e mode)
{
#ifdef CONFIG_TEE_CA
    tee_lpm_mode_e lpm_mode = 0U;

    if (mode == WAIT_MODE) {
        lpm_mode = TEE_LPM_MODE_WAIT;
    } else if (mode == DOZE_MODE) {
        lpm_mode = TEE_LPM_MODE_DOZE;
    } else if (mode == STOP_MODE) {
        lpm_mode = TEE_LPM_MODE_STOP;
    } else if (mode == STANDBY_MODE) {
        lpm_mode = TEE_LPM_MODE_STANDBY;
    } else {
        lpm_mode = TEE_LPM_MODE_WAIT;
    }

    csi_tee_enter_lpm((uint32_t)g_arch_cpu_saved, 0U, lpm_mode);

    if (mode == STOP_MODE) {
        resume_context_from_stop_mode();
    }

#else
    wj_pmu_reg_t *pmu_base = (wj_pmu_reg_t *)WJ_PMU_BASE;

    set_resume_func(pmu_base, (uint32_t *)&arch_resume_context);

    VIC->ICPR[0] = 0xFFFFFFFFU; /* clear pend IRQ */

    if (mode == WAIT_MODE) {
        wj_pmu_lp_wait_clk_on_mode_enable(pmu_base);
        wj_pmu_allow_lp_mode(pmu_base);
        __WFI();
    } else if (mode == DOZE_MODE) {
        wj_pmu_lp_wait_clk_off_mode_enable(pmu_base);
        wj_pmu_allow_lp_mode(pmu_base);
        __WFI();
    } else if (mode == STOP_MODE) {
        wj_pmu_lp_stop_mode_enable(pmu_base);
        wj_pmu_allow_lp_mode(pmu_base);
        __STOP();
    } else if (mode == STANDBY_MODE) {
        wj_pmu_lp_standby_mode_enable(pmu_base);
        wj_pmu_allow_lp_mode(pmu_base);
        __STOP();
    } else {
        wj_pmu_lp_wait_clk_on_mode_enable(pmu_base);
        wj_pmu_allow_lp_mode(pmu_base);
        __WFI();
    }

#endif
}

/**
  \brief       choose the pmu mode to enter
  \param[in]   handle  pmu handle to operate.
  \param[in]   mode    \ref pmu_mode_e
  \return      error code
*/
csi_error_t soc_pm_enter_sleep(csi_pm_mode_t mode)
{
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

            if (arch_do_cpu_save() == 0U) {
                soc_sleep(STOP_MODE);
            }

            pmu_action(PM_DEV_RESUME);
            break;

        case PM_MODE_DEEP_SLEEP_2:
            pmu_action(PM_DEV_SUSPEND);
            soc_sleep(STANDBY_MODE);
            pmu_action(PM_DEV_RESUME);
            break;

        default:
            return CSI_ERROR;
    }

    return CSI_OK;
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

            if (wakeup_num == WJ_IOCTL_Wakeupn) {
                putreg32(1U << WAKEUP_IO, (uint32_t *)CSKY_SOC_WAKEUP_REG0);
                putreg32(0U, (uint32_t *)CSKY_SOC_WAKEUP_LEVEL);
            }

            if (wakeup_num < 32U) {
                wj_set_wakeup_source0(pmu_base, wakeup_num);
            } else {
                wj_set_wakeup_source1(pmu_base, (wakeup_num - 32U));
            }

        } else {
            csi_vic_clear_wakeup_irq(CONFIG_VIC_WAKEUP_BIT);

            if (wakeup_num < 32U) {
                wj_clear_wakeup_source0(pmu_base, wakeup_num);
            } else {
                wj_clear_wakeup_source0(pmu_base, (wakeup_num - 32U));
            }

        }

    }

    return ret;
}
#endif

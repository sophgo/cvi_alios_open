/*
 *	Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/*******************************************************
 * @file       pwmr.c
 * @brief      source file for pwmr csi driver
 * @version    V1.0
 * @date       23. Sep 2020
 * ******************************************************/

#include <drv/pwmr.h>
#include <drv/cvi_irq.h>
#include "wj_pwmr_ll.h"


extern const uint16_t pwmr_tx_hs_num[1];

static  void wj_pwmr_reset_register(wj_pwmr_regs_t *pwmr_base);

/**
  \brief       PWM interrupt handling function
  \param[in]   arg    Callback function member variables
  \return      None
*/
void wj_pwmr_irq_handler(void *arg)
{
    csi_pwmr_t *pwmr = (csi_pwmr_t *)arg;
    wj_pwmr_regs_t *pwmr_base = (wj_pwmr_regs_t *)HANDLE_REG_BASE(pwmr);
    uint32_t state;

    if (pwmr->callback) {
        state = wj_pwmr_get_int_status(pwmr_base);

        if (state & WJ_PWMR_IS_INT0M_CNTZERO_Msk) {
            pwmr->callback(pwmr,
                           PWMR_EVENT_COUNT_ZERO,
                           0,
                           pwmr->arg);
        }

        if (state & WJ_PWMR_IS_INT0M_CNTTOP_Msk) {
            pwmr->callback(pwmr,
                           PWMR_EVENT_COUNT_EQU_TOP,
                           0,
                           pwmr->arg);

        }

        if (state & WJ_PWMR_IS_INT0M_CMPU_Msk) {
            pwmr->callback(pwmr,
                           PWMR_EVENT_COUNT_EQU_COMP_UP,
                           0,
                           pwmr->arg);
        }

        if (state & WJ_PWMR_IS_INT0M_CMPD_Msk) {
            pwmr->callback(pwmr,
                           PWMR_EVENT_COUNT_EQU_COMP_DOWN,
                           0,
                           pwmr->arg);
        }

        if (state & WJ_PWMR_IS_INT0M_OVERTIME_Msk) {
            pwmr->callback(pwmr,
                           PWMR_EVENT_OVERTIME,
                           0,
                           pwmr->arg);
        }

        wj_pwmr_set_clear_int(pwmr_base, state);

    }

}

/**
  \brief       Initialize PWMR interface. Initializes the resources needed for the PWMR interface
  \param[in]   pwmr    Handle to operate
  \param[in]   idx    PWMR idx
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_pwmr_init(csi_pwmr_t *pwmr, uint32_t idx)
{
    CSI_PARAM_CHK(pwmr, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    wj_pwmr_regs_t *pwmr_base = NULL;

    if (0 == target_get(DEV_WJ_PWMR_TAG, idx, &pwmr->dev)) {
        pwmr_base = (wj_pwmr_regs_t *)HANDLE_REG_BASE(pwmr);
        wj_pwmr_reset_register(pwmr_base);
    } else {
        ret = CSI_ERROR;
    }

    return ret;
}

static  void wj_pwmr_reset_register(wj_pwmr_regs_t *pwmr_base)
{
    CSI_PARAM_CHK_NORETVAL(pwmr_base);

    wj_pwmr_dis(pwmr_base);
    wj_pwmr_en_int0_cntzero_mask(pwmr_base);
    wj_pwmr_en_int0_cnttop_mask(pwmr_base);
    wj_pwmr_en_int0_cnt_cnt_equ_comp_up_mask(pwmr_base);
    wj_pwmr_en_int0_cnt_cnt_equ_comp_down_mask(pwmr_base);
    wj_pwmr_en_int1_overtime_mask(pwmr_base);
    wj_pwmr_en_int1_cntzero_mask(pwmr_base);
    wj_pwmr_en_int1_cnttop_mask(pwmr_base);
    wj_pwmr_en_int1_cnt_cnt_equ_comp_up_mask(pwmr_base);
    wj_pwmr_en_int1_cnt_cnt_equ_comp_down_mask(pwmr_base);
    wj_pwmr_en_int1_overtime_mask(pwmr_base);
    wj_pwmr_clear_value0_buf(pwmr_base);
    wj_pwmr_clear_value0_buf(pwmr_base);
    wj_pwmr_dis_cnt0_trig(pwmr_base);
    wj_pwmr_dis_cnt1_trig(pwmr_base);
    wj_pwmr_set_clear_int(pwmr_base, 0xffff);
}

/**
  \brief       De-initialize PWMR interface. Stops operation and releases the software resources used by the interface
  \param[in]   pwmr    Handle to operate
  \return      None
*/
void csi_pwmr_uninit(csi_pwmr_t *pwmr)
{
    CSI_PARAM_CHK_NORETVAL(pwmr);
    wj_pwmr_regs_t *pwmr_base = (wj_pwmr_regs_t *)HANDLE_REG_BASE(pwmr);
    wj_pwmr_reset_register(pwmr_base);
}

/**
  \brief       Config PWMR out mode
  \param[in]   pwmr              Handle to operate
  \param[in]   channel           Channel num
  \param[in]   duty              The PWMR high duty cycle.0~1000 means 0~100%.
  \param[in]   frequency         The PWMR frequency.Min value is apb0/4,max vlaue is apb0/8000000
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_pwmr_out_config(csi_pwmr_t *pwmr,
                                uint32_t channel,
                                uint32_t duty,
                                uint32_t frequency)
{
    CSI_PARAM_CHK(pwmr, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    wj_pwmr_regs_t *pwmr_base ;
    uint32_t tmp ;
    uint32_t i, div, top_value, cmp_value;
    tmp = soc_get_apb_freq(0) / frequency;

    if ((duty >= 1000) || (tmp < 8) || (tmp > 8000000)) {
        ret = CSI_ERROR;
    }

    if (ret == CSI_OK) {
        pwmr_base = (wj_pwmr_regs_t *)HANDLE_REG_BASE(pwmr);

        for (i = 0; i < 7; i++) {
            if (tmp / (1 << i) < 0xffff) {
                div = i;
                top_value = soc_get_apb_freq(0) / ((1 << i) * frequency);
                break;
            }
        }

        if (i == 7) {    ///< It shouldn't be necessary.
            ret = CSI_ERROR;
        }

        if (ret == CSI_OK) {
            switch (div) {
                case 0:
                    div = 0xff ;
                    break;

                case 1:
                    div = WJ_PWMR_CONFIG_DIV_MODE_2 ;
                    break;

                case 2:
                    div = WJ_PWMR_CONFIG_DIV_MODE_4 ;
                    break;

                case 3:
                    div = WJ_PWMR_CONFIG_DIV_MODE_8;
                    break;

                case 4:
                    div = WJ_PWMR_CONFIG_DIV_MODE_16;
                    break;

                case 5:
                    div = WJ_PWMR_CONFIG_DIV_MODE_32;
                    break;

                case 6:
                    div = WJ_PWMR_CONFIG_DIV_MODE_64;
                    break;

                case 7:
                    div = WJ_PWMR_CONFIG_DIV_MODE_128;
                    break;

                default:
                    ret = CSI_ERROR;
                    break;
            }

            cmp_value = top_value * (1000 - duty) / 1000;

            if (div == 0xff) {
                wj_pwmr_dis_div_en(pwmr_base);
            } else {
                wj_pwmr_en_div_en(pwmr_base);
            }

            wj_pwmr_set_div(pwmr_base, div);
            wj_pwmr_set_gen0_top_value(pwmr_base, top_value);
            wj_pwmr_set_gen0_compare_value(pwmr_base, cmp_value);
            wj_pwmr_en_cnt0_up_mode(pwmr_base);
            wj_pwmr_en_cnt1_invert(pwmr_base);
            wj_pwmr_dis_cnt0_invert(pwmr_base);
        }
    }

    return ret;
}

/**
  \brief       Start generate PWMR signal
  \param[in]   pwmr       Handle to operate
  \param[in]   channel    Channel num
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_pwmr_out_start(csi_pwmr_t *pwmr, uint32_t channel)
{
    CSI_PARAM_CHK(pwmr, CSI_ERROR);
    wj_pwmr_regs_t *pwmr_base = (wj_pwmr_regs_t *)HANDLE_REG_BASE(pwmr);
    wj_pwmr_en(pwmr_base);
    return CSI_OK;
}

/**
  \brief       Stop generate PWMR signal
  \param[in]   pwmr       Handle to operate
  \param[in]   channel    Channel num
  \return      None
*/
void csi_pwmr_out_stop(csi_pwmr_t *pwmr, uint32_t channel)
{
    CSI_PARAM_CHK_NORETVAL(pwmr);
    wj_pwmr_regs_t *pwmr_base = (wj_pwmr_regs_t *)HANDLE_REG_BASE(pwmr);
    wj_pwmr_dis(pwmr_base);
}

/**
  \brief       Attach PWMR callback
  \param[in]   pwmr         Handle to operate
  \param[in]   callback    Callback func
  \param[in]   arg         Callback's param
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_pwmr_attach_callback(csi_pwmr_t *pwmr, void *callback, void *arg)
{
    CSI_PARAM_CHK(pwmr, CSI_ERROR);
    wj_pwmr_regs_t *pwmr_base = (wj_pwmr_regs_t *)HANDLE_REG_BASE(pwmr);
    pwmr->callback = callback;
    pwmr->arg = arg;
    csi_irq_attach((uint32_t)pwmr->dev.irq_num, &wj_pwmr_irq_handler, &pwmr->dev);
    csi_irq_enable((uint32_t)pwmr->dev.irq_num);
    wj_pwmr_dis_int0_cntzero_mask(pwmr_base);
    wj_pwmr_dis_int0_cnttop_mask(pwmr_base);
    wj_pwmr_dis_int0_cnt_cnt_equ_comp_up_mask(pwmr_base);
    wj_pwmr_dis_int0_cnt_cnt_equ_comp_down_mask(pwmr_base);
    wj_pwmr_dis_int0_overtime_mask(pwmr_base);

    return CSI_OK;
}

/**
  \brief       Detach PWMR callback
  \param[in]   pwmr    Handle to operate
  \return      None
*/
void csi_pwmr_detach_callback(csi_pwmr_t *pwmr)
{
    CSI_PARAM_CHK_NORETVAL(pwmr);
    wj_pwmr_regs_t *pwmr_base = (wj_pwmr_regs_t *)HANDLE_REG_BASE(pwmr);
    pwmr->callback = NULL;
    pwmr->arg = NULL;
    wj_pwmr_en_int0_cntzero_mask(pwmr_base);
    wj_pwmr_en_int0_cnttop_mask(pwmr_base);
    wj_pwmr_en_int0_cnt_cnt_equ_comp_up_mask(pwmr_base);
    wj_pwmr_en_int0_cnt_cnt_equ_comp_down_mask(pwmr_base);
    wj_pwmr_en_int0_overtime_mask(pwmr_base);
    csi_irq_disable((uint32_t)pwmr->dev.irq_num);
    csi_irq_detach((uint32_t)pwmr->dev.irq_num);
}

#ifdef CONFIG_PM
csi_error_t wj_pwmr_pm_action(csi_dev_t *dev, csi_pm_dev_action_t action)
{
    CSI_PARAM_CHK(dev, CSI_ERROR);

    csi_error_t ret = CSI_OK;
    csi_pm_dev_t *pm_dev = &dev->pm_dev;

    switch (action) {
        case PM_DEV_SUSPEND:
            csi_pm_dev_save_regs(pm_dev->reten_mem, (uint32_t *)dev->reg_base, 5U);
            csi_pm_dev_save_regs(pm_dev->reten_mem + 5U, (uint32_t *)(dev->reg_base + 28U), 1U);
            break;

        case PM_DEV_RESUME:
            csi_pm_dev_restore_regs(pm_dev->reten_mem, (uint32_t *)dev->reg_base, 5U);
            csi_pm_dev_restore_regs(pm_dev->reten_mem + 5U, (uint32_t *)(dev->reg_base + 28U), 1U);
            break;

        default:
            ret = CSI_ERROR;
            break;
    }

    return ret;
}

csi_error_t csi_pwmr_enable_pm(csi_pwmr_t *pwmr)
{
    return csi_pm_dev_register(&pwmr->dev, wj_pwmr_pm_action, 144U, 0U);
}

void csi_pwmr_dis_pm(csi_pwmr_t *pwmr)
{
    csi_pm_dev_unregister(&pwmr->dev);
}
#endif

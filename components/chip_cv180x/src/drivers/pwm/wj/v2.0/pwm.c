/*
 *	Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/*******************************************************
 * @file       pwm.c
 * @brief      source file for pwm csi driver
 * @version    V2.0
 * @date       23. Sep 2020
 * ******************************************************/

#include <drv/pwm.h>
#include <drv/cvi_irq.h>
#include "wj_pwm_ll.h"

/**
 * Calculating trigger period of acquisition signal based on two time stamps
*/
#define WJ_PWM_CAL_ELAPSED_TIME_US(_CUR_, _PER_)                ((_CUR_ > _PER_) ? (_CUR_ - _PER_) : (((0xFFFFFFFFU - _PER_) + _CUR_) + 1U))

/**
 * Return interrupt trigger type according to user configuration
*/
#define WJ_PWM_GET_EVENT_STATE(_STA_)                           ((_STA_ == 0U) ? PWM_EVENT_CAPTURE_POSEDGE  :  \
        (_STA_ == 1U) ? PWM_EVENT_CAPTURE_NEGEDGE  :  \
        (_STA_ == 3U) ? PWM_EVENT_CAPTURE_BOTHEDGE :  \
        PWM_EVENT_ERROR)

#define WJ_PWM_CHECK_CHANNEL_NUM(_CH_)                          ((_CH_ >= PWM_CHANNEL_CH_MAX) ? CSI_ERROR : CSI_OK)

/**
 * Return current division factor
*/
#define WJ_PWM_GET_DIV_FRQ_VALUE(_ADDR_)                        ((_ADDR_) & 0x08000000U ? (2 << (((_ADDR_) & 0x07000000U) >> 24U)) : (1U))

volatile uint32_t g_pwm_totaltime[PWM_CHANNEL_CH_MAX];          ///< Cache cumulative signal cycle time
volatile uint32_t g_pwm_timestamp[PWM_CHANNEL_CH_MAX];          ///< Cache previous timestamp
volatile uint32_t g_pwm_cur_count[PWM_CHANNEL_CH_MAX];          ///< Cache current interrupt triggers
/**
  \brief       PWM interrupt handling function
  \param[in]   arg    Callback function member variables
  \return      None
*/
void wj_pwm_irq_handler(void *arg)
{
    csi_pwm_t *pwm = (csi_pwm_t *)arg;
    wj_pwm_regs_t *pwm_base = (wj_pwm_regs_t *)HANDLE_REG_BASE(pwm);
    uint32_t ch;

    for (ch = PWM_CHANNEL_CH_0; ch < PWM_CHANNEL_CH_MAX; ch++) {

        if (wj_pwm_capint_is_time_ch_get_sta(pwm_base, ch)) {
            volatile uint32_t timeout = 0U;

            /**
             * Timeout mechanism in interrupt processing function,
             * which performs cyclic read state clearing operation due to hardware defects
            */
            while (wj_pwm_capint_is_time_ch_get_sta(pwm_base, ch)) {
                wj_pwm_capint_ic_time_ch_en(pwm_base, ch);

                if (timeout++ > 10000000U) {
                    break;
                }
            }

            volatile uint32_t temp = wj_pwm_capture_read_count_ch(pwm_base, ch);
            g_pwm_totaltime[ch] += WJ_PWM_CAL_ELAPSED_TIME_US(temp, g_pwm_timestamp[ch]);
            g_pwm_timestamp[ch] = temp;
            g_pwm_cur_count[ch]++;

            if (g_pwm_cur_count[ch] >= wj_pwm_capture_read_match_ch(pwm_base, ch)) {
                if (pwm->callback) {
                    g_pwm_totaltime[ch] /= (soc_get_pwm_freq(pwm->dev.idx) / (WJ_PWM_GET_DIV_FRQ_VALUE(*(uint32_t *)pwm_base)) / 1000000U);
                    pwm->callback(pwm,
                                  WJ_PWM_GET_EVENT_STATE(wj_pwm_capctl_event_ch_get_sta(pwm_base, ch)),
                                  ch,
                                  g_pwm_totaltime[ch],
                                  pwm->arg);
                }

                g_pwm_totaltime[ch] = 0U;
                g_pwm_cur_count[ch] = 0U;
            }
        }
    }
}

/**
  \brief       Initialize PWM Interface. 1. Initializes the resources needed for the PWM interface
  \param[in]   pwm    pwm handle to operate
  \param[in]   idx    pwm idx
  \return      error code
*/
csi_error_t csi_pwm_init(csi_pwm_t *pwm, uint32_t idx)
{
    CSI_PARAM_CHK(pwm, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    wj_pwm_regs_t *pwm_base = NULL;

    if (0 == target_get(DEV_WJ_PWM_TAG, idx, &pwm->dev)) {
        pwm_base = (wj_pwm_regs_t *)HANDLE_REG_BASE(pwm);
        wj_pwm_reset_register(pwm_base);
    } else {
        ret = CSI_ERROR;
    }

    return ret;
}

/**
  \brief       De-initialize PWM Interface. stops operation and releases the software resources used by the interface
  \param[in]   pwm    pwm handle to operate
  \return      None
*/
void csi_pwm_uninit(csi_pwm_t *pwm)
{
    CSI_PARAM_CHK_NORETVAL(pwm);
    wj_pwm_regs_t *pwm_base = (wj_pwm_regs_t *)HANDLE_REG_BASE(pwm);
    wj_pwm_reset_register(pwm_base);
}

/**
  \brief       Config pwm out mode
  \param[in]   pwm               pwm handle to operate
  \param[in]   channel           channel num
  \param[in]   period_us         the PWM period in us
  \param[in]   pulse_width_us    the PMW pulse width in us
  \param[in]   polarity          the PWM polarity \ref csi_pwm_polarity_t
  \return      error code
*/
csi_error_t csi_pwm_out_config(csi_pwm_t *pwm,
                               uint32_t  channel,
                               uint32_t period_us,
                               uint32_t pulse_width_us,
                               csi_pwm_polarity_t polarity)
{
    CSI_PARAM_CHK(pwm, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    wj_pwm_regs_t *pwm_base = (wj_pwm_regs_t *)HANDLE_REG_BASE(pwm);

    uint8_t count_div;
    uint8_t cnt_div_frq[8] = {1U, 2U, 4U, 8U, 16U, 32U, 64U, 128U};
    uint8_t cnt_div_para[8] = {0U, 0U, 1U, 2U, 3U, 4U, 5U, 6U};
    uint32_t count_unit = soc_get_pwm_freq((uint32_t)pwm->dev.idx) / 1000000U;
    uint32_t count_tmp = count_unit * period_us;
    uint32_t counter;
    uint32_t width_tmp = 0U;            ///< default low level width

    ret = WJ_PWM_CHECK_CHANNEL_NUM(channel);

    if (period_us < pulse_width_us) {
        ret = CSI_ERROR;
    }

    switch (polarity) {
        case PWM_POLARITY_LOW:
            width_tmp = pulse_width_us;
            break;

        case PWM_POLARITY_HIGH:
            width_tmp = period_us - pulse_width_us;
            break;

        default:
            ret = CSI_ERROR;
            break;
    }

    if (CSI_OK == ret) {
        /**
         * Select an internal frequency division according to the current clock frequency
         * and signal period, and control the internal loading value within 16 bit width
        */
        for (count_div = 0U; count_div < (sizeof(cnt_div_frq) / sizeof(uint8_t)); count_div++) {

            counter = (count_tmp  / cnt_div_frq[count_div]);

            if ((counter <= 0xffffU)) {
                break;
            }
        }

        /**
         * When the load value calculated according to the current clock frequency and
         * signal period cannot meet the 16 bit width, an abnormal error is reported
        */
        if (count_div < (sizeof(cnt_div_frq) / sizeof(uint8_t))) {
            uint32_t cmp_counter = ((count_unit * width_tmp) / cnt_div_frq[count_div]);

            if (0U == count_div) {
                wj_pwm_config_frq_div_dis(pwm_base);
            } else {
                wj_pwm_config_frq_div_en(pwm_base);
                wj_pwm_config_frq_div_select(pwm_base, (uint32_t)cnt_div_para[count_div]);
            }

            wj_pwm_ctl_ch_up_mode(pwm_base, channel);
            wj_pwm_counter_write_load_ch(pwm_base, channel, counter);

            wj_pwm_counter_write_compare_a_ch(pwm_base, channel, cmp_counter);
            wj_pwm_counter_write_compare_b_ch(pwm_base, channel, cmp_counter);
        } else {
            ret = CSI_ERROR;
        }

    }

    return ret;
}

/**
  \brief       Start generate pwm signal
  \param[in]   pwm        pwm handle to operate
  \param[in]   channel    channel num
  \return      error code
*/
csi_error_t csi_pwm_out_start(csi_pwm_t *pwm, uint32_t channel)
{
    CSI_PARAM_CHK(pwm, CSI_ERROR);
    wj_pwm_regs_t *pwm_base = (wj_pwm_regs_t *)HANDLE_REG_BASE(pwm);

    wj_pwm_config_output_en(pwm_base, channel);

    return CSI_OK;
}

/**
  \brief       Stop generate pwm signal
  \param[in]   pwm        pwm handle to operate
  \param[in]   channel    channel num
  \return      None
*/
void csi_pwm_out_stop(csi_pwm_t *pwm, uint32_t channel)
{
    CSI_PARAM_CHK_NORETVAL(pwm);
    wj_pwm_regs_t *pwm_base = (wj_pwm_regs_t *)HANDLE_REG_BASE(pwm);

    wj_pwm_config_output_dis(pwm_base, channel);
}

/**
  \brief       Config pwm capture mode
  \param[in]   pwm         pwm handle to operate
  \param[in]   channel     channel num
  \param[in]   polarity    pwm capture polarity \ref csi_pwm_capture_polarity_t
  \param[in]   count       pwm capture polarity count
  \return      error code \ref csi_error_t
*/
csi_error_t csi_pwm_capture_config(csi_pwm_t *pwm,
                                   uint32_t channel,
                                   csi_pwm_capture_polarity_t polarity,
                                   uint32_t count)
{
    CSI_PARAM_CHK(pwm, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    wj_pwm_regs_t *pwm_base = (wj_pwm_regs_t *)HANDLE_REG_BASE(pwm);

    ret = WJ_PWM_CHECK_CHANNEL_NUM(channel);

    if (CSI_OK == ret) {
        switch (polarity) {
            case PWM_CAPTURE_POLARITY_POSEDGE:
                wj_pwm_capctl_event_ch_pose(pwm_base, channel);
                break;

            case PWM_CAPTURE_POLARITY_NEGEDGE:
                wj_pwm_capctl_event_ch_nege(pwm_base, channel);
                break;

            case PWM_CAPTURE_POLARITY_BOTHEDGE:
                wj_pwm_capctl_event_ch_both(pwm_base, channel);
                break;

            default:
                ret = CSI_ERROR;
                break;
        }

        g_pwm_cur_count[channel] = 0U;

        wj_pwm_capture_write_match_ch(pwm_base, channel, count);    ///< Configure the number of triggers

        wj_pwm_capctl_mode_ch_time(pwm_base, channel);               ///< Configure time mode

        wj_pwm_capint_ie_time_ch_en(pwm_base, channel);              ///< Enable time interrupt
    }

    return ret;
}

/**
  \brief       Start pwm capture
  \param[in]   pwm        pwm handle to operate
  \param[in]   channel    channel num
  \return      error code \ref csi_error_t
*/
csi_error_t csi_pwm_capture_start(csi_pwm_t *pwm, uint32_t channel)
{
    CSI_PARAM_CHK(pwm, CSI_ERROR);
    wj_pwm_regs_t *pwm_base = (wj_pwm_regs_t *)HANDLE_REG_BASE(pwm);

    wj_pwm_config_output_ch_dis(pwm_base, channel);             ///< Turn off the normal PWM output

    wj_pwm_config_input_capture_ch_en(pwm_base, channel);       ///< Turn on the capture PWM input

    g_pwm_timestamp[channel] = wj_pwm_capture_read_count_ch(pwm_base, channel);
    g_pwm_totaltime[channel] = 0U;

    return CSI_OK;
}

/**
  \brief       Stop pwm capture
  \param[in]   pwm        pwm handle to operate
  \param[in]   channel    channel num
  \return      None
*/
void csi_pwm_capture_stop(csi_pwm_t *pwm, uint32_t channel)
{
    CSI_PARAM_CHK_NORETVAL(pwm);
    wj_pwm_regs_t *pwm_base = (wj_pwm_regs_t *)HANDLE_REG_BASE(pwm);

    wj_pwm_config_input_capture_ch_dis(pwm_base, channel);      ///< Turn off the capture PWM input

    wj_pwm_capint_ie_cnt_ch_dis(pwm_base, channel);             ///< Turn off count interrupt

    g_pwm_timestamp[channel] = 0U;
}
/**
  \brief       Attach pwm callback
  \param[in]   pwm         pwm handle to operate
  \param[in]   callback    callback func
  \param[in]   arg         user param passed to callback
  \return      error code \ref csi_error_t
*/
csi_error_t csi_pwm_attach_callback(csi_pwm_t *pwm, void *callback, void *arg)
{
    CSI_PARAM_CHK(pwm, CSI_ERROR);

    pwm->callback = callback;
    pwm->arg = arg;
    csi_irq_attach((uint32_t)pwm->dev.irq_num, &wj_pwm_irq_handler, &pwm->dev);
    csi_irq_enable((uint32_t)pwm->dev.irq_num);

    return CSI_OK;
}

/**
  \brief       Detach pwm callback
  \param[in]   pwm    pwm handle to operate
  \return      None
*/
void csi_pwm_detach_callback(csi_pwm_t *pwm)
{
    CSI_PARAM_CHK_NORETVAL(pwm);

    pwm->callback = NULL;
    pwm->arg = NULL;
    csi_irq_disable((uint32_t)pwm->dev.irq_num);
    csi_irq_detach((uint32_t)pwm->dev.irq_num);
}

#ifdef CONFIG_PM
csi_error_t wj_pwm_pm_action(csi_dev_t *dev, csi_pm_dev_action_t action)
{
    CSI_PARAM_CHK(dev, CSI_ERROR);

    csi_error_t ret = CSI_OK;
    csi_pm_dev_t *pm_dev = &dev->pm_dev;

    switch (action) {
        case PM_DEV_SUSPEND:
            csi_pm_dev_save_regs(pm_dev->reten_mem, (uint32_t *)dev->reg_base, 7U);
            csi_pm_dev_save_regs(pm_dev->reten_mem[7], (uint32_t *)(dev->reg_base + 36U), 2U);
            csi_pm_dev_save_regs(pm_dev->reten_mem[9], (uint32_t *)(dev->reg_base + 52U), 4U);
            csi_pm_dev_save_regs(pm_dev->reten_mem[13], (uint32_t *)(dev->reg_base + 80U), 11U);
            csi_pm_dev_save_regs(pm_dev->reten_mem[24], (uint32_t *)(dev->reg_base + 128U), 1U);
            csi_pm_dev_save_regs(pm_dev->reten_mem[25], (uint32_t *)(dev->reg_base + 148U), 4U);
            csi_pm_dev_save_regs(pm_dev->reten_mem[29], (uint32_t *)(dev->reg_base + 168U), 1U);
            csi_pm_dev_save_regs(pm_dev->reten_mem[30], (uint32_t *)(dev->reg_base + 176U), 6U);
            csi_pm_dev_save_regs(pm_dev->reten_mem[36], (uint32_t *)(dev->reg_base + 212U), 18U);
            break;

        case PM_DEV_RESUME:
            csi_pm_dev_restore_regs(pm_dev->reten_mem, (uint32_t *)dev->reg_base, 7U);
            csi_pm_dev_restore_regs(pm_dev->reten_mem[7], (uint32_t *)(dev->reg_base + 36U), 2U);
            csi_pm_dev_restore_regs(pm_dev->reten_mem[9], (uint32_t *)(dev->reg_base + 52U), 4U);
            csi_pm_dev_restore_regs(pm_dev->reten_mem[13], (uint32_t *)(dev->reg_base + 80U), 11U);
            csi_pm_dev_restore_regs(pm_dev->reten_mem[24], (uint32_t *)(dev->reg_base + 128U), 1U);
            csi_pm_dev_restore_regs(pm_dev->reten_mem[25], (uint32_t *)(dev->reg_base + 148U), 4U);
            csi_pm_dev_restore_regs(pm_dev->reten_mem[29], (uint32_t *)(dev->reg_base + 168U), 1U);
            csi_pm_dev_restore_regs(pm_dev->reten_mem[30], (uint32_t *)(dev->reg_base + 176U), 6U);
            csi_pm_dev_restore_regs(pm_dev->reten_mem[36], (uint32_t *)(dev->reg_base + 212U), 18U);
            break;

        default:
            ret = CSI_ERROR;
            break;
    }

    return ret;
}

csi_error_t csi_pwm_enable_pm(csi_pwm_t *pwm)
{
    return csi_pm_dev_register(&pwm->dev, wj_pwm_pm_action, 220U, 0U);
}

void csi_pwm_disable_pm(csi_pwm_t *pwm)
{
    csi_pm_dev_unregister(&pwm->dev);
}
#endif

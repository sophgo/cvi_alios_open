/*
* Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
*/

#include <drv/common.h>
#include <drv/pwm.h>
#include <drv/cvi_irq.h>
#include <drv/pin.h>
#include <drv/cvi_irq.h>
#include "cvi_pwm.h"

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

    if (target_get(DEV_CVI_PWM_TAG, idx, &pwm->dev) != CSI_OK) {
        pr_err("pwm %d init failed!\n", idx);
        ret = CSI_ERROR;
    }

    pr_debug("pwm %d reg_base 0x%lx\n", pwm->dev.idx, pwm->dev.reg_base);

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
    // unsigned long reg_base = HANDLE_REG_BASE(pwm);
    // cvi_pwm_reset_register(reg_base);
}

/**
  \brief       De-initialize PWM Interface. stops operation and releases the software resources used by the interface
  \param[in]   pwm    pwm handle to operate
  \return      None
*/
csi_error_t csi_pwm_out_config(csi_pwm_t *pwm,
                               uint32_t  channel,
                               uint32_t period_ns,
                               uint32_t pulse_width_ns,
                               csi_pwm_polarity_t polarity)
{
    CSI_PARAM_CHK(pwm, CSI_ERROR);

    csi_error_t ret = CSI_OK;
    unsigned long reg_base = HANDLE_REG_BASE(pwm);

    // uint32_t count_unit = soc_get_pwm_freq((uint32_t)pwm->dev.idx) / 1000000U;
    const uint64_t count_unit = 100000000;  // 100M count per second
    const uint64_t NSEC_COUNT = 1000000000;  // ns
    // const uint32_t count_tmp = count_unit * period_ns;
    unsigned long long duty_clk, period_clk;

    ret = CVI_PWM_CHECK_CHANNEL_NUM(channel);

    if (period_ns < pulse_width_ns) {
        pr_err("pulse_width_ns %d large than period_ns %d\n", pulse_width_ns, period_ns);
        ret = CSI_ERROR;
    }

    pr_debug("========period_ns: %u, duty_ns:%u=======\n", period_ns, pulse_width_ns);
    period_clk = (period_ns * count_unit) / NSEC_COUNT;   

    switch (polarity) {
        case PWM_POLARITY_LOW:
            duty_clk = pulse_width_ns;
            break;

        case PWM_POLARITY_HIGH:
            duty_clk = period_ns - pulse_width_ns;
            break;

        default:
            ret = CSI_ERROR;
            break;
    }

    if (CSI_OK == ret) {
        cvi_pwm_set_polarity_low_ch(reg_base, (channel & 0x3));

        duty_clk = (duty_clk * count_unit) / NSEC_COUNT;
        pr_debug("========period_clk: %llu, duty_clk:%llu=======\n", period_clk, duty_clk);
        cvi_pwm_set_high_period_ch(reg_base, (channel & 0x3), duty_clk);
        cvi_pwm_set_period_ch(reg_base, (channel & 0x3), period_clk);
        cvi_pwm_output_en_ch(reg_base, (channel & 0x3));
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
    unsigned long reg_base = HANDLE_REG_BASE(pwm);

    cvi_pwm_start_en_ch(reg_base, (channel & 0x3));

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
    unsigned long reg_base = HANDLE_REG_BASE(pwm);

    cvi_pwm_start_dis_ch(reg_base, (channel & 0x3));
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
    unsigned long reg_base = HANDLE_REG_BASE(pwm);

    ret = CVI_PWM_CHECK_CHANNEL_NUM(channel);

    if (CSI_OK == ret) {
        switch (polarity) {
            case PWM_CAPTURE_POLARITY_POSEDGE:  // input capture only set to pose edge
                break;

            case PWM_CAPTURE_POLARITY_NEGEDGE:
            case PWM_CAPTURE_POLARITY_BOTHEDGE:
                ret = CSI_UNSUPPORTED;
                break;

            default:
                ret = CSI_ERROR;
                break;
        }

         cvi_pwm_input_en_ch(reg_base, (channel & 0x3));              ///< Enable time interrupt
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
    unsigned long reg_base = HANDLE_REG_BASE(pwm);

    // g_pwm_timestamp[channel] = wj_pwm_capture_read_count_ch(pwm_base, channel);
    // g_pwm_totaltime[channel] = 0U;
    cvi_cap_set_freqnum_ch(reg_base, (channel & 0x3), 1);
    cvi_cap_freq_en_ch(reg_base, (channel & 0x3));

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
    unsigned long reg_base = HANDLE_REG_BASE(pwm);

    cvi_cap_freq_dis_ch(reg_base, (channel & 0x3));

    // g_pwm_timestamp[channel] = 0U;
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

    return CSI_UNSUPPORTED;
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

    return;
}

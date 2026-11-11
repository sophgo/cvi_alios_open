/*
 *    Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/*******************************************************
 * @file       rtc.c
 * @brief      source file for rtc csi driver
 * @version    V1.0
 * @date       23. Sep 2020
 * ******************************************************/

#include <drv/rtc.h>
#include <drv/cvi_irq.h>
#include "rtc_alg.h"
#include "qx_rtc_ll.h"

#define NORMAL_RTC_FREQ         (32768U)
#define REAL_RTC_FREQ           (soc_get_rtc_freq(0U))

static uint32_t load_time = 0U;
static uint32_t int_number = 0U;
static uint32_t count = 0U;
/**
  \brief       Internal timeout interrupt process function
  \param[in]   rtc    handle rtc handle to operate
  \return      None
*/
void qx_rtc_irq_handler(void *arg)
{
    csi_rtc_t *rtc = (csi_rtc_t *)arg;
    uint32_t timecount,ret;
    qx_rtc_regs_t *rtc_base = (qx_rtc_regs_t *)HANDLE_REG_BASE(rtc);
    timecount = 0U;
    ret =0;

    do {
        qx_rtc_clear_intr_sta(rtc_base);
        QX_RTC_WAIT_IS_TIMEOUT(++timecount, ret);

        if (ret < 0) {
            break;
        }
    } while (qx_rtc_get_intr_sta(rtc_base));

    count++;

    if (count >= (int_number * (REAL_RTC_FREQ / NORMAL_RTC_FREQ))) {
        count = 0U;

        if (rtc->callback) {
            rtc->callback(rtc, rtc->arg);
        }
    }

}
/**
  \brief       Initialize RTC Interface. Initializes the resources needed for the RTC interface
  \param[in]   rtc    rtc handle to operate
  \param[in]   idx    rtc index
  \return      error code \ref csi_error_t
*/
csi_error_t csi_rtc_init(csi_rtc_t *rtc, uint32_t idx)
{
    CSI_PARAM_CHK(rtc, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    qx_rtc_regs_t *rtc_base;

    if (0 == target_get(DEV_QX_RTC_TAG, idx, &rtc->dev)) {
        rtc_base = (qx_rtc_regs_t *)HANDLE_REG_BASE(rtc);
        qx_rtc_reset_register(rtc_base);
    } else {
        ret = CSI_ERROR;
    }

    return ret;
}
/**
  \brief       De-initialize RTC Interface. stops operation and releases the software resources used by the interface
  \param[in]   rtc    rtc handle to operate
  \return      None
*/
void csi_rtc_uninit(csi_rtc_t *rtc)
{
    CSI_PARAM_CHK_NORETVAL(rtc);
    qx_rtc_regs_t *rtc_base = (qx_rtc_regs_t *)HANDLE_REG_BASE(rtc);
    qx_rtc_reset_register(rtc_base);
}

/**
  \brief       Set system date
  \param[in]   rtc        handle rtc handle to operate
  \param[in]   rtctime    pointer to rtc time
  \return      error code \ref csi_error_t
*/
csi_error_t csi_rtc_set_time(csi_rtc_t *rtc, const csi_rtc_time_t *rtctime)
{
    CSI_PARAM_CHK(rtc, CSI_ERROR);
    CSI_PARAM_CHK(rtctime, CSI_ERROR);
    qx_rtc_regs_t *rtc_base = (qx_rtc_regs_t *)HANDLE_REG_BASE(rtc);
    csi_error_t ret = CSI_OK;
    int32_t load_sec;

    do {
        ret = (csi_error_t)clock_check_tm_ok((const struct tm *)rtctime);

        if (ret < CSI_OK) {
            break;
        }

        load_sec = clock_update_basetime((const struct tm *)rtctime);

        if (load_sec <= 0) {        ///< Error returned when total seconds overflow is negative
            ret = CSI_ERROR;
            break;
        }

        load_time = (uint32_t)load_sec;
        ret = (csi_error_t)qx_rtc_write_loadtime(rtc_base, (uint32_t)load_sec);

        if (ret < CSI_OK) {
            break;
        }

        qx_rtc_counter_enable(rtc_base);

    } while (0);

    return ret;
}
/**
  \brief       Set system date but no wait
  \param[in]   rtc        rtc handle to operate
  \param[in]   rtctime    pointer to rtc time
  \return      error code \ref csi_error_t
*/
csi_error_t csi_rtc_set_time_no_wait(csi_rtc_t *rtc, const csi_rtc_time_t *rtctime)
{
    CSI_PARAM_CHK(rtc, CSI_ERROR);
    CSI_PARAM_CHK(rtctime, CSI_ERROR);
    qx_rtc_regs_t *rtc_base = (qx_rtc_regs_t *)HANDLE_REG_BASE(rtc);
    csi_error_t ret = CSI_OK;
    int32_t load_sec;

    do {

        ret = (csi_error_t)clock_check_tm_ok((const struct tm *)rtctime);

        if (ret < CSI_OK) {
            break;
        }

        load_sec = clock_update_basetime((const struct tm *)rtctime);

        if (load_sec <= 0) {        ///< Error returned when total seconds overflow is negative
            ret = CSI_ERROR;
            break;
        }

        qx_rtc_write_load_count(rtc_base, (uint32_t)load_sec);

        qx_rtc_counter_enable(rtc_base);

    } while (0);

    return ret;
}
/**
  \brief       Get system date
  \param[in]   rtc        handle rtc handle to operate
  \param[out]  rtctime    pointer to rtc time
  \return      error code \ref csi_error_t
*/
csi_error_t csi_rtc_get_time(csi_rtc_t *rtc, csi_rtc_time_t *rtctime)
{
    uint32_t cur_time;
    time_t time;

    CSI_PARAM_CHK(rtc, CSI_ERROR);
    CSI_PARAM_CHK(rtctime, CSI_ERROR);
    qx_rtc_regs_t *rtc_base = (qx_rtc_regs_t *)HANDLE_REG_BASE(rtc);

    cur_time = qx_rtc_read_runningtime(rtc_base);
    time = (time_t)(((cur_time - load_time) / (REAL_RTC_FREQ / NORMAL_RTC_FREQ)) + load_time);
    gmtime_r(&time, (struct tm *)rtctime);

    return CSI_OK;
}

/**
  \brief       Get alarm remaining time
  \param[in]   rtc    rtc handle to operate
  \return      the remaining time(s)
*/
uint32_t csi_rtc_get_alarm_remaining_time(csi_rtc_t *rtc)
{
    CSI_PARAM_CHK(rtc, 0U);

    return (int_number - (count / (REAL_RTC_FREQ / NORMAL_RTC_FREQ)));
}

/**
  \brief       Config RTC alarm ture timer
  \param[in]   rtc         handle rtc handle to operate
  \param[in]   rtctime     time(s) to wake up
  \param[in]   callback    callback function
  \param[in]   arg         callback's param
  \return      error code \ref csi_error_t
*/
csi_error_t csi_rtc_set_alarm(csi_rtc_t *rtc, const csi_rtc_time_t *rtctime, void *callback, void *arg)
{
    CSI_PARAM_CHK(rtc, CSI_ERROR);
    CSI_PARAM_CHK(rtctime, CSI_ERROR);
    qx_rtc_regs_t *rtc_base = (qx_rtc_regs_t *)HANDLE_REG_BASE(rtc);
    csi_error_t ret = (csi_error_t)clock_check_tm_ok((const struct tm *)rtctime);
    csi_rtc_time_t current_time;

    if (CSI_OK == ret) {
        rtc->callback = callback;
        rtc->arg = arg;
        csi_irq_attach((uint32_t)rtc->dev.irq_num, &qx_rtc_irq_handler, &rtc->dev);
        csi_irq_enable((uint32_t)rtc->dev.irq_num);

        csi_rtc_get_time(rtc, &current_time);            ///< get current time
        count = 0U;
        int_number = ((uint32_t)mktime((struct tm *)rtctime) - (uint32_t)mktime((struct tm *)&current_time));
        qx_rtc_intr_enable(rtc_base);
    }

    return ret;
}

/**
  \brief       Cancel the rtc alarm
  \param[in]   rtc    rtc handle to operate
  \return      error code \ref csi_error_t
*/
csi_error_t csi_rtc_cancel_alarm(csi_rtc_t *rtc)
{
    CSI_PARAM_CHK(rtc, CSI_ERROR);
    qx_rtc_regs_t *rtc_base = (qx_rtc_regs_t *)HANDLE_REG_BASE(rtc);

    rtc->callback = NULL;
    rtc->arg = NULL;
    qx_rtc_intr_disable(rtc_base);
    qx_rtc_write_match_count(rtc_base, 0U);
    csi_irq_disable((uint32_t)rtc->dev.irq_num);
    csi_irq_detach((uint32_t)rtc->dev.irq_num);

    return CSI_OK;
}

/**
  \brief       Judge rtc is working
  \param[in]   rtc    handle rtc handle to operate
  \return      state of work
               ture - rtc is running
               false -rtc is not running
*/
bool csi_rtc_is_running(csi_rtc_t *rtc)
{
    CSI_PARAM_CHK(rtc, false);
    qx_rtc_regs_t *rtc_base = (qx_rtc_regs_t *)HANDLE_REG_BASE(rtc);

    return (qx_rtc_get_counter_sta(rtc_base) ? true : false);
}

#ifdef CONFIG_PM
csi_error_t csi_rtc_enable_pm(csi_rtc_t *rtc)
{
    return CSI_UNSUPPORTED;
}

void csi_rtc_disable_pm(csi_rtc_t *rtc)
{
    return;
}
#endif

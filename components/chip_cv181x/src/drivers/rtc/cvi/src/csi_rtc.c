/*
 *    Copyright (C) Cvitek Co., Ltd. 2019-2022. All rights reserved.
 */

/*******************************************************
 * @file       rtc.c
 * @brief      source file for rtc csi driver
 * @version    V1.0
 * @date       24. Jan 2022
 * ******************************************************/

#include <stdint.h>
#include <drv/rtc.h>
#include <drv/cvi_irq.h>
#include <cvi_rtc.h>
#include <mmio.h>

/**
  \brief       Initialize RTC Interface. Initializes the resources needed for the RTC interface
  \param[in]   rtc    rtc handle to operate
  \param[in]   idx    rtc index
  \return      error code \ref csi_error_t
*/
csi_error_t csi_rtc_init(csi_rtc_t *rtc, uint32_t idx)
{
    CSI_PARAM_CHK(rtc, CSI_ERROR);
    csi_error_t ret;
	struct cvi_rtc *cvi_rtc;

	if(target_get(DEV_CVI_RTC_TAG, idx, &rtc->dev)){
		return CSI_ERROR;
	}

	cvi_rtc = malloc(sizeof(cvi_rtc));
	if(!cvi_rtc){
		printf("cvi_rtc malloc error\n");
		return CSI_ERROR;
	}

	rtc->priv = cvi_rtc;

	ret = cvi_rtc_init(cvi_rtc, (uintptr_t)HANDLE_REG_BASE(rtc), rtc->dev.irq_num);
	if(ret){
		printf("cvi_rtc_init failed\n");
		return CSI_ERROR;
	}

    return 0;
}

/**
  \brief       De-initialize RTC Interface. stops operation and releases the software resources used by the interface
  \param[in]   rtc    rtc handle to operate
  \return      None
*/
void csi_rtc_uninit(csi_rtc_t *rtc)
{
    CSI_PARAM_CHK_NORETVAL(rtc);
	cvi_rtc_uninit();
	free(rtc->priv);
}

/**
  \brief       Set system date
  \param[in]   rtc        handle rtc handle to operate
  \param[in]   rtctime    pointer to rtc time
  \return      error code \ref csi_error_t
*/
csi_error_t csi_rtc_set_time(csi_rtc_t *rtc, const csi_rtc_time_t *tm)
{
	CSI_PARAM_CHK(rtc, CSI_ERROR);
    CSI_PARAM_CHK(tm, CSI_ERROR);

	return cvi_rtc_set_time((struct cvi_rtc *)rtc->priv, (const cvi_rtc_time_t *)tm);
}

csi_error_t csi_rtc_set_time_no_wait(csi_rtc_t *rtc, const csi_rtc_time_t *tm)
{
	CSI_PARAM_CHK(rtc, CSI_ERROR);
    CSI_PARAM_CHK(tm, CSI_ERROR);

	return cvi_rtc_set_time_no_wait(rtc->priv, (const cvi_rtc_time_t *)tm);
}

/**
  \brief       Get system date
  \param[in]   rtc        handle rtc handle to operate
  \param[out]  rtctime    pointer to rtc time
  \return      error code \ref csi_error_t
*/
csi_error_t csi_rtc_get_time(csi_rtc_t *rtc, csi_rtc_time_t *tm)
{
    CSI_PARAM_CHK(rtc, CSI_ERROR);
    CSI_PARAM_CHK(tm, CSI_ERROR);

	return cvi_rtc_get_time(rtc->priv, (cvi_rtc_time_t *)tm);
}

/**
  \brief       Get alarm remaining time
  \param[in]   rtc    rtc handle to operate
  \return      the remaining time(s)
*/
uint32_t csi_rtc_get_alarm_remaining_time(csi_rtc_t *rtc)
{
    CSI_PARAM_CHK(rtc, 0U);
	return cvi_rtc_get_alarm_remaining_time(rtc->priv);
}

void csi_rtc_default_callback(struct cvi_rtc *cvi_rtc)
{
    csi_rtc_t *rtc = (csi_rtc_t *)cvi_rtc->arg;

	if (rtc->callback) {
            rtc->callback(rtc, rtc->arg);
    }
}

/**
  \brief       Config RTC alarm ture timer
  \param[in]   rtc         handle rtc handle to operate
  \param[in]   rtctime     time(s) to wake up
  \param[in]   callback    callback function
  \param[in]   arg         callback's param
  \return      error code \ref csi_error_t
*/
csi_error_t csi_rtc_set_alarm(csi_rtc_t *rtc, const csi_rtc_time_t *tm, void *callback, void *arg)
{
    CSI_PARAM_CHK(rtc, CSI_ERROR);
    CSI_PARAM_CHK(tm, CSI_ERROR);
	int ret;

	rtc->callback = callback;
	rtc->arg = arg;

	ret = cvi_rtc_set_alarm(rtc->priv, (const cvi_rtc_time_t *)tm, csi_rtc_default_callback, rtc);

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

    rtc->callback = NULL;
    rtc->arg = NULL;

    return cvi_rtc_cancel_alarm(rtc->priv);
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

    return 1U;
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


/*
* Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
*/

#include <drv/common.h>
#include <drv/pin.h>
#include <drv/cvi_irq.h>
#include <drv/tick.h>
#include <drv/wdt.h>
#include "cvi_wdt.h"

#define WDT_FREQ_DEFAULT 25000000UL
#define CVI_WDT_MAX_TOP		15

static void dw_wdt_irq_handler(unsigned int irqn, void *arg)
{
    csi_wdt_t *wdt = (csi_wdt_t *)arg;
    unsigned long reg_base = HANDLE_REG_BASE(wdt);

    if (cvi_wdt_get_irq_stat(reg_base))
    {
        cvi_wdt_clr_irq_en(reg_base);
        if (wdt->callback) {
            wdt->callback(wdt, wdt->arg);
        }
    }
}

/**
  \brief       Initialize WDT Interface. Initializes the resources needed for the WDT interface
  \param[in]   wdt    wdt handle to operate
  \param[in]   idx    wdt index
  \return      error code \ref csi_error_t
*/
csi_error_t csi_wdt_init(csi_wdt_t *wdt, uint32_t idx)
{
    CSI_PARAM_CHK(wdt, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    // unsigned long reg_base;

    if (0 == target_get(DEV_DW_WDT_TAG, idx, &wdt->dev)) {
        // reg_base = HANDLE_REG_BASE(wdt);
        // dw_wdt_reset_register(wdt_base);
    } else {
        ret = CSI_ERROR;
    }

    cvi_wdt_top_setting();

    return ret;
}

/**
  \brief       De-initialize WDT Interface. stops operation and releases the software resources used by the interface
  \param[in]   wdt    handle to operate
  \return      None
*/
void csi_wdt_uninit(csi_wdt_t *wdt)
{
    CSI_PARAM_CHK_NORETVAL(wdt);
    // dw_wdt_regs_t *wdt_base = (dw_wdt_regs_t *)HANDLE_REG_BASE(wdt);
    // dw_wdt_reset_register(wdt_base);
}

static inline int wdt_top_in_ms(unsigned int top)
{
	/*
	 * There are 16 possible timeout values in 0..15 where the number of
	 * cycles is 2 ^ (16 + i) and the watchdog counts down.
	 */
    // pr_debug("wdt top in seconds: %d/%d=%d\n", (1U << (16 + top)), chip->clk_hz, (1U << (16 + top)) / chip->clk_hz);
	return (1U << (16 + top)) / (WDT_FREQ_DEFAULT / 1000);
}

/**
  \brief       Set the WDT value
  \param[in]   wdt    handle to operate
  \param[in]   ms     the timeout value(ms)
  \return      error code \ref csi_error_t
*/
csi_error_t csi_wdt_set_timeout(csi_wdt_t *wdt, uint32_t ms)
{
    CSI_PARAM_CHK(wdt, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    unsigned long reg_base = HANDLE_REG_BASE(wdt);
    // uint32_t set_cnt = ((soc_get_wdt_freq((uint32_t)wdt->dev.idx) / 1000U) * ms) >> 16;
	int i, top_val = CVI_WDT_MAX_TOP;

	/*
	 * Iterate over the timeout values until we find the closest match. We
	 * always look for >=.
	 */
	for (i = 0; i <= CVI_WDT_MAX_TOP; ++i)
		if (wdt_top_in_ms(i) >= ms) {
			top_val = i;
			break;
		}

    if (i < CVI_WDT_MAX_TOP)
    {
        /*
        * Set the new value in the watchdog.  Some versions of wdt_chip
        * have TOPINIT in the TIMEOUT_RANGE register (as per
        * CP_WDT_DUAL_TOP in WDT_COMP_PARAMS_1).  On those we
        * effectively get a pat of the watchdog right here.
        */
        cvi_wdt_set_timeout(reg_base, top_val);
        // writel(top_val | top_val << WDOG_TIMEOUT_RANGE_TOPINIT_SHIFT,
        //     chip->reg_base + WDT_TORR);
        csi_wdt_feed(wdt);
    }
    else
    {
        ret = CSI_ERROR;
    }

    return ret;
}

/**
  \brief       Start the WDT
  \param[in]   wdt    handle to operate
  \return      error code \ref csi_error_t
*/
csi_error_t csi_wdt_start(csi_wdt_t *wdt)
{
    CSI_PARAM_CHK(wdt, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    unsigned long reg_base = HANDLE_REG_BASE(wdt);

    cvi_wdt_set_respond_system_reset(reg_base);

    cvi_wdt_start_en(reg_base);

    return ret;
}

/**
  \brief       Stop the WDT
  \param[in]   wdt    handle to operate
  \return      None
*/
void csi_wdt_stop(csi_wdt_t *wdt)
{
    CSI_PARAM_CHK_NORETVAL(wdt);
    unsigned long reg_base = HANDLE_REG_BASE(wdt);

    cvi_wdt_start_dis(reg_base);
}

/**
  \brief       Feed the WDT
  \param[in]   wdt    handle to operate
  \return      error code \ref csi_error_t
*/
csi_error_t csi_wdt_feed(csi_wdt_t *wdt)
{
    CSI_PARAM_CHK(wdt, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    unsigned long reg_base = HANDLE_REG_BASE(wdt);
    cvi_wdt_feed_en(reg_base);

    return ret;
}

/**
  \brief       Get the remaining time to timeout
  \param[in]   wdt    handle to operate
  \return      tne remaining time of wdt(ms)
*/
uint32_t csi_wdt_get_remaining_time(csi_wdt_t *wdt)
{
    CSI_PARAM_CHK(wdt, 0U);

    unsigned long reg_base = HANDLE_REG_BASE(wdt);

    // return (cvi_wdt_get_counter_value(reg_base) / (soc_get_wdt_freq((uint32_t)wdt->dev.idx) / 1000U));
    return (cvi_wdt_get_counter_value(reg_base) / (WDT_FREQ_DEFAULT / 1000U));
}

/**
  \brief       Check wdt is running
  \param[in]   wdt    handle wdt handle to operate
  \return      true->running, false->stopped
*/
bool csi_wdt_is_running(csi_wdt_t *wdt)
{
    CSI_PARAM_CHK(wdt, false);

    unsigned long reg_base = HANDLE_REG_BASE(wdt);

    return (cvi_wdt_get_start(reg_base) ? true : false);
}

#ifndef CONFIG_KERNEL_NONE

/**
  \brief       Attach the callback handler to wdt
  \param[in]   wdt         operate handle
  \param[in]   callback    callback function
  \param[in]   arg         callback's param
  \return      error code \ref csi_error_t
*/
csi_error_t csi_wdt_attach_callback(csi_wdt_t *wdt, void *callback, void *arg)
{
    CSI_PARAM_CHK(wdt, CSI_ERROR);
    unsigned long reg_base = HANDLE_REG_BASE(wdt);

	if (!wdt->dev.irq_num)
	{
		pr_err("wdt %d reg_base %x is unsupported\n", wdt->dev.idx, wdt->dev.reg_base);
		return CSI_UNSUPPORTED;
	}

    wdt->callback = callback;
    wdt->arg = arg;

#if 0
    csi_irq_attach((uint32_t)wdt->dev.irq_num, &dw_wdt_irq_handler, &wdt->dev);
#else
    request_irq((uint32_t)(wdt->dev.irq_num), &dw_wdt_irq_handler, 0, "wdt int", wdt);
#endif
    csi_irq_enable((uint32_t)wdt->dev.irq_num);
    cvi_wdt_set_respond_irq_then_reset(reg_base);

    return CSI_OK;
}

/**
  \brief       Detach the callback handler
  \param[in]   wdt    operate handle
  \return      None
*/
void csi_wdt_detach_callback(csi_wdt_t *wdt)
{
    CSI_PARAM_CHK_NORETVAL(wdt);
    unsigned long reg_base = HANDLE_REG_BASE(wdt);

    wdt->callback = NULL;
    wdt->arg = NULL;
    csi_irq_disable((uint32_t)wdt->dev.irq_num);
    csi_irq_detach((uint32_t)wdt->dev.irq_num);
    cvi_wdt_set_respond_system_reset(reg_base);
}

#endif

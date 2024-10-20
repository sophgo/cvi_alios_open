/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     reboot.c
 * @brief    source file for the reboot
 * @version  V1.0
 * @date     7. April 2020
 ******************************************************************************/

#include <soc.h>
#include <drv/wdt.h>
#include <drv/porting.h>
#include <cvi_rtc_pwr.h>

boot_reason_t soc_get_boot_reason(void)
{
    // FIXME:
    return BOOTREASON_OTHER;
}

__attribute__((weak)) void reboot_pre_hook(void)
{

}

void drv_reboot(void)
{
    reboot_pre_hook();
    csi_wdt_t wdt_handle;
    uint32_t irq_flag = 0U;

    irq_flag = csi_irq_save();

    csi_wdt_init(&wdt_handle, 0U);
    csi_wdt_set_timeout(&wdt_handle, 0U);
    csi_wdt_start(&wdt_handle);

    /* waiting for reboot */
    while (1);


    csi_irq_restore(irq_flag);
}

void drv_poweroff(void)
{
	//default wakup source: pwr_button1
	rtc_set_wakeup_source(RTC_EN_PWRUP_PWR_BUTTON1);

	//do poweroff
	rtc_do_poweroff();
}
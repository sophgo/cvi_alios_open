/*
 *    Copyright (C) Cvitek Co., Ltd. 2019-2022. All rights reserved.
 */

/*******************************************************
 * @file       rtc_pwr.c
 * @brief      source file for rtc csi driver
 * @version    V1.0
 * @date       2. Jul 2024
 * ******************************************************/

#include <stdint.h>
#include <drv/cvi_irq.h>
#include <top_reg.h>
#include <mmio.h>
#include <cvi_rtc_pwr.h>

static void rtc_selphy_pwr_io_status(void)
{
	//selphy IO status
	mmio_write_32(REG_ioblk_GRTC_BASE + REG_CTRL_selphy_PWR_BUTTON1, 0x0);
	mmio_write_32(REG_ioblk_GRTC_BASE + REG_CTRL_selphy_PWR_ON,	 0x0);
	mmio_write_32(REG_ioblk_GRTC_BASE + REG_CTRL_selphy_PWR_WAKEUP0, 0x0);
	mmio_write_32(REG_ioblk_GRTC_BASE + REG_CTRL_selphy_PWR_WAKEUP1, 0x0);
}

static void rtc_reset_pwr_io_pinmux(void)
{
	//set pinmux to pwr_wakeup0, pwr_wakeup1, pwr_button1, pwr_on
	mmio_write_32(PINMUX_BASE + REG_IOCTRL_PWR_WAKEUP0, 0x0);
	mmio_write_32(PINMUX_BASE + REG_IOCTRL_PWR_WAKEUP1, 0x0);
	mmio_write_32(PINMUX_BASE + REG_IOCTRL_PWR_BUTTON1, 0x0);
	mmio_write_32(PINMUX_BASE + REG_IOCTRL_PWR_ON, 0x0);
}

void rtc_set_wakeup_source(csi_wakeup_source_t source)
{
	//reset pwr io pinmux before poweroff
	rtc_reset_pwr_io_pinmux();
	mmio_write_16(REG_RTC_BASE + RTC_EN_PWR_WAKEUP, source);
}

void rtc_do_poweroff(void)
{
	/* Enable power suspend wakeup source mask */
	mmio_write_32(REG_RTC_BASE + RTC_APB_RDATA_SEL, 0x1); // 1 = select prdata from 32K domain

	/* Unlock rtc ctrl */
	mmio_write_32(REG_RTC_CTRL_BASE + RTC_CTRL_UNLOCKKEY, 0xAB18);

	/* select POR reset */
	mmio_write_32(REG_RTC_CTRL_BASE + RTCSYS_POR_RST_CTRL, 0x2);

	/* Enable vdet power up */
	mmio_write_32(REG_RTC_BASE + RTC_EN_PWR_VBAT_DET, 3);

	/* set pwr_wakup0 low activate */
	mmio_write_32(REG_RTC_BASE + RTC_PWR_WAKEUP_POLARITY, 0);

	/* selphy IO status */
	rtc_selphy_pwr_io_status();

	/* Send shutdown request */
	mmio_write_32(REG_RTC_BASE + RTC_EN_SHDN_REQ, 0x1);

	while (mmio_read_32(REG_RTC_BASE + RTC_EN_SHDN_REQ) != 0x01)
		;

	mmio_write_32(REG_RTC_CTRL_BASE + RTC_CTRL0, 0xFFFF0800 | (0x1 << 0));

	/* Wait some time until system down, otherwise, notice with a warn */
	mdelay(1000);

}
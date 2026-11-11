#include <drv/rtc.h>
#include "aos/cli.h"
#include <cvi_rtc.h>
#include <aos/hal/rtc.h>
#include <stdio.h>
#include <string.h>
#include <aos/aos.h>

#include <stdint.h>
#include <drv/cvi_irq.h>
#include <mmio.h>

/*******************************************************
 * Description:	This file provides RTC test code for CVITEK platform.
 *				It initializes the RTC, sets a specific time, and then
 *				continuously reads and prints the current time every second.
 * Version:	  V1.0
 */

int test_rtc_time_diff(csi_rtc_time_t tm1, csi_rtc_time_t tm2)
{
	int diff = 0;

	if (tm1.tm_year == tm2.tm_year && tm1.tm_mon == tm2.tm_mon && tm1.tm_mday == tm2.tm_mday &&
		tm1.tm_hour == tm2.tm_hour && tm1.tm_min == tm2.tm_min) {
		diff = tm1.tm_sec - tm2.tm_sec;
	} else {
		diff = 0;
	}
	return diff;
}
void test_rtc(int32_t argc, char **argv)
{

	int32_t ret;
	int32_t i = 0;
	int32_t sec_diff = 0;
	csi_rtc_t rtc_handle;
	csi_rtc_time_t rtc_time_set;
	csi_rtc_time_t rtc_time_get;

	// 1. init RTC
	printf("Initializing RTC...\n");
	ret = csi_rtc_init(&rtc_handle, 0);
	if (ret != CSI_OK) {
		printf("RTC init failed, ret = %d\n", ret);
		return;
	}
	printf("RTC init success.\n");

	// 2. set start time 2026-02-28 12:30:45
	memset(&rtc_time_set, 0, sizeof(csi_rtc_time_t));
	rtc_time_set.tm_year = 2026 - 1900;  // year since 1900
	rtc_time_set.tm_mon  = 2 - 1;		 // month 0-11
	rtc_time_set.tm_mday = 28;
	rtc_time_set.tm_hour = 12;
	rtc_time_set.tm_min  = 30;
	rtc_time_set.tm_sec  = 10;

	printf("Setting RTC time...\n");
	ret = csi_rtc_set_time(&rtc_handle, &rtc_time_set);
	if (ret != CSI_OK) {
		printf("RTC set time failed, ret = %d\n", ret);
		csi_rtc_uninit(&rtc_handle);
		return;
	}
	printf("RTC set time success.\n");

	// 3. read and print current time every second
	while (i++ < 10) {
		aos_msleep(1000);
		memset(&rtc_time_get, 0, sizeof(csi_rtc_time_t));
		ret = csi_rtc_get_time(&rtc_handle, &rtc_time_get);
		if (ret == CSI_OK) {
			printf("Current Time: %04d-%02d-%02d %02d:%02d:%02d\n",
				   rtc_time_get.tm_year + 1900,
				   rtc_time_get.tm_mon + 1,
				   rtc_time_get.tm_mday,
				   rtc_time_get.tm_hour,
				   rtc_time_get.tm_min,
				   rtc_time_get.tm_sec);
		} else {
			printf("RTC get time failed, ret = %d\n", ret);
		}
	}

	sec_diff = test_rtc_time_diff(rtc_time_get, rtc_time_set);
	printf("Time difference: %d seconds\n", sec_diff);
	if (sec_diff == 10 || sec_diff == 9) // allow 1 second deviation
		printf("[rtc]:cvitek rtc test pass\n");
	else
		printf("[rtc]:cvitek rtc test fail\n");

	csi_rtc_uninit(&rtc_handle);
}

ALIOS_CLI_CMD_REGISTER(test_rtc, test_rtc, test rtc function);
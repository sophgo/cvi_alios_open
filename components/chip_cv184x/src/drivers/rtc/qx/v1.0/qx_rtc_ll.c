/*
 *    Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/*******************************************************
 * @file       qx_rtc_ll.c
 * @brief      source file for rtc ll driver
 * @version    V1.0
 * @date       16. July 2020
 * ******************************************************/
#include <drv/rtc.h>
#include "rtc_alg.h"
#include "qx_rtc_ll.h"

uint32_t qx_rtc_read_current_count(qx_rtc_regs_t *rtc_base)
{
    volatile uint32_t second, week;

    rtc_latch(rtc_base);
    second = rtc_get_latch_second(rtc_base);
    week = rtc_get_latch_week(rtc_base);

    return ((week * QX_RTC_CAL_WEEK_TO_SECOND) + second);
}

void qx_rtc_write_load_count(qx_rtc_regs_t *rtc_base, uint32_t count)
{
    ;
}
void qx_rtc_write_match_count(qx_rtc_regs_t *rtc_base, uint32_t count)
{
    ;
}

uint32_t qx_rtc_read_runningtime(qx_rtc_regs_t *rtc_base)
{
    uint32_t index;
    uint32_t second, week;

    rtc_latch(rtc_base);

    for (index = 0U; index < 30000U; index++) {
        __NOP();
    }

    second = rtc_get_latch_second(rtc_base);
    week = rtc_get_latch_week(rtc_base);

    return ((week * QX_RTC_CAL_WEEK_TO_SECOND) + second);
}

uint32_t qx_rtc_read_alarmcount(qx_rtc_regs_t *rtc_base)
{
    return 0U;
}

int32_t qx_rtc_write_loadtime(qx_rtc_regs_t *rtc_base, uint32_t settime)
{
    uint32_t second;
    int16_t subsecond = 0;
    uint16_t week;
    int32_t ret = 0;
    uint32_t timecount;

    week = (uint16_t)(settime / QX_RTC_CAL_WEEK_TO_SECOND);
    second = settime % QX_RTC_CAL_WEEK_TO_SECOND;
    timecount = 0U;

    while (rtc_get_weekupdate_sta(rtc_base)) {

        QX_RTC_WAIT_IS_TIMEOUT(++timecount, ret);

        if (ret < 0) {
            break;
        }

    }
    rtc_set_current_week(rtc_base, (uint32_t)week);

    timecount = 0U;

    while (rtc_get_weekupdate_sta(rtc_base)) {

        QX_RTC_WAIT_IS_TIMEOUT(++timecount, ret);

        if (ret < 0) {
            break;
        }

    }
    timecount = 0U;

    while (rtc_get_secondupdate_sta(rtc_base)) {

        QX_RTC_WAIT_IS_TIMEOUT(++timecount, ret);

        if (ret < 0) {
            break;
        }

    }
    rtc_set_current_second(rtc_base, second);

    timecount = 0U;

    while (rtc_get_secondupdate_sta(rtc_base)) {

        QX_RTC_WAIT_IS_TIMEOUT(++timecount, ret);

        if (ret < 0) {
            break;
        }

    }
    timecount = 0U;

    while (rtc_get_subsecondupdate_sta(rtc_base)) {

        QX_RTC_WAIT_IS_TIMEOUT(++timecount, ret);

        if (ret < 0) {
            break;
        }

    }
    rtc_set_current_subsecond(rtc_base, (uint32_t)subsecond);

    timecount = 0U;

    while (rtc_get_subsecondupdate_sta(rtc_base)) {

        QX_RTC_WAIT_IS_TIMEOUT(++timecount, ret);

        if (ret < 0) {
            break;
        }

    }

    return ret;
}

void qx_rtc_write_loadtime_no_wait(qx_rtc_regs_t *rtc_base, uint32_t settime)
{
    ;
}

void qx_rtc_write_matchtime(qx_rtc_regs_t *rtc_base, uint32_t settime)
{
    ;
}

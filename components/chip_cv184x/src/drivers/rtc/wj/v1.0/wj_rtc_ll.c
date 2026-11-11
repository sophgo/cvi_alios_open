/*
 *	Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/*******************************************************
 * @file 	wj_rtc_ll.c
 * @brief	source file for rtc ll driver
 * @version V1.0
 * @date	10. April 2020
 * ******************************************************/
#include <drv/rtc.h>
#include "rtc_alg.h"
#include "wj_rtc_ll.h"


#ifdef CONFIG_RTC_FAMILY_A
#define WJ_RTC_CONVERT_CALENDAR2TIME(_val_, _rtc_base_)                               (((uint64_t)_val_ * soc_get_rtc_freq(0U)) / 1000U)
#define WJ_RTC_CONVERT_TIME2CALENDAR(_val_, _rtc_base_)                               (((uint64_t)_val_ * 1000U) / soc_get_rtc_freq(0U))
#endif

#ifdef CONFIG_RTC_FAMILY_B
#define WJ_RTC_CONVERT_CALENDAR2TIME(_val_, _rtc_base_)                               (((uint64_t)_val_ * (soc_get_rtc_freq(0U)) / 1000U) / 16384U)
#define WJ_RTC_CONVERT_TIME2CALENDAR(_val_, _rtc_base_)                               (((uint64_t)_val_ * 16384U) / (soc_get_rtc_freq(0U) / 1000U))
#endif

#ifdef CONFIG_RTC_FAMILY_C
#define WJ_RTC_GET_DIVIDER_VALUE(_rtc_base_)                                          ((wj_rtc_get_divider(_rtc_base_) + 1U) << 1U)
#define WJ_RTC_CONVERT_CALENDAR2TIME(_val_, _rtc_base_)                               (((uint64_t)_val_ * (27373U * 2U)) / (WJ_RTC_GET_DIVIDER_VALUE(_rtc_base_) * 1000U))
#define WJ_RTC_CONVERT_TIME2CALENDAR(_val_, _rtc_base_)                               (((uint64_t)_val_ * (1000U * WJ_RTC_GET_DIVIDER_VALUE(_rtc_base_))) / (27373U * 2U))
#endif

#ifdef CONFIG_RTC_FAMILY_D
#define WJ_RTC_GET_DIVIDER_VALUE(_rtc_base_)                                          ((wj_rtc_get_divider(_rtc_base_) + 1U) << 1U)
#define WJ_RTC_CONVERT_CALENDAR2TIME(_val_, _rtc_base_)                               ((((uint64_t)_val_ * (soc_get_rtc_freq(0U) << 1)) / WJ_RTC_GET_DIVIDER_VALUE(_rtc_base_)) / 1000U)
#define WJ_RTC_CONVERT_TIME2CALENDAR(_val_, _rtc_base_)                               (((uint64_t)_val_ * 1000U) / ((soc_get_rtc_freq(0U) << 1) / WJ_RTC_GET_DIVIDER_VALUE(_rtc_base_)))
#endif

#ifdef CONFIG_RTC_FAMILY_E
#define WJ_RTC_GET_DIVIDER_VALUE(_rtc_base_)                                          ((wj_rtc_get_divider(_rtc_base_) + 1U) << 1U)
#define WJ_RTC_CONVERT_CALENDAR2TIME(_val_, _rtc_base_)                               (((uint64_t)_val_ * (soc_get_rtc_freq(0U) / WJ_RTC_GET_DIVIDER_VALUE(_rtc_base_))) / 1000U)
#define WJ_RTC_CONVERT_TIME2CALENDAR(_val_, _rtc_base_)                               (((uint64_t)_val_ * 1000U) / (soc_get_rtc_freq(0U) / WJ_RTC_GET_DIVIDER_VALUE(_rtc_base_)))
#endif

#define WJ_RTC_WAIT_WRITED_10S                                                        (10000U)

#define WJ_RTC_WAIT_IS_TIMEOUT(_time_ms_, _result_)                                   {        \
        do {                                                                                   \
            if (_time_ms_ >= WJ_RTC_WAIT_WRITED_10S) {                                         \
                _result_ = -1;                                                                 \
            }                                                                                  \
            mdelay(1U);                                                                        \
        } while(0);                                                                            \
    }

uint64_t wj_rtc_read_current_count(wj_rtc_regs_t *rtc_base)
{
    uint64_t retcount = 0U;

    retcount = wj_rtc_get_currcount_h(rtc_base);
    retcount <<= 32U;
    retcount += wj_rtc_get_currcount_l(rtc_base);

    return retcount;
}
uint64_t wj_rtc_read_load_count(wj_rtc_regs_t *rtc_base)
{
    uint64_t retcount = 0U;

    retcount = wj_rtc_get_loadcount_h(rtc_base);
    retcount <<= 32U;
    retcount += wj_rtc_get_loadcount_l(rtc_base);

    return retcount;
}
uint64_t wj_rtc_read_match_count(wj_rtc_regs_t *rtc_base)
{
    uint64_t retcount = 0U;

    retcount = wj_rtc_get_matchcount_h(rtc_base);
    retcount <<= 32U;
    retcount += wj_rtc_get_matchcount_l(rtc_base);

    return retcount;
}

void wj_rtc_write_load_count(wj_rtc_regs_t *rtc_base, uint64_t count)
{
    wj_rtc_set_loadcount_l(rtc_base, (uint32_t)count);
    wj_rtc_set_loadcount_h(rtc_base, (uint32_t)(count >> 32U));
}
void wj_rtc_write_match_count(wj_rtc_regs_t *rtc_base, uint64_t count)
{
    wj_rtc_set_matchcount_l(rtc_base, (uint32_t)count);
    wj_rtc_set_matchcount_h(rtc_base, (uint32_t)(count >> 32U));
}

uint64_t wj_rtc_read_runningtime(wj_rtc_regs_t *rtc_base)
{
    uint64_t currecount;
    uint64_t loadcount;
    uint64_t retime = 0U;

    currecount = wj_rtc_read_current_count(rtc_base);
    loadcount = wj_rtc_read_load_count(rtc_base);

    retime = WJ_RTC_CONVERT_TIME2CALENDAR((currecount - loadcount), (wj_rtc_regs_t *)rtc_base);

    return retime;
}

uint64_t wj_rtc_read_remaincount(wj_rtc_regs_t *rtc_base)
{
    uint64_t matchcount;
    uint64_t currecount;
    uint64_t retime = 0U;

    matchcount = wj_rtc_read_match_count(rtc_base);
    currecount = wj_rtc_read_current_count(rtc_base);

    if (matchcount > currecount) {
        retime = WJ_RTC_CONVERT_TIME2CALENDAR((matchcount - currecount), (wj_rtc_regs_t *)rtc_base);
    }

    return retime;
}

int32_t wj_rtc_write_loadtime(wj_rtc_regs_t *rtc_base, uint64_t settime)
{
    int32_t ret = 0;
    uint32_t loopcount = 10U;
    uint32_t timecount = 0U;
    uint64_t currcount = 0U;

    /**
     * Make sure the data has been written properly
    */
    do {
        loopcount++;

        if (loopcount > 10U) {
            loopcount = 0U;
            wj_rtc_write_load_count(rtc_base, (uint64_t)settime);
        }

        currcount = wj_rtc_read_current_count(rtc_base);

        WJ_RTC_WAIT_IS_TIMEOUT(++timecount, ret);

        if (ret < 0) {
            break;
        }
    } while (currcount != settime);

    return ret;
}

void wj_rtc_write_loadtime_no_wait(wj_rtc_regs_t *rtc_base, uint64_t settime)
{
    wj_rtc_write_load_count(rtc_base, settime);
}

void wj_rtc_write_matchtime(wj_rtc_regs_t *rtc_base, uint64_t settime)
{
    uint64_t matchcount;

    matchcount  = WJ_RTC_CONVERT_CALENDAR2TIME(settime, (wj_rtc_regs_t *)rtc_base);

    matchcount += wj_rtc_read_current_count(rtc_base);

    wj_rtc_write_match_count(rtc_base, matchcount);

}

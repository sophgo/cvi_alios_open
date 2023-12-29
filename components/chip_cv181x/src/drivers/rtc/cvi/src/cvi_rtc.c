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
#include <drv/cvi_irq.h>
#include <cvi_rtc.h>
#include <mmio.h>
#include <errno.h>

int cvi_rtc_get_chipsn(uint64_t *sn)
{
	if(sn == NULL)
	{
		printf("null pointer\n");
		return 0;
	}
	*sn = mmio_read_64((long unsigned int)RTC_OFFSET_SN);
	printf("the chipsn is %lx\n",*sn);
	return 1;
}

static void rtc_clk_enable()
{
	hal_cvi_rtc_clk_set(1);
}

static void rtc_clk_disable()
{
	hal_cvi_rtc_clk_set(0);
}

int cvi_rtc_init(struct cvi_rtc *rtc, uintptr_t reg_base, uint32_t irq_num)
{
	CVI_PARAM_CHK(rtc, -EINVAL);

	rtc->reg_base = reg_base;
	rtc->irq_num = irq_num;

	rtc_clk_enable();
#if defined(CV_RTC_FINE_CALIB)
	hal_cvi_rtc_32k_coarse_value_calib(rtc->reg_base);
	hal_cvi_rtc_32k_fine_value_calib(rtc->reg_base);
#endif
	hal_cvi_rtc_enable_sec_counter(rtc->reg_base);

	return 0;
}

void cvi_rtc_uninit(void)
{
    rtc_clk_disable();
}

static int __rtc_set_time(struct cvi_rtc *rtc, const cvi_rtc_time_t *tm, unsigned long *ret_sec)
{
	int ret;
    uintptr_t rtc_base = rtc->reg_base;
    unsigned long sec;

	rtc_dbg("enter csi_rtc_set_time\n");

	/* convert tm to seconds. */
	ret = rtc_valid_tm(tm);
	if (ret)
		return ret;

	sec = rtc_tm_to_time64(tm);
	*ret_sec = sec;

	rtc_dbg("%s %lx, %d\n", __func__, sec, __LINE__);

	rtc_dbg("time set to %lx. %d/%d/%d %d:%02x:%02x\n",
		sec,
		tm->tm_mon+1,
		tm->tm_mday,
		tm->tm_year+1900,
		tm->tm_hour,
		tm->tm_min,
		tm->tm_sec
	);

	hal_cvi_rtc_set_time(rtc_base, sec);

	return 0;
}

int rtc_get_time_sec(struct cvi_rtc *rtc, unsigned long *ret_sec)
{
	CVI_PARAM_CHK(rtc, -EINVAL);
    size_t irq_state;
	int ret;
	uintptr_t rtc_base = rtc->reg_base;

	irq_state = csi_irq_save();

	ret = hal_cvi_rtc_get_time_sec(rtc_base, ret_sec);

	csi_irq_restore(irq_state);

	return ret;
}

int cvi_rtc_set_time(struct cvi_rtc *rtc, const cvi_rtc_time_t *tm)
{
	CVI_PARAM_CHK(rtc, -EINVAL);
    CVI_PARAM_CHK(tm, -EINVAL);
	int ret;
	unsigned long sec;
	unsigned long cur_sec;
	int loop_count = 0;
	
	ret = __rtc_set_time(rtc, tm, &sec);
	if(ret){
		rtc_dbg("__rtc_set_time error\n");
		return ret;
	}

	ret = rtc_get_time_sec(rtc, &cur_sec);

	while((sec != cur_sec || ret) && loop_count < 100){
		loop_count++;
		udelay(100);
		ret = rtc_get_time_sec(rtc, &cur_sec);
	}

	if(ret || sec != cur_sec){
		rtc_dbg("loop_count=%d\n", loop_count);
		rtc_dbg("ret=%d, sec=%lu, cur_sec=%lu\n", ret, sec, cur_sec);
		return -EFAULT;
	}

	rtc_dbg("cvi_rtc_set_time:%lx\n", cur_sec);

    return 0;
}

int cvi_rtc_set_time_no_wait(struct cvi_rtc*rtc, const cvi_rtc_time_t *tm)
{
	CVI_PARAM_CHK(rtc, CSI_ERROR);
    CVI_PARAM_CHK(tm, CSI_ERROR);
	int ret;
	unsigned long sec;

	ret = __rtc_set_time(rtc, tm, &sec);
	if(ret)
		return ret;

	rtc_dbg("cvi_rtc_set_time_no_wait:%lx\n", sec);

	return 0;
}

int cvi_rtc_get_time(struct cvi_rtc *rtc, cvi_rtc_time_t *tm)
{
    CVI_PARAM_CHK(rtc, CSI_ERROR);
    CVI_PARAM_CHK(tm, CSI_ERROR);

	unsigned long sec;
	int ret = 0;

	ret = rtc_get_time_sec(rtc, &sec);
	if(ret)
		return ret;

	rtc_time64_to_tm(sec, tm);

	rtc_dbg("%s %lu\n", __func__, sec);

	rtc_dbg("time read as %lu. %d/%d/%d %d:%02u:%02u\n",
		sec,
		tm->tm_mon + 1,
		tm->tm_mday,
		tm->tm_year + 1900,
		tm->tm_hour,
		tm->tm_min,
		tm->tm_sec
	);

    return ret;
}

uint32_t cvi_rtc_get_alarm_remaining_time(struct cvi_rtc *rtc)
{
    CVI_PARAM_CHK(rtc, 0U);
	uint32_t alarm_time;
	unsigned long sec;
	uintptr_t rtc_base = rtc->reg_base;

	rtc_get_time_sec(rtc, &sec);
	alarm_time = hal_cvi_rtc_get_alarm_time(rtc_base);

    return alarm_time - sec;
}

void cvi_rtc_irq_handler(unsigned int irqn, void *arg)
{
    struct cvi_rtc *rtc = (struct cvi_rtc *)arg;
    uintptr_t rtc_base = rtc->reg_base;

	hal_cvi_rtc_disable(rtc_base);
	if (rtc->callback) {
            rtc->callback(rtc);
    }
}

int cvi_rtc_set_alarm(struct cvi_rtc *rtc, const cvi_rtc_time_t *tm, void *callback, void *arg)
{
    CVI_PARAM_CHK(rtc, -EINVAL);
    CVI_PARAM_CHK(tm, -EINVAL);
	unsigned long alarm_time;
	
    uintptr_t rtc_base = rtc->reg_base;

	alarm_time = rtc_tm_to_time64(tm);
	rtc_dbg("CVITEK set alarm %lu\n", alarm_time);
	rtc_dbg("CVITEK current time %d\n", mmio_read_32(rtc_base + CVI_RTC_SEC_CNTR_VALUE));

	if (alarm_time > RTC_SEC_MAX_VAL)
		return -EINVAL;

	rtc->callback = callback;
	rtc->arg = arg;
	request_irq(rtc->irq_num, &cvi_rtc_irq_handler, 0, "rtc", rtc);

	hal_cvi_rtc_set_alarm(rtc_base, alarm_time);

    return 0;
}

int cvi_rtc_cancel_alarm(struct cvi_rtc *rtc)
{
    CVI_PARAM_CHK(rtc, -EINVAL);
    uintptr_t rtc_base = rtc->reg_base;

    rtc->callback = NULL;
    rtc->arg = NULL;

	hal_cvi_rtc_disable(rtc_base);
    csi_irq_disable(rtc->irq_num);
    csi_irq_detach(rtc->irq_num);

    return 0;
}

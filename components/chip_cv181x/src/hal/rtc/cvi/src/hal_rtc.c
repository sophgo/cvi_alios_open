
#include <stdint.h>
#include <mmio.h>
#include <errno.h>
#include <hal_rtc.h>
#include <drv/common.h>

void hal_cvi_rtc_clk_set(int enable)
{
	uint32_t clk_state;

	clk_state = mmio_read_32((long unsigned int)CLK_EN_0);

	if(enable)
		clk_state |= CLK_RTC_25M_BIT;
	else
		clk_state &= ~(CLK_RTC_25M_BIT);

	mmio_write_32((long unsigned int)CLK_EN_0, clk_state);
}

void hal_cvi_rtc_enable_sec_counter(uintptr_t rtc_base)
{
	uint32_t value = 0;

	value = mmio_read_32(rtc_base + CVI_RTC_SEC_PULSE_GEN) & ~(1 << 31);
	mmio_write_32(rtc_base + CVI_RTC_SEC_PULSE_GEN, value);

	value = mmio_read_32(rtc_base + CVI_RTC_ANA_CALIB) & ~(1 << 31);
	mmio_write_32(rtc_base + CVI_RTC_ANA_CALIB, value);

	mmio_read_32(rtc_base + CVI_RTC_SEC_CNTR_VALUE);
	mmio_write_32(rtc_base + CVI_RTC_ALARM_ENABLE, 0x0);
}

void hal_cvi_rtc_set_time(uintptr_t rtc_base, unsigned long sec)
{
	mmio_write_32(rtc_base + CVI_RTC_SET_SEC_CNTR_VALUE, sec);
	mmio_write_32(rtc_base + CVI_RTC_SET_SEC_CNTR_TRIG, 1);
	mmio_write_32(rtc_base + RTC_MACRO_RG_SET_T, sec);
	mmio_write_32(rtc_base + RTC_MACRO_DA_CLEAR_ALL, 1);
	mmio_write_32(rtc_base + RTC_MACRO_DA_SOC_READY, 1);
	mmio_write_32(rtc_base + RTC_MACRO_DA_CLEAR_ALL, 0);
	mmio_write_32(rtc_base + RTC_MACRO_RG_SET_T, 0);
	mmio_write_32(rtc_base + RTC_MACRO_DA_SOC_READY, 0);
}

int hal_cvi_rtc_get_time_sec(uintptr_t rtc_base,unsigned long *ret_sec)
{
	int ret = 0;
	unsigned long sec;
	unsigned long sec_ro_t;

	sec = mmio_read_32(rtc_base + CVI_RTC_SEC_CNTR_VALUE);
	sec_ro_t = mmio_read_32(rtc_base + RTC_MACRO_RO_T);

	rtc_dbg("sec=%lx, sec_ro_t=%lx\n", sec, sec_ro_t);

	if (sec_ro_t > 0x30000000) {
			sec = sec_ro_t;
			// Writeback to SEC CVI_RTC_SEC_CNTR_VALUE
			mmio_write_32(rtc_base + CVI_RTC_SET_SEC_CNTR_VALUE, sec);
			mmio_write_32(rtc_base + CVI_RTC_SET_SEC_CNTR_TRIG, 1);
	} else if (sec < 0x30000000) {
		rtc_dbg("RTC invalid time\n");
		ret = -EINVAL;
	}

	*ret_sec = sec;

	return ret;
}

uint32_t hal_cvi_rtc_get_alarm_time(uintptr_t rtc_base)
{
	return mmio_read_32(rtc_base + CVI_RTC_ALARM_TIME);
}

void hal_cvi_rtc_disable(uintptr_t rtc_base)
{
	/* Clear interrupt bit */
	mmio_write_32((uintptr_t)(rtc_base + CVI_RTC_ALARM_ENABLE), 0x0);
}

void hal_cvi_rtc_set_alarm(uintptr_t rtc_base, uint32_t alarm_time)
{
	uint32_t wakeup_src_mask;

	mmio_write_32(rtc_base + CVI_RTC_ALARM_ENABLE, 0x0);
	rtc_dbg("CVITEK CVI_RTC_ALARM_ENABLE %x\n", mmio_read_32(rtc_base + CVI_RTC_ALARM_ENABLE));
	udelay(200);

	mmio_write_32(rtc_base + CVI_RTC_ALARM_TIME, (u32)alarm_time);
	mmio_write_32(rtc_base + CVI_RTC_APB_RDATA_SEL, 0x1);
	mmio_write_32(rtc_base + CVI_RTC_ALARM_ENABLE, 0x1);
	wakeup_src_mask = mmio_read_32(rtc_base + CVI_RTC_EN_PWR_WAKEUP) | 0x30;
	mmio_write_32(rtc_base + CVI_RTC_EN_PWR_WAKEUP, wakeup_src_mask);
	mmio_read_32(rtc_base + CVI_RTC_SEC_CNTR_VALUE);
	rtc_dbg("CVITEK CVI_RTC_ALARM_ENABLE %x\n", mmio_read_32(rtc_base + CVI_RTC_ALARM_ENABLE));
}

#if defined(CV_RTC_FINE_CALIB)
void hal_cvi_rtc_32k_coarse_value_calib(uintptr_t rtc_base)
{
	uint32_t analog_calib_value = 0;
	uint32_t fc_coarse_time1 = 0;
	uint32_t fc_coarse_time2 = 0;
	uint32_t fc_coarse_value = 0;
	uint32_t offset = 128;
	uint32_t value = 0;
	uintptr_t rtc_ctrl_base = CVI_RTC_CTRL_BASE;

	rtc_dbg("enter rtc_32k_coarse_value_calib\n");

	mmio_write_32(rtc_base + CVI_RTC_ANA_CALIB, 0x10100);
	udelay(200);

	// Select 32K OSC tuning value source from rtc_sys
	value = mmio_read_32(rtc_base + CVI_RTC_SEC_PULSE_GEN) & ~(1 << 31);
	mmio_write_32(rtc_base + CVI_RTC_SEC_PULSE_GEN, value);

	analog_calib_value = mmio_read_32(rtc_base + CVI_RTC_ANA_CALIB);
	rtc_dbg("RTC_ANA_CALIB: 0x%x\n", analog_calib_value);

	mmio_write_32(rtc_ctrl_base + CVI_RTC_FC_COARSE_EN, 1);

	while (1) {
		fc_coarse_time1 = mmio_read_32(rtc_ctrl_base + CVI_RTC_FC_COARSE_CAL);
		fc_coarse_time1 >>= 16;
		rtc_dbg("fc_coarse_time1 = 0x%x\n", fc_coarse_time1);
		rtc_dbg("fc_coarse_time2 = 0x%x\n", fc_coarse_time2);

		while (fc_coarse_time2 <= fc_coarse_time1) {
			fc_coarse_time2 = mmio_read_32(rtc_ctrl_base + CVI_RTC_FC_COARSE_CAL);
			fc_coarse_time2 >>= 16;
			rtc_dbg("fc_coarse_time2 = 0x%x\n", fc_coarse_time2);
		}

		udelay(400);
		fc_coarse_value = mmio_read_32(rtc_ctrl_base + CVI_RTC_FC_COARSE_CAL);
		fc_coarse_value &= 0xFFFF;
		rtc_dbg("fc_coarse_value = 0x%x\n", fc_coarse_value);

		if (fc_coarse_value > 770) {
			analog_calib_value += offset;
			offset >>= 1;
			mmio_write_32(rtc_base + CVI_RTC_ANA_CALIB, analog_calib_value);
		} else if (fc_coarse_value < 755) {
			analog_calib_value -= offset;
			offset >>= 1;
			mmio_write_32(rtc_base + CVI_RTC_ANA_CALIB, analog_calib_value);
		} else {
			mmio_write_32(rtc_ctrl_base + CVI_RTC_FC_COARSE_EN, 0);
			rtc_dbg("RTC coarse calib done\n");
			break;
		}
		if (offset == 0) {
			rtc_err("RTC calib failed\n");
			break;
		}
		rtc_dbg("RTC_ANA_CALIB: 0x%x\n", analog_calib_value);
	}
}

static inline int64_t div_u64_rem(uint64_t dividend, uint32_t divisor, uint32_t *remainder)
{
	*remainder = dividend % divisor;
	return dividend / divisor;
}

void hal_cvi_rtc_32k_fine_value_calib(uintptr_t rtc_base)
{
	uint32_t fc_fine_time1 = 0;
	uint32_t fc_fine_time2 = 0;
	uint32_t fc_fine_value = 0;
	uint64_t freq;
	uint32_t sec_cnt;
	uint32_t frac_ext = 10000;
	uintptr_t rtc_ctrl_base = CVI_RTC_CTRL_BASE;
	uint32_t remainder;

	rtc_dbg("enter rtc_32k_fine_value_calib\n");

	mmio_write_32(rtc_ctrl_base + CVI_RTC_FC_FINE_EN, 1);

	fc_fine_time1 = mmio_read_32(rtc_ctrl_base + CVI_RTC_FC_FINE_CAL);
	fc_fine_time1 >>= 24;
	rtc_dbg("fc_fine_time1 = 0x%x\n", fc_fine_time1);

	while (fc_fine_time2 <= fc_fine_time1) {
		fc_fine_time2 = mmio_read_32(rtc_ctrl_base + CVI_RTC_FC_FINE_CAL);
		fc_fine_time2 >>= 24;
		rtc_dbg("fc_fine_time2 = 0x%x\n", fc_fine_time2);
	}

	fc_fine_value = mmio_read_32(rtc_ctrl_base + CVI_RTC_FC_FINE_CAL);
	fc_fine_value &= 0xFFFFFF;
	rtc_dbg("fc_fine_value = 0x%x\n", fc_fine_value);

	// Frequency = 256 / (RTC_FC_FINE_VALUE x 40ns)
	//freq = 256000000000 / 40;
	freq = 6400000000 * frac_ext;
	freq = div_u64_rem(freq, fc_fine_value, &remainder);
	rtc_dbg("freq = %lx\n", freq);

	sec_cnt = ((freq / frac_ext) << 8) + (((freq % frac_ext) * 256) / frac_ext & 0xFF);
	rtc_dbg("sec_cnt = 0x%x\n", sec_cnt);

	mmio_write_32(rtc_base + CVI_RTC_SEC_PULSE_GEN, sec_cnt);
	mmio_write_32(rtc_ctrl_base + CVI_RTC_FC_FINE_EN, 0);
}
#endif
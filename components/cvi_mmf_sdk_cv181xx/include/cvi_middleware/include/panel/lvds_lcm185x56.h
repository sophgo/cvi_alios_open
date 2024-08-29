#ifndef _LVDS_PARAM_LCM185X56_H_
#define _LVDS_PARAM_LCM185X56_H_

#include <cvi_comm_vo.h>

VO_LVDS_ATTR_S lvds_lcm185x56_cfg = {
	.lvds_vesa_mode = VO_LVDS_MODE_VESA,
	.out_bits = VO_LVDS_OUT_8BIT,
	.chn_num = 1,
	.data_big_endian = 0,
	.lane_id = {VO_LVDS_LANE_0, VO_LVDS_LANE_1, VO_LVDS_LANE_CLK, VO_LVDS_LANE_2, VO_LVDS_LANE_3},
	.lane_pn_swap = {true, true, true, true, true},
	.stSyncInfo = {
		.u16Hpw = 10,
		.u16Vbb = 88,
		.u16Vfb= 62,
		.u16Hact = 1280,
		.u16Vpw = 4,
		.u16Vbb = 23,
		.u16Vfb = 11,
		.u16Vact = 800,
		.bIhs = 0,
		.bIhs = 0,
		.u16FrameRate = 60,
	},
};

#endif // _LVDS_PARAM_LCM185X56_H_

#ifndef __Q03_CMOS_PARAM_H_
#define __Q03_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"
#include "q03_cmos_ex.h"

static const Q03_MODE_S g_astQ03_mode[Q03_MODE_NUM] = {
	[Q03_MODE_1296P30] = {
		.name = "1296p30",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2312,
				.u32Height = 1296,
			},
			.stWndRect = {
				.s32X = 4,
				.s32Y = 0,
				.u32Width = 2304,
				.u32Height = 1296,
			},
			.stMaxSize = {
				.u32Width = 2312,
				.u32Height = 1296,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 0.61, /* 1350 * 30 / 0xFFFF */
		.u32HtsDef = 3600,
		.u32VtsDef = 1350,
		.stExp[0] = {
			.u16Min = 1,
			.u16Max = 1350,
			.u16Def = 400,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u16Min = 1024,
			.u16Max = 15872,
			.u16Def = 1024,
			.u16Step = 1,
		},
		.stDgain[0] = {
			.u16Min = 1024,
			.u16Max = 1024,
			.u16Def = 1024,
			.u16Step = 1,
		},
	},
};

static ISP_CMOS_BLACK_LEVEL_S g_stIspBlcCalibratio = {
	.bUpdate = CVI_TRUE,
	.blcAttr = {
		.Enable = 1,
		.enOpType = OP_TYPE_AUTO,
		.stManual = {64, 64, 64, 64, 0, 0, 0, 0
		},
		.stAuto = {
			{64, 64, 64, 64, 64, 64, 64, 64, 64, /*8*/64, 64, 64, 64, 64, 64, 64},
			{64, 64, 64, 64, 64, 64, 64, 64, 64, /*8*/64, 64, 64, 64, 64, 64, 64},
			{64, 64, 64, 64, 64, 64, 64, 64, 64, /*8*/64, 64, 64, 64, 64, 64, 64},
			{64, 64, 64, 64, 64, 64, 64, 64, 64, /*8*/64, 64, 64, 64, 64, 64, 64},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		},
	},
};

struct combo_dev_attr_s q03_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_200M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {0, 1, 2, -1, -1},
		.pn_swap = {1, 1, 1, 0, 0},
		.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
	},
	.mclk = {
		.cam = 0,
		.freq = CAMPLL_FREQ_27M,
	},
	.devno = 0,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __Q03_CMOS_PARAM_H_ */

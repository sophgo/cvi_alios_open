#ifndef __SC2356_CMOS_PARAM_H_
#define __SC2356_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "cvi_type.h"
#include "cvi_sns_ctrl.h"
#include "sc2356_cmos_ex.h"

static const SC2356_MODE_S g_astSC2356_mode[SC2356_MODE_NUM] = {
	[SC2356_MODE_496X360P93] = {
		.name = "360p93",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 496,
				.u32Height = 360,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 448,
				.u32Height = 360,
			},
			.stMaxSize = {
				.u32Width = 496,
				.u32Height = 360,
			},
		},
		.f32MaxFps = 93,
		.f32MinFps = 1.141, /* 402 * 93 / 0x7FFF */
		.u32HtsDef = 1800,
		.u32VtsDef = 402,
		.stExp[0] = {
			.u16Min = 0,
			.u16Max = 402 - 4 ,// vts - 4
			.u16Def = 200,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u16Min = 1024,
			.u16Max = 64512,
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
	[SC2356_MODE_1600X1200P30] = {
		.name = "1600X1200P30",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1600,
				.u32Height = 1200,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1600,
				.u32Height = 1200,
			},
			.stMaxSize = {
				.u32Width = 1600,
				.u32Height = 1200,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 2.28, /* 402 * 93 / 0x7FFF */
		.u32HtsDef = 2560,
		.u32VtsDef = 1250,
		.stExp[0] = {
			.u16Min = 1,
			.u16Max = 1250 - 6,// vts - 4
			.u16Def = 100,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u16Min = 1024,
			.u16Max = 64512,
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
		.stManual = {260, 260, 260, 260, 0, 0, 0, 0
#ifdef ARCH_CV182X
		, 1093, 1093, 1093, 1093
#endif
		},
		.stAuto = {
			{260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260 },
			{260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260 },
			{260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260 },
			{260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260 },
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
#ifdef ARCH_CV182X
			{1083, 1083, 1083, 1083, 1083, 1083, 1083, 1083,
				/*8*/1083, 1083, 1083, 1083, 1083, 1083, 1083, 1083},
			{1083, 1083, 1083, 1083, 1083, 1083, 1083, 1083,
				/*8*/1083, 1083, 1083, 1083, 1083, 1083, 1083, 1083},
			{1083, 1083, 1083, 1083, 1083, 1083, 1083, 1083,
				/*8*/1083, 1083, 1083, 1083, 1083, 1083, 1083, 1083},
			{1083, 1083, 1083, 1083, 1083, 1083, 1083, 1083,
				/*8*/1083, 1083, 1083, 1083, 1083, 1083, 1083, 1083},
#endif
		},
	},
};

struct combo_dev_attr_s sc2356_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_200M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {0, 1, -1, -1, -1},
		.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
		.dphy = {
			.enable = 1,
			.hs_settle = 8,
		},
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


#endif /* __SC2356_CMOS_PARAM_H_ */

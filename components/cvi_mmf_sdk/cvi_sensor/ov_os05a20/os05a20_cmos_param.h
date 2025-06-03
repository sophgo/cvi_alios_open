#ifndef __OS05A20_CMOS_PARAM_H_
#define __OS05A20_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"
#include "os05a20_cmos_ex.h"

static const OS05A20_MODE_S g_astOs05a20_mode[OS05A20_MODE_NUM] = {
	[OS05A20_MODE_1944P15_12BIT] = {
		.name = "1944p15_12bit",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2688,
				.u32Height = 1944,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2688,
				.u32Height = 1944,
			},
			.stMaxSize = {
				.u32Width = 2688,
				.u32Height = 1944,
			},
		},
		.f32MaxFps = 15,
		.f32MinFps = 0.78, /* 3386 * 15 / 0xFFFF */
		.u32HtsDef = 2362,
		.u32VtsDef = 3386,
		.stExp[0] = {
			.u16Min = 4,
			.u16Max = 3386 - 8,
			.u16Def = 500,  //可以任意值
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 15872,  //15.5倍
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 1024,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
};

static ISP_CMOS_BLACK_LEVEL_S g_stIspBlcCalibratio = {
	.bUpdate = CVI_TRUE,
	.blcAttr = {
		.Enable = 1,
		.enOpType = OP_TYPE_AUTO,
		.stManual = {256, 256, 256, 256, 0, 0, 0, 0
#ifdef ARCH_CV182X
		, 1057, 1057, 1057, 1057
#endif
		},
		.stAuto = {
			{256, 256, 256, 256, 256, 256, 256, 256, /*8*/256, 256, 256, 256, 256, 256, 256, 256},
			{256, 256, 256, 256, 256, 256, 256, 256, /*8*/256, 256, 256, 256, 256, 256, 256, 256},
			{256, 256, 256, 256, 256, 256, 256, 256, /*8*/256, 256, 256, 256, 256, 256, 256, 256},
			{256, 256, 256, 256, 256, 256, 256, 256, /*8*/256, 256, 256, 256, 256, 256, 256, 256},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
#ifdef ARCH_CV182X
			{1057, 1057, 1057, 1057, 1057, 1057, 1057, 1057,
				/*8*/1057, 1057, 1057, 1057, 1057, 1057, 1057, 1057},
			{1057, 1057, 1057, 1057, 1057, 1057, 1057, 1057,
				/*8*/1057, 1057, 1057, 1057, 1057, 1057, 1057, 1057},
			{1057, 1057, 1057, 1057, 1057, 1057, 1057, 1057,
				/*8*/1057, 1057, 1057, 1057, 1057, 1057, 1057, 1057},
			{1057, 1057, 1057, 1057, 1057, 1057, 1057, 1057,
				/*8*/1057, 1057, 1057, 1057, 1057, 1057, 1057, 1057},
#endif
		},
	},
};

struct combo_dev_attr_s os05a20_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_400M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_12BIT,
		.lane_id = {0, 1, 2, -1, -1},
		.pn_swap = {0, 0, 0, 0, 0},
		.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
		.dphy = {
			.enable = 1,
			.hs_settle = 8,
		},
	},
	.mclk = {
		.cam = 0,
		.freq = CAMPLL_FREQ_24M,
	},
	.devno = 0,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __OS05A20_CMOS_PARAM_H_ */

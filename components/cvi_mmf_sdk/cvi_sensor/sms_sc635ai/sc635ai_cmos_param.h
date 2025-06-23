#ifndef __SC635AI_CMOS_PARAM_H_
#define __SC635AI_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"
#include "sc635ai_cmos_ex.h"

static const SC635AI_MODE_S g_astSC635AI_mode[SC635AI_MODE_NUM] = {
	[SC635AI_MODE_1800P60] = {
		.name = "1800p60",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 3200,
				.u32Height = 1800,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 3200,
				.u32Height = 1800,
			},
			.stMaxSize = {
				.u32Width = 3200,
				.u32Height = 1800,
			},
		},
		.f32MaxFps = 60,
		.f32MinFps = 1.7, /* 1875 * 60 / 0xFFF0*/
		.u32HtsDef = 3840,
		.u32VtsDef = 1875,
		.stExp[0] = {
			.u32Min = 2,
			.u32Max = 1875 - 8, //vts - 8
			.u32Def = 1867, //60fps
			.u32Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 85800,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16128,
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
		.stManual = {260, 260, 260, 260, 0, 0, 0, 0
#ifdef ARCH_CV182X
			, 1093, 1093, 1093, 1093
#endif
		},
		.stAuto = {
			{260, 260, 260, 260, 260, 260, 260, 260, /*8*/260, 260, 260, 260, 260, 260, 260, 260},
			{260, 260, 260, 260, 260, 260, 260, 260, /*8*/260, 260, 260, 260, 260, 260, 260, 260},
			{260, 260, 260, 260, 260, 260, 260, 260, /*8*/260, 260, 260, 260, 260, 260, 260, 260},
			{260, 260, 260, 260, 260, 260, 260, 260, /*8*/260, 260, 260, 260, 260, 260, 260, 260},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
#ifdef ARCH_CV182X
			{1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093,
				/*8*/1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093},
			{1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093,
				/*8*/1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093},
			{1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093,
				/*8*/1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093},
			{1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093,
				/*8*/1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093},
#endif
		},
	},
};

struct combo_dev_attr_s sc635ai_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_600M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {0, 1, 2, -1, -1},
		.pn_swap = {1, 1, 1, 1, 1},
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


#endif /* __SC635AI_CMOS_PARAM_H_ */

#ifndef __BF314A_CMOS_PARAM_H_
#define __BF314A_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef ARCH_CV182X
#include "cvi_vip_cif_uapi.h"
#else
#include "cif_uapi.h"
#endif
#include "cvi_type.h"
#include "cvi_sns_ctrl.h"
#include "bf314a_cmos_ex.h"

static const BF314A_MODE_S g_astBf314a_mode[BF314A_MODE_NUM] = {
	[BF314A_MODE_1280X720P30] = {
		.name = "1280X720P30",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1288,
				.u32Height = 728,
			},
			.stWndRect = {
				.s32X = 4,
				.s32Y = 4,
				.u32Width = 1280,
				.u32Height = 720,
			},
			.stMaxSize = {
				.u32Width = 1288,
				.u32Height = 728,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 0.34, /* vts * 30 / 0xFFFF */
		.u32HtsDef = 1600,
		.u32VtsDef = 750,
		.stExp[0] = {
			.u16Min = 1,
			.u16Max = 750,
			.u16Def = 450,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 16384,
			.u32Def = 1024,	//
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16384,
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
		.stManual = {56, 56, 56, 56, 0, 0, 0, 0
#ifdef ARCH_CV182X
		, 1093, 1093, 1093, 1093
#endif
		},
		.stAuto = {
			{56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56 },
			{56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56 },
			{56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56 },
			{56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56 },
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
#ifdef ARCH_CV182X
			{1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093,
				1093, 1093},
			{1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093,
				1093, 1093},
			{1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093,
				1093, 1093},
			{1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093,
				1093, 1093},
#endif
		},
	},
};

struct combo_dev_attr_s bf314a_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_200M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {3, 4, -1, -1, -1},
		.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
		.dphy = {
			.enable = 1,
			.hs_settle = 8,
		}
	},
	.mclk = {
		.cam = 1,
		.freq = CAMPLL_FREQ_24M,
	},
	.devno = 0,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __BF314A_CMOS_PARAM_H_ */


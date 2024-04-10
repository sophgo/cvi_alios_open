#ifndef __C2599_CMOS_PARAM_H_
#define __C2599_CMOS_PARAM_H_

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
#include "c2599_cmos_ex.h"

static const C2599_MODE_S g_astC2599_mode[C2599_MODE_NUM] = {
	[C2599_MODE_1200P30] = {
		.name = "1200p30",
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
		.f32MinFps = 1, /* 1472 * 30 / 0xFFFF*/
		.u32HtsDef = 2432,
		.u32VtsDef = 1232,
		.stExp[0] = {
			.u32Min = 4,
			.u32Max = 1224, //vts - 8
			.u32Def = 400,
			.u32Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 8192,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 7936,
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
		.stManual = {252, 252, 252, 252, 0, 0, 0, 0
#ifdef ARCH_CV182X
			, 1093, 1093, 1093, 1093
#endif
		},
		.stAuto = {
			{252, 252, 252, 252, 252, 252, 252, 252, /*8*/252, 252, 252, 252, 252, 252, 252, 252},
			{252, 252, 252, 252, 252, 252, 252, 252, /*8*/252, 252, 252, 252, 252, 252, 252, 252},
			{252, 252, 252, 252, 252, 252, 252, 252, /*8*/252, 252, 252, 252, 252, 252, 252, 252},
			{252, 252, 252, 252, 252, 252, 252, 252, /*8*/252, 252, 252, 252, 252, 252, 252, 252},
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

struct combo_dev_attr_s c2599_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_200M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {3, 4, -1, -1, -1},
		.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
		.dphy = {
			.enable = 1,
			.hs_settle = 8,      //ecc error 时需要修改hs-settle
		}
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


#endif /* __C2599_CMOS_PARAM_H_ */

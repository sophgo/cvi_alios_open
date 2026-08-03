#ifndef __MIS5001_CMOS_PARAM_H_
#define __MIS5001_CMOS_PARAM_H_

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
#include "mis5001_cmos_ex.h"

static const MIS5001_MODE_S g_astMIS5001_mode[MIS5001_MODE_NUM] = {
	[MIS5001_MODE_1080P15] = {
		.name = "1080p15",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1920,
				.u32Height = 1920,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1920,
				.u32Height = 1920,
			},
			.stMaxSize = {
				.u32Width = 1920,
				.u32Height = 1920,
			},
		},
		.f32MaxFps = 15,
		.f32MinFps = 1, /* 1500 * 30 / 0x7FFF*/
		.u32HtsDef = 3000,
		.u32VtsDef = 2026,
		.stExp[0] = {
			.u32Min = 2,
			.u32Max = 2026 - 4,
			.u32Def = 128,
			.u32Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 16128,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16320,
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
		.stManual = {128, 128, 128, 128, 0, 0, 0, 0
#ifdef ARCH_CV182X
			, 1040, 1040, 1040, 1040
#endif
		},
		.stAuto = {
			{128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128},
			{128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128},
			{128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128},
			{128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128},
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

struct combo_dev_attr_s mis5001_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_200M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {1, 2, 0, -1, -1},
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


#endif /* __MIS5001_CMOS_PARAM_H_ */

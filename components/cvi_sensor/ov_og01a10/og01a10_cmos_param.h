#ifndef __OG01A10_CMOS_PARAM_H_
#define __OG01A10_CMOS_PARAM_H_

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
#include "og01a10_cmos_ex.h"

static const OG01A10_MODE_S g_stOg01a10_mode = {
	.name = "1280X1024P30",
	.stImg = {
		.stSnsSize = {
			.u32Width = 1280,
			.u32Height = 1024,
		},
		.stWndRect = {
			.s32X = 0,
			.s32Y = 0,
			.u32Width = 1280,
			.u32Height = 1024,
		},
		.stMaxSize = {
			.u32Width = 1280,
			.u32Height = 1024,
		},
	},
	.f32MaxFps = 30,
	.f32MinFps = 2.07, /* 1125 * 30 / 0x3FFF */
	.u32HtsDef = 1780,
	.u32VtsDef = 2352,
	.stExp = {
		.u16Min = 1,
		.u16Max = 1125,
		.u16Def = 400,
		.u16Step = 1,
	},
	.stAgain = {
		.u32Min = 1024,
		.u32Max = 15872,
		.u32Def = 1024,
		.u32Step = 1,
	},
	.stDgain = {
		.u32Min = 1024,
		.u32Max = 15872,
		.u32Def = 1024,
		.u32Step = 1,
	},
};

static ISP_CMOS_BLACK_LEVEL_S g_stIspBlcCalibratio = {
	.bUpdate = CVI_TRUE,
	.blcAttr = {
		.Enable = 1,
		.enOpType = OP_TYPE_MANUAL,
		.stManual = {256, 256, 256, 256, 1092, 1092, 1092, 1092},
		.stAuto = {
			{256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256 },
			{256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256 },
			{256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256 },
			{256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256 },
			{1092, 1092, 1092, 1092, 1092 ,1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092},
			{1092, 1092, 1092, 1092, 1092 ,1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092},
			{1092, 1092, 1092, 1092, 1092 ,1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092},
			{1092, 1092, 1092, 1092, 1092 ,1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092},
		},
	},
};

struct combo_dev_attr_s og01a10_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_500M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {5, 4, 3, -1, -1},
		.pn_swap = {1, 1, 1, 0, 0},
		.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
		.dphy = {
			.enable = 1,
			.hs_settle = 12,
		},
	},
	.mclk = {
		.cam = 1,
		.freq = CAMPLL_FREQ_25M,

	},

	.devno = 1,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __OG01A10_CMOS_PARAM_H_ */
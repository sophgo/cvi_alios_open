#ifndef __OV02B10_CMOS_PARAM_H_
#define __OV02B10_CMOS_PARAM_H_

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
#include "ov02b10_cmos_ex.h"

static const OV02B10_MODE_S g_astOv02b10_mode[OV02B10_MODE_NUM] = {
	[OV02B10_MODE_1600X1200P30] = {
		.name = "1600x1200p30",
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
		.f32MinFps = 0.711, /* 0x4e2 * 30 / 0xFFFF */
		.u32HtsDef = 448*4,
		.u32VtsDef = 1221,
		.stExp[0] = {
			.u16Min = 4,
			.u16Max = 1221 - 7,
			.u16Def = 538,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 15872,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 5120,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[OV02B10_MODE_800X600P60] = {
		.name = "800x600p60",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 800,
				.u32Height = 600,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 800,
				.u32Height = 600,
			},
			.stMaxSize = {
				.u32Width = 800,
				.u32Height = 600,
			},
		},

		.f32MaxFps = 30,
		.f32MinFps = 0.711, /* 0x4e2 * 30 / 0xFFFF */
		.u32HtsDef = 448*4,
		.u32VtsDef = 1104,
		.stExp[0] = {
			.u16Min = 4,
			.u16Max = 545 - 7,
			.u16Def = 538,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 15872,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 5120,
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
			, 1039, 1039, 1039, 1039
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
			{1039, 1039, 1039, 1039, 1039, 1039, 1039, 1039,
				/*8*/1039, 1039, 1039, 1039, 1204, 1039, 1039, 1039},
			{1039, 1039, 1039, 1039, 1039, 1039, 1039, 1039,
				/*8*/1039, 1039, 1039, 1039, 1204, 1039, 1039, 1039},
			{1039, 1039, 1039, 1039, 1039, 1039, 1039, 1039,
				/*8*/1039, 1039, 1039, 1039, 1204, 1039, 1039, 1039},
			{1039, 1039, 1039, 1039, 1039, 1039, 1039, 1039,
				/*8*/1039, 1039, 1039, 1039, 1204, 1039, 1039, 1039},
#endif
		},
	},
};


struct combo_dev_attr_s ov02b10_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_200M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_8BIT,
		.lane_id = {2, 3, -1, -1, -1},
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


#endif /* __OV02B10_CMOS_PARAM_H_ */


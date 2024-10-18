#ifndef __OV9732_CMOS_PARAM_H_
#define __OV9732_CMOS_PARAM_H_

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
#include "ov9732_cmos_ex.h"

static const OV9732_MODE_S g_astOv9732_mode[OV9732_MODE_NUM] = {
	[OV9732_MODE_1280X720P30] = {
		.name = "1280x720p30",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1280,
				.u32Height = 720,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1280,
				.u32Height = 720,
			},
			.stMaxSize = {
				.u32Width = 1280,
				.u32Height = 720,
			},
		},

		.f32MaxFps = 30,
		.f32MinFps = 0.572, /* 0x4e2 * 30 / 0xFFFF */
		.u32HtsDef = 1500,
		.u32VtsDef = 800,
		.stExp[0] = {
			.u16Min = 4,
			.u16Max = 800 - 4,
			.u16Def = 512,
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
			.u32Max = 2752,
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
		.stManual = {68, 68, 68, 68, 0, 0, 0, 0
#ifdef ARCH_CV182X
			, 1039, 1039, 1039, 1039
#endif
		},
		.stAuto = {
			{68, 68, 68, 68, 68, 68, 68, 68, /*8*/68, 68, 68, 68, 68, 68, 68, 68},
			{68, 68, 68, 68, 68, 68, 68, 68, /*8*/68, 68, 68, 68, 68, 68, 68, 68},
			{68, 68, 68, 68, 68, 68, 68, 68, /*8*/68, 68, 68, 68, 68, 68, 68, 68},
			{68, 68, 68, 68, 68, 68, 68, 68, /*8*/68, 68, 68, 68, 68, 68, 68, 68},
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


struct combo_dev_attr_s ov9732_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_400M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
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


#endif /* __OV5647_CMOS_PARAM_H_ */


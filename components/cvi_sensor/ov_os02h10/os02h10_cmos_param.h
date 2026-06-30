#ifndef __OS02H10_CMOS_PARAM_H_
#define __OS02H10_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "cvi_comm_cif.h"
#include "cvi_type.h"
#include "cvi_sns_ctrl.h"
#include "os02h10_cmos_ex.h"

static const OS02H10_MODE_S g_astOs02h10_mode[OS02H10_MODE_NUM] = {
	[OS02H10_MODE_1920X1080P30] = {
		.name = "1920x1080p30",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1920,
				.u32Height = 1080,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1920,
				.u32Height = 1080,
			},
			.stMaxSize = {
				.u32Width = 1920,
				.u32Height = 1080,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 0.711,
		.u32HtsDef = 1248, //P1:0xda/0xdb (0x04e0)
		.u32VtsDef = 2250, //P1:0x4e/0x4f (0x08ca)
		.stExp[0] = {
			.u16Min = 1,
			.u16Max = 2250 - 2,
			.u16Def = 0x04, //P1:0x03/0x04 (0x04)
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
			.u32Max = 32752,
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
		.stManual = {252, 252, 252, 252, 252, 252, 252, 252
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
		},
	},
};


struct combo_dev_attr_s os02h10_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_400M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {0, 1, 2, -1, -1},
		.pn_swap = {0, 0, 0, 0, 0},
		.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
		.dphy = {
			.enable = 1,
			.hs_settle = 10,
		},
	},
	.mclk = {
		.cam = 0,
#if OS02H10_CHIP_VERSION_1C
		.freq = CAMPLL_FREQ_24M,
#else
		.freq = CAMPLL_FREQ_27M,
#endif
	},
	.devno = 0,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __OS02H10_CMOS_PARAM_H_ */
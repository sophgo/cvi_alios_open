#ifndef __OV9282_CMOS_PARAM_H_
#define __OV9282_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"
#include "ov9282_cmos_ex.h"

static const OV9282_MODE_S g_astOv9282_mode[OV9282_MODE_NUM] = {
	[OV9282_MODE_800P15_2L_MASTER_10BIT] = {
		.name = "800p15_master_10bit",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1280,
				.u32Height = 800,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1280,
				.u32Height = 800,
			},
			.stMaxSize = {
				.u32Width = 1280,
				.u32Height = 800,
			},
		},
		.f32MaxFps = 15,
		.f32MinFps = 1.68, /* 7324 * 15 / 0xFFFF */
		.u32HtsDef = 1464,
		.u32VtsDef = 7324,
		.stExp[0] = {
			.u16Min = 1,
			.u16Max = 7324 - 25,
			.u16Def = 681,
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
			.u32Max = 1024,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[OV9282_MODE_800P15_2L_SLAVE_10BIT] = {
		.name = "800p15_slave_10bit",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1280,
				.u32Height = 800,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1280,
				.u32Height = 800,
			},
			.stMaxSize = {
				.u32Width = 1280,
				.u32Height = 800,
			},
		},
		.f32MaxFps = 15,
		.f32MinFps = 1.68, /* 7324 * 15 / 0xFFFF */
		.u32HtsDef = 1464,
		.u32VtsDef = 7324,
		.stExp[0] = {
			.u16Min = 1,
			.u16Max = 7324 - 25,
			.u16Def = 681,
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
		.stManual = {252, 252, 252, 252, 0, 0, 0, 0
#ifdef ARCH_CV182X
		, 1057, 1057, 1057, 1057
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

struct combo_dev_attr_s ov9282_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_200M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {6, 7, 8, -1, -1},
		.pn_swap = {0, 0, 0, 0, 0},
		.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
		.dphy = {
			.enable = 1,
			.hs_settle = 14,
		},
	},
	.mclk = {
		.cam = 2,
		.freq = CAMPLL_FREQ_24M,
	},
	.devno = 0,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __OV9282_CMOS_PARAM_H_ */

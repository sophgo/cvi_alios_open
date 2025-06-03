#ifndef __OS04E10_CMOS_PARAM_H_
#define __OS04E10_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"
#include "os04e10_cmos_ex.h"

static const OS04E10_MODE_S g_astOs04e10_mode[OS04E10_MODE_NUM] = {
	[OS04E10_MODE_2048X2048_30_12BIT] = {
		.name = "2048x2048p30",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2048,
				.u32Height = 2048,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2048,
				.u32Height = 2048,
			},
			.stMaxSize = {
				.u32Width = 2048,
				.u32Height = 2048,
			},
		},
		.f32MaxFps = 33,
		.f32MinFps = 0.73, /* 0x626 * 30 / 0xFFFF */
		.u32HtsDef = 0x651,
		.u32VtsDef = 0x874,
		.stExp[0] = {
			.u16Min = 2,
			.u16Max = 2164 - 8,
			.u16Def = 500,
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
			.u32Max = 16384,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[OS04E10_MODE_2048X2048_30_10BIT_2L_MASTER] = {
		.name = "2048x2048p30_2L",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2048,
				.u32Height = 2048,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2048,
				.u32Height = 2048,
			},
			.stMaxSize = {
				.u32Width = 2048,
				.u32Height = 2048,
			},
		},
		.f32MaxFps = 31,
		.f32MinFps = 0.73, /* 0x626 * 30 / 0xFFFF */
		.u32HtsDef = 0x67e,
		.u32VtsDef = 0x89b,
		.stExp[0] = {
			.u16Min = 2,
			.u16Max = 2105 - 8,
			.u16Def = 500,
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
			.u32Max = 16384,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[OS04E10_MODE_2048X2048_30_10BIT_2L_SLAVE] = {
		.name = "2048x2048p30_2L_SLAVE",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2048,
				.u32Height = 2048,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2048,
				.u32Height = 2048,
			},
			.stMaxSize = {
				.u32Width = 2048,
				.u32Height = 2048,
			},
		},
		.f32MaxFps = 31,
		.f32MinFps = 0.73, /* 0x626 * 30 / 0xFFFF */
		.u32HtsDef = 0x67e,
		.u32VtsDef = 0x89b,
		.stExp[0] = {
			.u16Min = 2,
			.u16Max = 2105 - 8,
			.u16Def = 500,
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
			.u32Max = 16384,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[OS04E10_MODE_2048X2048_30_WDR] = {
		.name = "2048x2048p30wdr",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2048,
				.u32Height = 2048,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2048,
				.u32Height = 2048,
			},
			.stMaxSize = {
				.u32Width = 2048,
				.u32Height = 2048,
			},
		},
		.astImg[1] = {
			.stSnsSize = {
				.u32Width = 2048,
				.u32Height = 2048,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2048,
				.u32Height = 2048,
			},
			.stMaxSize = {
				.u32Width = 2048,
				.u32Height = 2048,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 0.77, /* 1682 * 30 / 0xFFFF */
		.u32HtsDef = 0x31a,
		.u32VtsDef = 0x89b,
		.u16L2sOffset = 4,
		.u16TopBoundary = 24,
		.u16BotBoundary = 244,
		.stExp[0] = {
			.u16Min = 8,
			.u16Max = 88,
			.u16Def = 88,
			.u16Step = 1,
		},
		.stExp[1] = {
			.u16Min = 8,
			.u16Max = 0x486 - 4 - 88,
			.u16Def = 500,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 15872,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stAgain[1] = {
			.u32Min = 1024,
			.u32Max = 15872,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16384,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[1] = {
			.u32Min = 1024,
			.u32Max = 16384,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.u32L2S_offset = 4,
		.u32IspResTime = 50,	/* about 1ms * line rate */
		.u32HdrMargin = 40,	/* black_line + zero_line + ISP_offset * 2 */
	},
	[OS04E10_MODE_2048X2048_30_WDR_2L_MASTER] = {
		.name = "2048x2048p30wdr_2L",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2048,
				.u32Height = 2048,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2048,
				.u32Height = 2048,
			},
			.stMaxSize = {
				.u32Width = 2048,
				.u32Height = 2048,
			},
		},
		.astImg[1] = {
			.stSnsSize = {
				.u32Width = 2048,
				.u32Height = 2048,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2048,
				.u32Height = 2048,
			},
			.stMaxSize = {
				.u32Width = 2048,
				.u32Height = 2048,
			},
		},
		.f32MaxFps = 21,
		.f32MinFps = 0.77, /* 1682 * 30 / 0xFFFF */
		.u32HtsDef = 0x4d1,
		.u32VtsDef = 0x850,
		.u16L2sOffset = 4,
		.u16TopBoundary = 24,
		.u16BotBoundary = 244,
		.stExp[0] = {
			.u16Min = 8,
			.u16Max = 88,
			.u16Def = 88,
			.u16Step = 1,
		},
		.stExp[1] = {
			.u16Min = 8,
			.u16Max = 0x486 - 4 - 88,
			.u16Def = 500,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 15872,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stAgain[1] = {
			.u32Min = 1024,
			.u32Max = 15872,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16384,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[1] = {
			.u32Min = 1024,
			.u32Max = 16384,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.u32L2S_offset = 4,
		.u32IspResTime = 50,	/* about 1ms * line rate */
		.u32HdrMargin = 40,	/* black_line + zero_line + ISP_offset * 2 */
	},
	[OS04E10_MODE_2048X2048_30_WDR_2L_SLAVE] = {
		.name = "2048x2048p30wdr_2L_SALVE",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2048,
				.u32Height = 2048,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2048,
				.u32Height = 2048,
			},
			.stMaxSize = {
				.u32Width = 2048,
				.u32Height = 2048,
			},
		},
		.astImg[1] = {
			.stSnsSize = {
				.u32Width = 2048,
				.u32Height = 2048,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2048,
				.u32Height = 2048,
			},
			.stMaxSize = {
				.u32Width = 2048,
				.u32Height = 2048,
			},
		},
		.f32MaxFps = 21,
		.f32MinFps = 0.77, /* 1682 * 30 / 0xFFFF */
		.u32HtsDef = 0x4d1,
		.u32VtsDef = 0x850,
		.u16L2sOffset = 4,
		.u16TopBoundary = 24,
		.u16BotBoundary = 244,
		.stExp[0] = {
			.u16Min = 8,
			.u16Max = 88,
			.u16Def = 88,
			.u16Step = 1,
		},
		.stExp[1] = {
			.u16Min = 8,
			.u16Max = 0x486 - 4 - 88,
			.u16Def = 500,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 15872,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stAgain[1] = {
			.u32Min = 1024,
			.u32Max = 15872,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16384,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[1] = {
			.u32Min = 1024,
			.u32Max = 16384,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.u32L2S_offset = 4,
		.u32IspResTime = 50,	/* about 1ms * line rate */
		.u32HdrMargin = 40,	/* black_line + zero_line + ISP_offset * 2 */
	},
};

static ISP_CMOS_BLACK_LEVEL_S g_stIspBlcCalibratio = {
	.bUpdate = CVI_TRUE,
	.blcAttr = {
		.Enable = 1,
		.enOpType = OP_TYPE_AUTO,
		.stManual = {128, 128, 128, 128, 0, 0, 0, 0
		},
		.stAuto = {
			{128, 128, 128, 128, 128, 128, 128, 128, /*8*/128, 128, 128, 128, 128, 128, 128, 128},
			{128, 128, 128, 128, 128, 128, 128, 128, /*8*/128, 128, 128, 128, 128, 128, 128, 128},
			{128, 128, 128, 128, 128, 128, 128, 128, /*8*/128, 128, 128, 128, 128, 128, 128, 128},
			{128, 128, 128, 128, 128, 128, 128, 128, /*8*/128, 128, 128, 128, 128, 128, 128, 128},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		},
	},
};

static ISP_CMOS_BLACK_LEVEL_S g_stIspBlcCalibratio10Bit = {
	.bUpdate = CVI_TRUE,
	.blcAttr = {
		.Enable = 1,
		.enOpType = OP_TYPE_AUTO,
		.stManual = {256, 256, 256, 256, 0, 0, 0, 0
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
		},
	},
};

struct combo_dev_attr_s os04e10_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_400M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {0, 1, 2, 3, 4},
		.wdr_mode = CVI_MIPI_WDR_MODE_VC,
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


#endif /* __OS04E10_CMOS_PARAM_H_ */

#ifndef __TP9950_CMOS_PARAM_H_
#define __TP9950_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "cvi_comm_cif.h"
#include "cvi_type.h"
#include "cvi_sns_ctrl.h"
#include "tp9950_cmos_ex.h"

static const TP9950_MODE_S g_astTP9950_mode[TP9950_MODE_NUM] = {
	[TP9950_MODE_1080P_30P] = {
		.name = "1080p30",
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
		.f32MinFps = 0.119,
		.u32HtsDef = 2200,
		.u32VtsDef = 1125,
		.stExp[0] = {
			.u16Min = 1,
			.u16Max = 1120,
			.u16Def = 400,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u16Min = 1024,
			.u16Max = 15872,
			.u16Def = 1024,
			.u16Step = 1,
		},
		.stDgain[0] = {
			.u16Min = 1024,
			.u16Max = 4096,
			.u16Def = 1024,
			.u16Step = 1,
		},
		.u8DgainReg = 0x50,
	},
	[TP9950_MODE_1080P_25P] = {
		.name = "1080p25",
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
		.f32MaxFps = 25,
		.f32MinFps = 0.119,
		.u32HtsDef = 2640,
		.u32VtsDef = 1125,
		.stExp[0] = {
			.u16Min = 1,
			.u16Max = 1120,
			.u16Def = 400,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u16Min = 1024,
			.u16Max = 15872,
			.u16Def = 1024,
			.u16Step = 1,
		},
		.stDgain[0] = {
			.u16Min = 1024,
			.u16Max = 4096,
			.u16Def = 1024,
			.u16Step = 1,
		},
		.u8DgainReg = 0x50,
	},
	[TP9950_MODE_720P_30P] = {
		.name = "720p30",
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
		.f32MinFps = 0.119,
		.u32HtsDef = 3300,
		.u32VtsDef = 750,
		.stExp[0] = {
			.u16Min = 1,
			.u16Max = 1120,
			.u16Def = 400,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u16Min = 1024,
			.u16Max = 15872,
			.u16Def = 1024,
			.u16Step = 1,
		},
		.stDgain[0] = {
			.u16Min = 1024,
			.u16Max = 4096,
			.u16Def = 1024,
			.u16Step = 1,
		},
		.u8DgainReg = 0x50,
	},
	[TP9950_MODE_720P_25P] = {
		.name = "720p25",
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
		.f32MaxFps = 25,
		.f32MinFps = 0.119,
		.u32HtsDef = 3960,
		.u32VtsDef = 750,
		.stExp[0] = {
			.u16Min = 1,
			.u16Max = 1120,
			.u16Def = 400,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u16Min = 1024,
			.u16Max = 15872,
			.u16Def = 1024,
			.u16Step = 1,
		},
		.stDgain[0] = {
			.u16Min = 1024,
			.u16Max = 4096,
			.u16Def = 1024,
			.u16Step = 1,
		},
		.u8DgainReg = 0x50,
	},
};

struct combo_dev_attr_s tp9950_rx_attr = {
	.input_mode = INPUT_MODE_BT656_9B,
	.mac_clk = RX_MAC_CLK_200M,
	.ttl_attr = {
		.vi = TTL_VI_SRC_VI1,
		.func = {
			-1, -1, -1, -1,
			2, 3, 4, 5,
			6, 7, 8, 9,
			-1, -1, -1, -1,
			-1, -1, -1, -1,
		},
	},
	.mclk = {
		.cam = 2,
		.freq = CAMPLL_FREQ_27M,
	},
	.devno = 0,
};



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __TP9950_CMOS_PARAM_H_ */

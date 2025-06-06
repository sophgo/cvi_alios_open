#ifndef __SC1346_1L_CMOS_PARAM_H_
#define __SC1346_1L_CMOS_PARAM_H_

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
#include "sc1346_1L_cmos_ex.h"

static const SC1346_1L_MODE_S g_astSC1346_1L_mode[SC1346_1L_MODE_NUM] = {
	[SC1346_1L_MODE_720P30] = {
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
		.f32MinFps = 1.37, /* 750 * 30 / 0x3FFF*/
		.u32HtsDef = 1280,
		.u32VtsDef = 750,
		.stExp[0] = {
			.u16Min = 2,
			.u16Max = 750 - 6,
			.u16Def = 100,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u16Min = 1024,
			.u16Max = 32768,
			.u16Def = 1024,
			.u16Step = 1,
		},
		.stDgain[0] = {
			.u16Min = 1024,
			.u16Max = 4032,
			.u16Def = 1024,
			.u16Step = 1,
		},
	},
	[SC1346_1L_MODE_720P30_WDR] = {
		.name = "720p30wdr",
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
		.astImg[1] = {
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
		.f32MinFps = 1.37, /* 750 * 30 / 0x3FFF*/
		.u32HtsDef = 1280,
		.u32VtsDef = 750,
		.u16SexpMaxReg = 0x13E,
	},
	[SC1346_1L_MODE_720P60] = {
		.name = "720p60",
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
		.f32MaxFps = 60,
		.f32MinFps = 2.74, /* 750 * 60 / 0x3FFF*/
		.u32HtsDef = 1280,
		.u32VtsDef = 750,
		.stExp[0] = {
			.u16Min = 2,
			.u16Max = 750 - 6,
			.u16Def = 100,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u16Min = 1024,
			.u16Max = 32768,
			.u16Def = 1024,
			.u16Step = 1,
		},
		.stDgain[0] = {
			.u16Min = 1024,
			.u16Max = 4032,
			.u16Def = 1024,
			.u16Step = 1,
		},
	},
	[SC1346_1L_MODE_720P60_WDR] = {
		.name = "720p60wdr",
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
		.astImg[1] = {
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
		.f32MaxFps = 60,
		.f32MinFps = 1.37, /* 750 * 30 / 0x3FFF*/
		.u32HtsDef = 1280,
		.u32VtsDef = 750,
		.u16SexpMaxReg = 0x13E,
	},
	[SC1346_1L_MODE_600P30] = {
		.name = "600p30",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1024,
				.u32Height = 600,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1024,
				.u32Height = 600,
			},
			.stMaxSize = {
				.u32Width = 1024,
				.u32Height = 600,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 1.37, /* 750 * 30 / 0x3FFF*/
		.u32HtsDef = 1280,
		.u32VtsDef = 750,
		.stExp[0] = {
			.u16Min = 2,
			.u16Max = 750 - 6,
			.u16Def = 100,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u16Min = 1024,
			.u16Max = 32768,
			.u16Def = 1024,
			.u16Step = 1,
		},
		.stDgain[0] = {
			.u16Min = 1024,
			.u16Max = 4032,
			.u16Def = 1024,
			.u16Step = 1,
		},
	},
	[SC1346_1L_MODE_600P30_WDR] = {
		.name = "720p30wdr",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1024,
				.u32Height = 600,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1024,
				.u32Height = 600,
			},
			.stMaxSize = {
				.u32Width = 1024,
				.u32Height = 600,
			},
		},
		.astImg[1] = {
			.stSnsSize = {
				.u32Width = 1024,
				.u32Height = 600,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1024,
				.u32Height = 600,
			},
			.stMaxSize = {
				.u32Width = 1024,
				.u32Height = 600,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 1.37, /* 750 * 30 / 0x3FFF*/
		.u32HtsDef = 1280,
		.u32VtsDef = 750,
		.u16SexpMaxReg = 0x13E,
	},
	[SC1346_1L_MODE_600P60] = {
		.name = "600p60",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1024,
				.u32Height = 600,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1024,
				.u32Height = 600,
			},
			.stMaxSize = {
				.u32Width = 1024,
				.u32Height = 600,
			},
		},
		.f32MaxFps = 60,
		.f32MinFps = 2.74, /* 750 * 60 / 0x3FFF*/
		.u32HtsDef = 1280,
		.u32VtsDef = 750,
		.stExp[0] = {
			.u16Min = 2,
			.u16Max = 750 - 6,
			.u16Def = 100,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u16Min = 1024,
			.u16Max = 32768,
			.u16Def = 1024,
			.u16Step = 1,
		},
		.stDgain[0] = {
			.u16Min = 1024,
			.u16Max = 4032,
			.u16Def = 1024,
			.u16Step = 1,
		},
	},
	[SC1346_1L_MODE_600P60_WDR] = {
		.name = "600p60wdr",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1024,
				.u32Height = 600,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1024,
				.u32Height = 600,
			},
			.stMaxSize = {
				.u32Width = 1024,
				.u32Height = 600,
			},
		},
		.astImg[1] = {
			.stSnsSize = {
				.u32Width = 1024,
				.u32Height = 600,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1024,
				.u32Height = 600,
			},
			.stMaxSize = {
				.u32Width = 1024,
				.u32Height = 600,
			},
		},
		.f32MaxFps = 60,
		.f32MinFps = 1.37, /* 750 * 30 / 0x3FFF*/
		.u32HtsDef = 1280,
		.u32VtsDef = 750,
		.u16SexpMaxReg = 0x13E,
	},
};

static ISP_CMOS_BLACK_LEVEL_S g_stIspBlcCalibratio = {
	.bUpdate = CVI_TRUE,
	.blcAttr = {
		.Enable = 1,
		.enOpType = OP_TYPE_AUTO,
		.stManual = {260, 260, 260, 260, 0, 0, 0, 0
		#ifdef ARCH_CV182X
		, 1092, 1092, 1092, 1092
		#endif
		},
		.stAuto = {
			{260, 260, 260, 260, 260, 260, 260, 260, /*8*/260, 260, 260, 260, 260, 260, 260, 260},
			{260, 260, 260, 260, 260, 260, 260, 260, /*8*/260, 260, 260, 260, 260, 260, 260, 260},
			{260, 260, 260, 260, 260, 260, 260, 260, /*8*/260, 260, 260, 260, 260, 260, 260, 260},
			{260, 260, 260, 260, 260, 260, 260, 260, /*8*/260, 260, 260, 260, 260, 260, 260, 260},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			#ifdef ARCH_CV182X
			{1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092,
				/*8*/1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092},
			{1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092,
				/*8*/1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092},
			{1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092,
				/*8*/1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092},
			{1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092,
				/*8*/1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092},
			#endif
		},
	},
};

struct combo_dev_attr_s sc1346_1L_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_200M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {3, 2, -1, -1, -1},
		.pn_swap = {0, 0, 0, 0, 0},
		.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
		.dphy = {
			.enable = 1,
			.hs_settle = 8,
		}
	},
	.mclk = {
		.cam = 0,
		.freq = CAMPLL_FREQ_27M,
	},
	.devno = 0,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __SC1346_1L_CMOS_PARAM_H_ */
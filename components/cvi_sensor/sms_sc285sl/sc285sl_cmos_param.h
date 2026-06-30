#ifndef __SC285SL_CMOS_PARAM_H_
#define __SC285SL_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "cvi_comm_cif.h"
#include "cvi_type.h"
#include "cvi_sns_ctrl.h"
#include "sc285sl_cmos_ex.h"

static const SC285SL_MODE_S g_astSc285sl_mode[SC285SL_MODE_NUM] = {
	[SC285SL_MODE_1920X1080P30] = {
		.name = "1920X1080P30",
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
		.f32MinFps = 0.1, /* 1125 * 30 / SC285SL_FULL_LINES_MAX */
		.u32HtsDef = 2200,
		.u32VtsDef = 1125,
		.stExp[0] = {
			.u16Min = 1,
			.u16Max = 1125 - 8,
			.u16Def = 1117,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 168652,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16256,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[SC285SL_MODE_1920X1080P30_WDR] = {
		.name = "1920X1080P30_WDR",
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
		.astImg[1] = {
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
		.f32MinFps = 0.1,
		.u32HtsDef = 2200, // NA
		.u32VtsDef = 2750,
		.u16SexpMaxReg = 170,
		.stExp[0] = {
			.u16Min = 2,
			.u16Max = 170 - 11, //u16SexpMaxReg -11
			.u16Def = 159,
			.u16Step = 2,
		},
		.stExp[1] = {
			.u16Min = 2,
			.u16Max = 2750 - 170 - 13, //VTS - u16SexpMaxReg -13
			.u16Def = 2544,
			.u16Step = 2,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 168652,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stAgain[1] = {
			.u32Min = 1024,
			.u32Max = 168652,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 3072,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[1] = {
			.u32Min = 1024,
			.u32Max = 3072,
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
		.stManual = {0, 0, 0, 0, 0, 0, 0, 0

#ifdef ARCH_CV182X
		, 1091, 1091, 1091, 1091
#endif
		},
		.stAuto = {
			{252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252 },
			{252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252 },
			{252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252 },
			{252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252, 252 },
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
#ifdef ARCH_CV182X
			{1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091,
				1091, 1091},
			{1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091,
				1091, 1091},
			{1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091,
				1091, 1091},
			{1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091, 1091,
				1091, 1091},
#endif
		},
	},
};

struct combo_dev_attr_s sc285sl_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_200M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {1, 0, 2, -1, -1},
		.pn_swap = {1, 1, 1, 0, 0},
		.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
		.dphy = {
			.enable = CVI_TRUE,
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


#endif /* __SC285SL_CMOS_PARAM_H_ */


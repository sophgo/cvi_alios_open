#ifndef __SC533HAI_CMOS_PARAM_H_
#define __SC533HAI_CMOS_PARAM_H_

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
#include "sc533hai_cmos_ex.h"

static const SC533HAI_MODE_S g_astSC533HAI_mode[SC533HAI_MODE_NUM] = {
	[SC533HAI_MODE_1620P30] = {
		.name = "1620p30",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2880,
				.u32Height = 1620,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2880,
				.u32Height = 1620,
			},
			.stMaxSize = {
				.u32Width = 2880,
				.u32Height = 1620,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 1.7, /* 1875 * 60 / 0xFFF0*/
		.u32HtsDef = 3200,
		.u32VtsDef = 1650,
		.stExp[0] = {
			.u32Min = 2,
			.u32Max = 1650 - 8, //vts - 8
			.u32Def = 1642,
			.u32Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 83077,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16128,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[SC533HAI_MODE_1620P30_WDR_27M] = {
		.name = "1620p30wdr",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2880,
				.u32Height = 1620,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2880,
				.u32Height = 1620,
			},
			.stMaxSize = {
				.u32Width = 2880,
				.u32Height = 1620,
			},
		},
		.astImg[1] = {
			.stSnsSize = {
				.u32Width = 2880,
				.u32Height = 1620,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2880,
				.u32Height = 1620,
			},
			.stMaxSize = {
				.u32Width = 2880,
				.u32Height = 1620,
			},
		},
		.stExp[0] = {
			.u32Min = 4,
			.u32Max = 204 - 13, //u16SexpMaxReg - 13
			.u32Def = 191,
			.u32Step = 1,
		},
		.stExp[1] = {
			.u32Min = 4,
			.u32Max = 3300 - 204 - 15, //vts -u16SexpMaxReg - 15
			.u32Def = 3056,
			.u32Step = 1,
		},
		.f32MaxFps = 30,
		.f32MinFps = 0.377,
		.u32HtsDef = 3490,  /* NA */
		.u32VtsDef = 3300,
		.u16SexpMaxReg = 204,
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 83077,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16128,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stAgain[1] = {
			.u32Min = 1024,
			.u32Max = 83077,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[1] = {
			.u32Min = 1024,
			.u32Max = 16128,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[SC533HAI_MODE_1620P30_WDR_24M] = {
		.name = "1620p30wdr",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2880,
				.u32Height = 1620,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2880,
				.u32Height = 1620,
			},
			.stMaxSize = {
				.u32Width = 2880,
				.u32Height = 1620,
			},
		},
		.astImg[1] = {
			.stSnsSize = {
				.u32Width = 2880,
				.u32Height = 1620,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2880,
				.u32Height = 1620,
			},
			.stMaxSize = {
				.u32Width = 2880,
				.u32Height = 1620,
			},
		},
		.stExp[0] = {
			.u32Min = 4,
			.u32Max = 204 - 13, //u16SexpMaxReg - 13
			.u32Def = 191,
			.u32Step = 1,
		},
		.stExp[1] = {
			.u32Min = 4,
			.u32Max = 3300 - 204 - 15, //vts -u16SexpMaxReg - 15
			.u32Def = 3056,
			.u32Step = 1,
		},
		.f32MaxFps = 30,
		.f32MinFps = 0.377,
		.u32HtsDef = 3490,  /* NA */
		.u32VtsDef = 3300,
		.u16SexpMaxReg = 204,
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 83077,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16128,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stAgain[1] = {
			.u32Min = 1024,
			.u32Max = 83077,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[1] = {
			.u32Min = 1024,
			.u32Max = 16128,
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
		.stManual = {260, 260, 260, 260, 0, 0, 0, 0
#ifdef ARCH_CV182X
			, 1093, 1093, 1093, 1093
#endif
		},
		.stAuto = {
			{260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260},
			{260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260},
			{260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260},
			{260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
#ifdef ARCH_CV182X
			{1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093,
				/*8*/1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093},
			{1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093,
				/*8*/1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093},
			{1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093,
				/*8*/1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093},
			{1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093,
				/*8*/1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093},
#endif
		},
	},
};

struct combo_dev_attr_s sc533hai_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_600M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {0, 1, 2, -1, -1},
		.pn_swap = {1, 1, 1, 1, 1},
		.wdr_mode = CVI_MIPI_WDR_MODE_VC,
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


#endif /* __SC533HAI_CMOS_PARAM_H_ */

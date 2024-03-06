#ifndef __SC530AI_4L_CMOS_PARAM_H_
#define __SC530AI_4L_CMOS_PARAM_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "cif_uapi.h"
#include "cvi_type.h"
#include "cvi_sns_ctrl.h"
#include "sc530ai_4L_cmos_ex.h"

	static const SC530AI_4L_MODE_S g_astSC530AI_4L_mode[SC530AI_4L_MODE_NUM] = {
		[SC530AI_4L_MODE_1620P30_LINEAR] = {
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
			.f32MinFps = 1.52, // 1650 * 30 / 0x7FFF
			.u32HtsDef = 2560, // NA
			.u32VtsDef = 1650,
			.stExp[0] = {
				.u16Min = 3,
				.u16Max = 3290, // 2 * vts - 10
				.u16Def = 400,
				.u16Step = 1,
			},
			.stAgain[0] = {
				.u32Min = 1024,
				.u32Max = 78315,
				.u32Def = 1024,
				.u32Step = 1,
			},
			.stDgain[0] = {
				.u32Min = 1024,
				.u32Max = 4032,
				.u32Def = 1024,
				.u32Step = 1,
			},
		},
		[SC530AI_4L_MODE_1620P30_WDR] = {
			.name = "1620p30_wdr",
			// sexp
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
			// lexp
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

			.f32MaxFps = 30,
			.f32MinFps = 6,	   // vts * 30 / 0x7FFF
			.u32HtsDef = 2560, // NA
			.u32VtsDef = 3300,
			.u16SexpMaxReg = 0xC8,

			// sexp
			.stExp[0] = {
				.u16Min = 8,
				.u16Max = 386, // 2 * max_short_exp - 14
				.u16Def = 200,
				.u16Step = 4,
			},
			// lexp
			.stExp[1] = {
				.u16Min = 8,
				// max_short_exp = 0x13e
				.u16Max = 6182, // 2 * vts- 2 * max_short_exp -  18
				.u16Def = 400,
				.u16Step = 4,
			},

			// sexp
			.stAgain[0] = {
				.u32Min = 1024,
				.u32Max = 78315,
				.u32Def = 1024,
				.u32Step = 1,
			},
			// lexp
			.stAgain[1] = {
				.u32Min = 1024,
				.u32Max = 78315,
				.u32Def = 1024,
				.u32Step = 1,
			},

			// sexp
			.stDgain[0] = {
				.u32Min = 1024,
				.u32Max = 4032,
				.u32Def = 1024,
				.u32Step = 1,
			},
			// lexp
			.stDgain[1] = {
				.u32Min = 1024,
				.u32Max = 4032,
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
			.stManual = {260, 260, 260, 260, 0, 0, 0, 0},
			.stAuto = {
				{260, 260, 260, 260, 260, 260, 260, 260, /*8*/ 260, 260, 260, 260, 260, 260, 260, 260},
				{260, 260, 260, 260, 260, 260, 260, 260, /*8*/ 260, 260, 260, 260, 260, 260, 260, 260},
				{260, 260, 260, 260, 260, 260, 260, 260, /*8*/ 260, 260, 260, 260, 260, 260, 260, 260},
				{260, 260, 260, 260, 260, 260, 260, 260, /*8*/ 260, 260, 260, 260, 260, 260, 260, 260},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
				{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			},
		},
	};

	struct combo_dev_attr_s sc530ai_4l_rx_attr = {
		.input_mode = INPUT_MODE_MIPI,
		.mac_clk = RX_MAC_CLK_200M,
		.mipi_attr = {
			.raw_data_type = RAW_DATA_10BIT,
			.lane_id = {2, 0, 3, 1, 4},
			.pn_swap = {1, 1, 1, 1, 1},
			.wdr_mode = CVI_MIPI_WDR_MODE_VC,
			// .wdr_mode = CVI_MIPI_WDR_MODE_NONE,
			.dphy = {
				.enable = 1,
				.hs_settle = 8,
			},
		},
		.mclk = {
			.cam = 0,
			.freq = CAMPLL_FREQ_27M,
		},
		.devno = 0,
	};

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif /* __SC530AI_4L_CMOS_PARAM_H_ */

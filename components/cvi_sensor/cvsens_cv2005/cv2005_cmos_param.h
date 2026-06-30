#ifndef __CV2005_CMOS_PARAM_H_
#define __CV2005_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"
#include "cv2005_cmos_ex.h"

static const CV2005_MODE_S g_astCV2005_mode[CV2005_MODE_NUM] = {
	[CV2005_MODE_1920X1080P15] = {
		.name = "1920X1080P15",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1928,
				.u32Height = 1088,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1920,
				.u32Height = 1080,
			},
			.stMaxSize = {
				.u32Width = 1928,
				.u32Height = 1088,
			},
		},
		.f32MaxFps = 15,
		.f32MinFps = 0.023, /* 2360 * 15 / 0x0FFFFF  */
		.u32HtsDef = 1024,	//hts_reg
		/* FRAME_LENGTH default value: 0x938 (20'h938) */
		.u32VtsDef = 2360,	// FRAME_LENGTH
		.stExp[0] = {
			.u16Min = 1,
			.u16Max = 2360 -10,
			.u16Def = 2360 -6,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 32768,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 32000,
			.u32Def = 1024,
			.u32Step = 64,
		},
	},
	[CV2005_MODE_1920X1080P12P5] = {
		.name = "1920X1080P12P5",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1928,
				.u32Height = 1088,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1920,
				.u32Height = 1080,
			},
			.stMaxSize = {
				.u32Width = 1928,
				.u32Height = 1088,
			},
		},
		.f32MaxFps = 12.5,
		.f32MinFps = 0.496, /* 2640 * 12.5 / 0x0FFFFF  */
		.u32HtsDef = 1024,	//hts_reg
		/* FRAME_LENGTH default value: 0xA50 (20'ha50) */
		.u32VtsDef = 2640,	// FRAME_LENGTH
		.stExp[0] = {
			.u16Min = 1,
			.u16Max = 2640 -10,
			.u16Def = 2640 -6,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 32768,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 32000,
			.u32Def = 1024,
			.u32Step = 64,
		},
	},
};


static ISP_CMOS_BLACK_LEVEL_S g_stIspBlcCalibratio = {
	.bUpdate = CVI_TRUE,
	.blcAttr = {
		.Enable = 1,
		.enOpType = OP_TYPE_AUTO,
		.stManual = {264, 264, 264, 264, 0, 0, 0, 0
#ifdef ARCH_CV182X
			, 1094, 1094, 1094, 1094
#endif
		},
		.stAuto = {
			{264, 264, 264, 264, 264, 264, 264, 264, 264, 264, 264, 264, 264, 264, 264, 264 },
			{264, 264, 264, 264, 264, 264, 264, 264, 264, 264, 264, 264, 264, 264, 264, 264 },
			{264, 264, 264, 264, 264, 264, 264, 264, 264, 264, 264, 264, 264, 264, 264, 264 },
			{264, 264, 264, 264, 264, 264, 264, 264, 264, 264, 264, 264, 264, 264, 264, 264 },
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
#ifdef ARCH_CV182X
			{1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094,
				1094, 1094},
			{1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094,
				1094, 1094},
			{1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094,
				1094, 1094},
			{1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094, 1094,
				1094, 1094},
#endif
		},
	},
};

struct combo_dev_attr_s cv2005_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_200M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {2, 3, -1, -1, -1},
		.pn_swap = {0, 0, 0, 0, 0},
		.wdr_mode = CVI_MIPI_WDR_MODE_VC,
	},
	.mclk = {
		.cam = 1,
		.freq = CAMPLL_FREQ_24M,
	},
	.devno = 0,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __CV2005_CMOS_PARAM_H_ */

#ifndef __CV2002_CMOS_PARAM_H_
#define __CV2002_CMOS_PARAM_H_

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
#include "cv2002_cmos_ex.h"

static const CV2002_MODE_S g_astCV2002_mode[CV2002_MODE_NUM] = {
	[CV2002_MODE_1920X1080P30] = {
		.name = "1920X1080P30",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1928,
				.u32Height = 1088,
			},
			.stWndRect = {
				.s32X = 4,
				.s32Y = 4,
				.u32Width = 1920,
				.u32Height = 1080,
			},
			.stMaxSize = {
				.u32Width = 1928,
				.u32Height = 1088,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 0.143, /* 2500 * 30 / (0x0FFFFF / 2)  */
		.u32HtsDef = 624,	// hts_reg  (cv2002_coms.c里用不到, 随便写)
		.u32VtsDef = 2500,	// vts_reg / 2	, (u32FLStd)
		.stExp[0] = {		// SHUTTER0
			.u16Min = 3,
			.u16Max = 2500-1,	// FRAME_LENGTH - 2
			.u16Def = 282,	// SHUTTER0_init_reg / 2
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 16384,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16384,
			.u32Def = 1024,
			.u32Step = 64,
		},
	},
	[CV2002_MODE_1280X720P30] = {
		.name = "1280X720P30",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1288,
				.u32Height = 728,
			},
			.stWndRect = {
				.s32X = 4,
				.s32Y = 4,
				.u32Width = 1280,
				.u32Height = 720,
			},
			.stMaxSize = {
				.u32Width = 1288,
				.u32Height = 728,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 0.143, /* 2500 * 30 / (0x0FFFFF / 2)  */
		.u32HtsDef = 624,	// hts_reg  (cv2002_coms.c里用不到, 随便写)
		.u32VtsDef = 2500,	// vts_reg / 2	, (u32FLStd)
		.stExp[0] = {		// SHUTTER0
			.u16Min = 3,
			.u16Max = 2500-1,	// FRAME_LENGTH - 2
			.u16Def = 282,	// SHUTTER0_init_reg / 2
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 16384,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16384,
			.u32Def = 1024,
			.u32Step = 64,
		},
	},
	[CV2002_MODE_1920X1080P30_WDR] = {
		.name = "1920X1080P30_WDR",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1928,
				.u32Height = 1088,
			},
			.stWndRect = {
				.s32X = 4,
				.s32Y = 4,
				.u32Width = 1920,
				.u32Height = 1080,
			},
			.stMaxSize = {
				.u32Width = 1928,
				.u32Height = 1088,
			},
		},
		.astImg[1] = {
			.stSnsSize = {
				.u32Width = 1928,
				.u32Height = 1088,
			},
			.stWndRect = {
				.s32X = 4,
				.s32Y = 4,
				.u32Width = 1920,
				.u32Height = 1080,
			},
			.stMaxSize = {
				.u32Width = 1928,
				.u32Height = 1088,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 0.286, /* 1250 * 30 / (0x0FFFFF / 4) */
		.u32HtsDef = 624,	//hts_reg (cv2002_coms.c里用不到, 随便写)
		.u32VtsDef = 1250,	//vts_reg / 4 , (FPS)
		.stExp[0] = {	    // SHUTTER0
			.u16Min = 141,	// (HDR_RS1 + 10) / 4 ,  HDR_RS1 = 554
			.u16Max = 1250 - 1,	// (FRAME_LENGTH - 4) / 4
			.u16Def = 141,
			.u16Step = 1,
		},
		.stExp[1] = {		// SHUTTER1
			.u16Min = 2,
			.u16Max = 137,	// (HDR_RS1 - 4 + 2) / 4
			.u16Def = 2,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 16384,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stAgain[1] = {
			.u32Min = 1024,
			.u32Max = 16384,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16384,
			.u32Def = 1024,
			.u32Step = 64,
		},
		.stDgain[1] = {
			.u32Min = 1024,
			.u32Max = 16384,
			.u32Def = 1024,
			.u32Step = 64,
		},
	},
	[CV2002_MODE_1280X720P30_WDR] = {
		.name = "1280X720P30_WDR",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1288,
				.u32Height = 728,
			},
			.stWndRect = {
				.s32X = 4,
				.s32Y = 4,
				.u32Width = 1280,
				.u32Height = 720,
			},
			.stMaxSize = {
				.u32Width = 1288,
				.u32Height = 720,
			},
		},
		.astImg[1] = {
			.stSnsSize = {
				.u32Width = 1288,
				.u32Height = 728,
			},
			.stWndRect = {
				.s32X = 4,
				.s32Y = 4,
				.u32Width = 1280,
				.u32Height = 720,
			},
			.stMaxSize = {
				.u32Width = 1288,
				.u32Height = 1728,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 0.286, /* 1250 * 30 / (0x0FFFFF / 4) */
		.u32HtsDef = 624,	//hts_reg (cv2002_coms.c里用不到, 随便写)
		.u32VtsDef = 1250,	//vts_reg / 4 , (FPS)
		.stExp[0] = {	    // SHUTTER0
			.u16Min = 141,	// (HDR_RS1 + 10) / 4 ,  HDR_RS1 = 554
			.u16Max = 1250 - 1,	// (FRAME_LENGTH - 4) / 4
			.u16Def = 141,
			.u16Step = 1,
		},
		.stExp[1] = {		// SHUTTER1
			.u16Min = 2,
			.u16Max = 137,	// (HDR_RS1 - 4 + 2) / 4
			.u16Def = 2,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 16384,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stAgain[1] = {
			.u32Min = 1024,
			.u32Max = 16384,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16384,
			.u32Def = 1024,
			.u32Step = 64,
		},
		.stDgain[1] = {
			.u32Min = 1024,
			.u32Max = 16384,
			.u32Def = 1024,
			.u32Step = 64,
		},
	}
};

static ISP_CMOS_BLACK_LEVEL_S g_stIspBlcCalibratio = {
	.bUpdate = CVI_TRUE,
	.blcAttr = {
		.Enable = 1,
		.enOpType = OP_TYPE_AUTO,
		.stManual = {200, 200, 200, 200, 0, 0, 0, 0
		},
		.stAuto = {
			{200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200 },
			{200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200 },
			{200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200 },
			{200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200 },
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		},
	},
};

static ISP_CMOS_BLACK_LEVEL_S g_stIspBlcCalibratio_wdr = {
	.bUpdate = CVI_TRUE,
	.blcAttr = {
		.Enable = 1,
		.enOpType = OP_TYPE_AUTO,
		.stManual = {200, 200, 200, 200, 0, 0, 0, 0
		},
		.stAuto = {
			{200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200 },
			{200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200 },
			{200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200 },
			{200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200, 200 },
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		},
	},
};

struct combo_dev_attr_s cv2002_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_200M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {2, 1, 3, -1, -1},
		.pn_swap = {0, 0, 0, 0, 0},
		.wdr_mode = CVI_MIPI_WDR_MODE_VC,
		.dphy = {
			.enable = 1,
			.hs_settle = 12,
		}
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


#endif /* __CV2002_CMOS_PARAM_H_ */


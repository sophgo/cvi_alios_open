#ifndef __SC1330_CMOS_PARAM_H_
#define __SC1330_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"
#include "sc1330_cmos_ex.h"

static const SC1330_MODE_S g_astSC1330_mode[SC1330_MODE_NUM] = {
	[SC1330_MODE_960P30] = {
		.name = "960p30",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1280,
				.u32Height = 960,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1280,
				.u32Height = 960,
			},
			.stMaxSize = {
				.u32Width = 1280,
				.u32Height = 960,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 1.02, /* 1000 * 30 / 0xFFFF*/
		.u32HtsDef = 1280,
		.u32VtsDef = 1000,
		.stExp[0] = {
			.u32Min = 1,
			.u32Max = 1000 - 4,/* vts-4 */
			.u32Def = 100,
			.u32Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 88400,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 32512,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[SC1330_MODE_960P30_MASTER] = {
		.name = "960p30_master",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1280,
				.u32Height = 960,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1280,
				.u32Height = 960,
			},
			.stMaxSize = {
				.u32Width = 1280,
				.u32Height = 960,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 1.02, /* 1000 * 30 / 0xFFFF*/
		.u32HtsDef = 1280,
		.u32VtsDef = 1000,
		.stExp[0] = {
			.u32Min = 1,
			.u32Max = 1000 - 4,/* vts-4 */
			.u32Def = 100,
			.u32Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 88400,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 32512,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[SC1330_MODE_960P30_SLAVE] = {
		.name = "960p30_slave",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1280,
				.u32Height = 960,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1280,
				.u32Height = 960,
			},
			.stMaxSize = {
				.u32Width = 1280,
				.u32Height = 960,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 1.02, /* 1000 * 30 / 0xFFFF*/
		.u32HtsDef = 1280,
		.u32VtsDef = 1000,
		.stExp[0] = {
			.u32Min = 1,
			.u32Max = 1000 - 4,/* vts-4 */
			.u32Def = 100,
			.u32Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 88400,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 32512,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[SC1330_MODE_960P30_1L] = {
		.name = "960p30_1l",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1280,
				.u32Height = 960,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1280,
				.u32Height = 960,
			},
			.stMaxSize = {
				.u32Width = 1280,
				.u32Height = 960,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 1.02, /* 1000 * 30 / 0xFFFF*/
		.u32HtsDef = 1280,
		.u32VtsDef = 1000,
		.stExp[0] = {
			.u32Min = 1,
			.u32Max = 1000 - 4,/* vts-4 */
			.u32Def = 100,
			.u32Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 88400,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 32512,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[SC1330_MODE_960P30_1L_MASTER] = {
		.name = "960p30_1l_master",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1280,
				.u32Height = 960,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1280,
				.u32Height = 960,
			},
			.stMaxSize = {
				.u32Width = 1280,
				.u32Height = 960,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 1.02, /* 1000 * 30 / 0xFFFF*/
		.u32HtsDef = 1280,
		.u32VtsDef = 1000,
		.stExp[0] = {
			.u32Min = 1,
			.u32Max = 1000 - 4,/* vts-4 */
			.u32Def = 100,
			.u32Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 88400,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 32512,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[SC1330_MODE_960P30_1L_SLAVE] = {
		.name = "960p30_1l_slave",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1280,
				.u32Height = 960,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1280,
				.u32Height = 960,
			},
			.stMaxSize = {
				.u32Width = 1280,
				.u32Height = 960,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 1.02, /* 1000 * 30 / 0xFFFF*/
		.u32HtsDef = 1280,
		.u32VtsDef = 1000,
		.stExp[0] = {
			.u32Min = 1,
			.u32Max = 1000 - 4,/* vts-4 */
			.u32Def = 100,
			.u32Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 88400,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 32512,
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
		.stManual = {
			260, 260, 260, 260, 0, 0, 0, 0
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
		},
	},
};

struct combo_dev_attr_s sc1330_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_200M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {2, 3, -1, -1, -1},
		.pn_swap = {1, 1, 0, 0, 0},
		.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
		.dphy = {
			.enable = 1,
			.hs_settle = 11,
		},
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


#endif /* __SC1330_CMOS_PARAM_H_ */

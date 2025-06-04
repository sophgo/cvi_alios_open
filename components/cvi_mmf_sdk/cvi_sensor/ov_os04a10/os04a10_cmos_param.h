#ifndef __OS04A10_CMOS_PARAM_H_
#define __OS04A10_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"
#include "os04a10_cmos_ex.h"

static const OS04A10_MODE_S g_astOs04a10_mode[OS04A10_MODE_NUM] = {
	[OS04A10_MODE_1440P30_12BIT] = {
		.name = "1440p30_12bit",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2560,
				.u32Height = 1440,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2560,
				.u32Height = 1440,
			},
			.stMaxSize = {
				.u32Width = 2560,
				.u32Height = 1440,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 0.74, /* 0x658 * 30 / 0xFFFF */
		.u32HtsDef = 1484,
		.u32VtsDef = 2432,
		.stExp[0] = {
			.u16Min = 1,
			.u16Max = 2432 - 8,
			.u16Def = 500,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 68200,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16373,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[OS04A10_MODE_1440P30_2L_10BIT] = {
		.name = "1440p30_10bit",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2560,
				.u32Height = 1440,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2560,
				.u32Height = 1440,
			},
			.stMaxSize = {
				.u32Width = 2560,
				.u32Height = 1440,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 0.74, /* 0x658 * 30 / 0xFFFF */
		.u32HtsDef = 1464,
		.u32VtsDef = 1708,
		.stExp[0] = {
			.u16Min = 1,
			.u16Max = 1708 - 8,
			.u16Def = 500,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 68200,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16373,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[OS04A10_MODE_1440P30_2L_MASTER_10BIT] = {
		.name = "1440p30_master_10bit",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2560,
				.u32Height = 1440,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2560,
				.u32Height = 1440,
			},
			.stMaxSize = {
				.u32Width = 2560,
				.u32Height = 1440,
			},
		},
		.f32MaxFps = 31,
		.f32MinFps = 0.74, /* 0x658 * 30 / 0xFFFF */
		.u32HtsDef = 1464,
		.u32VtsDef = 1624,
		.stExp[0] = {
			.u16Min = 1,
			.u16Max = 1624 - 8,
			.u16Def = 500,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 68200,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16373,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[OS04A10_MODE_1440P30_2L_SLAVE_10BIT] = {
		.name = "1440p30_slave_10bit",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2560,
				.u32Height = 1440,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2560,
				.u32Height = 1440,
			},
			.stMaxSize = {
				.u32Width = 2560,
				.u32Height = 1440,
			},
		},
		.f32MaxFps = 31,
		.f32MinFps = 0.74, /* 0x658 * 30 / 0xFFFF */
		.u32HtsDef = 1464,
		.u32VtsDef = 1624,
		.stExp[0] = {
			.u16Min = 1,
			.u16Max = 1624 - 8,
			.u16Def = 500,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 68200,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16373,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[OS04A10_MODE_1440P30_WDR] = {
		.name = "1440p30wdr",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2688,
				.u32Height = 1520,
			},
			.stWndRect = {
				.s32X = 64,
				.s32Y = 40,
				.u32Width = 2560,
				.u32Height = 1440,
			},
			.stMaxSize = {
				.u32Width = 2688,
				.u32Height = 1520,
			},
		},
		.astImg[1] = {
			.stSnsSize = {
				.u32Width = 2688,
				.u32Height = 1520,
			},
			.stWndRect = {
				.s32X = 64,
				.s32Y = 40,
				.u32Width = 2560,
				.u32Height = 1440,
			},
			.stMaxSize = {
				.u32Width = 2688,
				.u32Height = 1520,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 0.74, /* 1624 * 30 / 0xFFFF */
		.u32HtsDef = 2972,
		.u32VtsDef = 1624,
		.u32L2S_offset = 40,
		.u32IspResTime = 49, /* ceil((u32Vts * f32MaxFps) / 1000); about 1ms*/
		.u32VStart = 0,
		.u32VEnd = 0x5ff,
		.stExp[0] = {
			.u16Min = 2,
			.u16Max = 88,
			.u16Def = 88,
			.u16Step = 1,
		},
		.stExp[1] = {
			.u16Min = 2,
			.u16Max = 0x486 - 4 - 88,
			.u16Def = 500,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 68200,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stAgain[1] = {
			.u32Min = 1024,
			.u32Max = 68200,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16373,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[1] = {
			.u32Min = 1024,
			.u32Max = 16373,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[OS04A10_MODE_1440P30_MASTER_WDR] = {
		.name = "1440p_master_30wdr",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2560,
				.u32Height = 1440,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2560,
				.u32Height = 1440,
			},
			.stMaxSize = {
				.u32Width = 2560,
				.u32Height = 1440,
			},
		},
		.astImg[1] = {
			.stSnsSize = {
				.u32Width = 2560,
				.u32Height = 1440,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2560,
				.u32Height = 1440,
			},
			.stMaxSize = {
				.u32Width = 2560,
				.u32Height = 1440,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 0.74, /* 1624 * 30 / 0xFFFF */
		.u32HtsDef = 2972,
		.u32VtsDef = 1624,
		.u32L2S_offset = 40,
		.u32IspResTime = 49, /* ceil((u32Vts * f32MaxFps) / 1000); about 1ms*/
		.u32VStart = 0,
		.u32VEnd = 0x5ff,
		.stExp[0] = {
			.u16Min = 2,
			.u16Max = 88,
			.u16Def = 88,
			.u16Step = 1,
		},
		.stExp[1] = {
			.u16Min = 2,
			.u16Max = 0x486 - 4 - 88,
			.u16Def = 500,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 68200,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stAgain[1] = {
			.u32Min = 1024,
			.u32Max = 68200,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16373,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[1] = {
			.u32Min = 1024,
			.u32Max = 16373,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[OS04A10_MODE_1440P30_SLAVE_WDR] = {
		.name = "1440p30_slave_wdr",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2560,
				.u32Height = 1440,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2560,
				.u32Height = 1440,
			},
			.stMaxSize = {
				.u32Width = 2560,
				.u32Height = 1440,
			},
		},
		.astImg[1] = {
			.stSnsSize = {
				.u32Width = 2560,
				.u32Height = 1440,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2560,
				.u32Height = 1440,
			},
			.stMaxSize = {
				.u32Width = 2560,
				.u32Height = 1440,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 0.74, /* 1624 * 30 / 0xFFFF */
		.u32HtsDef = 2972,
		.u32VtsDef = 1624,
		.u32L2S_offset = 40,
		.u32IspResTime = 49, /* ceil((u32Vts * f32MaxFps) / 1000); about 1ms*/
		.u32VStart = 0,
		.u32VEnd = 0x5ff,
		.stExp[0] = {
			.u16Min = 2,
			.u16Max = 88,
			.u16Def = 88,
			.u16Step = 1,
		},
		.stExp[1] = {
			.u16Min = 2,
			.u16Max = 0x486 - 4 - 88,
			.u16Def = 500,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 68200,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stAgain[1] = {
			.u32Min = 1024,
			.u32Max = 68200,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 16373,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[1] = {
			.u32Min = 1024,
			.u32Max = 16373,
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

struct combo_dev_attr_s os04a10_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_400M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_12BIT,
		.lane_id = {0, 1, 2, 3, 4},
		.wdr_mode = CVI_MIPI_WDR_MODE_VC,
		.dphy = {
			.enable = 1,
			.hs_settle = 8,
		},
	},
	.mclk = {
		.cam = 0,
		.freq = CAMPLL_FREQ_25M,
	},
	.devno = 0,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __OS04A10_CMOS_PARAM_H_ */

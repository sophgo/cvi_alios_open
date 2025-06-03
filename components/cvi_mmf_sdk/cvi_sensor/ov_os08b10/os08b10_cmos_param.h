#ifndef __OS08B10_CMOS_PARAM_H_
#define __OS08B10_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"
#include "os08b10_cmos_ex.h"

static const OS08B10_MODE_S g_astos08b10_mode[OS08B10_MODE_NUM] = {
	[OS08B10_MODE_3840X2160P30] = {
		.name = "3840x2160p30",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 3840,
				.u32Height = 2160,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 3840,
				.u32Height = 2160,
			},
			.stMaxSize = {
				.u32Width = 3840,
				.u32Height = 2160,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 0.51,
		.u32HtsDef = 0x054B,
		.u32VtsDef = 0x0453,
		.stExp[0] = {
			.u16Min = 1,
			.u16Max = 0x0453 - 8,
			.u16Def = 0x0140,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 65800,
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
	[OS08B10_MODE_3840X2160P30_WDR] = {
		.name = "3840x2160p30wdr",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 3840,
				.u32Height = 2160,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 3840,
				.u32Height = 2160,
			},
			.stMaxSize = {
				.u32Width = 3840,
				.u32Height = 2160,
			},
		},
		.astImg[1] = {
			.stSnsSize = {
				.u32Width = 3840,
				.u32Height = 2160,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 3840,
				.u32Height = 2160,
			},
			.stMaxSize = {
				.u32Width = 3840,
				.u32Height = 2160,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 0.51,	/* 0x0453 * 30 / 0xFFFFF */
		.u32HtsDef = 0x065a,
		.u32VtsDef = 0x0453,
		.u32L2S_offset = 40,
		.u32IspResTime = 49, /* ceil((u32Vts * f32MaxFps) / 1000); about 1ms*/
		.u32VStart = 0,
		.u32VEnd = 0x5ff,
		.stExp[0] = {
			.u16Min = 2,
			.u16Max = 84,
			.u16Def = 84,
			.u16Step = 1,
		},
		.stExp[1] = {
			.u16Min = 2,
			.u16Max = 0x0453 - 4 - 84,
			.u16Def = 320,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 65800,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stAgain[1] = {
			.u32Min = 1024,
			.u32Max = 65800,
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
		.stManual = {256, 256, 256, 256, 0, 0, 0, 0},
		.stAuto = {
			{256, 256, 256, 256, 256, 256, 256, 256, /*8*/256, 256, 256, 256, 256, 256, 256, 256},
			{256, 256, 256, 256, 256, 256, 256, 256, /*8*/256, 256, 256, 256, 256, 256, 256, 256},
			{256, 256, 256, 256, 256, 256, 256, 256, /*8*/256, 256, 256, 256, 256, 256, 256, 256},
			{256, 256, 256, 256, 256, 256, 256, 256, /*8*/256, 256, 256, 256, 256, 256, 256, 256},
			{0, 0, 0, 0, 0, 0, 0, 0,
				/*8*/0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0,
				/*8*/0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0,
				/*8*/0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0,
				/*8*/0, 0, 0, 0, 0, 0, 0, 0},
		},
	},
};

struct combo_dev_attr_s os08b10_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_600M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {4, 3, 2, 0, 1},
		.wdr_mode = CVI_MIPI_WDR_MODE_VC,
		.dphy = {
			.enable = 1,
			.hs_settle = 14,
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


#endif /* __os08b10_CMOS_PARAM_H_ */

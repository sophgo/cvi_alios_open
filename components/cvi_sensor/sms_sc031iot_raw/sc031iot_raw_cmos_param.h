#ifndef __SC031IOT_RAW_CMOS_PARAM_H_
#define __SC031IOT_RAW_CMOS_PARAM_H_

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
#include "sc031iot_raw_cmos_ex.h"

static const SC031IOT_RAW_MODE_S g_astSc031iot_raw_mode[SC031IOT_RAW_MODE_NUM] = {
	[SC031IOT_RAW_MODE_640X480P30] = {
		.name = "640X480P30",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 640,
				.u32Height = 480,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 640,
				.u32Height = 480,
			},
			.stMaxSize = {
				.u32Width = 640,
				.u32Height = 480,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 0.23,
		.u32HtsDef = 0x708,
		.u32VtsDef = 0x1f4,
		.stExp[0] = {
			.u16Min = 1,
			.u16Max = 2 * (0x1f4) - 8,
			.u16Def = 100,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 63897,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 8130,
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
		.stManual = {92, 92, 92, 92, 0, 0, 0, 0
#ifdef ARCH_CV182X
		, 1047, 1047, 1047, 1047
#endif
		},
		.stAuto = {
			{92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92 },
			{92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92 },
			{92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92 },
			{92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92, 92 },
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
#ifdef ARCH_CV182X
			{1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047,
				1047, 1047},
			{1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047,
				1047, 1047},
			{1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047,
				1047, 1047},
			{1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047, 1047,
				1047, 1047},
#endif
		},
	},
};

struct combo_dev_attr_s sc031iot_raw_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_200M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {2, 1, -1, -1, -1},
		.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
		.dphy = {
			.enable = 1,
			.hs_settle = 8,
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


#endif /* __SC031IOT_RAW_CMOS_PARAM_H_ */
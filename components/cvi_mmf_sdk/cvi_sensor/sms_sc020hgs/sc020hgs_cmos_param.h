#ifndef __SC020HGS_CMOS_PARAM_H_
#define __SC020HGS_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"
#include "sc020hgs_cmos_ex.h"

static const SC020HGS_MODE_S g_astSC020HGS_mode[SC020HGS_MODE_NUM] = {
	[SC020HGS_MODE_400X400P120] = {
		.name = "400p120",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 400,
				.u32Height = 400,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 400,
				.u32Height = 400,
			},
			.stMaxSize = {
				.u32Width = 400,
				.u32Height = 400,
			},
		},
		.f32MaxFps = 120,
		.f32MinFps = 1.58, /* 62.5Mhz/(600*0xfff0) * 120 */
		.u32HtsDef = 600,
		.u32VtsDef = 868,
		.stExp[0] = {
			.u16Min = 4,
			.u16Max = (868 - 9) << 4,
			.u16Def = 400 << 4,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 16128,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 7168,
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
		.stManual = {256, 256, 256, 256, 0, 0, 0, 0
#ifdef ARCH_CV182X
		, 1092, 1092, 1092, 1092
#endif
		}, /* [TODO] check black leve*/
		.stAuto = {
			{256, 256, 256, 256, 256, 256, 256, 256, /*8*/256, 256, 256, 256, 256, 256, 256, 256},
			{256, 256, 256, 256, 256, 256, 256, 256, /*8*/256, 256, 256, 256, 256, 256, 256, 256},
			{256, 256, 256, 256, 256, 256, 256, 256, /*8*/256, 256, 256, 256, 256, 256, 256, 256},
			{256, 256, 256, 256, 256, 256, 256, 256, /*8*/256, 256, 256, 256, 256, 256, 256, 256},
			{0, 0, 0, 0, 0, 0, 0, 0,/*8*/0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0,/*8*/0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0,/*8*/0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0,/*8*/0, 0, 0, 0, 0, 0, 0, 0},
#ifdef ARCH_CV182X
			{1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092,
			 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092},
			{1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092,
			 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092},
			{1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092,
			 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092},
			{1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092,
			 1092, 1092, 1092, 1092, 1092, 1092, 1092, 1092},
#endif
		},
	},
};

struct combo_dev_attr_s sc020hgs_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_200M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {2, 0, -1, -1, -1},
		.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
		.pn_swap = {0, 0, 0, 0, 0},
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


#endif /* __SC020HGS_CMOS_PARAM_H_ */

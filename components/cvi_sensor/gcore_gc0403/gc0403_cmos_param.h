#ifndef __GC0403_CMOS_PARAM_H_
#define __GC0403_CMOS_PARAM_H_

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
#include "gc0403_cmos_ex.h"


static const GC0403_MODE_S g_stGc0403_mode[GC0403_MODE_NUM] = {
	[GC0403_MODE_768X576P60] = {
		.name = "768X576P60",
		.stImg = {
			.stSnsSize = {
				.u32Width = 768,
				.u32Height = 576,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 768,
				.u32Height = 576,
			},
			.stMaxSize = {
				.u32Width = 768,
				.u32Height = 576,
			},
		},
		.f32MaxFps = 61.6,
		.f32MinFps = 2.454, /* 1340 * 30 / 0x3FFF */
		.u32HtsDef = 2688,  /* 0x540 * 2 */
		.u32VtsDef = 663,
		.stExp = {
			.u16Min = 1,
			.u16Max = 663 - 8,
			.u16Def = 597,
			.u16Step = 1,
		},
		.stAgain = {
			.u32Min = 1024,
			.u32Max = 16 * 2489,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain = {
			.u32Min = 1024,
			.u32Max = 1024,
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
			, 1090, 1090, 1090, 1090
#endif
		},
		.stAuto = {
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			// {248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248},
			// {248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248},
			// {248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248},
			// {248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
#ifdef ARCH_CV182X
			{1090, 1090, 1090, 1090, 1090, 1090, 1090, 1090,
				1090, 1090, 1090, 1090, 1090, 1090, 1090, 1090},
			{1090, 1090, 1090, 1090, 1090, 1090, 1090, 1090,
				1090, 1090, 1090, 1090, 1090, 1090, 1090, 1090},
			{1090, 1090, 1090, 1090, 1090, 1090, 1090, 1090,
				1090, 1090, 1090, 1090, 1090, 1090, 1090, 1090},
			{1090, 1090, 1090, 1090, 1090, 1090, 1090, 1090,
				1090, 1090, 1090, 1090, 1090, 1090, 1090, 1090},
#endif
		},
	},
};

struct combo_dev_attr_s gc0403_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_200M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {1, 0, -1, -1, -1},
		.pn_swap = {0, 0, 0, 0, 0},
		.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
		.dphy = {
			.enable = 1,
			.hs_settle = 12,
		},
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


#endif /* __GC0403_CMOS_PARAM_H_ */


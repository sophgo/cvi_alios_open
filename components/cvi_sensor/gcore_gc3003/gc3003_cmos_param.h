#ifndef __GC3003_CMOS_PARAM_H_
#define __GC3003_CMOS_PARAM_H_

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
#include "gc3003_cmos_ex.h"


static const GC3003_MODE_S g_stGc3003_mode = {
	.name = "2304X1296P30",
	.stImg = {
		.stSnsSize = {
			.u32Width = 2304,
			.u32Height = 1296,
		},
		.stWndRect = {
			.s32X = 0,
			.s32Y = 0,
			.u32Width = 2304,
			.u32Height = 1296,
		},
		.stMaxSize = {
			.u32Width = 2304,
			.u32Height = 1296,
		},
	},
	.f32MaxFps = 30,
	.f32MinFps = 2.454, /* 1340 * 30 / 0x3FFF */
	.u32HtsDef = 2688,  /* 0x540 * 2 */
	.u32VtsDef = 1340,
	.stExp = {
		.u16Min = 1,
		.u16Max = 1340 -1,
		.u16Def = 400,
		.u16Step = 1,
	},
	.stAgain = {
		.u32Min = 1024,
		.u32Max = 4096 * 16,
		.u32Def = 1024,
		.u32Step = 1,
	},
	.stDgain = {
		.u32Min = 1024,
		.u32Max = 1024,
		.u32Def = 1024,
		.u32Step = 1,
	},
};

static ISP_CMOS_BLACK_LEVEL_S g_stIspBlcCalibratio = {
	.bUpdate = CVI_TRUE,
	.blcAttr = {
		.Enable = 1,
		.enOpType = OP_TYPE_AUTO,
		.stManual = {248, 248, 248, 248, 0, 0, 0, 0
#ifdef ARCH_CV182X
			, 1090, 1090, 1090, 1090
#endif
		},
		.stAuto = {
			{248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248},
			{248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248},
			{248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248},
			{248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248, 248},
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

struct combo_dev_attr_s gc3003_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_200M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {3, 2, 4, -1, -1},
		.pn_swap = {0, 0, 0, 0, 0},
		.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
	},
	.mclk = {
		.cam = 1,
		.freq = CAMPLL_FREQ_27M,
	},
	.devno = 0,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __GC3003_CMOS_PARAM_H_ */


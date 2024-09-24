#ifndef __GC0308_CMOS_PARAM_H_
#define __GC0308_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "cvi_comm_cif.h"
#include "cvi_type.h"
#include "cvi_sns_ctrl.h"
#include "gc0308_cmos_ex.h"

static const GC0308_MODE_S g_astGc0308_mode = {
	.name = "640X480P30",
	.stImg = {
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
};

struct combo_dev_attr_s gc0308_rx_attr = {
	.input_mode = INPUT_MODE_BT601,
	.mac_clk = RX_MAC_CLK_200M,
	.ttl_attr = {
		.vi = TTL_VI_SRC_VI1,
		.ttl_fmt = TTL_VSDE_11B,
		.raw_data_type = RAW_DATA_8BIT,
		.func = {
			8, -1, -1, 9,
			0, 1, 2, 3,
			4, 5, 6, 7,
			-1, -1, -1, -1,
			-1, -1, -1, -1,
		},
	},
	.mclk = {
		.cam = 2,
		.freq = CAMPLL_FREQ_NONE,
	},
	.devno = 0,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __GC0308_CMOS_PARAM_H_ */

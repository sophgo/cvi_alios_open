#ifndef __LT6911_CMOS_PARAM_H_
#define __LT6911_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"
#include "lt6911_cmos_ex.h"

static LT6911_MODE_S g_astLt6911_mode[LT6911_MODE_NUM] = {
	[LT6911_MODE_4K60] = {
		.name = "lt6911",
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
	},
	[LT6911_MODE_1080P60] = {
		.name = "lt6911",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1920,
				.u32Height = 1080,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1920,
				.u32Height = 1080,
			},
			.stMaxSize = {
				.u32Width = 1920,
				.u32Height = 1080,
			},
		},
	},
	[LT6911_MODE_720P60] = {
		.name = "lt6911",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1280,
				.u32Height = 720,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 1280,
				.u32Height = 720,
			},
			.stMaxSize = {
				.u32Width = 1280,
				.u32Height = 720,
			},
		},
	},
};

struct combo_dev_attr_s lt6911_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_900M,
	.mipi_attr = {
		.raw_data_type = YUV422_8BIT,
		.lane_id = {2, 0, 1, 3, 4}, //3, 0, 1, 2, 4  ;   1, 4, 3, 2, 0  2, 0, 1, 3, 4   3, 1, 2, 4, 0
		.pn_swap = {1, 1, 1, 1, 1},
		.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
		.dphy = {
			.enable = 1,
			.hs_settle = 8,
		},
	},
	.mclk = {
		.cam = 0,
		.freq = CAMPLL_FREQ_NONE,
	},
	.devno = 1,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __LT6911_CMOS_PARAM_H_ */
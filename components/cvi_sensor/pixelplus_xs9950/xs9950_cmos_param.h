#ifndef __XS9950_CMOS_PARAM_H_
#define __XS9950_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "cvi_comm_cif.h"
#include "cvi_type.h"
#include "cvi_sns_ctrl.h"
#include "xs9950_cmos_ex.h"

static const XS9950_MODE_S g_astXs9950_mode[XS9950_MODE_NUM] = {
	[XS9950_MODE_720P_25] = {
		.name = "720p25",
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
	[XS9950_MODE_720P_30] = {
		.name = "720p30",
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
	[XS9950_MODE_1080P_25] = {
		.name = "1080p25",
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
	[XS9950_MODE_1080P_30] = {
		.name = "1080p30",
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
};

struct combo_dev_attr_s xs9950_rx_attr = {
	// .input_mode = INPUT_MODE_MIPI,
	// .mac_clk = RX_MAC_CLK_200M,
	// .mipi_attr = {
	// 	.raw_data_type = YUV422_8BIT,
	// 	.lane_id = {3, 4, 5, -1, -1},
	// 	.pn_swap = {1, 1, 1, 0, 0},
	// 	.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
	// },
	// .mclk = {
	// 	.cam = 1,
	// 	.freq = CAMPLL_FREQ_27M,
	// },

	// .devno = 0,
	.input_mode = INPUT_MODE_BT656_9B,
	.mac_clk = RX_MAC_CLK_200M,
	.mclk = {
		.cam = 0,
		.freq = CAMPLL_FREQ_27M,
	},
	.ttl_attr = {
		.vi = TTL_VI_SRC_VI1,
		.func = {
			-1, -1, -1, -1,
			 2, 3, 4, 5, 6, 7,8,9,
			-1, -1, -1, -1,
			-1, -1, -1, -1,
		},
		.sav_vld = 0x8000,
		.sav_blk = 0xA000,
	},
	.devno = 0,
};

struct combo_dev_attr_s xs9950_rx1_attr = {
	// .input_mode = INPUT_MODE_MIPI,
	// .mac_clk = RX_MAC_CLK_200M,
	// .mipi_attr = {
	// 	.raw_data_type = YUV422_8BIT,
	// 	.lane_id = {3, 4, 5, -1, -1},
	// 	.pn_swap = {1, 1, 1, 0, 0},
	// 	.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
	// },
	// .mclk = {
	// 	.cam = 1,
	// 	.freq = CAMPLL_FREQ_27M,
	// },
	// .devno = 1,
		.input_mode = INPUT_MODE_BT656_9B,
	.mac_clk = RX_MAC_CLK_200M,
	.mclk = {
		.cam = 1,
		.freq = CAMPLL_FREQ_27M,
	},
	.ttl_attr = {
		.vi = TTL_VI_SRC_VI1,
		.func = {
			-1, -1, -1, -1,
			 2, 3, 4, 5, 6, 7,8,9,
			-1, -1, -1, -1,
			-1, -1, -1, -1,
		},
	},
	.devno = 1,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __XS9950_CMOS_PARAM_H_ */

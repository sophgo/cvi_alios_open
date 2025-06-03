#ifndef __nc021_CMOS_PARAM_H_
#define __nc021_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef ARCH_CV182X
#include <linux/cvi_vip_cif.h>
#include <linux/cvi_vip_snsr.h>
#include "cvi_type.h"
#else
#include <cvi_comm_cif.h>
#include <vi_snsr.h>
#include <cvi_type.h>
#endif
#include "cvi_sns_ctrl.h"
#include "nc021_cmos_ex.h"

static const NC021_MODE_S g_astnc021_mode[NC021_MODE_NUM] = {
	[NC021_MODE_1080P] = {
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
};

struct combo_dev_attr_s nc021_multi_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_600M,
	.mipi_attr = {
		.raw_data_type = YUV422_8BIT,
		.lane_id = {0, 1, 2, 3, 4},
		.pn_swap = {1, 1, 1, 1, 1},
	},
	.mclk = {
		.cam = 0,
		.freq = CAMPLL_FREQ_NONE,
	},
	.devno = 0,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __nc021_CMOS_PARAM_H_ */

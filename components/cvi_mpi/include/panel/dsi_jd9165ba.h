#ifndef _MIPI_TX_PARAM_JD9165BA_H_
#define _MIPI_TX_PARAM_JD9165BA_H_

#define JD9165BA_HACT 1024
#define JD9165BA_HSA  24
#define JD9165BA_HBP  136
#define JD9165BA_HFP  160

#define JD9165BA_VACT 600
#define JD9165BA_VSA  2
#define JD9165BA_VBP  21
#define JD9165BA_VFP  12

#define PIXEL_CLK(x) ((x##_VACT + x##_VSA + x##_VBP + x##_VFP) * (x##_HACT + x##_HSA + x##_HBP + x##_HFP) * 60 / 1000)

struct combo_dev_cfg_s dev_cfg_jd9165ba_1024x600 = {
	.devno = 0,
	.lane_id = {MIPI_TX_LANE_0, MIPI_TX_LANE_1, MIPI_TX_LANE_CLK, -1, -1},
	.lane_pn_swap = {false, false, false, false, false},
	.output_mode = OUTPUT_MODE_DSI_VIDEO,
	.video_mode = BURST_MODE,
	.output_format = OUT_FORMAT_RGB_24_BIT,
	.sync_info = {
		.vid_hsa_pixels = JD9165BA_HSA,
		.vid_hbp_pixels = JD9165BA_HBP,
		.vid_hfp_pixels = JD9165BA_HFP,
		.vid_hline_pixels = JD9165BA_HACT,
		.vid_vsa_lines = JD9165BA_VSA,
		.vid_vbp_lines = JD9165BA_VBP,
		.vid_vfp_lines = JD9165BA_VFP,
		.vid_active_lines = JD9165BA_VACT,
		.vid_vsa_pos_polarity = true,
		.vid_hsa_pos_polarity = true,
	},
	.pixel_clk = PIXEL_CLK(JD9165BA),
};

const struct hs_settle_s hs_timing_cfg_jd9165ba_1024x600 = { .prepare = 6, .zero = 32, .trail = 1 };

static CVI_U8 data_jd9165ba_0[] = { 0x11 };
static CVI_U8 data_jd9165ba_1[] = { 0x29 };
const struct dsc_instr dsi_init_cmds_jd9165ba_1024x600[] = {
	{.delay = 120, .data_type = 0x05, .size = 1, .data = data_jd9165ba_0 },
	{.delay = 25, .data_type = 0x05, .size = 1, .data = data_jd9165ba_1 }
};

#else
#error "MIPI_TX_PARAM multi-delcaration!!"
#endif // _MIPI_TX_PARAM_JD9165BA_H_

#ifndef _MIPI_TX_PARAM_ST_7701_480X640_H_
#define _MIPI_TX_PARAM_ST_7701_480X640_H_

#include <cvi_comm_mipi_tx.h>

#define ST7701_480x640_VACT	640
#define ST7701_480x640_VSA	6
#define ST7701_480x640_VBP	30
#define ST7701_480x640_VFP	14

#define ST7701_480x640_HACT	480
#define ST7701_480x640_HSA	30
#define ST7701_480x640_HBP	37
#define ST7701_480x640_HFP	37

#define ST7701_480x640_PIXEL_CLK(x) ((x##_VACT + x##_VSA + x##_VBP + x##_VFP) \
	* (x##_HACT + x##_HSA + x##_HBP + x##_HFP) * 60 / 1000)

struct combo_dev_cfg_s dev_cfg_st7701_480x640 = {
	.devno = 0,
	.lane_id = {MIPI_TX_LANE_CLK, MIPI_TX_LANE_0, MIPI_TX_LANE_1, -1, -1},
	.lane_pn_swap = {false, false, false, false, false},
	.output_mode = OUTPUT_MODE_DSI_VIDEO,
	.video_mode = BURST_MODE,
	.output_format = OUT_FORMAT_RGB_24_BIT,
	.sync_info = {
		.vid_hsa_pixels = ST7701_480x640_HSA,
		.vid_hbp_pixels = ST7701_480x640_HBP,
		.vid_hfp_pixels = ST7701_480x640_HFP,
		.vid_hline_pixels = ST7701_480x640_HACT,
		.vid_vsa_lines = ST7701_480x640_VSA,
		.vid_vbp_lines = ST7701_480x640_VBP,
		.vid_vfp_lines = ST7701_480x640_VFP,
		.vid_active_lines = ST7701_480x640_VACT,
		.vid_vsa_pos_polarity = true,
		.vid_hsa_pos_polarity = false,
	},
	.pixel_clk = ST7701_480x640_PIXEL_CLK(ST7701_480x640),
};

struct hs_settle_s hs_timing_cfg_st7701_480x640 = { .prepare = 6, .zero = 32, .trail = 1 };

static CVI_U8 data_st7701_480x640_0[] = { 0x11 };
static CVI_U8 data_st7701_480x640_1[] = { 0xff, 0x77, 0x01, 0x00, 0x00, 0x13 };
static CVI_U8 data_st7701_480x640_2[] = { 0xef, 0x08 };
static CVI_U8 data_st7701_480x640_3[] = { 0xff, 0x77, 0x01, 0x00, 0x00, 0x10 };
static CVI_U8 data_st7701_480x640_4[] = { 0xc0, 0x4f, 0x00 };
static CVI_U8 data_st7701_480x640_5[] = { 0xc1, 0x12, 0x02 };
static CVI_U8 data_st7701_480x640_6[] = { 0xc2, 0x07, 0x19 };
static CVI_U8 data_st7701_480x640_7[] = { 0xcc, 0x10 };
static CVI_U8 data_st7701_480x640_8[] = {
	0xB0, 0x80, 0xD0, 0xDA, 0x0A, 0x0F, 0x06, 0x0B, 0x0B, 0x09,
	0x25, 0x08, 0x14, 0x11, 0x68, 0xAC, 0x4F
};
static CVI_U8 data_st7701_480x640_9[] = {
	0xb1, 0x80, 0xDB, 0xE0, 0x0F, 0x11, 0x05, 0x08, 0x06, 0x06,
	0x21, 0x01, 0x8F, 0x0F, 0x25, 0xAB, 0x4F
};
static CVI_U8 data_st7701_480x640_10[] = { 0xff, 0x77, 0x01, 0x00, 0x00, 0x11 };
static CVI_U8 data_st7701_480x640_11[] = { 0xb0, 0x2D };
static CVI_U8 data_st7701_480x640_12[] = { 0xb1, 0x61 };
static CVI_U8 data_st7701_480x640_13[] = { 0xb2, 0x01 };
static CVI_U8 data_st7701_480x640_14[] = { 0xb3, 0x80 };
static CVI_U8 data_st7701_480x640_15[] = { 0xb5, 0x4e };
static CVI_U8 data_st7701_480x640_16[] = { 0xb7, 0x85 };
static CVI_U8 data_st7701_480x640_17[] = { 0xb8, 0x20 };
static CVI_U8 data_st7701_480x640_18[] = { 0xc1, 0x78 };
static CVI_U8 data_st7701_480x640_19[] = { 0xc2, 0x78 };
static CVI_U8 data_st7701_480x640_20[] = { 0xd0, 0x88 };
static CVI_U8 data_st7701_480x640_21[] = { 0xe0, 0x00, 0x00, 0x02 };
static CVI_U8 data_st7701_480x640_22[] = {
	0xe1, 0x04, 0xA0, 0x06, 0xA0, 0x04, 0xA0, 0x05, 0xA0, 0x00,
	0x44, 0x44
};


static CVI_U8 data_st7701_480x640_23[] = {
	0xe2, 0x20, 0x20, 0x40, 0x40, 0x99, 0xA0, 0x00, 0xA0, 0x99,
	0xA0, 0x00, 0xA0, 0x00
};
static CVI_U8 data_st7701_480x640_24[] = { 0xe3, 0x00, 0x00, 0x22, 0x22 };
static CVI_U8 data_st7701_480x640_25[] = { 0xe4, 0x44, 0x44 };
static CVI_U8 data_st7701_480x640_26[] = {
	0xe5, 0x0E, 0x96, 0xA0, 0xA0, 0x10, 0x98, 0xA0, 0xA0, 0x0A,
	0x92, 0xA0, 0xA0, 0x0C, 0x94, 0xA0, 0xA0
};
static CVI_U8 data_st7701_480x640_27[] = { 0xe6, 0x00, 0x00, 0x22, 0x22 };
static CVI_U8 data_st7701_480x640_28[] = { 0xe7, 0x44, 0x44 };
static CVI_U8 data_st7701_480x640_29[] = {
	0xe8, 0x0D, 0x95, 0xA0, 0xA0, 0x0F, 0x97, 0xA0, 0xA0, 0x09,
	0x91, 0xA0, 0xA0, 0x0B, 0x93, 0xA0, 0xA0
};
static CVI_U8 data_st7701_480x640_30[] = { 0xe9, 0x02, 0x00, 0x00 };
static CVI_U8 data_st7701_480x640_31[] = {
	0xeb, 0x00, 0x00, 0x4E, 0x4E, 0x00, 0x00, 0x00
};
static CVI_U8 data_st7701_480x640_32[] = { 0xec, 0x02, 0x01 };
static CVI_U8 data_st7701_480x640_33[] = {
	0xed, 0xFF, 0xB1, 0xBA, 0x29, 0x84, 0x56, 0x7F, 0xFF, 0xFF,
	0xF7, 0x65, 0x48, 0x92, 0xAB, 0x1B, 0xFF
};
static CVI_U8 data_st7701_480x640_34[] = {
	0xef, 0x08, 0x08, 0x08, 0x45, 0x3F, 0x54
};
static CVI_U8 data_st7701_480x640_35[] = { 0x36, 0x00 };

static CVI_U8 data_st7701_480x640_36[] = { 0x29 };
static CVI_U8 data_st7701_480x640_37[] = { 0x35, 0x00 };

struct dsc_instr dsi_init_cmds_st7701_480x640[] = {
	{.delay = 120, .data_type = 0x05, .size = sizeof(data_st7701_480x640_0), .data = data_st7701_480x640_0 },
	{.delay = 0, .data_type = 0x29, .size = sizeof(data_st7701_480x640_1), .data = data_st7701_480x640_1 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_st7701_480x640_2), .data = data_st7701_480x640_2 },
	{.delay = 0, .data_type = 0x29, .size = sizeof(data_st7701_480x640_3), .data = data_st7701_480x640_3 },
	{.delay = 0, .data_type = 0x29, .size = sizeof(data_st7701_480x640_4), .data = data_st7701_480x640_4 },
	{.delay = 0, .data_type = 0x29, .size = sizeof(data_st7701_480x640_5), .data = data_st7701_480x640_5 },
	{.delay = 0, .data_type = 0x29, .size = sizeof(data_st7701_480x640_6), .data = data_st7701_480x640_6 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_st7701_480x640_7), .data = data_st7701_480x640_7 },
	{.delay = 0, .data_type = 0x29, .size = sizeof(data_st7701_480x640_8), .data = data_st7701_480x640_8 },
	{.delay = 0, .data_type = 0x29, .size = sizeof(data_st7701_480x640_9), .data = data_st7701_480x640_9 },
	{.delay = 0, .data_type = 0x29, .size = sizeof(data_st7701_480x640_10), .data = data_st7701_480x640_10 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_st7701_480x640_11), .data = data_st7701_480x640_11 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_st7701_480x640_12), .data = data_st7701_480x640_12 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_st7701_480x640_13), .data = data_st7701_480x640_13 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_st7701_480x640_14), .data = data_st7701_480x640_14 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_st7701_480x640_15), .data = data_st7701_480x640_15 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_st7701_480x640_16), .data = data_st7701_480x640_16 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_st7701_480x640_17), .data = data_st7701_480x640_17 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_st7701_480x640_18), .data = data_st7701_480x640_18 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_st7701_480x640_19), .data = data_st7701_480x640_19 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_st7701_480x640_20), .data = data_st7701_480x640_20 },
	{.delay = 0, .data_type = 0x29, .size = sizeof(data_st7701_480x640_21), .data = data_st7701_480x640_21 },
	{.delay = 0, .data_type = 0x29, .size = sizeof(data_st7701_480x640_22), .data = data_st7701_480x640_22 },
	{.delay = 0, .data_type = 0x29, .size = sizeof(data_st7701_480x640_23), .data = data_st7701_480x640_23 },
	{.delay = 0, .data_type = 0x29, .size = sizeof(data_st7701_480x640_24), .data = data_st7701_480x640_24 },
	{.delay = 0, .data_type = 0x29, .size = sizeof(data_st7701_480x640_25), .data = data_st7701_480x640_25 },
	{.delay = 0, .data_type = 0x29, .size = sizeof(data_st7701_480x640_26), .data = data_st7701_480x640_26 },
	{.delay = 0, .data_type = 0x29, .size = sizeof(data_st7701_480x640_27), .data = data_st7701_480x640_27 },
	{.delay = 0, .data_type = 0x29, .size = sizeof(data_st7701_480x640_28), .data = data_st7701_480x640_28 },
	{.delay = 0, .data_type = 0x29, .size = sizeof(data_st7701_480x640_29), .data = data_st7701_480x640_29 },
	{.delay = 0, .data_type = 0x29, .size = sizeof(data_st7701_480x640_30), .data = data_st7701_480x640_30 },
	{.delay = 0, .data_type = 0x29, .size = sizeof(data_st7701_480x640_31), .data = data_st7701_480x640_31 },
	{.delay = 0, .data_type = 0x29, .size = sizeof(data_st7701_480x640_32), .data = data_st7701_480x640_32 },
	{.delay = 0, .data_type = 0x29, .size = sizeof(data_st7701_480x640_33), .data = data_st7701_480x640_33 },
	{.delay = 0, .data_type = 0x29, .size = sizeof(data_st7701_480x640_34), .data = data_st7701_480x640_34 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_st7701_480x640_35), .data = data_st7701_480x640_35 },
	{.delay = 120, .data_type = 0x05, .size = sizeof(data_st7701_480x640_36), .data = data_st7701_480x640_36 },
	{.delay = 0, .data_type = 0x15, .size = sizeof(data_st7701_480x640_37), .data = data_st7701_480x640_37 }
};

#else
#error "MIPI_TX_PARAM multi-delcaration!!"
#endif // _MIPI_TX_PARAM_ST_7701_480X640_H_

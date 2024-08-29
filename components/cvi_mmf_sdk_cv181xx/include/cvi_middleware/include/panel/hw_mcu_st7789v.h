#ifndef _MCU_PARAM_ST7789V_H_
#define _MCU_PARAM_ST7789V_H_

#include <cvi_comm_hw_i80.h>

#define RESET_DELAY (10 * 1000)

#define PACKET_DCS 0x29
#define COMMAND 0
#define DATA    1

#define data_st7789_xmz200_2  0x11
#define data_st7789_xmz200_3  0x36
#define data_st7789_xmz200_4  0x00 //0x80
#define data_st7789_xmz200_5  0x35
#define data_st7789_xmz200_6  0x00
#define data_st7789_xmz200_7  0x3A
#define data_st7789_xmz200_8  0x55
#define data_st7789_xmz200_9  0x21
#define data_st7789_xmz200_10  0xB2
#define data_st7789_xmz200_11  0x05
#define data_st7789_xmz200_12  0x05
#define data_st7789_xmz200_13  0x00
#define data_st7789_xmz200_14  0x33
#define data_st7789_xmz200_15  0x33
#define data_st7789_xmz200_16  0xB7
#define data_st7789_xmz200_17  0x75
#define data_st7789_xmz200_18  0xBB
#define data_st7789_xmz200_19  0x22 //Vcom=1.35V
#define data_st7789_xmz200_20  0xC0
#define data_st7789_xmz200_21  0x2C
#define data_st7789_xmz200_22  0xC2
#define data_st7789_xmz200_23  0x01
#define data_st7789_xmz200_24  0xC3
#define data_st7789_xmz200_25  0x13 //如太亮可改为15
#define data_st7789_xmz200_26  0xC4
#define data_st7789_xmz200_27  0x20 //VDV, 0x20:0v
#define data_st7789_xmz200_28  0xC6
#define data_st7789_xmz200_29  0x11 //0x0F:60Hz
#define data_st7789_xmz200_30  0xD0
#define data_st7789_xmz200_31  0xA4
#define data_st7789_xmz200_32  0xA1
#define data_st7789_xmz200_33  0xE0
#define data_st7789_xmz200_34  0xD0
#define data_st7789_xmz200_35  0x05
#define data_st7789_xmz200_36  0x0A
#define data_st7789_xmz200_37  0x09
#define data_st7789_xmz200_38  0x08
#define data_st7789_xmz200_39  0x05
#define data_st7789_xmz200_40  0x2e
#define data_st7789_xmz200_41  0x44
#define data_st7789_xmz200_42  0x45
#define data_st7789_xmz200_43  0x0f
#define data_st7789_xmz200_44  0x17
#define data_st7789_xmz200_45  0x16
#define data_st7789_xmz200_46  0x2b
#define data_st7789_xmz200_47  0x33
#define data_st7789_xmz200_48  0xE1
#define data_st7789_xmz200_49  0xd0
#define data_st7789_xmz200_50  0x05
#define data_st7789_xmz200_51  0x0a
#define data_st7789_xmz200_52  0x09
#define data_st7789_xmz200_53  0x08
#define data_st7789_xmz200_54  0x05
#define data_st7789_xmz200_55  0x2e
#define data_st7789_xmz200_56  0x44
#define data_st7789_xmz200_57  0x45
#define data_st7789_xmz200_58  0x0f
#define data_st7789_xmz200_59  0x17
#define data_st7789_xmz200_60  0x16
#define data_st7789_xmz200_61  0x2B
#define data_st7789_xmz200_62  0x33
#define data_st7789_xmz200_63  0x2A
#define data_st7789_xmz200_64  0x00
#define data_st7789_xmz200_65  0x00
#define data_st7789_xmz200_66  0x00
#define data_st7789_xmz200_67  0xEF  //240
#define data_st7789_xmz200_68  0x2B
#define data_st7789_xmz200_69  0x00
#define data_st7789_xmz200_70  0x00
#define data_st7789_xmz200_71  0x01
#define data_st7789_xmz200_72  0x3F  //320
#define data_st7789_xmz200_73  0x29
#define data_st7789_xmz200_74  0x2C

HW_I80_CFG_S st7789vCfg = {
	.pins = {
		.pin_num = 11,
		.d_pins = {
			{VO_MIPI_TXM4, VO_MUX_MCU_CS},
			{VO_MIPI_TXP4, VO_MUX_MCU_RS},
			{VO_MIPI_TXM3, VO_MUX_MCU_DATA1},
			{VO_MIPI_TXP3, VO_MUX_MCU_DATA0},
			{VO_MIPI_TXM2, VO_MUX_MCU_DATA3},
			{VO_MIPI_TXP2, VO_MUX_MCU_DATA2},
			{VO_MIPI_TXM1, VO_MUX_MCU_DATA5},
			{VO_MIPI_TXP1, VO_MUX_MCU_DATA4},
			{VO_MIPI_TXM0, VO_MUX_MCU_DATA7},
			{VO_MIPI_TXP0, VO_MUX_MCU_DATA6},
			{VO_MIPI_RXP5, VO_MUX_MCU_WR},
		}
	},
	.mode = VO_MCU_MODE_RGB565,
	.instrs = {
		.instr_num = 73,
		.instr_cmd = {
			{.delay = 120, .data_type = COMMAND, .data =  data_st7789_xmz200_2},
      {.delay = 0,   .data_type = COMMAND, .data =  data_st7789_xmz200_3},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_4},
      {.delay = 0,   .data_type = COMMAND, .data =  data_st7789_xmz200_5},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_6},
      {.delay = 0,   .data_type = COMMAND, .data =  data_st7789_xmz200_7},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_8},
      {.delay = 0,   .data_type = COMMAND, .data =  data_st7789_xmz200_9},
      {.delay = 0,   .data_type = COMMAND, .data =  data_st7789_xmz200_10},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_11},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_12},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_13},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_14},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_15},
      {.delay = 0,   .data_type = COMMAND, .data =  data_st7789_xmz200_16},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_17},
      {.delay = 0,   .data_type = COMMAND, .data =  data_st7789_xmz200_18},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_19},
      {.delay = 0,   .data_type = COMMAND, .data =  data_st7789_xmz200_20},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_21},
      {.delay = 0,   .data_type = COMMAND, .data =  data_st7789_xmz200_22},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_23},
      {.delay = 0,   .data_type = COMMAND, .data =  data_st7789_xmz200_24},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_25},
      {.delay = 0,   .data_type = COMMAND, .data =  data_st7789_xmz200_26},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_27},
      {.delay = 0,   .data_type = COMMAND, .data =  data_st7789_xmz200_28},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_29},
      {.delay = 0,   .data_type = COMMAND, .data =  data_st7789_xmz200_30},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_31},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_32},
      {.delay = 0,   .data_type = COMMAND, .data =  data_st7789_xmz200_33},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_34},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_35},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_36},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_37},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_38},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_39},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_40},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_41},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_42},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_43},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_44},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_45},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_46},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_47},
      {.delay = 0,   .data_type = COMMAND, .data =  data_st7789_xmz200_48},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_49},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_50},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_51},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_52},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_53},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_54},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_55},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_56},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_57},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_58},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_59},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_60},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_61},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_62},
      {.delay = 0,   .data_type = COMMAND, .data =  data_st7789_xmz200_63},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_64},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_65},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_66},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_67},
      {.delay = 0,   .data_type = COMMAND, .data =  data_st7789_xmz200_68},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_69},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_70},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_71},
      {.delay = 0,   .data_type = DATA, .data =  data_st7789_xmz200_72},
      {.delay = 20,  .data_type = COMMAND, .data =  data_st7789_xmz200_73},
      {.delay = 0,   .data_type = COMMAND, .data =  data_st7789_xmz200_74},
		}
	},
	.sync_info = {
		.vid_hsa_pixels = 2,
		.vid_hbp_pixels = 0,
		.vid_hfp_pixels = 16,
		.vid_hline_pixels = 240,
		.vid_vsa_lines = 2,
		.vid_vbp_lines = 0,
		.vid_vfp_lines = 32,
		.vid_active_lines = 320,
		.vid_vsa_pos_polarity = false,
		.vid_hsa_pos_polarity = false,
	},
	.u16FrameRate = 60,
};

#endif // _MCU_PARAM_ST7789V_H_

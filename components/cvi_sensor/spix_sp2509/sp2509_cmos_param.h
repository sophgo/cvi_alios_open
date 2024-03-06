#ifndef __SP2509_CMOS_PARAM_H_
#define __SP2509_CMOS_PARAM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "cif_uapi.h"
#include "cvi_sns_ctrl.h"
#include "cvi_type.h"
#include "sp2509_cmos_ex.h"

static const SP2509_MODE_S g_astSp2509_mode[SP2509_MODE_NUM] = {
  [SP2509_MODE_1600X1200P30] =
	  {
		.name = "1600X1200P30",
		.astImg[0] =
			{
			  .stSnsSize =
				  {
					.u32Width = 1600,
					.u32Height = 1200,
				  },
			  .stWndRect =
				  {
					.s32X = 0,
					.s32Y = 0,
					.u32Width = 1600,
					.u32Height = 1200,
				  },
			  .stMaxSize =
				  {
					.u32Width = 1600,
					.u32Height = 1200,
				  },
			},
		.f32MaxFps = 30,
		.f32MinFps = 2.32, /* VtsDef * 30 / 0x3FFF */
		.u32HtsDef = 0x402,
		.u32VtsDef = 0x4f2,
		.stExp[0] =
			{
			  .u16Min = 1,
			  .u16Max = 0xffff,
			  .u16Def = 0x168,
			  .u16Step = 1,
			},
		.stAgain[0] =
			{
			  .u32Min = 1024,
			  .u32Max = 15872,  // 15.5 * 1024
			  .u32Def = 2048,   // 2 * 1024
			  .u32Step = 1,
			},
		.stDgain[0] =
			{
			  .u32Min = 1024,
			  .u32Max = 1024,
			  .u32Def = 1024,
			  .u32Step = 1,
			},
	  },
  [SP2509_MODE_800X600P30] =
	  {
		.name = "800X600P30",
		.astImg[0] =
			{
			  .stSnsSize =
				  {
					.u32Width = 800,
					.u32Height = 600,
				  },
			  .stWndRect =
				  {
					.s32X = 0,
					.s32Y = 0,
					.u32Width = 800,
					.u32Height = 600,
				  },
			  .stMaxSize =
				  {
					.u32Width = 800,
					.u32Height = 600,
				  },
			},
		.f32MaxFps = 30,
		.f32MinFps = 2.34, /* u32VtsDef * 30 / 0x3FFF */
		.u32HtsDef = 1766,
		.u32VtsDef = 634,
		.stExp[0] =
			{
			  .u16Min = 2,
			  .u16Max = 634 - 8,
			  .u16Def = 100,
			  .u16Step = 1,
			},
		.stAgain[0] =
			{
			  .u32Min = 1024,
			  .u32Max = 16 * 646,
			  .u32Def = 1024,
			  .u32Step = 1,
			},
		.stDgain[0] =
			{
			  .u32Min = 1024,
			  .u32Max = 1024,
			  .u32Def = 1024,
			  .u32Step = 1,
			},
	  },
};

static ISP_CMOS_BLACK_LEVEL_S g_stIspBlcCalibratio = {
  .bUpdate = CVI_TRUE,
  .blcAttr =
	  {
		.Enable = 1,
		.enOpType = OP_TYPE_AUTO,
		.stManual = {260, 260, 260, 260, 0, 0, 0, 0},
		.stAuto =
			{
			  {260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260},
			  {260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260},
			  {260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260},
			  {260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260, 260},
			  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			},
	  },
};

struct combo_dev_attr_s sp2509_rx_attr = {
  .input_mode = INPUT_MODE_MIPI,
  .mac_clk = RX_MAC_CLK_200M,
  .mipi_attr =
	  {
		.raw_data_type = RAW_DATA_10BIT,
		.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
	  },
  .mclk =
	  {
		.cam = 0,
		.freq = CAMPLL_FREQ_27M,
	  },
  .devno = 0,
};

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif /* __SP2509_CMOS_PARAM_H_ */

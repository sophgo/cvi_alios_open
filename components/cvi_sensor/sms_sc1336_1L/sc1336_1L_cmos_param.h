#ifndef __SC1336_1L_CMOS_PARAM_H_
#define __SC1336_1L_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "cvi_comm_cif.h"
#include "cvi_type.h"
#include "cvi_sns_ctrl.h"
#include "sc1336_1L_cmos_ex.h"

static const SC1336_1L_MODE_S g_astSC1336_1L_mode[SC1336_1L_MODE_NUM] = {
	[SC1336_1L_MODE_720P30] = {
		.name = "720p30",
		.stImg[0] = {
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
		.f32MaxFps = 30,
		.f32MinFps = 1.37, /* 720 * 30 / 0x3FFF*/
		.u32HtsDef = 1280,
		.u32VtsDef = 780,
		.stExp = {
			.u16Min = 2,
			.u16Max = 1492,/* 2*vts-8 */
			.u16Def = 100,
			.u16Step = 1,
		},
		.stAgain = {
			.u32Min = 1024,
			.u32Max = 65536, /* 64x1024 */
			.u32Def = 1024,
			.u32Step = 4,
		},
		.stDgain = {
			.u32Min = 1024,
			.u32Max = 32256, /* 31.5x1024 */
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[SC1336_1L_MODE_720P30_WDR] = {
		.name = "720p30wdr",
		.stImg[0] = {
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
		.stImg[1] = {
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
		.f32MaxFps = 30,
		.f32MinFps = 1.37, /* 720 * 30 / 0x3FFF*/
		.u32HtsDef = 1280,
		.u32VtsDef = 780,
		.u16SexpMaxReg = 0x13E,
	},
	[SC1336_1L_MODE_720P60] = {
		.name = "720p60",
		.stImg[0] = {
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
		.f32MaxFps = 60,
		.f32MinFps = 2.74, /* 750 * 60 / 0x3FFF*/
		.u32HtsDef = 1280,
		.u32VtsDef = 780,
		.stExp = {
			.u16Min = 2,
			.u16Max = 750 - 6,
			.u16Def = 100,
			.u16Step = 1,
		},
		.stAgain = {
			.u32Min = 1024,
			.u32Max = 32768,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain = {
			.u32Min = 1024,
			.u32Max = 4032,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[SC1336_1L_MODE_720P60_WDR] = {
		.name = "720p60wdr",
		.stImg[0] = {
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
		.stImg[1] = {
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
		.f32MaxFps = 60,
		.f32MinFps = 1.37, /* 750 * 30 / 0x3FFF*/
		.u32HtsDef = 1280,
		.u32VtsDef = 780,
		.u16SexpMaxReg = 0x13E,
	},
};

static ISP_CMOS_NOISE_CALIBRATION_S g_stIspNoiseCalibratio = {.CalibrationCoef = {
	{	//iso  100
		{0.02792946062982082367,	3.36534714698791503906}, //B: slope, intercept
		{0.02071751467883586884,	5.34583568572998046875}, //Gb: slope, intercept
		{0.02110148966312408447,	5.02954530715942382813}, //Gr: slope, intercept
		{0.02168512716889381409,	4.89776754379272460938}, //R: slope, intercept
	},
	{	//iso  200
		{0.03194080293178558350,	5.61192893981933593750}, //B: slope, intercept
		{0.02428408525884151459,	7.94834280014038085938}, //Gb: slope, intercept
		{0.02499442733824253082,	7.72430133819580078125}, //Gr: slope, intercept
		{0.02584112435579299927,	7.20574426651000976563}, //R: slope, intercept
	},
	{	//iso  400
		{0.04612467437982559204,	6.88752269744873046875}, //B: slope, intercept
		{0.03022909909486770630,	11.05101776123046875000}, //Gb: slope, intercept
		{0.03175539523363113403,	10.60332489013671875000}, //Gr: slope, intercept
		{0.03522306308150291443,	9.36425399780273437500}, //R: slope, intercept
	},
	{	//iso  800
		{0.06092500314116477966,	9.79670524597167968750}, //B: slope, intercept
		{0.03984217345714569092,	15.30182266235351562500}, //Gb: slope, intercept
		{0.04019560664892196655,	14.93132972717285156250}, //Gr: slope, intercept
		{0.04470816254615783691,	13.26843166351318359375}, //R: slope, intercept
	},
	{	//iso  1600
		{0.08295634388923645020,	14.20334625244140625000}, //B: slope, intercept
		{0.05075264349579811096,	20.99221038818359375000}, //Gb: slope, intercept
		{0.05426201224327087402,	20.08068656921386718750}, //Gr: slope, intercept
		{0.05945669487118721008,	19.02898788452148437500}, //R: slope, intercept
	},
	{	//iso  3200
		{0.09782519936561584473,	21.84967994689941406250}, //B: slope, intercept
		{0.06690908223390579224,	26.53993988037109375000}, //Gb: slope, intercept
		{0.06954573839902877808,	25.74129104614257812500}, //Gr: slope, intercept
		{0.09061723947525024414,	22.98998260498046875000}, //R: slope, intercept
	},
	{	//iso  6400
		{0.14311420917510986328,	28.96467971801757812500}, //B: slope, intercept
		{0.08148498833179473877,	37.93062591552734375000}, //Gb: slope, intercept
		{0.08273542672395706177,	38.37096405029296875000}, //Gr: slope, intercept
		{0.12093253433704376221,	33.31475067138671875000}, //R: slope, intercept
	},
	{	//iso  12800
		{0.17958122491836547852,	43.49506759643554687500}, //B: slope, intercept
		{0.09839969873428344727,	55.43268966674804687500}, //Gb: slope, intercept
		{0.10201884806156158447,	52.97607040405273437500}, //Gr: slope, intercept
		{0.15302789211273193359,	47.54779434204101562500}, //R: slope, intercept
	},
	{	//iso  25600
		{0.25833165645599365234,	56.96470642089843750000}, //B: slope, intercept
		{0.13260601460933685303,	74.69016265869140625000}, //Gb: slope, intercept
		{0.14035490155220031738,	75.44366455078125000000}, //Gr: slope, intercept
		{0.23465165495872497559,	60.52228164672851562500}, //R: slope, intercept
	},
	{	//iso  51200
		{0.37595292925834655762,	78.54853057861328125000}, //B: slope, intercept
		{0.21475413441658020020,	102.12300872802734375000}, //Gb: slope, intercept
		{0.20840260386466979980,	103.65763854980468750000}, //Gr: slope, intercept
		{0.34428051114082336426,	87.83551025390625000000}, //R: slope, intercept
	},
	{	//iso  102400
		{0.51122575998306274414,	113.49224090576171875000}, //B: slope, intercept
		{0.29245173931121826172,	154.26939392089843750000}, //Gb: slope, intercept
		{0.31501635909080505371,	148.29017639160156250000}, //Gr: slope, intercept
		{0.47034618258476257324,	124.06208038330078125000}, //R: slope, intercept
	},
	{	//iso  204800
		{0.67213481664657592773,	134.71751403808593750000}, //B: slope, intercept
		{0.40368056297302246094,	189.80801391601562500000}, //Gb: slope, intercept
		{0.43581819534301757813,	186.44682312011718750000}, //Gr: slope, intercept
		{0.60127359628677368164,	160.66384887695312500000}, //R: slope, intercept
	},
	{	//iso  409600
		{0.81907004117965698242,	103.53753662109375000000}, //B: slope, intercept
		{0.56758689880371093750,	134.64016723632812500000}, //Gb: slope, intercept
		{0.60227775573730468750,	125.39395904541015625000}, //Gr: slope, intercept
		{0.76318585872650146484,	111.18676757812500000000}, //R: slope, intercept
	},
	{	//iso  819200
		{0.81907004117965698242,	103.53753662109375000000}, //B: slope, intercept
		{0.56758689880371093750,	134.64016723632812500000}, //Gb: slope, intercept
		{0.60227775573730468750,	125.39395904541015625000}, //Gr: slope, intercept
		{0.76318585872650146484,	111.18676757812500000000}, //R: slope, intercept
	},
	{	//iso  1638400
		{0.81907004117965698242,	103.53753662109375000000}, //B: slope, intercept
		{0.56758689880371093750,	134.64016723632812500000}, //Gb: slope, intercept
		{0.60227775573730468750,	125.39395904541015625000}, //Gr: slope, intercept
		{0.76318585872650146484,	111.18676757812500000000}, //R: slope, intercept
	},
	{	//iso  3276800
		{0.81907004117965698242,	103.53753662109375000000}, //B: slope, intercept
		{0.56758689880371093750,	134.64016723632812500000}, //Gb: slope, intercept
		{0.60227775573730468750,	125.39395904541015625000}, //Gr: slope, intercept
		{0.76318585872650146484,	111.18676757812500000000}, //R: slope, intercept
	},
} };

static ISP_CMOS_BLACK_LEVEL_S g_stIspBlcCalibratio = {
	.bUpdate = CVI_TRUE,
	.blcAttr = {
		.Enable = 1,
		.enOpType = OP_TYPE_AUTO,
		.stManual = {
			260, 260, 260, 260, 0, 0, 0, 0
#ifdef ARCH_CV182X
			, 1093, 1093, 1093, 1093
#endif
		},
		.stAuto = {
			{260, 260, 260, 260, 260, 260, 260, 260, /*8*/260, 260, 260, 260, 260, 260, 260, 260},
			{260, 260, 260, 260, 260, 260, 260, 260, /*8*/260, 260, 260, 260, 260, 260, 260, 260},
			{260, 260, 260, 260, 260, 260, 260, 260, /*8*/260, 260, 260, 260, 260, 260, 260, 260},
			{260, 260, 260, 260, 260, 260, 260, 260, /*8*/260, 260, 260, 260, 260, 260, 260, 260},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
#ifdef ARCH_CV182X
			{1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093,
				/*8*/1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093},
			{1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093,
				/*8*/1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093},
			{1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093,
				/*8*/1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093},
			{1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093,
				/*8*/1093, 1093, 1093, 1093, 1093, 1093, 1093, 1093},
#endif
		},
	},
};

struct combo_dev_attr_s sc1336_1l_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_200M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {2, 3, -1, -1, -1},
		.pn_swap = {1, 1, 0, 0, 0},
		.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
		.dphy = {
			.enable = 1,
			.hs_settle = 11,
		},
	},
	.mclk = {
		.cam = 0,
		.freq = CAMPLL_FREQ_24M,
	},
	.devno = 0,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __SC1336_1L_CMOS_PARAM_H_ */

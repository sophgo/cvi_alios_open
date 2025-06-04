#ifndef __IMX415_CMOS_PARAM_H_
#define __IMX415_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"
#include "imx415_cmos_ex.h"

static const IMX415_MODE_S g_astImx415_mode[IMX415_MODE_NUM] = {
	[IMX415_MODE_4M25] = {
		.name = "4M25",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2568,
				.u32Height = 1440,
			},
			.stWndRect = {
				.s32X = 4,
				.s32Y = 0,
				.u32Width = 2560,
				.u32Height = 1440,
			},
			.stMaxSize = {
				.u32Width = 2568,
				.u32Height = 1440,
			},
		},
		.f32MaxFps = 25,
		.f32MinFps = 0.0,
		.u32HtsDef = 0x044c,
		.u32VtsDef = 0x0a8e, // 25fps
		.stExp[0] = {
			.u16Min = 8,
			.u16Max = 0x0a8e - 4,
			.u16Def = 8,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 32381, // 30db
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 128913, // 42db
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[IMX415_MODE_8M25] = {
		.name = "8M25",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 3864,
				.u32Height = 2192,
			},
			.stWndRect = {
				.s32X = 12,
				.s32Y = 16,
				.u32Width = 3840,
				.u32Height = 2160,
			},
			.stMaxSize = {
				.u32Width = 3864,
				.u32Height = 2192,
			},
		},
		.f32MaxFps = 25,
		.f32MinFps = 0.0,
		.u32HtsDef = 0x044c,
		.u32VtsDef = 0x0a8e,
		.stExp[0] = {
			.u16Min = 8,
			.u16Max = 0x0a8e - 4,
			.u16Def = 8,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 32381, // 30db
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 128913, // 42db
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[IMX415_MODE_8M30] = {
		.name = "8M30",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 3864,
				.u32Height = 2192,
			},
			.stWndRect = {
				.s32X = 12,
				.s32Y = 16,
				.u32Width = 3840,
				.u32Height = 2160,
			},
			.stMaxSize = {
				.u32Width = 3864,
				.u32Height = 2192,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 0.0,
		.u32HtsDef = 0x0215,
		.u32VtsDef = 0x1197,
		.stExp[0] = {
			.u16Min = 8,
			.u16Max = 0x1197 - 4,
			.u16Def = 8,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 32381, // 30db
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 128913, // 42db
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[IMX415_MODE_5M25] = {
		.name = "5M25",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2568,
				.u32Height = 2160,
			},
			.stWndRect = {
				.s32X = 4,
				.s32Y = 0,
				.u32Width = 2560,
				.u32Height = 2160,
			},
			.stMaxSize = {
				.u32Width = 2568,
				.u32Height = 2160,
			},
		},
		.f32MaxFps = 25,
		.f32MinFps = 0.0,
		.u32HtsDef = 0x044c,
		.u32VtsDef = 0x0a8e,
		.stExp[0] = {
			.u16Min = 8,
			.u16Max = 0x0a8e - 4,
			.u16Def = 8,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 32381, // 30db
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 128913, // 42db
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[IMX415_MODE_2M60] = {
		.name = "2M60",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 1944,
				.u32Height = 1097,
			},
			.stWndRect = {
				.s32X = 6,
				.s32Y = 11,
				.u32Width = 1920,
				.u32Height = 1080,
			},
			.stMaxSize = {
				.u32Width = 1944,
				.u32Height = 1097,
			},
		},
		.f32MaxFps = 60,
		.f32MinFps = 0.0,
		.u32HtsDef = 0x021E,
		.u32VtsDef = 0x08EB,
		.stExp[0] = {
			.u16Min = 4,
			.u16Max = 0x08EB - 8,
			.u16Def = 400,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 32381, // 30db
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 128913, // 42db
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
	[IMX415_MODE_4M25_WDR] = {
		.name = "4M25_WDR",
		/* sef */
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2616,
				.u32Height = 1964,
			},
			.stWndRect = {
				.s32X = 12,
				.s32Y = 12,
				.u32Width = 2592,
				.u32Height = 1944,
			},
			.stMaxSize = {
				.u32Width = 2616,
				.u32Height = 1964,
			},
		},
		/* lef */
		.astImg[1] = {
			.stSnsSize = {
				.u32Width = 2616,
				.u32Height = 1964,
			},
			.stWndRect = {
				.s32X = 12,
				.s32Y = 12,
				.u32Width = 2592,
				.u32Height = 1944,
			},
			.stMaxSize = {
				.u32Width = 2616,
				.u32Height = 1964,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 0.13, /* 0x1194 * 30 / 0xFFFFF */
		.u32HtsDef = 0x0898,
		.u32VtsDef = 0x1194,
		.stExp[0] = {
			.u16Min = 8,
			.u16Max = 481,
			.u16Def = 8,
			.u16Step = 1,
		},
		.stExp[1] = {
			.u16Min = 128,
			.u16Max = 7696,
			.u16Def = 128,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u32Min = 1024,
			.u32Max = 62416,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stAgain[1] = {
			.u32Min = 1024,
			.u32Max = 62416,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[0] = {
			.u32Min = 1024,
			.u32Max = 38485,
			.u32Def = 1024,
			.u32Step = 1,
		},
		.stDgain[1] = {
			.u32Min = 1024,
			.u32Max = 38485,
			.u32Def = 1024,
			.u32Step = 1,
		},
	},
};

static ISP_CMOS_NOISE_CALIBRATION_S g_stIspNoiseCalibratio = {.CalibrationCoef = {
	{	//iso  100
		{0.02535128779709339142,	1.61768496036529541016}, //B: slope, intercept
		{0.00795035623013973236,	15.21613883972167968750}, //Gb: slope, intercept
		{0.01330664381384849548,	10.40423774719238281250}, //Gr: slope, intercept
		{0.02679967880249023438,	1.82649695873260498047}, //R: slope, intercept
	},
	{	//iso  200
		{0.02753821015357971191,	6.47844934463500976563}, //B: slope, intercept
		{0.00463790073990821838,	25.24199485778808593750}, //Gb: slope, intercept
		{0.00451094703748822212,	25.23709869384765625000}, //Gr: slope, intercept
		{0.02894908934831619263,	6.20713567733764648438}, //R: slope, intercept
	},
	{	//iso  400
		{0.03178063780069351196,	12.60536384582519531250}, //B: slope, intercept
		{0.00517552718520164490,	35.03299713134765625000}, //Gb: slope, intercept
		{0.00502182589843869209,	35.23723602294921875000}, //Gr: slope, intercept
		{0.03328817337751388550,	12.19208049774169921875}, //R: slope, intercept
	},
	{	//iso  800
		{0.03747911751270294189,	21.80648422241210937500}, //B: slope, intercept
		{0.00625439779832959175,	49.06682205200195312500}, //Gb: slope, intercept
		{0.00620671268552541733,	49.14254379272460937500}, //Gr: slope, intercept
		{0.03955777361989021301,	20.63272857666015625000}, //R: slope, intercept
	},
	{	//iso  1600
		{0.04763090237975120544,	34.16785049438476562500}, //B: slope, intercept
		{0.00715198600664734840,	70.35113525390625000000}, //Gb: slope, intercept
		{0.00717207882553339005,	70.42241668701171875000}, //Gr: slope, intercept
		{0.05030791088938713074,	32.31000518798828125000}, //R: slope, intercept
	},
	{	//iso  3200
		{0.06291828304529190063,	51.79273986816406250000}, //B: slope, intercept
		{0.00942948460578918457,	99.58425903320312500000}, //Gb: slope, intercept
		{0.00938474666327238083,	99.87895202636718750000}, //Gr: slope, intercept
		{0.06596329063177108765,	49.59438705444335937500}, //R: slope, intercept
	},
	{	//iso  6400
		{0.08704897761344909668,	76.54403686523437500000}, //B: slope, intercept
		{0.02136226370930671692,	135.76541137695312500000}, //Gb: slope, intercept
		{0.02061788551509380341,	136.24209594726562500000}, //Gr: slope, intercept
		{0.09058564901351928711,	73.56949615478515625000}, //R: slope, intercept
	},
	{	//iso  12800
		{0.11864978075027465820,	116.15978240966796875000}, //B: slope, intercept
		{0.03285352513194084167,	193.26387023925781250000}, //Gb: slope, intercept
		{0.03131035342812538147,	194.77830505371093750000}, //Gr: slope, intercept
		{0.12536112964153289795,	110.30144500732421875000}, //R: slope, intercept
	},
	{	//iso  25600
		{0.16936406493186950684,	172.24114990234375000000}, //B: slope, intercept
		{0.05775514617562294006,	267.55535888671875000000}, //Gb: slope, intercept
		{0.05725358799099922180,	268.19198608398437500000}, //Gr: slope, intercept
		{0.17778857052326202393,	166.38156127929687500000}, //R: slope, intercept
	},
	{	//iso  51200
		{0.23955665528774261475,	255.52276611328125000000}, //B: slope, intercept
		{0.09076436609029769897,	378.79702758789062500000}, //Gb: slope, intercept
		{0.08728235960006713867,	384.05020141601562500000}, //Gr: slope, intercept
		{0.25822344422340393066,	249.06506347656250000000}, //R: slope, intercept
	},
	{	//iso  102400
		{0.36010745167732238770,	362.82952880859375000000}, //B: slope, intercept
		{0.15752448141574859619,	513.11077880859375000000}, //Gb: slope, intercept
		{0.15595595538616180420,	518.77105712890625000000}, //Gr: slope, intercept
		{0.38190475106239318848,	360.52606201171875000000}, //R: slope, intercept
	},
	{	//iso  204800
		{0.44082218408584594727,	536.35937500000000000000}, //B: slope, intercept
		{0.22994761168956756592,	701.87817382812500000000}, //Gb: slope, intercept
		{0.22570468485355377197,	707.03558349609375000000}, //Gr: slope, intercept
		{0.46145606040954589844,	543.70227050781250000000}, //R: slope, intercept
	},
	{	//iso  409600
		{0.24378113448619842529,	865.24688720703125000000}, //B: slope, intercept
		{0.06541594862937927246,	1050.97998046875000000000}, //Gb: slope, intercept
		{0.06018084660172462463,	1068.13745117187500000000}, //Gr: slope, intercept
		{0.25154441595077514648,	904.18511962890625000000}, //R: slope, intercept
	},
	{	//iso  819200
		{0.24305926263332366943,	867.16644287109375000000}, //B: slope, intercept
		{0.06559922546148300171,	1050.18383789062500000000}, //Gb: slope, intercept
		{0.06018084660172462463,	1068.13745117187500000000}, //Gr: slope, intercept
		{0.25154441595077514648,	904.18511962890625000000}, //R: slope, intercept
	},
	{	//iso  1638400
		{0.24378113448619842529,	865.24688720703125000000}, //B: slope, intercept
		{0.06582942605018615723,	1048.96228027343750000000}, //Gb: slope, intercept
		{0.06089610978960990906,	1064.75537109375000000000}, //Gr: slope, intercept
		{0.25244551897048950195,	901.38049316406250000000}, //R: slope, intercept
	},
	{	//iso  3276800
		{0.24428291618824005127,	863.30468750000000000000}, //B: slope, intercept
		{0.06582942605018615723,	1048.96228027343750000000}, //Gb: slope, intercept
		{0.06080029532313346863,	1065.15185546875000000000}, //Gr: slope, intercept
		{0.25226309895515441895,	901.83532714843750000000}, //R: slope, intercept
	},
} };

static ISP_CMOS_BLACK_LEVEL_S g_stIspBlcCalibratio = {
	.bUpdate = CVI_TRUE,
	.blcAttr = {
		.Enable = 1,
		.enOpType = OP_TYPE_AUTO,
		.stManual = {216, 216, 216, 216, 0, 0, 0, 0
#ifdef ARCH_CV182X
			, 1076, 1076, 1076, 1076
#endif
		},
		.stAuto = {
			{216, 216, 216, 216, 216, 216, 216, 216, /*8*/216, 216, 216, 216, 216, 216, 216, 216},
			{216, 216, 216, 216, 216, 216, 216, 216, /*8*/216, 216, 216, 216, 216, 216, 216, 216},
			{216, 216, 216, 216, 216, 216, 216, 216, /*8*/216, 216, 216, 216, 216, 216, 216, 216},
			{216, 216, 216, 216, 216, 216, 216, 216, /*8*/216, 216, 216, 216, 216, 216, 216, 216},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
#ifdef ARCH_CV182X
			{1076, 1077, 1077, 1077, 1077, 1077, 1077, 1078,
				/*8*/1082, 1096, 1138, 1228, 1365, 1365, 1365, 1365},
			{1077, 1077, 1077, 1077, 1077, 1076, 1076, 1076,
				/*8*/1077, 1088, 1122, 1196, 1344, 1344, 1347, 1347},
			{1077, 1077, 1077, 1077, 1077, 1077, 1077, 1076,
				/*8*/1078, 1090, 1126, 1203, 1358, 1359, 1360, 1360},
			{1076, 1077, 1077, 1077, 1077, 1077, 1077, 1079,
				/*8*/1082, 1096, 1137, 1226, 1365, 1365, 1365, 1365},
#endif
		},
	},
};

struct combo_dev_attr_s imx415_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_900M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_12BIT,
		.lane_id = {4, 0, 1, 2, 3},
		.pn_swap = {0, 0, 0, 0, 0},
		.wdr_mode = CVI_MIPI_WDR_MODE_NONE,
		.dphy = {
			.enable = 1,
			.hs_settle = 12,
		},
	},
	.mclk = {
		.cam = 0,
		.freq = CAMPLL_FREQ_37P125M,
	},
	.devno = 0,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __IMX415_CMOS_PARAM_H_ */

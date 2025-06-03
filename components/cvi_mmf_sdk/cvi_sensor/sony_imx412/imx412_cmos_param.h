#ifndef __IMX412_CMOS_PARAM_H_
#define __IMX412_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"
#include "imx412_cmos_ex.h"

static const IMX412_MODE_S g_astImx412_mode[IMX412_MODE_NUM] = {
	[IMX412_MODE_12M30] = {
			.name = "IMX412_12M30",
			.astImg[0] = {
				.stSnsSize = {
					.u32Width = 4056,
					.u32Height = 3040,
				},
				.stWndRect = {
					.s32X = 12,
					.s32Y = 20,
					.u32Width = 4032,
					.u32Height = 3000,
				},
				.stMaxSize = {
					.u32Width = 4056,
					.u32Height = 3040,
				},
			},
			.f32MaxFps = 30,
			.f32MinFps = 0.07, /* 0x8CA * 30 / 0xFFFFF */
			.u32HtsDef = 0x4C4,
			.u32VtsDef = 0x8CA,
			.stExp[0] = {
				.u16Min = 5,
				.u16Max = 0x8CA - 1,
				.u16Def = 5,
				.u16Step = 1,
			},
			.stAgain[0] = {
				.u32Min = 1024,
				.u32Max = 32381,
				.u32Def = 1024,
				.u32Step = 1,
			},
			.stDgain[0] = {
				.u32Min = 1024,
				.u32Max = 128914,
				.u32Def = 1024,
				.u32Step = 1,
			},
		},
	[IMX412_MODE_12M30_WDR] = {
		.name = "IMX412_12M30_WDR",
		/* sef */
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 4056,
				.u32Height = 3040,
			},
			.stWndRect = {
				.s32X = 12,
				.s32Y = 20,
				.u32Width = 4032,
				.u32Height = 3000,
			},
			.stMaxSize = {
				.u32Width = 4056,
				.u32Height = 3040,
			},
		},
		/* lef */
		.astImg[1] = {
			.stSnsSize = {
				.u32Width = 4056,
				.u32Height = 3040,
			},
			.stWndRect = {
				.s32X = 12,
				.s32Y = 20,
				.u32Width = 4032,
				.u32Height = 3000,
			},
			.stMaxSize = {
				.u32Width = 4056,
				.u32Height = 3040,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 0.07, /* 0x8CA * 30 / 0xFFFFF */
		.u32HtsDef = 0x11A0,	// reg 0x0342 0x0343	LINE_LENGTH_PCK
		.u32VtsDef = 0xC1F,	// reg 0x0340 0x0341	FRM_LENGTH_LINES
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
		.u16BRL = 3079,
	},
	[IMX412_MODE_8M30_WDR] = {
		.name = "IMX412_8M30_WDR",
		/* sef */
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 4060,
				.u32Height = 2288,
			},
			.stWndRect = {
				.s32X = 14,
				.s32Y = 0,
				.u32Width = 4032,
				.u32Height = 2288,
			},
			.stMaxSize = {
				.u32Width = 4060,
				.u32Height = 2288,
			},
		},
		/* lef */
		.astImg[1] = {
			.stSnsSize = {
				.u32Width = 4060,
				.u32Height = 2288,
			},
			.stWndRect = {
				.s32X = 14,
				.s32Y = 0,
				.u32Width = 4032,
				.u32Height = 2288,
			},
			.stMaxSize = {
				.u32Width = 4060,
				.u32Height = 2288,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 0.07,
		.u32HtsDef = 0x11A0,	// reg 0x0342 0x0343	LINE_LENGTH_PCK
		.u32VtsDef = 0xC1D,	// reg 0x0340 0x0341	FRM_LENGTH_LINES
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
		.u16BRL = 2325,
	},

};

static ISP_CMOS_NOISE_CALIBRATION_S g_stIspNoiseCalibratio = {.CalibrationCoef = {
	{	//iso  100
		{0.02690107934176921844,	4.50223064422607421875}, //B: slope, intercept
		{0.02101177349686622620,	6.80538892745971679688}, //Gb: slope, intercept
		{0.02104613743722438812,	6.86702489852905273438}, //Gr: slope, intercept
		{0.02316275984048843384,	5.60397005081176757813}, //R: slope, intercept
	},
	{	//iso  200
		{0.03363476321101188660,	6.67160606384277343750}, //B: slope, intercept
		{0.02475655637681484222,	9.95575618743896484375}, //Gb: slope, intercept
		{0.02472607791423797607,	10.01574611663818359375}, //Gr: slope, intercept
		{0.02842517755925655365,	8.12014675140380859375}, //R: slope, intercept
	},
	{	//iso  400
		{0.04830884941271679688,	8.92552661895751953125}, //B: slope, intercept
		{0.03319310769438743591,	13.42267227172851562500}, //Gb: slope, intercept
		{0.03305986151099205017,	13.43060302734375000000}, //Gr: slope, intercept
		{0.03901003301143646240,	11.01109695434570312500}, //R: slope, intercept
	},
	{	//iso  800
		{0.06106014549732208252,	13.93724441528320312500}, //B: slope, intercept
		{0.04092391207814216614,	20.69492340087890625000}, //Gb: slope, intercept
		{0.04070277512073516846,	20.86536216735839843750}, //Gr: slope, intercept
		{0.04916029050946235657,	17.03823661804199218750}, //R: slope, intercept
	},
	{	//iso  1600
		{0.07344199717044830322,	24.47761344909667968750}, //B: slope, intercept
		{0.05544847622513771057,	32.42719268798828125000}, //Gb: slope, intercept
		{0.05420234426856040955,	33.45277023315429687500}, //Gr: slope, intercept
		{0.06873583048582077026,	26.04830932617187500000}, //R: slope, intercept
	},
	{	//iso  3200
		{0.09675583988428115845,	37.05550765991210937500}, //B: slope, intercept
		{0.07141858339309692383,	48.99865341186523437500}, //Gb: slope, intercept
		{0.07161217182874679565,	49.22959899902343750000}, //Gr: slope, intercept
		{0.09100358933210372925,	39.16574478149414062500}, //R: slope, intercept
	},
	{	//iso  6400
		{0.12762205302715301514,	57.20859146118164062500}, //B: slope, intercept
		{0.09197103977203369141,	75.74417114257812500000}, //Gb: slope, intercept
		{0.09082328528165817261,	77.31325531004129375000}, //Gr: slope, intercept
		{0.11900347471237182617,	60.56912612915039062500}, //R: slope, intercept
	},
	{	//iso  12800
		{0.18264676630496978760,	85.40633392333984375000}, //B: slope, intercept
		{0.12531952559947967529,	115.33919525146484375000}, //Gb: slope, intercept
		{0.12440066039562225342,	117.71653747541293750000}, //Gr: slope, intercept
		{0.16512213647365570068,	93.01001739501953125000}, //R: slope, intercept
	},
	{	//iso  25600
		{0.26012355089187622070,	128.12574768066406250000}, //B: slope, intercept
		{0.17714048922061920166,	173.11187744140625000000}, //Gb: slope, intercept
		{0.17438966035842895508,	178.83584594726562500000}, //Gr: slope, intercept
		{0.23661704361438751221,	139.58770751953125000000}, //R: slope, intercept
	},
	{	//iso  51200
		{0.40982273221015930176,	178.17179870605468750000}, //B: slope, intercept
		{0.26149463653564453125,	256.04803466796875000000}, //Gb: slope, intercept
		{0.25542837381362915039,	269.09127807617187500000}, //Gr: slope, intercept
		{0.36157473921775817871,	202.78886413574218750000}, //R: slope, intercept
	},
	{	//iso  102400
		{0.67492419481277465820,	220.58061218261718750000}, //B: slope, intercept
		{0.47528272867202758789,	311.01812744140625000000}, //Gb: slope, intercept
		{0.46269500255584716797,	331.09899902343750000000}, //Gr: slope, intercept
		{0.60283488035202026367,	257.87683105468750000000}, //R: slope, intercept
	},
	{	//iso  204800
		{0.84576326608657836914,	318.21844482421875000000}, //B: slope, intercept
		{0.67934745550155639648,	410.28952026367187500000}, //Gb: slope, intercept
		{0.63606011867523193359,	452.85031127929687500000}, //Gr: slope, intercept
		{0.77183848619461059570,	369.87619018554687500000}, //R: slope, intercept
	},
	{	//iso  409600
		{0.84576326608657836914,	318.21844482421875000000}, //B: slope, intercept
		{0.67934745550155639648,	410.28952026367187500000}, //Gb: slope, intercept
		{0.63606011867523193359,	452.85031127929687500000}, //Gr: slope, intercept
		{0.77183848619461059570,	369.87619018554687500000}, //R: slope, intercept
	},
	{	//iso  819200
		{0.84576326608657836914,	318.21844482421875000000}, //B: slope, intercept
		{0.67934745550155639648,	410.28952026367187500000}, //Gb: slope, intercept
		{0.63606011867523193359,	452.85031127929687500000}, //Gr: slope, intercept
		{0.77183848619461059570,	369.87619018554687500000}, //R: slope, intercept
	},
	{	//iso  1638400
		{0.84576326608657836914,	318.21844482421875000000}, //B: slope, intercept
		{0.67934745550155639648,	410.28952026367187500000}, //Gb: slope, intercept
		{0.63606011867523193359,	452.85031127929687500000}, //Gr: slope, intercept
		{0.77183848619461059570,	369.87619018554687500000}, //R: slope, intercept
	},
	{	//iso  3276800
		{0.84576326608657836914,	318.21844482421875000000}, //B: slope, intercept
		{0.67934745550155639648,	410.28952026367187500000}, //Gb: slope, intercept
		{0.63606011867523193359,	452.85031127929687500000}, //Gr: slope, intercept
		{0.77183848619461059570,	369.87619018554687500000}, //R: slope, intercept
	},
} };

static ISP_CMOS_BLACK_LEVEL_S g_stIspBlcCalibratio = {
	.bUpdate = CVI_TRUE,
	.blcAttr = {
		.Enable = 1,
		.enOpType = OP_TYPE_AUTO,
		.stManual = {200, 200, 200, 200, 1076, 1076, 1076, 1076},
		.stAuto = {
			{200, 201, 200, 201, 201, 202, 199, 200, /*8*/206, 233, 329, 599, 878, 893, 909, 925},
			{200, 201, 200, 201, 201, 202, 198, 200, /*8*/205, 232, 327, 598, 875, 890, 906, 920},
			{200, 200, 200, 201, 201, 201, 197, 197, /*8*/201, 222, 310, 564, 831, 847, 863, 873},
			{200, 201, 200, 201, 201, 201, 197, 198, /*8*/202, 223, 311, 565, 833, 851, 864, 874},
			{1077, 1077, 1077, 1077, 1077, 1077, 1076, 1077,
				/*8*/1078, 1086, 1113, 1199, 1303, 1310, 1316, 1323},
			{1077, 1077, 1077, 1077, 1077, 1077, 1076, 1077,
				/*8*/1078, 1085, 1113, 1199, 1302, 1308, 1315, 1321},
			{1077, 1077, 1077, 1077, 1077, 1077, 1076, 1076,
				/*8*/1077, 1083, 1108, 1188, 1285, 1291, 1297, 1301},
			{1077, 1077, 1077, 1077, 1077, 1077, 1076, 1076,
				/*8*/1077, 1083, 1108, 1188, 1285, 1293, 1298, 1302},
		},
	},
};


struct combo_dev_attr_s imx412_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_900M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {2, 3, 1, 4, 0},
		.pn_swap = {1, 1, 1, 1, 1},
		.wdr_mode = CVI_MIPI_WDR_MODE_VC,
		.dphy = {
			.enable = 1,
			.hs_settle = 12,
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

#endif /* __IMX412_CMOS_PARAM_H_ */

#ifndef __OS08A20_CMOS_PARAM_H_
#define __OS08A20_CMOS_PARAM_H_

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <cvi_comm_cif.h>
#include <cvi_type.h>
#include "cvi_sns_ctrl.h"
#include "os08a20_cmos_ex.h"

static const OS08A20_MODE_S g_astOs08a20_mode[OS08A20_MODE_NUM] = {
	[OS08A20_MODE_2592X1944P30] = {
		.name = "2592x1944p30",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2592,
				.u32Height = 1944,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2592,
				.u32Height = 1944,
			},
			.stMaxSize = {
				.u32Width = 2592,
				.u32Height = 1944,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 1.06, /* 0x90a * 30 / 0xFFFF */
		.u32HtsDef = 0x818,
		.u32VtsDef = 0x90a,
		.stExp[0] = {
			.u16Min = 8,
			.u16Max = 0x90a - 8,
			.u16Def = 0x462,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u16Min = 1024,
			.u16Max = 15872,
			.u16Def = 1024,
			.u16Step = 1,
		},
		.stDgain[0] = {
			.u16Min = 1024,
			.u16Max = 16383,
			.u16Def = 1024,
			.u16Step = 1,
		},
	},
	[OS08A20_MODE_2592X1944P30_WDR] = {
		.name = "2592x1944p30wdr",
		.astImg[0] = {
			.stSnsSize = {
				.u32Width = 2592,
				.u32Height = 1944,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2592,
				.u32Height = 1944,
			},
			.stMaxSize = {
				.u32Width = 2592,
				.u32Height = 1944,
			},
		},
		.astImg[1] = {
			.stSnsSize = {
				.u32Width = 2592,
				.u32Height = 1944,
			},
			.stWndRect = {
				.s32X = 0,
				.s32Y = 0,
				.u32Width = 2592,
				.u32Height = 1944,
			},
			.stMaxSize = {
				.u32Width = 2592,
				.u32Height = 1944,
			},
		},
		.f32MaxFps = 30,
		.f32MinFps = 1.08, /* 0x92a * 30 / 0xFFFF */
		.u32HtsDef = 0x40c,
		.u32VtsDef = 0x92a,
		.u16L2sOffset = 4,
		.u16TopBoundary = 36,
		.u16BotBoundary = 8,
		.stExp[0] = {
			.u16Min = 8,
			.u16Max = 312,
			.u16Def = 100,
			.u16Step = 1,
		},
		.stExp[1] = {
			.u16Min = 8,
			.u16Max = 0x486 - 4 - 312,
			.u16Def = 500,
			.u16Step = 1,
		},
		.stAgain[0] = {
			.u16Min = 1024,
			.u16Max = 15872,
			.u16Def = 1024,
			.u16Step = 1,
		},
		.stAgain[1] = {
			.u16Min = 1024,
			.u16Max = 15872,
			.u16Def = 1024,
			.u16Step = 1,
		},
		.stDgain[0] = {
			.u16Min = 1024,
			.u16Max = 16383,
			.u16Def = 1024,
			.u16Step = 1,
		},
		.stDgain[1] = {
			.u16Min = 1024,
			.u16Max = 16383,
			.u16Def = 1024,
			.u16Step = 1,
		},
		.u32L2S_MAX = 0x13D,
	},
};

static ISP_CMOS_NOISE_CALIBRATION_S g_stIspNoiseCalibratio = {.CalibrationCoef = {
	{	//iso  100
		{0.03190412744879722595,	3.86617159843444824219}, //B: slope, intercept
		{0.02441397681832313538,	4.87663412094116210938}, //Gb: slope, intercept
		{0.02471913769841194153,	4.85860157012939453125}, //Gr: slope, intercept
		{0.02652409672737121582,	4.48996973037719726563}, //R: slope, intercept
	},
	{	//iso  200
		{0.04349273815751075745,	5.49785566329956054688}, //B: slope, intercept
		{0.03087714128196239471,	7.49881792068481445313}, //Gb: slope, intercept
		{0.03145518898963928223,	7.35760211944580078125}, //Gr: slope, intercept
		{0.03530855849385261536,	6.56339693069458007813}, //R: slope, intercept
	},
	{	//iso  400
		{0.05788951739668846130,	8.34851074218750000000}, //B: slope, intercept
		{0.04015926644206047058,	11.21226787567138671875}, //Gb: slope, intercept
		{0.04018958285450935364,	11.29319286346435546875}, //Gr: slope, intercept
		{0.04840981587767601013,	9.35622310638427734375}, //R: slope, intercept
	},
	{	//iso  800
		{0.08085585385560989380,	11.91084861755371093750}, //B: slope, intercept
		{0.05333026498556137085,	16.38746833801269531250}, //Gb: slope, intercept
		{0.05413141101598739624,	16.21148872375488281250}, //Gr: slope, intercept
		{0.06530260294675827026,	13.91790008544921875000}, //R: slope, intercept
	},
	{	//iso  1600
		{0.09407941251993179321,	17.58893775939941406250}, //B: slope, intercept
		{0.05309880897402763367,	24.13221168518066406250}, //Gb: slope, intercept
		{0.05440055206418037415,	23.84361267089843750000}, //Gr: slope, intercept
		{0.06805266439914703369,	20.56119728088378906250}, //R: slope, intercept
	},
	{	//iso  3200
		{0.09407941251993179321,	17.58893775939941406250}, //B: slope, intercept
		{0.05309880897402763367,	24.13221168518066406250}, //Gb: slope, intercept
		{0.05440055206418037415,	23.84361267089843750000}, //Gr: slope, intercept
		{0.06805266439914703369,	20.56119728088378906250}, //R: slope, intercept
	},
	{	//iso  6400
		{0.09407941251993179321,	17.58893775939941406250}, //B: slope, intercept
		{0.05309880897402763367,	24.13221168518066406250}, //Gb: slope, intercept
		{0.05440055206418037415,	23.84361267089843750000}, //Gr: slope, intercept
		{0.06805266439914703369,	20.56119728088378906250}, //R: slope, intercept
	},
	{	//iso  12800
		{0.09407941251993179321,	17.58893775939941406250}, //B: slope, intercept
		{0.05309880897402763367,	24.13221168518066406250}, //Gb: slope, intercept
		{0.05440055206418037415,	23.84361267089843750000}, //Gr: slope, intercept
		{0.06805266439914703369,	20.56119728088378906250}, //R: slope, intercept
	},
	{	//iso  25600
		{0.48950406908988952637,	67.73629760742187500000}, //B: slope, intercept
		{0.25487670302391052246,	101.95156860351562500000}, //Gb: slope, intercept
		{0.24315287172794342041,	105.36353302001953125000}, //Gr: slope, intercept
		{0.33707463741302490234,	86.65032958984375000000}, //R: slope, intercept
	},
	{	//iso  51200
		{0.48950406908988952637,	67.73629760742187500000}, //B: slope, intercept
		{0.25487670302391052246,	101.95156860351562500000}, //Gb: slope, intercept
		{0.24315287172794342041,	105.36353302001953125000}, //Gr: slope, intercept
		{0.33707463741302490234,	86.65032958984375000000}, //R: slope, intercept
	},
	{	//iso  102400
		{0.48950406908988952637,	67.73629760742187500000}, //B: slope, intercept
		{0.25487670302391052246,	101.95156860351562500000}, //Gb: slope, intercept
		{0.24315287172794342041,	105.36353302001953125000}, //Gr: slope, intercept
		{0.33707463741302490234,	86.65032958984375000000}, //R: slope, intercept
	},
	{	//iso  204800
		{0.48950406908988952637,	67.73629760742187500000}, //B: slope, intercept
		{0.25487670302391052246,	101.95156860351562500000}, //Gb: slope, intercept
		{0.24315287172794342041,	105.36353302001953125000}, //Gr: slope, intercept
		{0.33707463741302490234,	86.65032958984375000000}, //R: slope, intercept
	},
	{	//iso  409600
		{0.48950406908988952637,	67.73629760742187500000}, //B: slope, intercept
		{0.25487670302391052246,	101.95156860351562500000}, //Gb: slope, intercept
		{0.24315287172794342041,	105.36353302001953125000}, //Gr: slope, intercept
		{0.33707463741302490234,	86.65032958984375000000}, //R: slope, intercept
	},
	{	//iso  819200
		{0.48950406908988952637,	67.73629760742187500000}, //B: slope, intercept
		{0.25487670302391052246,	101.95156860351562500000}, //Gb: slope, intercept
		{0.24315287172794342041,	105.36353302001953125000}, //Gr: slope, intercept
		{0.33707463741302490234,	86.65032958984375000000}, //R: slope, intercept
	},
	{	//iso  1638400
		{0.48950406908988952637,	67.73629760742187500000}, //B: slope, intercept
		{0.25487670302391052246,	101.95156860351562500000}, //Gb: slope, intercept
		{0.24315287172794342041,	105.36353302001953125000}, //Gr: slope, intercept
		{0.33707463741302490234,	86.65032958984375000000}, //R: slope, intercept
	},
	{	//iso  3276800
		{0.48950406908988952637,	67.73629760742187500000}, //B: slope, intercept
		{0.25487670302391052246,	101.95156860351562500000}, //Gb: slope, intercept
		{0.24315287172794342041,	105.36353302001953125000}, //Gr: slope, intercept
		{0.33707463741302490234,	86.65032958984375000000}, //R: slope, intercept
	},
} };

static ISP_CMOS_BLACK_LEVEL_S g_stIspBlcCalibratio = {
	.bUpdate = CVI_TRUE,
	.blcAttr = {
		.Enable = 1,
		.enOpType = OP_TYPE_AUTO,
		.stManual = {256, 256, 256, 256, 0, 0, 0, 0
		},
		.stAuto = {
			{256, 256, 256, 256, 256, 256, 256, 256, /*8*/256, 256, 256, 256, 256, 256, 256, 256},
			{256, 256, 256, 256, 256, 256, 256, 256, /*8*/256, 256, 256, 256, 256, 256, 256, 256},
			{256, 256, 256, 256, 256, 256, 256, 256, /*8*/256, 256, 256, 256, 256, 256, 256, 256},
			{256, 256, 256, 256, 256, 256, 256, 256, /*8*/256, 256, 256, 256, 256, 256, 256, 256},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
			{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		},
	},
};

struct combo_dev_attr_s os08a20_rx_attr = {
	.input_mode = INPUT_MODE_MIPI,
	.mac_clk = RX_MAC_CLK_600M,
	.mipi_attr = {
		.raw_data_type = RAW_DATA_10BIT,
		.lane_id = {3, 2, 1, 4, 0},
		.pn_swap = {1, 1, 1, 1, 1},
		.wdr_mode = CVI_MIPI_WDR_MODE_VC,
	},
	.mclk = {
		.cam = 0,
		.freq = CAMPLL_FREQ_25M,
	},
	.devno = 0,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* __OS08A20_CMOS_PARAM_H_ */

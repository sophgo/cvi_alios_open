#include <aos/kernel.h>
#include <stdio.h>
#include "pthread.h"
#include <aos/cli.h>
#include "cvi_isp.h"
#include "cvi_ae.h"
#include "cvi_awb.h"

#define DELAY_500MS() (aos_msleep(500))

#define SAMPLE_IR_CALIBRATION_MODE 0
#define SAMPLE_IR_AUTO_MODE        1
#define SAMPLE_IR_AUTO_DISABLE     2

static VI_PIPE ViPipe;
static CVI_BOOL g_bEnableRun;

static void print_usage(char *sPrgNm)
{
	printf("Usage : %s <mode> <vipipe> <u32Normal2IrIsoThr> <u32Ir2NormalIsoThr> ", sPrgNm);
	printf("<u32RGMax> <u32RGMin> <u32BGMax> <u32BGMin> <enIrStatus>\n");

	printf("mode:\n");
	printf("\t 0) SAMPLE_IR_CALIBRATION_MODE.\n");
	printf("\t 1) SAMPLE_IR_AUTO_MODE.\n");
	printf("\t 2) disable ir_auto thread.\n");

	printf("vipipe:\n");
	printf("\t Pipe number [0-2].\n");

	printf("u32Normal2IrIsoThr:\n");
	printf("\t ISO threshold of switching from normal to IR mode.\n");

	printf("u32Ir2NormalIsoThr:\n");
	printf("\t ISO threshold of switching from IR to normal mode.\n");

	printf("u32RGMax/u32RGMin/u32BGMax/u32BGMin:\n");
	printf("\t Maximum(Minimum) value of R/G(B/G) in IR scene.\n");

	printf("enIrStatus:\n");
	printf("\t Current IR status. 0: Normal mode; 1: IR mode.\n");

	printf("e.g : %s 0 0 (SAMPLE_IR_CALIBRATION_MODE)\n", sPrgNm);
	printf("e.g : %s 1 0 16000 400 280 190 280 190 0 (SAMPLE_IR_AUTO_MODE, user_define parameters)\n", sPrgNm);
}

static void switch_to_ir(void)
{
	// 1. switch pq BIN
	// 2. switch ir cut

	printf("\nNormal --> IR\n");
}

static void switch_to_normal(void)
{
	// 1. switch pq BIN
	// 2. switch ir cut

	printf("\nIR --> Normal\n");
}

static void get_ae_awb_info(CVI_U32 *u32ISO, CVI_U32 *u32RGgain, CVI_U32 *u32BGgain)
{
#define IR_WB_GAIN_FORMAT	256
#define IR_DIV_0_TO_1(a)	((0 == (a)) ? 1 : (a))

	ISP_EXP_INFO_S aeInfo;
	CVI_U16 grayWorldRgain, grayWorldBgain;

	CVI_ISP_QueryExposureInfo(ViPipe, &aeInfo);
	CVI_ISP_GetGrayWorldAwbInfo(ViPipe, &grayWorldRgain, &grayWorldBgain);

	*u32ISO = aeInfo.u32ISO;
	*u32RGgain = IR_WB_GAIN_FORMAT * 1024 / IR_DIV_0_TO_1(grayWorldRgain);
	*u32BGgain = IR_WB_GAIN_FORMAT * 1024 / IR_DIV_0_TO_1(grayWorldBgain);
}

static int run_calibration(void *arg)
{
#define GAIN_MAX_COEF 280
#define GAIN_MIN_COEF 190

	CVI_U32 u32ISO;
	CVI_U32 RGgain, BGgain;

	switch_to_ir();

	while (g_bEnableRun) {

		get_ae_awb_info(&u32ISO, &RGgain, &BGgain);

		printf("\n");
		printf("ISO: %d, RGgain: %d, BGgain: %d\n", u32ISO, RGgain, BGgain);
		printf("Reference range: RGMax: %d, RGMin: %d, BGMax: %d, BGMin: %d\n",
			(RGgain * GAIN_MAX_COEF) >> 8, (RGgain * GAIN_MIN_COEF) >> 8,
			(BGgain * GAIN_MAX_COEF) >> 8, (BGgain * GAIN_MIN_COEF) >> 8);

		DELAY_500MS();
	}

	return CVI_SUCCESS;
}

#define ENABLE_RUN_IR_AUTO_DEBUG	1

#if ENABLE_RUN_IR_AUTO_DEBUG
static void print_ae_awb_info(void)
{
	CVI_U32 u32ISO;
	CVI_U32 RGgain, BGgain;

	get_ae_awb_info(&u32ISO, &RGgain, &BGgain);
	printf("Current, ISO: %d, RGgain: %d, BGgain: %d\n", u32ISO, RGgain, BGgain);
}
#endif

static int run_ir_auto(void *arg)
{
	char **argv = (char **)arg;
	CVI_S32 s32Ret = CVI_SUCCESS;

#define STABLE_COUNT_THR   3
	CVI_U8 u8StableCount = 0;

	ISP_IR_AUTO_ATTR_S stIrAttr;

	stIrAttr.bEnable = CVI_TRUE;

	stIrAttr.u32Normal2IrIsoThr = atoi(argv[3]);
	stIrAttr.u32Ir2NormalIsoThr = atoi(argv[4]);
	stIrAttr.u32RGMax = atoi(argv[5]);
	stIrAttr.u32RGMin = atoi(argv[6]);
	stIrAttr.u32BGMax = atoi(argv[7]);
	stIrAttr.u32BGMin = atoi(argv[8]);
	stIrAttr.enIrStatus = atoi(argv[9]);

	if (stIrAttr.enIrStatus != ISP_IR_STATUS_NORMAL &&
		stIrAttr.enIrStatus != ISP_IR_STATUS_IR) {
		printf("the mode is invalid!\n");
		goto exit;
	}

	while (g_bEnableRun) {

#if ENABLE_RUN_IR_AUTO_DEBUG
		printf("\n");
		printf("input, u32Normal2IrIsoThr: %d, u32Ir2NormalIsoThr: %d, ",
			stIrAttr.u32Normal2IrIsoThr,
			stIrAttr.u32Ir2NormalIsoThr);
		printf("RG: %d - %d, BG: %d - %d, enIrStatus: %d\n",
			stIrAttr.u32RGMax,
			stIrAttr.u32RGMin,
			stIrAttr.u32BGMax,
			stIrAttr.u32BGMin,
			stIrAttr.enIrStatus);
		print_ae_awb_info();
#endif

		s32Ret = CVI_ISP_IrAutoRunOnce(ViPipe, &stIrAttr);

#if ENABLE_RUN_IR_AUTO_DEBUG
		printf("enIrSwitch: %d, u8StableCount: %d\n", stIrAttr.enIrSwitch, u8StableCount);
#endif

		if (stIrAttr.enIrSwitch == ISP_IR_SWITCH_TO_IR &&
			u8StableCount++ > STABLE_COUNT_THR) {

			switch_to_ir();

			stIrAttr.enIrStatus = ISP_IR_STATUS_IR;

		} else if (stIrAttr.enIrSwitch == ISP_IR_SWITCH_TO_NORMAL &&
			u8StableCount++ > STABLE_COUNT_THR) {

			switch_to_normal();

			stIrAttr.enIrStatus = ISP_IR_STATUS_NORMAL;

		}

		if (stIrAttr.enIrSwitch == ISP_IR_SWITCH_NONE) {
			u8StableCount = 0;
		}

		DELAY_500MS();
	}

	return s32Ret;

exit:
	print_usage(argv[0]);
	return CVI_FAILURE;
}

int ir_auto_test(int argc, char **argv)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_U32 u32Mode = SAMPLE_IR_CALIBRATION_MODE;

	if (argc != 3 && argc != 10) {
		print_usage(argv[0]);
		return CVI_FAILURE;
	}

	u32Mode = atoi(argv[1]);
	ViPipe = atoi(argv[2]);

	if(u32Mode == SAMPLE_IR_AUTO_DISABLE){
		g_bEnableRun = CVI_FALSE;
		return s32Ret;
	}

	if (u32Mode != SAMPLE_IR_CALIBRATION_MODE &&
		u32Mode != SAMPLE_IR_AUTO_MODE) {
		printf("the mode is invalid!\n");
		print_usage(argv[0]);
		return CVI_FAILURE;
	}

	if(g_bEnableRun == CVI_TRUE)
		return s32Ret;


	g_bEnableRun = CVI_TRUE;

	char threadname[64] = {0};
	struct sched_param param;
	pthread_attr_t pthread_attr;
	pthread_t pthreadId = 0;

	param.sched_priority = 31;
	pthread_attr_init(&pthread_attr);
	pthread_attr_setschedpolicy(&pthread_attr, SCHED_RR);
	pthread_attr_setschedparam(&pthread_attr, &param);
	pthread_attr_setinheritsched(&pthread_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setstacksize(&pthread_attr, 6*1024);
	snprintf(threadname,sizeof(threadname),"ir_auto_test%d",0);
	pthread_setname_np(pthreadId, threadname);

	if (u32Mode == SAMPLE_IR_CALIBRATION_MODE) {
		pthread_create(&pthreadId,&pthread_attr,(void *)run_calibration,(void *)argv);
	} else if (u32Mode == SAMPLE_IR_AUTO_MODE) {
		pthread_create(&pthreadId,&pthread_attr,(void *)run_ir_auto,(void *)argv);
	}

	return s32Ret;
}

ALIOS_CLI_CMD_REGISTER(ir_auto_test, ir_auto_test, ir_auto_test);

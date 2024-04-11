#include "common_vi.h"

#define MAX_SENSOR_NUM  3

static int g_sns_type[MAX_SENSOR_NUM] = {CONFIG_SNS0_TYPE, CONFIG_SNS1_TYPE, CONFIG_SNS2_TYPE};

typedef struct _SNS_CONFIG_S {
	SIZE_S			stSize;
	WDR_MODE_E		enWdrMode;
	BAYER_FORMAT_E		enBayerFormat;
	PIXEL_FORMAT_E		enPixelFormat;
	SNS_TYPE_E		enSnsType;
} SNS_CONFIG_S;

ISP_CMOS_SENSOR_IMAGE_MODE_S snsr_image_mode = {
	.u16Width = 1280,
	.u16Height = 720,
	.f32Fps = 30,
	.u8SnsMode = WDR_MODE_NONE,
};

VI_DEV_ATTR_S vi_dev_attr_base = {
	.enIntfMode = VI_MODE_MIPI,
	.enWorkMode = VI_WORK_MODE_1Multiplex,
	.enScanMode = VI_SCAN_PROGRESSIVE,
	.as32AdChnId = {-1, -1, -1, -1},
	.enDataSeq = VI_DATA_SEQ_YUYV,
	.stSynCfg = {
	/*port_vsync	port_vsync_neg	  port_hsync			  port_hsync_neg*/
	VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH,
	/*port_vsync_valid	   port_vsync_valid_neg*/
	VI_VSYNC_VALID_SIGNAL, VI_VSYNC_VALID_NEG_HIGH,

	/*hsync_hfb  hsync_act	hsync_hhb*/
	{0, 		  1280, 	  0,
	/*vsync0_vhb vsync0_act vsync0_hhb*/
	 0, 		  720, 		  0,
	/*vsync1_vhb vsync1_act vsync1_hhb*/
	 0, 		   0,		  0}
	},
	.enInputDataType = VI_DATA_TYPE_RGB,
	.stSize = {1280, 720},
	.stWDRAttr = {WDR_MODE_NONE, 720},
	.enBayerFormat = BAYER_FORMAT_BG,
};

VI_PIPE_ATTR_S vi_pipe_attr_base = {
	.enPipeBypassMode = VI_PIPE_BYPASS_NONE,
	.bYuvSkip = CVI_FALSE,
	.bIspBypass = CVI_FALSE,
	.u32MaxW = 1280,
	.u32MaxH = 720,
	.enPixFmt = PIXEL_FORMAT_RGB_BAYER_12BPP,
	.enCompressMode = COMPRESS_MODE_TILE,
	.enBitWidth = DATA_BITWIDTH_12,
	.bNrEn = CVI_FALSE,
	.bSharpenEn = CVI_FALSE,
	.stFrameRate = {-1, -1},
	.bDiscardProPic = CVI_FALSE,
	.bYuvBypassPath = CVI_FALSE,
};


VI_CHN_ATTR_S vi_chn_attr_base = {
	.stSize = {1280, 720},
	.enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_420,
	.enDynamicRange = DYNAMIC_RANGE_SDR8,
	.enVideoFormat = VIDEO_FORMAT_LINEAR,
	.enCompressMode = COMPRESS_MODE_TILE,
	.bMirror = CVI_FALSE,
	.bFlip = CVI_FALSE,
	.u32Depth = 0,
	.stFrameRate = {-1, -1},
};

CVI_S32 get_sensor_type(CVI_S32 dev_id)
{
    if (dev_id >= MAX_SENSOR_NUM)
        return SNS_TYPE_NONE;

    return g_sns_type[dev_id];
}

CVI_S32 getSnsType(CVI_S32 *snsr_type, CVI_U8 *devNum)
{
    for (CVI_U8 i = 0; i < MAX_SENSOR_NUM; i++) {
        if (g_sns_type[i] != SNS_TYPE_NONE)
            *devNum += 1;
    }
    for (CVI_U8 i = 0; i < *devNum; i++) {
        snsr_type[i] = get_sensor_type(i);
    }

    return CVI_SUCCESS;
}

ISP_SNS_OBJ_S *getSnsObj(SNS_TYPE_E enSnsType)
{
	switch (enSnsType) {
#if CONFIG_SENSOR_GCORE_GC02M1
	case GCORE_GC02M1_MIPI_2M_30FPS_10BIT:
	case GCORE_GC02M1_MIPI_600P_30FPS_10BIT:
		return &stSnsGc02m1_Obj;
#endif
#if CONFIG_SENSOR_GCORE_GC02M1_SLAVE
	case GCORE_GC02M1_SLAVE_MIPI_2M_30FPS_10BIT:
	case GCORE_GC02M1_SLAVE_MIPI_600P_30FPS_10BIT:
		return &stSnsGc02m1_Slave_Obj;
#endif
#if CONFIG_SENSOR_GCORE_GC02M1_MULTI
	case GCORE_GC02M1_MULTI_MIPI_2M_30FPS_10BIT:
		return &stSnsGc02m1_Multi_Obj;
#endif
#if CONFIG_SENSOR_GCORE_GC1054
	case GCORE_GC1054_MIPI_1M_30FPS_10BIT:
		return &stSnsGc1054_Obj;
#endif
#if CONFIG_SENSOR_GCORE_GC1084
	case GCORE_GC1084_MIPI_1M_30FPS_10BIT:
		return &stSnsGc1084_Obj;
#endif
#if CONFIG_SENSOR_GCORE_GC2053
	case GCORE_GC2053_MIPI_2M_30FPS_10BIT:
		return &stSnsGc2053_Obj;
#endif
#if CONFIG_SENSOR_GCORE_GC2053_1L
	case GCORE_GC2053_1L_MIPI_2M_30FPS_10BIT:
		return &stSnsGc2053_1l_Obj;
#endif
#if CONFIG_SENSOR_GCORE_GC2093
	case GCORE_GC2093_MIPI_2M_30FPS_10BIT:
	case GCORE_GC2093_MIPI_2M_30FPS_10BIT_WDR2TO1:
		return &stSnsGc2093_Obj;
#endif
#if CONFIG_SENSOR_GCORE_GC2083_1L
	case GCORE_GC2083_1L_MIPI_2M_30FPS_10BIT:
		return &stSnsGc2083_1L_Obj;
#endif
#if CONFIG_SENSOR_GCORE_GC4653
	case GCORE_GC4653_MIPI_4M_30FPS_10BIT:
		return &stSnsGc4653_Obj;
#endif
#if CONFIG_SENSOR_OPNOUS_OPN8018
	case OPNOUS_OPN8018_MIPI_480P_30FPS_12BIT:
		return &stSnsOpn8018_Obj;
#endif
#if CONFIG_SENSOR_SMS_SC200AI
	case SMS_SC200AI_MIPI_2M_30FPS_10BIT:
	case SMS_SC200AI_MIPI_2M_30FPS_10BIT_WDR2TO1:
		return &stSnsSC200AI_Obj;
#endif
#if CONFIG_SENSOR_SMS_SC530AI_4L
    case SMS_SC530AI_4L_MIPI_4M_30FPS_10BIT:
        return &stSnsSC530AI_4L_Obj;
#endif
#if CONFIG_SENSOR_SMS_SC2336
	case SMS_SC2336_MIPI_2M_30FPS_10BIT:
		return &stSnsSC2336_Obj;
#endif
#if CONFIG_SENSOR_SMS_SC2336_1L
	case SMS_SC2336_1L_MIPI_2M_30FPS_10BIT:
		return &stSnsSC2336_1L_Obj;
#endif
#if CONFIG_SENSOR_SMS_SC2336P
	case SMS_SC2336P_MIPI_2M_30FPS_10BIT:
		return &stSnsSC2336P_Obj;
#endif
#if CONFIG_SENSOR_SMS_SC2336P_1L
	case SMS_SC2336P_1L_MIPI_2M_30FPS_10BIT:
		return &stSnsSC2336P_1L_Obj;
#endif
#if CONFIG_SENSOR_SMS_SC030IOT
	case SMS_SC030IOT_MIPI_480P_30FPS_8BIT:
		return &stSnsSC030IOT_Obj;
#endif
#if CONFIG_SENSOR_SMS_SC230AI
    case SMS_SC230AI_MIPI_2M_30FPS_10BIT:
        return &stSnsSC230AI_Obj;
#endif
#if CONFIG_SENSOR_SMS_SC230AI_SLAVE
    case SMS_SC230AI_SLAVE_MIPI_2M_30FPS_10BIT:
        return &stSnsSC230AI_SLAVE_Obj;
#endif
#if CONFIG_SENSOR_SMS_SC201CS
	case SMS_SC201CS_MIPI_2M_30FPS_10BIT:
		return &stSnsSC201CS_Obj;
#endif
#if CONFIG_SENSOR_SMS_SC201CS_SLAVE
	case SMS_SC201CS_SLAVE_MIPI_2M_30FPS_10BIT:
		return &stSnsSC201CS_SLAVE_Obj;
#endif
#if CONFIG_SENSOR_SMS_SC201CS_MULTI
	case SMS_SC201CS_MULTI_MIPI_2M_30FPS_10BIT:
		return &stSnsSC201CS_MULTI_Obj;
#endif
#if CONFIG_SENSOR_SMS_SC202CS
	case SMS_SC202CS_MIPI_2M_30FPS_10BIT:
		return &stSnsSC202CS_Obj;
#endif
#if CONFIG_SENSOR_SMS_SC202CS_SLAVE
	case SMS_SC202CS_SLAVE_MIPI_2M_30FPS_10BIT:
		return &stSnsSC202CS_SLAVE_Obj;
#endif
#if CONFIG_SENSOR_SMS_SC202CS_MULTI
	case SMS_SC202CS_MULTI_MIPI_2M_30FPS_10BIT:
		return &stSnsSC202CS_MULTI_Obj;
#endif
#if CONFIG_SENSOR_SMS_SC031IOT
	case SMS_SC031IOT_MIPI_480P_30FPS_8BIT:
		return &stSnsSC031IOT_Obj;
#endif
#if CONFIG_SENSOR_SMS_SC031IOT_RAW
	case SMS_SC031IOT_MIPI_RAW_480P_30FPS_8BIT:
		return &stSnsSC031IOT_RAW_Obj;
#endif
#if CONFIG_SENSOR_SMS_SC301IOT
	case SMS_SC301IOT_MIPI_3M_30FPS_10BIT:
		return &stSnsSC301IOT_Obj;
#endif
#if CONFIG_SENSOR_SONY_IMX307
	case SONY_IMX307_MIPI_2M_30FPS_12BIT:
	case SONY_IMX307_MIPI_2M_30FPS_12BIT_WDR2TO1:
		return &stSnsImx307_Obj;
#endif
#if CONFIG_SENSOR_SONY_IMX307_2L
	case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT:
	case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT_WDR2TO1:
		return &stSnsImx307_2l_Obj;
#endif
#if CONFIG_SENSOR_SONY_IMX307_SLAVE
	case SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT:
	case SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT_WDR2TO1:
		return &stSnsImx307_Slave_Obj;
#endif
#if CONFIG_SENSOR_SONY_IMX327
	case SONY_IMX327_MIPI_2M_30FPS_12BIT:
	case SONY_IMX327_MIPI_2M_30FPS_12BIT_WDR2TO1:
		return &stSnsImx327_Obj;
#endif
#if CONFIG_SENSOR_BYD_BF314A
	case BYD_BF314A_MIPI_720P_30FPS_10BIT:
		return &stSnsBf314a_Obj;
#endif
#if CONFIG_SENSOR_BYD_BF2253L
	case BYD_BF2253L_MIPI_1200P_30FPS_10BIT:
		return &stSnsBF2253L_Obj;
#endif
#if CONFIG_SENSOR_BYD_BF2257CS
	case BYD_BF2257CS_MIPI_1200P_30FPS_10BIT:
		return &stSnsBF2257CS_Obj;
#endif
#if CONFIG_SENSOR_BYD_BF2257CS_SLAVE
	case BYD_BF2257CS_SLAVE_MIPI_1200P_30FPS_10BIT:
		return &stSnsBF2257CS_Slave_Obj;
#endif
#if CONFIG_SENSOR_SPIX_SP2509
	case SPIX_SP2509_MIPI_2M_30FPS_10BIT:
		return &stSnsSp2509_Obj;
#endif
#if CONFIG_SENSOR_OV_OV02B10
    case OV_OV02B10_MIPI_2M_30FPS_10BIT:
	case OV_OV02B10_MIPI_600P_30FPS_10BIT:
        return &stSnsOv02b10_Obj;
#endif
#if CONFIG_SENSOR_CISTA_C2599
	case CISTA_C2599_MIPI_1200P_30FPS_10BIT:
		return &stSnsC2599_Obj;
#endif
	default:
		return CVI_NULL;
	}
}

CVI_S32 getSnsMode(CVI_S32 dev_id, ISP_CMOS_SENSOR_IMAGE_MODE_S *snsr_mode)
{
	SNS_SIZE_S sns_size;
	CVI_S32 sensor_type;

	if (dev_id >= MAX_SENSOR_NUM) {
		return CVI_FAILURE;
	}

	sensor_type = get_sensor_type(dev_id);

	memcpy(snsr_mode, &snsr_image_mode, sizeof(ISP_CMOS_SENSOR_IMAGE_MODE_S));

	getPicSize(dev_id, &sns_size);

	snsr_mode->u16Height = sns_size.u32Height;
	snsr_mode->u16Width = sns_size.u32Width;

	if (sensor_type >= SNS_TYPE_LINEAR_BUTT)
		snsr_mode->u8SnsMode = WDR_MODE_2To1_LINE;

	return CVI_SUCCESS;
}

CVI_S32 getPicSize(CVI_S32 dev_id, SNS_SIZE_S *pstSize)
{
	CVI_S32 sensor_type;

	if (dev_id >= MAX_SENSOR_NUM) {
		return CVI_FAILURE;
	}

	sensor_type = get_sensor_type(dev_id);

	switch (sensor_type) {
    case SMS_SC530AI_4L_MIPI_4M_30FPS_10BIT:
        pstSize->u32Width = 2880;
        pstSize->u32Height = 1620;
        break;
    case SMS_SC301IOT_MIPI_3M_30FPS_10BIT:
		pstSize->u32Width  = 2048;
		pstSize->u32Height = 1536;
		break;
	case GCORE_GC02M1_MIPI_2M_30FPS_10BIT:
	case GCORE_GC02M1_SLAVE_MIPI_2M_30FPS_10BIT:
	case GCORE_GC02M1_MULTI_MIPI_2M_30FPS_10BIT:
	case SMS_SC201CS_MIPI_2M_30FPS_10BIT:
	case SMS_SC201CS_MULTI_MIPI_2M_30FPS_10BIT:
	case BYD_BF2253L_MIPI_1200P_30FPS_10BIT:
	case BYD_BF2257CS_MIPI_1200P_30FPS_10BIT:
	case BYD_BF2257CS_SLAVE_MIPI_1200P_30FPS_10BIT:
	case SMS_SC202CS_MIPI_2M_30FPS_10BIT:
	case SMS_SC202CS_SLAVE_MIPI_2M_30FPS_10BIT:
	case SMS_SC202CS_MULTI_MIPI_2M_30FPS_10BIT:
	case SPIX_SP2509_MIPI_2M_30FPS_10BIT:
	case OV_OV02B10_MIPI_2M_30FPS_10BIT:
	case CISTA_C2599_MIPI_1200P_30FPS_10BIT:
		pstSize->u32Width  = 1600;
		pstSize->u32Height = 1200;
		break;
	case GCORE_GC02M1_MIPI_600P_30FPS_10BIT:
	case GCORE_GC02M1_SLAVE_MIPI_600P_30FPS_10BIT:
	case OV_OV02B10_MIPI_600P_30FPS_10BIT:
		pstSize->u32Width  = 800;
		pstSize->u32Height = 600;
		break;
	case GCORE_GC1054_MIPI_1M_30FPS_10BIT:
	case GCORE_GC1084_MIPI_1M_30FPS_10BIT:
	case BYD_BF314A_MIPI_720P_30FPS_10BIT:
		pstSize->u32Width  = 1280;
		pstSize->u32Height = 720;
		break;
	case GCORE_GC4653_MIPI_4M_30FPS_10BIT:
		pstSize->u32Width  = 2560;
		pstSize->u32Height = 1440;
		break;
	case OPNOUS_OPN8018_MIPI_480P_30FPS_12BIT:
		pstSize->u32Width  = 648;
		pstSize->u32Height = 1464;
		break;
	case SMS_SC030IOT_MIPI_480P_30FPS_8BIT:
	case SMS_SC031IOT_MIPI_480P_30FPS_8BIT:
	case SMS_SC031IOT_MIPI_RAW_480P_30FPS_8BIT:
		pstSize->u32Width  = 640;
		pstSize->u32Height = 480;
		break;

	default:
		pstSize->u32Width  = 1920;
		pstSize->u32Height = 1080;
		break;
	}

	return CVI_SUCCESS;
}

CVI_S32 getDevAttr(VI_DEV ViDev, VI_DEV_ATTR_S *pstViDevAttr)
{
	CVI_S32 sensor_type;
	SNS_SIZE_S sns_size;

	if (ViDev >= MAX_SENSOR_NUM) {
		return CVI_FAILURE;
	}

	sensor_type = get_sensor_type(ViDev);
	getPicSize(ViDev, &sns_size);

	memcpy(pstViDevAttr, &vi_dev_attr_base, sizeof(VI_DEV_ATTR_S));

	pstViDevAttr->stSize.u32Width = sns_size.u32Width;
	pstViDevAttr->stSize.u32Height = sns_size.u32Height;
	pstViDevAttr->stWDRAttr.u32CacheLine = sns_size.u32Height;

	switch (sensor_type) {
	case SONY_IMX327_MIPI_2M_30FPS_12BIT:
	case SONY_IMX327_MIPI_2M_30FPS_12BIT_WDR2TO1:
	case SONY_IMX307_MIPI_2M_30FPS_12BIT:
	case SONY_IMX307_MIPI_2M_30FPS_12BIT_WDR2TO1:
	case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT:
	case SONY_IMX307_2L_MIPI_2M_30FPS_12BIT_WDR2TO1:
	case SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT:
	case SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT_WDR2TO1:
	// GalaxyCore
	case GCORE_GC02M1_MIPI_2M_30FPS_10BIT:
	case GCORE_GC02M1_SLAVE_MIPI_2M_30FPS_10BIT:
	case GCORE_GC02M1_MULTI_MIPI_2M_30FPS_10BIT:
    case GCORE_GC02M1_MIPI_600P_30FPS_10BIT:
    case GCORE_GC02M1_SLAVE_MIPI_600P_30FPS_10BIT:
	case GCORE_GC2053_MIPI_2M_30FPS_10BIT:
	case GCORE_GC2053_1L_MIPI_2M_30FPS_10BIT:
	case GCORE_GC1054_MIPI_1M_30FPS_10BIT:
	case GCORE_GC2093_MIPI_2M_30FPS_10BIT:
	case GCORE_GC2093_MIPI_2M_30FPS_10BIT_WDR2TO1:
	case BYD_BF314A_MIPI_720P_30FPS_10BIT:
	case GCORE_GC2083_1L_MIPI_2M_30FPS_10BIT:
		pstViDevAttr->enBayerFormat = BAYER_FORMAT_RG;
		break;
	case GCORE_GC4653_MIPI_4M_30FPS_10BIT:
	case GCORE_GC1084_MIPI_1M_30FPS_10BIT:
		pstViDevAttr->enBayerFormat = BAYER_FORMAT_GR;
		break;
	case SMS_SC030IOT_MIPI_480P_30FPS_8BIT:
	case SMS_SC031IOT_MIPI_480P_30FPS_8BIT:
		pstViDevAttr->enDataSeq = VI_DATA_SEQ_YUYV;
		pstViDevAttr->enInputDataType = VI_DATA_TYPE_YUV;
		pstViDevAttr->enIntfMode = VI_MODE_MIPI_YUV422;
		break;
	default:
		pstViDevAttr->enBayerFormat = BAYER_FORMAT_BG;
		break;
	};

	if (sensor_type >= SNS_TYPE_LINEAR_BUTT)
		pstViDevAttr->stWDRAttr.enWDRMode = WDR_MODE_2To1_LINE;

	return CVI_SUCCESS;
}

CVI_S32 getPipeAttr(VI_DEV ViDev, VI_PIPE_ATTR_S *pstViPipeAttr)
{
	SNS_SIZE_S sns_size;
	VI_DEV_ATTR_S stViDevAttr;

	if (ViDev >= MAX_SENSOR_NUM) {
		return CVI_FAILURE;
	}

	getPicSize(ViDev, &sns_size);
	getDevAttr(ViDev, &stViDevAttr);

	memcpy(pstViPipeAttr, &vi_pipe_attr_base, sizeof(VI_PIPE_ATTR_S));

	pstViPipeAttr->u32MaxW = sns_size.u32Width;
	pstViPipeAttr->u32MaxH = sns_size.u32Height;

	if (stViDevAttr.enInputDataType == VI_DATA_TYPE_YUV) {
		pstViPipeAttr->bYuvBypassPath = CVI_TRUE;
	}

	return CVI_SUCCESS;
}

CVI_S32 getChnAttr(VI_DEV ViDev, VI_CHN_ATTR_S *pstViChnAttr)
{
	VI_DEV_ATTR_S stViDevAttr;


	if (ViDev >= MAX_SENSOR_NUM) {
		return CVI_FAILURE;
	}

	getDevAttr(ViDev, &stViDevAttr);

	memcpy(pstViChnAttr, &vi_chn_attr_base, sizeof(VI_CHN_ATTR_S));

	pstViChnAttr->stSize.u32Width = stViDevAttr.stSize.u32Width;
	pstViChnAttr->stSize.u32Height = stViDevAttr.stSize.u32Height;
	pstViChnAttr->enPixelFormat = PIXEL_FORMAT_NV21;


	if (stViDevAttr.enInputDataType == VI_DATA_TYPE_YUV) {
		pstViChnAttr->enPixelFormat = PIXEL_FORMAT_YUV_PLANAR_422;
	}

	return CVI_SUCCESS;
}

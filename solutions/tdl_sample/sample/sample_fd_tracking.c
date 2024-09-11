/**
 * This is a sample code for face tracking.
 */
#define LOG_TAG "SampleFaceTracking"
#define LOG_LEVEL LOG_LEVEL_INFO

#include "aos/cli.h"
#include "middleware_utils.h"
#include "sample_utils.h"
#include "vi_vo_utils.h"

#include <core/utils/vpss_helper.h>
#include <cvi_comm.h>
#include <rtsp.h>
#include <sample_comm.h>
#include "cvi_tdl.h"

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static volatile bool bExit = false;

MUTEXAUTOLOCK_INIT(ResultMutex);

typedef struct {
  SAMPLE_TDL_MW_CONTEXT *pstMWContext;
  cvitdl_service_handle_t stServiceHandle;
} SAMPLE_TDL_VENC_THREAD_ARG_S;

typedef struct {
  cvitdl_handle_t stTDLHandle;
  bool bTrackingWithFeature;
} SAMPLE_TDL_TDL_THREAD_ARG_S;

static cvtdl_face_t g_stFaceMeta = {0};
static cvtdl_tracker_t g_stTrackerMeta = {0};

void set_sample_mot_config(cvtdl_deepsort_config_t *ds_conf) {
  ds_conf->ktracker_conf.max_unmatched_num = 10;
  ds_conf->ktracker_conf.accreditation_threshold = 10;
  ds_conf->ktracker_conf.P_beta[2] = 0.1;
  ds_conf->ktracker_conf.P_beta[6] = 2.5e-2;
  ds_conf->kfilter_conf.Q_beta[2] = 0.1;
  ds_conf->kfilter_conf.Q_beta[6] = 2.5e-2;
}

cvtdl_service_brush_t get_random_brush(uint64_t seed, int min) {
  float scale = (256. - (float)min) / 256.;
  srand((uint32_t)seed);
  cvtdl_service_brush_t brush = {
      .color.r = (int)((floor(((float)rand() / (RAND_MAX)) * 256.)) * scale) + min,
      .color.g = (int)((floor(((float)rand() / (RAND_MAX)) * 256.)) * scale) + min,
      .color.b = (int)((floor(((float)rand() / (RAND_MAX)) * 256.)) * scale) + min,
      .size = 2,
  };

  return brush;
}

void *run_venc(void *args) {
  printf("Enter encoder thread\n");
  SAMPLE_TDL_VENC_THREAD_ARG_S *pstArgs = (SAMPLE_TDL_VENC_THREAD_ARG_S *)args;
  VIDEO_FRAME_INFO_S stFrame;
  CVI_S32 s32Ret;
  cvtdl_face_t stFaceMeta = {0};
  cvtdl_tracker_t stTrackerMeta = {0};

  cvtdl_service_brush_t stGreyBrush = CVI_TDL_Service_GetDefaultBrush();
  stGreyBrush.color.r = 105;
  stGreyBrush.color.g = 105;
  stGreyBrush.color.b = 105;

  cvtdl_service_brush_t stGreenBrush = CVI_TDL_Service_GetDefaultBrush();
  stGreenBrush.color.r = 0;
  stGreenBrush.color.g = 255;
  stGreenBrush.color.b = 0;

  while (bExit == false) {
    s32Ret = CVI_VPSS_GetChnFrame(0, 0, &stFrame, 2000);
    if (s32Ret != CVI_SUCCESS) {
      printf("CVI_VPSS_GetChnFrame chn0 failed with %#x\n", s32Ret);
      break;
    }

    {
      MutexAutoLock(ResultMutex, lock);
      CVI_TDL_CopyFaceMeta(&g_stFaceMeta, &stFaceMeta);
      CVI_TDL_CopyTrackerMeta(&g_stTrackerMeta, &stTrackerMeta);
    }

    // Draw different color for bbox accourding to tracker state.
    cvtdl_service_brush_t *brushes = malloc(stFaceMeta.size * sizeof(cvtdl_service_brush_t));
    for (uint32_t fid = 0; fid < stFaceMeta.size; fid++) {
      if (stTrackerMeta.info[fid].state == CVI_TRACKER_NEW) {
        brushes[fid] = stGreenBrush;
      } else if (stTrackerMeta.info[fid].state == CVI_TRACKER_UNSTABLE) {
        brushes[fid] = stGreyBrush;
      } else {  // CVI_TRACKER_STABLE
        brushes[fid] = get_random_brush(stFaceMeta.info[fid].unique_id, 64);
      }
    }

    // Fill name with unique id.
    for (uint32_t fid = 0; fid < stFaceMeta.size; fid++) {
      snprintf(stFaceMeta.info[fid].name, sizeof(stFaceMeta.info[fid].name), "UID: %" PRIu64 "",
               stFaceMeta.info[fid].unique_id);
    }

    if (s32Ret != CVI_TDL_SUCCESS) {
      CVI_VPSS_ReleaseChnFrame(0, 0, &stFrame);
      printf("Draw fame fail!, ret=%x\n", s32Ret);
      goto error;
    }

    s32Ret = SAMPLE_TDL_Send_Frame_RTSP(&stFrame, pstArgs->pstMWContext);
  error:
    free(brushes);
    CVI_TDL_Free(&stFaceMeta);
    CVI_TDL_Free(&stTrackerMeta);
    CVI_VPSS_ReleaseChnFrame(0, 0, &stFrame);
    if (s32Ret != CVI_SUCCESS) {
      bExit = true;
    }
  }
  printf("Exit encoder thread\n");
  pthread_exit(NULL);
}

void *run_tdl_thread(void *args) {
  printf("Enter TDL thread\n");
  SAMPLE_TDL_TDL_THREAD_ARG_S *pstTDLArgs = (SAMPLE_TDL_TDL_THREAD_ARG_S *)args;

  VIDEO_FRAME_INFO_S stFrame;
  cvtdl_face_t stFaceMeta = {0};
  cvtdl_tracker_t stTrackerMeta = {0};

  CVI_S32 s32Ret;
  while (bExit == false) {
    s32Ret = CVI_VPSS_GetChnFrame(0, 1, &stFrame, 2000);

    if (s32Ret != CVI_SUCCESS) {
      printf("CVI_VPSS_GetChnFrame failed with %#x\n", s32Ret);
      goto get_frame_failed;
    }

    //*******************************************
    // Step 1: Face detection.
    GOTO_IF_FAILED(CVI_TDL_FaceDetection(pstTDLArgs->stTDLHandle, &stFrame,
                                         CVI_TDL_SUPPORTED_MODEL_RETINAFACE, &stFaceMeta),
                   s32Ret, inf_error);

    if (pstTDLArgs->bTrackingWithFeature) {
      // Step 2: Extract feature for all face in stFaceMeta.
      GOTO_IF_FAILED(CVI_TDL_FaceRecognition(pstTDLArgs->stTDLHandle, &stFrame, &stFaceMeta),
                     s32Ret, inf_error);
    }

    // Step 3: Multi-Object Tracking inference.
    GOTO_IF_FAILED(CVI_TDL_DeepSORT_Face(pstTDLArgs->stTDLHandle, &stFaceMeta, &stTrackerMeta),
                   s32Ret, inf_error);

    printf("face detected: %d\n", stFaceMeta.size);
    //*******************************************

    {
      MutexAutoLock(ResultMutex, lock);
      CVI_TDL_CopyFaceMeta(&stFaceMeta, &g_stFaceMeta);
      CVI_TDL_CopyTrackerMeta(&stTrackerMeta, &g_stTrackerMeta);
    }

  inf_error:
    CVI_VPSS_ReleaseChnFrame(0, 1, &stFrame);
  get_frame_failed:
    CVI_TDL_Free(&stFaceMeta);
    CVI_TDL_Free(&stTrackerMeta);
    if (s32Ret != CVI_SUCCESS) {
      bExit = true;
    }
  }

  printf("Exit TDL thread\n");
  pthread_exit(NULL);
}

static void SampleHandleSig(CVI_S32 signo) {
  signal(SIGINT, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
  printf("handle signal, signo: %d\n", signo);
  if (SIGINT == signo || SIGTERM == signo) {
    bExit = true;
  }
}

static int sys_vi_init(void)
{
	MMF_VERSION_S stVersion;
	SAMPLE_INI_CFG_S	   stIniCfg;
	SAMPLE_VI_CONFIG_S stViConfig;

	PIC_SIZE_E enPicSize;
	SIZE_S stSize;
	CVI_S32 s32Ret = CVI_SUCCESS;
	LOG_LEVEL_CONF_S log_conf;

	CVI_SYS_GetVersion(&stVersion);
	SAMPLE_PRT("MMF Version:%s\n", stVersion.version);

	log_conf.enModId = CVI_ID_LOG;
	log_conf.s32Level = CVI_DBG_INFO;
	CVI_LOG_SetLevelConf(&log_conf);

	// Get config from ini if found.
	s32Ret = SAMPLE_COMM_VI_ParseIni(&stIniCfg);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("Parse fail\n");
	} else {
		SAMPLE_PRT("Parse complete\n");
	}

	//Set sensor number
	CVI_VI_SetDevNum(stIniCfg.devNum);
	/************************************************
	 * step1:  Config VI
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_IniToViCfg(&stIniCfg, &stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VI_IniToViCfg failed with %#x\n", s32Ret);
		return s32Ret;
  }

	// memcpy(&g_stViConfig, &stViConfig, sizeof(SAMPLE_VI_CONFIG_S));
	// memcpy(&g_stIniCfg, &stIniCfg, sizeof(SAMPLE_INI_CFG_S));

	/************************************************
	 * step2:  Get input size
	 ************************************************/
	s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(stIniCfg.enSnsType[0], &enPicSize);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_VI_GetSizeBySensor failed with %#x\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("SAMPLE_COMM_SYS_GetPicSize failed with %#x\n", s32Ret);
		return s32Ret;
	}

	/************************************************
	 * step3:  Init modules
	 ************************************************/
	VB_CONFIG_S        stVbConf;
	CVI_U32	       u32BlkSize;
	memset(&stVbConf, 0, sizeof(VB_CONFIG_S));
	stVbConf.u32MaxPoolCnt		= 3;
	u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height, SAMPLE_PIXEL_FORMAT,
					 DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	stVbConf.astCommPool[0].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[0].u32BlkCnt	= 3;
	SAMPLE_PRT("common pool[0] BlkSize %d\n", u32BlkSize);

  u32BlkSize = COMMON_GetPicBufferSize(1920, 1080, SAMPLE_PIXEL_FORMAT,
					 DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	stVbConf.astCommPool[1].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[1].u32BlkCnt	= 3;
	SAMPLE_PRT("common pool[1] BlkSize %d\n", u32BlkSize);

	//Control the second sensor resolution to avoid issues that can't get VB BLK.
	u32BlkSize = COMMON_GetPicBufferSize(768, 1280, PIXEL_FORMAT_BGR_888_PLANAR,
					 DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	stVbConf.astCommPool[2].u32BlkSize	= u32BlkSize;
	stVbConf.astCommPool[2].u32BlkCnt	= 1;
	SAMPLE_PRT("common pool[2] BlkSize %d\n", u32BlkSize);

	s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("sys init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

  VI_VPSS_MODE_S stVIVPSSMode;
  stVIVPSSMode.aenMode[0] = VI_OFFLINE_VPSS_ONLINE;
  s32Ret = CVI_SYS_SetVIVPSSMode(&stVIVPSSMode);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("set vi vpss mode failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	s32Ret = SAMPLE_PLAT_VI_INIT(&stViConfig);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vi init failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	return CVI_SUCCESS;
}

void fdt_main(void *arg) {
  int argc = 2;
  char **argv = (char **)arg;
  argv[0] = "sample_tdl_fdt";
  if (argc != 3 && argc != 2) {
    printf(
        "\nUsage: %s DET_MODEL_PATH [FR_MODEL_PATH]\n\n"
        "\tDET_MODEL_PATH, path to retinaface model\n"
        "\tFR_MODEL_PATH (optinal), path to face recognition model.\n",
        argv[0]);
    return -1;
  }

  argv[1] = "/mnt/sd/retinaface_mnet0.25_342_608.cvimodel";

  signal(SIGINT, SampleHandleSig);
  signal(SIGTERM, SampleHandleSig);

  CVI_S32 s32Ret = CVI_SUCCESS;
  s32Ret = sys_vi_init();
  if(s32Ret != CVI_SUCCESS){
    printf("vi init failed !!!!!!\n");
  }

  // Setup frame size of video encoder to 1080p
  SIZE_S stVencSize = {
      .u32Width = 1920,
      .u32Height = 1080,
  };

	/************************************************
	 * Init VPSS
	 ************************************************/
	VPSS_GRP_ATTR_S    stVpssGrpAttr;
	VPSS_GRP		   VpssGrp		  = 0;
	VPSS_CHN           VpssChn0        = VPSS_CHN0;
	VPSS_CHN           VpssChn1        = VPSS_CHN1;
	CVI_BOOL           abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {0};
	VPSS_CHN_ATTR_S    astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];

	stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
	stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
	stVpssGrpAttr.enPixelFormat                  = SAMPLE_PIXEL_FORMAT;
	stVpssGrpAttr.u32MaxW                        = 2560;
	stVpssGrpAttr.u32MaxH                        = 1440;
	stVpssGrpAttr.u8VpssDev                      = 1;

	astVpssChnAttr[VpssChn0].u32Width                    = stVencSize.u32Width;
	astVpssChnAttr[VpssChn0].u32Height                   = stVencSize.u32Height;
	astVpssChnAttr[VpssChn0].enVideoFormat               = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn0].enPixelFormat               = SAMPLE_PIXEL_FORMAT;
	astVpssChnAttr[VpssChn0].stFrameRate.s32SrcFrameRate = 15;
	astVpssChnAttr[VpssChn0].stFrameRate.s32DstFrameRate = 15;
	astVpssChnAttr[VpssChn0].u32Depth                    = 1;
	astVpssChnAttr[VpssChn0].bMirror                     = CVI_FALSE;
	astVpssChnAttr[VpssChn0].bFlip                       = CVI_FALSE;
	astVpssChnAttr[VpssChn0].stAspectRatio.enMode        = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn0].stAspectRatio.bEnableBgColor = CVI_FALSE;
	astVpssChnAttr[VpssChn0].stAspectRatio.u32BgColor    = RGB_8BIT(0, 0, 0);
	astVpssChnAttr[VpssChn0].stNormalize.bEnable         = CVI_FALSE;

  astVpssChnAttr[VpssChn1].u32Width                    = stVencSize.u32Width;
	astVpssChnAttr[VpssChn1].u32Height                   = stVencSize.u32Height;
	astVpssChnAttr[VpssChn1].enVideoFormat               = VIDEO_FORMAT_LINEAR;
	astVpssChnAttr[VpssChn1].enPixelFormat               = SAMPLE_PIXEL_FORMAT;
	astVpssChnAttr[VpssChn1].stFrameRate.s32SrcFrameRate = 15;
	astVpssChnAttr[VpssChn1].stFrameRate.s32DstFrameRate = 15;
	astVpssChnAttr[VpssChn1].u32Depth                    = 1;
	astVpssChnAttr[VpssChn1].bMirror                     = CVI_FALSE;
	astVpssChnAttr[VpssChn1].bFlip                       = CVI_FALSE;
	astVpssChnAttr[VpssChn1].stAspectRatio.enMode        = ASPECT_RATIO_AUTO;
	astVpssChnAttr[VpssChn1].stAspectRatio.bEnableBgColor = CVI_FALSE;
	astVpssChnAttr[VpssChn1].stAspectRatio.u32BgColor    = RGB_8BIT(0, 0, 0);
	astVpssChnAttr[VpssChn1].stNormalize.bEnable         = CVI_FALSE;

  VPSS_MODE_S stVPSSMode = {0};
  stVPSSMode.enMode = VPSS_MODE_DUAL;
  stVPSSMode.aenInput[0] = VPSS_INPUT_MEM;
  stVPSSMode.ViPipe[0] = 0;
  stVPSSMode.aenInput[1] = VPSS_INPUT_ISP;
  stVPSSMode.ViPipe[1] = 0;
  s32Ret = CVI_SYS_SetVPSSModeEx(&stVPSSMode);
  if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("set vi vpss mode failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

	/*start vpss*/
	abChnEnable[0] = CVI_TRUE;
	abChnEnable[1] = CVI_TRUE;
	s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
	}

	s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
	}

  s32Ret = SAMPLE_COMM_VI_Bind_VPSS(0, 0, 0);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

  s32Ret = CVI_VPSS_AttachVbPool(0, 0, 1);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vpss attach vb failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

  s32Ret = CVI_VPSS_AttachVbPool(0, 1, 0);
	if (s32Ret != CVI_SUCCESS) {
		SAMPLE_PRT("vpss attach vb failed. s32Ret: 0x%x !\n", s32Ret);
		return s32Ret;
	}

  // Get default VENC configurations
  SAMPLE_TDL_MW_CONFIG_S stMWConfig = {0};
  SAMPLE_TDL_Get_Input_Config(&stMWConfig.stVencConfig.stChnInputCfg);
  stMWConfig.stVencConfig.u32FrameWidth = stVencSize.u32Width;
  stMWConfig.stVencConfig.u32FrameHeight = stVencSize.u32Height;

  SAMPLE_TDL_Get_RTSP_Config(&stMWConfig.stRTSPConfig.stRTSPConfig);

  SAMPLE_TDL_MW_CONTEXT stMWContext = {0};
  s32Ret = SAMPLE_TDL_Init_WM(&stMWConfig, &stMWContext);
  if (s32Ret != CVI_SUCCESS) {
    printf("init middleware failed! ret=%x\n", s32Ret);
    return -1;
  }

  cvitdl_handle_t stTDLHandle = NULL;

  // Create TDL handle and assign VPSS Grp1 Device 0 to TDL SDK
  GOTO_IF_FAILED(CVI_TDL_CreateHandle2(&stTDLHandle, 1, 0), s32Ret, create_tdl_fail);

  GOTO_IF_FAILED(CVI_TDL_SetVBPool(stTDLHandle, 0, 2), s32Ret, create_service_fail);

  CVI_TDL_SetVpssTimeout(stTDLHandle, 1000);

  printf("==============================================\n");
  cli_handle_input("proc_sys");
  printf("==============================================\n");
  cli_handle_input("proc_mipi_rx");
  printf("==============================================\n");
  cli_handle_input("proc_vi_dbg");

  cvi_tpu_init();
	aos_msleep(1000);

  cvitdl_service_handle_t stServiceHandle = NULL;
  GOTO_IF_FAILED(CVI_TDL_OpenModel(stTDLHandle, CVI_TDL_SUPPORTED_MODEL_RETINAFACE, argv[1]),
                 s32Ret, setup_tdl_fail);

  bool bTrackingWithFeature = false;
  if (argc == 3) {
    // Tracking with face recognition features
    GOTO_IF_FAILED(CVI_TDL_OpenModel(stTDLHandle, CVI_TDL_SUPPORTED_MODEL_FACERECOGNITION, argv[2]),
                   s32Ret, setup_tdl_fail);
    bTrackingWithFeature = true;
  }

  // Init DeepSORT
  CVI_TDL_DeepSORT_Init(stTDLHandle, true);
  cvtdl_deepsort_config_t ds_conf;
  CVI_TDL_DeepSORT_GetDefaultConfig(&ds_conf);
  set_sample_mot_config(&ds_conf);
  CVI_TDL_DeepSORT_SetConfig(stTDLHandle, &ds_conf, -1, false);

  pthread_t stVencThread, stTDLThread;
  SAMPLE_TDL_VENC_THREAD_ARG_S venc_args = {
      .pstMWContext = &stMWContext,
      .stServiceHandle = stServiceHandle,
  };

  SAMPLE_TDL_TDL_THREAD_ARG_S ai_args = {.stTDLHandle = stTDLHandle,
                                         .bTrackingWithFeature = bTrackingWithFeature};


  pthread_create(&stVencThread, NULL, run_venc, &venc_args);
  pthread_create(&stTDLThread, NULL, run_tdl_thread, &ai_args);

  printf("==============================================\n");
  cli_handle_input("proc_vpss");

  pthread_join(stVencThread, NULL);
  pthread_join(stTDLThread, NULL);

setup_tdl_fail:
  cvi_tpu_deinit();
create_service_fail:
  CVI_TDL_DestroyHandle(stTDLHandle);
create_tdl_fail:
  SAMPLE_TDL_Destroy_MW(&stMWContext);

  return 0;
}

void sample_fdt(int argc, char **argv) {
	pthread_attr_t attr;
	pthread_t thread;
  int ret;
  pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, 64*1024);
  ret = pthread_create(&thread, &attr, fdt_main, (void *)argv);
  if(ret != CVI_SUCCESS) {
    printf("Error create adas thread! \n");
  }
  pthread_join(thread, NULL);
  printf("Adas thread finished! \n");
}

ALIOS_CLI_CMD_REGISTER(sample_fdt, sample_tdl_fdt, cvi_tdl sample face tracking);
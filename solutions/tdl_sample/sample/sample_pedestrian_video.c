#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/time.h>
#include <unistd.h>

#include <fcntl.h>
#include "cvi_ae.h"
#include "cvi_ae_comm.h"
#include "cvi_awb.h"
#include "cvi_awb_comm.h"
#include "cvi_buffer.h"
#include "cvi_comm_isp.h"
#include "cvi_comm_sns.h"
#include "cvi_isp.h"
#include "cvi_sns_ctrl.h"
#include "sample_comm.h"

#include <aos/cli.h>
#include "debug/debug_cli_cmd.h"

#include "core/utils/vpss_helper.h"
#include "cvi_tdl.h"
#include "cvi_tdl_media.h"
#include "cvi_tpu_interface.h"

static SAMPLE_VI_CONFIG_S g_stViConfig;
static SAMPLE_INI_CFG_S g_stIniCfg;
static SIZE_S g_stSize;
static CVI_BOOL g_abChnEnable[VPSS_MAX_PHY_CHN_NUM];

static int sys_vi_init(void)
{
    MMF_VERSION_S stVersion;
    SAMPLE_INI_CFG_S stIniCfg;
    SAMPLE_VI_CONFIG_S stViConfig;

    PIC_SIZE_E enPicSize;
    SIZE_S stSize;
    CVI_S32 s32Ret = CVI_SUCCESS;
    LOG_LEVEL_CONF_S log_conf;

    CVI_SYS_GetVersion(&stVersion);
    SAMPLE_PRT("MMF Version:%s\n", stVersion.version);

    log_conf.enModId  = CVI_ID_LOG;
    log_conf.s32Level = CVI_DBG_INFO;
    CVI_LOG_SetLevelConf(&log_conf);

    // Get config from ini if found.
    s32Ret = SAMPLE_COMM_VI_ParseIni(&stIniCfg);
    if (s32Ret != CVI_SUCCESS) {
        SAMPLE_PRT("Parse fail\n");
    } else {
        SAMPLE_PRT("Parse complete\n");
    }

    // Set sensor number
    CVI_VI_SetDevNum(stIniCfg.devNum);
    /************************************************
     * step1:  Config VI
     ************************************************/
    s32Ret = SAMPLE_COMM_VI_IniToViCfg(&stIniCfg, &stViConfig);
    if (s32Ret != CVI_SUCCESS)
        return s32Ret;

    memcpy(&g_stViConfig, &stViConfig, sizeof(SAMPLE_VI_CONFIG_S));
    memcpy(&g_stIniCfg, &stIniCfg, sizeof(SAMPLE_INI_CFG_S));

    /************************************************
     * step2:  Get input size
     ************************************************/
    s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(stIniCfg.enSnsType[0], &enPicSize);
    if (s32Ret != CVI_SUCCESS) {
        CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_VI_GetSizeBySensor failed with %#x\n", s32Ret);
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);
    if (s32Ret != CVI_SUCCESS) {
        CVI_TRACE_LOG(CVI_DBG_ERR, "SAMPLE_COMM_SYS_GetPicSize failed with %#x\n", s32Ret);
        return s32Ret;
    }
    memcpy(&g_stSize, &stSize, sizeof(SIZE_S));

    /************************************************
     * step3:  Init modules
     ************************************************/
    s32Ret = SAMPLE_PLAT_SYS_INIT(stSize);
    if (s32Ret != CVI_SUCCESS) {
        CVI_TRACE_LOG(CVI_DBG_ERR, "sys init failed. s32Ret: 0x%x !\n", s32Ret);
        return s32Ret;
    }

    s32Ret = SAMPLE_PLAT_VI_INIT(&stViConfig);
    if (s32Ret != CVI_SUCCESS) {
        CVI_TRACE_LOG(CVI_DBG_ERR, "vi init failed. s32Ret: 0x%x !\n", s32Ret);
        return s32Ret;
    }

    return CVI_SUCCESS;
}

static void sys_vi_deinit(void)
{
    SAMPLE_COMM_VI_DestroyIsp(&g_stViConfig);

    SAMPLE_COMM_VI_DestroyVi(&g_stViConfig);

    SAMPLE_COMM_VI_CLOSE();

    SAMPLE_COMM_SYS_Exit();
}

static CVI_S32 vpss_init(void)
{

    CVI_S32 s32Ret = CVI_SUCCESS;

    /************************************************
     * step4:  Init VPSS
     ************************************************/
    VPSS_GRP VpssGrp = 0;
    VPSS_GRP_ATTR_S stVpssGrpAttr;
    VPSS_CHN VpssChn                                     = VPSS_CHN0;
    CVI_BOOL abChnEnable[VPSS_MAX_PHY_CHN_NUM]           = {0};
    VPSS_CHN_ATTR_S astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM] = {0};

    stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
    stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;
    stVpssGrpAttr.enPixelFormat               = PIXEL_FORMAT_NV21;
    stVpssGrpAttr.u32MaxW                     = g_stSize.u32Width;
    stVpssGrpAttr.u32MaxH                     = g_stSize.u32Height;
    stVpssGrpAttr.u8VpssDev                   = 1;

    astVpssChnAttr[VpssChn].u32Width                    = 1280;
    astVpssChnAttr[VpssChn].u32Height                   = 720;
    astVpssChnAttr[VpssChn].enVideoFormat               = VIDEO_FORMAT_LINEAR;
    astVpssChnAttr[VpssChn].enPixelFormat               = PIXEL_FORMAT_RGB_888_PLANAR;
    astVpssChnAttr[VpssChn].stFrameRate.s32SrcFrameRate = 30;
    astVpssChnAttr[VpssChn].stFrameRate.s32DstFrameRate = 30;
    astVpssChnAttr[VpssChn].u32Depth                    = 1;
    astVpssChnAttr[VpssChn].bMirror                     = CVI_FALSE;
    astVpssChnAttr[VpssChn].bFlip                       = CVI_FALSE;
    astVpssChnAttr[VpssChn].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    astVpssChnAttr[VpssChn].stNormalize.bEnable         = CVI_FALSE;

    /*start vpss*/
    abChnEnable[0] = CVI_TRUE;
    memcpy(&g_abChnEnable, &abChnEnable, VPSS_MAX_PHY_CHN_NUM * sizeof(CVI_BOOL));

    s32Ret = SAMPLE_COMM_VPSS_Init(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
    if (s32Ret != CVI_SUCCESS) {
        SAMPLE_PRT("init vpss group failed. s32Ret: 0x%x !\n", s32Ret);
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
    if (s32Ret != CVI_SUCCESS) {
        SAMPLE_PRT("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_VI_Bind_VPSS(0, 0, VpssGrp);
    if (s32Ret != CVI_SUCCESS) {
        SAMPLE_PRT("vi bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
        return s32Ret;
    }

    return s32Ret;
}

static void vpss_deinit(void)
{
    SAMPLE_COMM_VI_UnBind_VPSS(0, 0, 0);
    SAMPLE_COMM_VPSS_Stop(0, &g_abChnEnable);
}

void* tdl_thread(void* args)
{
    char** argv    = (char**)args;
    CVI_S32 s32Ret = CVI_SUCCESS;
    VIDEO_FRAME_INFO_S stVideoFrame;

    cvitdl_handle_t tdl_handle = NULL;
    s32Ret                     = CVI_TDL_CreateHandle(&tdl_handle);
    if (s32Ret != CVI_SUCCESS) {
        printf("Create ai handle failed with %#x!\n", s32Ret);
        pthread_exit(NULL);
    }

    const char* fd_model = argv[1];
    s32Ret =
        CVI_TDL_OpenModel(tdl_handle, CVI_TDL_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN, fd_model);
    if (s32Ret != CVI_SUCCESS) {
        printf("open model failed with %#x!\n", s32Ret);
        pthread_exit(NULL);
    }

    cvtdl_face_t obj_meta = {0};

    while (1) {
        s32Ret = CVI_VPSS_GetChnFrame(0, 0, &stVideoFrame, -1);
        if (s32Ret != CVI_SUCCESS) {
            printf("vpss get chn frame failed, exit!\n");
            CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrame);
            break;
        }

        memset(&obj_meta, 0, sizeof(cvtdl_face_t));
        s32Ret = CVI_TDL_Detection(tdl_handle, &stVideoFrame,
                                   CVI_TDL_SUPPORTED_MODEL_MOBILEDETV2_PEDESTRIAN, &obj_meta);
        if (s32Ret != CVI_SUCCESS) {
            printf("CVI_TDL_Detection failed with %#x!\n", s32Ret);
            CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrame);
            break;
        }
        CVI_VPSS_ReleaseChnFrame(0, 0, &stVideoFrame);
        printf("obj count: %d\n", obj_meta.size);
    }

    CVI_TDL_Free(&obj_meta);
    CVI_TDL_DestroyHandle(tdl_handle);
    pthread_exit(NULL);
}

void sample_pd_video(int argc, char** argv)
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    pthread_attr_t attr;
    pthread_t thread;

    s32Ret = sys_vi_init();
    if (s32Ret != CVI_SUCCESS) {
        printf("sys_vi_init failed, exit!\n");
        return;
    }

    s32Ret = vpss_init();
    if (s32Ret != CVI_SUCCESS) {
        printf("VI VPSS failed, exit!\n");
        return;
    }

    cvi_tpu_init();
    aos_msleep(1000);

    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 64 * 1024);
    s32Ret = pthread_create(&thread, &attr, tdl_thread, (void*)argv);
    if (s32Ret != CVI_SUCCESS) {
        printf("Error create tdl thread! \n");
    }
    pthread_join(thread, NULL);
    printf("tdl thread finished! \n");

    sleep(1);

    vpss_deinit();
    sys_vi_deinit();
    cvi_tpu_deinit();

    return;
}
ALIOS_CLI_CMD_REGISTER(sample_pd_video, sample_pd_video, cvi_tdl sample pd with video);
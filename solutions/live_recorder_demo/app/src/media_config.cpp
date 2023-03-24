/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */

#include <string>
#include "cvi_common.h"
#include "cvi_comm_sys.h"
#include "cvi_vb.h"
#include "sys_uapi.h"
#include "cvi_param.h"
#include "media_video.h"
#include "vi_snsr_i2c.h"
#include "cvi_buffer.h"
#include "cvi_math.h"
#include "vi_isp.h"
#include "vi_uapi.h"
#include "common_vi.h"
#include "vpss_uapi.h"
#include "vo_uapi.h"
#include "rgn_uapi.h"
#include "ldc_platform.h"
#include "cvi_sys.h"
#include "cvi_vi.h"
#include "board_config.h"
#if CONFIG_SENSOE_IR0
#include "fc_custom_media.h"
#include "ir_custom_param.h"
#endif
#include "media_config.h"
#include <ulog/ulog.h>
#include <cx/record.h>
#include <cx/cloud_lv_service.h>

#define TAG "MediaConfig"

using namespace std;

static int _meida_sensor_init(PARAM_VI_CFG_S * pstViCtx,CVI_U8 *devNum)
{
    //Sensor
    SNS_COMBO_DEV_ATTR_S devAttr;
    memset(&devAttr, 0, sizeof(devAttr));
    CVI_S32 snsr_type[2];
    ISP_SNS_OBJ_S *pSnsObj[2];
    ISP_CMOS_SENSOR_IMAGE_MODE_S stSnsrMode[2];
    CVI_U8 dev_num;
    ISP_SNS_COMMBUS_U unSnsrBusInfo = {0};
    ISP_SENSOR_EXP_FUNC_S stSnsrSensorFunc[2] = {0};
    ISP_INIT_ATTR_S InitAttr = {0};
    ALG_LIB_S stAeLib = {0};
    ALG_LIB_S stAwbLib = {0};
    CVI_S32 s32Ret;
    struct snsr_rst_gpio_s snsr_gpio;

    if(pstViCtx == NULL) {
        LOGE(TAG, "%s pstViCtx NULL err \n",__func__);
        return CVI_FAILURE;
    }
    MEDIA_CHECK_RET(getSnsType(snsr_type, &dev_num), "getSnsType fail");
    for (CVI_U8 i = 0; i < *devNum; i++) {
        pSnsObj[i] = getSnsObj((SNS_TYPE_E)snsr_type[i]);
        MEDIA_CHECK_RET(getSnsMode(i, &stSnsrMode[i]), "stSnsrMode fail");
    }
    /* clock enable */
    // vip_clk_en();
    /************************************************
     * start sensor
     ************************************************/
    InitAttr.enGainMode = SNS_GAIN_MODE_SHARE;

    for (CVI_U8  i = 0; i < *devNum; ++i) {
        if (!pSnsObj[i]) {
            LOGE(TAG, "sns obj[%d] is null.\n", i);
            return CVI_FAILURE;
        }
        unSnsrBusInfo.s8I2cDev = pstViCtx->pstSensorCfg[i].s8I2cDev;
        snsr_gpio.snsr_rst_port_idx = pstViCtx->pstSensorCfg[i].u32Rst_port_idx;
        snsr_gpio.snsr_rst_pin = pstViCtx->pstSensorCfg[i].u32Rst_pin;
        snsr_gpio.snsr_rst_pol = pstViCtx->pstSensorCfg[i].u32Rst_pol;
        cvi_cif_reset_snsr_gpio_init(i, &snsr_gpio);
		InitAttr.u16UseHwSync = pstViCtx->pstSensorCfg[i].bHwSync;
        if(pstViCtx->pstSensorCfg[i].s32I2cAddr != -1) {
            pSnsObj[i]->pfnPatchI2cAddr(pstViCtx->pstSensorCfg[i].s32I2cAddr);
        }
        pSnsObj[i]->pfnSetInit(i, &InitAttr);
        LOGD(TAG, "bus info:%d\n", unSnsrBusInfo.s8I2cDev);
        pSnsObj[i]->pfnSetBusInfo(i, unSnsrBusInfo);
        pSnsObj[i]->pfnRegisterCallback(i, &stAeLib, &stAwbLib);
        pSnsObj[i]->pfnExpSensorCb(&stSnsrSensorFunc[i]);
        stSnsrSensorFunc[i].pfn_cmos_sensor_global_init(i);
        s32Ret = stSnsrSensorFunc[i].pfn_cmos_set_image_mode(i, &stSnsrMode[i]);
        if (s32Ret != CVI_SUCCESS) {
            LOGE(TAG, "sensor set image mode failed!\n");
            return CVI_FAILURE;
        }
        s32Ret = stSnsrSensorFunc[i].pfn_cmos_set_wdr_mode(i, stSnsrMode[i].u8SnsMode);
        if (s32Ret != CVI_SUCCESS) {
            LOGE(TAG, "sensor set wdr mode failed!\n");
            return CVI_FAILURE;
        }
        if(i==0) {
            cif_reset_snsr_gpio(i, 1);
        }
        cif_reset_mipi(i);
        usleep(100);
        pSnsObj[i]->pfnGetRxAttr(i, &devAttr);
        //devAttr.mipi_attr.lane_id = pstViCtx->pstSensorCfg[i].as16LaneId;
        cif_set_dev_attr(&devAttr);
        cif_enable_snsr_clk(i, 1);
        usleep(100);
        cif_reset_snsr_gpio(i, 0);
        usleep(1000);
        if (pSnsObj[i]->pfnSnsProbe) {
            s32Ret = pSnsObj[i]->pfnSnsProbe(i);
            if (s32Ret) {
                LOGE(TAG, "sensor probe failed!\n");
                return CVI_FAILURE;
            }
        }
    }
    return CVI_SUCCESS;
}

static int MediaVideoRGBVbInit()
{
    CVI_U32 	u32BlkSize;
    CVI_U32 	u32RotBlkSize;
    VB_CONFIG_S stVbConfig;

    memset(&stVbConfig, 0, sizeof(VB_CONFIG_S));

    int i=0;
	u32BlkSize = COMMON_GetPicBufferSize(320, 480, PIXEL_FORMAT_NV21, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32RotBlkSize = COMMON_GetPicBufferSize(480, 320, PIXEL_FORMAT_NV21, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSize = MAX2(u32BlkSize, u32RotBlkSize);

	stVbConfig.astCommPool[i].u32BlkSize	= u32BlkSize;
	stVbConfig.astCommPool[i].u32BlkCnt	= 3;
	LOGD(TAG, "common pool[%d] BlkSize %d count %d\n", i, u32BlkSize, stVbConfig.astCommPool[i].u32BlkCnt);

    i++;
    u32BlkSize = COMMON_GetPicBufferSize(480, 640, PIXEL_FORMAT_NV21, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32RotBlkSize = COMMON_GetPicBufferSize(640, 480, PIXEL_FORMAT_NV21, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSize = MAX2(u32BlkSize, u32RotBlkSize);

	stVbConfig.astCommPool[i].u32BlkSize	= u32BlkSize;
	stVbConfig.astCommPool[i].u32BlkCnt	= 2;
	LOGD(TAG, "common pool[%d] BlkSize %d count %d\n", i, u32BlkSize, stVbConfig.astCommPool[i].u32BlkCnt);

    i++;
	u32BlkSize = COMMON_GetPicBufferSize(960, 720, PIXEL_FORMAT_NV21, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32RotBlkSize = COMMON_GetPicBufferSize(720, 960, PIXEL_FORMAT_NV21, DATA_BITWIDTH_8, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
	u32BlkSize = MAX2(u32BlkSize, u32RotBlkSize);

	stVbConfig.astCommPool[i].u32BlkSize	= u32BlkSize;
	stVbConfig.astCommPool[i].u32BlkCnt	= 5;
	LOGD(TAG, "common pool[%d] BlkSize %d count %d\n", i, u32BlkSize, stVbConfig.astCommPool[i].u32BlkCnt);

    stVbConfig.u32MaxPoolCnt = i+1;

    MEDIA_CHECK_RET(CVI_VB_SetConfig(&stVbConfig), "CVI_VB_SetConfig failed\n");
    MEDIA_CHECK_RET(CVI_VB_Init(), "CVI_VB_Init failed\n");
    return 0;
}

int sensor_init_pre_hook(string sensor_name)
{
    extern PARAM_VI_CFG_S g_stViCtx;
    extern PARAM_VI_CFG_S g_stFcViCtx;
    if(sensor_name == "rgb0") {
        CustomEvent_IRGpioSet(0);
        CVI_U8 devNum = 0;

        devNum = g_stViCtx.u32WorkSnsCnt;
        MEDIA_CHECK_RET(_meida_sensor_init(&g_stViCtx, &devNum),"_meida_sensor_init fail");
        LOGD(TAG, "VIDEO The devNum is %d \n",devNum);
        CVI_VI_SetDevNum(devNum);
    } else if(sensor_name == "ir0") {
        CustomEvent_IRGpioSet(1);
        CVI_U8 devNum = 0;

        devNum = g_stFcViCtx.u32WorkSnsCnt;
        MEDIA_CHECK_RET(_meida_sensor_init(&g_stFcViCtx, &devNum),"_meida_sensor_init fail");
        LOGD(TAG, "VIDEO The devNum is %d \n",devNum);
        CVI_VI_SetDevNum(devNum);
    }
    return 0;
}

// extern cx_preview_hdl_t preview_handle;
extern cx_dvr_hdl_t record_handle;
extern cxlv_hdl_t g_cloud_lv_handle;
extern "C" void dvr_record_callback(cx_dvr_event_t event, void *data);
extern "C" void cloud_lv_callback_func(cx_lv_event_e event, void *data);

static pthread_mutex_t *sensor_toggle_lock = nullptr;
static int RgbSensorInit = 0;
int MediaVideoRGBInit()
{
    if(RgbSensorInit) {
        return 0;
    }
    if(!sensor_toggle_lock) {
        sensor_toggle_lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(sensor_toggle_lock, 0);
    }
    pthread_mutex_lock(sensor_toggle_lock);

    LOGD(TAG, "################## RGB enter\n");
    RgbSensorInit = 1;
    MEDIA_CHECK_RET(CVI_SYS_Init(), "CVI_SYS_Init failed\n");
    VPSS_MODE_S stVPSSMode;
    memset(&stVPSSMode, 0, sizeof(stVPSSMode));
    VI_VPSS_MODE_S stVIVPSSMode;
    memset(&stVIVPSSMode, 0, sizeof(stVIVPSSMode));
    stVIVPSSMode.aenMode[0] = VI_OFFLINE_VPSS_ONLINE;
    stVPSSMode.enMode = VPSS_MODE_DUAL;
    stVPSSMode.aenInput[0] = VPSS_INPUT_ISP;
    stVPSSMode.ViPipe[0] = 0;
    stVPSSMode.aenInput[1] = VPSS_INPUT_MEM;
    stVPSSMode.ViPipe[1] = 0;
    MEDIA_CHECK_RET(CVI_SYS_SetVIVPSSMode(&stVIVPSSMode),"CVI_SYS_SetVIVPSSMode failed\n");
    CVI_SYS_SetVPSSModeEx(&stVPSSMode);

    MediaVideoRGBVbInit();

    /* start all RGB service*/
    record_handle = cx_dvr_start("recorder", dvr_record_callback);
    // preview_handle = cx_preview_start("preview", PREVIEW_MODE_RGB);
    g_cloud_lv_handle = cx_lv_start("linkvisual", cloud_lv_callback_func);

    pthread_mutex_unlock(sensor_toggle_lock);

    return 0;
}

int MediaVideoRGBDeInit() 
{
    if(!RgbSensorInit) {
        return 0;
    }
    if(!sensor_toggle_lock) {
        sensor_toggle_lock = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(sensor_toggle_lock, 0);
    }
    pthread_mutex_lock(sensor_toggle_lock);

    /* close all RGB service*/
    // cx_preview_stop(preview_handle);
    cx_dvr_stop(record_handle);
    cx_lv_stop(g_cloud_lv_handle);

    RgbSensorInit = 0;
    LOGD(TAG, "################## RGB exit\n");
    CVI_VB_Exit();
    CVI_SYS_Exit();

    pthread_mutex_unlock(sensor_toggle_lock);

    return 0;
}

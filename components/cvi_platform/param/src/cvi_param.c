#include "cvi_param.h"


#define PARAM_DEBUG 0

#if PARAM_DEBUG
static void PARAM_Printf(void)
{
    PARAM_VI_CFG_S      * pstViCtx = PARAM_GET_MANAGER_CFG()->pstViCtx;
    PARAM_VPSS_CFG_S    * pstVpssCtx = PARAM_GET_MANAGER_CFG()->pstVpssCfg;
    PARAM_VENC_CFG_S    * pstVencCtx = PARAM_GET_MANAGER_CFG()->pstVencCfg;

    if (PARAM_GET_MANAGER_CFG()->pstModuleCtx->alios_sys_mode) {
        PARAM_SYS_CFG_S     *pstSysCtx = PARAM_GET_MANAGER_CFG()->pstSysCtx;
        aos_debug_printf("*********************SYS***************** \r\n");
        aos_debug_printf("The pstSysCtx->u8SbmCnt is %d \r\n", pstSysCtx->u8SbmCnt);
        for (int i = 0; i < pstSysCtx->u8SbmCnt; i++) {
            aos_debug_printf("The pstSysCtx->pstSbmCfg[%d].bEnable is %d \r\n", i, pstSysCtx->pstSbmCfg[i].bEnable);
            aos_debug_printf("The pstSysCtx->pstSbmCfg[%d].s32SbmGrp is %d \r\n", i, pstSysCtx->pstSbmCfg[i].s32SbmGrp);
            aos_debug_printf("The pstSysCtx->pstSbmCfg[%d].s32SbmChn is %d \r\n", i, pstSysCtx->pstSbmCfg[i].s32SbmChn);
            aos_debug_printf("The pstSysCtx->pstSbmCfg[%d].s32WrapBufLine is %d \r\n", i, pstSysCtx->pstSbmCfg[i].s32WrapBufLine);
            aos_debug_printf("The pstSysCtx->pstSbmCfg[%d].s32WrapBufSize is %d \r\n", i, pstSysCtx->pstSbmCfg[i].s32WrapBufSize);
        }
        aos_debug_printf("The pstSysCtx->bMipiSwitchEnable is %d \r\n", pstSysCtx->stSwitchCfg.bMipiSwitchEnable);
        aos_debug_printf("The pstSysCtx->u32MipiSwitchGpioIdx is %d \r\n", pstSysCtx->stSwitchCfg.u32MipiSwitchGpioIdx);
        aos_debug_printf("The pstSysCtx->u32MipiSwitchGpio is %d \r\n", pstSysCtx->stSwitchCfg.u32MipiSwitchGpio);
        aos_debug_printf("The pstSysCtx->bMipiSwitchPull is %d \r\n", pstSysCtx->stSwitchCfg.bMipiSwitchPull);
        aos_debug_printf("The pstSysCtx->u32SwitchPipe0 is %d \r\n", pstSysCtx->stSwitchCfg.u32SwitchPipe0);
        aos_debug_printf("The pstSysCtx->u32SwitchPipe1 is %d \r\n", pstSysCtx->stSwitchCfg.u32SwitchPipe1);
        aos_debug_printf("The pstSysCtx->u8ViCnt is %d \r\n", pstSysCtx->u8ViCnt);
        aos_debug_printf("The pstSysCtx->stVIVPSSMode.aenMode[0] is %d \r\n", pstSysCtx->stVIVPSSMode.aenMode[0]);
        aos_debug_printf("The pstSysCtx->stVIVPSSMode.aenMode[1] is %d \r\n", pstSysCtx->stVIVPSSMode.aenMode[1]);
        aos_debug_printf("The pstSysCtx->stVIVPSSMode.aenMode[3] is %d \r\n", pstSysCtx->stVIVPSSMode.aenMode[3]);
        aos_debug_printf("The pstSysCtx->stVPSSMode.ViPipe[0] is %d \r\n", pstSysCtx->stVPSSMode.ViPipe[0]);
        aos_debug_printf("The pstSysCtx->stVPSSMode.ViPipe[1] is %d \r\n", pstSysCtx->stVPSSMode.ViPipe[1]);
        aos_debug_printf("The pstSysCtx->stVPSSMode.enMode is %d \r\n", pstSysCtx->stVPSSMode.enMode);
        aos_debug_printf("The pstSysCtx->stVPSSMode.aenInput[0] is %d \r\n", pstSysCtx->stVPSSMode.aenInput[0]);
        aos_debug_printf("The pstSysCtx->stVPSSMode.aenInput[1] is %d \r\n", pstSysCtx->stVPSSMode.aenInput[1]);
        aos_debug_printf("The pstSysCtx->u8VbPoolCnt is %d \r\n", pstSysCtx->u8VbPoolCnt);
        for (int i = 0 ; i < pstSysCtx->u8VbPoolCnt; i++) {
            aos_debug_printf("pstSysCtx->pstVbPool[%d].u16width %d \r\n", i,pstSysCtx->pstVbPool[i].u16width);
            aos_debug_printf("pstSysCtx->pstVbPool[%d].u16height %d \r\n", i,pstSysCtx->pstVbPool[i].u16height);
            aos_debug_printf("pstSysCtx->pstVbPool[%d].fmt %d \r\n", i,pstSysCtx->pstVbPool[i].fmt);
            aos_debug_printf("pstSysCtx->pstVbPool[%d].enBitWidth %d \r\n", i,pstSysCtx->pstVbPool[i].enBitWidth);
            aos_debug_printf("pstSysCtx->pstVbPool[%d].enCmpMode %d \r\n", i,pstSysCtx->pstVbPool[i].enCmpMode);
            aos_debug_printf("pstSysCtx->pstVbPool[%d].u8VbBlkCnt %d \r\n", i,pstSysCtx->pstVbPool[i].u8VbBlkCnt);
        }
    }

    if (PARAM_GET_MANAGER_CFG()->pstModuleCtx->alios_vi_mode) {
        aos_debug_printf("*********************VI***************** \r\n");
        aos_debug_printf("pstViCtx->u32WorkSnsCnt is %d \r\n", pstViCtx->u32WorkSnsCnt);
        aos_debug_printf("pstViCtx->u32IspSceneNum is %d \r\n", pstViCtx->u32IspSceneNum);
        aos_debug_printf("pstViCtx->bFastConverge is %d \r\n", pstViCtx->bFastConverge);
        for (int i = 0; i < pstViCtx->u32WorkSnsCnt; i++) {
            aos_debug_printf("pstViCtx->pstSensorCfg[%d].s32Framerate %d \r\n", i,pstViCtx->pstSensorCfg[i].s32Framerate);
            aos_debug_printf("pstViCtx->pstSensorCfg[%d].enWDRMode %d \r\n", i,pstViCtx->pstSensorCfg[i].enWDRMode);
            aos_debug_printf("pstViCtx->pstSensorCfg[%d].s8I2cDev %d \r\n", i,pstViCtx->pstSensorCfg[i].s8I2cDev);
            aos_debug_printf("pstViCtx->pstSensorCfg[%d].s32BusId %d \r\n", i,pstViCtx->pstSensorCfg[i].s32BusId);
            aos_debug_printf("pstViCtx->pstSensorCfg[%d].s32I2cAddr %d \r\n", i,pstViCtx->pstSensorCfg[i].s32I2cAddr);
            aos_debug_printf("pstViCtx->pstSensorCfg[%d].MipiDev %d \r\n", i,pstViCtx->pstSensorCfg[i].MipiDev);
            aos_debug_printf("pstViCtx->pstSensorCfg[%d].bMclkEn %d \r\n", i,pstViCtx->pstSensorCfg[i].bMclkEn);
            aos_debug_printf("pstViCtx->pstSensorCfg[%d].u8Mclk %d \r\n", i,pstViCtx->pstSensorCfg[i].u8Mclk);
            aos_debug_printf("pstViCtx->pstSensorCfg[%d].u8MclkCam %d \r\n", i,pstViCtx->pstSensorCfg[i].u8MclkCam);
            aos_debug_printf("pstViCtx->pstSensorCfg[%d].u8MclkFreq %d \r\n", i,pstViCtx->pstSensorCfg[i].u8MclkFreq);
            aos_debug_printf("pstViCtx->pstSensorCfg[%d].u8Orien %d \r\n", i,pstViCtx->pstSensorCfg[i].u8Orien);
            aos_debug_printf("pstViCtx->pstSensorCfg[%d].bHwSync %d \r\n", i,pstViCtx->pstSensorCfg[i].bHwSync);
            aos_debug_printf("pstViCtx->pstSensorCfg[%d].u32Rst_port_idx %d \r\n", i,pstViCtx->pstSensorCfg[i].u32Rst_port_idx);
            aos_debug_printf("pstViCtx->pstSensorCfg[%d].u32Rst_pin %d \r\n", i,pstViCtx->pstSensorCfg[i].u32Rst_pin);
            aos_debug_printf("pstViCtx->pstSensorCfg[%d].u32Rst_pol %d \r\n", i,pstViCtx->pstSensorCfg[i].u32Rst_pol);
            aos_debug_printf("pstViCtx->pstSensorCfg[%d].u8Rotation %d \r\n", i,pstViCtx->pstSensorCfg[i].u8Rotation);
            aos_debug_printf("pstViCtx->pstSensorCfg[%d].enSnsType %d \r\n", i,pstViCtx->pstSensorCfg[i].enSnsType);


            //vi param
            aos_debug_printf("pstViCtx->pstChnInfo[%d].s32ChnId %d \r\n", i,pstViCtx->pstChnInfo[i].s32ChnId);
            aos_debug_printf("pstViCtx->pstChnInfo[%d].enWDRMode %d \r\n", i,pstViCtx->pstChnInfo[i].enWDRMode);
            aos_debug_printf("pstViCtx->pstChnInfo[%d].enDynamicRange %d \r\n", i,pstViCtx->pstChnInfo[i].enDynamicRange);
            aos_debug_printf("pstViCtx->pstChnInfo[%d].bYuvBypassPath %d \r\n", i,pstViCtx->pstChnInfo[i].bYuvBypassPath);
            aos_debug_printf("pstViCtx->pstChnInfo[%d].f32Fps %d \r\n", i,(int)pstViCtx->pstChnInfo[i].f32Fps);
            aos_debug_printf("pstViCtx->pstChnInfo[%d].u32Width %d \r\n", i,pstViCtx->pstChnInfo[i].u32Width);
            aos_debug_printf("pstViCtx->pstChnInfo[%d].u32Height %d \r\n", i,pstViCtx->pstChnInfo[i].u32Height);
            aos_debug_printf("pstViCtx->pstChnInfo[%d].enVideoFormat %d \r\n", i,pstViCtx->pstChnInfo[i].enVideoFormat);
            aos_debug_printf("pstViCtx->pstChnInfo[%d].enCompressMode %d \r\n", i,pstViCtx->pstChnInfo[i].enCompressMode);

            for (unsigned int j = 0; j < VI_MAX_PIPE_NUM; j++) {
                aos_debug_printf("pstViCtx->pstPipeInfo[%d].pipe[%d] %d \r\n", i, j, pstViCtx->pstPipeInfo[i].pipe[j]);
            }

            aos_debug_printf("pstViCtx->pstIspCfg[%d].s8FastConvergeAvailableNode %d \r\n", i,pstViCtx->pstIspCfg[i].s8FastConvergeAvailableNode);
            for (int j = 0; j < pstViCtx->pstIspCfg[i].s8FastConvergeAvailableNode; j++) {
                    aos_debug_printf("pstViCtx->pstIspCfg[%d].as16firstFrLuma[%d] %d\r\n", i,j, pstViCtx->pstIspCfg[i].as16firstFrLuma[j]);
                    aos_debug_printf("pstViCtx->pstIspCfg[%d].as16targetBv[%d] %d\r\n", i,j, pstViCtx->pstIspCfg[i].as16targetBv[j]);
            }
        }
    }

    if (PARAM_GET_MANAGER_CFG()->pstModuleCtx->alios_vpss_mode) {
        aos_debug_printf("*********************VPSS***************** \r\n");
        aos_debug_printf("pstVpssCtx->u8GrpCnt:%d. \r\n", pstVpssCtx->u8GrpCnt);
        for (int i = 0; i < pstVpssCtx->u8GrpCnt; i++) {
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].bEnable:%d. \r\n"
                , i, pstVpssCtx->pstVpssGrpCfg[i].bEnable);
            if (pstVpssCtx->pstVpssGrpCfg[i].bEnable) {
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].VpssGrp:%d. \r\n"
                    , i, pstVpssCtx->pstVpssGrpCfg[i].VpssGrp);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].stVpssGrpAttr.enPixelFormat:%d. \r\n"
                    , i, pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpAttr.enPixelFormat);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].stVpssGrpAttr.stFrameRate:%d. \r\n"
                    , i, pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpAttr.stFrameRate.s32SrcFrameRate);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].stVpssGrpAttr.dst_framerate:%d. \r\n"
                    , i, pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpAttr.stFrameRate.s32DstFrameRate);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].stVpssGrpAttr.u8VpssDev:%d. \r\n"
                    , i, pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpAttr.u8VpssDev);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].stVpssGrpAttr.u32MaxW:%d. \r\n"
                    , i, pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpAttr.u32MaxW);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].stVpssGrpAttr.u32MaxH:%d. \r\n"
                    , i, pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpAttr.u32MaxH);

                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].stVpssGrpCropInfo.bEnable:%d. \r\n"
                    , i, pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpCropInfo.bEnable);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].stVpssGrpCropInfo.enCropCoordinate:%d. \r\n"
                    , i, pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpCropInfo.enCropCoordinate);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].stVpssGrpCropInfo.stCropRect.s32X:%d. \r\n"
                    , i, pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpCropInfo.stCropRect.s32X);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].stVpssGrpCropInfo.stCropRect.s32Y:%d. \r\n"
                    , i, pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpCropInfo.stCropRect.s32Y);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].stVpssGrpCropInfo.stCropRect.u32Width:%d. \r\n"
                    , i, pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpCropInfo.stCropRect.u32Width);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].stVpssGrpCropInfo.stCropRect.u32Height:%d. \r\n"
                    , i, pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpCropInfo.stCropRect.u32Height);

                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].bBindMode:%d. \r\n", i, pstVpssCtx->pstVpssGrpCfg[i].bBindMode);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].s32BindVidev:%d. \r\n", i, pstVpssCtx->pstVpssGrpCfg[i].s32BindVidev);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].astChn[0].enModId:%d. \r\n", i, pstVpssCtx->pstVpssGrpCfg[i].astChn[0].enModId);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].astChn[0].s32DevId:%d. \r\n", i, pstVpssCtx->pstVpssGrpCfg[i].astChn[0].s32DevId);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].astChn[0].s32ChnId:%d. \r\n", i, pstVpssCtx->pstVpssGrpCfg[i].astChn[0].s32ChnId);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].astChn[1].enModId:%d. \r\n", i, pstVpssCtx->pstVpssGrpCfg[i].astChn[1].enModId);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].astChn[1].s32DevId:%d. \r\n", i, pstVpssCtx->pstVpssGrpCfg[i].astChn[1].s32DevId);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].astChn[1].s32ChnId:%d. \r\n", i, pstVpssCtx->pstVpssGrpCfg[i].astChn[1].s32ChnId);

                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].u8ChnCnt:%d. \r\n", i, pstVpssCtx->pstVpssGrpCfg[i].u8ChnCnt);
            }
            for (int j = 0; j < pstVpssCtx->pstVpssGrpCfg[i].u8ChnCnt; j++) {
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].abChnEnable:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].abChnEnable);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.u32Width:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.u32Width);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.u32Height:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.u32Height);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.enVideoFormat:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.enVideoFormat);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.enPixelFormat:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.enPixelFormat);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.stFrameRate.s32SrcFrameRate:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.stFrameRate.s32SrcFrameRate);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.stFrameRate.s32DstFrameRate:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.stFrameRate.s32DstFrameRate);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.u32Depth:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.u32Depth);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.bMirror:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.bMirror);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.bFlip:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.bFlip);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.stAspectRatio.enMode:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.stAspectRatio.enMode);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.stAspectRatio.stVideoRect.s32X:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.stAspectRatio.stVideoRect.s32X);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.stAspectRatio.stVideoRect.s32Y:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.stAspectRatio.stVideoRect.s32Y);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.stAspectRatio.stVideoRect.u32Width:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.stAspectRatio.stVideoRect.u32Width);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.stAspectRatio.stVideoRect.u32Height:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.stAspectRatio.stVideoRect.u32Height);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.stAspectRatio.bEnableBgColor:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.stAspectRatio.bEnableBgColor);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.stAspectRatio.u32BgColor:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.stAspectRatio.u32BgColor);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.stNormalize.bEnable:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.stNormalize.bEnable);

                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnCropInfo.bEnable:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnCropInfo.bEnable);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnCropInfo.enCropCoordinate:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnCropInfo.enCropCoordinate);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnCropInfo.stCropRect.s32X:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnCropInfo.stCropRect.s32X);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnCropInfo.stCropRect.s32Y:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnCropInfo.stCropRect.s32Y);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnCropInfo.stCropRect.u32Width:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnCropInfo.stCropRect.u32Width);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnCropInfo.stCropRect.u32Height:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnCropInfo.stCropRect.u32Height);

                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnBufWrap.bEnable:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnBufWrap.bEnable);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnBufWrap.u32BufLine:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnBufWrap.u32BufLine);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnBufWrap.u32WrapBufferSize:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnBufWrap.u32WrapBufferSize);

                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].u8VpssAttachEnable:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].u8VpssAttachEnable);
                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].u8VpssAttachId:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].u8VpssAttachId);

                aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].u8Rotation:%d. \r\n"
                    , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].u8Rotation);
            }
        }
    }

    if (PARAM_GET_MANAGER_CFG()->pstModuleCtx->alios_venc_mode) {
        aos_debug_printf("*********************VENC***************** \r\n");
        aos_debug_printf("pstVencCtx->s32VencChnCnt:%d \r\n", pstVencCtx->s32VencChnCnt);
        for (unsigned int venc_idx = 0; venc_idx < pstVencCtx->s32VencChnCnt; venc_idx++) {
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam: \r\n", venc_idx);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.VencChn:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.u8VencChn);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.bEnable:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.bEnable);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.bStart:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.bStart);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.enBindMode:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.enBindMode);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.astChn[0].enModId:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.astChn[0].enModId);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.astChn[0].s32DevId:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.astChn[0].s32DevId);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.astChn[0].s32ChnId:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.astChn[0].s32ChnId);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.astChn[1].enModId:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.astChn[1].enModId);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.astChn[1].s32DevId:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.astChn[1].s32DevId);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.astChn[1].s32ChnId:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.astChn[1].s32ChnId);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.u8Profile:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.u8Profile);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.u16Width:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.u16Width);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.u16Height:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.u16Height);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.u8EsBufQueueEn:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.u8EsBufQueueEn);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.u16EnType:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.u16EnType);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.u32BitStreamBufSize:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.u32BitStreamBufSize);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.StreamTo:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.StreamTo);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.u8EntropyEncModeI:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.u8EntropyEncModeI);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.u8EntropyEncModeP:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.u8EntropyEncModeP);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stGopParam.u16gopMode:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stGopParam.u16gopMode);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stGopParam.s8IPQpDelta:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stGopParam.s8IPQpDelta);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8DstFrameRate:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8DstFrameRate);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u16BitRate:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u16BitRate);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8Qfactor:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8Qfactor);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u32MaxBitRate:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u32MaxBitRate);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8VariFpsEn:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8VariFpsEn);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8StartTime:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8StartTime);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u16RcMode:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u16RcMode);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u16FirstFrmstartQp:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u16FirstFrmstartQp);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u16InitialDelay:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u16InitialDelay);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u16ThrdLv:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u16ThrdLv);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u16BgDeltaQp:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u16BgDeltaQp);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8MinIprop:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8MinIprop);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8MaxIprop:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8MaxIprop);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8MinIqp:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8MinIqp);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8MaxIqp:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8MaxIqp);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8MinQp:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8MinIqp);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8MaxQp:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8MaxQp);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8MaxReEncodeTimes:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8MaxReEncodeTimes);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8QpMapEn:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8QpMapEn);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8ChangePos:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8ChangePos);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.bSingleCore:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.bSingleCore);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u32FixedIQp:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u32FixedIQp);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u32FixedPQp:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u32FixedPQp);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u32Duration:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u32Duration);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.s32MinStillPercent:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.s32MinStillPercent);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u32MaxStillQP:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u32MaxStillQP);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u32MinStillPSNR:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u32MinStillPSNR);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u32MotionSensitivity:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u32MotionSensitivity);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.s32AvbrFrmLostOpen:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.s32AvbrFrmLostOpen);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.s32AvbrFrmGap:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.s32AvbrFrmGap);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.s32AvbrPureStillThr:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.s32AvbrPureStillThr);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.s16Quality:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.s16Quality);
            aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.s16MCUPerECS:%d \r\n"
                , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.s16MCUPerECS);
        }
    }
}
#endif


//parm 结构 入口
CVI_S32 PARAM_LoadCfg(CVI_VOID) {
    //Media 流媒体 能力集
    PARAM_INIT_MANAGER_CFG();
#if PARAM_DEBUG
    PARAM_Printf();
#endif
    return 0;
}
//MODULE
PARAM_MODULE_CFG_S *PARAM_getModuleCtx(void)
{
    if(PARAM_GET_MANAGER_CFG() == NULL) {
        return NULL;
    }
    return PARAM_GET_MANAGER_CFG()->pstModuleCtx;
}
//VB
PARAM_SYS_CFG_S *PARAM_getSysCtx(void)
{
    if(PARAM_GET_MANAGER_CFG() == NULL) {
        return NULL;
    }
    return PARAM_GET_MANAGER_CFG()->pstSysCtx;
}
#if CONFIG_SUPPORT_VI
//VI
PARAM_VI_CFG_S *PARAM_getViCtx(void)
{
    return PARAM_GET_MANAGER_CFG()->pstViCtx;
}
#endif
//VPSS
PARAM_VPSS_CFG_S *PARAM_getVpssCtx(void)
{
    if(PARAM_GET_MANAGER_CFG() == NULL) {
        return NULL;
    }
    return PARAM_GET_MANAGER_CFG()->pstVpssCfg;
}
//VENC
PARAM_VENC_CFG_S *PARAM_getVencCtx(void)
{
    if(PARAM_GET_MANAGER_CFG() == NULL) {
        return NULL;
    }
    return PARAM_GET_MANAGER_CFG()->pstVencCfg;
}
//VO
PARAM_VO_CFG_S *PARAM_getVoCtx(void)
{
    if(PARAM_GET_MANAGER_CFG() == NULL) {
        return NULL;
    }
    return PARAM_GET_MANAGER_CFG()->pstVoCfg;
}

void PARAM_setPipeline(int pipeline)
{
    PARAM_SET_MANAGER_CFG_PIPE(pipeline);
}

int PARAM_getPipeline(void)
{
    return PARAM_GET_MANAGER_CFG_PIPE();
}
#if CONFIG_SUPPORT_VI
static int s_scene_mode = 0;

void PARAM_setSceneMode(int mode)
{
    s_scene_mode = mode;
}

int PARAM_getSceneMode()
{
    return s_scene_mode;
}

int PARAM_Reinit_RawReplay(void)
{
    PARAM_getSysCtx()->pstVbPool->u8VbBlkCnt = 2;
    PARAM_getSysCtx()->u8VbPoolCnt = 1;
    PARAM_getSysCtx()->u8ViCnt = 2;
    PARAM_getSysCtx()->u8VbPoolCnt = 1;

    PARAM_getSysCtx()->stVPSSMode.enMode = VPSS_MODE_DUAL;
    PARAM_getSysCtx()->stVIVPSSMode.aenMode[0] = VI_OFFLINE_VPSS_ONLINE;
    PARAM_getSysCtx()->stVPSSMode.aenInput[0] = VPSS_INPUT_MEM;
    PARAM_getSysCtx()->stVIVPSSMode.aenMode[1] = VI_OFFLINE_VPSS_ONLINE;
    PARAM_getSysCtx()->stVPSSMode.aenInput[1] = VPSS_INPUT_ISP;

    PARAM_getViCtx()->u32WorkSnsCnt = 1;
    PARAM_getViCtx()->pstChnInfo[0].enCompressMode = COMPRESS_MODE_NONE;
    PARAM_getViCtx()->pstChnInfo[0].bYuvBypassPath = CVI_FALSE;

    PARAM_getVpssCtx()->u8GrpCnt = 1;
    PARAM_getVpssCtx()->pstVpssGrpCfg[0].VpssGrp = 0;
    PARAM_getVpssCtx()->pstVpssGrpCfg[0].u8ChnCnt = 1;
    PARAM_getVpssCtx()->pstVpssGrpCfg[0].s32BindVidev = 0;
    PARAM_getVpssCtx()->pstVpssGrpCfg[0].stVpssGrpAttr.u8VpssDev = 1;
    PARAM_getVpssCtx()->pstVpssGrpCfg[0].bBindMode = CVI_FALSE;
    PARAM_getVpssCtx()->pstVpssGrpCfg[0].astChn[0].enModId = CVI_ID_VI;
    PARAM_getVpssCtx()->pstVpssGrpCfg[0].astChn[0].s32DevId = 0;
    PARAM_getVpssCtx()->pstVpssGrpCfg[0].astChn[0].s32ChnId = 0;
    PARAM_getVpssCtx()->pstVpssGrpCfg[0].astChn[1].enModId = CVI_ID_VPSS;
    PARAM_getVpssCtx()->pstVpssGrpCfg[0].astChn[1].s32DevId = 0;
    PARAM_getVpssCtx()->pstVpssGrpCfg[0].astChn[1].s32ChnId = 0;

    PARAM_getVencCtx()->s32VencChnCnt = 1;
    PARAM_getVencCtx()->pstVencChnCfg[0].stChnParam.u8VencChn = 0;
    return 0;
}
#endif

int _param_check_head(unsigned char * buffer)
{
    PARTITION_CHECK_HAED_S * pstParam = (PARTITION_CHECK_HAED_S *)(buffer);
    if (buffer == NULL) {
        return -1;
    }
    if (pstParam->magic_number != 0xDEADBEEF) {
        return -1;
    }
    return 0;
}


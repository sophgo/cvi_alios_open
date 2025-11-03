/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2022. All rights reserved.
 *
 * File Name: include/cvi_isp.h
 * Description:
 */

#ifndef __CVI_ISP_H__
#define __CVI_ISP_H__

#include "stdio.h"

#include "cvi_comm_isp.h"
#include "cvi_comm_3a.h"
#include "cvi_comm_sns.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

typedef struct _ISP_FUNC_FROM_OUT_S {
	CVI_S32 (*pfnMediaVideoDeinit)(CVI_BOOL);
	CVI_S32 (*pfnMediaVideoInit)(CVI_BOOL);
} ISP_FUNC_FROM_OUT_S;

CVI_S32 CVI_ISP_Client_Init(VI_PIPE ViPipe);
CVI_S32 CVI_ISP_Client_Exit(VI_PIPE ViPipe);
/* Firmware Main Operation */
CVI_S32 CVI_ISP_Init(VI_PIPE ViPipe);
CVI_S32 CVI_ISP_MemInit(VI_PIPE ViPipe);
CVI_S32 CVI_ISP_Run(VI_PIPE ViPipe);
CVI_S32 CVI_ISP_RunOnce(VI_PIPE ViPipe);
CVI_S32 CVI_ISP_Exit(VI_PIPE ViPipe);



/* Each Param set / get Operation */
CVI_S32 CVI_ISP_SetPubAttr(VI_PIPE ViPipe, const ISP_PUB_ATTR_S *pstPubAttr);
CVI_S32 CVI_ISP_GetPubAttr(VI_PIPE ViPipe, ISP_PUB_ATTR_S *pstPubAttr);
CVI_S32 CVI_ISP_SetModParam(const ISP_MOD_PARAM_S *pstModParam);
CVI_S32 CVI_ISP_GetModParam(ISP_MOD_PARAM_S *pstModParam);
CVI_S32 CVI_ISP_SetCtrlParam(VI_PIPE ViPipe,
				const ISP_CTRL_PARAM_S *pstIspCtrlParam);
CVI_S32 CVI_ISP_GetCtrlParam(VI_PIPE ViPipe, ISP_CTRL_PARAM_S *pstIspCtrlParam);
CVI_S32 CVI_ISP_SetFMWState(VI_PIPE ViPipe, const ISP_FMW_STATE_E enState);
CVI_S32 CVI_ISP_GetFMWState(VI_PIPE ViPipe, ISP_FMW_STATE_E *penState);
/* sensor related register */
CVI_S32 CVI_ISP_SensorRegCallBack(VI_PIPE ViPipe,
				ISP_SNS_ATTR_INFO_S *pstSnsAttrInfo, ISP_SENSOR_REGISTER_S *pstRegister);
CVI_S32 CVI_ISP_SensorUnRegCallBack(VI_PIPE ViPipe, SENSOR_ID SensorId);

/* 3a algo &static related function. */
CVI_S32 CVI_AWB_Register(VI_PIPE ViPipe, ALG_LIB_S *pstAeLib);
CVI_S32 CVI_AWB_UnRegister(VI_PIPE ViPipe, ALG_LIB_S *pstAeLib);

CVI_S32 CVI_ISP_AELibRegCallBack(VI_PIPE ViPipe, ALG_LIB_S *pstAeLib,
				ISP_AE_REGISTER_S *pstRegister);
CVI_S32 CVI_ISP_AELibUnRegCallBack(VI_PIPE ViPipe, ALG_LIB_S *pstAeLib);

CVI_S32 CVI_ISP_AWBLibRegCallBack(VI_PIPE ViPipe, ALG_LIB_S *pstAwbLib,
				ISP_AWB_REGISTER_S *pstRegister);
CVI_S32 CVI_ISP_AWBLibUnRegCallBack(VI_PIPE ViPipe, ALG_LIB_S *pstAwbLib);

CVI_S32 CVI_ISP_AFLibRegCallBack(VI_PIPE ViPipe, ALG_LIB_S *pstAfLib,
				ISP_AF_REGISTER_S *pstRegister);
CVI_S32 CVI_ISP_AFLibUnRegCallBack(VI_PIPE ViPipe, ALG_LIB_S *pstAwbLib);

CVI_S32 CVI_ISP_SetBindAttr(VI_PIPE ViPipe, const ISP_BIND_ATTR_S *pstBindAttr);
CVI_S32 CVI_ISP_GetBindAttr(VI_PIPE ViPipe, ISP_BIND_ATTR_S *pstBindAttr);

CVI_S32 CVI_ISP_SetModuleControl(VI_PIPE ViPipe, const ISP_MODULE_CTRL_U *punModCtrl);
CVI_S32 CVI_ISP_GetModuleControl(VI_PIPE ViPipe, ISP_MODULE_CTRL_U *punModCtrl);
CVI_S32 CVI_ISP_SetRegister(VI_PIPE ViPipe, CVI_U32 u32Addr, CVI_U32 u32Value);
CVI_S32 CVI_ISP_GetRegister(VI_PIPE ViPipe, CVI_U32 u32Addr, CVI_U32 *pu32Value);

//-----------------------------------------------------------------------------
//  Black Level Correction(BLC)
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetBlackLevelAttr(VI_PIPE ViPipe, const ISP_BLACK_LEVEL_ATTR_S *pstBlackLevelAttr);
CVI_S32 CVI_ISP_GetBlackLevelAttr(VI_PIPE ViPipe, ISP_BLACK_LEVEL_ATTR_S *pstBlackLevelAttr);

//-----------------------------------------------------------------------------
//  Local Black Level Correction(LBLC)
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetLblcAttr(VI_PIPE ViPipe, const ISP_LBLC_ATTR_S *pstLblcAttr);
CVI_S32 CVI_ISP_GetLblcAttr(VI_PIPE ViPipe, ISP_LBLC_ATTR_S *pstLblcAttr);
CVI_S32 CVI_ISP_SetLblcLutAttr(VI_PIPE ViPipe, const ISP_LBLC_LUT_ATTR_S *pstLblcLutAttr);
CVI_S32 CVI_ISP_GetLblcLutAttr(VI_PIPE ViPipe, ISP_LBLC_LUT_ATTR_S *pstLblcLutAttr);

//-----------------------------------------------------------------------------
//  Dead pixel correction(DPC)
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetDPDynamicAttr(VI_PIPE ViPipe, const ISP_DP_DYNAMIC_ATTR_S *pstDPCDynamicAttr);
CVI_S32 CVI_ISP_GetDPDynamicAttr(VI_PIPE ViPipe, ISP_DP_DYNAMIC_ATTR_S *pstDPCDynamicAttr);

//-----------------------------------------------------------------------------
//  Crosstalk
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetCrosstalkAttr(VI_PIPE ViPipe, const ISP_CROSSTALK_ATTR_S *pstCrosstalkAttr);
CVI_S32 CVI_ISP_GetCrosstalkAttr(VI_PIPE ViPipe, ISP_CROSSTALK_ATTR_S *pstCrosstalkAttr);

//-----------------------------------------------------------------------------
//  Bayer domain noise reduction (BNR)
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetBNRAttr(VI_PIPE ViPipe, const ISP_BNR_ATTR_S *pstBNRAttr);
CVI_S32 CVI_ISP_GetBNRAttr(VI_PIPE ViPipe, ISP_BNR_ATTR_S *pstBNRAttr);
CVI_S32 CVI_ISP_SetBNRFilterAttr(VI_PIPE ViPipe, const ISP_BNR_FILTER_ATTR_S *pstBNRFilterAttr);
CVI_S32 CVI_ISP_GetBNRFilterAttr(VI_PIPE ViPipe, ISP_BNR_FILTER_ATTR_S *pstBNRFilterAttr);

//-----------------------------------------------------------------------------
//  Demosaic
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetDemosaicAttr(VI_PIPE ViPipe, const ISP_DEMOSAIC_ATTR_S *pstDemosaicAttr);
CVI_S32 CVI_ISP_GetDemosaicAttr(VI_PIPE ViPipe, ISP_DEMOSAIC_ATTR_S *pstDemosaicAttr);
CVI_S32 CVI_ISP_SetDemosaicDemoireAttr(VI_PIPE ViPipe, const ISP_DEMOSAIC_DEMOIRE_ATTR_S *pstDemosaicDemoireAttr);
CVI_S32 CVI_ISP_GetDemosaicDemoireAttr(VI_PIPE ViPipe, ISP_DEMOSAIC_DEMOIRE_ATTR_S *pstDemosaicDemoireAttr);

//-----------------------------------------------------------------------------
//  Mesh lens shading correction (MLSC)
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetMeshShadingAttr(VI_PIPE ViPipe, const ISP_MESH_SHADING_ATTR_S *pstMeshShadingAttr);
CVI_S32 CVI_ISP_GetMeshShadingAttr(VI_PIPE ViPipe, ISP_MESH_SHADING_ATTR_S *pstMeshShadingAttr);
CVI_S32 CVI_ISP_SetMeshShadingGainLutAttr(VI_PIPE ViPipe,
				const ISP_MESH_SHADING_GAIN_LUT_ATTR_S *pstMeshShadingGainLutAttr);
CVI_S32 CVI_ISP_GetMeshShadingGainLutAttr(VI_PIPE ViPipe,
				ISP_MESH_SHADING_GAIN_LUT_ATTR_S *pstMeshShadingGainLutAttr);

//-----------------------------------------------------------------------------
//  Radial lens shading correction (MLSC)
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetRadialShadingAttr(VI_PIPE ViPipe, const ISP_RADIAL_SHADING_ATTR_S *pstRadialShadingAttr);
CVI_S32 CVI_ISP_GetRadialShadingAttr(VI_PIPE ViPipe, ISP_RADIAL_SHADING_ATTR_S *pstRadialShadingAttr);
CVI_S32 CVI_ISP_SetRadialShadingGainLutAttr(VI_PIPE ViPipe,
				const ISP_RADIAL_SHADING_GAIN_LUT_ATTR_S *pstRadialShadingGainLutAttr);
CVI_S32 CVI_ISP_GetRadialShadingGainLutAttr(VI_PIPE ViPipe,
				ISP_RADIAL_SHADING_GAIN_LUT_ATTR_S *pstRadialShadingGainLutAttr);

//-----------------------------------------------------------------------------
//  CCM
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetSaturationAttr(VI_PIPE ViPipe, const ISP_SATURATION_ATTR_S *pstSaturationAttr);
CVI_S32 CVI_ISP_GetSaturationAttr(VI_PIPE ViPipe, ISP_SATURATION_ATTR_S *pstSaturationAttr);
CVI_S32 CVI_ISP_SetCCMSaturationAttr(VI_PIPE ViPipe, const ISP_CCM_SATURATION_ATTR_S *pstCCMSaturationAttr);
CVI_S32 CVI_ISP_GetCCMSaturationAttr(VI_PIPE ViPipe, ISP_CCM_SATURATION_ATTR_S *pstCCMSaturationAttr);
CVI_S32 CVI_ISP_SetCCMAttr(VI_PIPE ViPipe, const ISP_CCM_ATTR_S *pstCCMAttr);
CVI_S32 CVI_ISP_GetCCMAttr(VI_PIPE ViPipe, ISP_CCM_ATTR_S *pstCCMAttr);

//-----------------------------------------------------------------------------
//  CSC
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetCSCAttr(VI_PIPE ViPipe, const ISP_CSC_ATTR_S *pstCSCAttr);
CVI_S32 CVI_ISP_GetCSCAttr(VI_PIPE ViPipe, ISP_CSC_ATTR_S *pstCSCAttr);

//-----------------------------------------------------------------------------
//  Color tone
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetColorToneAttr(VI_PIPE ViPipe,
				const ISP_COLOR_TONE_ATTR_S *pstColorToneAttr);
CVI_S32 CVI_ISP_GetColorToneAttr(VI_PIPE ViPipe, ISP_COLOR_TONE_ATTR_S *pstColorToneAttr);

//-----------------------------------------------------------------------------
//  FSHDR
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetFSHDRAttr(VI_PIPE ViPipe, const ISP_FSHDR_ATTR_S *pstFSHDRAttr);
CVI_S32 CVI_ISP_GetFSHDRAttr(VI_PIPE ViPipe, ISP_FSHDR_ATTR_S *pstFSHDRAttr);
CVI_S32 CVI_ISP_GetFSHDRMapCurveInfo(VI_PIPE ViPipe, ISP_FSHDR_MAP_CURVE_INFO_S *pstFSHDRMapCurveInfo);

//-----------------------------------------------------------------------------
//  DRC
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetDRCAttr(VI_PIPE ViPipe, const ISP_DRC_ATTR_S *pstDRCAttr);
CVI_S32 CVI_ISP_GetDRCAttr(VI_PIPE ViPipe, ISP_DRC_ATTR_S *pstDRCAttr);

//-----------------------------------------------------------------------------
//  PFR
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetPFRAttr(VI_PIPE ViPipe, const ISP_PFR_ATTR_S *pstPFRAttr);
CVI_S32 CVI_ISP_GetPFRAttr(VI_PIPE ViPipe, ISP_PFR_ATTR_S *pstPFRAttr);

//-----------------------------------------------------------------------------
//  RGB Gamma
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetGammaAttr(VI_PIPE ViPipe, const ISP_GAMMA_ATTR_S *pstGammaAttr);
CVI_S32 CVI_ISP_GetGammaAttr(VI_PIPE ViPipe, ISP_GAMMA_ATTR_S *pstGammaAttr);
CVI_S32 CVI_ISP_GetGammaCurveByType(VI_PIPE ViPipe, ISP_GAMMA_ATTR_S *pstGammaAttr,
				const ISP_GAMMA_CURVE_TYPE_E curveType);

//-----------------------------------------------------------------------------
//  CLUT
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetClutAttr(VI_PIPE ViPipe, const ISP_CLUT_ATTR_S *pstClutAttr);
CVI_S32 CVI_ISP_GetClutAttr(VI_PIPE ViPipe, ISP_CLUT_ATTR_S *pstClutAttr);
CVI_S32 CVI_ISP_SetClutHslAttr(VI_PIPE ViPipe, const ISP_CLUT_HSL_ATTR_S *pstClutHslAttr);
CVI_S32 CVI_ISP_GetClutHslAttr(VI_PIPE ViPipe, ISP_CLUT_HSL_ATTR_S *pstClutHslAttr);

//-----------------------------------------------------------------------------
//  LDCI
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetLDCIAttr(VI_PIPE ViPipe, const ISP_LDCI_ATTR_S *pstLDCIAttr);
CVI_S32 CVI_ISP_GetLDCIAttr(VI_PIPE ViPipe, ISP_LDCI_ATTR_S *pstLDCIAttr);

//-----------------------------------------------------------------------------
//  DCI
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetDCIAttr(VI_PIPE ViPipe, const ISP_DCI_ATTR_S *pstDCIAttr);
CVI_S32 CVI_ISP_GetDCIAttr(VI_PIPE ViPipe, ISP_DCI_ATTR_S *pstDCIAttr);
CVI_S32 CVI_ISP_SetDciAutoGammaAttr(VI_PIPE ViPipe, const ISP_DCI_AUTO_GAMMA_ATTR_S *pstGammaAttr);
CVI_S32 CVI_ISP_GetDciAutoGammaAttr(VI_PIPE ViPipe, ISP_DCI_AUTO_GAMMA_ATTR_S *pstGammaAttr);

//-----------------------------------------------------------------------------
//  CA (CA/CP)
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetCAAttr(VI_PIPE ViPipe, const ISP_CA_ATTR_S *pstCAAttr);
CVI_S32 CVI_ISP_GetCAAttr(VI_PIPE ViPipe, ISP_CA_ATTR_S *pstCAAttr);


//  Y Contrast
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetYContrastAttr(VI_PIPE ViPipe, const ISP_YCONTRAST_ATTR_S *pstYContrastAttr);
CVI_S32 CVI_ISP_GetYContrastAttr(VI_PIPE ViPipe, ISP_YCONTRAST_ATTR_S *pstYContrastAttr);

//-----------------------------------------------------------------------------
//  CA2
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetCA2Attr(VI_PIPE ViPipe, const ISP_CA2_ATTR_S *pstCA2Attr);
CVI_S32 CVI_ISP_GetCA2Attr(VI_PIPE ViPipe, ISP_CA2_ATTR_S *pstCA2Attr);

//-----------------------------------------------------------------------------
//  PreSharpen
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetPreSharpenAttr(VI_PIPE ViPipe, const ISP_PRESHARPEN_ATTR_S *pstPreSharpenAttr);
CVI_S32 CVI_ISP_GetPreSharpenAttr(VI_PIPE ViPipe, ISP_PRESHARPEN_ATTR_S *pstPreSharpenAttr);
CVI_S32 CVI_ISP_SetPreSharpenRefineAttr(VI_PIPE ViPipe, const ISP_PRESHARPEN_REFINE_ATTR_S *pstRefineAttr);
CVI_S32 CVI_ISP_GetPreSharpenRefineAttr(VI_PIPE ViPipe, ISP_PRESHARPEN_REFINE_ATTR_S *pstRefineAttr);
CVI_S32 CVI_ISP_SetPreSharpenEdgeExtAttr(VI_PIPE ViPipe, const ISP_PRESHARPEN_EDGE_EXT_ATTR_S *pstEdgeExtAttr);
CVI_S32 CVI_ISP_GetPreSharpenEdgeExtAttr(VI_PIPE ViPipe, ISP_PRESHARPEN_EDGE_EXT_ATTR_S *pstEdgeExtAttr);

//-----------------------------------------------------------------------------
//  Time-domain noise reduction (TNR)
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_GetTNRAttr(VI_PIPE ViPipe, ISP_TNR_ATTR_S *pstTNRAttr);
CVI_S32 CVI_ISP_GetTNRMvAttr(VI_PIPE ViPipe, ISP_TNR_MV_ATTR_S *pstTNRMVAttr);
CVI_S32 CVI_ISP_GetTNRPsAttr(VI_PIPE ViPipe, ISP_TNR_PS_ATTR_S *pstTNRPSAttr);
CVI_S32 CVI_ISP_GetTNRNrAttr(VI_PIPE ViPipe, ISP_TNR_NR_ATTR_S *pstTNRNRAttr);

CVI_S32 CVI_ISP_SetTNRAttr(VI_PIPE ViPipe, const ISP_TNR_ATTR_S *pstTNRAttr);
CVI_S32 CVI_ISP_SetTNRMvAttr(VI_PIPE ViPipe, const ISP_TNR_MV_ATTR_S *pstTNRMVAttr);
CVI_S32 CVI_ISP_SetTNRPsAttr(VI_PIPE ViPipe, const ISP_TNR_PS_ATTR_S *pstTNRPSAttr);
CVI_S32 CVI_ISP_SetTNRNrAttr(VI_PIPE ViPipe, const ISP_TNR_NR_ATTR_S *pstTNRNRAttr);

//-----------------------------------------------------------------------------
//  UV domain noise reduction (CNR)
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetCNRAttr(VI_PIPE ViPipe, const ISP_CNR_ATTR_S *pstCNRAttr);
CVI_S32 CVI_ISP_GetCNRAttr(VI_PIPE ViPipe, ISP_CNR_ATTR_S *pstCNRAttr);
CVI_S32 CVI_ISP_SetCNRFilterAttr(VI_PIPE ViPipe, const ISP_CNR_FILTER_ATTR_S *pstCNRFilterAttr);
CVI_S32 CVI_ISP_GetCNRFilterAttr(VI_PIPE ViPipe, ISP_CNR_FILTER_ATTR_S *pstCNRFilterAttr);

//-----------------------------------------------------------------------------
//  Sharpen
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetSharpenAttr(VI_PIPE ViPipe, const ISP_SHARPEN_ATTR_S *pstSharpenAttr);
CVI_S32 CVI_ISP_GetSharpenAttr(VI_PIPE ViPipe, ISP_SHARPEN_ATTR_S *pstSharpenAttr);

//-----------------------------------------------------------------------------
//  Mono
//-----------------------------------------------------------------------------
CVI_S32 CVI_ISP_SetMonoAttr(VI_PIPE ViPipe, const ISP_MONO_ATTR_S *pstMonoAttr);
CVI_S32 CVI_ISP_GetMonoAttr(VI_PIPE ViPipe, ISP_MONO_ATTR_S *pstMonoAttr);

//-----------------------------------------------------------------------------
//  TEAISP
//-----------------------------------------------------------------------------
CVI_S32 CVI_TEAISP_Init(VI_PIPE ViPipe, CVI_S32 maxDev);

//-----------------------------------------------------------------------------
//  TEAISP BNR
//-----------------------------------------------------------------------------
CVI_S32 CVI_TEAISP_BNR_SetModel(VI_PIPE ViPipe, const TEAISP_BNR_MODEL_INFO_S *pstModelInfo);
CVI_S32 CVI_TEAISP_BNR_SetAttr(VI_PIPE ViPipe, const TEAISP_BNR_ATTR_S *pstTEAISPBNRAttr);
CVI_S32 CVI_TEAISP_BNR_GetAttr(VI_PIPE ViPipe, TEAISP_BNR_ATTR_S *pstTEAISPBNRAttr);
CVI_S32 CVI_TEAISP_BNR_SetNoiseProfileAttr(VI_PIPE ViPipe, const TEAISP_BNR_NP_S *np);
CVI_S32 CVI_TEAISP_BNR_GetNoiseProfileAttr(VI_PIPE ViPipe, TEAISP_BNR_NP_S *np);


CVI_S32 CVI_ISP_SetStatisticsConfig(VI_PIPE ViPipe, const ISP_STATISTICS_CFG_S *pstStatCfg);
CVI_S32 CVI_ISP_GetStatisticsConfig(VI_PIPE ViPipe, ISP_STATISTICS_CFG_S *pstStatCfg);
CVI_S32 CVI_ISP_GetAEStatistics(VI_PIPE ViPipe, ISP_AE_STATISTICS_S *pstAeStat);
CVI_S32 CVI_ISP_GetWBStatistics(VI_PIPE ViPipe, ISP_WB_STATISTICS_S *pstWBStat);
CVI_S32 CVI_ISP_GetFocusStatistics(VI_PIPE ViPipe, ISP_AF_STATISTICS_S *pstAfStat);
CVI_S32 CVI_ISP_GetLightboxGain(VI_PIPE ViPipe, ISP_AWB_LightBox_Gain_S *pstAWBLightBoxGain);

CVI_S32 CVI_ISP_SetNoiseProfileAttr(VI_PIPE ViPipe, const ISP_CMOS_NOISE_CALIBRATION_S *pstNoiseProfileAttr);
CVI_S32 CVI_ISP_GetNoiseProfileAttr(VI_PIPE ViPipe, ISP_CMOS_NOISE_CALIBRATION_S *pstNoiseProfileAttr);

CVI_S32 CVI_ISP_QueryInnerStateInfo(VI_PIPE ViPipe, ISP_INNER_STATE_INFO_S *pstInnerStateInfo);
CVI_S32 CVI_ISP_GetVDTimeOut(VI_PIPE ViPipe, ISP_VD_TYPE_E enIspVDType, CVI_U32 u32MilliSec);
//CVI_S32 CVI_ISP_GetAEStitchStatistics(VI_PIPE ViPipe, ISP_AE_STITCH_STATISTICS_S *pstStitchStat);
//CVI_S32 CVI_ISP_GetMGStatistics(VI_PIPE ViPipe, ISP_MG_STATISTICS_S *pstMgStat);
//CVI_S32 CVI_ISP_GetWBStitchStatistics(VI_PIPE ViPipe, ISP_WB_STITCH_STATISTICS_S *pstStitchWBStat);

CVI_S32 CVI_ISP_DumpHwRegisterToFile(VI_PIPE ViPipe, FILE *fp);
CVI_S32 CVI_ISP_DumpFrameRawInfoToFile(VI_PIPE ViPipe, FILE *fp);

CVI_S32 CVI_ISP_IrAutoRunOnce(ISP_DEV IspDev, ISP_IR_AUTO_ATTR_S *pstIrAttr);

CVI_S32 CVI_ISP_SetSmartInfo(VI_PIPE ViPipe, const ISP_SMART_INFO_S *pstSmartInfo, CVI_U8 TimeOut);
CVI_S32 CVI_ISP_GetSmartInfo(VI_PIPE ViPipe, ISP_SMART_INFO_S *pstSmartInfo);

CVI_S32 CVI_ISP_SetStitchAttr(VI_PIPE ViPipe, ISP_STITCH_ATTR_S *pstStitchAttr);
CVI_S32 CVI_ISP_GetStitchAttr(VI_PIPE ViPipe, ISP_STITCH_ATTR_S *pstStitchAttr);
CVI_S32 CVI_ISP_StitchCalibartion(VI_PIPE ViPipe, ISP_STITCH_ATTR_S *pstAttr, CVI_U8 chnNum);

CVI_S32 CVI_ISP_Register(ISP_FUNC_FROM_OUT_S stFunction);
CVI_S32 CVI_ISP_MediaVideoInit(CVI_BOOL IsRawReplayMode);
CVI_S32 CVI_ISP_MediaVideoDeinit(CVI_BOOL IsRawReplayMode);
CVI_S32 CVI_ISP_GetIonInfo(CVI_S32 *ptotal, CVI_S32 *pused, CVI_S32 *pmfree, CVI_S32 *ppeak);

CVI_S32 CVI_ISP_SetBypassFrm(VI_PIPE ViPipe, CVI_U8 bypassNum);
CVI_S32 CVI_ISP_GetBypassFrm(VI_PIPE ViPipe, CVI_U8 *pbypassNum);

CVI_S32 CVI_ISP_GetIspRunState(VI_PIPE ViPipe, CVI_BOOL *bIspRun);

CVI_S32 CVI_ISP_SetModuleEnAttr(VI_PIPE ViPipe, const ISP_ModuleEn_ATTR_S *pstModuleEnAttr);
CVI_S32 CVI_ISP_GetModuleEnAttr(VI_PIPE ViPipe, ISP_ModuleEn_ATTR_S *pstModuleEnAttr);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*__CVI_ISP_H__ */

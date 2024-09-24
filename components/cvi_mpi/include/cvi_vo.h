/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_vo.h
 * Description:
 *   MMF Programe Interface for video output management moudle
 */

#ifndef __CVI_VO_H__
#define __CVI_VO_H__

#include <cvi_comm_vo.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/* Device Relative Settings */

/* CVI_VO_SetPubAttr: Configure VO public properties
 *
 * @param VoDev: Video output device
 * @param pstPubAttr: public properties
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetPubAttr(VO_DEV VoDev, const VO_PUB_ATTR_S *pstPubAttr);

/* CVI_VO_GetPubAttr: get VO public properties
 *
 * @param VoDev: Video output device
 * @param pstPubAttr: public properties
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetPubAttr(VO_DEV VoDev, VO_PUB_ATTR_S *pstPubAttr);

/* CVI_VO_IsEnabled: Check if VO is enabled
 *
 * @param VoDev: Video output device
 * @return: Is it enabled.
 */
CVI_BOOL CVI_VO_IsEnabled(VO_DEV VoDev);

/* CVI_VO_Enable: enable vo
 *
 * @param VoDev: Video output device
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_Enable(VO_DEV VoDev);

/* CVI_VO_Disable: disable vo
 *
 * @param VoDev: Video output device
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_Disable(VO_DEV VoDev);

/* CVI_VO_ShowPattern: show internal test patterns
 *
 * @param VoDev: Video output device
 * @param PatternId: pattern id
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_ShowPattern(VO_DEV VoDev, enum VO_PATTERN_MODE PatternId);

/* CVI_VO_CloseFd: close vo fd
 *
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_CloseFd(void);

/* Video Relative Settings */

/* CVI_VO_SetVideoLayerAttr: Set video layer properties
 *
 * @param VoLayer: Video layer
 * @param pstLayerAttr: layer properties
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetVideoLayerAttr(VO_LAYER VoLayer, const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr);

/* CVI_VO_SetVideoLayerAttrEx: Set video layer properties(for alios bootlogo)
 *
 * @param VoLayer: Video layer
 * @param pstLayerAttr: layer properties
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetVideoLayerAttrEx(VO_LAYER VoLayer, const VO_VIDEO_LAYER_ATTR_S *pstLayerAttr);

/* CVI_VO_GetVideoLayerAttr: Get video layer properties
 *
 * @param VoLayer: Video layer
 * @param pstLayerAttr: layer properties
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetVideoLayerAttr(VO_LAYER VoLayer, VO_VIDEO_LAYER_ATTR_S *pstLayerAttr);

/* CVI_VO_EnableVideoLayer: enable video layer
 *
 * @param VoLayer: Video layer
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_EnableVideoLayer(VO_LAYER VoLayer);

/* CVI_VO_DisableVideoLayer: disable video layer
 *
 * @param VoLayer: Video layer
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_DisableVideoLayer(VO_LAYER VoLayer);

/* CVI_VO_GetLayerProcAmpCtrl: get layer proc ctrl parameters
 *
 * @param VoLayer: Video layer
 * @param type: Brightness, hue, saturation, contrast
 * @param ctrl: Control parameters
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetLayerProcAmpCtrl(VO_LAYER VoLayer, PROC_AMP_E type, PROC_AMP_CTRL_S *ctrl);

/* CVI_VO_GetLayerProcAmp: get layer proc value
 *
 * @param VoLayer: Video layer
 * @param type: Brightness, hue, saturation, contrast
 * @param value: value
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetLayerProcAmp(VO_LAYER VoLayer, PROC_AMP_E type, CVI_S32 *value);

/* CVI_VO_SetLayerProcAmp: set layer proc value
 *
 * @param VoLayer: Video layer
 * @param type: Brightness, hue, saturation, contrast
 * @param value: value
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetLayerProcAmp(VO_LAYER VoLayer, PROC_AMP_E type, CVI_S32 value);

/* Display relative operations */

/* CVI_VO_SetDisplayBufLen: set layer display buf depth
 *
 * @param VoLayer: Video layer
 * @param u32BufLen: How many VBs
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetDisplayBufLen(VO_LAYER VoLayer, CVI_U32 u32BufLen);

/* CVI_VO_GetDisplayBufLen: set layer display buf depth
 *
 * @param VoLayer: Video layer
 * @param u32BufLen: How many VBs
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetDisplayBufLen(VO_LAYER VoLayer, CVI_U32 *pu32BufLen);

/* Channel Relative Operations */

/* CVI_VO_SetChnAttr: set chn attr
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param pstChnAttr: chn attr
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetChnAttr(VO_LAYER VoLayer, VO_CHN VoChn, const VO_CHN_ATTR_S *pstChnAttr);

/* CVI_VO_GetChnAttr: get chn attr
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param pstChnAttr: chn attr
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetChnAttr(VO_LAYER VoLayer, VO_CHN VoChn, VO_CHN_ATTR_S *pstChnAttr);

/* CVI_VO_EnableChn: enable chn
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_EnableChn(VO_LAYER VoLayer, VO_CHN VoChn);

/* CVI_VO_DisableChn: disable chn
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_DisableChn(VO_LAYER VoLayer, VO_CHN VoChn);

/* CVI_VO_PauseChn: pause chn
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_PauseChn(VO_LAYER VoLayer, VO_CHN VoChn);

/* CVI_VO_PauseChn: resume chn
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_ResumeChn(VO_LAYER VoLayer, VO_CHN VoChn);

/* CVI_VO_PauseChn: show chn
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_ShowChn(VO_LAYER VoLayer, VO_CHN VoChn);

/* CVI_VO_PauseChn: hide chn
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_HideChn(VO_LAYER VoLayer, VO_CHN VoChn);

/* CVI_VO_SendFrame: send to chn frame
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param pstVideoFrame: frame info
 * @param s32MilliSec: timeout
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SendFrame(VO_LAYER VoLayer, VO_CHN VoChn, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec);

/* CVI_VO_SendLogoFromIon: send vdec bootlogo frame to vo
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param pstVideoFrame: frame info
 * @param s32MilliSec: timeout
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SendLogoFromIon(VO_LAYER VoLayer, VO_CHN VoChn, VIDEO_FRAME_INFO_S *pstVideoFrame, CVI_S32 s32MilliSec);

/* CVI_VO_ClearChnBuf: clear chn buffer
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param bClrAll: Do you want to clear all
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_ClearChnBuf(VO_LAYER VoLayer, VO_CHN VoChn, CVI_BOOL bClrAll);

/* CVI_VO_SetChnRotation: set chn Rotation type
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param enRotation: Rotation type
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetChnRotation(VO_LAYER VoLayer, VO_CHN VoChn, ROTATION_E enRotation);

/* CVI_VO_GetChnRotation: get chn Rotation type
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param penRotation: Rotation type
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetChnRotation(VO_LAYER VoLayer, VO_CHN VoChn, ROTATION_E *penRotation);

/* Module Parameter Settings */

/* CVI_VO_Get_Panel_Status: get the status of panel
 *
 * @param VoLayer: Video layer
 * @param VoChn: Video chn
 * @param is_init: Is the panel initialized
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_Get_Panel_Status(VO_LAYER VoLayer, VO_CHN VoChn, CVI_U32 *is_init);

/* CVI_VO_RegPmCallBack: Register power management callback function
 *
 * @param VoDev: Video output device
 * @param pstPmOps: Power management function register
 * @param pvData: Private data
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_RegPmCallBack(VO_DEV VoDev, VO_PM_OPS_S *pstPmOps, void *pvData);

/* CVI_VO_UnRegPmCallBack: Cancel power management callback function
 *
 * @param VoDev: Video output device
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_UnRegPmCallBack(VO_DEV VoDev);

/* CVI_VO_SetGammaInfo: set gamma attr
 *
 * @param VO_GAMMA_INFO_S: gamma attr
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_SetGammaInfo(VO_GAMMA_INFO_S *pinfo);

/* CVI_VO_GetGammaInfo: get gamma attr
 *
 * @param VO_GAMMA_INFO_S: gamma attr
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_GetGammaInfo(VO_GAMMA_INFO_S *pinfo);

/* CVI_VO_Suspend: Suspend vo
 *
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_Suspend(void);

/* CVI_VO_Resume: Resume vo
 *
 * @return: status of operation. CVI_SUCCESS if OK.
 */
CVI_S32 CVI_VO_Resume(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif // __CVI_VO_H__

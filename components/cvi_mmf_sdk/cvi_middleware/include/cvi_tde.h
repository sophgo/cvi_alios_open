/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_TDE.h
 * Description:
 *   TDE interfaces.
 */

#ifndef __CVI_TDE_H__
#define __CVI_TDE_H__

#include <cvi_common.h>
#include <cvi_comm_tde.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* API Declaration */

/**
 * @brief Open tde device.
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_TDE_Open(CVI_VOID);

/**
 * @brief Close tde device.
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_VOID CVI_TDE_Close(CVI_VOID);

/**
 * @brief Begin a TDE job.
 *
 * @return TDE_HANDLE Return.
 */
TDE_HANDLE CVI_TDE_BeginJob(CVI_VOID);

/**
 * @brief End a TDE job.
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_TDE_EndJob(TDE_HANDLE s32Handle, CVI_BOOL bSync, CVI_BOOL bBlock, CVI_U32 u32TimeOut);

/**
 * @brief wait for all the TDE jobs to complete
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_TDE_WaitAllDone(CVI_VOID);

/**
 * @brief Cancel a TDE job.
 *
 * @return CVI_S32 Return CVI_SUCCESS if succeed.
 */
CVI_S32 CVI_TDE_CancelJob(TDE_HANDLE s32Handle);

/* Add a rotation task to a TDE job.
 *
 * @param s32Handle(In): TDE_HANDLE handle
 * @param pstSrc(In): to describe what to do
 * @param enRotateAngle(In): for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_TDE_Rotate(TDE_HANDLE s32Handle, TDE_SURFACE_S *pstSrc,
	TDE_SURFACE_S *pstDst, TDE_ROTATE_ANGLE_E enRotateAngle);

/* Add a drawline task to a TDE job.
 *
 * @param s32Handle(In): TDE_HANDLE handle
 * @param pstSrc(In): to describe what to do
 * @param pstLine(In): for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_TDE_DrawLine(TDE_HANDLE s32Handle, TDE_SURFACE_S *pstSrc,
	TDE_SURFACE_S *pstDst, TDE_LINE_S *pstLine);

/* Add a quick copy task to a TDE job.
 *
 * @param handle(In): TDE_HANDLE handle
 * @param pstTask(In): to describe what to do
 * @param pstAffineAttr(In): for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_TDE_QuickCopy(TDE_HANDLE s32Handle, TDE_SURFACE_S *pstSrc, TDE_SURFACE_S *pstDst);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __CVI_TDE_H__ */

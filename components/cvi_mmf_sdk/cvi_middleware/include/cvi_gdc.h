/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_gdc.h
 * Description:
 *   gdc interfaces.
 */

#ifndef __CVI_GDC_H__
#define __CVI_GDC_H__

#include <cvi_common.h>
#include <cvi_comm_video.h>
#include <cvi_comm_gdc.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/* init gdc moudle, include dev and gdc software subsystem.
 *
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_Init(void);

/* deinit gdc moudle, include dev and gdc software subsystem.
 *
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_DeInit(void);

/* Begin a gdc job,then add task into the job,gdc will finish all the task in the job.
 *
 * @param phHandle(Out): GDC_HANDLE *phHandle
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_BeginJob(GDC_HANDLE *phHandle);

/* End a job,all tasks in the job will be submmitted to gdc.
 *
 * @param hHandle(In): GDC_HANDLE hHandle
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_EndJob(GDC_HANDLE hHandle);

/* Cancel a job ,then all tasks in the job will not be submmitted to gdc.
 *
 * @param hHandle(In): GDC_HANDLE hHandle
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_CancelJob(GDC_HANDLE hHandle);

/* set identity attribution for a job ,identity attribution is unique id for a job.
 *
 * @param hHandle(In): GDC_HANDLE hHandle
 * @param identity_attr(In): GDC_IDENTITY_ATTR_S *identity_attr
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_SetJobIdentity(GDC_HANDLE hHandle, GDC_IDENTITY_ATTR_S *identity_attr);

/* Add a rotation task to a gdc job.
 *
 * @param hHandle(In): GDC_HANDLE hHandle
 * @param pstTask(In): to describe what to do
 * @param enRotation(In): for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_AddRotationTask(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask, ROTATION_E enRotation);

/* Add a ldc task to a gdc job.
 *
 * @param hHandle(In): GDC_HANDLE hHandle
 * @param pstTask(In): to describe what to do
 * @param pstLDCAttr(In): for further settings
 * @param enRotation(In): for further settings
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_AddLDCTask(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask,
	const LDC_ATTR_S *pstLDCAttr, ROTATION_E enRotation);

/* dump mesh data for gdc task, mesh specific that
 * coordinate mapping relationship between the original image and the target image.
 *
 * @param pMeshDumpAttr(In): MESH_DUMP_ATTR_S *pMeshDumpAttr
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_DumpMesh(MESH_DUMP_ATTR_S *pMeshDumpAttr);

/* load mesh data for gdc task, mesh specific that
 * coordinate mapping relationship between the original image and the target image.
 *
 * @param pMeshDumpAttr(In): MESH_DUMP_ATTR_S *pMeshDumpAttr
 * @param LDC_ATTR_S(In): LDC_ATTR_S *pstLDCAttr
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_LoadMesh(MESH_DUMP_ATTR_S *pMeshDumpAttr, const LDC_ATTR_S *pstLDCAttr);

/**
 * @brief load mesh buf for GDC, mesh data is load from user
 *
 * @param pMeshDumpAttr(In), mesh dump attribute
 * @param pstLDCAttr(In), LDC_ATTR_S
 * @param pBuf(In), mesh buf
 * @param Len(In), mesh buf len
 * @return CVI_S32 Return CVI_SUCCESS if succeed
 */
CVI_S32 CVI_GDC_LoadMeshWithBuf(MESH_DUMP_ATTR_S *pMeshDumpAttr,
		const LDC_ATTR_S *pstLDCAttr, CVI_VOID *pBuf, CVI_U32 Len);

/* get the job currently being used by gdc dev.
 *
 * @param phHandle(Out): GDC_HANDLE* phHandle
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_GetWorkJob(GDC_HANDLE *phHandle);

/* get frame for specific gdc job, It only works with async io.
 *
 * @param identity(In): GDC_IDENTITY_ATTR_S *identity
 * @param pstFrameInfo(Out): VIDEO_FRAME_INFO_S *pstFrameInfo
 * @param s32MilliSec(In): timeout
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_GetChnFrame(GDC_IDENTITY_ATTR_S *identity, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec);

/* Suspend gdc dev and gdc software subsystem.
 *
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_Suspend(void);

/* Resume gdc dev and gdc software subsystem.
 *
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_Resume(void);

/* attach vb pool for specific gdc callback, It only works with internal mode.
 *
 * @param pChn(In): chn which to call gdc
 * @param u32VbPool(In): vb pool to attach
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_AttachVbPool(MMF_CHN_S *pChn, VB_POOL u32VbPool);

/* detach vb pool for specific gdc callback, It only works with internal mode.
 *
 * @param pChn(In): chn which to call gdc
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_GDC_DetachVbPool(MMF_CHN_S *pChn);

CVI_S32 CVI_GDC_GenLDCMesh(CVI_U32 u32Width, CVI_U32 u32Height, const LDC_ATTR_S *pstLDCAttr,
			const char *name, CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr);

CVI_S32 CVI_GDC_LoadLDCMesh(CVI_U32 u32Width, CVI_U32 u32Height, const char *fileNname
		, const char *tskName, CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr);

CVI_VOID CVI_GDC_FreeCurTaskMesh(CVI_CHAR *tskName);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __CVI_GDC_H__ */

#include "platform_gdc.h"

CVI_S32 CVI_GDC_Suspend(void)
{
	return platform_gdc_suspend();
}

CVI_S32 CVI_GDC_Resume(void)
{
	return platform_gdc_resume();
}

CVI_S32 CVI_GDC_Init(void)
{
	return platform_gdc_init();
}

CVI_S32 CVI_GDC_DeInit(void)
{
	return platform_gdc_deinit();
}

CVI_S32 CVI_GDC_BeginJob(GDC_HANDLE *phHandle)
{
	return platform_gdc_beginjob(phHandle);
}

CVI_S32 CVI_GDC_SetJobIdentity(GDC_HANDLE hHandle, GDC_IDENTITY_ATTR_S *identity_attr)
{
	return platform_gdc_setjobidentity(hHandle, identity_attr);
}

CVI_S32 CVI_GDC_EndJob(GDC_HANDLE hHandle)
{
	return platform_gdc_endjob(hHandle);
}

CVI_S32 CVI_GDC_CancelJob(GDC_HANDLE hHandle)
{
	return platform_gdc_canceljob(hHandle);
}

CVI_S32 CVI_GDC_AddRotationTask(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask, ROTATION_E enRotation)
{
	return platform_gdc_addrotationtask(hHandle, pstTask, enRotation);
}

CVI_S32 CVI_GDC_AddLDCTask(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask
	, const LDC_ATTR_S *pstLDCAttr, ROTATION_E enRotation)
{
	return platform_gdc_addldctask(hHandle, pstTask, pstLDCAttr, enRotation);
}

CVI_S32 CVI_GDC_DumpMesh(MESH_DUMP_ATTR_S *pMeshDumpAttr)
{
	return platform_gdc_dumpmesh(pMeshDumpAttr);
}

CVI_S32 CVI_GDC_LoadMesh(MESH_DUMP_ATTR_S *pMeshDumpAttr, const LDC_ATTR_S *pstLDCAttr)
{
	return platform_gdc_loadmesh(pMeshDumpAttr, pstLDCAttr);
}

CVI_S32 CVI_GDC_GetWorkJob(GDC_HANDLE *phHandle)
{
	return platform_gdc_getworkjob(phHandle);
}

CVI_S32 CVI_GDC_GetChnFrame(GDC_IDENTITY_ATTR_S *identity, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec)
{
	return platform_gdc_getchnframe(identity, pstFrameInfo, s32MilliSec);
}

CVI_S32 CVI_GDC_GenLDCMesh(CVI_U32 u32Width, CVI_U32 u32Height, const LDC_ATTR_S *pstLDCAttr,
		const char *name, CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr)
{
	return platform_gdc_genldcmesh(u32Width, u32Height, pstLDCAttr, name, pu64PhyAddr, ppVirAddr);
}

CVI_S32 CVI_GDC_LoadLDCMesh(CVI_U32 u32Width, CVI_U32 u32Height, const char *fileNname
	, const char *tskName, CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr)
{
	return platform_gdc_loadldcmesh(u32Width, u32Height, fileNname, tskName, pu64PhyAddr, ppVirAddr);
}

CVI_VOID CVI_GDC_FreeCurTaskMesh(CVI_CHAR *tskName)
{
	return platform_gdc_freecurtaskmesh(tskName);
}

CVI_S32 CVI_GDC_AttachVbPool(MMF_CHN_S *pChn, VB_POOL u32VbPool)
{
	return platform_gdc_attachvbpool(pChn, u32VbPool);
}

CVI_S32 CVI_GDC_DetachVbPool(MMF_CHN_S *pChn)
{
	return platform_gdc_detachvbpool(pChn);
}

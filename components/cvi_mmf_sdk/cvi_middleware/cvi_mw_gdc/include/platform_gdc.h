#ifndef __PLATFORM_GDC_H__
#define __PLATFORM_GDC_H__

#include "cvi_comm_gdc.h"

CVI_S32 platform_gdc_suspend(void);

CVI_S32 platform_gdc_resume(void);

CVI_S32 platform_gdc_init(void);

CVI_S32 platform_gdc_deinit(void);

CVI_S32 platform_gdc_beginjob(GDC_HANDLE *phHandle);

CVI_S32 platform_gdc_setjobidentity(GDC_HANDLE hHandle, GDC_IDENTITY_ATTR_S *identity_attr);

CVI_S32 platform_gdc_endjob(GDC_HANDLE hHandle);

CVI_S32 platform_gdc_canceljob(GDC_HANDLE hHandle);

CVI_S32 platform_gdc_addrotationtask(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask, ROTATION_E enRotation);

CVI_S32 platform_gdc_addldctask(GDC_HANDLE hHandle, const GDC_TASK_ATTR_S *pstTask
	, const LDC_ATTR_S *pstLDCAttr, ROTATION_E enRotation);

CVI_S32 platform_gdc_dumpmesh(MESH_DUMP_ATTR_S *pMeshDumpAttr);

CVI_S32 platform_gdc_loadmesh(MESH_DUMP_ATTR_S *pMeshDumpAttr, const LDC_ATTR_S *pstLDCAttr);

CVI_S32 platform_gdc_getworkjob(GDC_HANDLE *phHandle);

CVI_S32 platform_gdc_getchnframe(GDC_IDENTITY_ATTR_S *identity, VIDEO_FRAME_INFO_S *pstFrameInfo, CVI_S32 s32MilliSec);

CVI_S32 platform_gdc_genldcmesh(CVI_U32 u32Width, CVI_U32 u32Height, const LDC_ATTR_S *pstLDCAttr,
		const char *name, CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr);

CVI_S32 platform_gdc_loadldcmesh(CVI_U32 u32Width, CVI_U32 u32Height, const char *fileNname
	, const char *tskName, CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr);

CVI_VOID platform_gdc_freecurtaskmesh(CVI_CHAR *tskName);

CVI_S32 platform_gdc_attachvbpool(MMF_CHN_S *pChn, VB_POOL u32VbPool);

CVI_S32 platform_gdc_detachvbpool(MMF_CHN_S *pChn);

#endif
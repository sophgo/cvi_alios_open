/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_comm_gdc.h
 * Description:
 *   Common gdc definitions.
 */

#ifndef __CVI_COMM_GDC_H__
#define __CVI_COMM_GDC_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include <cvi_type.h>
#include <cvi_common.h>
#include <cvi_comm_video.h>
#include <cvi_comm_vb.h>

#define FISHEYE_MAX_REGION_NUM 4
#define AFFINE_MAX_REGION_NUM 32

typedef CVI_S64 GDC_HANDLE;

/*
 * stImgIn: Input picture
 * stImgOut: Output picture
 * au64privateData[4]: RW; Private data of task
 * reserved: RW; Debug information,state of current picture
 */
typedef struct _GDC_TASK_ATTR_S {
	VIDEO_FRAME_INFO_S stImgIn;
	VIDEO_FRAME_INFO_S stImgOut;
	CVI_U64 au64privateData[4];
	CVI_U64 reserved;
	CVI_CHAR name[32];
} GDC_TASK_ATTR_S;

typedef struct _POINT2F_S {
	CVI_FLOAT x;
	CVI_FLOAT y;
} POINT2F_S;
typedef struct _VI_MESH_ATTR_S {
	VI_PIPE pipe;
	VI_CHN chn;
} VI_MESH_ATTR_S;

typedef struct _VPSS_MESH_ATTR_S {
	VPSS_GRP grp;
	VPSS_CHN chn;
} VPSS_MESH_ATTR_S;

typedef struct _MESH_DUMP_ATTR_S {
	CVI_CHAR binFileName[128];
	MOD_ID_E enModId;
	union {
		VI_MESH_ATTR_S viMeshAttr;
		VPSS_MESH_ATTR_S vpssMeshAttr;
	};
} MESH_DUMP_ATTR_S;

typedef struct _GDC_IDENTITY_ATTR_S {
	CVI_CHAR Name[32];
	MOD_ID_E enModId;
	CVI_U32 u32ID;
	CVI_BOOL syncIo;
} GDC_IDENTITY_ATTR_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __CVI_COMM_GDC_H__ */

/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_comm_TDE.h
 * Description:
 *   Common TDE definitions.
 */

#ifndef __CVI_COMM_TDE_H__
#define __CVI_COMM_TDE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include <cvi_type.h>
#include <cvi_common.h>
#include <cvi_comm_video.h>

#define TDE_INVALID_HANDLE (-1)
#define TDE_ALIGN (16)


typedef CVI_S32 TDE_HANDLE;


typedef struct _TDE_SURFACE_S {
	CVI_U64 u64PhyAddr;/* Header address of a bitmap or the Y component */
	CVI_U32 u32Height;/* Bitmap height */
	CVI_U32 u32Width;/* Bitmap width */
	CVI_U32 u32Stride;/* Stride of a bitmap */
	PIXEL_FORMAT_E enColorFmt;
} TDE_SURFACE_S;

typedef enum _TDE_ROTATE_ANGLE_E {
	TDE_ROTATE_NONE = 0,/* No ratate */
	TDE_ROTATE_90,/* ratate 90 */
	TDE_ROTATE_270,/* ratate 270 */
	TDE_ROTATE_MAX
} TDE_ROTATE_ANGLE_E;

typedef struct _TDE_LINE_S {
	CVI_S32 s32StartX;
	CVI_S32 s32StartY;
	CVI_S32 s32EndX;
	CVI_S32 s32EndY;
	CVI_U32 u32Thick;
	CVI_U32 u32Color;
} TDE_LINE_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __CVI_COMM_TDE_H__ */

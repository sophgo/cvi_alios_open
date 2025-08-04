#ifndef __PLATFORM_REGION_H__
#define __PLATFORM_REGION_H__

#include "cvi_comm_region.h"

CVI_S32 platform_rgn_create(RGN_HANDLE Handle, const RGN_ATTR_S *pstRegion);

CVI_S32 platform_rgn_destroy(RGN_HANDLE Handle);

CVI_S32 platform_rgn_getattr(RGN_HANDLE Handle, RGN_ATTR_S *pstRegion);

CVI_S32 platform_rgn_setattr(RGN_HANDLE Handle, const RGN_ATTR_S *pstRegion);

CVI_S32 platform_rgn_setbitmap(RGN_HANDLE Handle, const BITMAP_S *pstBitmap);

CVI_S32 platform_rgn_attachtochn(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, const RGN_CHN_ATTR_S *pstChnAttr);

CVI_S32 platform_rgn_detachfromchn(RGN_HANDLE Handle, const MMF_CHN_S *pstChn);

CVI_S32 platform_rgn_setdisplayattr(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, const RGN_CHN_ATTR_S *pstChnAttr);

CVI_S32 platform_rgn_getdisplayattr(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, RGN_CHN_ATTR_S *pstChnAttr);

CVI_S32 platform_rgn_getcanvasinfo(RGN_HANDLE Handle, RGN_CANVAS_INFO_S *pstCanvasInfo);

CVI_S32 platform_rgn_updatecanvas(RGN_HANDLE Handle);

CVI_S32 platform_rgn_invertcolor(RGN_HANDLE Handle, MMF_CHN_S *pstChn, CVI_U32 *pu32Color);

CVI_S32 platform_rgn_setchnpalette(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, RGN_PALETTE_S *pstPalette);

CVI_S32 platform_rgn_setcmpr_size(RGN_HANDLE Handle, CVI_U32 *bs_size);

#endif
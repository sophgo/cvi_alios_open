#include "platform_region.h"

CVI_S32 CVI_RGN_Create(RGN_HANDLE Handle, const RGN_ATTR_S *pstRegion)
{
	return platform_rgn_create(Handle, pstRegion);
}

CVI_S32 CVI_RGN_Destroy(RGN_HANDLE Handle)
{
	return platform_rgn_destroy(Handle);
}

CVI_S32 CVI_RGN_GetAttr(RGN_HANDLE Handle, RGN_ATTR_S *pstRegion)
{
	return platform_rgn_getattr(Handle, pstRegion);
}

CVI_S32 CVI_RGN_SetAttr(RGN_HANDLE Handle, const RGN_ATTR_S *pstRegion)
{
	return platform_rgn_setattr(Handle, pstRegion);
}

CVI_S32 CVI_RGN_SetBitMap(RGN_HANDLE Handle, const BITMAP_S *pstBitmap)
{
	return platform_rgn_setbitmap(Handle, pstBitmap);
}

CVI_S32 CVI_RGN_AttachToChn(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, const RGN_CHN_ATTR_S *pstChnAttr)
{
	return platform_rgn_attachtochn(Handle, pstChn, pstChnAttr);
}

CVI_S32 CVI_RGN_DetachFromChn(RGN_HANDLE Handle, const MMF_CHN_S *pstChn)
{
	return platform_rgn_detachfromchn(Handle, pstChn);
}

CVI_S32 CVI_RGN_SetDisplayAttr(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, const RGN_CHN_ATTR_S *pstChnAttr)
{
	return platform_rgn_setdisplayattr(Handle, pstChn, pstChnAttr);
}

CVI_S32 CVI_RGN_GetDisplayAttr(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, RGN_CHN_ATTR_S *pstChnAttr)
{
	return platform_rgn_getdisplayattr(Handle, pstChn, pstChnAttr);
}

CVI_S32 CVI_RGN_GetCanvasInfo(RGN_HANDLE Handle, RGN_CANVAS_INFO_S *pstCanvasInfo)
{
	return platform_rgn_getcanvasinfo(Handle, pstCanvasInfo);
}

CVI_S32 CVI_RGN_UpdateCanvas(RGN_HANDLE Handle)
{
	return platform_rgn_updatecanvas(Handle);
}

CVI_S32 CVI_RGN_Invert_Color(RGN_HANDLE Handle, MMF_CHN_S *pstChn, CVI_U32 *pu32Color)
{
	return platform_rgn_invertcolor(Handle, pstChn, pu32Color);
}

CVI_S32 CVI_RGN_SetChnPalette(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, RGN_PALETTE_S *pstPalette)
{
	return platform_rgn_setchnpalette(Handle, pstChn, pstPalette);
}

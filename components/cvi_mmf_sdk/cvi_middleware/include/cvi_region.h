/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * Description:
 *   region Interface
 */

#ifndef __CVI_REGION_H__
#define __CVI_REGION_H__

#include <cvi_comm_region.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* Create a region.
 *
 * @param Handle(In): region handle
 * @param pstRegion(In): region attribute pointer
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_RGN_Create(RGN_HANDLE Handle, const RGN_ATTR_S * pstRegion);

/* Destroy a region.
 *
 * @param Handle(In): region handle
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_RGN_Destroy(RGN_HANDLE Handle);

/* Get region attribute.
 *
 * @param Handle(In): region handle
 * @param pstRegion(Out): region attribute pointer
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_RGN_GetAttr(RGN_HANDLE Handle, RGN_ATTR_S *pstRegion);

/* Set region attribute.
 *
 * @param Handle(In): region handle
 * @param pstRegion(In): region attribute pointer
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_RGN_SetAttr(RGN_HANDLE Handle, const RGN_ATTR_S *pstRegion);

/* Set the region bitmap and fill the bitmap into the region.
 *
 * @param Handle(In): region handle
 * @param pstBitmap(In): bitmap attribute pointer
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_RGN_SetBitMap(RGN_HANDLE Handle, const BITMAP_S *pstBitmap);

/* Attach the region onto the target channel.
 *
 * @param Handle(In): region handle
 * @param pstChn(In): target channel information pointer
 * @param pstChnAttr(In): region chn attribute pointer
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_RGN_AttachToChn(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, const RGN_CHN_ATTR_S *pstChnAttr);

/* Detach the region from the target channel.
 *
 * @param Handle(In): region handle
 * @param pstChn(In): target channel information pointer
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_RGN_DetachFromChn(RGN_HANDLE Handle, const MMF_CHN_S *pstChn);

/* Set the channel display attribute for the region.
 *
 * @param Handle(In): region handle
 * @param pstChn(In): target channel information pointer
 * @param pstChnAttr(In): region chn attribute pointer
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_RGN_SetDisplayAttr(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, const RGN_CHN_ATTR_S *pstChnAttr);

/* Get the channel display attribute for the region.
 *
 * @param Handle(In): region handle
 * @param pstChn(In): target channel information pointer
 * @param pstChnAttr(Out): region chn attribute pointer
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_RGN_GetDisplayAttr(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, RGN_CHN_ATTR_S *pstChnAttr);

/* Get the canvas information for the region.
 *
 * @param Handle(In): region handle
 * @param pstCanvasInfo(Out): canvas information pointer
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_RGN_GetCanvasInfo(RGN_HANDLE Handle, RGN_CANVAS_INFO_S *pstCanvasInfo);

/* Update the region canvas.
 *
 * @param Handle(In): region handle
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_RGN_UpdateCanvas(RGN_HANDLE Handle);

/* Invert region color.
 *
 * @param Handle(In): region handle
 * @param pstChn(In): target channel information pointer
 * @param pu32Color(In): color information
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_RGN_Invert_Color(RGN_HANDLE Handle, MMF_CHN_S *pstChn, CVI_U32 *pu32Color);

/* Set channel palette information.
 *
 * @param Handle(In): region handle
 * @param pstChn(In): target channel information pointer
 * @param pstPalette(In): palette information
 * @return Error code (0 if successful)
 */
CVI_S32 CVI_RGN_SetChnPalette(RGN_HANDLE Handle, const MMF_CHN_S *pstChn, RGN_PALETTE_S *pstPalette);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* __CVI_REGION_H__ */

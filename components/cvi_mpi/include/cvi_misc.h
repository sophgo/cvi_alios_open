/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_misc.h
 * Description:
 *   MMF Programe Interface for system
 */

#ifndef __CVI_MISC_H__
#define __CVI_MISC_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <cvi_type.h>

typedef enum {
	CVI_EFUSE_AREA_USER, /* efuse area for user-defined use */
	CVI_EFUSE_AREA_DEVICE_ID, /* efuse area for device ID */
	CVI_EFUSE_AREA_HASH0_PUBLIC, /* secureboot HASH area of an RSA public key */
	CVI_EFUSE_AREA_LOADER_EK, /* secureboot AES key area */
	CVI_EFUSE_AREA_DEVICE_EK, /* device ek area */
	CVI_EFUSE_AREA_CHIP_SN, /* efuse area for chip_sn */
	CVI_EFUSE_AREA_LAST /*end identifier*/
} CVI_EFUSE_AREA_E;
typedef enum {
	CVI_EFUSE_LOCK_HASH0_PUBLIC, /* lock the secureboot RSA public key hash area */
	CVI_EFUSE_LOCK_LOADER_EK, /* lock the secureboot AES key area */
	CVI_EFUSE_LOCK_DEVICE_EK, /* lock device ek area */
	CVI_EFUSE_LOCK_LAST /*end identifier  */
} CVI_EFUSE_LOCK_E;

CVI_S32 CVI_MISC_GetChipSNSize(CVI_U32 *pu32SNSize);
CVI_S32 CVI_MISC_GetChipSN(CVI_U8 *pu8SN, CVI_U32 u32SNSize);

CVI_S32 CVI_MISC_StartPMThread(void);
CVI_S32 CVI_MISC_StopPMThread(void);
/** <!-- [EFUSE] */
CVI_S32 CVI_EFUSE_GetSize(CVI_EFUSE_AREA_E area, CVI_U32 *size);
CVI_S32 CVI_EFUSE_Read(CVI_EFUSE_AREA_E area, CVI_U8 *buf, CVI_U32 buf_size);
CVI_S32 CVI_EFUSE_Write(CVI_EFUSE_AREA_E area, const CVI_U8 *buf,
			CVI_U32 buf_size);
CVI_S32 CVI_EFUSE_EnableSecureBoot(void);
CVI_S32 CVI_EFUSE_IsSecureBootEnabled(void);
CVI_S32 CVI_EFUSE_EnableFastBoot(void);
CVI_S32 CVI_EFUSE_IsFastBootEnabled(void);
/**
 * @brief Lock eFuse area for both READING and WRITING.
 *
 * @param[in] lock - The area to be locked.
 *
 * @return CVI_SUCCESS (0) on success.
 * @return Non-zero error code on failure.
 */
CVI_S32 CVI_EFUSE_Lock(CVI_EFUSE_LOCK_E lock);
CVI_S32 CVI_EFUSE_IsLocked(CVI_EFUSE_LOCK_E lock);
/**
 * @brief Lock eFuse area for WRITING only.
 *
 * @param[in] lock - The area to be locked.
 *
 * @return CVI_SUCCESS (0) on success.
 * @return Non-zero error code on failure.
 */
CVI_S32 CVI_EFUSE_LockWrite(CVI_EFUSE_LOCK_E lock);
CVI_S32 CVI_EFUSE_IsWriteLocked(CVI_EFUSE_LOCK_E lock);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /*__CVI_MISC_H__ */

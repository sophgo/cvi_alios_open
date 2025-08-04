#ifndef __PLATFORM_SYS_H__
#define __PLATFORM_SYS_H__

#include "cvi_comm_sys.h"
#include "cvi_debug.h"


CVI_S32 platform_sys_init(CVI_VOID);

CVI_S32 platform_sys_exit(CVI_VOID);

CVI_S32 platform_sys_bind(const MMF_CHN_S *pstSrcChn, const MMF_CHN_S *pstDestChn);

CVI_S32 platform_sys_unbind(const MMF_CHN_S *pstSrcChn, const MMF_CHN_S *pstDestChn);

CVI_S32 platform_sys_getbindbydest(const MMF_CHN_S *pstDestChn, MMF_CHN_S *pstSrcChn);

CVI_S32 platform_sys_getbindbysrc(const MMF_CHN_S *pstSrcChn, MMF_BIND_DEST_S *pstBindDest);

CVI_S32 platform_sys_getversion(MMF_VERSION_S *pstVersion);

CVI_S32 platform_sys_getchipid(CVI_U32 *pu32ChipId);

CVI_S32 platform_sys_getpoweronreason(CVI_U32 *pu32PowerOnReason);

CVI_S32 platform_sys_getchipversion(CVI_U32 *pu32ChipVersion);

CVI_VOID *platform_sys_mmap(CVI_U64 u64PhyAddr, CVI_U32 u32Size);

CVI_VOID *platform_sys_mmapcache(CVI_U64 u64PhyAddr, CVI_U32 u32Size);

CVI_S32 platform_sys_munmap(void *pVirAddr, CVI_U32 u32Size);

CVI_S32 platform_sys_ionalloc(CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr,
	const CVI_CHAR *strName, CVI_U32 u32Len);

CVI_S32 platform_sys_ionalloc_cached(CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr,
	const CVI_CHAR *strName, CVI_U32 u32Len);

CVI_S32 platform_sys_ionfree(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr);

CVI_S32 platform_sys_ionflushcache(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr, CVI_U32 u32Len);

CVI_S32 platform_sys_ioninvalidatecache(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr, CVI_U32 u32Len);

CVI_S32 platform_sys_setvivpssmode(const VI_VPSS_MODE_S *pstVIVPSSMode);

CVI_S32 platform_sys_getvivpssmode(VI_VPSS_MODE_S *pstVIVPSSMode);

CVI_S32 platform_sys_setlevelconf(LOG_LEVEL_CONF_S *pstConf);

CVI_S32 platform_sys_getlevelconf(LOG_LEVEL_CONF_S *pstConf);


#endif

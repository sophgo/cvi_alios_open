#include <time.h>

#include "cvi_debug.h"
#include "cvi_errno.h"
#include "platform_sys.h"

CVI_S32 CVI_SYS_Init(CVI_VOID)
{
	return platform_sys_init();
}

CVI_S32 CVI_SYS_Exit(CVI_VOID)
{
	return platform_sys_exit();
}

CVI_S32 CVI_SYS_Bind(const MMF_CHN_S *pstSrcChn, const MMF_CHN_S *pstDestChn)
{
	return platform_sys_bind(pstSrcChn, pstDestChn);
}

CVI_S32 CVI_SYS_UnBind(const MMF_CHN_S *pstSrcChn, const MMF_CHN_S *pstDestChn)
{
	return platform_sys_unbind(pstSrcChn, pstDestChn);
}

CVI_S32 CVI_SYS_GetBindbyDest(const MMF_CHN_S *pstDestChn, MMF_CHN_S *pstSrcChn)
{
	return platform_sys_getbindbydest(pstDestChn, pstSrcChn);
}

CVI_S32 CVI_SYS_GetBindbySrc(const MMF_CHN_S *pstSrcChn, MMF_BIND_DEST_S *pstBindDest)
{
	return platform_sys_getbindbysrc(pstSrcChn, pstBindDest);
}

CVI_S32 CVI_SYS_GetVersion(MMF_VERSION_S *pstVersion)
{
	return platform_sys_getversion(pstVersion);
}

CVI_S32 CVI_SYS_GetChipId(CVI_U32 *pu32ChipId)
{
	return platform_sys_getchipid(pu32ChipId);
}

CVI_S32 CVI_SYS_GetPowerOnReason(CVI_U32 *pu32PowerOnReason)
{
	return platform_sys_getpoweronreason(pu32PowerOnReason);
}

CVI_S32 CVI_SYS_GetChipVersion(CVI_U32 *pu32ChipVersion)
{
	return platform_sys_getchipversion(pu32ChipVersion);
}

CVI_VOID *CVI_SYS_Mmap(CVI_U64 u64PhyAddr, CVI_U32 u32Size)
{
	return platform_sys_mmap(u64PhyAddr, u32Size);
}

CVI_VOID *CVI_SYS_MmapCache(CVI_U64 u64PhyAddr, CVI_U32 u32Size)
{
	return platform_sys_mmapcache(u64PhyAddr, u32Size);
}

CVI_S32 CVI_SYS_Munmap(void *pVirAddr, CVI_U32 u32Size)
{
	return platform_sys_munmap(pVirAddr, u32Size);
}

CVI_S32 CVI_SYS_IonAlloc(CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr, const CVI_CHAR *strName, CVI_U32 u32Len)
{
	return platform_sys_ionalloc(pu64PhyAddr, ppVirAddr, strName, u32Len);
}

CVI_S32 CVI_SYS_IonAlloc_Cached(CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr,
				 const CVI_CHAR *strName, CVI_U32 u32Len)
{
	return platform_sys_ionalloc_cached(pu64PhyAddr, ppVirAddr, strName, u32Len);
}

CVI_S32 CVI_SYS_IonFree(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr)
{
	return platform_sys_ionfree(u64PhyAddr, pVirAddr);
}

CVI_S32 CVI_SYS_IonFlushCache(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr, CVI_U32 u32Len)
{
	return platform_sys_ionflushcache(u64PhyAddr, pVirAddr, u32Len);
}

CVI_S32 CVI_SYS_IonInvalidateCache(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr, CVI_U32 u32Len)
{
	return platform_sys_ioninvalidatecache(u64PhyAddr, pVirAddr, u32Len);
}

CVI_S32 CVI_SYS_SetVIVPSSMode(const VI_VPSS_MODE_S *pstVIVPSSMode)
{
	return platform_sys_setvivpssmode(pstVIVPSSMode);
}

CVI_S32 CVI_SYS_GetVIVPSSMode(VI_VPSS_MODE_S *pstVIVPSSMode)
{
	return platform_sys_getvivpssmode(pstVIVPSSMode);
}

const CVI_CHAR *CVI_SYS_GetModName(MOD_ID_E id)
{
	return CVI_GET_MOD_NAME(id);
}

CVI_S32 CVI_LOG_SetLevelConf(LOG_LEVEL_CONF_S *pstConf)
{
	return platform_sys_setlevelconf(pstConf);
}

CVI_S32 CVI_LOG_GetLevelConf(LOG_LEVEL_CONF_S *pstConf)
{
	return platform_sys_getlevelconf(pstConf);
}

CVI_S32 CVI_SYS_GetCurPTS(CVI_U64 *pu64CurPTS)
{
	struct timespec ts;

	if (!pu64CurPTS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "NULL pointer\n");
		return CVI_ERR_SYS_NULL_PTR;
	}

	clock_gettime(CLOCK_MONOTONIC, &ts);
	*pu64CurPTS = ts.tv_sec*1000000 + ts.tv_nsec/1000;

	return CVI_SUCCESS;
}



#include "cvi_errno.h"
#include "platform_sys.h"
#include "base_uapi.h"
#include "sys_uapi.h"
#include "driver_base.h"
#include "driver_sys.h"
#include "driver_vi.h"
#include "driver_vpss.h"
#include "driver_vc.h"
#include "driver_vo.h"
#include "driver_rgn.h"
#include "osal.h"

#define MMF_VERSION  (CVI_CHIP_NAME MMF_VER_PRIX MK_VERSION(VER_X, VER_Y, VER_Z) VER_D)

#define MOD_CHECK_NULL_PTR(id, ptr) \
	do { \
		if (!(ptr)) { \
			CVI_TRACE_ID(CVI_DBG_ERR, id, #ptr " NULL pointer\n"); \
			return CVI_DEF_ERR(id, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR); \
		} \
	} while (0)


static osal_atomic sys_init_count;
CVI_S32 log_levels[CVI_ID_BUTT] = { [0 ... CVI_ID_BUTT - 1] = CVI_DBG_WARN};
CVI_CHAR const *log_name[8] = {
	(CVI_CHAR *)"EMG", (CVI_CHAR *)"ALT", (CVI_CHAR *)"CRI", (CVI_CHAR *)"ERR",
	(CVI_CHAR *)"WRN", (CVI_CHAR *)"NOT", (CVI_CHAR *)"INF", (CVI_CHAR *)"DBG"
};


static CVI_S32 _sys_bind_ioctl(const MMF_CHN_S *pstSrcChn, const MMF_CHN_S *pstDestChn, CVI_U8 is_bind)
{
	CVI_S32 ret = 0;
	struct sys_bind_cfg bind_cfg;

	memset(&bind_cfg, 0, sizeof(struct sys_bind_cfg));
	bind_cfg.is_bind = is_bind;
	bind_cfg.mmf_chn_src = *pstSrcChn;
	bind_cfg.mmf_chn_dst = *pstDestChn;

	ret = driver_base_ioctl(BASE_SET_BINDCFG, (unsigned long)&bind_cfg);
	if (ret)
		CVI_TRACE_SYS(CVI_DBG_ERR, "_sys_bind_ioctl()failed\n");

	return ret;
}

static CVI_S32 _sys_ion_malloc(struct sys_ion_data *para)
{
	CVI_S32 ret;

	ret = driver_base_ioctl(BASE_ION_ALLOC, (unsigned long)para);
	if (ret) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "ioctl BASE_ION_ALLOC failed\n");
		return ret;
	}

	return CVI_SUCCESS;
}

static CVI_S32 _sys_ion_free(struct sys_ion_data *para)
{
	CVI_S32 ret;

	ret = driver_base_ioctl(BASE_ION_FREE, (unsigned long)para);
	if (ret) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "ioctl BASE_ION_FREE failed\n");
		return ret;
	}

	return CVI_SUCCESS;
}

static CVI_S32 _sys_ion_alloc_cache(CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr,
			     CVI_U32 u32Len, CVI_BOOL cached, const CVI_CHAR *name)
{
	struct sys_ion_data ion_data;

	ion_data.size = u32Len;
	ion_data.cached = cached;
	// Set buffer as "anonymous" when user is passing null pointer.
	if (name)
		osal_strncpy((char *)(ion_data.name), name, MAX_ION_BUFFER_NAME);
	else
		osal_strncpy((char *)(ion_data.name), "anonymous", MAX_ION_BUFFER_NAME);

	if (_sys_ion_malloc(&ion_data) != CVI_SUCCESS) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "alloc failed.\n");
		return CVI_ERR_SYS_NOMEM;
	}

	*pu64PhyAddr = ion_data.addr_p;

	if (ppVirAddr) {
		if (cached)
			*ppVirAddr = platform_sys_mmapcache(*pu64PhyAddr, u32Len);
		else
			*ppVirAddr = platform_sys_mmap(*pu64PhyAddr, u32Len);
		if (*ppVirAddr == NULL) {
			_sys_ion_free(&ion_data);
			CVI_TRACE_SYS(CVI_DBG_ERR, "mmap failed.\n");
			return CVI_ERR_SYS_REMAPPING;
		}
	}
	return CVI_SUCCESS;
}


CVI_S32 platform_sys_init(CVI_VOID)
{
	CVI_S32 s32ret = CVI_SUCCESS;

	CVI_TRACE_SYS(CVI_DBG_INFO, "[%d]+\n", osal_atomic_read(&sys_init_count));

	if (osal_atomic_inc_return(&sys_init_count) == 1) {
		//init
		driver_vi_open();
	}

	CVI_TRACE_SYS(CVI_DBG_INFO, "[%d]-\n", osal_atomic_read(&sys_init_count));

	return s32ret;
}

CVI_S32 platform_sys_exit(CVI_VOID)
{
	CVI_S32 s32ret = CVI_SUCCESS;

	CVI_TRACE_SYS(CVI_DBG_INFO, "[%d]+\n", osal_atomic_read(&sys_init_count));

	if (osal_atomic_dec_return(&sys_init_count) > 0)
		return CVI_SUCCESS;

	if (osal_atomic_read(&sys_init_count) < 0) {
		osal_atomic_set(&sys_init_count, 0);
		return CVI_SUCCESS;
	}

	//exit
	driver_base_release();
	driver_vi_release();
	driver_vpss_release();
	driver_venc_release();
	driver_vdec_release();
#if (!defined(CONFIG_SUPPORT_VO) || (CONFIG_SUPPORT_VO))
	driver_vo_release();
#endif
	driver_rgn_release();

	CVI_TRACE_SYS(CVI_DBG_INFO, "[%d]-\n", osal_atomic_read(&sys_init_count));

	return s32ret;
}

CVI_S32 platform_sys_bind(const MMF_CHN_S *pstSrcChn, const MMF_CHN_S *pstDestChn)
{
	return _sys_bind_ioctl(pstSrcChn, pstDestChn, 1);
}

CVI_S32 platform_sys_unbind(const MMF_CHN_S *pstSrcChn, const MMF_CHN_S *pstDestChn)
{
	return _sys_bind_ioctl(pstSrcChn, pstDestChn, 0);
}

CVI_S32 platform_sys_getbindbydest(const MMF_CHN_S *pstDestChn, MMF_CHN_S *pstSrcChn)
{
	CVI_S32 ret = 0;
	struct sys_bind_cfg bind_cfg;

	osal_memset(&bind_cfg, 0, sizeof(struct sys_bind_cfg));
	bind_cfg.get_by_src = 0;
	bind_cfg.mmf_chn_dst = *pstDestChn;

	ret = driver_base_ioctl(BASE_GET_BINDCFG, (unsigned long)&bind_cfg);

	if (ret) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "CVI_SYS_GetBindbyDest() failed\n");
		return ret;
	}

	osal_memcpy(pstSrcChn, &bind_cfg.mmf_chn_src, sizeof(MMF_CHN_S));
	return CVI_SUCCESS;

}

CVI_S32 platform_sys_getbindbysrc(const MMF_CHN_S *pstSrcChn, MMF_BIND_DEST_S *pstBindDest)
{
	CVI_S32 ret = 0;
	struct sys_bind_cfg bind_cfg;

	osal_memset(&bind_cfg, 0, sizeof(struct sys_bind_cfg));
	bind_cfg.get_by_src = 1;
	bind_cfg.mmf_chn_src = *pstSrcChn;

	ret = driver_base_ioctl(BASE_GET_BINDCFG, (unsigned long)&bind_cfg);

	if (ret) {
		CVI_TRACE_SYS(CVI_DBG_NOTICE, "CVI_SYS_GetBindbySrc() failed\n");
		osal_memset(pstBindDest, 0, sizeof(MMF_BIND_DEST_S));
		return ret;
	}
	osal_memcpy(pstBindDest, &bind_cfg.bind_dst, sizeof(MMF_BIND_DEST_S));

	return CVI_SUCCESS;
}

CVI_S32 platform_sys_getversion(MMF_VERSION_S *pstVersion)
{
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstVersion);

	snprintf(pstVersion->version, VERSION_NAME_MAXLEN, "%s", MMF_VERSION);
	return CVI_SUCCESS;
}

CVI_S32 platform_sys_getchipid(CVI_U32 *pu32ChipId)
{
	static CVI_U32 id = 0xffffffff;

	if (id == 0xffffffff) {
		CVI_U32 tmp = 0;

		if (driver_sys_ioctl(SYS_IOC_READ_CHIP_ID, (unsigned long)&tmp) < 0) {
			CVI_TRACE_SYS(CVI_DBG_ERR, "ioctl SYS_IOC_READ_CHIP_ID failed\n");
			return CVI_FAILURE;
		}

		id = tmp;
	}

	*pu32ChipId = id;
	return CVI_SUCCESS;
}

CVI_S32 platform_sys_getpoweronreason(CVI_U32 *pu32PowerOnReason)
{
	CVI_U32 ret_val = 0x0;
	CVI_U32 reason = 0x0;

	if (driver_sys_ioctl(SYS_IOC_READ_CHIP_PWR_ON_REASON, (unsigned long)&reason) < 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "SYS_IOC_READ_CHIP_PWR_ON_REASON failed\n");
		return CVI_FAILURE;
	}

	switch (reason) {
	case E_CHIP_PWR_ON_COLDBOOT:
		ret_val = CVI_COLDBOOT;
	break;
	case E_CHIP_PWR_ON_WDT:
		ret_val = CVI_WDTBOOT;
	break;
	case E_CHIP_PWR_ON_SUSPEND:
		ret_val = CVI_SUSPENDBOOT;
	break;
	case E_CHIP_PWR_ON_WARM_RST:
		ret_val = CVI_WARMBOOT;
	break;
	default:
		CVI_TRACE_SYS(CVI_DBG_ERR, "unknown reason (%#x)\n", reason);
		return CVI_ERR_SYS_NOT_PERM;
	break;
	}

	*pu32PowerOnReason = ret_val;
	return CVI_SUCCESS;
}

CVI_S32 platform_sys_getchipversion(CVI_U32 *pu32ChipVersion)
{
	static CVI_U32 version = 0xffffffff;

	if (version == 0xffffffff) {
		CVI_U32 tmp = 0;

		if (driver_sys_ioctl(SYS_IOC_READ_CHIP_VERSION, (unsigned long)&tmp) < 0) {
			CVI_TRACE_SYS(CVI_DBG_ERR, "ioctl SYS_IOC_READ_CHIP_VERSION failed\n");
			return CVI_FAILURE;
		}

		switch (tmp) {
		case E_CHIPVERSION_U01:
			version = CVIU01;
		break;
		case E_CHIPVERSION_U02:
			version = CVIU02;
		break;
		default:
			CVI_TRACE_SYS(CVI_DBG_ERR, "unknown version(%#x)\n", tmp);
			return CVI_ERR_SYS_NOT_PERM;
		break;
		}
	}

	*pu32ChipVersion = version;
	return CVI_SUCCESS;
}

void *platform_sys_mmap(CVI_U64 u64PhyAddr, CVI_U32 u32Size)
{
	return (void *)u64PhyAddr;
}

/* CVI_SYS_MmapCache - mmap the physical address to cached virtual-address
 *
 * @param pu64PhyAddr: the phy-address of the buffer
 * @param u32Size: the length of the buffer
 * @return virtual-address if success; 0 if fail.
 */
void *platform_sys_mmapcache(CVI_U64 u64PhyAddr, CVI_U32 u32Size)
{
	platform_sys_ioninvalidatecache(u64PhyAddr, (void *)u64PhyAddr, u32Size);
	return (void *)u64PhyAddr;
}

CVI_S32 platform_sys_munmap(void *pVirAddr, CVI_U32 u32Size)
{
	return CVI_SUCCESS;
}

CVI_S32 platform_sys_ionalloc(CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr, const CVI_CHAR *strName, CVI_U32 u32Len)
{
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pu64PhyAddr);

	return _sys_ion_alloc_cache(pu64PhyAddr, ppVirAddr, u32Len, CVI_FALSE, strName);
}

/* CVI_SYS_IonAlloc_Cached - acquire buffer of u32Len from ion
 *
 * @param pu64PhyAddr: the phy-address of the buffer
 * @param ppVirAddr: the cached vir-address of the buffer
 * @param strName: the name of the buffer
 * @param u32Len: the length of the buffer acquire
 * @return CVI_SUCCES if ok
 */
CVI_S32 platform_sys_ionalloc_cached(CVI_U64 *pu64PhyAddr, CVI_VOID **ppVirAddr,
				 const CVI_CHAR *strName, CVI_U32 u32Len)
{
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pu64PhyAddr);

	return _sys_ion_alloc_cache(pu64PhyAddr, ppVirAddr, u32Len, CVI_TRUE, strName);
}

CVI_S32 platform_sys_ionfree(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr)
{
	struct sys_ion_data ion_data;
	int ret;

	ion_data.addr_p = u64PhyAddr;
	ret = _sys_ion_free(&ion_data);
	if (ret) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "ionFree failed\n");
		return ret;
	}

	return CVI_SUCCESS;
}

CVI_S32 platform_sys_ionflushcache(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr, CVI_U32 u32Len)
{
	CVI_S32 ret = CVI_SUCCESS;
	struct sys_cache_op cache_cfg;

	if (pVirAddr == NULL) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "pVirAddr Null.\n");
		return CVI_ERR_SYS_NULL_PTR;
	}

	cache_cfg.addr_p = u64PhyAddr;
	cache_cfg.addr_v = pVirAddr;
	cache_cfg.size = u32Len;

	ret = driver_base_ioctl(BASE_CACHE_FLUSH, (unsigned long)&cache_cfg);
	if (ret < 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "ion flush err.\n");
		ret = CVI_ERR_SYS_NOTREADY;
	}
	return ret;
}

CVI_S32 platform_sys_ioninvalidatecache(CVI_U64 u64PhyAddr, CVI_VOID *pVirAddr, CVI_U32 u32Len)
{
	CVI_S32 ret = CVI_SUCCESS;
	struct sys_cache_op cache_cfg;

	if (pVirAddr == NULL) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "pVirAddr Null.\n");
		return CVI_ERR_SYS_NULL_PTR;
	}

	cache_cfg.addr_p = u64PhyAddr;
	cache_cfg.addr_v = pVirAddr;
	cache_cfg.size = u32Len;

	ret = driver_base_ioctl(BASE_CACHE_INVLD, (unsigned long)&cache_cfg);
	if (ret < 0) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "ion invalid err.\n");
		ret = CVI_ERR_SYS_NOTREADY;
	}
	return ret;
}

CVI_S32 platform_sys_setvivpssmode(const VI_VPSS_MODE_S *pstVIVPSSMode)
{
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstVIVPSSMode);

	return driver_sys_ioctl(SYS_IOC_SET_VIVPSSMODE, (unsigned long)pstVIVPSSMode);
}

CVI_S32 platform_sys_getvivpssmode(VI_VPSS_MODE_S *pstVIVPSSMode)
{
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstVIVPSSMode);

	return driver_sys_ioctl(SYS_IOC_GET_VIVPSSMODE, (unsigned long)pstVIVPSSMode);
}

CVI_S32 platform_sys_setlevelconf(LOG_LEVEL_CONF_S *pstConf)
{
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstConf);

	if (pstConf->enModId >= CVI_ID_BUTT) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Invalid ModId(%d)\n", pstConf->enModId);
		return CVI_ERR_SYS_ILLEGAL_PARAM;
	}

	log_levels[pstConf->enModId] = pstConf->s32Level;
	return CVI_SUCCESS;
}

CVI_S32 platform_sys_getlevelconf(LOG_LEVEL_CONF_S *pstConf)
{
	MOD_CHECK_NULL_PTR(CVI_ID_SYS, pstConf);

	if (pstConf->enModId >= CVI_ID_BUTT) {
		CVI_TRACE_SYS(CVI_DBG_ERR, "Invalid ModId(%d)\n", pstConf->enModId);
		return CVI_ERR_SYS_ILLEGAL_PARAM;
	}

	pstConf->s32Level = log_levels[pstConf->enModId];
	return CVI_SUCCESS;
}


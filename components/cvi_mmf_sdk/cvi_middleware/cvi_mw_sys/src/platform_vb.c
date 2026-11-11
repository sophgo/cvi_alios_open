#include "cvi_errno.h"
#include "cvi_comm_vb.h"
#include "cvi_debug.h"
#include "platform_sys.h"
#include "platform_vb.h"
#include "vb_ioctl.h"


#define MOD_CHECK_NULL_PTR(id, ptr) \
	do { \
		if (!(ptr)) { \
			CVI_TRACE_ID(CVI_DBG_ERR, id, #ptr " NULL pointer\n"); \
			return CVI_DEF_ERR(id, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR); \
		} \
	} while (0)


static int vb_inited;

/**************************************************************************
 *   Public APIs.
 **************************************************************************/
CVI_S32 platform_vb_init(void)
{
	CVI_S32 s32Ret;

	// Only init once until exit.
	if (vb_inited)
		return CVI_SUCCESS;

	s32Ret = vb_ioctl_init();
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_init fail, ret(%d)\n", s32Ret);
		return CVI_FAILURE;
	}
	vb_inited = 1;

	return CVI_SUCCESS;
}

CVI_S32 platform_vb_exit(void)
{
	CVI_S32 s32Ret;

	// Only exit once.
	if (!vb_inited)
		return CVI_SUCCESS;

	s32Ret = vb_ioctl_exit();
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_exit fail, ret(%d)\n", s32Ret);
		return CVI_FAILURE;
	}
	vb_inited = 0;

	return CVI_SUCCESS;
}

/* platform_vb_getblock: acquice a vb_blk with specific size from pool.
 *
 * @param pool: the pool to acquice blk. if VB_INVALID_POOLID, go through common-pool to search.
 * @param u32BlkSize: the size of vb_blk to acquire.
 * @return: the vb_blk if available. otherwise, VB_INVALID_HANDLE.
 */
VB_BLK platform_vb_getblock(VB_POOL Pool, CVI_U32 u32BlkSize)
{
	CVI_S32 s32Ret;
	struct vb_blk_cfg cfg;

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.pool_id = Pool;
	cfg.blk_size = u32BlkSize;
	s32Ret = vb_ioctl_get_block(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_get_block fail, ret(%d)\n", s32Ret);
		return VB_INVALID_HANDLE;
	}
	return (VB_BLK)cfg.blk;
}

/* platform_vb_releaseblock: release a vb_blk.
 *
 * @param Block: the vb_blk going to be released.
 * @return: CVI_SUCCESS if success; others if fail.
 */
CVI_S32 platform_vb_releaseblock(VB_BLK Block)
{
	CVI_S32 s32Ret;

	s32Ret = vb_ioctl_release_block(Block);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_release_block fail, ret(%d)\n", s32Ret);
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}

VB_BLK platform_vb_physaddr2handle(CVI_U64 u64PhyAddr)
{
	CVI_S32 s32Ret;
	struct vb_blk_info blk_info;

	osal_memset(&blk_info, 0, sizeof(blk_info));
	blk_info.phy_addr = u64PhyAddr;
	s32Ret = vb_ioctl_phys_to_handle(&blk_info);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_phys_to_handle fail, ret(%d)\n", s32Ret);
		return VB_INVALID_HANDLE;
	}
	return (VB_BLK)blk_info.blk;
}

CVI_U64 platform_vb_handle2physaddr(VB_BLK Block)
{
	CVI_S32 s32Ret;
	struct vb_blk_info blk_info;

	osal_memset(&blk_info, 0, sizeof(blk_info));
	blk_info.blk = Block;
	s32Ret = vb_ioctl_get_blk_info(&blk_info);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_get_blk_info fail, ret(%d)\n", s32Ret);
		return 0;
	}
	return (CVI_U64)blk_info.phy_addr;
}

VB_POOL platform_vb_handle2poolid(VB_BLK Block)
{
	CVI_S32 s32Ret;
	struct vb_blk_info blk_info;

	osal_memset(&blk_info, 0, sizeof(blk_info));
	blk_info.blk = Block;
	s32Ret = vb_ioctl_get_blk_info(&blk_info);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_get_blk_info fail, ret(%d)\n", s32Ret);
		return VB_INVALID_POOLID;
	}
	return (VB_POOL)blk_info.pool_id;
}

CVI_S32 platform_vb_inquireusercnt(VB_BLK Block, CVI_U32 *pCnt)
{
	CVI_S32 s32Ret;
	struct vb_blk_info blk_info;

	MOD_CHECK_NULL_PTR(CVI_ID_VB, pCnt);

	osal_memset(&blk_info, 0, sizeof(blk_info));
	blk_info.blk = Block;
	s32Ret = vb_ioctl_get_blk_info(&blk_info);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_get_blk_info fail, ret(%d)\n", s32Ret);
		return CVI_FAILURE;
	}
	*pCnt = blk_info.usr_cnt;
	return CVI_SUCCESS;
}

VB_POOL platform_vb_createpool(VB_POOL_CONFIG_S *pstVbPoolCfg)
{
	CVI_S32 s32Ret;
	struct vb_pool_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VB, pstVbPoolCfg);

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.blk_size = pstVbPoolCfg->u32BlkSize;
	cfg.blk_cnt = pstVbPoolCfg->u32BlkCnt;
	cfg.remap_mode = pstVbPoolCfg->enRemapMode;
	strncpy((char *)cfg.pool_name, pstVbPoolCfg->acName, VB_POOL_NAME_LEN - 1);

	s32Ret = vb_ioctl_create_pool(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_create_pool fail, ret(%d)\n", s32Ret);
		return VB_INVALID_POOLID;
	}

	return (VB_POOL)cfg.pool_id;
}

VB_POOL platform_vb_createexpool(VB_POOL_CONFIG_EX_S *pstVbPoolExCfg)
{
	CVI_S32 s32Ret, i;
	struct vb_pool_ex_cfg cfg;

	MOD_CHECK_NULL_PTR(CVI_ID_VB, pstVbPoolExCfg);

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.blk_cnt = pstVbPoolExCfg->u32BlkCnt;
	for (i = 0; i < VB_POOL_MAX_BLK; i++) {
		cfg.addr_p[i][0] = pstVbPoolExCfg->astUserBlk[i].au64PhyAddr[0];
		cfg.addr_p[i][1] = pstVbPoolExCfg->astUserBlk[i].au64PhyAddr[1];
		cfg.addr_p[i][2] = pstVbPoolExCfg->astUserBlk[i].au64PhyAddr[2];
	}

	s32Ret = vb_ioctl_create_ex_pool(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_create_ex_pool fail, ret(%d)\n", s32Ret);
		return VB_INVALID_POOLID;
	}

	return (VB_POOL)cfg.pool_id;
}

CVI_S32 platform_vb_destroypool(VB_POOL Pool)
{
	CVI_S32 s32Ret;

	s32Ret = vb_ioctl_destroy_pool(Pool);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_destroy_pool fail, ret(%d)\n", s32Ret);
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}

CVI_S32 platform_vb_setconfig(const VB_CONFIG_S *pstVbConfig)
{
	CVI_S32 s32Ret;
	struct vb_cfg cfg;
	CVI_U32 i;

	MOD_CHECK_NULL_PTR(CVI_ID_VB, pstVbConfig);
	if (pstVbConfig->u32MaxPoolCnt > VB_COMM_POOL_MAX_CNT) {
		CVI_TRACE_VB(CVI_DBG_ERR, "Invalid vb u32MaxPoolCnt(%d)\n",
			pstVbConfig->u32MaxPoolCnt);
		return CVI_ERR_VB_ILLEGAL_PARAM;
	}

	osal_memset(&cfg, 0, sizeof(cfg));
	cfg.comm_pool_cnt = pstVbConfig->u32MaxPoolCnt;
	for (i = 0; i < cfg.comm_pool_cnt; ++i) {
		cfg.comm_pool[i].blk_size = pstVbConfig->astCommPool[i].u32BlkSize;
		cfg.comm_pool[i].blk_cnt = pstVbConfig->astCommPool[i].u32BlkCnt;
		cfg.comm_pool[i].remap_mode = pstVbConfig->astCommPool[i].enRemapMode;
		osal_strncpy((char *)cfg.comm_pool[i].pool_name,
			pstVbConfig->astCommPool[i].acName, VB_POOL_NAME_LEN - 1);
	}
	s32Ret = vb_ioctl_set_config(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_set_config fail, ret(%d)\n", s32Ret);
		return CVI_FAILURE;
	}
	return CVI_SUCCESS;
}

CVI_S32 platform_vb_getconfig(VB_CONFIG_S *pstVbConfig)
{
	CVI_S32 s32Ret;
	struct vb_cfg cfg;
	CVI_U32 i;

	MOD_CHECK_NULL_PTR(CVI_ID_VB, pstVbConfig);

	s32Ret = vb_ioctl_get_config(&cfg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_get_config fail, ret(%d)\n", s32Ret);
		return CVI_FAILURE;
	}

	osal_memset(pstVbConfig, 0, sizeof(*pstVbConfig));
	pstVbConfig->u32MaxPoolCnt = cfg.comm_pool_cnt;
	for (i = 0; i < cfg.comm_pool_cnt; ++i) {
		pstVbConfig->astCommPool[i].u32BlkSize = cfg.comm_pool[i].blk_size;
		pstVbConfig->astCommPool[i].u32BlkCnt = cfg.comm_pool[i].blk_cnt;
		pstVbConfig->astCommPool[i].enRemapMode = cfg.comm_pool[i].remap_mode;
		osal_strncpy(pstVbConfig->astCommPool[i].acName, (char *)cfg.comm_pool[i].pool_name,
			MAX_VB_POOL_NAME_LEN - 1);
	}
	return CVI_SUCCESS;
}

/* CVI_VB_MmapPool - mmap the whole pool to get virtual-address
 *
 * @param Pool: pool id
 * @return CVI_SUCCESS if success; others if fail
 */
CVI_S32 platform_vb_mmappool(VB_POOL Pool)
{
	return CVI_SUCCESS;
}

CVI_S32 platform_vb_munmappoold(VB_POOL Pool)
{
	return CVI_SUCCESS;
}

/* CVI_VB_GetBlockVirAddr - to get virtual-address of the Block
 *
 * @param Pool: pool id
 * @param Block: block id
 * @param ppVirAddr: virtual-address of the Block, cached if pool create with VB_REMAP_MODE_CACHED
 * @return CVI_SUCCESS if success; others if fail
 */
CVI_S32 platform_vb_getblockviraddr(VB_POOL Pool, VB_BLK Block, void **ppVirAddr)
{
	CVI_U64 phyAddr;

	MOD_CHECK_NULL_PTR(CVI_ID_VB, ppVirAddr);

	phyAddr = platform_vb_handle2physaddr(Block);
	if (!phyAddr) {
		CVI_TRACE_VB(CVI_DBG_ERR, "phyAddr = 0.\n");
		return CVI_ERR_VB_ILLEGAL_PARAM;
	}

	*ppVirAddr = (void *)phyAddr;
	return CVI_SUCCESS;
}

CVI_VOID platform_vb_printpool(VB_POOL Pool)
{
	CVI_S32 s32Ret;

	s32Ret = vb_ioctl_print_pool(Pool);
	if (s32Ret != CVI_SUCCESS)
		CVI_TRACE_VB(CVI_DBG_ERR, "vb_ioctl_print_pool fail, ret(%d)\n", s32Ret);
}


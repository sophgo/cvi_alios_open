#include "platform_vb.h"


VB_BLK CVI_VB_GetBlock(VB_POOL Pool, CVI_U32 u32BlkSize)
{
	return platform_vb_getblock(Pool, u32BlkSize);
}

CVI_S32 CVI_VB_ReleaseBlock(VB_BLK Block)
{
	return platform_vb_releaseblock(Block);
}

VB_BLK CVI_VB_PhysAddr2Handle(CVI_U64 u64PhyAddr)
{
	return platform_vb_physaddr2handle(u64PhyAddr);
}

CVI_U64 CVI_VB_Handle2PhysAddr(VB_BLK Block)
{
	return platform_vb_handle2physaddr(Block);
}

VB_POOL CVI_VB_Handle2PoolId(VB_BLK Block)
{
	return platform_vb_handle2poolid(Block);
}

CVI_S32 CVI_VB_InquireUserCnt(VB_BLK Block, CVI_U32 *pCnt)
{
	return platform_vb_inquireusercnt(Block, pCnt);
}

CVI_S32 CVI_VB_Init(CVI_VOID)
{
	return platform_vb_init();
}

CVI_S32 CVI_VB_Exit(CVI_VOID)
{
	return platform_vb_exit();
}

VB_POOL CVI_VB_CreatePool(VB_POOL_CONFIG_S *pstVbPoolCfg)
{
	return platform_vb_createpool(pstVbPoolCfg);
}

VB_POOL CVI_VB_CreateExPool(VB_POOL_CONFIG_EX_S *pstVbPoolExCfg)
{
	return platform_vb_createexpool(pstVbPoolExCfg);
}

CVI_S32 CVI_VB_DestroyPool(VB_POOL Pool)
{
	return platform_vb_destroypool(Pool);
}

CVI_S32 CVI_VB_SetConfig(const VB_CONFIG_S *pstVbConfig)
{
	return platform_vb_setconfig(pstVbConfig);
}

CVI_S32 CVI_VB_GetConfig(VB_CONFIG_S *pstVbConfig)
{
	return platform_vb_getconfig(pstVbConfig);
}

CVI_S32 CVI_VB_MmapPool(VB_POOL Pool)
{
	return platform_vb_mmappool(Pool);
}

CVI_S32 CVI_VB_MunmapPool(VB_POOL Pool)
{
	return platform_vb_munmappoold(Pool);
}

CVI_S32 CVI_VB_GetBlockVirAddr(VB_POOL Pool, VB_BLK Block, void **ppVirAddr)
{
	return platform_vb_getblockviraddr(Pool, Block, ppVirAddr);
}

CVI_VOID CVI_VB_PrintPool(VB_POOL Pool)
{
	return platform_vb_printpool(Pool);
}


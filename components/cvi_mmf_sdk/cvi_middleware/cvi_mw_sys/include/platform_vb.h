#ifndef __PLATFORM_VB_H__
#define __PLATFORM_VB_H__

#include "cvi_comm_vb.h"

CVI_S32 platform_vb_init(CVI_VOID);

CVI_S32 platform_vb_exit(CVI_VOID);

VB_BLK platform_vb_getblock(VB_POOL Pool, CVI_U32 u32BlkSize);

CVI_S32 platform_vb_releaseblock(VB_BLK Block);

VB_BLK platform_vb_physaddr2handle(CVI_U64 u64PhyAddr);

CVI_U64 platform_vb_handle2physaddr(VB_BLK Block);

VB_POOL platform_vb_handle2poolid(VB_BLK Block);

CVI_S32 platform_vb_inquireusercnt(VB_BLK Block, CVI_U32 *pCnt);

VB_POOL platform_vb_createpool(VB_POOL_CONFIG_S *pstVbPoolCfg);

VB_POOL platform_vb_createexpool(VB_POOL_CONFIG_EX_S *pstVbPoolExCfg);

CVI_S32 platform_vb_destroypool(VB_POOL Pool);

CVI_S32 platform_vb_setconfig(const VB_CONFIG_S *pstVbConfig);

CVI_S32 platform_vb_getconfig(VB_CONFIG_S *pstVbConfig);

CVI_S32 platform_vb_mmappool(VB_POOL Pool);

CVI_S32 platform_vb_munmappoold(VB_POOL Pool);

CVI_S32 platform_vb_getblockviraddr(VB_POOL Pool, VB_BLK Block, void **ppVirAddr);

CVI_VOID platform_vb_printpool(VB_POOL Pool);


#endif

/*
* Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File : typedef.h
* Description :
* History :
*   Author  : xyliu <xyliu@allwinnertech.com>
*   Date    : 2016/04/13
*   Comment :
*
*
*/


#ifndef SC_INTERFACE_H
#define SC_INTERFACE_H

#include <stddef.h>
#include <stdint.h>
#include <assert.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct ScMemOpsS
{
    int  (*open)(void);
    int  (*open2)(void *, void *);
    void (*close)(void);

    int  (*total_size)(void);

    void *(*palloc)(int /*size*/, void *, void *);

    void *(*palloc_no_cache)(int /*size*/, void *, void *);

    void (*pfree)(void* /*mem*/, void *, void *);

    void (*flush_cache)(void * /*mem*/, int /*size*/);

    void *(*ve_get_phyaddr)(void * /*viraddr*/);

    void *(*ve_get_viraddr)(void * /*phyaddr*/);

    void *(*cpu_get_phyaddr)(void * /*viraddr*/);

    void *(*cpu_get_viraddr)(void * /*phyaddr*/);

    int (*mem_set)(void * /*s*/, int /*c*/, size_t /*n*/);

    int (*mem_cpy)(void * /*dest*/, void * /*src*/, size_t /*n*/);

    int (*mem_read)(void * /*dest */, void * /*src*/, size_t /*n*/);

    int (*mem_write)(void * /*dest*/, void * /*src*/, size_t /*n*/);

    int (*setup)(void);

    int (*shutdown)(void);

    unsigned int (*get_ve_addr_offset)(void);

    int (*get_debug_info)(char*, int);

    void *(*get_vir_by_fd)(int/*dmafd*/);
    int (*get_phy_by_fd)(int/*dmafd*/, void* /*phy*/);
    int (*free_phy_by_fd)(int/*dmafd*/, unsigned long /*phy*/);
    int (*get_fd_by_vir)(void * /*viraddr*/);
};

static inline int CdcMemOpen(struct ScMemOpsS *memops)
{
    assert(memops != NULL);
    return memops->open();
}
static inline int CdcMemOpen2(struct ScMemOpsS *memops, void* veops, void* veself)
{
    assert(memops != NULL);
    return memops->open2(veops, veself);
}

//* close the memory adapter.
static inline void CdcMemClose(struct ScMemOpsS *memops)
{
    assert(memops != NULL);
    memops->close();
}

static inline int CdcMemTotalSize(struct ScMemOpsS *memops)
{
    assert(memops != NULL);
    return memops->total_size();
}

static inline void *CdcMemPalloc(struct ScMemOpsS *memops, int nSize, void *veOps, void *pVeopsSelf)
{
    assert(memops != NULL);
    return memops->palloc(nSize, veOps, pVeopsSelf);
}

static inline void CdcMemPfree(struct ScMemOpsS *memops, void* pMem, void *veOps, void *pVeopsSelf)
{
    assert(memops != NULL);
    memops->pfree(pMem, veOps, pVeopsSelf);
}

static inline void CdcMemFlushCache(struct ScMemOpsS *memops, void* pMem, int nSize)
{
    assert(memops != NULL);
    memops->flush_cache(pMem, nSize);
}

static inline void *CdcMemGetPhysicAddress(struct ScMemOpsS *memops, void* pVirtualAddress)
{
    assert(memops != NULL);
    return memops->ve_get_phyaddr(pVirtualAddress);
}

static inline void *CdcMemGetVirtualAddress(struct ScMemOpsS *memops, void* pPhysicAddress)
{
    assert(memops != NULL);
    return memops->ve_get_viraddr(pPhysicAddress);
}

static inline void CdcMemSet(struct ScMemOpsS *memops, void* pMem, int nValue, int nSize)
{
    assert(memops != NULL);
    memops->mem_set(pMem, nValue, nSize);
}

static inline void CdcMemCopy(struct ScMemOpsS *memops, void* pMemDst, void* pMemSrc, int nSize)
{
    assert(memops != NULL);
    memops->mem_cpy(pMemDst, pMemSrc, nSize);
}

static inline int CdcMemRead(struct ScMemOpsS *memops, void* pMemDst, void* pMemSrc, int nSize)
{
    assert(memops != NULL);
    memops->mem_read(pMemDst, pMemSrc, nSize);
    return 0;
}

static inline int CdcMemWrite(struct ScMemOpsS *memops,void* pMemDst,  void* pMemSrc, int nSize)
{
    assert(memops != NULL);
    memops->mem_write(pMemDst, pMemSrc, nSize);
    return 0;
}

static inline void *CdcMemGetPhysicAddressCpu(struct ScMemOpsS *memops, void *virt)
{
    assert(memops != NULL);
    return memops->cpu_get_phyaddr(virt);
}

static inline void *CdcMemGetVirtualAddressCpu(struct ScMemOpsS *memops, void *phy)
{
    assert(memops != NULL);
    return memops->cpu_get_viraddr(phy);
}

static inline int CdcMemSetup(struct ScMemOpsS *memops)
{
    assert(memops != NULL);
    return memops->setup();
}

static inline int CdcMemShutdown(struct ScMemOpsS *memops)
{
    assert(memops != NULL);
    return memops->shutdown();
}

static inline unsigned int CdcMemPallocGetVeAddrOffset(struct ScMemOpsS *memops)
{
    assert(memops != NULL);
    return memops->get_ve_addr_offset();
}

static inline int CdcMemGetDebugInfo(struct ScMemOpsS *memops,
                                                    char* pDst, int nDstBufSize)
{
    assert(memops != NULL);
    return memops->get_debug_info(pDst, nDstBufSize);
}

static inline void *CdcMemGetVirByFd(struct ScMemOpsS *memops, int nDmafd)
{
    assert(memops != NULL);
    return memops->get_vir_by_fd(nDmafd);
}

static inline int CdcMemGetPhyByFd(struct ScMemOpsS *memops,
                                                    int nDmafd, void* pPhyAddr)
{
    assert(memops != NULL);
    return memops->get_phy_by_fd(nDmafd, pPhyAddr);
}

static inline int CdcMemFreePhyByFd(struct ScMemOpsS *memops,
                                                    int nDmafd, unsigned long pPhyAddr)
{
    assert(memops != NULL);
    return memops->free_phy_by_fd(nDmafd, pPhyAddr);
}

static inline int CdcGetBufferFd(struct ScMemOpsS *memops, void* pVirAddr)
{
    assert(memops != NULL);
    return memops->get_fd_by_vir(pVirAddr);
}

#ifdef __cplusplus
}
#endif

#endif


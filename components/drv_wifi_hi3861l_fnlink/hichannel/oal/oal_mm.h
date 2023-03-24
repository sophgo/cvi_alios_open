/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: oal_mm.h $)A5DM7ND<~
 * Author: Hisilicon
 * Create: 2020-09-03
 */
#ifndef __OAL_MM_H__
#define __OAL_MM_H__

/* $)AM7ND<~0|:, */
//#include <linux/slab.h>
//#include <linux/hardirq.h>
//#include <linux/vmalloc.h>

#include "hi_types_base.h"
#include "aos/kernel.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* inline $)A:/J}IyCw */
/*****************************************************************************
 $)A9&D\ChJv  : IjGk:KPDL,5DDZ4f?U<d#,2"Ln3d0!#6TSZLinux2YWwO5M36xQT#,
             $)APhR*?<BGVP6OIOOBND:MDZ:KIOOBND5D2;M,Gi?v(GFP_KERNEL:MGFP_ATOMIC)!#
 $)AJdHk2NJ}  : ul_size: alloc mem size
 $)A75 ;X V5  : alloc mem addr
*****************************************************************************/
static inline hi_void* oal_memalloc(hi_u32 ul_size)
{
    hi_void   *puc_mem_space = HI_NULL;

    puc_mem_space = aos_malloc(ul_size);
    if (puc_mem_space == HI_NULL) {
        return HI_NULL;
    }

    return puc_mem_space;
}

static inline hi_void* oal_kzalloc(hi_u32 ul_size)
{
    return aos_malloc(ul_size);
}

static inline hi_void*  oal_vmalloc(hi_u32 ul_size)
{
    return aos_malloc(ul_size);
}

/*****************************************************************************
 $)A9&D\ChJv  : JM7E:KPDL,5DDZ4f?U<d!#
*****************************************************************************/
static inline hi_void oal_free(hi_void *p_buf)
{
    //printf("oal_free [0x%p]", p_buf);
    aos_free(p_buf);
}

static inline hi_void oal_vfree(hi_void *p_buf)
{
    aos_free(p_buf);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of oal_mm.h */


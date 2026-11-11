/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: oal_mutex.h ��ͷ�ļ�
 * Author: Hisilicon
 * Create: 2018-08-04
 */
#ifndef __OAL_MUTEX_H__
#define __OAL_MUTEX_H__

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include <aos/kernel.h>
#include "hi_types_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************
  2 STRUCT����
*****************************************************************************/
typedef aos_mutex_t          oal_mutex_stru;
typedef aos_mutex_t oal_spin_lock_stru;

static inline int mutex_init(aos_mutex_t *mutex)
{
    return aos_mutex_new(mutex);
}

static inline void mutex_destroy(aos_mutex_t *mutex)
{
    return aos_mutex_free(mutex);
}


#define    OAL_MUTEX_INIT(mutex)        mutex_init(mutex)
#define    OAL_MUTEX_DESTROY(mutex)     mutex_destroy(mutex)


/*****************************************************************************
  10 ��������
*****************************************************************************/
static inline hi_void oal_mutex_lock(oal_mutex_stru *lock)
{
    aos_mutex_lock(lock, AOS_WAIT_FOREVER);
}

static inline hi_void oal_mutex_unlock(oal_mutex_stru *lock)
{
    aos_mutex_unlock(lock);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of oal_mutex.h */


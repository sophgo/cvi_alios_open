/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: oal_spinlock.h $)A5DM7ND<~
 * Author: Hisilicon
 * Create: 2018-08-04
 */

#ifndef __OAL_SPINLOCK_H__
#define __OAL_SPINLOCK_H__

/*****************************************************************************
  1 $)AFdK{M7ND<~0|:,
*****************************************************************************/
//#include <linux/spinlock.h>
#include <aos/kernel.h>
#include "oal_mutex.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

extern aos_mutex_t g_spin_mutex;
/* inline $)A:/J}6(Re */
/*****************************************************************************
 $)A9&D\ChJv  : WTP}Kx3uJ<;/#,0QWTP}KxIhVCN*1#(N4KxW4L,#)!#
 $)AJdHk2NJ}  : *pst_lock: Kx5D5XV7
 $)AJd3v2NJ}  : N^
 $)A75 ;X V5  :
*****************************************************************************/
static inline hi_void  oal_spin_lock_init(oal_spin_lock_stru *pst_lock)
{
    aos_mutex_new(pst_lock);
}

static inline hi_void  oal_spin_lock_free(oal_spin_lock_stru *pst_lock)
{
    aos_mutex_free(pst_lock);
}

/*****************************************************************************
 $)A9&D\ChJv  : WTP}KxTZHmVP6ORT<0DZ:KO_3L5H:KPDL,IOOBND;7>3OB5D<SKx2YWw!#Hg9{
             $)AD\9;A"<4;q5CKx#,K|>MBmIO75;X#,7qTr#,K|=+WTP}TZDG@o#,V15=8CWTP}
             $)AKx5D1#3VU_JM7E#,UbJ1#,K|;q5CKx2"75;X!#
 $)AJdHk2NJ}  : *pst_lock:WTP}Kx5XV7
 $)AJd3v2NJ}  : N^
 $)A75 ;X V5  :
*****************************************************************************/
static inline hi_void  oal_spin_lock(oal_spin_lock_stru *pst_lock)
{
    aos_mutex_lock(pst_lock, AOS_WAIT_FOREVER);
}

/*****************************************************************************
 $)A9&D\ChJv  : SpinlockTZDZ:KO_3L5H:KPDL,IOOBND;7>3OB5D=bKx2YWw!#
 $)AJdHk2NJ}  : *pst_lock:WTP}Kx5XV7
 $)AJd3v2NJ}  : N^
 $)A75 ;X V5  :
*****************************************************************************/
static inline hi_void  oal_spin_unlock(oal_spin_lock_stru *pst_lock)
{
    aos_mutex_unlock(pst_lock);
}

/*****************************************************************************
 $)A9&D\ChJv  : WTP}KxTZHmVP6ORT<0DZ:KO_3L5H:KPDL,IOOBND;7>3OB5D<SKx2YWw!#Hg9{
             $)AD\9;A"<4;q5CKx#,K|>MBmIO75;X#,7qTr#,K|=+WTP}TZDG@o#,V15=8CWTP}
             $)AKx5D1#3VU_JM7E#,UbJ1#,K|;q5CKx2"75;X!#
 $)AJdHk2NJ}  : pst_lock:WTP}Kx5XV7
 $)AJd3v2NJ}  : N^
 $)A75 ;X V5  :
*****************************************************************************/
static inline hi_void oal_spin_lock_bh(oal_spin_lock_stru *pst_lock)
{

}

/*****************************************************************************
 $)A9&D\ChJv  : SpinlockTZHmVP6ORT<0DZ:KO_3L5H:KPDL,IOOBND;7>3OB5D=bKx2YWw!#
 $)AJdHk2NJ}  : N^
 $)AJd3v2NJ}  : N^
 $)A75 ;X V5  : hi_void
*****************************************************************************/
static inline hi_void oal_spin_unlock_bh(oal_spin_lock_stru *pst_lock)
{

}

/*****************************************************************************
 $)A9&D\ChJv  : ;q5CWTP}Kx5DM,J1;q5C1#4f1jV><D4fFw5DV5#,2"GRJ'P'1>5XVP6O!#
 $)AJdHk2NJ}  : *pst_lock:WTP}Kx5XV7
             pui_flags:$)A1jV><D4fFw
 $)AJd3v2NJ}  : N^
 $)A75 ;X V5  :
*****************************************************************************/
static inline hi_void  oal_spin_lock_irq_save(oal_spin_lock_stru *pst_lock, unsigned long *pui_flags)
{
    aos_mutex_lock(pst_lock, AOS_WAIT_FOREVER);
}

/*****************************************************************************
 $)A9&D\ChJv  : JM7EWTP}Kx5DM,J1#,;V841jV><D4fFw5DV5#,;V841>5XVP6O!#K|Skoal_sp-
             in_lock_irq$)AEd6TJ9SC
 $)AJdHk2NJ}  : *pst_lock:WTP}Kx5XV7
             pui_flags:$)A1jV><D4fFw
 $)AJd3v2NJ}  : N^
 $)A75 ;X V5  :
*****************************************************************************/
static inline hi_void  oal_spin_unlock_irq_restore(oal_spin_lock_stru *pst_lock, unsigned long *pui_flags)
{
    aos_mutex_unlock(pst_lock);
}



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of oal_spinlock.h */


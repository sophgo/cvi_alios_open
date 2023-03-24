/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: Header file for hcc_task.c.
 * Author: Hisilicon
 * Create: 2018-08-04
 */

#ifndef __HCC_TASK_H__
#define __HCC_TASK_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "hcc_host.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

hi_u32 hcc_task_init(hcc_handler_stru *hcc_handler);
hi_void hcc_task_exit(hi_void);

hi_void hcc_task_sched(hi_void);

hi_void hcc_exit_task_thread(hcc_handler_stru* hcc);

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

#endif /* end of frw_task.h */


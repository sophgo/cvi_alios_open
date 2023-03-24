/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: Oam external public interface header file.
 * Author: Hisilicon
 * Create: 2018-08-04
 */

#ifndef __OAM_EXT_IF_H__
#define __OAM_EXT_IF_H__

/*****************************************************************************
  1 其他头文件包含
*****************************************************************************/
#include "oal_util.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#if 1
#define oam_error_log2 printf
#define oam_error_log1 printf
#define oam_error_log0 printf
#define oam_warning_log3 printf
#define oam_error_log3 printf
#define oam_info_log0 printf
#define oam_info_log1 printf
#define oam_info_log4 printf
#define oam_warning_log0 printf

#endif


/*****************************************************************************
  6 ENUM定义
*****************************************************************************/
/* 日志级别 */
typedef enum {
    OAM_LOG_LEVEL_ERROR     =    1,       /* ERROR级别打印 */
    OAM_LOG_LEVEL_WARNING,                /* WARNING级别打印 */
    OAM_LOG_LEVEL_INFO,                   /* INFO级别打印 */

    OAM_LOG_LEVEL_BUTT
}oam_log_level_enum;

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

#endif /* end of oam_ext_if.h */

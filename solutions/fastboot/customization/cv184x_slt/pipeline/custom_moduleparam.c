/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: custom_moduleparam.c
 * Description:
 *   ....
 */
#include "custom_param.h"

PARAM_MODULE_CFG_S  g_stModuleCtx = {
    .alios_sys_mode = 1,
    .alios_vi_mode = 1,
    .alios_vpss_mode = 1,
    .alios_venc_mode = 1,
    .alios_vo_mode = 0,
};

PARAM_MODULE_CFG_S * PARAM_GET_MODULE_CFG(void) {
    return &g_stModuleCtx;
}

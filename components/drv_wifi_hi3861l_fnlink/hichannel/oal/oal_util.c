/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: oal_util.c.
 * Author: Hisilicon
 * Create: 2018-08-04
 */

/******************************************************************************
  1 ͷ�ļ�����
******************************************************************************/
#include "oal_util.h"

#ifdef __cplusplus
#if __cplusplus
    extern "C" {
#endif
#endif

/*****************************************************************************
2 ����ʵ��
*****************************************************************************/
hi_s32 oal_atoi(const hi_char *c_string)
{
    hi_s32 l_ret = 0;
    hi_s32 flag = 0;

    for (; ; c_string++) {
        switch (*c_string) {
            case '0' ... '9':
                l_ret = 10 * l_ret + (*c_string - '0'); /* 10:ʮ������ */
                break;
            case '-':
                flag = 1;
                break;
            case ' ':
                continue;
            default:
                return ((flag == 0) ? l_ret : (-l_ret));
        }
    }
}

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif


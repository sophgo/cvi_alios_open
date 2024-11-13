/*
 * Copyright (C) Cvitek Co., Ltd. 2024-2025. All rights reserved.
 *
 * File Name: cvi_defines.h
 * Description:
 *   The common definitions per chip capability.
 */
 /******************************************************************************         */

#ifndef __U_CVI_DEFINES_H__
#define __U_CVI_DEFINES_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define IS_CHIP_CV183X(x) (((x) == CVI1829) || ((x) == CVI1832) || ((x) == CVI1835) || ((x) == CVI1838))
#define IS_CHIP_CV182X(x) (((x) == CVI1820) || ((x) == CVI1821) || ((x) == CVI1822) \
						|| ((x) == CVI1823) || ((x) == CVI1825) || ((x) == CVI1826))

#define IS_CHIP_CV181X(x) (((x) == CV1820A) || ((x) == CV1821A) || ((x) == CV1822A) \
						|| ((x) == CV1823A) || ((x) == CV1825A) || ((x) == CV1826A) \
						|| ((x) == CV1810C) || ((x) == CV1811C) || ((x) == CV1812C) \
						|| ((x) == CV1811H) || ((x) == CV1812H) || ((x) == CV1813H))

#define IS_CHIP_CV180X(x) (((x) == CV1800B) || ((x) == CV1801B) \
							|| ((x) == CV1800C) || ((x) == CV1801C))

#define IS_CHIP_PKG_TYPE_QFN(x) (((x) == CV1820A) || ((x) == CV1821A) || ((x) == CV1822A) \
						|| ((x) == CV1810C) || ((x) == CV1811C) || ((x) == CV1812C) \
						|| IS_CHIP_CV180X(x))

#if defined(__CV181X__)
#include "cvi_cv181x_defines.h"
#elif defined(__CV180X__)
#include "cvi_cv180x_defines.h"
#else
#error "Unknown Chip Architecture!"
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __U_CVI_DEFINES_H__ */


/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: Header file for oal_util.h.
 * Author: Hisilicon
 * Create: 2018-08-04
 */

#ifndef __OAL_UTIL_H__
#define __OAL_UTIL_H__

/* ͷ�ļ����� */
//#include <linux/string.h>
//#include <linux/version.h>
#include <string.h>
#include "hi_types_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* �궨����� */
#define __builtin_expect(x, expected_value) (x)
#ifndef unlikely
#define unlikely(x)	__builtin_expect((!!(x)),0)
#endif
#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif
#define oal_likely(_expr)       likely(_expr)
#define oal_unlikely(_expr)     unlikely(_expr)

#define WARN_ON(condition) ({						\
	int __ret_warn_on = !!(condition);				\
	unlikely(__ret_warn_on);					\
})

#define OAL_BUG_ON(_con)        BUG_ON(_con)
#define OAL_WARN_ON(condition)  WARN_ON(condition)

#define oal_is_aligned(val, align)  (((hi_u32)(val) & ((align) - 1)) == 0)


/* inline �������� */
static inline hi_char *hi_strrchr(const hi_char *file, hi_char c)
{
    hi_char *p = HI_NULL;
    return (((p = strrchr(file, c)) != HI_NULL) ? (p + 1) : (HI_NULL));
}

/* �������� */
hi_s32 oal_atoi(const hi_char *c_string);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of oal_util.h */

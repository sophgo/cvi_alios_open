/*
 * Copyright (C) 2018-2020 Alibaba Group Holding Limited
 */

#ifndef __ATEMPO_ALL_H__
#define __ATEMPO_ALL_H__

#include "av/avutil/common.h"
#include "av/avutil/av_config.h"

__BEGIN_DECLS__

#define REGISTER_ATEMPOER(X, x)                                          \
    {                                                                     \
        extern int atempo_register_##x();                                 \
        if (CONFIG_ATEMPOER_##X)                                         \
            atempo_register_##x();                                        \
    }

/**
 * @brief  regist audio atempoer for sonic
 * @return 0/-1
 */
int atempo_register_sonic();

/**
 * @brief  regist audio atempoer for ipc
 * @return 0/-1
 */
int atempo_register_ipc();

/**
 * @brief  regist audio atempoer for dummy
 * @return 0/-1
 */
int atempo_register_dummy();

/**
 * @brief  regist audio atempoer
 * @return 0/-1
 */
static inline int atempo_register()
{
#if defined(CONFIG_ATEMPOER_SONIC)
    REGISTER_ATEMPOER(SONIC, sonic);
#endif
#if defined(CONFIG_ATEMPOER_IPC)
    REGISTER_ATEMPOER(IPC, ipc);
#endif
#if defined(CONFIG_ATEMPOER_DUMMY)
    REGISTER_ATEMPOER(DUMMY, dummy);
#endif
    return 0;
}

__END_DECLS__

#endif /* __ATEMPO_ALL_H__ */


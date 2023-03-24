/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */

#ifndef TM_PAD_DATA_H
#define TM_PAD_DATA_H

#ifdef __linux__
#include <tmedia_config.h>
#endif
#include <queue>

using namespace std;

class TMPadData
{
public:
    TMPadData() {}
    virtual ~TMPadData() {}
    uint64_t mDataID;
};

class TMPadDataQueue
{
public:
    uint64_t mNextDataID;
    std::queue<TMPadData *> mDataQueue;
};

#endif  // TM_PAD_DATA_H

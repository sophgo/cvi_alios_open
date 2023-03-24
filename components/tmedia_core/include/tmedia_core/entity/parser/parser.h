/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#pragma once

#include <string>

#ifdef __linux__
#include <tmedia_config.h>
#endif
#include <tmedia_core/common/common_inc.h>

using namespace std;

class TMParser
{
public:
    TMParser();
    virtual ~TMParser();

    virtual int Parse(TMPacket &packet, const uint8_t *buf, size_t buf_size) = 0;
};

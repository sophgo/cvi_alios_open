/*
 * Copyright (C) 2021 Alibaba Group Holding Limited
 */

#pragma once

#include <string>
#include <tmedia_core/common/common_inc.h>

using namespace std;

class TMUtilSave
{
public:
    static int PgmPicture(TMVideoFrame &frame, string &filename);
    static int Picture(TMVideoFrame &frame, string &filename);
    static int Packet(TMPacket &pkt, string &filename);
};


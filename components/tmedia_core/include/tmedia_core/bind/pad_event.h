/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */

#ifndef TM_PAD_EVENT_H
#define TM_PAD_EVENT_H

#ifdef __linux__
#include <tmedia_config.h>
#endif
#include <queue>

using namespace std;

class TMPadEvent
{
public:
    TMPadEvent();
    virtual ~TMPadEvent();

    enum class Type : uint32_t
    {
        UNKNOWN,

        /* TMEntity control */
        START,
        STOP,
        SEEK,

        STREAM_START,
        EOS,
    };

    uint64_t mEventID;
};

class TMPadEventQueue
{
public:
    uint64_t mNextEventID;
    std::queue<TMPadEvent *> mEventQueue;
};

#endif  // TM_PAD_EVENT_H

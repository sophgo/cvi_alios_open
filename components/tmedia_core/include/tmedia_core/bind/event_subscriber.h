/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */

#ifndef TM_EVENT_SUBSCRIBER_H
#define TM_EVENT_SUBSCRIBER_H

#ifdef __linux__
#include <tmedia_config.h>
#endif
#include <tmedia_core/bind/event_subscriber.h>

using namespace std;

typedef int (*TMEventHandleFunc)(int eventID, void *eventData);

class TMEventSubscriber
{
public:
    TMEventSubscriber(TMEventHandleFunc handleFunc);
    int RegisterHandleFunc(TMEventHandleFunc handleFunc);

private:
    TMEventHandleFunc mHandleFunc;
};

#endif  // TM_EVENT_SUBSCRIBER_H

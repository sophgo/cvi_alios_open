/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */

#ifndef TM_EVENT_HANDLER_H
#define TM_EVENT_HANDLER_H

#ifdef __linux__
#include <tmedia_config.h>
#endif
#include <vector>
#include <tmedia_core/bind/event_subscriber.h>

using namespace std;

class TMEventHandler
{
public:
    TMEventHandler();
    virtual ~TMEventHandler();

    int RegisterSubscriber(TMEventSubscriber *subscriber, uint32_t eventIDs = 0);
    int Subscribe(uint32_t eventIDs);   // eventIDs: bitmask of Entity event
    int Unsubscribe(uint32_t eventIDs); // eventIDs: bitmask of Entity event
    int GetEvent(uint32_t event, int timeout);

private:
    vector<TMEventSubscriber *> mSubscribers;
};

#endif  // TM_EVENT_HANDLER_H

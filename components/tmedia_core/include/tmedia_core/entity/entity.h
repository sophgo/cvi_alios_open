/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */

#include <tmedia_core/bind/pad.h>

#ifndef TM_ENTITY_H
#define TM_ENTITY_H

using namespace std;

#define TM_STR_BASE(R) #R
#define TM_STR(R)      TM_STR_BASE(R)

typedef void* (*class_new_t)();

namespace TMedia {
	void tmedia_backend_seno_init();
	void tmedia_backend_lgpl_init();
	void tmedia_backend_light_init();
	#define BackendInit() tmedia_backend_seno_init();TMedia::tmedia_backend_lgpl_init();TMedia::tmedia_backend_light_init();
}

class TMEntity
{
public:
    TMEntity(string name) : mPool(NULL)
    {
        mName = name;
    }
    virtual ~TMEntity()
    {
        DetachPool();
    }

    void AttachPool(TMBufferPool *pool)
    {
        mPool = pool;
    }

    void DetachPool()
    {
        mPool = NULL;
    }

    TMBufferPool* GetAttachPool()
    {
        return mPool;
    }

    string mName;

protected:
    TMBufferPool *mPool;
};

class TMSrcEntity : public TMEntity
{
public:
    TMSrcEntity(string name = "") : TMEntity(name) {}
    virtual TMSrcPad *GetSrcPad(int padID = 0) = 0;
};


class TMSinkEntity : public TMEntity
{
public:
    TMSinkEntity(string name = "") : TMEntity(name) {}
    virtual TMSinkPad *GetSinkPad(int padID = 0) = 0;
};

class TMFilterEntity : public TMEntity
{
public:
    TMFilterEntity(string name = "") : TMEntity(name) {}
    virtual TMSrcPad *GetSrcPad(int padID = 0) = 0;
    virtual TMSinkPad *GetSinkPad(int padID = 0) = 0;
};

#endif  // TM_ENTITY_H

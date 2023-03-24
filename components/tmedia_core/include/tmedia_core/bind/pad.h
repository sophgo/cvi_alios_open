/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */

#ifdef __linux__
#include <tmedia_config.h>
#endif
#include <typeinfo>

#include <tmedia_core/common/common_inc.h>
#include <tmedia_core/bind/pad_data.h>
#include <tmedia_core/bind/pad_event.h>

#ifndef TM_PAD_H
#define TM_PAD_H

using namespace std;
class TMEntity;

class TMPad
{
public:
    enum class Direction : uint32_t
    {
        UNKNOWN,    // Unkonw pin data direction
        SRC,        // Output pin data direction
        SINK        // Input  pin data direction
    };

    enum class Mode : uint32_t
    {
        AUTO,       // Pad will not handle dataflow
        PUSH,       // Pad handles dataflow in downstream push mode
        PULL        // Pad handles dataflow in upstream pull mode
    };

    typedef struct
    {
        string    name;
        Mode      mode;
    } Param_s;

    TMPad(TMEntity *entity, TMPad::Param_s *param);
    virtual ~TMPad();

    // data interface
    //type_info mDataTypeInfo;          // for setting data info by typeid(type/struct/class)
    int RequestData(TMPadData **data);  // alloc data for working
    int ReleaseData(TMPadData **data);  // free data back to idle data list

    // event interface
    int SendEvent(TMPadEvent *event);   // send event to peer pad, it'll triger ProcessEvent()
    int ProcessEvent(TMPadEvent *event);// Call to process event

    // connection interface
    virtual int Bind(TMPad *pad) = 0;   // Bind with peer pad
    virtual int UnBind() = 0;           // UnBind with peer pad
    int GetParent(TMEntity **entity);   // Get parent(entity)
    int GetPeer(TMPad **pad);           // Get Binded peer pad
    int GetPeer(TMEntity **entity);     // Get Binded peer pad's parent(entity)

    // state & mode interface
    int Active();
    bool IsSrcPad();
    bool IsSinkPad();

    // misc
    virtual void DumpInfo();

protected:
    int RemoveEvents(); // Called before inactive, remove all event from pad
    Direction mDirection;
    TMPadDataQueue mDataQueue;
    TMPadEventQueue mEventQueue;

    string mName;
    Mode mMode;
    bool mActive;
    TMEntity *mParent;
};

class TMSinkPad;
class TMSrcPad : public TMPad
{
    friend class TMSinkPad;

public:
    TMSrcPad(TMEntity *entity, TMPad::Param_s *param);
    virtual ~TMSrcPad() {}

    int Bind(TMPad *pad) override;  // Bind with peer TMPad
    int UnBind() override;          // UnBind with peer TMPad
    int PushData(TMPadData *data);  // push Data to peer Pad, will triger peer's ChainData()
    int GetData(TMPadData **data);  // Call by pull mode sink pad in PullData()
    void DumpInfo() override;

protected:
    TMSinkPad *mPeerPad;
};

//typedef int (*TMPadChainFunction) (TMSinkPad *pad, TMPadData *data);
using TMPadChainFunction = int(*)(TMSinkPad *pad, TMPadData *data);

class TMSinkPad : public TMPad
{
    friend class TMSrcPad;

public:
    TMSinkPad(TMEntity *entity, TMPad::Param_s *param);
    virtual ~TMSinkPad() {}

    int Bind(TMPad *pad) override;  // Bind with peer TMPad
    int UnBind() override;           // UnBind with peer TMPad
    int ChainData(TMPadData *data); // Call by push mode src pad in PushData()
    int PullData(TMPadData **data); // pull Data from peer Pad, it'll triger ChainData()

    int SetChainFunction(TMPadChainFunction func);

    void DumpInfo() override;

protected:
    static int DefaultChainFunction(TMSinkPad *pad, TMPadData *data);
    TMSrcPad *mPeerPad;
    TMPadChainFunction mChainFunc;
};

#endif  // TM_PAD_H

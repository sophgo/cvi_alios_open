/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#pragma once

#include <string>

#include <tmedia_core/common/media_info.h>

#include <tmedia_core/entity/codec/codec.h>
#include <tmedia_core/common/packet.h>
#include <tmedia_core/common/frame.h>
#include <tmedia_core/entity/entity.h>

using namespace std;

class TMVideoEncoder : public TMCodec, public TMFilterEntity
{
public:
    TMVideoEncoder();
    virtual ~TMVideoEncoder();

    // TMFilterEntity interface
    virtual TMSrcPad *GetSrcPad(int padID = 0) = 0;
    virtual TMSinkPad *GetSinkPad(int padID = 0) = 0;

    // TMCodec interface
    virtual int Open(TMCodecParams &codecParam, TMPropertyList *propList = NULL) = 0;
    virtual int SetConfig(TMPropertyList &propertyList) = 0;
    virtual int GetConfig(TMPropertyList &propertyList) = 0;
    virtual int Start() = 0;
    virtual int Flush() = 0;
    virtual int Stop() = 0;
    virtual int Close() = 0;

    // TMVideoEncoder extend interface
    virtual int SendFrame(TMVideoFrame &frame, int timeout) = 0;
    virtual int RecvPacket(TMVideoPacket &pkt, int timeout) = 0;
    virtual int ReleasePacket(TMVideoPacket &pkt) = 0;
};

class TMAudioEncoder : public TMCodec, public TMFilterEntity
{
public:
    TMAudioEncoder();
    virtual ~TMAudioEncoder();

    // TMFilterEntity interface
    virtual TMSrcPad *GetSrcPad(int padID = 0) = 0;
    virtual TMSinkPad *GetSinkPad(int padID = 0) = 0;

    // TMCodec interface
    virtual int Open(TMCodecParams &codecParam, TMPropertyList *propList = NULL) = 0;
    virtual int SetConfig(TMPropertyList &propList) = 0;
    virtual int GetConfig(TMPropertyList &propList) = 0;
    virtual int Start() = 0;
    virtual int Flush() = 0;
    virtual int Stop()  = 0;
    virtual int Close() = 0;

    // TMAudioDecoder extend interface
    virtual int SendFrame(TMAudioFrame &frame, int timeout) = 0;
    virtual int RecvPacket(TMAudioPacket &pkt, int timeout) = 0;
    virtual int ReleasePacket(TMAudioPacket &pkt) = 0;
};

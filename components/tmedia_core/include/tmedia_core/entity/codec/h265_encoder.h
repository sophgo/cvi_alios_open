/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */

#ifndef TM_H265_ENCODER_H
#define TM_H265_ENCODER_H

#include <tmedia_core/entity/codec/codec.h>

class TMH265Encoder : public TMVideoEncoder
{
public:
    enum class PropID : uint32_t
    {
        // ID                      Data Type
        OUTPUT_GOP_NUMBER,
        OUTPUT_TARGET_BITRATE,
        OUTPUT_FPS,
        OUTPUT_CHN_ATTR_OUTPUT_WIDTH,
        OUTPUT_CHN_ATTR_OUTPUT_HEIGHT,
    };

    TMH265Encoder() {}
    virtual ~TMH265Encoder() {}

    // TMFilterEntity interface
    virtual TMSrcPad *GetSrcPad(int padID = 0) = 0;
    virtual TMSinkPad *GetSinkPad(int padID = 0) = 0;
    // TMCodec interface
    virtual int Open(TMCodecParams &codecParam, TMPropertyList *propList = NULL) = 0;
    virtual int SetConfig(TMPropertyList &propertyList) = 0;
    virtual int GetConfig(TMPropertyList &propertyList) = 0;
    virtual int Start()                                 = 0;
    virtual int Flush()                                 = 0;
    virtual int Stop()                                  = 0;
    virtual int Close()                                 = 0;
 
    // TMVideoEncoder interface
    virtual int SendFrame(TMVideoFrame &frame, int timeout) = 0;
    virtual int RecvPacket(TMVideoPacket &pkt, int timeout) = 0;
    virtual int ReleasePacket(TMVideoPacket &pkt)           = 0;

protected:
    TMPropertyList mDefaultPropertyList;
    TMPropertyList mCurrentPropertyList;
};

#endif // TM_H265_ENCODER_H
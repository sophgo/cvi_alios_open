/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */

#ifndef TM_G711_ENCODER_H
#define TM_G711_ENCODER_H

#include <tmedia_core/entity/codec/codec_inc.h>

class TMG711Encoder : public TMAudioEncoder
{
public:
    enum class PropID : uint32_t
    {
        // ID                      Data Type
        CODEC_TYPE,
    };

    TMG711Encoder() {}
    virtual ~TMG711Encoder(){}

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
};


#endif  /* TM_G711_ENCODER_H */

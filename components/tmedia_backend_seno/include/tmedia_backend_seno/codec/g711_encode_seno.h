/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */

#ifndef TM_SENO_G711_ENCODER_H
#define TM_SENO_G711_ENCODER_H

#include <tmedia_core/entity/codec/codec_inc.h>

class TMG711EncoderSeno : public TMG711Encoder
{
public:
    TMG711EncoderSeno();
    virtual ~TMG711EncoderSeno();

    // TMFilterEntity interface
    TMSrcPad *GetSrcPad(int padID = 0) final override;
    TMSinkPad *GetSinkPad(int padID = 0) final override;

    // TMCodec interface
    int Open(TMCodecParams &codecParam, TMPropertyList *propList = NULL) final override;
    int SetConfig(TMPropertyList &propList) final override;
    int GetConfig(TMPropertyList &propList) final override;
    int Start() final override;
    int Flush() final override;
    int Stop()  final override;
    int Close() final override;

    // TMAudioEncoder extend interface
    int SendFrame(TMAudioFrame &frame, int timeout) final override;
    int RecvPacket(TMAudioPacket &pkt, int timeout) final override;
    int ReleasePacket(TMAudioPacket &pkt) final override;
private:
    std::vector<shared_ptr<TMAudioPacket>> pktVector;
    G711CodecType_e codecType;
};


#endif
/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#ifndef TM_JPEG_ENCODER_H
#define TM_JPEG_ENCODER_H

#include <string>

#ifdef __linux__
#include <tmedia_config.h>
#endif
#include <tmedia_core/common/media_info.h>

#include <tmedia_core/entity/codec/codec.h>
#include <tmedia_core/common/packet.h>
#include <tmedia_core/common/frame.h>

using namespace std;

class TMJpegEncoder : public TMVideoEncoder
{
public:
    enum class PropID : uint32_t
    {
        // ID                      Data Type
        ENCODE_QUALITY_FACTOR,    // int, [1, 99], 99 is the best
    };

    TMJpegEncoder() {InitDefaultPropertyList();}
    virtual ~TMJpegEncoder() {}

    // TMCodec interface
    virtual int Open(TMCodecParams &codecParam, TMPropertyList *propList = NULL) = 0;
    virtual int SetConfig(TMPropertyList &propertyList) = 0;
    virtual int GetConfig(TMPropertyList &propertyList) = 0;

    // TMVideoEncoder interface
    virtual int SendFrame(TMVideoFrame &frame, int timeout) = 0;
    virtual int RecvPacket(TMVideoPacket &pkt, int timeout) = 0;
    virtual int ReleasePacket(TMVideoPacket &pkt)           = 0;

protected:
    TMPropertyList mDefaultPropertyList;
    TMPropertyList mCurrentPropertyList;

    virtual void InitDefaultPropertyList()
    {
        TMPropertyList *pList[2] = {&mDefaultPropertyList, &mCurrentPropertyList};
        for (uint32_t i = 0; i < ARRAY_SIZE(pList); i++)
        {
            pList[i]->Add(TMProperty((int)PropID::ENCODE_QUALITY_FACTOR, 99, "quality_factor"));
        }
    }
};
#endif
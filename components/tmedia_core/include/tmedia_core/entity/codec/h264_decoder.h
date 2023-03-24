/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#ifndef TM_H264_DECODER_H
#define TM_H264_DECODER_H

#include <string>

#include <tmedia_core/common/media_info.h>
#include <tmedia_core/entity/codec/decoder.h>
#include <tmedia_core/util/util_inc.h>

using namespace std;

class TMH264Decoder : public TMVideoDecoder
{
public:
    enum class PropID : uint32_t
    {
        // ID                      Data Type
        OUTPUT_FRAME_WIDTH,     // int, <=0 means use original width
        OUTPUT_FRAME_HEIGHT,    // int, <=0 means use original height
        OUTPUT_PIXEL_FORMAT,    // TMImageInfo::PixelFormat
        FRAME_POOL_INIT_COUNT,  // int, <=0 means use platform default count
    };

    TMH264Decoder() {InitDefaultPropertyList();}
    virtual ~TMH264Decoder() {}

    // TMCodec interface
    virtual int Open(TMCodecParams &codecParam, TMPropertyList *propList = NULL) = 0;
    virtual int SetConfig(TMPropertyList &propList) = 0;
    virtual int GetConfig(TMPropertyList &propList) = 0;
    virtual int Start()                             = 0;
    virtual int Flush()                             = 0;
    virtual int Stop()                              = 0;
    virtual int Close()                             = 0;

    // TMFilterEntity interface
    virtual TMSrcPad *GetSrcPad(int padID = 0) = 0;
    virtual TMSinkPad *GetSinkPad(int padID = 0) = 0;

    // TMVideoDecoder interface
    virtual int  SendPacket(TMPacket &pkt, int timeout)      = 0;
    virtual int  RecvFrame(TMVideoFrame &frame, int timeout) = 0;

protected:
    TMPropertyList mDefaultPropertyList;
    TMPropertyList mCurrentPropertyList;

    virtual void InitDefaultPropertyList()
    {
        TMPropertyList *pList[2] = {&mDefaultPropertyList, &mCurrentPropertyList};
        for (uint32_t i = 0; i < ARRAY_SIZE(pList); i++)
        {
            pList[i]->Add(TMProperty((int)PropID::OUTPUT_FRAME_WIDTH, -1, "output frame width"));
            pList[i]->Add(TMProperty((int)PropID::OUTPUT_FRAME_HEIGHT,-1, "output frame height"));
            pList[i]->Add(TMProperty((int)PropID::OUTPUT_PIXEL_FORMAT, (int32_t)(TMImageInfo::PixelFormat::PIXEL_FORMAT_YUV420P), "output pixel format"));
            pList[i]->Add(TMProperty((int)PropID::FRAME_POOL_INIT_COUNT, -1, "frame pool init count"));
        }
    }
};

#endif  // TM_H264_DECODER_H

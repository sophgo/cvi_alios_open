/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */

#ifndef TM_JPEG_DECODER_H
#define TM_JPEG_DECODER_H

#include <string>

#ifdef __linux__
#include <tmedia_config.h>
#endif
#include <tmedia_core/common/media_info.h>

#include <tmedia_core/entity/codec/codec.h>
#include <tmedia_core/common/packet.h>
#include <tmedia_core/common/frame.h>

using namespace std;

class TMJpegDecoder : public TMVideoDecoder
{
public:
    enum class PropID : uint32_t
    {
        // ID                      Data Type
        OUTPUT_PIXEL_FORMAT,    // TMImageInfo::PixelFormat
    };

    TMJpegDecoder() {}
    virtual ~TMJpegDecoder() {}

    // TMCodec interface
    virtual int Open(TMCodecParams &codecParam, TMPropertyList *propList = NULL) = 0;
    virtual int SetConfig(TMPropertyList &propList) = 0;
    virtual int GetConfig(TMPropertyList &propList) = 0;
    virtual int Start() = 0;
    virtual int Flush() = 0;
    virtual int Stop() = 0;
    virtual int Close() = 0;

    // TMFilterEntity interface
    virtual TMSrcPad *GetSrcPad(int padID = 0) = 0;
    virtual TMSinkPad *GetSinkPad(int padID = 0) = 0;

    // TMVideoDecoder interface
    virtual int SendPacket(TMPacket &pkt, int timeout) = 0;
    virtual int RecvFrame(TMVideoFrame &frame, int timeout) = 0;
};

#endif  // TM_JPEG_DECODER_H

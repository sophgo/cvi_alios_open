/*
 * Copyright (C) 2018-2022 Alibaba Group Holding Limited
 */

#ifndef __TS_MUXER_SENO_H__
#define __TS_MUXER_SENO_H__

#include <tmedia_core/common/media_info.h>
#include <tmedia_core/entity/parser/parser.h>
#include <tmedia_core/entity/format/ts_muxer.h>
#include <mpegts/ts_muxer.hpp>

using namespace std;

typedef void (*muxer_cb_t)(void *user_data, const uint8_t *packet, size_t size);

class TMTsMuxerSeno final: public TMTsMuxer
{
public:
    TMTsMuxerSeno();
    virtual ~TMTsMuxerSeno();

    int  Open(TMPropertyList *propList = NULL);
    int  Close();
    int  SetConfig(TMPropertyList &propertyList);
    int  GetConfig(TMPropertyList &propertyList);
    int  Start();
    int  Stop();
    void SetMuxerCallback(mux_cb_t cb, void *user_data);
    int  AddStream(int sid, TMCodecParams &codecParam);
    int  WritePacket(int sid, TMPacket &packet);

private:
    TsMuxer*                  mMux;
    muxer_cb_t                mCallback;
    void*                     mUserData;
};

#endif /* __TS_MUXER_SENO_H__ */


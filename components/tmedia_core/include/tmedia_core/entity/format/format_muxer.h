/*
 * Copyright (C) 2021 Alibaba Group Holding Limited
 */

#ifndef TM_FORMAT_MUXER_H
#define TM_FORMAT_MUXER_H
#ifdef __linux__
#include <tmedia_config.h>
#endif
#include <tmedia_core/entity/codec/codec_params.h>
#include <tmedia_core/entity/format/format_info.h>
#include <tmedia_core/entity/format/format.h>
#include <tmedia_core/bind/bind_inc.h>
#include <tmedia_core/entity/entity.h>

using namespace std;

typedef void (*mux_cb_t)(void *user_data, const uint8_t *packet, size_t size);

class TMFormatMuxer : public TMFormat
{
public:
    TMFormatMuxer();
    virtual ~TMFormatMuxer();

    virtual int Open(TMPropertyList *propList = NULL)           = 0;
    virtual int Close()                                         = 0;

    virtual int SetConfig(TMPropertyList &propertyList)         = 0;
    virtual int GetConfig(TMPropertyList &propertyList)         = 0;

    virtual int Start()                                         = 0;
    virtual int Stop()                                          = 0;
    virtual void SetMuxerCallback(mux_cb_t cb, void *user_data) = 0;
    virtual int AddStream(int sid, TMCodecParams &codecParam)   = 0;
    virtual int WritePacket(int sid, TMPacket &packet)          = 0;

protected:
    virtual void InitDefaultPropertyList()                      = 0;
protected:
    TMMediaInfo::FormatID                mFormatID;
    TMPropertyList                       mDefaultPropertyList;
    TMPropertyList                       mCurrentPropertyList;
    map<int, shared_ptr<TMCodecParams>>  mStreamParams;
};

#endif  // TM_FORMAT_MUXER_H


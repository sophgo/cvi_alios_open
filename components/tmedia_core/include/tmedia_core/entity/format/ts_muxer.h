/*
 * Copyright (C) 2018-2022 Alibaba Group Holding Limited
 */

#ifndef __TS_MUXER_H__
#define __TS_MUXER_H__

#include <tmedia_core/common/media_info.h>
#include <tmedia_core/entity/parser/parser.h>
#include <tmedia_core/entity/format/format_muxer.h>

using namespace std;

class TMTsMuxer : public TMFormatMuxer
{
public:
    TMTsMuxer()
    {
        mFormatID = TMMediaInfo::FormatID::TS;
        InitDefaultPropertyList();
    }
    virtual ~TMTsMuxer() {}

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
    virtual void InitDefaultPropertyList()
    {
    }
};

#endif /* __TS_MUXER_H__ */


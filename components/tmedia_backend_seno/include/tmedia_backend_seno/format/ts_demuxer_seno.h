/*
 * Copyright (C) 2018-2022 Alibaba Group Holding Limited
 */

#ifndef __TS_DEMUXER_SENO_H__
#define __TS_DEMUXER_SENO_H__

#include <tmedia_core/common/common_inc.h>
#include <tmedia_core/entity/format/format_demuxer.h>
#include <tmedia_core/entity/format/format_info.h>
#include <tmedia_core/entity/format/ts_demuxer.h>
#include <mpegts/ts_demuxer.hpp>

using namespace std;

class TMTsDemuxerSeno final: public TMTsDemuxer
{
public:
    TMTsDemuxerSeno();
    ~TMTsDemuxerSeno();

    // TMSrcEntity interface
    TMSrcPad *GetSrcPad(int padID = 0);

    // TMFormatDemuxer interface
    int Open(string fileName, TMPropertyList *propList = NULL);
    int Close();
    int GetFormatInfo(TMFormatInfo &fmtInfo);
    int GetCodecParam(TMCodecParams &codecParams);

    int SetConfig(TMPropertyList &propertyList);
    int GetConfig(TMPropertyList &propertyList);

    int Start();
    int Stop();
    int Seek(int64_t timestamp);
    int ReadPacket(TMPacket &packet);

private:
    ifstream                  mIfile;
    TsPacket*                 mPacket;
    BytesIO*                  mIO;
    TsDemuxer*                mDemux;
};

#endif /* __TS_DEMUXER_SENO_H__ */


/*
 * Copyright (C) 2018-2022 Alibaba Group Holding Limited
 */

#ifndef __TS_DEMUXER_H__
#define __TS_DEMUXER_H__

#include <tmedia_core/common/common_inc.h>
#include <tmedia_core/entity/format/format_demuxer.h>
#include <tmedia_core/entity/format/format_info.h>

using namespace std;

class TMTsDemuxer : public TMFormatDemuxer
{
public:
    TMTsDemuxer()
    {
        mFormatID = TMMediaInfo::FormatID::TS;
        InitDefaultPropertyList();
    }
    virtual ~TMTsDemuxer() {}

    // TMSrcEntity interface
    virtual TMSrcPad *GetSrcPad(int padID = 0) = 0;

    // TMFormatDemuxer interface
    virtual int Open(string fileName, TMPropertyList *propList = NULL) = 0;
    virtual int Close()                                                = 0;
    virtual int GetFormatInfo(TMFormatInfo &fmtInfo)                   = 0;
    virtual int GetCodecParam(TMCodecParams &codecParams)              = 0;

    virtual int SetConfig(TMPropertyList &propertyList)                = 0;
    virtual int GetConfig(TMPropertyList &propertyList)                = 0;

    virtual int Start()                                                = 0;
    virtual int Stop()                                                 = 0;
    virtual int Seek(int64_t timestamp)                                = 0;
    virtual int ReadPacket(TMPacket &packet)                           = 0;

protected:
    virtual void InitDefaultPropertyList()
    {
    }
};




#endif /* __TS_DEMUXER_H__ */


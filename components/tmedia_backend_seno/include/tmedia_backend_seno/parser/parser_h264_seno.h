/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */
#ifndef TM_PARSER_H264_SENO_H
#define TM_PARSER_H264_SENO_H

#include <tmedia_core/entity/parser/parser_inc.h>

class TMParserH264Seno : public TMParser
{
public:
    TMParserH264Seno();
    ~TMParserH264Seno();

    int Parse(TMPacket &packet, const uint8_t *buf, size_t buf_size);

private:
    uint8_t *mRetainBuf;
    size_t mRetainBufSize;
    bool CompareByteArray(const uint8_t *byte_arr1, const uint8_t *byte_arr2, size_t size);

    TMMediaInfo::PictureType GetFrameType(const uint8_t *nalu_buf, size_t nalu_buf_size);
};

#endif  // TM_PARSER_H264_SENO_H

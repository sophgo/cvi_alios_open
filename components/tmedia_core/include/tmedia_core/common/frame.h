/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */

#ifndef TM_FRAME_H
#define TM_FRAME_H

#include <string>

#ifdef __linux__
#include <tmedia_config.h>
#endif
#include <tmedia_core/common/image_info.h>
#include <tmedia_core/common/audio_info.h>
#include <tmedia_core/common/media_info.h>
#include <tmedia_core/memory/buffer.h>
#include <tmedia_core/memory/buffer_pool.h>
#include <tmedia_core/memory/default_buffer_allocator.h>

using namespace std;

#define TMFRAME_MAX_PLANE 8



struct TMNativeFrameCtx;
typedef int (*TMNativeFrameCtxFreeFunc)(struct TMNativeFrameCtx *nativeFrameCtx);

struct TMNativeFrameCtx
{
public:
    static TMBuffer *CreateCtxBuffer();

public:
    int nativeId;
    TMNativeFrameCtxFreeFunc freeFunc;
    bool internalBuffer;
    void *priv;
};


class TMFrame
{
public:
    TMFrame();
    virtual ~TMFrame();
    virtual void Init();
    int SetBuffer(TMBuffer *buffer, int align = 0);
    TMBuffer *GetBuffer() const;
    int SetNativeFrameCtx(TMBuffer *buffer);
    TMBuffer *GetNativeFrameCtx() const;

    bool HasInternalBuffer();

    int Ref(const TMFrame *frame);
    int UnRef();
    int MoveRef(TMFrame *frame);

    virtual int CalcBufferSize(int align = 0) const = 0;
    virtual int PrepareBuffer(int align = 0, TMBufferType bufType = TMBUFFER_TYPE_USER_MALLOC, int flags = 0) = 0;

protected:
    virtual int CopyProperty(const TMFrame *frame) = 0;
    virtual int CopyData(const TMFrame *frame);
    virtual int CopyDataPointer(const TMFrame *frame);
    virtual int MapDataPointer(int align);
    virtual int UnmapDataPointer();

public:
    uint8_t *mData[TMFRAME_MAX_PLANE];
    uint32_t mPlanes;

protected:
    TMBuffer *mBuffer;
    TMBuffer *mNativeFrameCtx;
};

class TMVideoFrame;
typedef int (* TMVideoFrameReleaseFunc)(TMVideoFrame &videoFrame);


class TMVideoFrame : public TMFrame
{
public:
    TMVideoFrame();
    virtual ~TMVideoFrame();
    virtual void Init();
    int  RegisterReleaseFunc(TMVideoFrameReleaseFunc releaseFunc);

    int  Release();
    void Dump();

    virtual int CalcBufferSize(int align = 0) const;
    virtual int PrepareBuffer(int align = 0, TMBufferType bufType = TMBUFFER_TYPE_USER_MALLOC, int flags = 0);

protected:
    virtual int CopyProperty(const TMFrame *frame);
    virtual int CopyData(const TMFrame *frame);
    virtual int CopyDataPointer(const TMFrame *frame);
    virtual int MapDataPointer(int align);
    virtual int UnmapDataPointer();

public:
    TMImageInfo::PixelFormat mPixelFormat;

    int mStride[TMFRAME_MAX_PLANE];
    int mWidth;
    int mHeight;

    void *mPlatformPriv;

    uint64_t mPTS;

private:
    TMVideoFrameReleaseFunc mReleaseFunc;
};

class TMAudioFrame : public TMFrame
{
public:
    TMAudioFrame();
    virtual ~TMAudioFrame();
    virtual void Init();

    virtual int PrepareBuffer(int align = 0, TMBufferType bufType = TMBUFFER_TYPE_USER_MALLOC, int flags = 0);

protected:
    virtual int CopyProperty(const TMFrame *frame);
    virtual int CopyData(const TMFrame *frame);
    virtual int CopyDataPointer(const TMFrame *frame);
    virtual int CalcBufferSize(int align = 0) const;
    virtual int MapDataPointer(int align);
    virtual int UnmapDataPointer();

public:
    uint64_t mPTS;
    int mSampleRate;
    int mSampleCount;
    AudioSampleBits_e       mSampleBits;
    AudioSampleChannels_e   mSampleChannels;
    AudioPcmDataType_e      mPcmDataType;
    void *mPlatformPriv;
};

#endif  // TM_FRAME_H

/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */

#ifndef TM_UTIL_RINGBUFFER_H
#define TM_UTIL_RINGBUFFER_H

#include <iostream>

class TMUtilRingBuffer
{
public:
    TMUtilRingBuffer(int size)
    {
        mBuffer = (char *)malloc(size + 1);
        if(mBuffer == nullptr) {
            cout << "rb malloc fail" << endl;
        }
        mSize = size;
        mRidx = mWidx = 0;
    }
    virtual ~TMUtilRingBuffer() 
    {
        if(mBuffer) {
            free(mBuffer);
        }
    }
    int Read(char *buff, int size)
    {
        int RealSize = 0;
        if (size <= 0) {
            return -1;
        }
        if (IsEmpty()) {
            return 0;
        }
        /* get real read size */
        int buffer_size = AvaliableReadSpace();
        RealSize = std::min(size, buffer_size);
        /* cp data to user buffer */
        for (int i = 0; i < RealSize; i++) {
            buff[i] = mBuffer[mRidx];
            mRidx++;
            if (mRidx >= mSize + 1) {
                mRidx = 0;
            }
        }
        return RealSize;
    }
    int Write(char *buff, int size)
    {
        if (AvaliableWriteSpace() < size) {
            cout << "rb full" << endl;
            return -1;
        }

        for (int i = 0; i < size; i++) {
            mBuffer[mWidx] = buff[i];
            mWidx++;
            if (mWidx >= mSize + 1) {
                mWidx = 0;
            }
        }
        return size;
    }
    int Clear()
    {
        mRidx = mWidx = 0;
        return 0;
    }
    int AvaliableReadSpace()
    {
        if (mRidx == mWidx) {
            return 0;
        } else if (mRidx < mWidx) {
            return mWidx - mRidx;
        } else {
            return mSize - (mRidx - mWidx - 1);
        }
    }
    int AvaliableWriteSpace()
    {
        return (mSize - AvaliableReadSpace());
    }
    int IsEmpty()
    {
        return (AvaliableReadSpace() == 0);
    }
    int IsFull()
    {
        return (AvaliableWriteSpace() == 0);
    }
private:
    int mSize;
    int mRidx;
    int mWidx;
    char *mBuffer;
};

#endif  // TM_UTIL_RINGBUFFER_H


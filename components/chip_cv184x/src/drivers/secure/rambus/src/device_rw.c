/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */
#include "device_rw.h"
#include "soc.h"

uint32_t Device_Read32(const Device_Handle_t Device,
                       const unsigned int    ByteOffset)
{
        uint32_t v = *(uint32_t *)((uint64_t)Device + ByteOffset);

        return v;
}

void Device_Write32(const Device_Handle_t Device, const unsigned int ByteOffset,
                    const uint32_t Value)
{
        *(uint32_t *)((uint64_t)Device + ByteOffset) = Value;
}
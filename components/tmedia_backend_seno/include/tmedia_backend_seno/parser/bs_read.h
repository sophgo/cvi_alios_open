/*
 * Copyright (C) 2021-2022 Alibaba Group Holding Limited
 */
#ifndef BS_READ_H
#define BS_READ_H

#include <cstdint>

typedef struct
{
    const uint8_t *p_start;
    const uint8_t *p;
    const uint8_t *p_end;
    uint32_t i_left;
} bs_t;

void bs_init(bs_t *s, const uint8_t *p_data, uint32_t i_data);
uint32_t bs_read(bs_t *s, uint32_t i_count);
uint32_t bs_read1(bs_t *s);
int bs_read_ue(bs_t *s);
int bs_read_se(bs_t *s);

#endif  // BS_READ_H

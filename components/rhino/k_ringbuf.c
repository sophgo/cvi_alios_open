/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include "k_api.h"

#define RING_BUF_LEN sizeof(size_t)

kstat_t ringbuf_init(k_ringbuf_t *p_ringbuf, void *buf, size_t len, size_t type,
                     size_t block_size)
{
    p_ringbuf->type     = type;
    p_ringbuf->buf      = buf;
    p_ringbuf->end      = (uint8_t *)buf + len;
    p_ringbuf->blk_size = block_size;

    ringbuf_reset(p_ringbuf);

    return RHINO_SUCCESS;

}

kstat_t ringbuf_push(k_ringbuf_t *p_ringbuf, void *data, size_t len)
{
    size_t   len_bytes           = 0;
    size_t   split_len           = 0;
    uint8_t  c_len[RING_BUF_LEN] = {0};

    if (p_ringbuf->type == RINGBUF_TYPE_FIX) {

        if (p_ringbuf->freesize < p_ringbuf->blk_size) {
            return RHINO_RINGBUF_FULL;
        }

        if (p_ringbuf->tail == p_ringbuf->end) {
            p_ringbuf->tail = p_ringbuf->buf;
        }

        memcpy(p_ringbuf->tail, data, p_ringbuf->blk_size);
        p_ringbuf->tail     += p_ringbuf->blk_size;
        p_ringbuf->freesize -= p_ringbuf->blk_size;
    } else {
        if ((len == 0u) || (len >= (uint32_t) -1)) {
            return RHINO_INV_PARAM;
        }

        len_bytes = RING_BUF_LEN;

        /* for dynamic length ringbuf */
        if (p_ringbuf->freesize < (len_bytes + len)) {
            return RHINO_RINGBUF_FULL;
        }

        memcpy(c_len, &len, RING_BUF_LEN);

        if (p_ringbuf->tail == p_ringbuf->end) {
            p_ringbuf->tail = p_ringbuf->buf;
        }

        /* copy length data to buffer */
        split_len = p_ringbuf->end - p_ringbuf->tail;
        if (p_ringbuf->tail >= p_ringbuf->head &&
            split_len < len_bytes && split_len > 0) {
            memcpy(p_ringbuf->tail, &c_len[0], split_len);
            len_bytes -= split_len;
            p_ringbuf->tail =  p_ringbuf->buf;
            p_ringbuf->freesize -= split_len;
        } else {
            split_len = 0;
        }

        if (len_bytes > 0) {
            memcpy(p_ringbuf->tail, &c_len[split_len], len_bytes);
            p_ringbuf->freesize -= len_bytes;
            p_ringbuf->tail += len_bytes;
        }

        /* copy data to ringbuf, if break by buffer end, split data and copy to buffer head*/
        split_len = 0;

        if (p_ringbuf->tail == p_ringbuf->end) {
            p_ringbuf->tail = p_ringbuf->buf;
        }

        split_len = p_ringbuf->end - p_ringbuf->tail;
        if (p_ringbuf->tail >= p_ringbuf->head &&
            split_len < len && split_len > 0) {
            memcpy(p_ringbuf->tail, data, split_len);
            data = (uint8_t *)data + split_len;
            len -= split_len;
            p_ringbuf->tail =  p_ringbuf->buf;
            p_ringbuf->freesize -= split_len;
        }

        memcpy(p_ringbuf->tail, data, len);
        p_ringbuf->tail += len;
        p_ringbuf->freesize -= len;
    }

    return RHINO_SUCCESS;
}

kstat_t ringbuf_pop(k_ringbuf_t *p_ringbuf, void *pdata, size_t *plen)
{
    size_t   split_len           = 0;
    uint8_t *data                = pdata;
    size_t   len                 = 0;
    uint8_t  c_len[RING_BUF_LEN] = {0};
    size_t   len_bytes           = 0;

    if (p_ringbuf->type == RINGBUF_TYPE_FIX) {
        if (p_ringbuf->head == p_ringbuf->end) {
            p_ringbuf->head = p_ringbuf->buf;
        }

        memcpy(pdata, p_ringbuf->head, p_ringbuf->blk_size);
        p_ringbuf->head += p_ringbuf->blk_size;
        p_ringbuf->freesize += p_ringbuf->blk_size;

        if (plen != NULL) {
            *plen = p_ringbuf->blk_size;
        }

        return RHINO_SUCCESS;
    } else {

        len_bytes = RING_BUF_LEN;

        if (p_ringbuf->head == p_ringbuf->end) {
            p_ringbuf->head = p_ringbuf->buf;
        }
        split_len = p_ringbuf->end - p_ringbuf->head;
        if (split_len < len_bytes && split_len > 0) {
            memcpy(&c_len[0], p_ringbuf->head, split_len);
            p_ringbuf->head      =  p_ringbuf->buf;
            p_ringbuf->freesize += split_len;
        } else {
            split_len = 0;
        }

        if (len_bytes - split_len > 0) {
            memcpy(&c_len[split_len], p_ringbuf->head, (len_bytes - split_len));
            p_ringbuf->head     += (len_bytes - split_len);
            p_ringbuf->freesize += (len_bytes - split_len);
        }

        memcpy(&len, c_len, RING_BUF_LEN);
        *plen = len;

        if (p_ringbuf->head == p_ringbuf->end) {
            p_ringbuf->head = p_ringbuf->buf;
        }

        split_len = p_ringbuf->end - p_ringbuf->head;
        if (p_ringbuf->head > p_ringbuf->tail && split_len < len) {
            memcpy(pdata, p_ringbuf->head, split_len);
            data = (uint8_t *)pdata + split_len;
            len -= split_len;
            p_ringbuf->head      = p_ringbuf->buf;
            p_ringbuf->freesize += split_len;
        }

        memcpy(data, p_ringbuf->head, len);
        p_ringbuf->head     += len;
        p_ringbuf->freesize += len;

        return RHINO_SUCCESS;
    }
}

uint8_t ringbuf_is_full(k_ringbuf_t *p_ringbuf)
{
    if (p_ringbuf->type == RINGBUF_TYPE_DYN && p_ringbuf->freesize < (RING_BUF_LEN + 1)) {
        return 1;
    }

    if ((p_ringbuf->type == RINGBUF_TYPE_FIX) &&
        (p_ringbuf->freesize < p_ringbuf->blk_size)) {
        return 1;
    }

    return 0;
}

uint8_t ringbuf_is_empty(k_ringbuf_t *p_ringbuf)
{
    if (p_ringbuf->freesize == (size_t)(p_ringbuf->end - p_ringbuf->buf)) {
        return 1;
    }

    return 0;
}

kstat_t ringbuf_reset(k_ringbuf_t *p_ringbuf)
{
    p_ringbuf->head     = p_ringbuf->buf;
    p_ringbuf->tail     = p_ringbuf->buf;
    p_ringbuf->freesize = p_ringbuf->end - p_ringbuf->buf;

    return RHINO_SUCCESS;
}


/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: oal_netbuff.c.
 * Author: Hisilicon
 * Create: 2018-08-04
 */
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "oal_netbuf.h"
//#include "oam_ext_if.h"
#include "oal_util.h"
#include "oal_skbuff.h"
#include <aos/list.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


void skb_reserve(struct sk_buff *skb, int len)
{
	skb->data += len;
	skb->tail += len;
}

unsigned char *skb_tail_pointer(const struct sk_buff *skb)
{
	return skb->tail;
}

void skb_reset_tail_pointer(struct sk_buff *skb)
{
	skb->tail = skb->data;
}

void skb_set_tail_pointer(struct sk_buff *skb, const int offset)
{
	skb->tail = skb->data + offset;
}

unsigned char *skb_put(struct sk_buff *skb, unsigned int len)
{
    unsigned char *tmp = skb_tail_pointer(skb);
    skb->tail += len;
    skb->len  += len;
    if (unlikely(skb->tail > skb->end)) {
        printf("[error] func%s line %d\r\n",__func__,__LINE__);
        return NULL;
    };
    return tmp;
}

unsigned char *skb_push(struct sk_buff *skb, unsigned int len)
{
	skb->data -= len;
	skb->len  += len;
	if (unlikely(skb->data < skb->head)) {
        printf("[error] func%s line %d\r\n",__func__,__LINE__);
        return NULL;
    };
	return skb->data;
}

#ifndef USE_STATIC_SKB_BUF

#define ALIGN(x,a)                 (((x) + (a) - 1) & ~((a) - 1))
struct sk_buff *dev_alloc_skb(unsigned int len)
{
	hi_u8 *data;
    unsigned int size;
	struct sk_buff *skb;

	//len += NET_SKB_PAD;
    len = ALIGN(len, 32);
    
    skb = (struct sk_buff *)aos_malloc(sizeof(struct sk_buff));
    size = len;
    //printf("len %d\n", len);
    data = aos_malloc(len);
    if(data == HI_NULL) {
        printf("func %s malloc is error\r\n",__func__);
        aos_free(skb);
        return HI_NULL;
    }
    
	memset(data, 0, len);
	memset(skb, 0, offsetof(struct sk_buff, tail));
	skb->head = data;
	skb->data = data;
	skb_reset_tail_pointer(skb);
	skb->end = skb->tail + size; 
    //printf("good alloc skb %d\r\n", sizeof(struct sk_buff));
    //printf("data len %d\r\n", len);
    //printf("skb[0x%x] data[0x%x]  \r\n", skb, skb->data);
	return skb;
}
#else

#define MAX_SKB_BUF_NUM 40
#define ALIGN(x,a)                 (((x) + (a) - 1) & ~((a) - 1))

struct skb_data_pool_t {
    slist_t free_list;
    aos_mutex_t skb_mutex;
};

struct skb_data_pool_t skb_data_pool;

struct sk_buff *dev_alloc_skb(unsigned int len)
{
	// hi_u8 *data = HI_NULL;
    unsigned int size;
	struct sk_buff *skb;

	//len += NET_SKB_PAD;
    len = ALIGN(len, 32);
    size = len;
    //printf("%s, line %d, alloc size %d\r\n", __func__, __LINE__, size);
    
    aos_mutex_lock(&skb_data_pool.skb_mutex, AOS_WAIT_FOREVER);
    skb = slist_first_entry(&skb_data_pool.free_list, struct sk_buff, list);
    if (skb == HI_NULL) {
        aos_mutex_unlock(&skb_data_pool.skb_mutex);
        return skb;
    }
    slist_del(&skb->list, &skb_data_pool.free_list);
    aos_mutex_unlock(&skb_data_pool.skb_mutex);

	memset(skb, 0, offsetof(struct sk_buff, tail));
	skb->head = skb->data;

	skb_reset_tail_pointer(skb);
	skb->end = skb->tail + size; 
    //printf("good alloc skb %d\r\n", sizeof(struct sk_buff));
    //printf("data len %d\r\n", len);
    //printf("skb[0x%x] data[0x%x]  \r\n", skb, skb->data);
	return skb;
}

void dev_free_skb(struct sk_buff *skb)
{
    aos_mutex_lock(&skb_data_pool.skb_mutex, AOS_WAIT_FOREVER);
    slist_add_tail(&skb->list, &skb_data_pool.free_list);
    aos_mutex_unlock(&skb_data_pool.skb_mutex);
}

int dev_init_skb(void)
{
    hi_u8 i;
    int malloc_len = 1664;
    struct sk_buff *skb = NULL;

    aos_mutex_new(&skb_data_pool.skb_mutex);
    slist_init(&skb_data_pool.free_list);

    for (i = 0; i < MAX_SKB_BUF_NUM; i++) {
        skb = (struct sk_buff *)aos_zalloc(sizeof(struct sk_buff));
        skb->data = aos_zalloc(malloc_len);
        slist_add_tail(&skb->list, &skb_data_pool.free_list);
    }
    return 0;
}

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif



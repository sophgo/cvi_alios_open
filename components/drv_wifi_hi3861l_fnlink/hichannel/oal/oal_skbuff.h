#ifndef __OAL_SKBUFF_H__
#define __OAL_SKBUFF_H__

#define NET_SKB_PAD	64

unsigned char *skb_put(struct sk_buff *skb, unsigned int len);
unsigned char *skb_push(struct sk_buff *skb, unsigned int len);
struct sk_buff *dev_alloc_skb(unsigned int len);


#endif


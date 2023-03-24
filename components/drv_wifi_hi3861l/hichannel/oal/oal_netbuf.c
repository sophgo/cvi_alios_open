/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: oal_netbuff.c.
 * Author: Hisilicon
 * Create: 2018-08-04
 */
/*****************************************************************************
  1 $)AM7ND<~0|:,
*****************************************************************************/
#include "oal_netbuf.h"
//#include "oam_ext_if.h"
#include "oal_util.h"
#include "oal_skbuff.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

extern void skb_reserve(struct sk_buff *skb, int len);
extern void dev_free_skb(struct sk_buff *skb);
/*****************************************************************************
  3 $)A:/J}J5OV
*****************************************************************************/
#if 0
/*****************************************************************************
 $)A9&D\ChJv  : Ornetbu_head5DN22?4.=Snetbuf
*****************************************************************************/
hi_u32 oal_netbuf_concat(oal_netbuf_stru* netbuf_head, oal_netbuf_stru* netbuf)
{
     /* $)AEP6O?U<dJG7qWc9; */
    if (((hi_u32)netbuf_head->end - (hi_u32)netbuf_head->tail) < netbuf->len) {
        oal_netbuf_free(netbuf);
        oam_error_log3("oal_netbuf_concat::no enough space: end:%d, tail:%d, len:%d.",
            netbuf_head->end, netbuf_head->tail, netbuf->len);
        return HI_FAIL;
    }
    if (memcpy_s(skb_tail_pointer(netbuf_head), netbuf->len, netbuf->data, netbuf->len) != EOK) {
        oam_error_log0("oal_netbuf_concat:: memcpy_s failed");
        oal_netbuf_free(netbuf);
        return HI_FAIL;
    }

    skb_put(netbuf_head, netbuf->len);
    oal_netbuf_free(netbuf);
    return HI_SUCCESS;
}
#endif
/*****************************************************************************
 $)A9&D\ChJv  : ;qH!51G0netbufT*KX:s5D5Zn8vT*KX
 $)AJdHk2NJ}  : (1)FpJ<2iUR=Z5c
             (2)$)AOr:s2iUR5D8vJ}
 $)AJd3v2NJ}  : V8OrFZM{5Dnetbuf5DV8Uk
 $)A75 ;X V5  : FZM{5DbetbufT*KX5DV8Uk;r?UV8Uk
*****************************************************************************/
hi_u32  oal_netbuf_get_appointed_netbuf(const oal_netbuf_stru *netbuf, hi_u8 num, oal_netbuf_stru** expect_netbuf)
{
    hi_u8   buf_num;

    if (oal_unlikely((netbuf == HI_NULL) || (expect_netbuf == HI_NULL))) {
        return 100;  /* TBD: HI_ERR_CODE_PTR_NULL == 100 */
    }

    *expect_netbuf = HI_NULL;

    for (buf_num = 0; buf_num < num; buf_num++) {
        *expect_netbuf = oal_netbuf_next(netbuf);

        if (*expect_netbuf == HI_NULL) {
            break;
        }

        netbuf = *expect_netbuf;
    }

    return HI_SUCCESS;
}

/*****************************************************************************
 $)A9&D\ChJv  : EP6Oipv4 tcp1(NDJG7qN*tcp ack
*****************************************************************************/
hi_u8 oal_netbuf_is_tcp_ack(oal_ip_header_stru  *ip_hdr)
{
    oal_tcp_header_stru    *tcp_hdr = HI_NULL;
    hi_u32                 ip_pkt_len;
    hi_u32                 ip_hdr_len;
    hi_u32                 tcp_hdr_len;

    tcp_hdr     = (oal_tcp_header_stru *)(ip_hdr + 1);
    ip_pkt_len   = oal_net2host_short(ip_hdr->us_tot_len);
    ip_hdr_len   = (oal_low_half_byte(ip_hdr->us_ihl)) << 2;   /* 2: $)AWsRF2N; */
    tcp_hdr_len  = (oal_high_half_byte(tcp_hdr->offset)) << 2; /* 2: $)AWsRF2N; */
    if (tcp_hdr_len + ip_hdr_len == ip_pkt_len) {
        return HI_TRUE;
    }

    return HI_FALSE;
}

/*****************************************************************************
 $)A9&D\ChJv  : N*netbufIjGkDZ4f
 $)AJdHk2NJ}  : ul_size: 7VEdDZ4f5D4sP!
             l_reserve: data$)A8zV8UkM7V.<dR*T$At5D3$6H
             ul_align: $)APhR*<8WV=Z6TFk
 $)A75 ;X V5  : 3I9&75;X=a99LeV8Uk#;J'0\75;XOAL_PTR_NULL
*****************************************************************************/
oal_netbuf_stru* oal_netbuf_alloc(hi_u32 ul_size, hi_u32 l_reserve, hi_u32 l_align)
{
    oal_netbuf_stru *pst_netbuf = HI_NULL;
    hi_u32       ul_offset;

    /* $)A1#V$data2?7V5Dsize2;;aTYWV=Z6TFk:sP!SZT$OHOk7VEd5D4sP! */
    if (l_align) {
        ul_size += (l_align - 1);
    }

    // printf("ul_size 0x%x\r\n",ul_size);
    if (NET_SKB_PAD < 64) {                         /* net skb pad less than 64 */
        pst_netbuf = dev_alloc_skb(ul_size + 32);   /* $)AT$At32WV=Z8xM72?J9FdBzWcPhGs */
    } else {
        pst_netbuf = dev_alloc_skb(ul_size);
    }

    if (oal_unlikely(pst_netbuf == HI_NULL)) {
        return HI_NULL;
    }

    if (NET_SKB_PAD < 64) {                         /* net skb pad less than 64 */
        skb_reserve(pst_netbuf, l_reserve + 32);    /* $)AT$At32WV=Z8xM72?J9FdBzWcPhGs */
    } else {
        skb_reserve(pst_netbuf, l_reserve);
    }
    // printf("pst_netbuf 0x%x,tail 0x%x,end 0x%x,head 0x%x data 0x%x\r\n",pst_netbuf,pst_netbuf->tail,pst_netbuf->end,pst_netbuf->head,pst_netbuf->data);
    // printf("len 0x%x,data_len 0x%x,tot_len 0x%x\r\n",pst_netbuf->len,pst_netbuf->data_len,pst_netbuf->tot_len);
    
    if (l_align) {
        /* $)A<FKcN*AKD\J94WV=Z6TFk5DF+RFA? */
        ul_offset = (hi_u32)(((unsigned long)pst_netbuf->data) % (unsigned long)l_align);
        
        //printf("pst_netbuf->data 0x%x ul_offset 0x%x l_align 0x%x\r\n",pst_netbuf->data, ul_offset,l_align);
        if (ul_offset) {
            skb_reserve(pst_netbuf, l_align - ul_offset);
        }
    }
    //printf("wdp netbuf alloc\r\n");
    return pst_netbuf;
}

/*****************************************************************************
 $)A9&D\ChJv  : TZ;:3eGxN22?Tv<SJ}>]
 $)AJdHk2NJ}  : pst_netbuf: ;:3eGx=a99LeV8Uk
             ul_len: $)APhR*Tv<SJ}>]5D3$6H
*****************************************************************************/

hi_u8* oal_netbuf_put(oal_netbuf_stru* netbuf, hi_u32 len)
{
    return skb_put(netbuf, len);
}

/*****************************************************************************
 $)A9&D\ChJv  : TZ;:3eGx?*M7Tv<SJ}>]
*****************************************************************************/
hi_u8*  oal_netbuf_push(oal_netbuf_stru* netbuf, hi_u32 len)
{
    return skb_push(netbuf, len);
}

/*****************************************************************************
 $)A9&D\ChJv  : 4SskbM72?H!3vJ}>]
 $)AJdHk2NJ}  : pst_netbuf: skb=a99LeV8Uk
*****************************************************************************/
hi_u8* oal_netbuf_pull(oal_netbuf_stru* netbuf, hi_u32 len)
{
    if (len > netbuf->len) {
        return HI_NULL;
    }

    netbuf->len -= len;

    return (netbuf->data += len);
}

hi_void oal_set_netbuf_prev(oal_netbuf_stru* netbuf, oal_netbuf_stru* prev)
{
    netbuf->prev = prev;
}


/*****************************************************************************
 $)A:/ J} C{  : oal_netbuf_reserve
 $)A9&D\ChJv  : =+1(ND=a99Le5DdataV8Uk:MtailV8UkM,J1OBRF
 $)AJdHk2NJ}  : pst_netbuf1(ND=a99LeV8Uk
             len: $)AT$At3$6H
*****************************************************************************/
hi_void  oal_netbuf_reserve(oal_netbuf_stru* netbuf, hi_u32 l_len)
{
    skb_reserve(netbuf, l_len);
}

/*****************************************************************************
 $)A9&D\ChJv  : JM7E1(ND=a99LeDZ4f?U<d, 7GS2VP6O;7>3OBJ9SC
 $)AJdHk2NJ}  : pst_netbuf: 1(ND=a99LeV8Uk
 $)A75 ;X V5  : 3I9&75;XHI_SUCCESS#;J'0\75;XHI_ERR_CODE_PTR_NULL
*****************************************************************************/
hi_u32  oal_netbuf_free(oal_netbuf_stru* netbuf)
{
#ifndef USE_STATIC_SKB_BUF    
    oal_free(netbuf);
#else
    dev_free_skb(netbuf);
#endif    
    return HI_SUCCESS;
}


/*****************************************************************************
 $)A9&D\ChJv  : ;qH!M72??U<d4sP!
*****************************************************************************/
hi_u32  oal_netbuf_headroom(const oal_netbuf_stru* netbuf)
{
    // printf("[addr]data 0x%x head 0x%x\r\n",netbuf->data,netbuf->head);
    return netbuf->data - netbuf->head;
}


/*****************************************************************************
 $)A9&D\ChJv  : ;qH!N22??U<d4sP!
*****************************************************************************/
hi_u32  oal_netbuf_tailroom(const oal_netbuf_stru* netbuf)
{
	return netbuf->data_len ? 0 : netbuf->end - netbuf->tail;
}

/*****************************************************************************
 $)A9&D\ChJv  : =+skb<SHkskbA41mVP
 $)AJdHk2NJ}  : pst_new: R*2eHk5DPBskbV8Uk
             pst_prev: $)AN2=Z5c
             pst_head: skb$)AA41mM7V8Uk
*****************************************************************************/
hi_void  oal_netbuf_add_to_list(oal_netbuf_stru* netbuf, oal_netbuf_stru* prev, oal_netbuf_stru* next)
{
    netbuf->next   = next;
    netbuf->prev   = prev;
    next->prev  = netbuf;
    prev->next  = netbuf;
}

/*****************************************************************************
 $)A9&D\ChJv  : 75;XA41mVPV86(=Z5c5DOBR;8v=Z5c
*****************************************************************************/
oal_netbuf_stru* oal_netbuf_list_next(const oal_netbuf_stru* netbuf)
{
    return netbuf->next;
}

/*****************************************************************************
 $)A9&D\ChJv  : remove skb from list tail
 $)AJdHk2NJ}  : @head: the place to add it in the first list
 $)AJd3v2NJ}  : The list at @list is reinitialised
 $)A75 ;X V5  : the netbuf removed from the list
*****************************************************************************/
oal_netbuf_stru* oal_netbuf_delist_tail(oal_netbuf_head_stru *head)
{
    printf("warning: need to delist tail\r\n");
    return (oal_netbuf_stru*)head;
    //return skb_dequeue_tail(head);
}


/*****************************************************************************
 $)A9&D\ChJv  : ;qH!netbufK+OrA41mVPbuf5D8vJ}
*****************************************************************************/
hi_u32  oal_netbuf_get_buf_num(const oal_netbuf_head_stru *netbuf_head)
{
    return netbuf_head->qlen;
}

/*****************************************************************************
 $)A9&D\ChJv  : =+skbVP5DDZH]OHF+RFul_offset:s 04V86(3$6H?=145=V86(DZ4SVP
*****************************************************************************/
hi_u32 oal_netbuf_copydata(const oal_netbuf_stru *netbuf, hi_u32 offset, hi_void* dst,
                                         hi_u32 dst_len, hi_u32 len)
{
    hi_void* scr = HI_NULL;
    scr = oal_netbuf_data(netbuf) + offset;

    if (memcpy_s(dst, dst_len, scr, len) != EOK) {
        return HI_FAIL;
    }

    return HI_SUCCESS;
}

/*****************************************************************************
$)A9&D\ChJv: @)U9netbuf M72??U<d
*****************************************************************************/
hi_s32 oal_netbuf_expand_head(oal_netbuf_stru *netbuf, hi_s32 nhead,
                                            hi_s32 ntail, hi_s32 gfp_mask)
{
    //return pskb_expand_head(netbuf, nhead, ntail, gfp_mask);
    printf("warning: need to expand head\r\n");
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


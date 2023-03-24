/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: Header file for oal_netbuff.c.
 * Author: Hisilicon
 * Create: 2020-09-11
 */

#ifndef __OAL_NETBUFF_H__
#define __OAL_NETBUFF_H__

/* ͷ�ļ����� */
#include "securec.h"

#include "hi_types_base.h"
#include "oal_mm.h"
#include "oal_util.h"
#include <aos/list.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* �궨�� */
#define ETH_P_CONTROL                       0x0016      /* Card specific control frames */

#define oal_netbuf_list_num(_pst_head)              ((_pst_head)->qlen)
#define oal_net_dev_priv(_pst_dev)                  ((_pst_dev)->ml_priv)
#define oal_net_dev_wireless_dev(_pst_dev)          ((_pst_dev)->ieee80211_ptr)
#define oal_netbuf_next(_pst_buf)                   ((_pst_buf)->next)
#define oal_netbuf_prev(_pst_buf)                   ((_pst_buf)->prev)
#define oal_netbuf_head_next(_pst_buf_head)         ((_pst_buf_head)->next)
#define oal_netbuf_head_prev(_pst_buf_head)         ((_pst_buf_head)->prev)

#define oal_netbuf_protocol(_pst_buf)               ((_pst_buf)->protocol)
#define oal_netbuf_last_rx(_pst_buf)                ((_pst_buf)->last_rx)
#define oal_netbuf_data(_pst_buf)                   ((_pst_buf)->data)
#define oal_netbuf_header(_pst_buf)                 ((_pst_buf)->data)
#define oal_netbuf_payload(_pst_buf)                 ((_pst_buf)->data)

#define oal_netbuf_cb(_pst_buf)                     ((_pst_buf)->cb)
#define oal_netbuf_cb_size()                        (sizeof(((oal_netbuf_stru*)0)->cb))
#define oal_netbuf_len(_pst_buf)                    ((_pst_buf)->len)
#define oal_netbuf_priority(_pst_buf)               ((_pst_buf)->priority)
#define OAL_NETBUF_TAIL                              skb_tail_pointer
#define OAL_NETBUF_QUEUE_TAIL                        skb_queue_tail
#define OAL_NETBUF_QUEUE_HEAD_INIT                   skb_queue_head_init
#define OAL_NETBUF_DEQUEUE                           skb_dequeue

/* �����������ֽ���ת�� */
#ifdef HAVE_PCLINT_CHECK
#define oal_host2net_short(_x)      _x
#define oal_net2host_short(_x)      _x
#define oal_host2net_long(_x)       _x
#define oal_net2host_long(_x)       _x
#define oal_high_half_byte(a)       a
#define oal_low_half_byte(a)        a
#else
#define oal_host2net_short(_x)      hi_swap_byteorder_16(_x)
#define oal_net2host_short(_x)      hi_swap_byteorder_16(_x)
#define oal_host2net_long(_x)       hi_swap_byteorder_32(_x)
#define oal_net2host_long(_x)       hi_swap_byteorder_32(_x)
#define oal_high_half_byte(a)       (((a) & 0xF0) >> 4)
#define oal_low_half_byte(a)        ((a) & 0x0F)
#endif

typedef hi_u8 oal_mem_state_enum_uint8;
typedef unsigned char *sk_buff_data_t;

struct sk_buff {
  /** next pbuf in singly linked pbuf chain */
  union {
      struct {
          /* These two members must be first. */
          struct sk_buff      *next;
          struct sk_buff      *prev;
      };
      //struct list_head    list;
  };

  slist_t list;

  /** pointer to the actual data in the buffer */
  unsigned int   len,data_len;

  /**
   * total length of this buffer and all next buffers in chain
   * belonging to the same packet.
   *
   * For non-queue packet chains this is the invariant:
   * p->tot_len == p->len + (p->next? p->next->tot_len: 0)
   */
  unsigned int tot_len;
  sk_buff_data_t		tail;
  sk_buff_data_t		end;
  unsigned char		*head, *data;
#ifdef USE_STATIC_SKB_BUF  
  unsigned char skb_idx;
#endif
};

struct sk_buff_head {
	/* These two members must be first. */
	struct sk_buff	*next;
	struct sk_buff	*prev;

	hi_u32		qlen;
};

/* �ṹ�嶨�� */
typedef struct sk_buff                      oal_netbuf_stru;
typedef struct sk_buff_head                 oal_netbuf_head_stru;

typedef struct oal_netbuf_stru_tag {
    struct oal_netbuf_stru_tag*  next;
    oal_mem_state_enum_uint8    mem_state_flag;             /* �ڴ��״̬ */
    hi_u8                       subpool_id        :  4;     /* ��¼�����ӳ�id */
    hi_u8                       is_high_priority  :  1;
    hi_u8                       bit_resv          :  3;
    hi_u16                  us_index;
} oal_dev_netbuf_stru;

typedef struct {
    oal_dev_netbuf_stru*   next;
    oal_dev_netbuf_stru*   prev;
    hi_u32                 num;
} oal_dev_netbuf_head_stru;

typedef struct oal_ip_header {
    hi_u8    us_ihl: 4,
             version_ihl: 4;
    /* liuming modifed proxyst end */
    hi_u8    tos;
    hi_u16   us_tot_len;
    hi_u16   us_id;
    hi_u16   us_frag_off;
    hi_u8    ttl;
    hi_u8    protocol;
    hi_u16   us_check;
    hi_u32   saddr;
    hi_u32   daddr;
    /* The options start here */
} oal_ip_header_stru;

typedef struct oal_tcp_header {
    hi_u16  us_sport;
    hi_u16  us_dport;
    hi_u32  seqnum;
    hi_u32  acknum;
    hi_u8   offset;
    hi_u8   flags;
    hi_u16  us_window;
    hi_u16  us_check;
    hi_u16  us_urgent;
} oal_tcp_header_stru;

typedef struct {
    hi_void*  pointer;        /* Pointer to the data  (in user space) */
    hi_u16    length;         /* number of fields or size in bytes */
    hi_u16    flags;          /* Optional params */
} oal_iw_point_stru;

/*****************************************************************************
 ��������  : �ڻ�����β����������
 �������  : pst_netbuf: �������ṹ��ָ��
             ul_len: ��Ҫ�������ݵĳ���
*****************************************************************************/

hi_u8* oal_netbuf_put(oal_netbuf_stru* netbuf, hi_u32 len);

/*****************************************************************************
 ��������  : �ڻ�������ͷ��������
*****************************************************************************/
hi_u8*  oal_netbuf_push(oal_netbuf_stru* netbuf, hi_u32 len);

/*****************************************************************************
 ��������  : ��skbͷ��ȡ������
 �������  : pst_netbuf: skb�ṹ��ָ��
*****************************************************************************/
hi_u8* oal_netbuf_pull(oal_netbuf_stru* netbuf, hi_u32 len);
hi_void oal_set_netbuf_prev(oal_netbuf_stru* netbuf, oal_netbuf_stru* prev);


/*****************************************************************************
 �� �� ��  : oal_netbuf_reserve
 ��������  : �����Ľṹ���dataָ���tailָ��ͬʱ����
 �������  : pst_netbuf���Ľṹ��ָ��
             len: Ԥ������
*****************************************************************************/
hi_void  oal_netbuf_reserve(oal_netbuf_stru* netbuf, hi_u32 l_len);

/*****************************************************************************
 ��������  : �ͷű��Ľṹ���ڴ�ռ�, ��Ӳ�жϻ�����ʹ��
 �������  : pst_netbuf: ���Ľṹ��ָ��
 �� �� ֵ  : �ɹ�����HI_SUCCESS��ʧ�ܷ���HI_ERR_CODE_PTR_NULL
*****************************************************************************/
hi_u32  oal_netbuf_free(oal_netbuf_stru* netbuf);

/*****************************************************************************
 ��������  : ��ȡͷ���ռ��С
*****************************************************************************/
hi_u32  oal_netbuf_headroom(const oal_netbuf_stru* netbuf);

/*****************************************************************************
 ��������  : ��ȡβ���ռ��С
*****************************************************************************/
hi_u32  oal_netbuf_tailroom(const oal_netbuf_stru* netbuf);

/*****************************************************************************
 ��������  : ��skb����skb������
 �������  : pst_new: Ҫ�������skbָ��
             pst_prev: β�ڵ�
             pst_head: skb����ͷָ��
*****************************************************************************/
hi_void  oal_netbuf_add_to_list(oal_netbuf_stru* netbuf, oal_netbuf_stru* prev, oal_netbuf_stru* next);

/*****************************************************************************
 ��������  : ����������ָ���ڵ����һ���ڵ�
*****************************************************************************/
oal_netbuf_stru* oal_netbuf_list_next(const oal_netbuf_stru* netbuf);

/*****************************************************************************
 ��������  : remove skb from list tail
 �������  : @head: the place to add it in the first list
 �������  : The list at @list is reinitialised
 �� �� ֵ  : the netbuf removed from the list
*****************************************************************************/
oal_netbuf_stru* oal_netbuf_delist_tail(oal_netbuf_head_stru *head);

/*****************************************************************************
 ��������  : ��ȡnetbuf˫��������buf�ĸ���
*****************************************************************************/
hi_u32  oal_netbuf_get_buf_num(const oal_netbuf_head_stru *netbuf_head);

/*****************************************************************************
 ��������  : ��skb�е�������ƫ��ul_offset�� ��ָ�����ȿ�����ָ���ڴ���
*****************************************************************************/
hi_u32 oal_netbuf_copydata(const oal_netbuf_stru *netbuf, hi_u32 offset, hi_void* dst,
                                         hi_u32 dst_len, hi_u32 len);

/*****************************************************************************
��������: ��չnetbuf ͷ���ռ�
*****************************************************************************/
hi_s32 oal_netbuf_expand_head(oal_netbuf_stru *netbuf, hi_s32 nhead,
                                            hi_s32 ntail, hi_s32 gfp_mask);

/* �������� */
oal_netbuf_stru* oal_netbuf_alloc(hi_u32 ul_size, hi_u32 l_reserve, hi_u32 l_align);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

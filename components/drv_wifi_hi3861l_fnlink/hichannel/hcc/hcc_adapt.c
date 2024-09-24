/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: hcc layer frw task.
 * Author: dujinxin
 * Create: 2020-09-28
 */
/* $)AM7ND<~0|:, */
#include "oal_netbuf.h"
#include "hcc_host.h"
#include "oal_mm.h"
#include "oam_ext_if.h"
#include "wal_net.h"
//#include "wal_netlink.h"
#include "hcc_adapt.h"
#include "hcc_list.h"
#include "hi_types_base.h"
#include "hi3861l_devops.h"
#include "oal_skbuff.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#define  MAX_TEST_PACKAGE_LEN         1500
#define oal_round_up(_old_len, _align)   ((((_old_len) + ((_align) - 1)) / (_align)) * (_align))
typedef unsigned int    atomic_t;

static hi_char g_test_buf[MAX_TEST_PACKAGE_LEN] = {0};
// static atomic_t g_tx_ref = {0};
static atomic_t g_tx_num = {0};
//hi_void hcc_adapt_mem_free(hi_void *data);
static hi_void hcc_rx_test_speed(hcc_header_stru *hcc_hdr, hi_char* buf, int len);

hcc_unc_struc* hcc_structure_conversion(const hi_void* priv, hcc_stru_type type)
{
    oal_netbuf_stru *netbuf = HI_NULL;
    hcc_normal_struc* p_normal = HI_NULL;
    hcc_unc_struc *unc_buf = oal_memalloc(sizeof(hcc_unc_struc));
    if (unc_buf == HI_NULL) {
        return HI_NULL;
    }
    //printf("hcc_structure_conversion alloc %d\r\n", sizeof(hcc_unc_struc));
    //printf("unc_buf[0x%x]\r\n", unc_buf);
    memset_s(unc_buf, sizeof(hcc_unc_struc), 0, sizeof(hcc_unc_struc));

    switch (type) {
        case NETBUF_STRU_TYPE:
            netbuf = (oal_netbuf_stru *)priv;
            unc_buf->buf = (hi_char *)oal_netbuf_data(netbuf);
            unc_buf->length = oal_netbuf_len(netbuf);
            unc_buf->priv = (hi_void *)priv;
            unc_buf->priv_type = NETBUF_STRU_TYPE;
            unc_buf->msg_num = g_tx_num++;
            unc_buf->free = hcc_adapt_mem_free;
            break;
        case NORMAL_STRU_TYPE:
            p_normal = (hcc_normal_struc *)priv;
            unc_buf->buf = p_normal->buf;
            unc_buf->length = p_normal->length;
            unc_buf->priv = NULL;
            unc_buf->priv_type = NORMAL_STRU_TYPE;
            unc_buf->msg_num = g_tx_num++;
            unc_buf->free = hcc_adapt_mem_free;
            break;
        default:
            oal_free(unc_buf);
            unc_buf = HI_NULL;
            break;
    }
    return unc_buf;
}

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/ethip6.h"
#include "lwip/etharp.h"
#include "netif/ppp/pppoe.h"
#include "k_api.h"
//#include "net_cmds.h"

extern struct pbuf * wal_rx_data_proc(oal_netbuf_stru *netbuf);
extern struct netif hi3861l_netif[2];
static hi_void hcc_adapt_netbuf_rx_data_process(oal_netbuf_stru *netbuf, hcc_header_stru *hcc_hdr)
{
    oal_netbuf_next(netbuf) = HI_NULL;
    oal_netbuf_prev(netbuf) = HI_NULL;
    oal_netbuf_pull(netbuf, HCC_HDR_TOTAL_LEN + hcc_hdr->pad_align);
    oal_netbuf_len(netbuf) = hcc_hdr->pay_len;
    //printf("**success sub_type %d main_type %d pad_hdr %d **\r\n",hcc_hdr->sub_type,hcc_hdr->main_type,hcc_hdr->pad_hdr);
    //printf("**success pad_payload %d more %d seq %d pay_len %d**\r\n",hcc_hdr->pad_payload,hcc_hdr->more,hcc_hdr->seq,hcc_hdr->pay_len);
    
    //dump(netbuf->data,hcc_hdr->pay_len);
    //printf("******head hdr*****pay_len 0x%x\r\n",hcc_hdr->pay_len);
    //dump(hcc_hdr,sizeof(hcc_header_stru));
    struct netif *netif;
    netif = &hi3861l_netif[0];
    struct pbuf *p;
    switch (hcc_hdr->main_type) {
        case HCC_TYPE_DATA:
            // printf("***hcc type data\r\n");
            //break;
            //$)A4K4&0QnetbufPEO"V1=S4+5]8xlwip2c
            // if (wal_rx_data_proc(netbuf) != HI_SUCCESS) {
            //     oam_error_log0("hcc_rx_data_process:: wal_rx_data_proc failed");
            // }

            /* move received packet into a new pbuf */
            p = wal_rx_data_proc(netbuf);
            /* if no packet could be read, silently ignore this */
            if (p != NULL) {
                /* pass all packets to ethernet_input, which decides what packets it supports */
                if (netif->input(p, netif) != ERR_OK) {
                LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
                pbuf_free(p);
                p = NULL;
                }
            }
            oal_netbuf_push(netbuf, HCC_HDR_TOTAL_LEN + hcc_hdr->pad_align);
            //oal_free(netbuf->data);
            //printf("free netbuf->data[0x%x] netbuf[0x%x] unc_buf[0x%x]\r\n", netbuf->data, netbuf, unc_buf);
            //oal_netbuf_free(netbuf);
            break;
        case HCC_TYPE_TEST_XFER:
            hcc_rx_test_speed(hcc_hdr, (hi_char *)oal_netbuf_data(netbuf), hcc_hdr->pay_len);
            //oal_netbuf_free(netbuf);
            break;
        case HCC_TYPE_MSG:
            hi3861l_msg_rx(oal_netbuf_data(netbuf), oal_netbuf_len(netbuf));
            //oal_netbuf_free(netbuf);
            break;
        default:
            oam_error_log0("hcc_rx_data_process:: unknown main type.\n");
            //oal_netbuf_free(netbuf);
            break;
    }
}

static hi_u32 hcc_host_check_hdr(hcc_header_stru *hcc_hdr)
{
    if (hcc_hdr->main_type >= HCC_TYPE_BUFF) {
        return HI_FALSE;
    }
    return HI_TRUE;
}

/*****************************************************************************
 $)A9&D\ChJv  : 4&@m4Sdevice=SJU5DO{O"
 $)AJdHk2NJ}  : unc_buf M(SCbuf,4f7E8wVVK=SPJ}>]=a99#,8y>]2;M,5DJ}>]=a995wSC2;
             $)AM,5D4&@m:/J}
 $)AW"RbJBOn  : RT:sR*@)U9FdK{@`PM#,V;Phcase7VV'VPPBTv4&@m:/J}
 $)A75 ;X V5  : HI_SUCCESS O{O"4&@m3I9&#, HI_FAIL O{O"4&@mJ'0\#,O{O"M7Rl3#
*****************************************************************************/
static hi_u32 hcc_adapt_rx_data_process(hcc_unc_struc *unc_buf)
{
    oal_netbuf_stru *netbuf = HI_NULL;
    hcc_header_stru *hcc_hdr = (hcc_header_stru *)oal_unc_data(unc_buf);
    if (hcc_host_check_hdr(hcc_hdr) != HI_TRUE) {
        unc_buf->free(unc_buf);
        return HI_FAIL;
    }

    switch (oal_unc_priv_type(unc_buf)) {
        case NETBUF_STRU_TYPE:
            netbuf = (oal_netbuf_stru *)oal_unc_priv(unc_buf);
#ifndef USE_STATIC_SKB_BUF             
            unsigned char* tmp_data = netbuf->data;
#endif            
            //printf("free netbuf->data[0x%x] netbuf[0x%x] unc_buf[0x%x]\r\n", netbuf->data, netbuf, unc_buf);
            hcc_adapt_netbuf_rx_data_process(netbuf, hcc_hdr);
            //debug_mm_overview(NULL);
#ifndef USE_STATIC_SKB_BUF            
            oal_free(tmp_data);
#endif
            oal_netbuf_free(netbuf);
            oal_free(unc_buf);
            //debug_mm_overview(NULL);
            break;
        default:
            unc_buf->free(unc_buf);
            break;
    }
    return HI_SUCCESS;
}

hi_u32 hcc_tx_hcc_hdr_init(hcc_unc_struc *unc_buf, const hcc_transfer_param *param)
{
    hcc_header_stru *hcc_hdr = HI_NULL;
    hi_u32 payload_len;

    /* calculate the pad lengh to ensure the hcc_total_len is 64Bytes */
    payload_len = oal_unc_len(unc_buf) - HCC_HDR_TOTAL_LEN;

    // printf("hcc_header_stru 0x%x\r\n",sizeof(hcc_header_stru));
    hcc_hdr = (hcc_header_stru *)oal_unc_data(unc_buf);
    if (hcc_hdr == HI_NULL) {
        return HI_FAIL;
    }

    hcc_hdr->main_type = param->main_type;
    hcc_hdr->sub_type = param->sub_type;
    hcc_hdr->pay_len = payload_len;
    hcc_hdr->pad_hdr = HCC_HDR_RESERVED_MAX_LEN - param->extend_len;
    hcc_hdr->pad_align = 0;   /* Device alloc netbuf's payload all 4B aligned! */

    return HI_SUCCESS;
}

static hi_u32 hcc_host_tx(hcc_handler_stru *hcc_handler, oal_netbuf_stru *netbuf, const hcc_transfer_param *param)
{
    hi_u32 ret;
    hcc_trans_queue_stru *hcc_queue = HI_NULL;
    hcc_unc_struc *unc_buf = HI_NULL;
    hcc_queue = &hcc_handler->hcc_transer_info.hcc_queues[HCC_TX].queues[param->queue_id];
    unc_buf = hcc_structure_conversion(netbuf, NETBUF_STRU_TYPE);
    if (unc_buf == HI_NULL) {
        printf("[error]:func %s line %d\r\n",__func__,__LINE__);
        //oal_netbuf_free(netbuf);
        //oal_free(unc_buf);
        return HI_FAIL;
    }
    /* 1. build hcc header */
    ret = hcc_tx_hcc_hdr_init(unc_buf, param);
    if (ret != HI_SUCCESS) {
        printf("[error]:func %s line %d\r\n",__func__,__LINE__);
        //oal_netbuf_free(netbuf);
        oal_free(unc_buf);
        return HI_FAIL;
    }

    /* stop tcpip tx queue */
    if ((hcc_handler->hcc_bus_ops != HI_NULL) &&
        (hcc_handler->hcc_bus_ops->stop_tcpip_tx_queue != HI_NULL)) {
        hcc_handler->hcc_bus_ops->stop_tcpip_tx_queue(hcc_queue);
    }

    /* $)AW"2a;X5w=S?Z#,6*0|2YWw */
    if (hcc_handler->hcc_bus_ops != HI_NULL &&
        hcc_handler->hcc_bus_ops->tx_discard_key_frame) {
        ret = hcc_handler->hcc_bus_ops->tx_discard_key_frame(param->queue_id, hcc_queue, unc_buf);
        if (ret == HI_SUCCESS) {
            
            printf("func %s line %d\r\n",__func__,__LINE__);
            oal_netbuf_free((oal_netbuf_stru*)unc_buf->priv);
            oal_free(unc_buf);
            return HI_SUCCESS;
        }
    }

    //printf("****origin write start hcc host tx*****\n");
    //dump(unc_buf->buf , unc_buf->length);
    //printf("****origin write end hcc host tx*****\n");

    /* $)AW"2a;X5w=S?Z#,4&@m9X<|V!EEPr,?IRT=+9X<|V!2eHk6SAPG0Cf */
    if (hcc_handler->hcc_bus_ops != HI_NULL &&
        hcc_handler->hcc_bus_ops->tx_sort_key_frame) {
        ret = hcc_handler->hcc_bus_ops->tx_sort_key_frame(param->queue_id, hcc_queue, unc_buf);
        if (ret != HI_SUCCESS) {
            hcc_list_add_tail(&hcc_queue->queue_info, unc_buf);
            //printf("test-003\n");
        }
    } else {
        hcc_list_add_tail(&hcc_queue->queue_info, unc_buf);
        //printf("test-002\n");
    }

    /* 4. sched hcc tx */
    //printf("wdp-001\r\n");
    hcc_sched_transfer(hcc_handler);
    return HI_SUCCESS;
}

/* SDIO $)A4+JdMxBgJ}>]JJEdT$4&@m */
hi_u32 hcc_tx_data_adapt(oal_netbuf_stru *netbuf, hcc_type_enum type, hi_u32 sub_type)
{
    hi_u32 ret;
    hcc_transfer_param param;
    hcc_queue_type_enum queue_id = (type == HCC_TYPE_MSG) ? DATA_HI_QUEUE : DATA_LO_QUEUE;
    
    if (oal_netbuf_headroom(netbuf) < HCC_HDR_TOTAL_LEN) {
        oam_error_log1("hcc_tx_data_adapt:: headroom is not enough, headroom[%d]\n", oal_netbuf_headroom(netbuf));
        //oal_netbuf_free(netbuf);
        return HI_FAIL;
    }

    oal_netbuf_push(netbuf, HCC_HDR_TOTAL_LEN);
   // *((hi_u32*)((hi_u8*)oal_netbuf_data(netbuf) + 4)) = atomic_add_return(1, &g_tx_ref);
    //*((hi_u32*)((hi_u8*)oal_netbuf_data(netbuf) + 4)) = g_tx_ref++;
   
    //printf("param len 0x%x\r\n",sizeof(hcc_transfer_param));
    hcc_hdr_param_init(&param, type, sub_type, 0, HCC_FC_NONE, queue_id);

    ret = hcc_host_tx(hcc_host_get_handler(), netbuf, &param);
    if (ret != HI_SUCCESS) {
        oam_error_log1("hcc_tx_data_adapt:: hcc_host_tx failed[%d]\n", ret);
        return HI_FAIL;
    }
    return HI_SUCCESS;
}

static hcc_unc_struc *hcc_adapt_netbuf_len_align(hcc_unc_struc *unc_buf, oal_netbuf_stru *netbuf, int align_len)
{
    hi_s32 ret = HI_SUCCESS;
    hi_u32 len_algin, tail_room_len;
    hi_u32 len = oal_netbuf_len(netbuf);

    if (oal_is_aligned(len, align_len)) {
        printf("oal_is_aligned is ok len 0x%x align_len 0x%x\r\n",len,align_len);
        return unc_buf;
    }
    /* align the netbuf */
    len_algin = oal_round_up(len, align_len);
    if (len_algin < len) {
        oam_error_log2("hcc_netbuf_len_align::len_aglin[%d],len[%d]\n", len_algin, len);
        oal_free(unc_buf);
        return HI_NULL;
    }
    
    //printf("func %s len 0x%x len_algin 0x%x oal_netbuf_tailroom(netbuf) 0x%x data_len 0x%x\r\n",__func__,len,len_algin,oal_netbuf_tailroom(netbuf),netbuf->data_len);
    tail_room_len = len_algin - len;
    if (oal_unlikely(tail_room_len > oal_netbuf_tailroom(netbuf))) {
        /* tailroom not enough */
        ret = oal_netbuf_expand_head(netbuf, 0, (hi_s32)tail_room_len, 0);
        if (OAL_WARN_ON(ret != HI_SUCCESS)) {
            oal_free(unc_buf);
            return HI_NULL;
        }
    }
    oal_netbuf_put(netbuf, tail_room_len);
    /* $)AVXPBIhVCuncEdVC */
    unc_buf->buf = (hi_char *)oal_netbuf_data(netbuf);
    unc_buf->length = oal_netbuf_len(netbuf);
    unc_buf->priv = (hi_void *)netbuf;
    unc_buf->priv_type = NETBUF_STRU_TYPE;
    unc_buf->free = hcc_adapt_mem_free;
    return unc_buf;
}

static hcc_unc_struc* hcc_adapt_netbuf_alloc(hi_s32 len)
{
    oal_netbuf_stru *netbuf = (oal_netbuf_stru *)dev_alloc_skb(len);
    if (netbuf == HI_NULL) {
        oam_error_log1("{[WIFI][E]rx no mem:%u}", len);
        return NULL;
    }

    oal_netbuf_put(netbuf, len);
    /* $)AVXPBIhVCuncEdVC */
    return hcc_structure_conversion(netbuf, NETBUF_STRU_TYPE);
}

static hcc_unc_struc* hcc_adapt_nomal_alloc(hi_s32 len)
{
    hcc_normal_struc p_normal;

    p_normal.buf = oal_memalloc(len);
    if (p_normal.buf  == HI_NULL) {
        return HI_NULL;
    }
    p_normal.length = len;
    /* $)AVXPBIhVCuncEdVC */
    return hcc_structure_conversion(&p_normal, NORMAL_STRU_TYPE);
}

/*****************************************************************************
 $)A9&D\ChJv  : <l2iJG7qPhR*6*V!#,Hg9{2eHkTX6SAP5DJ}>]L+6`#,?I6*5tR;P)7G9X<|J}>]
 $)AJdHk2NJ}  : queue_id 6SAPid
             hcc_queue$)A6SAPM7V8Uk
             unc_buf $)AM(SCJ}>]=a99V8Uk
 $)A75 ;X V5  : HI_SUCCESS ?IRT6*5t4KV!#,HI_FAIL4KV!2;D\6*5t
*****************************************************************************/
#if 0
static hi_u32 hcc_adapt_check_discard_frame(hi_u32 queue_id,
    hcc_trans_queue_stru *hcc_queue, hcc_unc_struc *unc_buf)
{
    hcc_header_stru *hcc_hdr = HI_NULL;
    if (queue_id != DATA_LO_QUEUE) {
        return HI_FAIL;
    }

    hcc_hdr = (hcc_header_stru *)oal_unc_data(unc_buf);
    //TODO $)A:sPx?IRTTZ4K4&EP6OJG7qR*=+4KV!2eHk6SAP
    if (hcc_hdr->main_type == HCC_TYPE_TEST_XFER &&
        hcc_queue->queue_info.qlen > 0x1000) {
        usleep_range(100, 100);
    }
        
    return HI_FAIL;
}
#endif
/*****************************************************************************
 $)A9&D\ChJv  : JM7EK=SPJ}>]=a99@`PM
 $)AJdHk2NJ}  : data M(SCJ}>]=a99V8Uk
 $)A75 ;X V5  : N^
*****************************************************************************/
hi_void hcc_adapt_mem_free(hi_void *data)
{
    oal_netbuf_stru *netbuf = HI_NULL;
    hcc_unc_struc* unc_buf = (hcc_unc_struc*) data;
    if (unc_buf == HI_NULL) {
        return;
    }

    switch(unc_buf->priv_type) {
        case NETBUF_STRU_TYPE:
            netbuf = (oal_netbuf_stru *)unc_buf->priv;
            //printf("free netbuf->data[0x%x] netbuf[0x%x] unc_buf[0x%x]\r\n", netbuf->data, netbuf, unc_buf);
            if (netbuf) {
                if(netbuf->data) {
#ifndef USE_STATIC_SKB_BUF                    
                    oal_free(netbuf->data);
#endif                    
                }
                oal_netbuf_free(netbuf);
            }
            break;
        case NORMAL_STRU_TYPE:
            if(unc_buf->buf) {
                oal_free(unc_buf->buf);
            }
            break;
        default:
            oam_error_log1("unknown structure type:%d", unc_buf->priv_type);
            break;
    }
    oal_free(unc_buf);
}

/*****************************************************************************
 $)A9&D\ChJv  : stop tcpip tx_queue
*****************************************************************************/
hi_void hcc_adapt_stop_tcpip_tx_queue(hcc_trans_queue_stru *hcc_queue)
{
#if 0
    if ((hcc_list_len(&hcc_queue->queue_info) > MAX_CNT_IN_QUEUE) &&
        (hcc_queue->queue_info.flow_flag == HI_FALSE)) {
        oal_net_device_stru *netdev = oal_get_netdev_by_name("wlan0");
        if (netdev == HI_NULL) {
            oam_error_log0("hcc_adapt_stop_tcpip_tx_queue:: netdev is NULL");
            return;
        }
        oal_dev_put(netdev);
        oal_netif_stop_queue(netdev);
        hcc_queue->queue_info.flow_flag = HI_TRUE;
        oal_netif_stop_queue(netdev);
    }
#endif        
}

/*****************************************************************************
 $)A9&D\ChJv  : awake tcpip tx_queue
*****************************************************************************/
hi_void hcc_adapt_awake_tcpip_tx_queue(hcc_trans_queue_stru *hcc_queue)
{
#if 0
    if ((hcc_list_len(&hcc_queue->queue_info) < AWAKE_CNT_IN_QUEUE) && (hcc_queue->queue_info.flow_flag == HI_TRUE)) {
        oal_net_device_stru *netdev = oal_get_netdev_by_name("wlan0");
        if (netdev == HI_NULL) {
            oam_error_log0("hcc_adapt_awake_tcpip_tx_queue:: netdev is NULL");
            return;
        }
        oal_dev_put(netdev);
        oal_netif_wake_queue(netdev);
        hcc_queue->queue_info.flow_flag = HI_FALSE;
    }
#endif    
}

/*****************************************************************************
 $)A9&D\ChJv  : awake tcp/ip tx_queue
*****************************************************************************/

/*****************************************************************************
 $)A9&D\ChJv  : Uk6T2;M(=a99@`PM5D=a99LeWv6TFk2YWw
 $)AJdHk2NJ}  : unc_buf M(SCJ}>]=a99V8Uk
             align_len $)A6TFk3$6H
 $)AW"RbJBOn  #:Hg9{SPP)J}>]=a99RQ>-SP6TFk#,?IRTV1=S75;X
 $)A75 ;X V5  : 75;XM(SCJ}>]=a99V8Uk
*****************************************************************************/
static hcc_unc_struc *hcc_adapt_len_align(hcc_unc_struc *unc_buf, int align_len)
{
    oal_netbuf_stru *netbuf = HI_NULL;
    hcc_unc_struc *unc_buf_t = unc_buf;
    switch (oal_unc_priv_type(unc_buf)) {
        case NETBUF_STRU_TYPE:
            netbuf = (oal_netbuf_stru *)oal_unc_priv(unc_buf);
            unc_buf_t = hcc_adapt_netbuf_len_align(unc_buf, netbuf, align_len);
            break;
        default:
            break;
    }
    return unc_buf_t;
}

/*****************************************************************************
 $)A9&D\ChJv  : IjGkK=SPJ}>]=a99DZ4f#,2"W*;;N*M(SCJ}>]=a99
 $)AJdHk2NJ}  : len IjGk3$6H#,5%N;#:Byte
             type $)AIjGk5DK=SPJ}>]=a99@`PM
 $)AW"RbJBOn  #::sPxR*@)U9#,?ITZcase7VV'PBTvFdK{J}>]=a99@`PM
 $)A75 ;X V5  : 75;XM(SCJ}>]=a99V8Uk
*****************************************************************************/
static hcc_unc_struc* hcc_adapt_alloc_unc_buf(hi_s32 len, hcc_stru_type type)
{
    switch (type) {
        case NETBUF_STRU_TYPE:
            return hcc_adapt_netbuf_alloc(len);
        case NORMAL_STRU_TYPE:
            return hcc_adapt_nomal_alloc(len);
        default:
            break;
    }
    return HI_NULL;
}

/*****************************************************************************
 $)A9&D\ChJv  : Aw?X4&@m:/J}
 $)AJdHk2NJ}  : flow_type J}>]@`PM#,HCC_FLOWCTRL_CREDITN*9\@mV!#,HCC_FLOWCTRL_SDIO
             $)AN*J}>]V!
             dev_mem_cnt device$)ADZ4fWJT4J}
 $)A75 ;X V5  : HI_SUCCESS DZ4f3dWc#,<LPx7"KMJ}>]#, HI_FAIL deviceDZ4f2;Wc#,M#V97"
             $)AKMJ}>]
*****************************************************************************/
static hi_u32 hcc_adapt_tx_flow_ctrl_handle(hi_u16 flow_type, hi_u8 dev_mem_cnt)
{
    if (flow_type == HCC_FLOWCTRL_CREDIT) {
        if (dev_mem_cnt <= HI_PRI_MEM_LOW_LEVEL) {
            printf("[error]flow ctrl credit dev_mem_cnt 0x%x\r\n",dev_mem_cnt);
            return HI_FAIL;
        }

    } else if (flow_type == HCC_FLOWCTRL_SDIO) {
        if (dev_mem_cnt <= LOW_PRI_MEM_LOW_LEVEL) {
            //printf("[error]flow ctrl sdio dev_mem_cnt 0x%x\r\n",dev_mem_cnt);            
            return HI_FAIL;
        }
    }
    return HI_SUCCESS;
}

/*****************************************************************************
 $)A9&D\ChJv  : 2bJThccM(5@6AKYBJ#,M3<F4SdeviceJU5=0|5D8vJ}!#
 $)AJdHk2NJ}  : hcc_hdr hccO{O"M7
             buf $)A4f7EJ}>];:3eGx
             len $)A;:3eGx4f7EJ}>]4sP!
 $)A75 ;X V5  : N^
*****************************************************************************/
static hi_void hcc_rx_test_speed(hcc_header_stru *hcc_hdr, hi_char* buf, int len)
{
    if (len < sizeof(hi_s32)) {
        return;
    }

    if (hcc_hdr->sub_type == HCC_TEST_READ_START) {
        oam_error_log1("Read start.package:%d", *((hi_u32*)buf));
    } else if (hcc_hdr->sub_type == HCC_TEST_READ_TRANFER) {
        oam_error_log1("Package Num:%d", *((hi_u32*)buf));
    } else if (hcc_hdr->sub_type == HCC_TEST_READ_OVER) {
        oam_error_log1("Read over.package:%d", *((hi_u32*)buf));
    }
}

static void hcc_test_write(char* buf, int len, hcc_type_enum type, hi_u32 sub_type)
{
    oal_netbuf_stru *netbuf = HI_NULL;
    hi_u32 ret;

    printf("len 0x%x\r\n",len);
    netbuf = oal_netbuf_alloc(len + HCC_HDR_TOTAL_LEN + HIBUS_H2D_SCATT_BUFFLEN_ALIGN, HCC_HDR_TOTAL_LEN, 4);
    if (netbuf == HI_NULL) {
        oam_error_log0("hcc_test_write:: netbuf_alloc failed!");
        return;
    }

    oal_netbuf_put(netbuf, len);
    if (memcpy_s(oal_netbuf_data(netbuf), len, buf, len) != EOK) {
        oam_error_log0("hmac_hcc_tx_event_buf_to_netbuf:: memcpy_s failed!");
        oal_netbuf_free(netbuf);
        return;
    }

    ret = hcc_tx_data_adapt(netbuf, type, sub_type);
    if (ret != HI_SUCCESS) {
        oam_error_log0("hcc_tx_data_adapt falied!\n");
        oal_netbuf_free(netbuf);
    }
}

/*****************************************************************************
 $)A9&D\ChJv  : 2bJThccM(5@P4KYBJ
 $)AJdHk2NJ}  : N^
 $)A75 ;X V5  : N^
*****************************************************************************/
hi_void hcc_test_channel_speed(char *outbuf, int32_t length, int32_t argc, char **argv)
{
    int package_cnt = 0xBB;
    // int ret;
    hi_u32 len;
    hi_u32 mlen;
    int speed;
    // hi_u8 msgbuf[10];
    long long t1=aos_now_ms();
    printf("test write start.\n");
    memset_s(g_test_buf, sizeof(g_test_buf), 0x55, sizeof(g_test_buf));
    hcc_test_write((char*)&package_cnt, sizeof(package_cnt), HCC_TYPE_TEST_XFER, HCC_TEST_WRITE_START);
#if 0    
    
    for (int i= 0; i< package_cnt; i++) {
        hcc_test_write(g_test_buf, sizeof(g_test_buf), HCC_TYPE_TEST_XFER, HCC_TEST_WRITE_TRANFER);
    }
    
    hcc_test_write((char*)&package_cnt, sizeof(package_cnt), HCC_TYPE_TEST_XFER, HCC_TEST_WRITE_OVER);
    
    ret = csi_kernel_msgq_get(hcc_host_get_handler()->hcc_transer_info.hcc_test_tx, (void *)&msgbuf, -1);
    if(ret != 0) {
        printf("hcc_task_tx_thread:: hcc_task was interupterd by a singnal");
        return; 
    }
#endif    
    #if 0
    //TODO NEED
    //$)A5H4}hostOlS&
    ret = oal_wait_for_completion_timeout(&hcc_host_get_handler()->hcc_transer_info.hcc_test_tx, 1000 * HZ);
    if (ret == 0) {
        printf("hcc_task_tx_thread:: hcc_task was interupterd by a singnal\n");
    }
    
    #endif
    printf("func %s line %d\r\n",__func__,__LINE__);
    long long t2=aos_now_ms()+1000;
    len = (package_cnt * MAX_TEST_PACKAGE_LEN);
    mlen = 1024 * 1024; 
    speed =len / mlen;  
    printf("test write over. speed:%lld Mbps\n", (speed / ((t2 - t1) / 1000)) * 8);
}

static struct hcc_bus_adpta_ops g_hcc_bus_opt ={
    .rx_proc_queue          = hcc_adapt_rx_data_process,
    .tx_discard_key_frame   = HI_NULL,//hcc_adapt_check_discard_frame,
    .tx_sort_key_frame      = HI_NULL,
    .tx_flow_ctrl_handle    = hcc_adapt_tx_flow_ctrl_handle,
    .wlan_pm_set_packet_cnt = HI_NULL,
    .private_len_align      = hcc_adapt_len_align,
    .alloc_unc_buf          = hcc_adapt_alloc_unc_buf,
    .free_unc_buf           = hcc_adapt_mem_free,
    .wlan_pm_wakeup_dev     = HI_NULL,
    .channel_rx_test        = hcc_rx_test_speed,
    .stop_tcpip_tx_queue    = hcc_adapt_stop_tcpip_tx_queue,
    .awake_tcpip_tx_queue   = hcc_adapt_awake_tcpip_tx_queue
};

hi_u32 hcc_adapt_init(hi_void)
{
#ifdef USE_STATIC_SKB_BUF  
    extern int dev_init_skb(void);
    dev_init_skb();
#endif    
    return hcc_host_init(&g_hcc_bus_opt);
}

hi_void hcc_adapt_exit(hi_void)
{
    hcc_host_exit(hcc_host_get_handler());
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif


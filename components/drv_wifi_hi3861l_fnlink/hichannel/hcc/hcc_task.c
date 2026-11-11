/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: hcc layer frw task.
 * Author: Hisilicon
 * Create: 2020-09-28
 */
#include "hcc_task.h"
#include "hcc_host.h"
#include "oam_ext_if.h"
#include "securec.h"
#include "hcc_list.h"
//#include <linux/delay.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define HCC_TASK_PRIO       3
static volatile hi_u8   g_thread_exit_flag = HI_FALSE;
static hi_u8   g_thread_working = HI_FALSE;

#define HCC_TASK_NAME          "hisi_hcc0"
#define HCC_TASK_TX_NAME       "hisi_hcc_tx"
#define HCC_TASK_RX_NAME       "hisi_hcc_rx"
#define TX_WRITE               0
#define RX_READ                1
#define DELAY_10_US         10
#define HCC_TASK_SIZE       0x4000

hi_s32 queues_len_check(hcc_handler_stru *hcc_handler, hcc_chan_type dir)
{
    hi_s32 i;
    hcc_trans_queue_stru *p_queue;

    p_queue = hcc_handler->hcc_transer_info.hcc_queues[dir].queues;
    ///p_queue++;
    for (i = 0; i < HCC_QUEUE_COUNT; i++, p_queue++) {
        ///printf("len 0x%x\n", p_queue->queue_info.qlen);
        if (hcc_list_len(&p_queue->queue_info)) {
            return HI_TRUE;
        }
    }
    return HI_FALSE;
}

static  hi_s32  hcc_thread_tx_wait_event_cond_check(hcc_handler_stru *hcc_handler)
{
    hi_s32 ret;
    /*
     * please first check the condition
     * which may be ok likely to reduce the cpu mips
     */
    ret = queues_len_check(hcc_handler, HCC_TX);// &&
          //(hcc_handler->hcc_transer_info.channel_exception_flag == 0));
#ifdef _PRE_CONFIG_WLAN_THRANS_THREAD_DEBUG
    if (ret == HI_TRUE)
        hcc_handler->hcc_transer_info.thread_stat.wait_event_run_count++;
    if (ret == HI_FALSE)
        hcc_handler->hcc_transer_info.thread_stat.wait_event_block_count++;
#endif
    return ret;
}

static  hi_s32  hcc_thread_rx_wait_event_cond_check(hcc_handler_stru *hcc_handler)
{
    hi_s32 ret;

    /*
     * please first check the condition
     * which may be ok likely to reduce the cpu mips
     */
    ret = queues_len_check(hcc_handler, HCC_RX);
#ifdef _PRE_CONFIG_WLAN_THRANS_THREAD_DEBUG
    if (ret == HI_TRUE)
        hcc_handler->hcc_transer_info.thread_stat.wait_event_run_count++;
    if (ret == HI_FALSE)
        hcc_handler->hcc_transer_info.thread_stat.wait_event_block_count++;
#endif
    return ret;
}

hi_s32 hcc_thread_process(hcc_handler_stru *hcc_handler, hi_u8 wr)
{
    hi_s32 ret = 0;

    if (wr == TX_WRITE) {
        ret += hcc_host_proc_tx_queue(hcc_handler, DATA_HI_QUEUE);
        ret += hcc_host_proc_tx_queue(hcc_handler, DATA_LO_QUEUE);
    } else {
        ret += hcc_host_proc_rx_queue(hcc_handler, DATA_HI_QUEUE);
        ret += hcc_host_proc_rx_queue(hcc_handler, DATA_LO_QUEUE);
    }

    return ret;
}

hi_u8 hcc_get_thread_exit_flag(hi_void)
{
    return g_thread_exit_flag;
}
extern int usleep(__useconds_t usec);
hi_void hcc_exit_task_thread(hcc_handler_stru* hcc)
{
    hi_u16 retry_time = 10000;
    g_thread_exit_flag = HI_TRUE;
    hcc_sched_transfer(hcc);
    while (g_thread_working && retry_time > 0) {
        usleep(DELAY_10_US);
        retry_time--;
    }
}
#define WAIT_FOREVER  -1
extern aos_mutex_t g_sdio_tx_rx_mutex;

static void hcc_task_rx_thread(hi_void *data)
{
    // hi_s32 hcc_ret = 0;    
    // hcc_msg_t msg;
    hcc_handler_stru *hcc_handler = (hcc_handler_stru *)data;

    oam_info_log0("hcc_task_rx_thread:: hcc_task_rx_thread enter\n");
    g_thread_exit_flag = HI_FALSE;
    for (; ;) {
        if (g_thread_exit_flag == HI_TRUE) {
            oam_warning_log0("hcc_task_thread:: hcc_task leave\n");
            break;
        }
        aos_sem_wait(&hcc_handler->hcc_transer_info.hcc_rx_wq, AOS_WAIT_FOREVER);
        
        if(hcc_thread_rx_wait_event_cond_check(hcc_handler) != HI_TRUE) {
            //oam_info_log0("rx check error\n");
            continue;       
        }

        //aos_mutex_lock(&g_sdio_tx_rx_mutex, AOS_WAIT_FOREVER);
        hcc_thread_process(hcc_handler, RX_READ);
        //aos_mutex_unlock(&g_sdio_tx_rx_mutex);
    }
    
    printf("exit hcc_task_rx_thread\r\n");
    return;
}

/*****************************************************************************
 hcc task $)AO_3LVw3LPr
*****************************************************************************/
extern aos_mutex_t g_sdio_hsot_mutex;

static void hcc_task_tx_thread(hi_void *data)
{
    // hi_s32 hcc_ret = 0;
    // hcc_msg_t msg;
    hcc_handler_stru *hcc_handler = (hcc_handler_stru *)data;

    oam_info_log0("hcc_task_tx_thread:: hcc_task_tx_thread enter\n");
    g_thread_exit_flag = HI_FALSE;
    g_thread_working = HI_TRUE;
    for (; ;) {
        if (g_thread_exit_flag == HI_TRUE) {
            oam_warning_log0("hcc_task_tx_thread:: hcc_task leave");
            break;
        }
     /* wait CMD finish */
        //printf("wdp-004\r\n");
        aos_sem_wait(&hcc_handler->hcc_transer_info.hcc_tx_wq, AOS_WAIT_FOREVER);
        //printf("st\r\n");
        if(hcc_thread_tx_wait_event_cond_check(hcc_handler) != HI_TRUE) {
            //oam_info_log0("tx check error\n");
            continue;       
        }
        //printf("****origin write start hcc_task_tx_thread*****\n");
        //dump(hcc_handler->hcc_transer_info.hcc_queues[HCC_TX].queues[0]. , sum_len);
        //printf("****origin write end hcc_task_tx_thread*****\n");
        ///aos_mutex_lock(&g_sdio_tx_rx_mutex, AOS_WAIT_FOREVER);

        hcc_thread_process(hcc_handler, TX_WRITE);
        //aos_mutex_unlock(&g_sdio_tx_rx_mutex);
        //printf("ov\n");
    }
    g_thread_working = HI_FALSE;
    return;
}

hi_u32 hcc_task_init(hcc_handler_stru *hcc_handler)
{
    aos_task_t hcc_tx_handle;
    aos_task_t hcc_rx_handle;

    hcc_handler->hcc_transer_info.hcc_transfer_thread = aos_task_new_ext(&hcc_tx_handle,"hcc_task_tx", hcc_task_tx_thread, hcc_handler, 3*1024, 31);
    hcc_handler->hcc_transer_info.hcc_rx_thread = aos_task_new_ext(&hcc_rx_handle,"hcc_task_rx", hcc_task_rx_thread, hcc_handler, 3*1024, 31);
    //hcc_handler->hcc_transer_info.hcc_rx_thread = aos_task_new("hcc_task_rx", hcc_task_rx_thread, hcc_handler, 1024*1024);

    if ((hcc_handler->hcc_transer_info.hcc_transfer_thread) || (hcc_handler->hcc_transer_info.hcc_rx_thread)) {
        return HI_FAIL;
    }
    return HI_SUCCESS;
}

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif


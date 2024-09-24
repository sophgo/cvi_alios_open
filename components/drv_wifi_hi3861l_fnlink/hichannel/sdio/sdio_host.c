/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: hcc layer frw task.
 * Author: dujinxin
 * Create: 2020-09-28
 */
#include "soc.h"
#include "hcc_host.h"
#include "sdio_host.h"
#include "hcc_list.h"
#include "oam_ext_if.h"
#include "oal_mm.h"
#include "oal_util.h"
#include "securec.h"
#include "hi_types.h"
#include "io.h"
//#include "csi_kernel.h"
//#include "oal_completion.h"
//#include <linux/delay.h>
//#include <linux/interrupt.h>
//#include <linux/gpio.h>
//#include <linux/pm_runtime.h>
//#include <linux/mmc/sdio.h>
//#include <linux/scatterlist.h>

#include "hcc_adapt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#if 1

#define oal_malloc aos_malloc
#define oal_free aos_free
#endif
/* $)A:j6(Re */

#define GPIO_REG_WRITEL(addr, val)            *((volatile unsigned int *)addr) = val;

aos_mutex_t g_sdio_hsot_mutex;
aos_mutex_t g_spin_mutex;
aos_mutex_t g_sdio_tx_rx_mutex;
aos_mutex_t g_sdio_cmd_mutex;


/* $)AV'3V@kI"DZ4fJU7" */
#define _SCATTER_LIST_MEMORY

/* $)AH+>V1dA?6(Re */
static oal_channel_stru *g_hi_sdio_;

static inline void *sdio_get_drvdata(const struct sdio_func *func)
{
	return func->driver_data;
}

static inline void sdio_set_drvdata(struct sdio_func *func, void *data)
{
	func->driver_data = data;
}

static hi_void oal_sdio_print_state(hi_u32 old_state, hi_u32 new_state)
{
    if (old_state != new_state) {
        oam_info_log4("sdio state changed, tx[%d=>%d],rx[%d=>%d] (1:on, 0:off)\n",
            (old_state & OAL_SDIO_TX) ? 1 : 0, (new_state & OAL_SDIO_TX) ? 1 : 0,
            (old_state & OAL_SDIO_RX) ? 1 : 0, (new_state & OAL_SDIO_RX) ? 1 : 0);
    }
}

hi_void oal_free_sdio_stru(oal_channel_stru *hi_sdio)
{
    hi_unref_param(hi_sdio);
    oam_error_log0("oal_free_sdio_stru\n");
}

oal_channel_stru *oal_get_sdio_default_handler(hi_void)
{
    return g_hi_sdio_;
}

oal_channel_stru *oal_sdio_alloc(struct sdio_func *func)
{
    oal_channel_stru* hi_sdio = HI_NULL;
    if (func == HI_NULL) {
        oam_error_log0("oal_sdio_alloc: func null!\n");
        return HI_NULL;
    }

    hi_sdio = oal_get_sdio_default_handler();
    if (hi_sdio == HI_NULL) {
        oam_error_log0("Failed to alloc hi_sdio!\n");
        return HI_NULL;
    }

    hi_sdio->func           = func;
    sdio_set_drvdata(func, hi_sdio);

    return hi_sdio;
}

hi_void oal_disable_sdio_state(oal_channel_stru *hi_sdio, hi_u32 mask)
{
    hi_u32 old_state;
    if (hi_sdio == HI_NULL || hi_sdio->func == HI_NULL) {
        oam_error_log0("oal_enable_sdio_state: hi_sdio null!\n");
        return;
    }

    oal_sdio_claim_host(hi_sdio->func);
    old_state = hi_sdio->state;
    hi_sdio->state &= ~mask;
    oal_sdio_print_state(old_state, hi_sdio->state);
    oal_sdio_release_host(hi_sdio->func);
}

static hi_s32 oal_sdio_sleep_dev_internal(oal_channel_stru *hi_sdio)
{
    int    ret;
    if (hi_sdio == HI_NULL || hi_sdio->func == HI_NULL) {
        return -OAL_EFAIL;
    }

    oal_sdio_claim_host(hi_sdio->func);
    sdio_f0_writeb(hi_sdio->func, ALLOW_TO_SLEEP_VALUE, HISDIO_WAKEUP_DEV_REG, &ret);
    oal_sdio_release_host(hi_sdio->func);
    return ret;
}

hi_s32 oal_sdio_send_msg(oal_channel_stru *hi_sdio, unsigned long val)
{
    hi_s32       ret  = HI_SUCCESS;
    struct sdio_func *func = HI_NULL;
    if (hi_sdio == HI_NULL || hi_sdio->func == HI_NULL) {
        oam_error_log0("{oal_sdio_send_msg::sdio is not initialized,can't send sdio msg!}");
        return -OAL_EINVAL;
    }

    func = hi_sdio->func;
    oal_sdio_claim_host(func);
    oal_sdio_writel(func, (1 << val), HISDIO_REG_FUNC1_WRITE_MSG, &ret);
    if (ret) {
        oam_error_log2("{oal_sdio_send_msg::failed to send sdio msg[%lu]!ret=%d}", val, ret);
    }
    oal_sdio_release_host(func);
    return ret;
}

hi_s32 oal_sdio_get_state(const oal_channel_stru *hi_sdio, hi_u32 mask)
{
    if (hi_sdio == HI_NULL) {
        return HI_FALSE;
    }

    if ((hi_sdio->state & mask) == mask) {
        return HI_TRUE;
    } else {
        return HI_FALSE;
    }
}

hi_void oal_enable_sdio_state(oal_channel_stru *hi_sdio, hi_u32 mask)
{
    hi_u32 old_state;
    if (hi_sdio == HI_NULL || hi_sdio->func == HI_NULL) {
        oam_error_log0("oal_enable_sdio_state: hi_sdio null!\n");
        return;
    }

    oal_sdio_claim_host(hi_sdio->func);
    old_state = hi_sdio->state;
    hi_sdio->state |= mask;
    oal_sdio_print_state(old_state, hi_sdio->state);
    oal_sdio_release_host(hi_sdio->func);
}

hi_void oal_sdio_dev_shutdown(void *dev)
{
    hi_s32   ret;
    // hi_unref_param(dev);
    oal_channel_stru *hi_sdio = oal_get_sdio_default_handler();
    if ((hi_sdio == HI_NULL) || (hi_sdio->func == NULL)) {
        return;
    }

    oal_sdio_sleep_dev_internal(oal_get_sdio_default_handler());

    if (oal_sdio_send_msg(oal_get_sdio_default_handler(), H2D_MSG_PM_WLAN_OFF) != HI_SUCCESS) {
        return;
    }

    if (HI_TRUE != oal_sdio_get_state(hi_sdio, OAL_SDIO_ALL)) {
        return;
    }

    oal_sdio_claim_host(hi_sdio->func);
    ret = sdio_disable_func(hi_sdio->func);
    oal_sdio_release_host(hi_sdio->func);
    if (ret) {
        oam_error_log0("sdio_disable_func fail!\n");
    }
}

hi_s32 oal_sdio_transfer_rx_reserved_buff(const oal_channel_stru *hi_sdio)
{
    hi_s32 ret;
    if (hi_sdio->sdio_extend == HI_NULL) {
        oam_error_log0("{hi_sdio->sdio_extend NULL!}");
        return -OAL_EINVAL;
    }

    hi_u32 ul_extend_len = hi_sdio->sdio_extend->xfer_count;

    if (ul_extend_len == 0) {
        oam_error_log0("{extend_len is zero!}");
        return -OAL_EINVAL;
    }

    if (ul_extend_len > SDIO_MAX_XFER_LEN) {
        return -OAL_EINVAL;
    }

    ret = oal_sdio_readsb(hi_sdio->func, 0, hi_sdio->rx_buf, ul_extend_len);
    //dump(hi_sdio->rx_buf, ul_extend_len);

    return ret;
}

hi_void oal_copy_data_to_uncbuf(const oal_channel_stru *hi_sdio,
    const hcc_data_queue *head,
    hi_u8* buf,
    hi_u32 len)
{
    hcc_unc_struc *unc_buf = HI_NULL;
    hcc_unc_struc *tmp = HI_NULL;
    hi_u32 offset = 0;
    hi_s32 index =0;
    osal_list_for_each_entry_safe(unc_buf, tmp, (&head->data_queue), list) {
        memcpy_s(oal_unc_data(unc_buf), oal_unc_len(unc_buf), buf + offset, oal_unc_len(unc_buf));
        offset += (hi_sdio->sdio_extend->comm_reg[index] << HISDIO_D2H_SCATT_BUFFLEN_ALIGN_BITS);
        len -= (hi_sdio->sdio_extend->comm_reg[index] << HISDIO_D2H_SCATT_BUFFLEN_ALIGN_BITS);
        if (len  <= 0) {
            break;
        }
        index++;
    }
}

hi_s32 oal_sdio_tranfer_commit_memory(const oal_channel_stru *hi_sdio,
    const hcc_data_queue *head,
    hi_s32 rw)
{
    hi_s32 ret;
    hi_u32 queue_len;
    hi_u32 sum_len = 0;
    hi_u32 idx = 0;
    hcc_unc_struc *unc_buf = HI_NULL;
    hcc_unc_struc *tmp = HI_NULL;

    queue_len = hcc_list_len(head);
    //printf("func %s queue_len 0x%x\r\n",__func__,queue_len);
    
    osal_list_for_each_entry_safe(unc_buf, tmp, (&head->data_queue), list) {
        if (!oal_is_aligned((uintptr_t)oal_unc_data(unc_buf), 4)) {
            oam_error_log3("oal_sdio_transfer_netbuf_list netbuf 4 aligned fail!:: unc_buf[%p], len[%d], rw[%d]",
                oal_unc_data(unc_buf), oal_unc_len(unc_buf), rw);
            return -OAL_EINVAL;
        }

        if (WARN_ON(!oal_is_aligned(oal_unc_len(unc_buf), HIBUS_H2D_SCATT_BUFFLEN_ALIGN))) {
            printf("error func %s oal is not aligned scatt\r\n",__func__);
            return -OAL_EINVAL;
        }

        if (rw == SDIO_WRITE) {
            memcpy_s(hi_sdio->tx_buf + sum_len, SDIO_MAX_XFER_LEN, oal_unc_data(unc_buf), oal_unc_len(unc_buf));
        }
        //dump(oal_unc_data(unc_buf), oal_unc_len(unc_buf));
        sum_len += oal_unc_len(unc_buf);
        //printf("^^sum_len 0x%x\r\n",sum_len);
        idx++;
    }

    if (oal_unlikely(idx > queue_len)) {
        return -OAL_EINVAL;
    }

    if (WARN_ON(sum_len > SDIO_MAX_XFER_LEN)) {
        return -OAL_EINVAL;
    }

    if (sum_len < HISDIO_BLOCK_SIZE) {
        sum_len = hi_byte_align(sum_len, 4);
    } else {
        sum_len = hi_byte_align(sum_len, HISDIO_BLOCK_SIZE);
    }

    //printf("****origin write start commit_memory*****\n");
    //dump(hi_sdio->tx_buf , sum_len);
    //printf("****origin write end commit_memory*****\n");

    oal_sdio_claim_host(hi_sdio->func);
    
    if (rw == SDIO_WRITE) {
        ret = oal_sdio_writesb(hi_sdio->func, 0, hi_sdio->tx_buf, sum_len);
    } else {
        //printf("read-03\r\n");
        ret = oal_sdio_readsb(hi_sdio->func, 0, hi_sdio->rx_buf, sum_len);
    }
    oal_sdio_release_host(hi_sdio->func);

    /* $)A=+4Sdevice;qH!5DJ}>]?=145=netbuf */
    if (rw == SDIO_READ) {
        //printf("sum %d\n", sum_len);
        oal_copy_data_to_uncbuf(hi_sdio, head, hi_sdio->rx_buf, sum_len);
    }
    

    if(ret != 0)
        printf("error 0x%x !!!!!\n",ret);
    return ret;
}

hi_s32 oal_sdio_transfer_list(const oal_channel_stru *hi_sdio,
    const hcc_data_queue *head,
    hi_s32 rw)
{
    hi_s32 ret;

    if ((!hi_sdio) || (!head)) {
        oam_error_log0("hi_sdio head null\n");
        return -OAL_EINVAL;
    }

    if (WARN_ON(rw >= SDIO_OPT_BUTT)) {
        return -OAL_EINVAL;
    }

    if (WARN_ON(hcc_is_list_empty(head))) {
        return -OAL_EINVAL;
    }
    ret = oal_sdio_tranfer_commit_memory(hi_sdio, head, rw);
    return ret;
}

//extern hi_void hcc_adapt_mem_free(hi_void *data);

hi_s32 oal_sdio_build_rx_list(oal_channel_stru *hi_sdio, hcc_data_queue *head)
{
    hi_s32 i;
    hi_u8  buff_len;
    hi_u16 buff_len_t;
    hi_s32 ret = HI_SUCCESS;
    hi_u32 sum_len = 0;
    hcc_unc_struc *unc_buf = HI_NULL;

    if (!hcc_is_list_empty(head)) {
        oam_error_log0("oal_sdio_build_rx_list: oal netbuf list is not empty\n");
        return -OAL_EINVAL;
    }

    if (hi_sdio->bus_ops==HI_NULL ||
        hi_sdio->bus_ops->alloc_unc_buf == HI_NULL) {
        oam_error_log0("oal_sdio_build_rx_list: not register alloc_unc_buf fuction\n");
        return -OAL_EINVAL;
    }

    for (i = 0; i < HISDIO_EXTEND_REG_COUNT; i++) {
        buff_len = hi_sdio->sdio_extend->comm_reg[i];
        if (buff_len == 0) {
            break;
        }

        buff_len_t = buff_len << HISDIO_D2H_SCATT_BUFFLEN_ALIGN_BITS;

        unc_buf = hi_sdio->bus_ops->alloc_unc_buf(buff_len_t, NETBUF_STRU_TYPE);
        if (unc_buf ==  HI_NULL) {
            oam_error_log0("oal_sdio_build_rx_list: alloc_unc_buf fail\n");
            goto failed_unc_buf_alloc;
        }
        //printf("wdp-020\r\n");
        //debug_mm_overview(NULL);
        //printf("wdp-021\r\n");
        sum_len += buff_len_t;
        //hcc_adapt_mem_free(unc_buf);
        hcc_list_add_tail(head, unc_buf);
        
    }
    //return ret;
    if (OAL_WARN_ON(HISDIO_ALIGN_4_OR_BLK(sum_len) != hi_sdio->sdio_extend->xfer_count)) {
        oam_warning_log3("{[WIFI][E]scatt total len[0x%x] should = xfercount[0x%x],after pad len:0x%x}\n",
            sum_len, hi_sdio->sdio_extend->xfer_count, HISDIO_ALIGN_4_OR_BLK(sum_len));
        goto failed_unc_buf_alloc;
    }

    if (oal_unlikely(hcc_is_list_empty(head))) {
        return -OAL_EINVAL;
    }

    return ret;
failed_unc_buf_alloc:
    hcc_list_purge(head);
    ret = oal_sdio_transfer_rx_reserved_buff(hi_sdio);
    if (ret != HI_SUCCESS) {
        oam_error_log0("oal_sdio_transfer_rx_reserved_buff fail\n");
    }
    return -OAL_ENOMEM;
}

hi_s32 oal_sdio_dev_init(oal_channel_stru *hi_sdio)
{
    struct sdio_func   *func = HI_NULL;
    hi_s32               ret;
    hi_u8 value0 = 0xff;
    hi_u8 value1 = 0xff;

    if (hi_sdio == HI_NULL || hi_sdio->func == HI_NULL) {
        return -OAL_EFAIL;
    }

    func = hi_sdio->func;
    oal_sdio_claim_host(hi_sdio->func);
     /* $)A3,J1J1<dN*1000  */
    sdio_en_timeout(func)  = 1000;
    
    ret = sdio_enable_func(func);
    if (ret < 0) {
        oam_error_log1("failed to enable sdio function! ret=%d\n", ret);
        goto failed_enabe_func;
    }

    ret = sdio_set_block_size(func, HISDIO_BLOCK_SIZE);
    if (ret) {
        oam_error_log1("failed to set sdio blk size! ret=%d\n", ret);
        goto failed_set_block_size;
    }

    oal_sdio_writeb(func, HISDIO_FUNC1_INT_MASK, HISDIO_REG_FUNC1_INT_STATUS, &ret);
    if (ret) {
        oam_error_log1("failed to clear sdio interrupt! ret=%d\n", ret);
        goto failed_clear_func1_int;
    }
    
    value0 = oal_sdio_readb(func, HISDIO_REG_FUNC1_INT_STATUS, &ret);
    if(ret){
        oam_error_log1("failed to clear sdio interrupt! value=%d ret=%d\n",value0, ret);
        goto failed_clear_func1_int;      
    }
    oal_sdio_writeb(func, HISDIO_FUNC1_INT_MASK, HISDIO_REG_FUNC1_INT_ENABLE, &ret);
    if (ret < 0) {
        oam_error_log1("failed to enable sdio interrupt! ret=%d\n", ret);
        goto failed_enable_func1;
    }

    value1 = oal_sdio_readb(func, HISDIO_REG_FUNC1_INT_ENABLE, &ret);
    if(ret){
        oam_error_log1("failed to enable sdio interrupt! value=%d ret=%d\n", value1,ret);
        goto failed_clear_func1_int;      
    }
    
    oam_info_log0("maskvalue 0x%x value0 0x%x value1 0x%x\r\n",HISDIO_FUNC1_INT_MASK,value0, value1);

    oal_enable_sdio_state(hi_sdio, OAL_SDIO_ALL);
    oal_sdio_release_host(hi_sdio->func);

    return HI_SUCCESS;
failed_enable_func1:
failed_clear_func1_int:
failed_set_block_size:
    sdio_disable_func(func);
failed_enabe_func:
    oal_sdio_release_host(hi_sdio->func);
    return ret;
}
#if 0
static hi_void oal_sdio_dev_deinit(oal_channel_stru *hi_sdio)
{
    struct sdio_func   *func = HI_NULL;
    hi_s32           ret = 0;
    func  = hi_sdio->func;
    sdio_claim_host(func);
    oal_sdio_writeb(func, 0, HISDIO_REG_FUNC1_INT_ENABLE, &ret);
    sdio_disable_func(func);
    oal_disable_sdio_state(hi_sdio, OAL_SDIO_ALL);
    sdio_release_host(func);
}
#endif

hi_s32 oal_sdio_detectcard_to_core(struct sdio_func *func, hi_s32 sdio_dev_num)
{
    int ret = 0;

    if(func->card == NULL) {
        return HI_FAILURE;
    }
    func->card->host.base  = (sdif_handle_t)SDIO_BASE_ADDR;

    ret = SDIO_Init(func->card);
    if (ret != kStatus_Success) {
        printf("[error]func %s line %d ret %d !!!\n", __func__,__LINE__, ret);
        return ret;
    }
// TODO FOR SDIO_DEV_NUM
    return HI_SUCCESS;
}
#if 0
static hi_void oal_sdio_remove(struct sdio_func *func)
{
    oal_channel_stru *hi_sdio = HI_NULL;
    if (func == HI_NULL) {
        oam_error_log0("[Error]oal_sdio_remove: Invalid NULL func!\n");
        return;
    }

    hi_sdio = (oal_channel_stru *)sdio_get_drvdata(func);
    if (hi_sdio == HI_NULL) {
        oam_error_log0("[Error]Invalid NULL hi_sdio!\n");
        return;
    }

    oal_sdio_dev_deinit(hi_sdio);
    sdio_set_drvdata(func, NULL);
    oam_error_log0("Hisilicon connectivity sdio driver has been removed.");
}
#endif
hi_s32 oal_sdio_extend_buf_get(const oal_channel_stru *hi_sdio)
{
    hi_s32 ret = HI_SUCCESS;

    //printf("read-01\r\n");
    ret = oal_sdio_memcpy_fromio(hi_sdio->func, (hi_void *)hi_sdio->sdio_extend,
                                     HISDIO_EXTEND_BASE_ADDR, sizeof(hisdio_extend_func));
    if (ret != HI_SUCCESS) {
        oam_info_log0("{[SDIO][Err]sdio read extend_buf fail!}\n");
    }
    return ret;
}

hi_s32 oal_sdio_get_func1_int_status(const oal_channel_stru *hi_sdio, hi_u8 *int_stat)
{
    hi_sdio->sdio_extend->int_stat &= hi_sdio->func1_int_mask;
    *int_stat = (hi_sdio->sdio_extend->int_stat & 0xF);
    return HI_SUCCESS;
}

static hi_s32 oal_sdio_msg_stat(const oal_channel_stru *hi_sdio, hi_u32 *msg)
{
    hi_s32 ret = 0;

    *msg = oal_sdio_readl(hi_sdio->func, HISDIO_REG_FUNC1_MSG_FROM_DEV, &ret);

    if (ret) {
        oam_info_log1("sdio readb error![ret=%d]\n", ret);
        return ret;
    }
    hi_sdio->sdio_extend->msg_stat = *msg;

    return HI_SUCCESS;
}
static inline int test_bit(int nr, const volatile uint32_t *addr)
{
    return (*addr >> nr) & 0x1;
}
hi_s32 oal_sdio_msg_irq(oal_channel_stru *hi_sdio)
{
    hi_u32 bit = 0;
    //struct sdio_func    *func;
    hi_u32               msg = 0;
    hi_s32               ret;
    unsigned long        msg_tmp;

    //func       = hi_sdio->func;
    /* reading interrupt form ARM Gerneral Purpose Register(0x28)  */
    ret = oal_sdio_msg_stat(hi_sdio, &msg);
    if (ret) {
        oam_info_log1("[SDIO][Err]oal_sdio_msg_stat error![ret=%d]\n", ret);
        return ret;
    }
    msg_tmp = (unsigned long)msg;

    if (!msg) {
        return HI_SUCCESS;
    }
    if (test_bit(D2H_MSG_DEVICE_PANIC, (unsigned int *)&msg_tmp)) {
        oal_disable_sdio_state(hi_sdio, OAL_SDIO_ALL);
    }
    oal_sdio_release_host(hi_sdio->func);
    if (msg_tmp & BIT(D2H_MSG_DEVICE_PANIC)) {
        bit = D2H_MSG_DEVICE_PANIC;
        msg_tmp &= ~ BIT(bit);
        hi_sdio->msg[bit].count++;
        hi_sdio->msg[bit].cpu_time = aos_now_ms();
        if (hi_sdio->msg[bit].msg_rx) {
            oam_info_log1("device panic msg come, 0x%8x\n", msg);
            hi_sdio->msg[bit].msg_rx(hi_sdio->msg[bit].data);
        }
    }
    bit = 0;
    for(bit = 0; bit < D2H_MSG_COUNT + 1; bit++) {
        if((msg_tmp & BIT(bit)) == 0)
            continue;

        if (bit >= D2H_MSG_COUNT) {
            oam_info_log0("oal_sdio_msg_irq, bit >= D2H_MSG_COUNT\n");
            return -OAL_EFAIL;
        }
        hi_sdio->msg[bit].count++;
        hi_sdio->msg[bit].cpu_time = aos_now_ms();
        if (hi_sdio->msg[bit].msg_rx) {
            hi_sdio->msg[bit].msg_rx(hi_sdio->msg[bit].data);
        }
    }
    oal_sdio_claim_host(hi_sdio->func);

    return HI_SUCCESS;
}

hi_s32 oal_sdio_get_credit(const oal_channel_stru *hi_sdio, hi_u32 *uc_hipriority_cnt)
{
    hi_s32 ret;
    //sdio_claim_host(hi_sdio->func);
    //printf("read-04\r\n");
    ret = oal_sdio_memcpy_fromio(hi_sdio->func, (hi_u8 *)uc_hipriority_cnt,
                                 HISDIO_EXTEND_CREDIT_ADDR, sizeof(*uc_hipriority_cnt));
    //sdio_release_host(hi_sdio->func);
    /* $)A4K4&R*HC3vCPU */
    //schedule();
    return ret;
}

static hi_s32 oal_sdio_xfercount_get(const oal_channel_stru *hi_sdio, hi_u32 *xfercount)
{
    hi_s32 ret = 0;
    /* read from 0x0c */
    //printf("read-02\r\n");
    *xfercount = oal_sdio_readl(hi_sdio->func, HISDIO_REG_FUNC1_XFER_COUNT, &ret);
    if (oal_unlikely(ret)) {
        oam_error_log1("[E]sdio read xercount failed ret=%d\n", ret);
        return ret;
    }
    hi_sdio->sdio_extend->xfer_count = *xfercount;
    return HI_SUCCESS;
}

hi_s32 oal_sdio_transfer_register(oal_channel_stru *hi_sdio, struct hcc_bus_ops *bus_ops)
{
    if (hi_sdio == HI_NULL) {
        return -OAL_EINVAL;
    }
    hi_sdio->bus_ops  = bus_ops;
    return HI_SUCCESS;
}

hi_void oal_sdio_transfer_unregister(oal_channel_stru *hi_sdio)
{
    hi_sdio->bus_ops = HI_NULL;
}

hi_s32 oal_sdio_message_register(oal_channel_stru *hi_sdio, hi_u8 msg, sdio_msg_rx cb, hi_void *data)
{
    if (hi_sdio == HI_NULL || msg >= D2H_MSG_COUNT) {
        return -OAL_EFAIL;
    }
    hi_sdio->msg[msg].msg_rx = cb;
    hi_sdio->msg[msg].data = data;
    return HI_SUCCESS;
}

hi_void oal_sdio_message_unregister(oal_channel_stru *hi_sdio, hi_u8 msg)
{
    if (hi_sdio == HI_NULL || msg >= D2H_MSG_COUNT) {
        return;
    }
    hi_sdio->msg[msg].msg_rx = HI_NULL;
    hi_sdio->msg[msg].data = HI_NULL;
}

hi_s32 oal_sdio_data_sg_irq(oal_channel_stru *hi_sdio)
{
    struct sdio_func   *func = HI_NULL;
    hi_s32 ret;
    hi_u32              xfer_count;

    if (hi_sdio == HI_NULL || hi_sdio->func == HI_NULL || hi_sdio->bus_data == HI_NULL) {
        return -OAL_EINVAL;
    }
#if 1
    func = hi_sdio->func;
    ret = oal_sdio_xfercount_get(hi_sdio, &xfer_count);
    //printf("xfer_count 0x%x\r\n",xfer_count);
    if (oal_unlikely(ret)) {
        return -OAL_EFAIL;
    }
#endif
    /* beacuse get buf may cost lot of time, so release bus first */
    if (hi_sdio->bus_ops == HI_NULL ||
        hi_sdio->bus_ops->rx == HI_NULL) {
        return -OAL_EINVAL;
    }

    oal_sdio_release_host(func);
    hi_sdio->bus_ops->rx(hi_sdio->bus_data);
    oal_sdio_claim_host(func);

    return HI_SUCCESS;
}




hi_s32 oal_sdio_do_isr(oal_channel_stru *hi_sdio)
{
    hi_u8                   int_mask;
    hi_s32                   ret;
    
    if (oal_unlikely(HI_TRUE != oal_sdio_get_state(hi_sdio, OAL_SDIO_RX))) {
        oam_error_log0("oal_sdio_do_isr rx disable\n");
        return HI_SUCCESS;
    }

    ret = oal_sdio_extend_buf_get(hi_sdio);
    if (oal_unlikely(ret)) {
        return -OAL_EFAIL;
    }

    ret = oal_sdio_get_func1_int_status(hi_sdio, &int_mask);
    if (oal_unlikely(ret)) {
        return ret;
    }
    
    //printf("func %s int_mask 0x%x\r\n",__func__,int_mask);
    if (oal_unlikely(0 == (int_mask & HISDIO_FUNC1_INT_MASK))) {
        //oam_error_log0("no sdio isr\n");
        return HI_SUCCESS;
    }
#if 1
    /* message interrupt, flow control */
    if (int_mask & HISDIO_FUNC1_INT_MFARM) {
        //oam_error_log0("wdp 006\n");
        if (oal_sdio_msg_irq(hi_sdio) != HI_SUCCESS) {
            return -OAL_EFAIL;
        }
    }
#endif
    if (int_mask & HISDIO_FUNC1_INT_DREADY) {
        //oam_error_log0("wdp 007\n");
        return oal_sdio_data_sg_irq(hi_sdio);
    }

    return HI_SUCCESS;
}

hi_void oal_sdio_isr(struct sdio_func *func)
{
    oal_channel_stru     *hi_sdio = HI_NULL;
    hi_s32                     ret;
    if (func == HI_NULL) {
        oam_error_log0("oal_sdio_isr func null\n");
        return;
    }

    hi_sdio = sdio_get_drvdata(func);
    if (hi_sdio == HI_NULL || hi_sdio->func == HI_NULL) {
        oam_error_log1("hi_sdio/hi_sdio->func is NULL :%p\n", (void *)hi_sdio);
        return;
    }

    oal_sdio_claim_host(hi_sdio->func);
    ret = oal_sdio_do_isr(hi_sdio);
    if (oal_unlikely(ret)) {
        oam_error_log0("oal_sdio_do_isr fail\n");
    }
    oal_sdio_release_host(hi_sdio->func);
}

static hi_s32 oal_sdio_probe(struct sdio_func *func)
{
    oal_channel_stru *hi_sdio = HI_NULL;
    // int ret;

    if (func == HI_NULL || func->card == HI_NULL) {
        oam_error_log0("oal_sdio_probe:func func->card->host ids null\n");
        return -OAL_EFAIL;
    }

    hi_sdio = oal_sdio_alloc(func);
    if (hi_sdio == HI_NULL) {
        oam_error_log0("failed to alloc hi_sdio!\n");
        goto failed_sdio_alloc;
    }

    /* $)AW"2aGPIOVP6O RT<0VP6O4&@m:/J} TODO*/

    oal_disable_sdio_state(hi_sdio, OAL_SDIO_ALL);

    if (oal_sdio_dev_init(hi_sdio) != HI_SUCCESS) {
        oam_error_log0("sdio dev init failed!\n");
        goto failed_sdio_dev_init;
    }

    return HI_SUCCESS;

failed_sdio_dev_init:
   // oal_free(hi_sdio);
failed_sdio_alloc:
    return -OAL_EFAIL;
}


hi_s32 oal_sdio_func_probe(oal_channel_stru *hi_sdio, hi_s32 sdio_dev_num)
{
    hi_s32 ret;
    // hi_s32 times = SDIO_PROBLE_TIMES;
    if (hi_sdio == HI_NULL) {
        return -OAL_EFAIL;
    }

    if(hi_sdio == NULL) {
        goto failed_sdio_reg;
    }

    ret = oal_sdio_detectcard_to_core(hi_sdio->func, sdio_dev_num);
    if (ret) {
        oam_error_log1("fail to detect sdio card, ret=%d\n", ret);
        goto failed_sdio_reg;
    }

    //oal_sdio_claim_host(hi_sdio->func);
    oal_disable_sdio_state(hi_sdio, OAL_SDIO_ALL);
    //oal_sdio_release_host(hi_sdio->func);
    return HI_SUCCESS;

failed_sdio_reg:
    return -OAL_EFAIL;
}

hi_void oal_sdio_credit_info_init(oal_channel_stru *hi_sdio)
{
    hi_sdio->sdio_credit_info.large_free_cnt = 0;
    hi_sdio->sdio_credit_info.short_free_cnt = 0;
    oal_spin_lock_init(&hi_sdio->sdio_credit_info.credit_lock);
}

oal_channel_stru *oal_sdio_init_module(hcc_handler_stru *hcc_handler, struct hcc_bus_ops *bus_ops)
{
    oal_channel_stru *hi_sdio;

    hi_sdio = (oal_channel_stru *)oal_malloc(sizeof(oal_channel_stru));
    if (hi_sdio == HI_NULL) {
        oam_error_log1("[error]func %s, line %d\n", __func__, __LINE__);     
        return HI_NULL;
    }
    memset_s(hi_sdio, sizeof(oal_channel_stru), 0, sizeof(oal_channel_stru));

    hi_sdio->rx_buf = (hi_u8 *)oal_malloc(SDIO_MAX_XFER_LEN);
    if (hi_sdio->rx_buf == HI_NULL) {
        oam_error_log1("[error]func %s, line %d\n", __func__, __LINE__);             
        goto alloc_sdio_struct_fail;
    }

    hi_sdio->tx_buf = (hi_u8 *)oal_malloc(SDIO_MAX_XFER_LEN);
    if (hi_sdio->tx_buf == HI_NULL) {
        oam_error_log1("[error]func %s, line %d\n", __func__, __LINE__);             
        goto alloc_rx_buf_fail;
    }

    oal_sdio_credit_info_init(hi_sdio);

    hi_sdio->sdio_extend = (hisdio_extend_func *)oal_malloc(sizeof(hisdio_extend_func));
    if (hi_sdio->sdio_extend == HI_NULL) {
        oam_error_log1("[error]func %s, line %d\n", __func__, __LINE__);             
        goto failed_sdio_extend_alloc;
    }
    memset_s(hi_sdio->sdio_extend, sizeof(hisdio_extend_func), 0, sizeof(hisdio_extend_func));

    hi_sdio->sdio_align_buff = oal_malloc(HISDIO_BLOCK_SIZE);
    if (hi_sdio->sdio_align_buff == HI_NULL) {
        oam_error_log1("[error]func %s, line %d\n", __func__, __LINE__);             
        goto failed_sdio_align_buff_fail;
    }

    hi_sdio->func1_int_mask = HISDIO_FUNC1_INT_MASK;
    hi_sdio->bus_data = (hi_void*)hcc_handler;

    oal_sdio_transfer_register(hi_sdio, bus_ops);

    hi_sdio->func = (struct sdio_func *)oal_malloc(sizeof(struct sdio_func));
    if (hi_sdio->func == HI_NULL) {
        oam_error_log1("[error]func %s, line %d\n", __func__, __LINE__);             
        goto failed_sdio_extend_alloc;
    }
    memset_s(hi_sdio->func,sizeof(struct sdio_func), 0, sizeof(struct sdio_func));

    hi_sdio->func->card = (sdio_card_t *)oal_malloc(sizeof(sdio_card_t));
    if (hi_sdio->func->card == HI_NULL) {
        oam_error_log1("[error]func %s, line %d\n", __func__, __LINE__);             
        goto failed_sdio_extend_alloc;
    }
    memset_s(hi_sdio->func->card, sizeof(sdio_card_t), 0, sizeof(sdio_card_t));

    hi_sdio->func->num = 1;//func1

    hi_sdio->func->tmpbuf = (unsigned char *)oal_malloc(4);
    if (hi_sdio->func->tmpbuf == HI_NULL) {
        oam_error_log1("[error]func %s, line %d\n", __func__, __LINE__);             
        goto failed_sdio_extend_alloc;
    }
    memset_s(hi_sdio->func->tmpbuf, 4, 0, 4);

    g_hi_sdio_ = hi_sdio;

    return hi_sdio;
failed_sdio_align_buff_fail:
failed_sdio_extend_alloc:
    oal_free(hi_sdio->tx_buf);
    hi_sdio->tx_buf = HI_NULL;
alloc_rx_buf_fail:
    oal_free(hi_sdio->rx_buf);
    hi_sdio->rx_buf = HI_NULL;
alloc_sdio_struct_fail:
    oal_free(hi_sdio);
    return HI_NULL;
}


hi_void  oal_sdio_exit_module(oal_channel_stru *hi_sdio)
{
    oal_free(hi_sdio->tx_buf);
    oal_free(hi_sdio->rx_buf);
    oal_free(hi_sdio->sdio_extend);
    oal_sdio_transfer_unregister(hi_sdio);
    oal_free(hi_sdio);
    g_hi_sdio_ = HI_NULL;
}

/* hi_s32 val$)A?(Q!Tq */
hi_s32 oal_sdio_init(void *data, hi_s32 sdio_dev_num, struct hcc_bus_ops *bus_ops)
{
    oal_channel_stru *hi_sdio;
    hi_s32 ret;
    hcc_handler_stru * hcc_handler = (hcc_handler_stru *)data;

    oam_info_log0("aos mutex new g_sdio_hsot_mutex start\n");
    ret = aos_mutex_new(&g_sdio_hsot_mutex);
    if (ret != 0) {
        oam_error_log1("aos mutex new g_sdio_hsot_mutex error\n");
        return -1;
    }
    ret = aos_mutex_new(&g_spin_mutex);
    if (ret != 0) {
        oam_error_log1("aos mutex new g_spin_mutex error\n");
        return -1;
    }
    ret = aos_mutex_new(&g_sdio_tx_rx_mutex);
    if (ret != 0) {
        oam_error_log1("aos mutex new g_sdio_tx_rx_mutex error\n");
        return -1;
    }
    
    aos_mutex_new(&g_sdio_cmd_mutex);
    
    oam_info_log0("aos mutex new g_sdio_hsot_mutex g_spin_mutex g_sdio_tx_rx_mutex exit\n");

    hi_sdio = oal_sdio_init_module(hcc_handler, bus_ops);
    if (hi_sdio == NULL) {
        oam_error_log0("oal_sdio_init_module: failed");
        return HI_FAILURE;
    }

    ret = oal_sdio_func_probe(hi_sdio, sdio_dev_num);
    if (ret != HI_SUCCESS) {
        oam_error_log1("regeste_sdio_drv:: oal_sdio_func_probe failed:%d", ret);
        goto oal_sdio_func_probe_fail;
    }

    ret = oal_sdio_probe(hi_sdio->func);
    if (ret != HI_SUCCESS) {
        oam_error_log1("regeste_sdio_drv:: oal_sdio_func_probe failed:%d", ret);
        goto oal_sdio_func_probe_fail;
    }

    oal_enable_sdio_state(hi_sdio, OAL_SDIO_ALL);

    hcc_handler->hi_channel = (hi_void*)hi_sdio;

    return HI_SUCCESS;
oal_sdio_func_probe_fail:
    oal_free(hi_sdio);
    return HI_FAILURE;
}

hi_void oal_sdio_exit(oal_channel_stru *hi_sdio)
{
    oal_disable_sdio_state(hi_sdio, OAL_SDIO_ALL);
    oal_sdio_exit_module(hi_sdio);
}

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif


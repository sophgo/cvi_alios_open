/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: HMAC module HCC layer adaptation.
 * Author: kongcheng
 * Create: 2018-08-04
 */

/* $)AM7ND<~0|:, */
#include "hcc_comm.h"
#include "wal_net.h"
#include "securec.h"
#include "hcc_adapt.h"
#include "oal_netbuf.h"
#include "oam_ext_if.h"
//#include "pin_name.h"
//#include "drv_gpio.h"
#include "sdio_host.h"
//#include "if_v.h"
#include <drv/pin.h>
#include <drv/gpio.h>
#include "pinctrl-mars.h"
#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* static $)A:/J}IyCw */

#define WAL_IOCTL_CMD_LEN   512
#define WAL_ETH_DATA_LEN    1518

hi_u32  hi_channel_send_msg_to_dev(hi_u8 *buf, hi_u32 len)
{
    printf("wdp--msg-%d\r\n", len);
    hi_u32 ret;
    hi_char *payload = HI_NULL;
    oal_netbuf_stru *netbuf = HI_NULL;
    oal_iw_point_stru iw_point;

    if ((buf == HI_NULL) || (len == 0)) {
        oam_error_log2("hi_channel_send_msg_to_dev:: invalid params[%p]", buf);
        return HI_FAIL;
    }
    
    iw_point.length = len;
    iw_point.pointer = buf;
    
    if (iw_point.length > WAL_IOCTL_CMD_LEN) {
        return HI_FAILURE;
    }

    //if(netbuf == NULL)
    {
        netbuf = (oal_netbuf_stru *)oal_netbuf_alloc(iw_point.length + HCC_HDR_TOTAL_LEN, HCC_HDR_TOTAL_LEN, 4);
        if (netbuf == HI_NULL) {
            oam_error_log0("oal_netbuf_alloc is error\n");
            return HI_FAILURE;
        }
    }
    oal_netbuf_put(netbuf, iw_point.length);
    payload = (hi_char *)oal_netbuf_data(netbuf);
    (hi_void)memcpy_s(payload, iw_point.length, iw_point.pointer, iw_point.length);
    oal_netbuf_next(netbuf) = HI_NULL;
    oal_netbuf_prev(netbuf) = HI_NULL;

    ret = hcc_tx_data_adapt(netbuf, HCC_TYPE_MSG, HCC_SUB_TYPE_USER_MSG);
    if (ret != HI_SUCCESS) {
        oam_error_log0("hcc_tx_data_adapt is error\n");
        oal_netbuf_free(netbuf);
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}


hi_u32  hi_channel_send_data_to_dev(hi_u8 *buf, hi_u32 len, struct pbuf *p)
{
    hi_u32 ret;
    hi_char *payload = HI_NULL;
    oal_netbuf_stru *netbuf = HI_NULL;
    oal_iw_point_stru iw_point;
    struct pbuf *q = NULL;
    hi_u32 i = 0;

    if ((buf == HI_NULL) || (len == 0)) {
        oam_error_log2("hi_channel_send_msg_to_dev:: invalid params[%p]", buf);
        return HI_FAIL;
    }
    
    iw_point.length = len;
    iw_point.pointer = buf;
    
    if (iw_point.length > WAL_ETH_DATA_LEN) {
        oam_error_log2("hi_channel_send_msg_to_dev:: len=%d", iw_point.length);
        return HI_FAILURE;
    }
    
    //if(netbuf == NULL)
    {
        netbuf = (oal_netbuf_stru *)oal_netbuf_alloc(iw_point.length + HCC_HDR_TOTAL_LEN, HCC_HDR_TOTAL_LEN, 0);
        if (netbuf == HI_NULL) {
            //oam_error_log0("oal_netbuf_alloc is error\n");
            return HI_FAILURE;
        }
    }
    //printf("wdp send_data_to_dev netbuf 0x%x\r\n", netbuf);
    oal_netbuf_put(netbuf, iw_point.length);
    payload = (hi_char *)oal_netbuf_data(netbuf);

    for (q = p; q != NULL; q = q->next)
    {
        /* Send the data from the pbuf to the interface, one pbuf at a
        time. The size of the data in each pbuf is kept in the ->len
        variable. */
        //send data from(q->payload, q->len);
        memcpy((uint8_t *)&payload[i], (uint8_t *)q->payload, q->len);
        i = i + q->len;
        if ((i > 1520) || (i > p->tot_len))
        {
            printf("low_level_output: error, len=%" U32_F ",tot_len=%" U32_F "\n\t", i, p->tot_len);
            return HI_FAILURE;
        }
    }

    if (i != p->tot_len) {
        printf("length mismatch\r\n");
        return HI_FAILURE;
    }
    
    //(hi_void)memcpy_s(payload, iw_point.length, iw_point.pointer, iw_point.length);
    oal_netbuf_next(netbuf) = HI_NULL;
    oal_netbuf_prev(netbuf) = HI_NULL;
    //aos_free(buf);
    //printf("****origin write start send data to dev*****\n");
    //dump(netbuf->data , iw_point.length);
    //printf("****origin write end send data to dev*****\n");

    ret = hcc_tx_data_adapt(netbuf, HCC_TYPE_DATA, HCC_SUB_TYPE_IP_DATA);
    if (ret != HI_SUCCESS) {
        oam_error_log0("hi_channel_send_data_to_dev is error\n");
        oal_netbuf_free(netbuf);
        return HI_FAILURE;
    }
    //oal_netbuf_free(netbuf);
    return HI_SUCCESS;
}


#define GMAC_BUF_LEN (1600 + 20)

uint8_t WalSendDataBuf[GMAC_BUF_LEN];
uint8_t WalRecvDataBuf[GMAC_BUF_LEN];

int32_t wal_net_get_packet(hi_u8 *buf, hi_u32 len)
{
    // int32_t ac_len = 0;

    if(buf == NULL) return -1;
    memcpy_s(buf, len, WalRecvDataBuf, len);

    return len;
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
#include "hcc_host.h"
extern struct netif hi3861l_netif[2];
struct pbuf * wal_rx_data_proc(oal_netbuf_stru *netbuf)
{
    uint8_t *recv_buf = NULL;
    if (netbuf == HI_NULL) {
        oam_error_log0("wal_rx_data_proc:: netdev is NULL");
        //oal_netbuf_free(netbuf);
        return NULL;
    }
    //printf("wal_rx_data_proc [0x%x]\r\n", oal_netbuf_data(netbuf));
    recv_buf = oal_netbuf_data(netbuf);
    // dump(WalRecvDataBuf, oal_netbuf_len(netbuf));

//struct netif *netif;
//netif = &hi3861l_netif[0];
      //struct ethernetif *ethernetif = netif->state;
  struct pbuf *p, *q;
  int32_t len;
  uint32_t i = 0;

  /* Obtain the size of the packet and put it into the "len"
     variable. */
  CPSR_ALLOC();
  //RHINO_CPU_INTRPT_DISABLE()
  RHINO_CPU_INTRPT_DISABLE();
//   len = csi_receive_packet(RecvDataBuf, GMAC_BUF_LEN);
len = oal_netbuf_len(netbuf);
  //RHINO_CPU_INTRPT_ENABLE()
  RHINO_CPU_INTRPT_ENABLE();

  if((len <= 0) || (len > GMAC_BUF_LEN)) {
    LWIP_DEBUGF(NETIF_DEBUG, ("low_level_input: len error. len: %d\n", len));
    return NULL;
  }

#if ETH_PAD_SIZE
  len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif

  /* We allocate a pbuf chain of pbufs from the pool. */
  p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

  if (p != NULL) {

#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

    /* We iterate over the pbuf chain until we have read the entire
     * packet into the pbuf. */
    for (q = p; q != NULL; q = q->next) {
      /* Read enough bytes to fill this pbuf in the chain. The
       * available data in the pbuf is given by the q->len
       * variable.
       * This does not necessarily have to be a memcpy, you can also preallocate
       * pbufs for a DMA-enabled MAC and after receiving truncate it to the
       * actually received size. In this case, ensure the tot_len member of the
       * pbuf is the sum of the chained pbuf len members.
       */
      memcpy((u8_t*)q->payload, (u8_t*)&recv_buf[i], q->len);
      i = i + q->len;
     // read data into(q->payload, q->len);
    }

    if((i != p->tot_len) || (i > len)) { 
        return NULL;
    }
   // acknowledge that packet has been read();

    MIB2_STATS_NETIF_ADD(netif, ifinoctets, p->tot_len);
    if (((u8_t*)p->payload)[0] & 1) {
      /* broadcast or multicast packet*/
      MIB2_STATS_NETIF_INC(netif, ifinnucastpkts);
    } else {
      /* unicast packet*/
      MIB2_STATS_NETIF_INC(netif, ifinucastpkts);
    }
#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

    LINK_STATS_INC(link.recv);
  } else {
    //drop packet();
    LWIP_DEBUGF(NETIF_DEBUG, ("drop packet.\n"));
    LINK_STATS_INC(link.memerr);
    LINK_STATS_INC(link.drop);
    MIB2_STATS_NETIF_INC(netif, ifindiscards);
  }

  return p;
    // return HI_SUCCESS;
}

typedef struct {
    volatile int counter;
} atomic_t;

#define ATOMIC_INIT(i)      { (i) }
#define atomic_read(v)      (*(volatile int *)&(v)->counter)
#define atomic_set(v,i)     (((v)->counter) = (i))
#define IDLE                0
#define WAIT_MCU_INTR       1
#define READY_SEND_DATA     2
#define ACK_FROM_MCU        3
#define TX_STATUS           5
#define RX_STATUS           6
#define PIN_MCU_TO_HOST     PC13

atomic_t flag = ATOMIC_INIT(0);

static int                  gs_init_flag        = 0;
//static gpio_pin_handle_t    gs_mcu_to_host_irq  = NULL;
extern aos_mutex_t g_sdio_hsot_mutex;
//extern uint32_t g_sdio_tx_ok_flag;
extern aos_mutex_t g_sdio_tx_rx_mutex;
//if_v_sema_t g_sem_wal_irq;
aos_sem_t    g_sem_wal_irq;
aos_sem_t    g_msgq_wal_irq;

extern oal_channel_stru *oal_get_sdio_default_handler(hi_void);
extern hi_s32 oal_sdio_do_isr(oal_channel_stru *hi_sdio);

void wal_irq_process(void *param)
{
    // hi_s32      hcc_ret = 0;
    // hcc_msg_t   msg;

    for(;;)
    {
        aos_sem_wait((aos_sem_t *)&g_sem_wal_irq, AOS_WAIT_FOREVER);
#if 0
        hcc_ret = csi_kernel_msgq_get(g_msgq_wal_irq, (void *)&msg, -1);
        if (hcc_ret != 0) {
            oam_info_log0("csi_kernel_msgq_get fail\n");
            break;
        }
#endif
        //printf("irq\n");
        //static int work_flag = 0;
        //if(!work_flag)
        {
            //printf("wal irq2\n");
            //work_flag = 1;
            //g_sdio_tx_ok_flag = 0;
#if 1
            uint32_t ret = -1;
            oal_channel_stru *hi_sdio = oal_get_sdio_default_handler();  
            if (hi_sdio == HI_NULL) {
                oam_error_log0("Failed to alloc hi_sdio!\n");
                return;
            }
            //aos_mutex_lock(&g_sdio_tx_rx_mutex, AOS_WAIT_FOREVER);
            oal_sdio_claim_host(hi_sdio->func);
            ret = oal_sdio_do_isr(hi_sdio);
            oal_sdio_release_host(hi_sdio->func);
            //aos_mutex_unlock(&g_sdio_tx_rx_mutex);
            if(ret == -1) {
                printf("[error] oal_read_from_device ret %d\r\n",ret);
            } else {
                // printf("[success] oal_read_from_device ret %d\r\n",ret);
            }
#endif
            //g_sdio_tx_ok_flag = 1;
            //work_flag = 0;
            //printf("rx int e\n");
        }
    }

}

static void wal_irq_sem_rx()
{
    //printf("sem\n");
    aos_sem_signal((aos_sem_t *)&g_sem_wal_irq);
}

// static void wal_irq_msgq_rx()
// {
//     hcc_msg_t msg;
//     memset(&msg, 0, sizeof(msg));
//     //csi_kernel_msgq_put(g_msgq_wal_irq, &msg, 0, 0);
// }

int wal_irq_config(void)
{
    int ret = 0;

    if(gs_init_flag)
    {
        printf("%s:%d alrady inited \n\r",__FUNCTION__,__LINE__);
        return -1;
    }

    // drv_pinmux_config(SDSLV_MUX,GPIO_MUX);

    //gs_mcu_to_host_irq = csi_gpio_pin_initialize(PIN_MCU_TO_HOST, /*wal_irq_msgq_rx*/wal_irq_sem_rx);
    //csi_gpio_pin_config_mode(gs_mcu_to_host_irq, GPIO_MODE_PULLUP);
    //csi_gpio_pin_config_direction(gs_mcu_to_host_irq, GPIO_DIRECTION_INPUT);
    //csi_gpio_pin_set_irq(gs_mcu_to_host_irq, GPIO_IRQ_MODE_RISING_EDGE, 1);
    static csi_gpio_t s_wifi_irqgpio = {0};
    if(gs_init_flag == 0) {
        if(gs_init_flag == 0 ) {
            PINMUX_CONFIG(PWR_WAKEUP0, PWR_GPIO_6);
            csi_gpio_init(&s_wifi_irqgpio, 4);
            csi_gpio_mode(&s_wifi_irqgpio , 1 << 6, GPIO_MODE_PULLUP);
            csi_gpio_dir(&s_wifi_irqgpio , 1 << 6, GPIO_DIRECTION_INPUT);
            csi_gpio_irq_mode(&s_wifi_irqgpio, 1 << 6, GPIO_IRQ_MODE_RISING_EDGE);
            csi_gpio_attach_callback(&s_wifi_irqgpio, wal_irq_sem_rx, NULL);
            csi_gpio_irq_enable(&s_wifi_irqgpio, 1 << 6, true);
        }
    }
    gs_init_flag = 1;
    atomic_set(&flag, IDLE);

    aos_sem_new((aos_sem_t *)&g_sem_wal_irq, 0);
    // g_msgq_wal_irq = csi_kernel_msgq_new(200, sizeof(hcc_msg_t));
    // if (NULL == g_msgq_wal_irq) {
    //     oam_error_log0("Creating g_msgq_wal_irq queue failed\n");
    //     return -1;
    // } 

    aos_task_t wal_irq_rx_thread;
    aos_task_new_ext(&wal_irq_rx_thread,"hcc_irq_task", wal_irq_process, NULL, 20*1024, 32);

    return ret;
}


#if 0
/*****************************************************************************
 $)A:/ J} C{  : wal_netdev_set_mac_addr
 $)A9&D\ChJv  : IhVCmac5XV7
 $)AJdHk2NJ}  : netdev: MxBgIh18
             addr : $)A5XV7
*****************************************************************************/
static hi_s32  wal_netdev_set_mac_addr(oal_net_device_stru *netdev, void *addr)
{
    oal_sockaddr_stru *mac_addr = HI_NULL;

    if ((netdev == HI_NULL) || (addr == HI_NULL)) {
        oam_error_log2("{wal_netdev_set_mac_addr::pst_net_dev or p_addr null ptr error %p, %p!}",
                       (uintptr_t)netdev, (uintptr_t)addr);

        return HI_FAILURE;
    }

    if (oal_netif_running(netdev)) {
        oam_warning_log0("{wal_netdev_set_mac_addr::cannot set address; device running!}");
        return HI_FAILURE;
    }

    mac_addr = (oal_sockaddr_stru *)addr;

    if (ether_is_multicast(mac_addr->sa_data)) {
        oam_warning_log0("{wal_netdev_set_mac_addr::can not set group/broadcast addr!}");
        return HI_FAILURE;
    }

    if (memcpy_s((netdev->dev_addr), WLAN_MAC_ADDR_LEN, (mac_addr->sa_data), WLAN_MAC_ADDR_LEN) != EOK) {
        oam_error_log0("{wal_netdev_set_mac_addr::mem safe function err!}");
        return HI_FAILURE;
    }

    /* 1131$)AHg9{returnTrN^7(M(9}C|AnEdVCmac5XV75=<D4fFw */
    /* when sta and ap are coexist,close the line "return HI_SUCCESS" DTS2016050607426 */
    /* set mac address,need open the following line,DTS2016102403276 */
    return HI_SUCCESS;
}

oal_net_device_ops_stru* wal_get_net_dev_ops(hi_void)
{
    return &g_wal_net_dev_ops;
}
#endif

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

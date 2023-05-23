/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <aos/cli.h>
#include <ulog/ulog.h>
#include <aos/kernel.h>

#include "netif/etharp.h"
#include <lwip/netif.h>
#include <lwip/netifapi.h>
#include <devices/netdrv.h>
#include <uservice/uservice.h>
#include <yoc/netmgr.h>
#include <yoc/netmgr_service.h>

#include "usbh_core.h"
#include "usbh_rndis.h"
#include "rndis_protocol.h"
#include "usbh_serial.h"

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

#define RNDIS_MSG_QUEUE_NUM 1000

#define RNDIS_MSG_TPYE_INPUT  0
#define RNDIS_MSG_TPYE_OUTPUT 1

#define RNDIS_BUFF_SIZE         2048
#define TAG "rndis"


typedef struct rndis_msg {
    int      type;
    uint8_t *buf;
    uint32_t len;
} rndis_msg_t;


struct netif           rndis_netif;
static struct usbh_urb rndis_bulkin_urb;
static struct usbh_urb rndis_bulkout_urb;
static aos_queue_t     s_rndis_msg_queue;
static rndis_msg_t     s_queue_msg[RNDIS_MSG_QUEUE_NUM];


static void low_level_init(struct netif *netif)
{
    //uint8_t mac[6] = { 0xf4, 0x12, 0xfa, 0xe5, 0x23, 0xf0 };
    uint8_t mac[6] = {0xf4, 0x12, 0xfa, 0xe5, 0x12, 0xb0};
    netif->hwaddr_len = ETH_ALEN;

    memcpy(&netif->hwaddr[0], mac, ETH_ALEN);

    /* Maximum transfer unit */
    netif->mtu = 1500;

#if LWIP_IPV6_AUTOCONFIG
    netif_set_ip6_autoconfig_enabled(netif, 1);
#endif
    /* device capabilities */
    /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
    netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP
#if defined(DHCP_USED)
                    | NETIF_FLAG_DHCP
#endif
        ;
}

static struct pbuf *low_level_input(struct netif *netif, uint8_t *buf, uint32_t buf_len)
{
    struct pbuf *p = NULL, *q = NULL;
    u16_t        s_len;
    u8_t *       bufptr;

    /* Obtain the size of the packet and put it into the "len"
     * variable. */

    if (buf_len < sizeof(rndis_data_packet_t)) {
        return NULL;
    }

    s_len  = buf_len - sizeof(rndis_data_packet_t);
    bufptr = buf + sizeof(rndis_data_packet_t);

#if ETH_PAD_SIZE
    s_len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif

    /* We allocate a pbuf chain of pbufs from the pool. */
    p = pbuf_alloc(PBUF_RAW, s_len, PBUF_RAM);

    if (p != NULL) {
#if ETH_PAD_SIZE
        pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

        /* Iterate over the pbuf chain until we have read the entire
         * packet into the pbuf. */
        for (q = p; q != NULL; q = q->next) {
            /* Read enough bytes to fill this pbuf in the chain. The
             * available data in the pbuf is given by the q->len
             * variable. */
            /* read data into(q->payload, q->len); */
            memcpy(q->payload, bufptr, q->len);
            bufptr += q->len;
        }
        /* Acknowledge that packet has been read(); */

#if ETH_PAD_SIZE
        pbuf_header(p, ETH_PAD_SIZE); /* Reclaim the padding word */
#endif

        LINK_STATS_INC(link.recv);
    } else {
        /* Drop packet(); */
        LINK_STATS_INC(link.memerr);
        LINK_STATS_INC(link.drop);
    }

    return p;
}

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
    struct pbuf *        q       = NULL;
    int                  datalen = 0;
    uint8_t *            buf, *packate;
    rndis_msg_t          msg;
    rndis_data_packet_t *hdr;

    int ret;

    packate = malloc(p->tot_len + sizeof(rndis_data_packet_t));

    if (packate == NULL) {
        printf("packate == NULL\n");
        return ERR_MEM;
    }

    buf = packate + sizeof(rndis_data_packet_t);

#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* Drop the padding word */
#endif

    for (q = p; q != NULL; q = q->next) {
        /* Send data from(q->payload, q->len); */
        memcpy(buf + datalen, q->payload, q->len);
        datalen += q->len;
    }

    hdr = (rndis_data_packet_t *)packate;

    memset(hdr, 0, sizeof(rndis_data_packet_t));
    hdr->MessageType   = REMOTE_NDIS_PACKET_MSG;
    hdr->MessageLength = sizeof(rndis_data_packet_t) + p->tot_len;
    hdr->DataOffset    = sizeof(rndis_data_packet_t) - sizeof(rndis_generic_msg_t);
    hdr->DataLength    = p->tot_len;

    msg.buf  = packate;
    msg.len  = p->tot_len + sizeof(rndis_data_packet_t);
    msg.type = RNDIS_MSG_TPYE_OUTPUT;

    ret = aos_queue_send(&s_rndis_msg_queue, &msg, sizeof(rndis_msg_t));

    if (ret) {
        aos_debug_printf("low_level_output queue send error\n");
        return ERR_ABRT;
    }

#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE); /* Reclaim the padding word */
#endif

    LINK_STATS_INC(link.xmit);

    return ERR_OK;
}

static err_t rndis_netif_init(struct netif *netif)
{
	printf("Entry %s \n", __func__);
    netif->output = etharp_output;
#if LWIP_IPV6
    netif->output_ip6 = ethip6_output;
#endif /* LWIP_IPV6 */
    netif->linkoutput = low_level_output;

    /* initialize the hardware */
    low_level_init(netif);

    return ERR_OK;
}

static int rndis_data_rx(void *buf, int nbytes)
{
    struct netif *netif = &rndis_netif;
    struct pbuf * p;

    // printf("%s ------------- %d\n", __FUNCTION__, nbytes);
    if (nbytes > 0) {
        p = low_level_input(&rndis_netif, buf, nbytes);
        if (p) {
            if (ERR_OK != netif->input(p, netif)) {
                aos_debug_printf("ethernetif_input error\n");
                pbuf_free(p);
                p = NULL;
            }
            return 0;
        } else {
            return -1;
        }
    }

    return -1;
}

int rndis_data_tx(void *data, uint32_t len)
{
    int                ret;
    struct usbh_rndis *rndis_class = (struct usbh_rndis *)usbh_find_class_instance("/dev/rndis");

    if (rndis_class == NULL) {
        printf("%s rndis_class is NULL\n", __func__);
        return -1;
    }

    // printf("%s     =========   %d\n",__FUNCTION__,len);

    rndis_bulkout_urb.type = USBH_APP_RNDIS;
    usbh_bulk_urb_fill(&rndis_bulkout_urb, rndis_class->bulkout, data, len, 1000, NULL, NULL);
    ret = usbh_submit_urb(&rndis_bulkout_urb);

    if (ret < 0) {
        aos_debug_printf("bulk out error,ret:%d\n", ret);
        return -1;
    }

    return 0;
}

static void rndis_data_input(void *data, int size)
{
    int                ret;
    rndis_msg_t        msg;
    struct usbh_rndis *rndis_class = (struct usbh_rndis *)usbh_find_class_instance("/dev/rndis");

    if (rndis_class == NULL) {
        printf("%s rndis_class is NULL\n", __func__);
        return;
    }

    msg.buf  = data;
    msg.len  = size;
    msg.type = RNDIS_MSG_TPYE_INPUT;

    ret = aos_queue_send(&s_rndis_msg_queue, &msg, sizeof(rndis_msg_t));

    void *buf = malloc(RNDIS_BUFF_SIZE);

    if (ret) {
        aos_debug_printf("queue send error,ret:%d\n", ret);
        return;
    }

    rndis_bulkout_urb.type = USBH_APP_RNDIS;
    usbh_bulk_urb_fill(&rndis_bulkin_urb, rndis_class->bulkin, buf, RNDIS_BUFF_SIZE, 0, rndis_data_input, buf);
    ret = usbh_submit_urb(&rndis_bulkin_urb);
    if (ret < 0) {
        printf("bulk out error,ret:%d\r\n", ret);
        return;
    }
}

static void rndis_task_entry(void *argv)
{
    rndis_msg_t        msg;
    size_t             size;
    int                ret;
    void *             data        = malloc(RNDIS_BUFF_SIZE);
    struct usbh_rndis *rndis_class = (struct usbh_rndis *)usbh_find_class_instance("/dev/rndis");

    if(rndis_class == NULL){
        printf("%s rndis_class is NULL\n", __func__);
        return ;
    }
    usbh_bulk_urb_fill(&rndis_bulkin_urb, rndis_class->bulkin, data, RNDIS_BUFF_SIZE, 0, rndis_data_input, data);
    ret = usbh_submit_urb(&rndis_bulkin_urb);
    if (ret < 0) {
        printf("bulk out error,ret:%d\r\n", ret);
        return ;
    }

    while (1) {
        aos_queue_recv(&s_rndis_msg_queue, AOS_WAIT_FOREVER, &msg, &size);

        if (msg.type == RNDIS_MSG_TPYE_INPUT) {
            rndis_data_rx(msg.buf, msg.len);

            if (msg.buf) {
                free(msg.buf);
                msg.buf = NULL;
            }
        } else if (msg.type == RNDIS_MSG_TPYE_OUTPUT) {
            rndis_data_tx(msg.buf, msg.len);

            if (msg.buf) {
                free(msg.buf);
                msg.buf = NULL;
            }
        } else {
            printf("Wrong Type");
        }
    }
}

static void esp32_rndis_start_dhcp()
{
    int dhcp_en = 1;
    netmgr_hdl_t hdl_eth = netmgr_get_handle("eth");
    if (hdl_eth) {
        netmgr_ipconfig(hdl_eth, dhcp_en, NULL, NULL, NULL);
    } else {
        printf("rndis device not found!\n");
    }
}

static void esp32_rndis_retrigger(void)
{
    int                ret;
    void *             data        = malloc(RNDIS_BUFF_SIZE);
    struct usbh_rndis *rndis_class = (struct usbh_rndis *)usbh_find_class_instance("/dev/rndis");

    if(rndis_class == NULL){
        printf("%s rndis_class is NULL\n", __func__);
        return ;
    }

    usbh_bulk_urb_fill(&rndis_bulkin_urb, rndis_class->bulkin, data, RNDIS_BUFF_SIZE, 0, rndis_data_input, data);
    ret = usbh_submit_urb(&rndis_bulkin_urb);
    if (ret < 0) {
        printf("bulk out error,ret:%d\r\n", ret);
        return ;
    }

    esp32_rndis_start_dhcp();
    printf("%s done!\n", __func__);
}

static void esp32_link_status(void *argv)
{
    static int task_created = 0;
    static int disconnect_flag = 0;
    struct usbh_rndis *rndis_class;

    while (1) {
        rndis_class = (struct usbh_rndis *)usbh_find_class_instance("/dev/rndis");

        if (rndis_class) {
            if (!task_created) {
                task_created = 1;
                printf("rndis create trx task\n");
                aos_task_t task_handle;
                aos_task_new_ext(&task_handle, "rndis_trx", rndis_task_entry, NULL, 10*1024, AOS_DEFAULT_APP_PRI - 1);
                netif_set_link_up(&rndis_netif);
                esp32_rndis_start_dhcp();
            }

            if (disconnect_flag) {
                disconnect_flag = 0;
                printf("rndis rx retrigger\n");
                esp32_rndis_retrigger();
            }
        }
        else {
            if (task_created && !disconnect_flag) {
                disconnect_flag = 1;
                printf("rndis disconnect.\n");
            }
        }

        aos_msleep(100);
    }
}

void esp32_rndis_net_init(void)
{
    aos_queue_new(&s_rndis_msg_queue, s_queue_msg, sizeof(s_queue_msg), sizeof(rndis_msg_t));

    aos_task_t task_handle;
    aos_task_new_ext(&task_handle, "esp32_status", esp32_link_status, NULL, 2048, AOS_DEFAULT_APP_PRI);

    netmgr_hdl_t app_netmgr_hdl;
    app_netmgr_hdl = netmgr_dev_eth_init();

    if (app_netmgr_hdl) {
        printf("app_netmgr_hdl is ok\r\n");
        utask_t *task = utask_new("netmgr", 10 * 1024, QUEUE_MSG_COUNT, AOS_DEFAULT_APP_PRI);
        netmgr_service_init(task);
        netmgr_start(app_netmgr_hdl);
    }

    printf("[%s] init done.\n", __func__);
}

void esp32_rndis_lwip_init(void)
{
    tcpip_init(NULL, NULL);

    rndis_netif.name[0] = 'r';
    rndis_netif.name[1] = '0';
    netifapi_netif_add(&rndis_netif, IPADDR_ANY, IPADDR_ANY, IPADDR_ANY, NULL, rndis_netif_init, tcpip_input);

    netifapi_netif_set_default(&rndis_netif);

    netifapi_netif_set_up(&rndis_netif);
}

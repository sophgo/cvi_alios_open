#include "platform_os.h"
#include "esnet_wifi.h"
#include "drv_espi.h"
#include "netvreg.h"
#include "lwip/tcpip.h"
#include "lwip/etharp.h"
#if CONFIG_ZOWEE_VIDEO_DOORBELL
#include "event_def.h"
#include <uservice/uservice.h>
#include <uservice/eventid.h>
#endif

typedef struct {
    struct netif netif[ESNET_WIFI_MAXIF];
    platform_queue_t *queuehandle;
    platform_thread_t *task;
} esnet_wifi_priv_t;

static esnet_wifi_priv_t g_esnet;

static esnet_wifi_priv_t *esnet_wifi_get_priv(void)
{
    return &g_esnet;
}

static struct netif *esnet_wifi_get_net(esnet_wifi_if_e idx)
{
    esnet_wifi_priv_t *priv = esnet_wifi_get_priv();

    if (IS_VALID_NETIF(idx) == 0) {
        platform_log_e("%s[%d] get net with idx%d failed\n", __FUNCTION__, __LINE__, idx);
        return NULL;
    }

    return &priv->netif[idx];
}

static int esnet_wifi_sta_recv(unsigned char *data, unsigned int len)
{
    struct netif *netif = esnet_wifi_get_net(ESNET_WIFI_STATION);
    struct pbuf *nbuf = pbuf_alloc(PBUF_TRANSPORT, len - DRV_ESSPI_DATA_OFFSET, PBUF_RAM);
    unsigned int offset = DRV_ESSPI_DATA_OFFSET;
    struct pbuf *tbuf = nbuf;

    if (nbuf == NULL) {
        platform_log_e("%s[%d] alloc pbuf failed\n", __FUNCTION__, __LINE__);
        return -1;
    }

    while (tbuf != NULL) {
        platform_memory_copy(tbuf->payload, data + offset, tbuf->len);
        offset += tbuf->len;
        tbuf = tbuf->next;
    }

    if (netif != NULL) {
        if (netif->input(nbuf, netif) != 0) {
            pbuf_free(nbuf);
        }
    } else {
        pbuf_free(nbuf);
    }

    platform_memory_free(data);

    return 0;
}

static signed char esnet_wifi_sta_send(struct netif *net, struct pbuf *nbuf)
{
    unsigned int len = nbuf->tot_len + DRV_ESSPI_DATA_OFFSET;
    unsigned int offset = DRV_ESSPI_DATA_OFFSET;
    unsigned char *sbuf = platform_memory_alloc(len);

    if (sbuf == NULL || nbuf == NULL) {
        platform_log_e("%s[%d] buff error\n", __FUNCTION__, __LINE__);
        return -1;
    }

    while (nbuf != NULL) {
        platform_memory_copy(sbuf + offset, nbuf->payload, nbuf->len);
        offset += nbuf->len;
        nbuf = nbuf->next;
    }

    drv_espi_sendto_peer(sbuf, len);

    return 0;
}

void esnet_wifi_config_station(void)
{
    vnet_reg_t *vreg = vnet_reg_get_addr();
    esnet_if_t *netif = esnet_wifi_get_net(ESNET_WIFI_STATION);
    unsigned char mac[ESNET_HWADDR_MAX] = {0};
    unsigned char *tmpchar = NULL;
    int idx;

    if (vreg->status == 0) {
        esnet_platform_set_ip(netif, 0, 0, 0);
        esnet_platform_net_down(netif);
        return;
    }

    tmpchar = (unsigned char *)&vreg->macAddr0;
    for (idx = 0; idx < NETIF_MAX_HWADDR_LEN; idx++) {
        if (idx < NETIF_MAX_HWADDR_LEN/2) {
            mac[idx] = tmpchar[NETIF_MAX_HWADDR_LEN/2 - idx - 1];
        } else {
            mac[idx] = tmpchar[NETIF_MAX_HWADDR_LEN + NETIF_MAX_HWADDR_LEN/2 - idx];
        }
    }

    memcpy(netif->hwaddr, mac, NETIF_MAX_HWADDR_LEN);
    netif->hwaddr_len = NETIF_MAX_HWADDR_LEN;
    esnet_platform_set_ip(netif, ntohl(vreg->ipv4Addr), ntohl(vreg->ipv4Mask), ntohl(vreg->gw0));
    netif->linkoutput = esnet_wifi_sta_send;
    esnet_platform_net_up(netif);
}

int esnet_wifi_send_msg(esnet_wifi_msg_t *msg)
{
    esnet_wifi_priv_t *priv = esnet_wifi_get_priv();

    if (priv->queuehandle) {
        return platform_queue_send(priv->queuehandle, (char *)msg, sizeof(esnet_wifi_msg_t), 0);
    } else {
        platform_log_e("%s[%d] queue may not init\n", __FUNCTION__, __LINE__);
        return -1;
    }
}

static err_t esnet_wifi_level_init(struct netif *netif)
{
    err_t status = ERR_OK;

#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "esnet";
#endif

    netif->name[0] = 'e';
    netif->name[1] = 's';

    netif->output = etharp_output;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP | NETIF_FLAG_IGMP | NETIF_FLAG_MLD6;
    netif->hwaddr_len = ESNET_HWADDR_MAX;
    netif->mtu = 1500;
    netif->linkoutput = esnet_wifi_sta_send;

    return status;
}

static int esnet_wifi_netif_init(void)
{
    struct netif *netif = esnet_wifi_get_net(ESNET_WIFI_STATION);
    // custom may init tcpip stack already, please ensure this handle not crash
    tcpip_init(NULL, NULL);
    if (netif_add(netif, NULL, NULL, NULL, NULL, esnet_wifi_level_init, tcpip_input) == NULL) {
        return -1;
    }

    return 0;
}

static int esnet_wifi_handle_int(unsigned int intnum)
{
#if CONFIG_ZOWEE_VIDEO_DOORBELL
    /* set intnum offset */
    intnum = intnum + EVENT_USER;
    if ((intnum < ESNET_WIFI_INTERRUPT_LINK_CS) || (intnum > ESNET_WIFI_INTERRUPT_MAX)) {
        platform_log_i("intnum: %d, intnum is  is invalid\n", intnum);
        return -1;
    }
#endif

    if (intnum == ESNET_WIFI_INTERRUPT_LINK_CS) {
        vnet_reg_t *vreg = vnet_reg_get_addr();
        unsigned int len = sizeof(vnet_reg_t) + DRV_ESSPI_DATA_OFFSET;
        unsigned char *rbuff = platform_memory_alloc(len + DRV_ESSPI_DATA_OFFSET);

        if (rbuff == NULL) {
            platform_log_e("%s[%d] alloc rbuff failed\n", __FUNCTION__, __LINE__);
            return -1;
        }
        drv_espi_read_info(0, rbuff, len);

        platform_memory_copy(vreg, rbuff + DRV_ESSPI_DATA_OFFSET, sizeof(vnet_reg_t));
        platform_memory_free(rbuff);
        esnet_wifi_config_station();
    } else {
#if CONFIG_ZOWEE_VIDEO_DOORBELL
        event_publish(intnum, NULL);
#endif
        platform_log_i("intnum %d\n", intnum);
    }

    return 0;
}
extern void eswin_rev_msg_handle(unsigned char *msg, unsigned int len);
static int esnet_wifi_handle_msg(unsigned char *msg, unsigned int len)
{
    int idx;
    unsigned char *pdata = msg;

    platform_log_d("\n###########recv msg len %d##############\n", len - DRV_ESSPI_DATA_OFFSET);
    for (idx = DRV_ESSPI_DATA_OFFSET; idx < len; idx++) {
        platform_log_i("0x%02x ", pdata[idx]);
        if ((idx + 1) % 16 == 0) {
            platform_log_i("\n");
        }
    }

    platform_log_i("\n");
    platform_log_i("###############################################\n");
    //platform_memory_free(msg);
    eswin_rev_msg_handle(msg, len);

    return 0;
}

static void esnet_wifi_task(void *arg)
{
    esnet_wifi_priv_t *priv = (esnet_wifi_priv_t *)arg;
    esnet_wifi_msg_t msg = {0};
    int ret;

    esnet_wifi_netif_init();
    do {
        ret = platform_queue_receive(priv->queuehandle, (char *)&msg, sizeof(esnet_wifi_msg_t), PLATFORM_WAIT_FOREVER);
        if (ret < 0) {
            platform_log_e("%s[%d] queue receive failed %d\n", __FUNCTION__, __LINE__, ret);
            break;
        }

        switch (msg.msgType) {
            case DRV_ESPI_TYPE_INT:
                esnet_wifi_handle_int(msg.msgValue);
                break;
            case DRV_ESPI_TYPE_STOH:
                esnet_wifi_sta_recv(msg.msgAddr, msg.msgValue);
                break;
            case DRV_ESPI_TYPE_MSG:
                esnet_wifi_handle_msg(msg.msgAddr, msg.msgValue);
                break;
        }
    } while (1);
}

void esnet_wifi_init(void)
{
    esnet_wifi_priv_t *priv = esnet_wifi_get_priv();

    platform_memory_set(priv, 0, sizeof(esnet_wifi_priv_t));
    priv->queuehandle = platform_queue_create("esnetQueue", ESNET_WIFI_QUEUE_DEPTH, sizeof(esnet_wifi_msg_t), 0);
    priv->task = platform_thread_init("esnetTask", ESNET_WIFI_TASK_PRI, ESNET_WIFI_TASK_STACK, esnet_wifi_task, priv);
}

void esnet_wifi_exit(void)
{
    esnet_wifi_priv_t *priv = esnet_wifi_get_priv();

    platform_queue_destory(priv->queuehandle);
    platform_thread_destory(priv->task);
}

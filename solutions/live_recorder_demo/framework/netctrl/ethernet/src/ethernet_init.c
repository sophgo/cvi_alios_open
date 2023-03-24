#include "aos/kernel.h"
#include "k_api.h"
#include "lwip/etharp.h"
#include "lwip/tcpip.h"
#include "lwip/netifapi.h"
#include "ulog/ulog.h"
#include "ethernetif.h"
#include "cvi_mac_phy.h"
#include "ntp.h"
#include <cx/cloud_lv_service.h>

#define TAG "ethernet"

/*Static IP ADDRESS*/
#ifndef IP_ADDR0
#define IP_ADDR0   192
#define IP_ADDR1   168
#define IP_ADDR2   1
#define IP_ADDR3   3
#endif
/*NETMASK*/
#ifndef NETMASK_ADDR0
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0
#endif
/*Gateway Address*/
#ifndef GW_ADDR0
#define GW_ADDR0   192
#define GW_ADDR1   168
#define GW_ADDR2   1
#define GW_ADDR3   1
#endif

struct netif xnetif; /* network interface structure */

#if 1
extern cxlv_hdl_t cloud_lv_handle;

static void ntp_task(void *arg)
{
    while(1) {
        if(ntp_sync_time(NULL) == 0) {
            break;
        }
        aos_msleep(1000);
    }
}

#if 0
static int iot_connect_handle(void)
{
    return 0;
}

static int iot_disconnect_handle(void)
{
    return 0;
}
#endif
static void cx_cloud_task(void *arg)
{
    cloud_lv_handle = cx_lv_start("linkvisual", NULL);
    cx_lv_connect(cloud_lv_handle);
}
#endif

void net_status_callback(struct netif *netif)
{
#if 1
    aos_task_t ntp_handle;
    aos_task_t cx_cloud_handle;
    if (!ip_addr_isany(ip_2_ip4(&netif->ip_addr))) {
        // event_publish(EVENT_NET_GOT_IP, NULL);
		LOGD(TAG, "IP address: %s", ipaddr_ntoa((ip4_addr_t *)ip_2_ip4(&netif->ip_addr)));
        LOGD(TAG, "Subnet mask: %s", ipaddr_ntoa((ip4_addr_t *)ip_2_ip4(&netif->netmask)));
        LOGD(TAG, "Default gateway:  %s", ipaddr_ntoa((ip4_addr_t *)ip_2_ip4(&netif->gw)));
        aos_task_new_ext(&ntp_handle, "ntp_task", ntp_task, NULL, 2048, AOS_DEFAULT_APP_PRI + 10);
        aos_task_new_ext(&cx_cloud_handle, "cx_cloud_task", cx_cloud_task, NULL, 8192, AOS_DEFAULT_APP_PRI + 10);
    }
#endif
}
static void eth_init_func(void *paras)
{
	ip_addr_t ipaddr;
    ip_addr_t netmask;
    ip_addr_t gw;
    int8_t idx = 0;
    /* Create tcp_ip stack thread */
    tcpip_init(NULL, NULL);

	IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
	IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
	IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);


	xnetif.name[0] = 'r';
	xnetif.name[1] = '0' + idx;
	netif_add(&xnetif, &ipaddr, &netmask, &gw, NULL, ethif_init, tcpip_input);

    /*  Registers the default network interface. */
    netif_set_default(&xnetif);

    /*  When the netif is fully configured this function must be called.*/
	netif_set_link_up(&xnetif);
	netif_set_up(&xnetif);

	// netif_set_ipaddr(&xnetif, NULL);
    // netif_set_netmask(&xnetif, NULL);
    // netif_set_gw(&xnetif[0], NULL);
	netif_set_status_callback(&xnetif, net_status_callback);
    LOGD(TAG, "DHCP start");
	netifapi_dhcp_start(&xnetif);
}

void ethernet_init(void)
{
    int ret;
    aos_task_t eth_hdl;

    csi_eth_mac_phy_init();
    ret = aos_task_new_ext(&eth_hdl, "eth_init", eth_init_func, NULL, 8192, AOS_DEFAULT_APP_PRI+1);
    if (RHINO_SUCCESS != ret) {
        printf("gmac_init task create fail!\n");
    }
}

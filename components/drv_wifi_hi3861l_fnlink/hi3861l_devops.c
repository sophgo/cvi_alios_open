#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lwip/apps/dhcps.h>
#include <lwip/netifapi.h>
#include <lwip/netif.h>
#include <lwip/dns.h>
#include <lwip/def.h>
#include <lwip/mem.h>
#include <lwip/pbuf.h>
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include <lwip/ethip6.h>
#include <lwip/etharp.h>
#include <netif/etharp.h>
#include <netif/ethernet.h>
#include <aos/kernel.h>
#include <k_api.h>
#include <devices/wifi.h>
#include <devices/hal/wifi_impl.h>
#include <devices/hal/net_impl.h>
#include <devices/netdrv.h>
#include <devices/driver.h>
#include "wal_net.h"
#include "hi3861l_devops.h"
#include "hi_types_base.h"
#include "oam_ext_if.h"
#include "wal_net.h"
#include "hcc_adapt.h"

#define GMAC_BUF_LEN (1500 + 20)
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

struct netif hi3861l_netif[2];

uint8_t SendDataBuf[GMAC_BUF_LEN];
uint8_t RecvDataBuf[GMAC_BUF_LEN];

hi3861l_wlan_dev_t g_wlan_dev;
uint8_t g_wifi_got_ip = 0;
static event_callback_t s_MsgrecvCallBack;

typedef struct {
	aos_dev_t   device;
	uint8_t mode;
	void (*write_event)(aos_dev_t *dev, int event_id, void *priv);
	void *priv;
} wifi_dev_t;

char host_cmd[][MAX_CMD_LEN] = {
	"cmd_get_mac",
	"cmd_get_ip",
	"cmd_set_filter",
	"cmd_get_rssi",
	"cmd_connect",
	"cmd_start_sta",
	"cmd_report_wpa_event", // 11
};

static int hi3861l_init(aos_dev_t *dev)
{
	int ret = 0;
	wal_irq_config();
	ret = hcc_adapt_init();
	if (ret != HI_SUCCESS) {
		goto hcc_host_init_fail;
	}
	return HI_SUCCESS;
hcc_host_init_fail:
	return -HI_FAILURE;
}

static int hi3861l_deinit(aos_dev_t *dev)
{
	return 0;
}

static int hi3861l_reset(aos_dev_t *dev)
{
	return 0;
}

static int hi3861l_set_mode(aos_dev_t *dev, wifi_mode_t mode)
{
	uint32_t ret = -1;

	unsigned char cdata[2] = {0};
	cdata[0] = HOST_CMD_START_STA;
	ret = hi_channel_send_msg_to_dev(cdata, 1);
	if (ret == -1) {
		printf("[error] oal_start_sta ret %d\r\n", ret);
	} else {
		printf("[success] oal_start_sta ret %d\r\n", ret);
	}
	return 0;
}

static int hi3861l_get_mode(aos_dev_t *dev, wifi_mode_t *mode)
{
	return 0;
}

static int hi3861l_install_event_cb(aos_dev_t *dev, wifi_event_func *evt_func)
{
	return WIFI_ERR_OK;
}

static int hi3861l_set_protocol(aos_dev_t *dev, uint8_t protocol_bitmap)
{
	return WIFI_ERR_OK;
}

static int hi3861l_get_protocol(aos_dev_t *dev, uint8_t *protocol_bitmap)
{
	return WIFI_ERR_OK;
}

static int hi3861l_set_country(aos_dev_t *dev, wifi_country_t country)
{
	return WIFI_ERR_OK;
}

static int hi3861l_get_country(aos_dev_t *dev, wifi_country_t *country)
{
	return WIFI_ERR_OK;
}

static int hi3861l_set_auto_reconnect(aos_dev_t *dev, bool en)
{
	return WIFI_ERR_OK;
}

static int hi3861l_get_auto_reconnect(aos_dev_t *dev, bool *en)
{
	return WIFI_ERR_OK;
}

static int hi3861l_set_lpm(aos_dev_t *dev, wifi_lpm_mode_t mode)
{
	return WIFI_ERR_OK;
}

static int hi3861l_get_lpm(aos_dev_t *dev, wifi_lpm_mode_t *mode)
{
	return WIFI_ERR_OK;
}

static int hi3861l_power_on(aos_dev_t *dev)
{
	return WIFI_ERR_OK;
}

static int hi3861l_power_off(aos_dev_t *dev)
{
	return WIFI_ERR_OK;
}

static int hi3861l_start_scan(aos_dev_t *dev, wifi_scan_config_t *config, bool block)
{
	return WIFI_ERR_OK;
}

#define TLV_HDR_LEN 4
#define CMD_CONNECT_TAG_SSID     0
#define CMD_CONNECT_TAG_AUTH     1
#define CMD_CONNECT_TAG_KEY      2
#define CMD_CONNECT_TAG_BSSID    3
#define CMD_CONNECT_TAG_PAIRWISE 4
#define CMD_TLV_FORMAT   "%02x%02lx%s"
static void hi3861l_start_sta(wifi_config_t *config)
{
	char *cmd, *tmp;
	unsigned long len;
	unsigned char bssid[6] = {0};

	len = 1 + 3 + strlen(config->ssid) + strlen(config->password) + 3 + sizeof(bssid);
	cmd = aos_malloc(len);
	tmp = cmd;
	if (cmd != NULL) {
		memset(tmp, 0, len);
		unsigned int offset = 0;
		cmd[0] = HOST_CMD_CONNECT;
		cmd[offset + 1] = 1;
		cmd[offset + 2] = strlen(config->ssid) % 256;
		cmd[offset + 3] = strlen(config->ssid) / 256;
		memcpy(&cmd[offset + 4], config->ssid, strlen(config->ssid));
		offset = offset + 3 + strlen(config->ssid);

		cmd[offset + 1] = 3;
		cmd[offset + 2] = strlen(config->password) % 256;
		cmd[offset + 3] = strlen(config->password) / 256;
		memcpy(&cmd[offset + 4], config->password, strlen(config->password));
		offset = offset + 3 + strlen(config->password);

		offset += 1;
		hi_channel_send_msg_to_dev((void *)cmd, offset); //strlen(cmd)
		aos_free(cmd);
		return;
	}
}

static int hi3861l_start(aos_dev_t *dev, wifi_config_t *config)
{
	hi3861l_set_mode(dev, WIFI_MODE_STA);
	hi3861l_start_sta(config);
	return WIFI_ERR_OK;
}

static int hi3861l_stop(aos_dev_t *dev)
{
	return WIFI_ERR_OK;
}

static int hi3861l_sta_get_link_status(aos_dev_t *dev, wifi_ap_record_t *ap_info)
{
	uint8_t ssid_len = 0;
	if (g_wifi_got_ip)
		ap_info->link_status = WIFI_STATUS_GOTIP;
	else
		ap_info->link_status = WIFI_STATUS_LINK_UP;

	ssid_len = strlen(g_wlan_dev.ssid);
	memcpy(ap_info->ssid, g_wlan_dev.ssid, ssid_len);
	memcpy(ap_info->bssid, g_wlan_dev.bssid, HI_WIFI_MAC_LEN);
	ap_info->channel = g_wlan_dev.channel;
	ap_info->rssi = g_wlan_dev.rssi;

	return WIFI_ERR_OK;
}

static int hi3861l_ap_get_sta_list(aos_dev_t *dev, wifi_sta_list_t *sta)
{
	return WIFI_ERR_OK;
}

static int hi3861l_set_mac_addr(aos_dev_t *dev, const uint8_t *mac)
{
	return WIFI_ERR_OK;
}

static int hi3861l_get_mac_addr(aos_dev_t *dev, uint8_t *mac)
{
	unsigned char cdata[2] = {0};
	cdata[0] = HOST_CMD_GET_MAC;
	hi_channel_send_msg_to_dev(cdata, 1);
	aos_msleep(30);
	memcpy(mac, g_wlan_dev.hw_addr, HI_WIFI_MAC_LEN);
	return WIFI_ERR_OK;
}

int hi3861l_start_mgnt_monitor(aos_dev_t *dev, wifi_mgnt_cb_t cb)
{
	return WIFI_ERR_OK;
}

int hi3861l_stop_mgnt_monitor(aos_dev_t *dev)
{
	return WIFI_ERR_OK;
}

static int hi3861l_start_monitor(aos_dev_t *dev, wifi_promiscuous_cb_t cb)
{
	return WIFI_ERR_OK;
}

static int hi3861l_stop_monitor(aos_dev_t *dev)
{
	return WIFI_ERR_OK;
}

static int hi3861l_send_80211_raw_frame(aos_dev_t *dev, void *buffer, uint16_t len)
{
	return WIFI_ERR_OK;
}

static int hi3861l_set_channel(aos_dev_t *dev, uint8_t primary, wifi_second_chan_t second)
{
	return WIFI_ERR_OK;
}

static int hi3861l_get_channel(aos_dev_t *dev, uint8_t *primary, wifi_second_chan_t *second)
{
	return WIFI_ERR_OK;
}

static int hi3861l_set_smartcfg(aos_dev_t *dev, int enable)
{
	return WIFI_ERR_OK;
}

static int hi3861l_set_dns_server(aos_dev_t *dev, ip_addr_t ipaddr[], uint32_t num)
{
	int n, i;

	n = MIN(num, DNS_MAX_SERVERS);

	for (i = 0; i < n; i ++) {
		dns_setserver(i, &ipaddr[i]);
	}

	return n;
}

static int hi3861l_get_dns_server(aos_dev_t *dev, ip_addr_t ipaddr[], uint32_t num)
{
	int n, i;

	n = MIN(num, DNS_MAX_SERVERS);

	for (i = 0; i < n; i ++) {
		if (!ip_addr_isany(dns_getserver(i))) {
			memcpy(&ipaddr[i], dns_getserver(i), sizeof(ip_addr_t));
		} else {
			return i;
		}
	}

	return n;
}

static int hi3861l_set_hostname(aos_dev_t *dev, const char *name)
{
#if LWIP_NETIF_HOSTNAME
	struct netif *netif = &hi3861l_netif[0];
	netif_set_hostname(netif, name);
	return 0;
#else
	return -1;
#endif
}

static const char *hi3861l_get_hostname(aos_dev_t *dev)
{
#if LWIP_NETIF_HOSTNAME
	struct netif *netif = &hi3861l_netif[0];
	return netif_get_hostname(netif);
#else
	return NULL;
#endif
}

static int hi3861l_set_link_up(aos_dev_t *dev)
{
	netif_set_link_up(&hi3861l_netif[0]);
	return 0;
}

static int hi3861l_set_link_down(aos_dev_t *dev)
{
	//netif_set_link_down(&hi3861l_netif[0]);
	return 0;
}

static int hi3861l_start_dhcp(aos_dev_t *dev)
{
	return 0;
}

static int hi3861l_stop_dhcp(aos_dev_t *dev)
{
	return 0;
}

static int hi3861l_set_ipaddr(aos_dev_t *dev, const ip_addr_t *ipaddr, const ip_addr_t *netmask,
			      const ip_addr_t *gw)
{
	return 0;
}

static int hi3861l_get_ipaddr(aos_dev_t *dev, ip_addr_t *ipaddr, ip_addr_t *netmask, ip_addr_t *gw)
{
	struct netif *netif = &hi3861l_netif[0];
	aos_check_return_einval(netif && ipaddr && netmask && gw);

	ip_addr_copy(*(ip4_addr_t *)ip_2_ip4(ipaddr), *netif_ip_addr4(netif));
	ip_addr_copy(*(ip4_addr_t *)ip_2_ip4(gw), *netif_ip_gw4(netif));
	ip_addr_copy(*(ip4_addr_t *)ip_2_ip4(netmask), *netif_ip_netmask4(netif));

	return 0;
}

static int hi3861l_ping_remote(aos_dev_t *dev, int type, char *remote_ip)
{
	return -1;
}

static int hi3861l_subscribe(aos_dev_t *dev, uint32_t event, event_callback_t cb, void *param)
{
	if (event == 0) {
		s_MsgrecvCallBack = cb;
	}
	return 0;
}

static int hi3861l_unsubscribe(aos_dev_t *dev, uint32_t event, event_callback_t cb, void *param)
{
	if (event == 0) {
		s_MsgrecvCallBack = NULL;
	}
	return 0;
}

static net_ops_t hi3861l_net_driver = {
	.set_mac_addr   = hi3861l_set_mac_addr,
	.get_mac_addr   = hi3861l_get_mac_addr,

	.set_dns_server = hi3861l_set_dns_server,
	.get_dns_server = hi3861l_get_dns_server,

	.set_hostname   = hi3861l_set_hostname,
	.get_hostname   = hi3861l_get_hostname,

	.set_link_up    = hi3861l_set_link_up,
	.set_link_down  = hi3861l_set_link_down,

	.start_dhcp     = hi3861l_start_dhcp,
	.stop_dhcp      = hi3861l_stop_dhcp,
	.set_ipaddr     = hi3861l_set_ipaddr,
	.get_ipaddr     = hi3861l_get_ipaddr,
	.ping           = hi3861l_ping_remote,

	.subscribe      = hi3861l_subscribe,
	.unsubscribe    = hi3861l_unsubscribe,
};

static wifi_driver_t hi3861l_wifi_driver = {
	.init             = hi3861l_init,
	.deinit           = hi3861l_deinit,
	.reset            = hi3861l_reset,
	.set_mode         = hi3861l_set_mode,
	.get_mode         = hi3861l_get_mode,
	.install_event_cb = hi3861l_install_event_cb,

	.set_protocol        = hi3861l_set_protocol,
	.get_protocol        = hi3861l_get_protocol,
	.set_country         = hi3861l_set_country,
	.get_country         = hi3861l_get_country,
	.set_mac_addr        = hi3861l_set_mac_addr,
	.get_mac_addr        = hi3861l_get_mac_addr,
	.set_auto_reconnect  = hi3861l_set_auto_reconnect,
	.get_auto_reconnect  = hi3861l_get_auto_reconnect,
	.set_lpm             = hi3861l_set_lpm,
	.get_lpm             = hi3861l_get_lpm,
	.power_on            = hi3861l_power_on,
	.power_off           = hi3861l_power_off,

	.start_scan          = hi3861l_start_scan,
	.start               = hi3861l_start,
	.stop                = hi3861l_stop,
	.sta_get_link_status = hi3861l_sta_get_link_status,
	.ap_get_sta_list     = hi3861l_ap_get_sta_list,

	.start_monitor        = hi3861l_start_monitor,
	.stop_monitor         = hi3861l_stop_monitor,
	.send_80211_raw_frame = hi3861l_send_80211_raw_frame,
	.set_channel          = hi3861l_set_channel,
	.get_channel          = hi3861l_get_channel,

	.set_smartcfg         = hi3861l_set_smartcfg,
};


static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
	int ret = 0;
#if ETH_PAD_SIZE
	pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif
	ret = hi_channel_send_data_to_dev(SendDataBuf, p->tot_len, p);
	if (ret == 0) {
#if ETH_PAD_SIZE
		pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
		LINK_STATS_INC(link.xmit);
		// RHINO_CPU_INTRPT_ENABLE()
		return ERR_OK;
	} else {
		return ERR_BUF;
	}
	return ERR_OK;
}

static void low_level_init(struct netif *netif)
{
	netif->hwaddr_len = ETH_ALEN;

	if (netif == &hi3861l_netif[0]) {
		memcpy(&netif->hwaddr[0], g_wlan_dev.hw_addr, ETH_ALEN);
	}

	/* Maximum transfer unit */
	netif->mtu = 1500;

#if LWIP_IPV6_AUTOCONFIG
	netif_set_ip6_autoconfig_enabled(netif, 1);
#endif
	/* device capabilities */
	/* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
	netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP |
			NETIF_FLAG_IGMP
#if defined(DHCP_USED)
			| NETIF_FLAG_DHCP
#endif
			;

#if LWIP_IPV6 && LWIP_IPV6_MLD
	/*
	 * For hardware/netifs that implement MAC filtering.
	 * All-nodes link-local is handled by default, so we must let the hardware know
	 * to allow multicast packets in.
	 * Should set mld_mac_filter previously. */
	if (netif->mld_mac_filter != NULL) {
		ip6_addr_t ip6_allnodes_ll;
		ip6_addr_set_allnodes_linklocal(&ip6_allnodes_ll);
		netif->mld_mac_filter(netif, &ip6_allnodes_ll, NETIF_ADD_MAC_FILTER);
	}
#endif /* LWIP_IPV6 && LWIP_IPV6_MLD */

	/* Do whatever else is needed to initialize interface. */
}

static err_t hi3861l_netif_init(struct netif *netif)
{
	LWIP_ASSERT("netif != NULL", (netif != NULL));
#if LWIP_NETIF_HOSTNAME
	/* Initialize interface hostname */
	netif->hostname = "hi3861l-wifi";
#endif /* LWIP_NETIF_HOSTNAME */

	//MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, NET_LINK_SPEED);

	netif->output = etharp_output;
#if LWIP_IPV6
	netif->output_ip6 = ethip6_output;
#endif /* LWIP_IPV6 */
	netif->linkoutput = low_level_output;

	/* initialize the hardware */
	low_level_init(netif);

	return ERR_OK;
}

static void hi3861l_lwip_init(void)
{
	tcpip_init(NULL, NULL);

	hi3861l_netif[0].name[0] = 'w';
	hi3861l_netif[0].name[1] = '0';
	netifapi_netif_add(&hi3861l_netif[0], IPADDR_ANY, IPADDR_ANY, IPADDR_ANY, NULL, hi3861l_netif_init, tcpip_input);
	//netif_set_status_callback(&hi3861l_netif[0], hi3861l_netif_status_changed);

	netifapi_netif_set_default(&hi3861l_netif[0]);

	netifapi_netif_set_up(&hi3861l_netif[0]);
}

static aos_dev_t *hi3861l_dev_init(driver_t *drv, void *config, int id)
{
	aos_dev_t *dev = device_new(drv, sizeof(wifi_dev_t), id);

	return dev;
}

static void hi3861l_dev_uninit(aos_dev_t *dev)
{
	aos_check_param(dev);

	device_free(dev);
}

static int hi3861l_dev_open(aos_dev_t *dev)
{
	return 0;
}

static int hi3861l_dev_close(aos_dev_t *dev)
{
	return 0;
}

static netdev_driver_t hi3861l_driver = {
	.drv =
	{
		.name   = "wifi",
		.init   = hi3861l_dev_init,
		.uninit = hi3861l_dev_uninit,
		.open   = hi3861l_dev_open,
		.close  = hi3861l_dev_close,
	},
	.link_type = NETDEV_TYPE_WIFI,
	.net_ops   = &hi3861l_net_driver,
	.link_ops  = &hi3861l_wifi_driver,
};

void wifi_hi3861l_register(hi3861l_wifi_param_t *config)
{
	memset((uint8_t *)&g_wlan_dev, 0, sizeof(g_wlan_dev));
	// char *cmd_mac = "cmd_get_mac";
	// hi_channel_send_msg_to_dev((void *)cmd_mac, strlen(cmd_mac));
	// aos_msleep(5);

	hi3861l_lwip_init();
	driver_register(&hi3861l_driver.drv, NULL, 0);
}

/******************************* register end **************************************************/

int wifi_is_connected_to_ap(void)
{
	return 0;
}

static void hi3861l_report_mac(unsigned char *mac, int len)
{
	printf("%s:report mac=%02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__,
	       mac[0] & 0xff, mac[1] & 0xff, mac[2] & 0xff, mac[3] & 0xff, mac[4] & 0xff, mac[5] & 0xff);
	memcpy(g_wlan_dev.hw_addr, mac, HI_WIFI_MAC_LEN);
}

static int hi3861l_report_ip(unsigned char *ip_addr, int len)
{
	if ((ip_addr[0] == 0) && (ip_addr[1] == 0) && (ip_addr[2] == 0) && (ip_addr[3] == 0)) {
		/* 1 2 3 ipaddr */
		//char *cmd_mac = "cmd_get_ip";
		//hi_channel_send_msg_to_dev((void *)cmd_mac, strlen(cmd_mac));
		return 0;
	}

	printf("%s:get ip=%d.%d.%d.%d mask=%d.%d.%d.%d gw=%d.%d.%d.%d\n", __FUNCTION__,
	       ip_addr[0] & 0xff, ip_addr[1] & 0xff, ip_addr[2] & 0xff, ip_addr[3] & 0xff,
	       ip_addr[4] & 0xff, ip_addr[5] & 0xff, ip_addr[6] & 0xff, ip_addr[7] & 0xff,
	       ip_addr[8] & 0xff, ip_addr[9] & 0xff, ip_addr[10] & 0xff, ip_addr[11] & 0xff);

	IP4_ADDR(&(g_wlan_dev.ip), (ip_addr[0] & 0xff), (ip_addr[1] & 0xff), (ip_addr[2] & 0xff), (ip_addr[3] & 0xff));
	IP4_ADDR(&(g_wlan_dev.mask), (ip_addr[4] & 0xff), (ip_addr[5] & 0xff), (ip_addr[6] & 0xff), (ip_addr[7] & 0xff));
	IP4_ADDR(&(g_wlan_dev.gw), (ip_addr[8] & 0xff), (ip_addr[9] & 0xff), (ip_addr[10] & 0xff), (ip_addr[11] & 0xff));

	netif_set_addr(&hi3861l_netif[0], &g_wlan_dev.ip, &g_wlan_dev.mask, &g_wlan_dev.gw);

	netif_set_link_up(&hi3861l_netif[0]);

	event_publish(EVENT_NET_GOT_IP, NULL);

	g_wifi_got_ip = 1;

	//extern void net_connected_callback(void);
	//net_connected_callback();

	return 0;
}

static void hi3861l_report_wpa_event(unsigned char *event, int len)
{
	if (event[0] == HI_WIFI_EVT_DISCONNECTED || event[0] == HI_WIFI_EVT_STA_FCON_NO_NETWORK) {
		event_publish(EVENT_WIFI_LINK_DOWN, NULL);
		g_wifi_got_ip = 0;
	} else if (event[0] == HI_WIFI_EVT_CONNECTED) {
		memcpy(g_wlan_dev.ssid, &event[1], MAX_SSID_SIZE + 1);
		memcpy(g_wlan_dev.bssid, &event[MAX_SSID_SIZE + 2], HI_WIFI_MAC_LEN);
		g_wlan_dev.channel = event[MAX_SSID_SIZE + 8];
		g_wlan_dev.rssi = event[MAX_SSID_SIZE + 9];
	}
}

void hi3861l_msg_rx(void *buf, int len)
{
	char *msg = (char *)buf;
	// int ret;
	int index = msg[0];

	switch (index) {
	case HOST_CMD_GET_MAC:
		hi3861l_report_mac((unsigned char *)&msg[4], HI_WIFI_MAC_LEN);
		break;
	case HOST_CMD_GET_IP:
		hi3861l_report_ip((unsigned char *)&msg[4], len - 4);
		break;
	case HOST_CMD_REPORT_WPA_EVENT:
		hi3861l_report_wpa_event((unsigned char *)&msg[4], len - 4);
		break;
	default:
		break;
	}
	if (s_MsgrecvCallBack) {
		s_MsgrecvCallBack(0, buf, (void *)&len);
	}

	return;
}
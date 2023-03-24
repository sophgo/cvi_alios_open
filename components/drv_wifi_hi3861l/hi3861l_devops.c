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
#include <devices/impl/wifi_impl.h>
#include <devices/impl/net_impl.h>
#include <devices/netdrv.h>
#include <devices/driver.h>
#include "wal_net.h"
#include "hi3861l_devops.h"

#define GMAC_BUF_LEN (1500 + 20)
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

struct netif hi3861l_netif[2];

uint8_t SendDataBuf[GMAC_BUF_LEN];
uint8_t RecvDataBuf[GMAC_BUF_LEN];

hi3861l_wlan_dev_t g_wlan_dev;

uint8_t g_wifi_got_ip = 0;

typedef struct {
    rvm_dev_t   device;
    uint8_t mode;
    void (*write_event)(rvm_dev_t *dev, int event_id, void *priv);
    void *priv;
} wifi_dev_t;

char host_cmd[][MAX_CMD_LEN] = {
    "cmd_get_mac",
    "cmd_get_ip",
    "cmd_set_filter",
    "cmd_get_rssi",
    "cmd_connect",
    "cmd_start_sta",
    "cmd_stop_sta",
    "cmd_start_reboot",
    "cmd_report_wpa_event", // 11
};
static int hi3861_init_flag = 0;
static int hi3861l_init(rvm_dev_t *dev)
{
    int ret;
    extern hi_s32 wlan_init(void);
    ret = wlan_init();
    if(ret != HI_SUCCESS) {
      return -1;
    }
    hi3861_init_flag = 1;
    return 0;
}

static int hi3861l_deinit(rvm_dev_t *dev)
{
    hi3861_init_flag = 0;
    return 0;
}

static int hi3861l_reset(rvm_dev_t *dev)
{
  uint32_t ret = -1;    

  // netif_set_link_down(&hi3861l_netif[0]);
  ret = hi_channel_send_msg_to_dev((hi_u8 *)host_cmd[HOST_CMD_STOP_STA], (hi_s32)strlen(host_cmd[HOST_CMD_STOP_STA]));
  if(ret == -1) {
      printf("[error] oal_stop_sta ret %d\r\n",ret);
  } 
  // else {
  //     printf("[success] oal_stop_sta ret %d\r\n",ret);
  // }

  return ret;
}

static int hi3861l_set_mode(rvm_dev_t *dev, rvm_hal_wifi_mode_t mode)
{
  uint32_t ret = -1;    

  ret = hi_channel_send_msg_to_dev((hi_u8 *)host_cmd[HOST_CMD_START_STA], (hi_s32)strlen(host_cmd[HOST_CMD_START_STA]));
  if(ret == -1) {
      printf("[error] oal_start_sta ret %d\r\n",ret);
  } 
  // else {
  //     printf("[success] oal_start_sta ret %d\r\n",ret);
  // }
  return 0;
}

static int hi3861l_get_mode(rvm_dev_t *dev, rvm_hal_wifi_mode_t *mode)
{
  return 0;
}

static int hi3861l_install_event_cb(rvm_dev_t *dev, rvm_hal_wifi_event_func *evt_func)
{
    return WIFI_ERR_OK;
}

static int hi3861l_set_protocol(rvm_dev_t *dev, uint8_t protocol_bitmap)
{
  return WIFI_ERR_OK;
}

static int hi3861l_get_protocol(rvm_dev_t *dev, uint8_t *protocol_bitmap)
{
  return WIFI_ERR_OK;
}

static int hi3861l_set_country(rvm_dev_t *dev, rvm_hal_wifi_country_t country)
{
    return WIFI_ERR_OK;
}

static int hi3861l_get_country(rvm_dev_t *dev, rvm_hal_wifi_country_t *country)
{
    return WIFI_ERR_OK;
}

static int hi3861l_set_auto_reconnect(rvm_dev_t *dev, bool en)
{
    return WIFI_ERR_OK;
}

static int hi3861l_get_auto_reconnect(rvm_dev_t *dev, bool *en)
{
    return WIFI_ERR_OK;
}

static int hi3861l_set_lpm(rvm_dev_t *dev, rvm_hal_wifi_lpm_mode_t mode)
{
    return WIFI_ERR_OK;
}

static int hi3861l_get_lpm(rvm_dev_t *dev, rvm_hal_wifi_lpm_mode_t *mode)
{
    return WIFI_ERR_OK;
}

static int hi3861l_power_on(rvm_dev_t *dev)
{
    return WIFI_ERR_OK;
}

static int hi3861l_power_off(rvm_dev_t *dev)
{
    return WIFI_ERR_OK;
}

static int hi3861l_start_scan(rvm_dev_t *dev, wifi_scan_config_t *config, bool block)
{
    return WIFI_ERR_OK;
}

#define TLV_HDR_LEN 4
#define CMD_CONNECT_TAG_SSID     0
#define CMD_CONNECT_TAG_AUTH     1
#define CMD_CONNECT_TAG_KEY      2
#define CMD_CONNECT_TAG_BSSID    3
#define CMD_CONNECT_TAG_PAIRWISE 4
#define CMD_TLV_FORMAT   "%02x%02x%s"
static void hi3861l_start_sta(rvm_hal_wifi_config_t *config)
{
  uint32_t ret = -1;
  char *cmd, *tmp;
  unsigned long len;
  int i;
  unsigned char bssid[6] = {0};
    

  len = strlen("cmd_connect") + \
            TLV_HDR_LEN + strlen(config->ssid) + \
            TLV_HDR_LEN + strlen(config->password) + \
            TLV_HDR_LEN + sizeof(bssid) * 2 + 1;

  cmd = aos_malloc(len);
	tmp = cmd;
	if(cmd != NULL) {
		memset(tmp, 0, len);

        memcpy(tmp, "cmd_connect", strlen("cmd_connect"));

        tmp += strlen("cmd_connect");
        snprintf(tmp, len, CMD_TLV_FORMAT, CMD_CONNECT_TAG_SSID, (uint32_t)strlen(config->ssid), config->ssid);

        tmp += strlen(config->ssid) + TLV_HDR_LEN;
        snprintf(tmp, len, CMD_TLV_FORMAT, CMD_CONNECT_TAG_KEY, (uint32_t)strlen(config->password), config->password);

        tmp += strlen(config->password) + TLV_HDR_LEN;
        snprintf(tmp, len, "%02x%02zx", CMD_CONNECT_TAG_BSSID, sizeof(bssid) * 2);

        tmp += TLV_HDR_LEN;
        for (i = 0; i < sizeof(bssid); i++) {
            snprintf(tmp, len, "%02x", 0);
            tmp += 2;
        }

		hi_channel_send_msg_to_dev((void*)cmd, strlen(cmd));
		aos_free(cmd);
		//return 1 for not do RT_WLAN_DEV_EVT_CONNECT report in wlan_dev.c
		return;
	}

    ret = hi_channel_send_msg_to_dev((hi_u8 *)host_cmd[HOST_CMD_GET_MAC], (hi_s32)strlen(host_cmd[HOST_CMD_GET_MAC]));
    if(ret == -1) {
        printf("[error] oal_send_msg_to_device ret %d\r\n",ret);
    } else {
        printf("[success] oal_send_msg_to_device ret %d\r\n",ret);
    }
}

static int hi3861l_start(rvm_dev_t *dev, rvm_hal_wifi_config_t *config)
{
  hi3861l_set_mode(dev, WIFI_MODE_STA);
  hi_channel_send_msg_to_dev((hi_u8 *)host_cmd[HOST_CMD_GET_MAC], (hi_s32)strlen(host_cmd[HOST_CMD_GET_MAC]));
  aos_msleep(50);
  hi3861l_start_sta(config);

  return WIFI_ERR_OK;
}

static int hi3861l_stop(rvm_dev_t *dev)
{
  return WIFI_ERR_OK;
}

static int hi3861l_sta_get_link_status(rvm_dev_t *dev, rvm_hal_wifi_ap_record_t *ap_info)
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

static int hi3861l_ap_get_sta_list(rvm_dev_t *dev, rvm_hal_wifi_sta_list_t *sta)
{
  return WIFI_ERR_OK;
}

static int hi3861l_set_mac_addr(rvm_dev_t *dev, const uint8_t *mac)
{
    return WIFI_ERR_OK;
}

static int hi3861l_get_mac_addr(rvm_dev_t *dev, uint8_t *mac)
{
    memcpy(mac, g_wlan_dev.hw_addr, HI_WIFI_MAC_LEN);
    return WIFI_ERR_OK;
}

int hi3861l_start_mgnt_monitor(rvm_dev_t *dev, rvm_hal_wifi_mgnt_cb_t cb)
{
    return WIFI_ERR_OK;
}

int hi3861l_stop_mgnt_monitor(rvm_dev_t *dev)
{
    return WIFI_ERR_OK;
}

static int hi3861l_start_monitor(rvm_dev_t *dev, rvm_hal_wifi_promiscuous_cb_t cb)
{
    return WIFI_ERR_OK;
}

static int hi3861l_stop_monitor(rvm_dev_t *dev)
{
    return WIFI_ERR_OK;
}

static int hi3861l_send_80211_raw_frame(rvm_dev_t *dev, void *buffer, uint16_t len)
{
    return WIFI_ERR_OK;
}

static int hi3861l_set_channel(rvm_dev_t *dev, uint8_t primary, rvm_hal_wifi_second_chan_t second)
{
    return WIFI_ERR_OK;
}

static int hi3861l_get_channel(rvm_dev_t *dev, uint8_t *primary, rvm_hal_wifi_second_chan_t *second)
{
    return WIFI_ERR_OK;
}

static int hi3861l_set_smartcfg(rvm_dev_t *dev, int enable)
{
    return WIFI_ERR_OK;
}

static int hi3861l_set_dns_server(rvm_dev_t *dev, ip_addr_t ipaddr[], uint32_t num)
{
    int n, i;

    n = MIN(num, DNS_MAX_SERVERS);

    for (i = 0; i < n; i ++) {
        dns_setserver(i, &ipaddr[i]);
    }

    return n;
}

static int hi3861l_get_dns_server(rvm_dev_t *dev, ip_addr_t ipaddr[], uint32_t num)
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

static int hi3861l_set_hostname(rvm_dev_t *dev, const char *name)
{
#if LWIP_NETIF_HOSTNAME
    struct netif *netif = &hi3861l_netif[0];
    netif_set_hostname(netif, name);
    return 0;
#else
    return -1;
#endif
}

static const char *hi3861l_get_hostname(rvm_dev_t *dev)
{
#if LWIP_NETIF_HOSTNAME
    struct netif *netif = &hi3861l_netif[0];
    return netif_get_hostname(netif);
#else
    return NULL;
#endif
}

static int hi3861l_set_link_up(rvm_dev_t *dev)
{
    //netif_set_link_up(&hi3861l_netif[0]);
    return 0;
}

static int hi3861l_set_link_down(rvm_dev_t *dev)
{
    //netif_set_link_down(&hi3861l_netif[0]);
    return 0;
}

static int hi3861l_start_dhcp(rvm_dev_t *dev)
{
    return 0;
}

static int hi3861l_stop_dhcp(rvm_dev_t *dev)
{
    return 0;
}

static int hi3861l_set_ipaddr(rvm_dev_t *dev, const ip_addr_t *ipaddr, const ip_addr_t *netmask,
                           const ip_addr_t *gw)
{
    return 0;
}

static int hi3861l_get_ipaddr(rvm_dev_t *dev, ip_addr_t *ipaddr, ip_addr_t *netmask, ip_addr_t *gw)
{
    struct netif *netif = &hi3861l_netif[0];
    aos_check_return_einval(netif && ipaddr && netmask && gw);

    ip_addr_copy(*(ip4_addr_t *)ip_2_ip4(ipaddr), *netif_ip_addr4(netif));
    ip_addr_copy(*(ip4_addr_t *)ip_2_ip4(gw), *netif_ip_gw4(netif));
    ip_addr_copy(*(ip4_addr_t *)ip_2_ip4(netmask), *netif_ip_netmask4(netif));

    return 0;
}

static int hi3861l_ping_remote(rvm_dev_t *dev, int type, char *remote_ip)
{
    return -1;
}

static int hi3861l_subscribe(rvm_dev_t *dev, uint32_t event, event_callback_t cb, void *param)
{
    return -1;
}

static int hi3861l_unsubscribe(rvm_dev_t *dev, uint32_t event, event_callback_t cb, void *param)
{
    return -1;
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
  //struct ethernetif *ethernetif = netif->state;
  int ret = 0;
  //initiate transfer();

#if ETH_PAD_SIZE
  pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif
  ret = hi_channel_send_data_to_dev(SendDataBuf, p->tot_len, p);
  if (ret == 0)
  {

    //RHINO_CPU_INTRPT_DISABLE()
    //hi_channel_send_data_to_dev(SendDataBuf, i, p);
    //RHINO_CPU_INTRPT_ENABLE()


#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

    LINK_STATS_INC(link.xmit);
    // RHINO_CPU_INTRPT_ENABLE()

    return ERR_OK;
  }
  else
  {
    return ERR_BUF;
  }
}

static void low_level_init(struct netif *netif)
{
  netif->hwaddr_len = ETH_ALEN;

  if (netif == &hi3861l_netif[0])
  {
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
  if (netif->mld_mac_filter != NULL)
  {
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

static rvm_dev_t *hi3861l_dev_init(driver_t *drv, void *config, int id)
{
    rvm_dev_t *dev = rvm_hal_device_new(drv, sizeof(wifi_dev_t), id);

    return dev;
}

static void hi3861l_dev_uninit(rvm_dev_t *dev)
{
    aos_check_param(dev);

    rvm_hal_device_free(dev);
}

static int hi3861l_dev_open(rvm_dev_t *dev)
{
    return 0;
}

static int hi3861l_dev_close(rvm_dev_t *dev)
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
  rvm_driver_register(&hi3861l_driver.drv, NULL, 0);
}

/******************************* register end **************************************************/

int wifi_is_connected_to_ap(void)
{
    return 0;
}

static void hi3861l_report_mac(unsigned char *mac, int len)
{
  printf("%s:report mac=%x:%x:%x:%x:%x:%x\n", __FUNCTION__,
         mac[0] & 0xff, mac[1] & 0xff, mac[2] & 0xff, mac[3] & 0xff, mac[4] & 0xff, mac[5] & 0xff);

  //save mac to g_wlan_dev
  memcpy(g_wlan_dev.hw_addr, mac, HI_WIFI_MAC_LEN);
}

static int hi3861l_report_ip(unsigned char *ip_addr, int len)
{
  if ((ip_addr[0] == 0) && (ip_addr[1] == 0) && (ip_addr[2] == 0) && (ip_addr[3] == 0))
  { /* 1 2 3 ipaddr */
    //char *cmd_mac = "cmd_get_ip";
    //hi_channel_send_msg_to_dev((void *)cmd_mac, strlen(cmd_mac));
    return 0;
  }

  printf("%s:get ip=%d.%d.%d.%d mask=%d.%d.%d.%d gw=%d.%d.%d.%d\n", __FUNCTION__,
         ip_addr[0] & 0xff, ip_addr[1] & 0xff, ip_addr[2] & 0xff, ip_addr[3] & 0xff,
         ip_addr[4] & 0xff, ip_addr[5] & 0xff, ip_addr[6] & 0xff, ip_addr[7] & 0xff,
         ip_addr[8] & 0xff, ip_addr[9] & 0xff, ip_addr[10] & 0xff, ip_addr[11] & 0xff);

  IP4_ADDR(&(g_wlan_dev.ip), (ip_addr[0]&0xff), (ip_addr[1]&0xff), (ip_addr[2]&0xff), (ip_addr[3]&0xff));
  IP4_ADDR(&(g_wlan_dev.mask), (ip_addr[4]&0xff), (ip_addr[5]&0xff), (ip_addr[6]&0xff), (ip_addr[7]&0xff));
  IP4_ADDR(&(g_wlan_dev.gw), (ip_addr[8]&0xff), (ip_addr[9]&0xff), (ip_addr[10]&0xff), (ip_addr[11]&0xff));
  
  netif_set_addr(&hi3861l_netif[0], &g_wlan_dev.ip, &g_wlan_dev.mask, &g_wlan_dev.gw);

  netif_set_link_up(&hi3861l_netif[0]);

  event_publish(EVENT_NET_GOT_IP, NULL);

  g_wifi_got_ip = 1;

  // extern void net_connected_callback(void);
  // net_connected_callback();

  return 0;
}

static void hi3861l_report_wpa_event(unsigned char *event, int len)
{
  if (event[0] == HI_WIFI_EVT_DISCONNECTED || event[0] == HI_WIFI_EVT_STA_FCON_NO_NETWORK) {
    event_publish(EVENT_WIFI_LINK_DOWN, NULL);
    g_wifi_got_ip = 0;
  } else if (event[0] == HI_WIFI_EVT_CONNECTED) {
    memcpy(g_wlan_dev.ssid, &event[1], MAX_SSID_SIZE+1);
    memcpy(g_wlan_dev.bssid, &event[MAX_SSID_SIZE+2], HI_WIFI_MAC_LEN);
    g_wlan_dev.channel = event[MAX_SSID_SIZE+8];
    g_wlan_dev.rssi = event[MAX_SSID_SIZE+9];
  }
}

void hi3861l_msg_rx(void *buf, int len)
{
  char *msg = (char *)buf;
  // int ret;
  int index = msg[0];

  switch (index)
  {
  case HOST_CMD_GET_MAC:
    hi3861l_report_mac((unsigned char *)&msg[1], HI_WIFI_MAC_LEN);
    break;
  case HOST_CMD_GET_IP:
    hi3861l_report_ip((unsigned char *)&msg[1], len - 1);
    break;
  case HOST_CMD_REPORT_WPA_EVENT:
    hi3861l_report_wpa_event((unsigned char *)&msg[1], len - 1);
    break;  
  default:
    break;
  }

  return;
}

void hi3861_reboot()
{
    if (hi3861_init_flag) {
        uint32_t ret = -1;
        ret = hi_channel_send_msg_to_dev((hi_u8 *)host_cmd[HOST_CMD_REBOOT], (hi_s32)strlen(host_cmd[HOST_CMD_REBOOT]));
        if(ret == -1) {
            printf("[error] hi3861_reboot ret %d\r\n",ret);
        } else {
            printf("[success] hi3861_reboot ret %d\r\n",ret);
        }
    }
}
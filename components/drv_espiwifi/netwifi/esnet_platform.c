#include "platform_os.h"
#include "esnet_wifi.h"
#include "drv_espi.h"
#include "lwip/netifapi.h"
#include "aos/cli.h"
#include "ota.h"

void esnet_platform_net_up(esnet_if_t *netif)
{
    netifapi_netif_set_up(netif);

    netif_set_default(netif);
}

void esnet_platform_net_down(esnet_if_t *netif)
{
    netifapi_netif_set_down(netif);
}

void esnet_platform_set_ip(esnet_if_t *netif, unsigned int ip, unsigned int mask, unsigned int gw)
{
    if (netif == NULL) {
        platform_log_e("%s[%d] set net ip failed\n", __FUNCTION__, __LINE__);
        return;
    }

    netif_set_addr(netif, (const ip4_addr_t *)&ip, (const ip4_addr_t *)&mask, (const ip4_addr_t *)&gw);
}

int esnet_platform_get_ip(esnet_if_t *netif, unsigned int *ip, unsigned int *mask, unsigned int *gw)
{
    if (netif == NULL) {
        platform_log_e("%s[%d] get net ip failed\n", __FUNCTION__, __LINE__);
        return -1;
    }

    if (ip)
        *ip = netif_ip4_addr(netif)->addr;
    if (mask)
        *mask = netif_ip4_netmask(netif)->addr;
    if (gw)
        *gw = netif_ip4_gw(netif)->addr;

    return 0;
}

void esnet_platform_dump_netif(struct netif *netif)
{
    unsigned int ip_addr = ntohl(ip4_addr_get_u32(&netif->ip_addr));
    unsigned int netmask = ntohl(ip4_addr_get_u32(&netif->netmask));
    unsigned int ip_gw = ntohl(ip4_addr_get_u32(&netif->gw));

    platform_log_i("     name:[%.*s]\n", 2, netif->name);
    platform_log_i("      num:[%d]\n", netif->num);
    platform_log_i("    flags:[0x%02x]\n", netif->flags);
    platform_log_i("       hw:[%02x:%02x:%02x:%02x:%02x:%02x]\n", netif->hwaddr[0], netif->hwaddr[1], netif->hwaddr[2],
        netif->hwaddr[3], netif->hwaddr[4], netif->hwaddr[5]);
    platform_log_i("      mtu:[%d]\n", netif->mtu);
    platform_log_i("  ip_addr:[%d.%d.%d.%d]\n", (ip_addr>>24) & 0xff, (ip_addr>>16) & 0xff, (ip_addr>>8) & 0xff, ip_addr & 0xff);
    platform_log_i("  netmask:[%d.%d.%d.%d]\n", (netmask>>24) & 0xff, (netmask>>16) & 0xff, (netmask>>8) & 0xff, netmask & 0xff);
    platform_log_i("       gw:[%d.%d.%d.%d]\n", (ip_gw>>24) & 0xff, (ip_gw>>16) & 0xff, (ip_gw>>8) & 0xff, ip_gw & 0xff);

    platform_log_i("----------------------------\n");
}

void esnet_platform_dump_dns(void)
{
    extern const ip_addr_t *dns_getserver(u8_t numdns);
    unsigned int dns = ntohl(ip4_addr_get_u32(dns_getserver(0)));

    platform_log_i("  dns:[%d:%d:%d:%d]\n", (dns>>24) & 0xff, (dns>>16) & 0xff, (dns>>8) & 0xff, dns & 0xff);
}

extern struct netif *netif_list;
extern void cli_reg_cmd_ping(void);

static void cmd_ifconfig_func(int argc, char **argv)
{
    struct netif *netif = netif_list;

    platform_log_i("\n");
    cli_reg_cmd_ping();
    esnet_platform_dump_dns();

    while (netif != NULL) {
        esnet_platform_dump_netif(netif);
        netif = netif->next;
    }
}

ALIOS_CLI_CMD_REGISTER(cmd_ifconfig_func, ifconfig, ifconfig info);

static void cmd_host_msg_func(int argc, char **argv)
{
    if (argc != 2) {
        platform_log_i("spimsg xxxxx\n");
    }
    drv_espi_send_type_data(DRV_ESPI_TYPE_MSG, (unsigned char *)argv[1], strlen(argv[1]));
}

ALIOS_CLI_CMD_REGISTER(cmd_host_msg_func, spimsg, spimsg send);

static void cmd_host_ota_func(int argc, char **argv)
{
    unsigned int offset = 0;
    unsigned int totallen = sizeof(otadata);
    unsigned int sendlen;

    while (offset != totallen) {
        sendlen = (totallen - offset > 1024) ? 1024 : (totallen - offset);
        drv_espi_send_type_data(DRV_ESPI_TYPE_OTA, otadata + offset, sendlen);
        offset += sendlen;
    }
}

ALIOS_CLI_CMD_REGISTER(cmd_host_ota_func, spiota, spiota send);


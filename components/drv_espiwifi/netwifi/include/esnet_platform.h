#ifndef __ESNET_PLATFORM_H__
#define __ESNET_PLATFORM_H__

#include "lwip/pbuf.h"
#include "lwip/netif.h"

// Net interface
typedef struct netif esnet_if_t;

#define ESNET_HWADDR_MAX NETIF_MAX_HWADDR_LEN

void esnet_platform_net_up(esnet_if_t *netif);
void esnet_platform_net_down(esnet_if_t *netif);
void esnet_platform_set_ip(esnet_if_t *netif, unsigned int ip, unsigned int mask, unsigned int gw);
int esnet_platform_get_ip(esnet_if_t *netif, unsigned int *ip, unsigned int *mask, unsigned int *gw);

#endif

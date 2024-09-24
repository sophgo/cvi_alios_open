/**
 * @file
 *
 * lwIP Options Configuration
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef LWIP_LWIPOPTS_H
#define LWIP_LWIPOPTS_H

//#include "lwip/arch.h"

/**
 * Include user defined options first. Anything not defined in these files
 * will be set to standard values. Override anything you dont like!
 */

/*
   -------------- NO SYS --------------
*/
#define NO_SYS                          0
#define SYS_LIGHTWEIGHT_PROT            (NO_SYS == 0)

/*
   ----------- Core locking -----------
*/
#define LWIP_TCPIP_CORE_LOCKING         0

/*
   ---------- Memory options ----------
*/
/* If MEM_LIBC_MALLOC and MEMP_MEM_MALLOC are defined to 1,
MEM_SIZE and MEMP_NUM_XXX will be invalid */
#define MEM_LIBC_MALLOC                 0
#define MEMP_MEM_MALLOC                 0
#define MEM_ALIGNMENT                   4

/* Application data is used for packet buffer directly */
//no meaning if MEM_LIBC_MALLOC = 1
#define MEM_SIZE                        (1024 * 250)

/*
   ---------- Internal Memory Pool Sizes ----------
*/
//no meaning if MEMP_MEM_MALLOC = 1
#define MEMP_NUM_PBUF                   32
#define MEMP_NUM_RAW_PCB                2
#define MEMP_NUM_UDP_PCB                32
#define MEMP_NUM_TCP_PCB                48
#define MEMP_NUM_TCP_PCB_LISTEN         8
#define MEMP_NUM_TCP_SEG                200
#define MEMP_NUM_REASSDATA              5
#define MEMP_NUM_ARP_QUEUE              30
#define MEMP_NUM_NETBUF                 32
#define MEMP_NUM_TCPIP_MSG_INPKT        160
#define MEMP_NUM_FRAG_PBUF              32
#define MEMP_NUM_NETCONN                96
#define MEMP_NUM_TCPIP_MSG_API          24
#define MEMP_NUM_NETIFDRV_MSG           4
#define MEMP_NUM_NETDB                  4

#define PBUF_POOL_SIZE                  250

/*
   ---------- ARP options ----------
*/
#define LWIP_ARP                        1

/*
   ---------- IP options ----------
*/
#define LWIP_IPV4                       1
#define IP_FORWARD                      0
#define IP_OPTIONS_ALLOWED              1
#define IP_REASSEMBLY                   0
#define IP_FRAG                         0
#define IP_REASS_MAXAGE                 3
#define IP_REASS_MAX_PBUFS              6
#define LWIP_RANDOMIZE_INITIAL_LOCAL_PORTS     1

/*
   ---------- ICMP options ----------
*/
#define LWIP_ICMP                       1

/*
   ---------- RAW options ----------
*/
#define LWIP_RAW                        1

/*
   ---------- DHCP options ----------
*/
#define LWIP_DHCP                       1
#define DHCP_DOES_ARP_CHECK             0

/*
   ---------- AUTOIP options ----------
*/
#define LWIP_AUTOIP                     0

/*
   ---------- SNMP options ----------
*/
#define LWIP_SNMP                       0

/*
   ---------- IGMP options ----------
*/
#define LWIP_IGMP                       1

/*
   ---------- DNS options -----------
*/
#define LWIP_DNS                        1
#define LWIP_DNS_SECURE                 0

#define DNS_MAX_NAME_LENGTH             128 //default is 256
#define DNS_SERVER_ADDRESS(ipaddr)      ip_addr_set_ip4_u32(ipaddr, ipaddr_addr("208.67.222.222")) /* resolver1.opendns.com */

/*
   ---------- MTU options -----------
*/

/*
   ---------- UDP options ----------
*/
#define LWIP_UDP                        1

/*
   ---------- TCP options ----------
*/
#define LWIP_TCP                        1
#define TCP_QUEUE_OOSEQ                 1

#define TCP_MSS                         (1440)
/* lwip_sanity_check, TCP_SND_BUF must be at least as much as (2 * TCP_MSS) */
#define TCP_SND_BUF                     (44 * TCP_MSS)
#define TCP_WND                         (44 * TCP_MSS)
#define LWIP_LISTEN_BACKLOG             0
#define TCP_OVERSIZE                    TCP_MSS
#define TCP_WND_UPDATE_THRESHOLD        (TCP_WND / 4)
#define LWIP_TCP_SACK_OUT               1

/*
   ---------- Pbuf options ----------
*/
#define PBUF_LINK_HLEN                  16

/*
   ---------- Network Interfaces options ----------
*/
#define LWIP_NETIF_LINK_CALLBACK        1
#define LWIP_NETIF_STATUS_CALLBACK      1

/*
   ---------- LOOPIF options ----------
*/
#define LWIP_HAVE_LOOPIF                1
#define LWIP_NETIF_LOOPBACK             0

/*
   ---------- Thread options ----------
*/
#define TCPIP_MBOX_SIZE                 128
#define DEFAULT_ACCEPTMBOX_SIZE         16
#define DEFAULT_RAW_RECVMBOX_SIZE       64
#define DEFAULT_UDP_RECVMBOX_SIZE       64
#define DEFAULT_TCP_RECVMBOX_SIZE       64
#define TCPIP_THREAD_STACKSIZE          1024 * 4
#define TCPIP_THREAD_PRIO               (AOS_DEFAULT_APP_PRI + 4)

/*
每個線程使用一個（本地線程）信號量調用socket / netconn函數，
而不是每netconn分配一個信號量（和每個選擇等）
*/
#define LWIP_NETCONN_SEM_PER_THREAD     1

/*
   ---------- Sequential layer options ----------
*/

#define LWIP_NETCONN                    1

/*
   ---------- Socket options ----------
*/
#define LWIP_SOCKET                     1
#define LWIP_COMPAT_SOCKETS             2
#define LWIP_POSIX_SOCKETS_IO_NAMES     0
#define LWIP_SOCKET_POLL 1
#define LWIP_SOCKET_SELECT 1

#define LWIP_SOCKET_OFFSET              20

#define LWIP_SO_SNDTIMEO                1
#define LWIP_SO_RCVTIMEO                1
#define SO_REUSE                        1
#define LWIP_SO_BINDTODEVICE            0
#define LWIP_SOCKET_SEND_NOCOPY         0

/*
   ---------- Statistics options ----------
*/
#define LWIP_STATS                      0
#define LWIP_STATS_DISPLAY              0

#define LINK_STATS                      0
#define ETHARP_STATS                    0
#define IP_STATS                        0
#define IPFRAG_STATS                    0
#define ICMP_STATS                      0
#define IGMP_STATS                      0
#define UDP_STATS                       0
#define TCP_STATS                       0
//#define MEM_STATS                       1 //((MEM_LIBC_MALLOC == 0) && (MEM_USE_POOLS == 0))
//#define MEMP_STATS                      (MEMP_MEM_MALLOC == 0)
//#define SYS_STATS                       (NO_SYS == 0)
#define IP6_STATS                       0
#define ICMP6_STATS                     0
#define IP6_FRAG_STATS                  0
#define MLD6_STATS                      0
#define ND6_STATS                       0
#define MIB2_STATS                      0

/*
   ---------- Checksum options ----------
*/
#define CHECKSUM_GEN_IP                 1
#define CHECKSUM_GEN_UDP                1
#define CHECKSUM_GEN_TCP                1
#define CHECKSUM_CHECK_IP               0
#define CHECKSUM_CHECK_UDP              0
#define CHECKSUM_CHECK_TCP              0

/*
   ---------- IPv6 options ---------------
*/

#define LWIP_IPV6                       0
#define LWIP_IPV6_FRAG                  0
#define LWIP_ICMP6                      1
#define LWIP_IPV6_REASS                 0
#define LWIP_IPV6_MLD                   0
#define LWIP_ND6_MAX_MULTICAST_SOLICIT  10

/*
   ---------- Hook options ---------------
*/
//#define LWIP_HOOK_IP4_ROUTE_SRC         lwip_ip4_route_src
#define SNTP_SET_SYSTEM_TIME_US(sec, us) sntp_set_system_time(sec, us)
// #define SNTP_SET_SYSTEM_TIME(sec)   set_system_time(sec)

/*
   ---------- sntp options ----------
*/
#define SNTP_MAX_SERVERS            1
#define SNTP_SERVER_DNS             1
#define SNTP_SERVER_ADDRESS         "ntp1.aliyun.com"

/*
   ---------- Debugging options ----------
*/
//#define LWIP_DEBUG

#define LWIP_DBG_MIN_LEVEL              LWIP_DBG_LEVEL_ALL
#define LWIP_DBG_TYPES_ON               (LWIP_DBG_ON|LWIP_DBG_TRACE|LWIP_DBG_STATE|LWIP_DBG_FRESH|LWIP_DBG_HALT)

#define MEM_DEBUG                       LWIP_DBG_OFF
#define MEMP_DEBUG                      LWIP_DBG_OFF
#define PBUF_DEBUG                      LWIP_DBG_OFF
#define API_LIB_DEBUG                   LWIP_DBG_OFF
#define API_MSG_DEBUG                   LWIP_DBG_OFF
#define TCPIP_DEBUG                     LWIP_DBG_OFF
#define NETIF_DEBUG                     LWIP_DBG_OFF
#define SOCKETS_DEBUG                   LWIP_DBG_OFF
#define IP_DEBUG                        LWIP_DBG_OFF
#define IP_REASS_DEBUG                  LWIP_DBG_OFF
#define RAW_DEBUG                       LWIP_DBG_OFF
#define ICMP_DEBUG                      LWIP_DBG_OFF
#define UDP_DEBUG                       LWIP_DBG_OFF
#define TCP_DEBUG                       LWIP_DBG_OFF
#define TCP_INPUT_DEBUG                 LWIP_DBG_OFF
#define TCP_OUTPUT_DEBUG                LWIP_DBG_OFF
#define TCP_RTO_DEBUG                   LWIP_DBG_OFF
#define TCP_CWND_DEBUG                  LWIP_DBG_OFF
#define TCP_WND_DEBUG                   LWIP_DBG_OFF
#define TCP_FR_DEBUG                    LWIP_DBG_OFF
#define TCP_QLEN_DEBUG                  LWIP_DBG_OFF
#define TCP_RST_DEBUG                   LWIP_DBG_OFF

#define LWIP_NOASSERT                   1

/*
   ---------- Performance tracking options ----------
*/

/*
   ---------- PPP options ----------
*/
#define PPP_SUPPORT                     0

#define LWIP_NETIF_API                  1
#define LWIP_NETIF_HOSTNAME             1

/*
   ---------- Extend options ----------
*/
#define LWIP_XR_EXT_MBUF_SUPPORT        0
#define LWIP_XR_EXT                     0

#define LWIP_EXT_SIM_ARP                0
#define LWIP_EXT_ZWP_LIMITED            0
#define LWIP_EXT_NO_DNS_TMR             0

#define LWIP_NETIF_DRV                  1

// PBUF_RAM is not limited if LWIP_EXT_MAX_MEM_SIZE = 0
#define LWIP_EXT_MAX_MEM_SIZE           (1024*10)

#endif /* LWIP_LWIPOPTS_H */

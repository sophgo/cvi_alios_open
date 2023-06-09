/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <aos/aos.h>

#include "lwip/opt.h"

#if LWIP_SOCKET /* don't build if not configured for use in lwipopts.h */

#include "lwip/sockets.h"
#include "lwip/api.h"
#include "lwip/sys.h"
#include "lwip/igmp.h"
#include "arpa/inet.h"
#include "lwip/tcp.h"
#include "lwip/raw.h"
#include "lwip/udp.h"
#include "lwip/memp.h"
#include "lwip/pbuf.h"
#include "lwip/priv/tcpip_priv.h"
#include "lwip/netdb.h"
//#include "lwip/ip_addr.h"
#if LWIP_CHECKSUM_ON_COPY
#include "lwip/inet_chksum.h"
#endif

#include <string.h>

#include "sal2lwip.h"

#include "sal.h"

/*
* Options and types related to multicast membership
*/
#define IP_ADD_MEMBERSHIP  3
#define IP_DROP_MEMBERSHIP 4

#define IP_MULTICAST_TTL   5
#define IP_MULTICAST_IF    6
#define IP_MULTICAST_LOOP  7

#define SAL_SO_SNDRCVTIMEO_GET_MS(optval) ((((const struct timeval *)(optval))->tv_sec * 1000U) + (((const struct timeval *)(optval))->tv_usec / 1000U))

#define SAL_TAG  "sals"

#ifdef SAL_USE_DEBUG
#define SAL_DEBUG(format, ...)  LOGD(SAL_TAG, format, ##__VA_ARGS__)
#define SAL_ERROR(format, ...)  LOGE(SAL_TAG, format, ##__VA_ARGS__)
#define SAL_ASSERT(msg, assertion) do { if (!(assertion)) { \
            LOGE(SAL_TAG, msg);} \
    } while (0)
#else
#define SAL_DEBUG(format, ...)
#define SAL_ERROR(format, ...)
#define SAL_ASSERT(msg, assertion)
#endif

#define SAL_SOCKET_MAX_PAYLOAD_SIZE  1512
#define SAL_SOCKET_IP4_ANY_ADDR      "0.0.0.0"
#define SAL_SOCKET_IP4_ADDR_LEN      16


//#define NETCONN_TYPE_IPV6            0x08
//#define NETCONNTYPE_ISIPV6(t)        (((t)&NETCONN_TYPE_IPV6) != 0)

#define SOCK_ADDR_TYPE_MATCH(name, sock) \
    ((((name)->sa_family == AF_INET) && !(NETCONNTYPE_ISIPV6((sock)->conn->type))) || \
     (((name)->sa_family == AF_INET6) && (NETCONNTYPE_ISIPV6((sock)->conn->type))))

#define SOCK_ADDR_TYPE_MATCH_OR_UNSPEC(name, sock) (((name)->sa_family == AF_UNSPEC) || \
        SOCK_ADDR_TYPE_MATCH(name, sock))

#define NUM_SOCKETS MEMP_NUM_NETCONN
#define NUM_EVENTS  MEMP_NUM_NETCONN

#define SAL_DRAIN_SENDMBOX_WAIT_TIME   50

#define SAL_EVENT_OFFSET (NUM_SOCKETS + SAL_SOCKET_OFFSET)

#ifndef SELWAIT_T
#define SELWAIT_T uint8_t
#endif

#define IPADDR_BROADCAST_STRING "255.255.255.255"

#define IP4ADDR_PORT_TO_SOCKADDR(sin, ipaddr, port) do { \
        (sin)->sin_len = sizeof(struct sockaddr_in); \
        (sin)->sin_family = AF_INET; \
        (sin)->sin_port = sal_htons((port)); \
        inet_addr_from_ip4addr(&(sin)->sin_addr, ipaddr); \
        memset((sin)->sin_zero, 0, SIN_ZERO_LEN); }while(0)

#define SOCKADDR4_TO_IP4ADDR_PORT(sin, ipaddr, port) do { \
        inet_addr_to_ip4addr(ip_2_ip4(ipaddr), &((sin)->sin_addr)); \
        (port) = sal_htons((sin)->sin_port); }while(0)

#define IPADDR_PORT_TO_SOCKADDR(sockaddr, ipaddr, port) \
    IP4ADDR_PORT_TO_SOCKADDR((struct sockaddr_in*)(void*)(sockaddr), ip_2_ip4(ipaddr), port)

int ip4addr_aton(const char *cp, ip4_addr_t *addr);

static int  sal_selscan(int maxfdp1, fd_set *readset_in, fd_set *writeset_in,
                        fd_set *exceptset_in, fd_set *readset_out,
                        fd_set *writeset_out, fd_set *exceptset_out);

static struct sal_sock *tryget_socket(int s);

static struct sal_event *tryget_event(int s);

#if SAL_PACKET_SEND_MODE_ASYNC
static void sal_packet_output(void *arg);
#endif

static int alloc_socket(sal_netconn_t *newconn, int accepted);

static void free_socket(struct sal_sock *sock);

struct sal_event {
    uint64_t counts;
    int used;
    int reads;
    int writes;
    /** semaphore to wake up a task waiting for select */
    sal_sem_t *psem;
};

/** Contains all internal pointers and states used for a socket */
struct sal_sock {
    /** sockets currently are built on netconns, each socket has one netconn */
    /*conn may delete in sal*/
    sal_netconn_t *conn;
    /** data that was left from the previous read */
    void *lastdata;
    /** offset in the data that was left from the previous read */
    uint16_t lastoffset;
    /** number of times data was received, set by event_callback(),
        tested by the receive and select functions */
    int16_t rcvevent;
    /** number of times data was ACKed (free send buffer),
        set by event_callback(), tested by select */
    uint16_t sendevent;
    /** error happened for this socket, set by event_callback(),
        tested by select */
    uint16_t errevent;
    /** last error that occurred on this socket (in fact,
        all our errnos fit into an uint8_t) */
    uint8_t err;
    /** counter of how many threads are waiting for this socket using select */
    SELWAIT_T select_waiting;
};

typedef struct sal_netbuf {
    void      *payload;
    u16_t     len;
    ip_addr_t addr;
    u16_t     port;
} sal_netbuf_t;

typedef struct sal_outputbuf {
    void *payload;
    u16_t len;
    u16_t remote_port;
    char  remote_ip[16];
} sal_outputbuf_t;

/** Description for a task waiting in select */
struct sal_select_cb {
    /** Pointer to the next waiting task */
    struct sal_select_cb *next;
    /** Pointer to the previous waiting task */
    struct sal_select_cb *prev;
    /** readset passed to select */
    fd_set *readset;
    /** writeset passed to select */
    fd_set *writeset;
    /** unimplemented: exceptset passed to select */
    fd_set *exceptset;
    /** don't signal the same semaphore twice: set to 1 when signalled */
    int sem_signalled;
    /** semaphore to wake up a task waiting for select */
    //SELECT_SEM_T sem;
    /** semaphore to wake up a task waiting for select */
    sal_sem_t *psem;
};

/** A struct sockaddr replacement that has the same alignment as sockaddr_in/
 *  sockaddr_in6 if instantiated.
 */
union sockaddr_aligned {
    struct sockaddr sa;
#if LWIP_IPV6
    struct sockaddr_in6 sin6;
#endif /* LWIP_IPV6 */
#if LWIP_IPV4
    struct sockaddr_in sin;
#endif /* LWIP_IPV4 */
};

#define IS_SOCK_ADDR_LEN_VALID(namelen)  \
    ((namelen) == sizeof(struct sockaddr_in))

#ifndef set_errno
#define set_errno(err) do { if (err) { errno = (err); } } while(0)
#endif

#define sock_set_errno(sk,e) do { \
        const int sockerr = (e); \
        sk->err = (u8_t)sockerr; \
        set_errno(sockerr); \
    } while (0)

/** The global array of available sockets */
static struct sal_sock sockets[NUM_SOCKETS];
/** The global array of available events */
static struct sal_event events[NUM_EVENTS];
/** The global list of tasks waiting for select */
static struct sal_select_cb *select_cb_list;
/** This counter is increased from sal_select when the list is changed
    and checked in event_callback to see if it has changed. */
static volatile int select_cb_ctr;

/* From http://www.iana.org/assignments/port-numbers:
   "The Dynamic and/or Private Ports are those from 49152 through 65535" */
#define LOCAL_PORT_RANGE_START  0xc000
#define LOCAL_PORT_RANGE_END    0xffff
#define ENSURE_LOCAL_PORT_RANGE(port) ((u16_t)(((port) & ~LOCAL_PORT_RANGE_START) + LOCAL_PORT_RANGE_START))

struct udp_pcb *sal_udp_pcbs;

struct tcp_pcb *sal_tcp_pcbs;

sal_mutex_t    lock_sal_core;

#define LOCK_SAL_CORE    sal_mutex_lock(&lock_sal_core)

#define UNLOCK_SAL_CORE  sal_mutex_unlock(&lock_sal_core)

void sal_deal_event(int s, enum netconn_evt evt);

static bool sal_init_done = 0;
#if SAL_PACKET_SEND_MODE_ASYNC
static bool sal_send_async_running = 0;
#endif

static u16_t sal_tcp_new_port(void)
{
    static u16_t tcp_port = 0;
    u16_t        n = 0;
    struct tcp_pcb *pcb = NULL;

    if (tcp_port == 0) {
        tcp_port = ENSURE_LOCAL_PORT_RANGE(rand());
    }

again:

    if (tcp_port++ == LOCAL_PORT_RANGE_END) {
        tcp_port = LOCAL_PORT_RANGE_START;
    }

    for (pcb = sal_tcp_pcbs; pcb != NULL; pcb = pcb->next) {
        if (pcb->local_port == tcp_port) {
            if (++n > (LOCAL_PORT_RANGE_END - LOCAL_PORT_RANGE_START)) {
                return 0;
            }

            goto again;
        }
    }

    return tcp_port;
}

static err_t sal_tcp_bind(struct tcp_pcb *pcb, const ip_addr_t *ipaddr, u16_t port)
{
    struct tcp_pcb *pstpcb = NULL;
    u8_t            rebind = 0;

    /*for local ip addr is not used for now,
      if ip addr is used there is something else need to check link udp_bind*/
    if (pcb == NULL) {
        return ERR_VAL;
    }

    for (pstpcb = sal_tcp_pcbs; pstpcb != NULL; pstpcb = pstpcb->next) {
        if (pstpcb == pcb) {
            rebind = 1;
            break;
        }
    }

    if (port == 0) {
        port = sal_tcp_new_port();

        if (port == 0) {
            SAL_ERROR("sal_tcp_bind: out of free tcp ports\n");
            return ERR_BUF;
        }
    } else {
        for (pstpcb = sal_tcp_pcbs; pstpcb != NULL; pstpcb = pstpcb->next) {
            if (pcb != pstpcb) {
                /*By default, we don't allow to bind a port that any other udp pcb is already bound to.
                  ip add port are matches */
                if (pstpcb->local_port == port) {
                    return ERR_USE;
                }
            }
        }
    }

    /*TODO ipaddr = null means any addr*/
    if (NULL != ipaddr) {
        ip_addr_set_ipaddr(&(pcb->local_ip), ipaddr);
    }

    pcb->local_port = port;

    /* pcb not active yet? */
    if (rebind == 0) {
        /* place the PCB on the active list if not already there */
        pcb->next = sal_tcp_pcbs;
        sal_tcp_pcbs = pcb;
    }

    return ERR_OK;

}

static void sal_tcp_remove(struct tcp_pcb *pcb)
{
    struct tcp_pcb *pstpcb;

    /* pcb to be removed is first in list? */
    if (sal_tcp_pcbs == pcb) {
        /* make list start at 2nd pcb */
        sal_tcp_pcbs = sal_tcp_pcbs->next;
        /* pcb not 1st in list */
    } else {
        for (pstpcb = sal_tcp_pcbs; pstpcb != NULL; pstpcb = pstpcb->next) {
            /* find pcb in udp_pcbs list */
            if (pstpcb->next != NULL && pstpcb->next == pcb) {
                /* remove pcb from list */
                pstpcb->next = pcb->next;
                break;
            }
        }
    }

    free(pcb);

}

static u16_t sal_udp_new_port(void)
{
    static u16_t   udp_port = 0;
    u16_t          n = 0;
    struct udp_pcb *pcb;

    if (udp_port == 0) {
        udp_port = ENSURE_LOCAL_PORT_RANGE(rand());
    }

again:

    if (udp_port++ == LOCAL_PORT_RANGE_END) {
        udp_port = LOCAL_PORT_RANGE_START;
    }

    for (pcb = sal_udp_pcbs; pcb != NULL; pcb = pcb->next) {
        if (pcb->local_port == udp_port) {
            if (++n > (LOCAL_PORT_RANGE_END - LOCAL_PORT_RANGE_START)) {
                return 0;
            }

            goto again;
        }
    }

    return udp_port;
}

static err_t sal_udp_bind(struct udp_pcb *pcb, const ip_addr_t *ipaddr, u16_t port)
{
    struct udp_pcb *pstpcb = NULL;
    u8_t            rebind = 0;

    /*for local ip addr is not used for now,
      if ip addr is used there is something else need to check link udp_bind*/
    if (pcb == NULL) {
        return ERR_VAL;
    }

    for (pstpcb = sal_udp_pcbs; pstpcb != NULL; pstpcb = pstpcb->next) {
        if (pstpcb == pcb) {
            rebind = 1;
            break;
        }
    }

    if (port == 0) {
        port = sal_udp_new_port();

        if (port == 0) {
            SAL_ERROR("sal_udp_bind: out of free udp ports\n");
            return ERR_BUF;
        }
    } else {
        for (pstpcb = sal_udp_pcbs; pstpcb != NULL; pstpcb = pstpcb->next) {
            if (pcb != pstpcb) {
                /*By default, we don't allow to bind a port that any other udp pcb is already bound to.
                  ip add port are matches */
                if (pstpcb->local_port == port) {
                    return ERR_USE;
                }
            }
        }
    }

    /*TODO ipaddr = null means any addr*/
    if (NULL != ipaddr) {
        ip_addr_set_ipaddr(&(pcb->local_ip), ipaddr);
    }

    pcb->local_port = port;

    /* pcb not active yet? */
    if (rebind == 0) {
        /* place the PCB on the active list if not already there */
        pcb->next = sal_udp_pcbs;
        sal_udp_pcbs = pcb;
    }

    return ERR_OK;

}

static void sal_udp_remove(struct udp_pcb *pcb)
{
    struct udp_pcb *pstpcb;

    /* pcb to be removed is first in list? */
    if (sal_udp_pcbs == pcb) {
        /* make list start at 2nd pcb */
        sal_udp_pcbs = sal_udp_pcbs->next;
        /* pcb not 1st in list */
    } else {
        for (pstpcb = sal_udp_pcbs; pstpcb != NULL; pstpcb = pstpcb->next) {
            /* find pcb in udp_pcbs list */
            if (pstpcb->next != NULL && pstpcb->next == pcb) {
                /* remove pcb from list */
                pstpcb->next = pcb->next;
                break;
            }
        }
    }

    free(pcb);

}

static void ip4_sockaddr_to_ipstr_port(const struct sockaddr *name, char *ip)
{
    struct sockaddr_in *saddr;
    union {
        uint32_t ip_u32;
        uint8_t ip_u8[4];
    } ip_u;

    if (!name || !ip) {
        return;
    }

    saddr = (struct sockaddr_in *)name;
    memset(ip, 0, 16);

    /* Convert network order ip_addr to ip str (dot number fomrat) */
    ip_u.ip_u32 = (uint32_t)(saddr->sin_addr.s_addr);
    snprintf(ip, SAL_SOCKET_IP4_ADDR_LEN, "%d.%d.%d.%d",
             ip_u.ip_u8[0], ip_u.ip_u8[1], ip_u.ip_u8[2], ip_u.ip_u8[3]);
    ip[SAL_SOCKET_IP4_ADDR_LEN - 1] = '\0';

    SAL_DEBUG("Socket address coverted to %s\n", ip);
}

// Caller to ensure a valid ip string
#if 0
static int ipstr_to_u32(char *ipstr, uint32_t *ip32)
{
    uint8_t *q = (uint8_t *)ip32, n = 0, stridx = 0, dotnum = 0;
    char *p = ipstr;

    if (!ipstr || !ip32) {
        return -1;
    }

    for (n = 0, stridx = 0, dotnum = 0;
         *p != '\0' && stridx < 15 && dotnum < 4;
         stridx++, p++) {
        if (*p == '.') {
            q[dotnum] = n; // saved in network order
            n = 0;
            dotnum++;
            continue;
        }

        if (*p < '0' || *p > '9') {
            return -1;
        }

        n = n * 10 + *p - '0';
    }

    if (dotnum >= 4 || stridx > 15) {
        return -1;
    } else {
        q[dotnum] = n;    // the last number
    }

    return 0;
}
#endif

static void sockaddr_to_ipaddr_port(const struct sockaddr *sockaddr, ip_addr_t *ipaddr, u16_t *port)
{
    SOCKADDR4_TO_IP4ADDR_PORT((const struct sockaddr_in *)(const void *)(sockaddr), ipaddr, *port);
#if LWIP_IPV4 && LWIP_IPV6
    ipaddr->type = IPADDR_TYPE_V4;
#endif
}

int sal_eventfd(unsigned int initval, int flags)
{
    int i;
    SAL_ARCH_DECL_PROTECT(lev);

    /* allocate a new socket identifier */
    for (i = 0; i < NUM_EVENTS; ++i) {
        /* Protect socket array */
        SAL_ARCH_PROTECT(lev);

        if (!events[i].used) {
            events[i].used = 1;
            events[i].counts = 0;
            events[i].reads = 0;
            events[i].writes = 0;
            events[i].psem = NULL;
            SAL_ARCH_UNPROTECT(lev);
            return i + SAL_EVENT_OFFSET;
        }

        SAL_ARCH_UNPROTECT(lev);
    }

    return -1;
}

static struct sal_event *
tryget_event(int s)
{
    s -= SAL_EVENT_OFFSET;

    if ((s < 0) || (s >= NUM_EVENTS)) {
        return NULL;
    }

    if (!events[s].used) {
        return NULL;
    }

    return &events[s];
}

/**
 * Same as get_socket but doesn't set errno
 *
 * @param s externally used socket index
 * @return struct sal_sock for the socket or NULL if not found
 */
static struct sal_sock *tryget_socket(int s)
{
    s -= SAL_SOCKET_OFFSET;

    if ((s < 0) || (s >= NUM_SOCKETS)) {
        return NULL;
    }

    if (!sockets[s].conn) {
        return NULL;
    }

    return &sockets[s];
}

static struct sal_sock *get_socket(int s)
{
    struct sal_sock *sock;

    s -= SAL_SOCKET_OFFSET;

    if ((s < 0) || (s >= NUM_SOCKETS)) {
        SAL_DEBUG("get_socket(%d): invalid", s + SAL_SOCKET_OFFSET);
        set_errno(EBADF);
        return NULL;
    }

    sock = &sockets[s];

    if (!sock->conn) {
        SAL_DEBUG("get_socket(%d): not active", s + SAL_SOCKET_OFFSET);
        set_errno(EBADF);
        return NULL;
    }

    return sock;
}

static int salpcb_new(sal_netconn_t *conn)
{
    if (NULL == conn) {
        SAL_ERROR("salpcb_new fail invalid input\n");
        return -1;
    }

    if (NULL != conn->pcb.tcp) {
        SAL_ERROR("salpcb_new conn %p already have a pcb\n", conn);
        return -1;
    }

    switch (NETCONNTYPE_GROUP(conn->type)) {
    case NETCONN_RAW:
        conn->pcb.raw = aos_zalloc(sizeof(struct raw_pcb));

        if (NULL == conn->pcb.raw) {
            return ERR_MEM;
        }

        break;

    case NETCONN_UDP:
        conn->pcb.udp = aos_zalloc(sizeof(struct udp_pcb));

        if (NULL == conn->pcb.udp) {
            return ERR_MEM;
        }

        break;

    case NETCONN_TCP:
        conn->pcb.tcp = aos_zalloc(sizeof(struct tcp_pcb));

        if (NULL == conn->pcb.tcp) {
            return ERR_MEM;
        }

        break;

    default:
        return ERR_VAL;
    }

    return ERR_OK;
}

static void salnetconn_drain(sal_netconn_t *conn)
{
    sal_netbuf_t *mem;

    if (sal_mbox_valid(&conn->recvmbox)) {
        while (sal_mbox_tryfetch(&conn->recvmbox, (void **)(&mem)) != SAL_MBOX_EMPTY) {
            if (mem != NULL) {
                if (mem->payload) {
                    free(mem->payload);
                    mem->payload = NULL;
                }

                free(mem);
            }
        }

        sal_mbox_free(&conn->recvmbox);
        sal_mbox_set_invalid(&conn->recvmbox);
    }

#if SAL_PACKET_SEND_MODE_ASYNC
    struct sal_sock *sock;
    int s;

    if (sal_mbox_valid(&conn->sendmbox)) {
        s = conn->socket;
        sock = get_socket(s);

        if (sock->sendevent > 0) {
            while (sal_mbox_tryfetch(&conn->sendmbox, (void **)(&mem)) != SAL_MBOX_EMPTY) {
                if (mem != NULL) {
                    if (mem->payload) {
                        free(mem->payload);
                        mem->payload = NULL;
                    }

                    free(mem);
                }
            }

            sal_mbox_free(&conn->sendmbox);
            sal_mbox_set_invalid(&conn->sendmbox);
        }
    }

#endif

#ifdef SAL_SERVER

    if (sal_mbox_valid(&conn->acceptmbox)) {
        while (sal_mbox_tryfetch(&conn->acceptmbox, (void **)(&mem)) != SAL_MBOX_EMPTY) {
            if (mem != NULL) {
                if (mem->payload) {
                    aos_free(mem->payload);
                    mem->payload = NULL;
                }

                aos_free(mem);
            }
        }

        sal_mbox_free(&conn->acceptmbox);
        sal_mbox_set_invalid(&conn->acceptmbox);
    }

#endif

    return;
}

#ifdef SAL_SERVER
static sal_netconn_t *salnetconn_new(enum netconn_type t, bool fromaccept)
#else
static sal_netconn_t *salnetconn_new(enum netconn_type t)
#endif
{
    sal_netconn_t *conn;
    err_t         err = ERR_OK;

    conn = (sal_netconn_t *)aos_zalloc(sizeof(sal_netconn_t));
    if (conn == NULL) {
        SAL_ERROR("salnetconn_new fail to new net conn \n");
        return NULL;
    }

    conn->type = t;
    conn->socket = -1;

    if (sal_mbox_new(&conn->recvmbox, CONFIG_SAL_DEFAULT_INPUTMBOX_SIZE) != ERR_OK) {
        SAL_ERROR("fai to new conn input mail box, size is %d \n", CONFIG_SAL_DEFAULT_INPUTMBOX_SIZE);
        goto err;
    }

#if SAL_PACKET_SEND_MODE_ASYNC

    if (sal_mbox_new(&conn->sendmbox, CONFIG_SAL_DEFAULT_OUTPUTMBOX_SIZE) != ERR_OK) {
        SAL_ERROR("fai to new conn input mail box, size is %d \n", CONFIG_SAL_DEFAULT_INPUTMBOX_SIZE);
        goto err;
    }

#endif

#ifdef  SAL_SERVER

    if (conn->type == NETCONN_TCP && !fromaccept) {
        if (sal_mbox_new(&conn->acceptmbox, CONFIG_SAL_DEFAULT_OUTPUTMBOX_SIZE) != ERR_OK) {
            SAL_ERROR("fai to new conn input mail box, size is %d \n", CONFIG_SAL_DEFAULT_INPUTMBOX_SIZE);
            goto err;
        }
    }

#endif

    err = salpcb_new(conn);

    if (ERR_OK != err) {
        SAL_ERROR("salnetconn_new fail to new pcb return value is %d \n", err);
        goto err;
    }

    return conn;
err:

    if (sal_mbox_valid(&conn->recvmbox)) {
        sal_mbox_free(&conn->recvmbox);
    }

#if SAL_PACKET_SEND_MODE_ASYNC

    if (sal_mbox_valid(&conn->sendmbox)) {
        sal_mbox_free(&conn->sendmbox);
    }

#endif

#ifdef SAL_SERVER

    if (sal_mbox_valid(&conn->acceptmbox)) {
        sal_mbox_free(&conn->acceptmbox);
    }

#endif

    aos_free(conn);

    return NULL;
}

static err_t salnetconn_delete(sal_netconn_t *conn)
{
    struct sal_sock *sock;
    int s;

    if (conn == NULL) {
        return ERR_OK;
    }

    salnetconn_drain(conn);

    if (NULL != conn->pcb.tcp) {
        switch (NETCONNTYPE_GROUP(conn->type)) {
        case NETCONN_TCP:
            sal_tcp_remove(conn->pcb.tcp);
            break;

        case NETCONN_UDP:
            sal_udp_remove(conn->pcb.udp);
            break;

        default:
            SAL_ERROR("Unsupported connect type 0x%x\n", conn->type);
            break;
        }

        //already free in sal_tcp_remove & sal_udp_remove
        //free(conn->pcb.tcp);
        conn->pcb.tcp = NULL;
    }

    s = conn->socket;
    sock = get_socket(s);

    if (sock) {
        sal_netbuf_t *mem = sock->lastdata;
        if (mem != NULL) {
            if (mem->payload) {
                free(mem->payload);
                mem->payload = NULL;
            }
            free(mem);
        }
        sock->conn = NULL;
    }

    free(conn);
    conn = NULL;

    return ERR_OK;
}

static err_t salnetconn_connect(sal_netconn_t *conn, int8_t *addr, u16_t port)
{
    sal_conn_t statconn = {0};
    ip_addr_t remoteipaddr, *remoteip = NULL, remoteip_v = {0,};
    char *ipv4anyadrr = "0.0.0.0";
    err_t err = ERR_OK;

    remoteip = &remoteip_v;

    if (NULL == conn) {
        SAL_ERROR("salnetconn_connect: invalid conn\n");
        return ERR_ARG;
    }

    if (conn->state != NETCONN_NONE) {
        SAL_ERROR("salnetconn_connect: socket %d is connect state is %d \n", conn->socket, conn->state);
        return ERR_ARG;
    }

    statconn.fd = conn->socket;
    statconn.r_port = port;
    statconn.l_port = -1;
    statconn.addr = (char *)addr;

    if (NULL == addr) {
        statconn.addr = ipv4anyadrr;
    }

    switch (NETCONNTYPE_GROUP(conn->type)) {
    case NETCONN_UDP:
        if (strcmp(IPADDR_BROADCAST_STRING, statconn.addr) != 0) {
            statconn.type = UDP_UNICAST;
        } else {
            statconn.type = UDP_BROADCAST;
        }

        if (conn->pcb.udp->local_port == 0) {
            statconn.l_port = sal_udp_new_port();
        } else {
            statconn.l_port = conn->pcb.udp->local_port;
        }

        err = sal_module_start(&statconn);

        if (ERR_OK != err) {
            SAL_ERROR("fail to setup udp connect, remote is %s port is %d.\n", statconn.addr, port);
            return -1;
        }

        if (conn->pcb.udp->local_port == 0) {
            err = sal_udp_bind(conn->pcb.udp, NULL, statconn.l_port);

            if (err != ERR_OK) {
                SAL_ERROR("sal_connect udp pcb %p bind local port %d fail.\n", conn->pcb.udp, statconn.l_port);
                return err;
            }
        }

        if (ip4addr_aton(statconn.addr, (ip4_addr_t *)ip_2_ip4(&remoteipaddr))) {
            memcpy(remoteip, &remoteipaddr, sizeof(ip_addr_t));
        }

        ip_addr_set_ipaddr(&(conn->pcb.udp->remote_ip), remoteip);
        conn->pcb.udp->remote_port = port;
        break;

    case NETCONN_TCP:
        statconn.type = TCP_CLIENT;
        err = sal_module_start(&statconn);

        if (ERR_OK != err) {
            SAL_ERROR("fail to setup tcp connect, remote is %s port is %d.\n", statconn.addr, port);
            return -1;
        }

        if (ip4addr_aton(statconn.addr, (ip4_addr_t *)ip_2_ip4(&remoteipaddr))) {
            memcpy(remoteip, &remoteipaddr, sizeof(ip_addr_t));
        }

        ip_addr_set_ipaddr(&(conn->pcb.tcp->remote_ip), remoteip);
        conn->pcb.tcp->remote_port = port;
        break;

    default:
        SAL_ERROR("Unsupported sal connection type.\n");
        return ERR_ARG;
    }

    /*init socket send event*/
#if SAL_PACKET_SEND_MODE_ASYNC
    struct sal_sock *sock;

    sock = get_socket(conn->socket);
    sock->sendevent = CONFIG_SAL_DEFAULT_OUTPUTMBOX_SIZE;
#else
    sal_deal_event(conn->socket, NETCONN_EVT_SENDPLUS);
#endif

    /* Update sal conn state here */
    conn->state = NETCONN_CONNECT;

    return ERR_OK;

}

#ifdef SAL_SERVER
static err_t salnetconn_listen(sal_netconn_t *conn)
{
    err_t err = ERR_OK;
    sal_conn_t statconn = {0};
#ifdef SAL_SERVER
    char *srcaddr = IPADDR_ANY_STRING;
#endif

    if (NULL == conn || NULL == conn->pcb.tcp) {
        return ERR_ARG;
    }

    if (NETCONNTYPE_GROUP(conn->type) != NETCONN_TCP) {
        SAL_ERROR("sal_listen is not conn type is not tcp\n");
        return ERR_ARG;
    }

    if (conn->state == NETCONN_LISTEN) {
        return ERR_OK;
    }

    if (conn->state != NETCONN_NONE) {
        SAL_ERROR("socket connect state is %d, can not listen\n", conn->socket);
        return ERR_ARG;
    }

    /*TO DO If local port is 0, what can we do*/
    statconn.fd = conn->socket;
#ifdef SAL_SERVER
    statconn.addr = srcaddr;
#endif
    statconn.l_port = conn->pcb.tcp->local_port;
    statconn.type = TCP_SERVER;
    err = sal_module_start(&statconn);

    if (ERR_OK != err) {
        SAL_ERROR("fail to listen %d, local port is %d.\n", conn->socket, statconn.l_port);
        return ERR_ARG;
    }

    conn->state = NETCONN_LISTEN;

    return err;
}
#endif

static err_t salnetconn_bind(sal_netconn_t *conn, const ip_addr_t *addr, u16_t port)
{
    err_t err = ERR_OK;
#ifdef SAL_SERVER
    sal_conn_t statconn = {0};
    char *srcaddr = IPADDR_ANY_STRING;
#endif

    if (NULL == conn) {
        SAL_ERROR("salnetconn_bind: invalid conn\n");
        return ERR_ARG;
    }

    switch (NETCONNTYPE_GROUP(conn->type)) {
    case NETCONN_UDP:
        err = sal_udp_bind(conn->pcb.udp, addr, port);

        if (err != ERR_OK) {
            SAL_ERROR("salnetconn_bind sock %d udp bind fail\n.", conn->socket);
            return err;
        }

#ifdef SAL_SERVER
        statconn.fd = conn->socket;
        statconn.addr = srcaddr;
        statconn.r_port = port; // TODO: remote port now equals to local port
        statconn.l_port = port;
        statconn.type = UDP_UNICAST;

        err = sal_module_start(&statconn);

        if (ERR_OK != err) {
            SAL_ERROR("fail to setup udp bind, local port is %d.\n", port);
            return -1;
        }

#endif
        break;

    case NETCONN_TCP:
        err = sal_tcp_bind(conn->pcb.tcp, addr, port);

        if (err != ERR_OK) {
            SAL_ERROR("salnetconn_bind sock %d tcp bind fail\n.", conn->socket);
            return err;
        }

        break;

    case NETCONN_RAW:
    default:
        /*for now wifi module did not support raw socket yet*/
        SAL_ERROR("salnetconn_bind invalid connect type %d.\n", NETCONNTYPE_GROUP(conn->type));
        return ERR_VAL;
    }

    return ERR_OK;

}

static err_t salnetconn_recv_data(sal_netconn_t *conn, sal_netbuf_t **new_buf)
{
    int ret = 0;
    void *buf = NULL;

    if (NULL == conn || NULL == new_buf) {
        SAL_ERROR("invalid input\n");
        return ERR_ARG;
    }

    if (!sal_mbox_valid(&conn->recvmbox)) {
        SAL_ERROR("socket %d connect invalid recvmbox\n", conn->socket);
        return ERR_CONN;
    }

    if (conn->state == NETCONN_CLOSE) {
        ret = sal_arch_mbox_tryfetch(&conn->recvmbox, &buf);
        if (ret == SYS_MBOX_EMPTY) {
            return ERR_CLSD;
        } else {
            sal_deal_event(conn->socket, NETCONN_EVT_RCVMINUS);
            *new_buf = buf;
            return ERR_OK;
        }
    }

    ret = sal_arch_mbox_fetch(&conn->recvmbox, &buf, conn->recv_timeout);

    if (ret == SAL_ARCH_TIMEOUT) {
        SAL_ERROR("sal recv data time out, socket %d conn %p timeout %d\n", conn->socket, conn, conn->recv_timeout);
        return ERR_TIMEOUT;
    }

    sal_deal_event(conn->socket, NETCONN_EVT_RCVMINUS);
    *new_buf = buf;
    return ERR_OK;
}

#ifdef SAL_SERVER
static err_t salnetconn_accept(sal_netconn_t *conn, sal_netconn_t **new_conn)
{
    void *accept_ptr;

    if (NULL == new_conn) {
        SAL_ERROR("sal accept Invaild input\n");
        return ERR_ARG;
    }

    LOG("in salnetconn_accept\n");
    *new_conn = NULL;

    if (NULL == conn || conn->type != NETCONN_TCP) {
        SAL_ERROR("sal accept Invaild input\n");
        return ERR_ARG;
    }

    if (!sal_mbox_valid(&conn->acceptmbox)) {
        SAL_ERROR("invalid connection to accept box\n");
        return ERR_CLSD;
    }

    if (sal_arch_mbox_fetch(&conn->acceptmbox, &accept_ptr, conn->recv_timeout) == SAL_ARCH_TIMEOUT) {
        SAL_ERROR("sal accept connection time out, socket %d conn %p timeout %d\n",
                  conn->socket, conn, conn->recv_timeout);
        return ERR_TIMEOUT;
    }

    sal_deal_event(conn->socket, NETCONN_EVT_RCVMINUS);
    *new_conn = (sal_netconn_t *)accept_ptr;

    if (*new_conn == NULL) {
        SAL_ERROR("sal fail to accept new connect\n");
        return ERR_CLSD;
    }

    return ERR_OK;
}

err_t salnetconn_getaddr(sal_netconn_t *conn, ip_addr_t *addr, u16_t *port)
{
    if (NULL == conn || NULL == addr || NULL == port) {
        SAL_ERROR("invalid input\n");
        return ERR_ARG;
    }

    if (conn->type == NETCONN_TCP) {
        *addr = conn->pcb.tcp->remote_ip;
        *port = conn->pcb.tcp->remote_port;
    } else if (conn->type == NETCONN_UDP) {
        *addr = conn->pcb.udp->remote_ip;
        *port = conn->pcb.udp->remote_port;
    } else {
        SAL_ERROR("Unsupported connect type 0x%x\n", conn->type);
        return ERR_ARG;
    }

    return ERR_OK;
}

int sal_accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
    struct sal_sock *sock, *nsock;
    sal_netconn_t *newconn;
    ip_addr_t naddr;
    u16_t port = 0;
    int newsock;
    err_t err;
    SAL_ARCH_DECL_PROTECT(lev);

    sock = get_socket(s);

    if (!sock) {
        return -1;
    }

    LOG("in sal accept\n");

    if (netconn_is_nonblocking(sock->conn) && (sock->rcvevent <= 0)) {
        SAL_DEBUG("sal_accept(%d): eturning EWOULDBLOCK\n", s);
        sock_set_errno(sock, EWOULDBLOCK);
        return -1;
    }

    /* wait for a new connection */
    err = salnetconn_accept(sock->conn, &newconn);

    if (err != ERR_OK) {
        SAL_DEBUG("sal_accept(%d): netconn_acept failed, err=%d\n", s, err);

        if (NETCONNTYPE_GROUP(sock->conn->type) != NETCONN_TCP) {
            sock_set_errno(sock, EOPNOTSUPP);
        } else if (err == ERR_CLSD) {
            sock_set_errno(sock, EINVAL);
        } else {
            sock_set_errno(sock, err_to_errno(err));
        }

        return -1;
    }

    SAL_ASSERT("newconn != NULL", newconn != NULL);

    newsock = alloc_socket(newconn, 1);

    if (newsock == -1) {
        salnetconn_delete(newconn);
        sock_set_errno(sock, ENFILE);
        return -1;
    }

    SAL_ASSERT("invalid socket index", (newsock >= SAL_SOCKET_OFFSET) && (newsock < NUM_SOCKETS + SAL_SOCKET_OFFSET));
    nsock = &sockets[newsock - SAL_SOCKET_OFFSET];

    /* See event_callback: If data comes in right away after an accept, even
     * though the server task might not have created a new socket yet.
     * In that case, newconn->socket is counted down (newconn->socket--),
     * so nsock->rcvevent is >= 1 here!
     */
    SAL_ARCH_PROTECT(lev);
    //nsock->rcvevent += (s16_t)(-1 - newconn->socket);
    newconn->socket = newsock;
    SAL_ARCH_UNPROTECT(lev);

    /* Note that POSIX only requires us to check addr is non-NULL. addrlen must
     * not be NULL if addr is valid.
     */

    if (addr != NULL) {
        union sockaddr_aligned tempaddr;
        /* get the IP address and port of the remote host */
        // herer
        err = salnetconn_getaddr(newconn, &naddr, &port);

        if (err != ERR_OK) {
            SAL_DEBUG("sal_accept(%d): netconn_peer failed, err=%d\n", s, err);
            salnetconn_delete(newconn);
            free_socket(nsock);
            sock_set_errno(sock, err_to_errno(err));
            return -1;
        }

        SAL_ASSERT("addr valid but addrlen NULL", addrlen != NULL);

        IPADDR_PORT_TO_SOCKADDR(&tempaddr, &naddr, port);

        if (*addrlen > tempaddr.sa.sa_len) {
            *addrlen = tempaddr.sa.sa_len;
        }

        memcpy(addr, &tempaddr, *addrlen);

        LOGD(SAL_TAG, "sal_accept(%d) returning new sock=%d remote %s\n", s, newsock, addr->sa_data);
    } else {
        SAL_DEBUG("sal_accept(%d) returning new sock=%d\n", s, newsock);
    }

    sock_set_errno(sock, 0);
    return newsock;
}
#else
int accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
    return -1;
}
#endif

int sal_select2(int maxfdp1, fd_set *readset, fd_set *writeset,
                fd_set *exceptset, struct timeval *timeout, void *semaphore)
{
    uint32_t waitres = 0;
    sal_sem_t *psem = (sal_sem_t *)semaphore;
    int nready;
    fd_set lreadset, lwriteset, lexceptset;
    uint32_t msectimeout;
    struct sal_select_cb select_cb;
    int i;
    sal_sem_t sem;
    int maxfdp2;
#if SAL_NETCONN_SEM_PER_THREAD
    int waited = 0;
#endif
    SAL_ARCH_DECL_PROTECT(lev);

    SAL_DEBUG("sal_select(%d, %p, %p, %p, tvsec=%d tvusec=%d)",
              maxfdp1, (void *)readset,
              (void *) writeset, (void *) exceptset,
              timeout ? (int32_t)timeout->tv_sec : (int32_t) - 1,
              timeout ? (int32_t)timeout->tv_usec : (int32_t) - 1);

    /* Go through each socket in each list to count number of sockets which
       currently match */
    nready = sal_selscan(maxfdp1, readset, writeset, exceptset,
                         &lreadset, &lwriteset, &lexceptset);

    /* If we don't have any current events, then suspend
       if we are supposed to */
    if (!nready) {
        if (timeout && timeout->tv_sec == 0 && timeout->tv_usec == 0) {
            SAL_DEBUG("sal_select: no timeout, returning 0");
            /* This is OK as the local fdsets are empty and nready is zero,
               or we would have returned earlier. */
            goto return_copy_fdsets;
        }

        /* None ready: add our semaphore to list:
           We don't actually need any dynamic memory. Our entry on the
           list is only valid while we are in this function, so it's ok
           to use local variables. */

        select_cb.next = NULL;
        select_cb.prev = NULL;
        select_cb.readset = readset;
        select_cb.writeset = writeset;
        select_cb.exceptset = exceptset;
        select_cb.sem_signalled = 0;
#if SAL_NETCONN_SEM_PER_THREAD
        select_cb.psem = SAL_NETCONN_THREAD_SEM_GET();
#else /* SAL_NETCONN_SEM_PER_THREAD */

        if (psem) {
            select_cb.psem = psem;
        } else {
            if (sal_sem_new(&sem, 0) != ERR_OK) {
                /* failed to create semaphore */
                set_errno(ENOMEM);
                return -1;
            }

            select_cb.psem = &sem;
        }

#endif /* SAL_NETCONN_SEM_PER_THREAD */

        /* Protect the select_cb_list */
        SAL_ARCH_PROTECT(lev);

        /* Put this select_cb on top of list */
        select_cb.next = select_cb_list;

        if (select_cb_list != NULL) {
            select_cb_list->prev = &select_cb;
        }

        select_cb_list = &select_cb;
        /* Increasing this counter tells event_callback
           that the list has changed. */
        select_cb_ctr++;

        /* Now we can safely unprotect */
        SAL_ARCH_UNPROTECT(lev);

        /* Increase select_waiting for each socket we are interested in */
        maxfdp2 = maxfdp1;

        for (i = SAL_SOCKET_OFFSET; i < maxfdp1; i++) {
            if ((readset && FD_ISSET(i, readset)) ||
                (writeset && FD_ISSET(i, writeset)) ||
                (exceptset && FD_ISSET(i, exceptset))) {
                struct sal_sock *sock;
                struct sal_event *event;
                SAL_ARCH_PROTECT(lev);
                sock = tryget_socket(i);
                event = tryget_event(i);

                if (sock != NULL) {
                    sock->select_waiting++;
                    SAL_ASSERT("sock->select_waiting > 0",
                               sock->select_waiting > 0);
                } else if (event != NULL) {
                    //event->psem = SELECT_SEM_PTR(select_cb.sem);
                    event->psem = select_cb.psem;
                } else {
                    /* Not a valid socket */
                    nready = -1;
                    maxfdp2 = i;
                    SAL_ARCH_UNPROTECT(lev);
                    break;
                }

                SAL_ARCH_UNPROTECT(lev);
            }
        }

        if (nready >= 0) {
            /* Call sal_selscan again: there could have been events between
               the last scan (without us on the list) and putting us on the list! */
            nready = sal_selscan(maxfdp1, readset, writeset, exceptset,
                                 &lreadset, &lwriteset, &lexceptset);

            if (!nready) {
                /* Still none ready, just wait to be woken */
                if (timeout == 0) {
                    /* Wait forever */
                    msectimeout = 0;
                } else {
                    msectimeout = ((timeout->tv_sec * 1000) + \
                                   ((timeout->tv_usec + 500) / 1000));

                    if (msectimeout == 0) {
                        /* Wait 1ms at least (0 means wait forever) */
                        msectimeout = 1;
                    }
                }

                waitres = sal_arch_sem_wait(select_cb.psem /*SELECT_SEM_PTR(select_cb.sem)*/,
                                            msectimeout);
#if SAL_NETCONN_SEM_PER_THREAD
                waited = 1;
#endif
            }
        }

        /* Decrease select_waiting for each socket we are interested in */
        for (i = SAL_SOCKET_OFFSET; i < maxfdp2; i++) {
            if ((readset && FD_ISSET(i, readset)) ||
                (writeset && FD_ISSET(i, writeset)) ||
                (exceptset && FD_ISSET(i, exceptset))) {
                struct sal_sock *sock;
                //struct sal_event *event;
                SAL_ARCH_PROTECT(lev);
                sock = tryget_socket(i);
                //event = tryget_event(i);

                if (sock != NULL) {
                    /* @todo: what if this is a new socket (reallocated?)
                       in this case, select_waiting-- would be wrong
                       (a global 'sockalloc' counter, stored per socket
                       could help) */
                    SAL_ASSERT("sock->select_waiting > 0",
                               sock->select_waiting > 0);

                    if (sock->select_waiting > 0) {
                        sock->select_waiting--;
                    }

                    //} else if (event != NULL) {
                    //    event->psem = NULL;
                } else {
                    /* Not a valid socket */
                    nready = -1;
                }

                SAL_ARCH_UNPROTECT(lev);
            }
        }

        /* Take us off the list */
        SAL_ARCH_PROTECT(lev);

        if (select_cb.next != NULL) {
            select_cb.next->prev = select_cb.prev;
        }

        if (select_cb_list == &select_cb) {
            SAL_ASSERT("select_cb.prev == NULL", select_cb.prev == NULL);
            select_cb_list = select_cb.next;
        } else {
            SAL_ASSERT("select_cb.prev != NULL", select_cb.prev != NULL);
            select_cb.prev->next = select_cb.next;
        }

        /* Increasing this counter tells event_callback
           that the list has changed. */
        select_cb_ctr++;
        SAL_ARCH_UNPROTECT(lev);

#if SAL_NETCONN_SEM_PER_THREAD

        if (select_cb.sem_signalled && (!waited || \
                                        (waitres == SAL_ARCH_TIMEOUT))) {
            /* don't leave the thread-local semaphore signalled */
            sal_arch_sem_wait(select_cb.psem, 1);
        }

#else /* SAL_NETCONN_SEM_PER_THREAD */

        if (!psem) {
            sal_sem_free(select_cb.psem);
        }

#endif /* SAL_NETCONN_SEM_PER_THREAD */

        if (nready < 0) {
            /* This happens when a socket got closed while waiting */
            set_errno(EBADF);
            return -1;
        }

        if (waitres == SAL_ARCH_TIMEOUT) {
            /* Timeout */
            SAL_DEBUG("sal_select: timeout expired");
            /* This is OK as the local fdsets are empty and nready is zero,
               or we would have returned earlier. */
            goto return_copy_fdsets;
        }

        /* See what's set */
        nready = sal_selscan(maxfdp1, readset, writeset, exceptset,
                             &lreadset, &lwriteset, &lexceptset);
    }

    SAL_DEBUG("sal_select: nready=%d", nready);
return_copy_fdsets:
    set_errno(0);

    if (readset) {
        *readset = lreadset;
    }

    if (writeset) {
        *writeset = lwriteset;
    }

    if (exceptset) {
        *exceptset = lexceptset;
    }

    return nready;
}

int
sal_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
           struct timeval *timeout)
{
    return sal_select2(maxfdp1, readset, writeset, exceptset, timeout, NULL);
}

//把有事件的标出来
static int sal_selscan(int maxfdp1, fd_set *readset_in, fd_set *writeset_in,
                       fd_set *exceptset_in, fd_set *readset_out,
                       fd_set *writeset_out, fd_set *exceptset_out)
{
    int i, nready = 0;
    fd_set lreadset, lwriteset, lexceptset;
    struct sal_sock *sock;
    struct sal_event *event;
    SAL_ARCH_DECL_PROTECT(lev);

    FD_ZERO(&lreadset);
    FD_ZERO(&lwriteset);
    FD_ZERO(&lexceptset);

    /* Go through each socket in each list to count number of sockets which
       currently match */
    for (i = SAL_SOCKET_OFFSET; i < maxfdp1; i++) {
        /* if this FD is not in the set, continue */
        if (!(readset_in && FD_ISSET(i, readset_in)) &&
            !(writeset_in && FD_ISSET(i, writeset_in)) &&
            !(exceptset_in && FD_ISSET(i, exceptset_in))) {
            continue;
        }

        /* First get the socket's status (protected)... */
        SAL_ARCH_PROTECT(lev);
        sock = tryget_socket(i);
        event = tryget_event(i);

        if (sock != NULL || event != NULL) {
            void *lastdata = sock ? sock->lastdata : NULL;
            int16_t rcvevent = sock ? sock->rcvevent : event->reads;
            uint16_t sendevent = sock ? sock->sendevent : event->writes;
            uint16_t errevent = sock ? sock->errevent : 0;
            SAL_ARCH_UNPROTECT(lev);

            /* See if netconn of this socket is ready for read */
            if (readset_in && FD_ISSET(i, readset_in) && \
                ((lastdata != NULL) || (rcvevent > 0))) {
                FD_SET(i, &lreadset);
                SAL_DEBUG("sal_selscan: fd=%d ready for reading", i);
                nready++;
            }

            /* See if netconn of this socket is ready for write */
            if (writeset_in && FD_ISSET(i, writeset_in) && (sendevent != 0)) {
                FD_SET(i, &lwriteset);
                SAL_DEBUG("sal_selscan: fd=%d ready for writing", i);
                nready++;
            }

            /* See if netconn of this socket had an error */
            if (exceptset_in && FD_ISSET(i, exceptset_in) && (errevent != 0)) {
                FD_SET(i, &lexceptset);
                SAL_DEBUG("sal_selscan: fd=%d ready for exception", i);
                nready++;
            }
        } else {
            SAL_ARCH_UNPROTECT(lev);
            /* continue on to next FD in list */
        }
    }

    /* copy local sets to the ones provided as arguments */
    *readset_out = lreadset;
    *writeset_out = lwriteset;
    *exceptset_out = lexceptset;

    SAL_ASSERT("nready >= 0", nready >= 0);
    return nready;
}

ssize_t sal_recvfrom(int s, void *mem, size_t len, int flags,
                     struct sockaddr *from, socklen_t *fromlen)
{
    struct sal_sock        *pstsock = NULL;
    sal_netbuf_t           *buf = NULL;
    int                    off = 0;
    u16_t                  buflen = 0;
    u16_t                  copylen = 0;
    err_t                  err = ERR_OK;
    uint8_t                done = 0;
    union sockaddr_aligned saddr;

    if (NULL == mem || 0 == len) {
        SAL_ERROR("sal_recvfrom invalid input\n");
        return -1;
    }

    SAL_DEBUG("sal recvfrom input s=%d mem=%p len=%u flags=0x%x \n", s, mem, len, flags);

    pstsock = get_socket(s);

    if (NULL == pstsock) {
        SAL_ERROR("sal_recvfrom cannot get socket %d\n", s);
        return -1;
    }

    do {
        SAL_DEBUG("sal recvfrom : top while sock->lastdata=%p\n", pstsock->lastdata);

        if (pstsock->lastdata) {
            buf = pstsock->lastdata;
        } else {
            if (((flags & MSG_DONTWAIT) || netconn_is_nonblocking(pstsock->conn)) &&
                (pstsock->rcvevent <= 0)) {
                if (off > 0) {
                    sock_set_errno(pstsock, 0);
                    return off;
                }

                SAL_ERROR("sal_recvfrom(%d): returning EWOULDBLOCK\n", s);
                sock_set_errno(pstsock, EWOULDBLOCK);
                return -1;
            }

            err = salnetconn_recv_data(pstsock->conn, &buf);
            if (err < 0) {
                SAL_ERROR("sal_recvfrom neconn_recv err=%d, netbuf=%p\r\n", err, buf);
            }

            if (err == ERR_CLSD) {
                return 0;
            }

            if (err != ERR_OK || buf == NULL || buf->payload == NULL) {
                if (off > 0) {
                    sock_set_errno(pstsock, 0);
                    return off;
                }

                sock_set_errno(pstsock, err_to_errno(err));
                return -1;
            }

            pstsock->lastdata = buf;
            pstsock->lastoffset = 0;
        }

        buflen = buf->len;
        SAL_DEBUG("sal recvfrom: buflen=%u, len=%u, off=%d, lastoffset=%u\n", buflen, len, off, pstsock->lastoffset);

        buflen -= pstsock->lastoffset;

        if (len - off > buflen) {
            copylen = buflen;
        } else {
            copylen = len - off;
        }

        memcpy(&((u8_t *)mem)[off], &((u8_t *)buf->payload)[pstsock->lastoffset], copylen);
        off += copylen;

        if (NETCONNTYPE_GROUP(pstsock->conn->type) == NETCONN_TCP) {
            if (len < copylen) {
                pstsock->lastdata = NULL;
                pstsock->lastoffset = 0;
                free(buf->payload);
                free(buf);
                buf = NULL;
                SAL_ERROR("invalid copylen %d, len = %d, it would underflow\n", copylen, len);
                return -1;
            }

            if ((len == off) || (pstsock->rcvevent <= 0) || ((flags & MSG_PEEK) != 0)) {
                done = 1;
            }
        } else {
            done = 1;
        }

        if (done) {
            if (from && fromlen) {
                if (NETCONNTYPE_GROUP(pstsock->conn->type) == NETCONN_TCP) {
                    IPADDR_PORT_TO_SOCKADDR(&saddr, &(pstsock->conn->pcb.tcp->remote_ip), (pstsock->conn->pcb.tcp->remote_port));
                } else {
                    if (ip_addr_isany(&buf->addr) && (buf->port == 0)) {
                        IPADDR_PORT_TO_SOCKADDR(&saddr, &(pstsock->conn->pcb.udp->remote_ip), (pstsock->conn->pcb.udp->remote_port));
                    } else {
                        IPADDR_PORT_TO_SOCKADDR(&saddr, &(buf->addr), buf->port);
                    }
                }

                if (*fromlen > saddr.sa.sa_len) {
                    *fromlen = saddr.sa.sa_len;
                }

                memcpy(from, &saddr, *fromlen);
            }
        }

        if ((flags & MSG_PEEK)  == 0) {
            if ((NETCONNTYPE_GROUP(pstsock->conn->type) == NETCONN_TCP) && (buflen > copylen)) {
                pstsock->lastdata = buf;
                pstsock->lastoffset += copylen;
            } else {
                pstsock->lastdata = NULL;
                pstsock->lastoffset = 0;
                free(buf->payload);
                free(buf);
                buf = NULL;
            }
        }
    } while (!done);

    sock_set_errno(pstsock, 0);
    return off;
}

int sal_read(int s, void *mem, int len)
{
    return sal_recvfrom(s, mem, len, 0, NULL, NULL);
}

ssize_t sal_recv(int s, void *mem, size_t len, int flags)
{
    return sal_recvfrom(s, mem, len, flags, NULL, NULL);
}

ssize_t sal_sendto(int s, const void *data, size_t size, int flags,
                   const struct sockaddr *to, socklen_t tolen)
{
    struct sal_sock *pstsalsock = NULL;
    err_t           err = ERR_OK;
    ip_addr_t       remote_addr;
    u16_t           remote_port;
    int8_t          ip_str[SAL_SOCKET_IP4_ADDR_LEN] = {0};

#if SAL_PACKET_SEND_MODE_ASYNC
    if (NULL == data || size == 0 || size > SAL_SOCKET_MAX_PAYLOAD_SIZE) {
#else
    if (NULL == data || size == 0) {
#endif
        SAL_ERROR("sal_send fail to data to send is %p or size is %d\n", data, size);
        return -ERR_ARG;
    }

    pstsalsock = get_socket(s);
    if (NULL == pstsalsock) {
        SAL_ERROR("sal_sendto fail to get sal socket by fd %d \n", s);
        return ERR_ARG;
    }

    if (pstsalsock->conn == NULL) {
        SAL_ERROR("sal_sendto sal socket %d conn is null\n", s);
        return ERR_ARG;
    }

    /* TODO have no consider tcp server send to client*/
    if (NETCONNTYPE_GROUP(pstsalsock->conn->type) == NETCONN_TCP) {
        if (pstsalsock->conn->state == NETCONN_NONE) {
            SAL_ERROR("sal_sendto socket %d connect state is %d\n", s, pstsalsock->conn->state);
            return ERR_VAL;
        }
    }

    if (NETCONNTYPE_GROUP(pstsalsock->conn->type) == NETCONN_UDP) {
        /*TO DO if to != NULL, check sockaddr match socket type or not*/
        if (to) {
            if (pstsalsock->conn->state == NETCONN_NONE) {
                sockaddr_to_ipaddr_port(to, &remote_addr, &remote_port);
                ip4_sockaddr_to_ipstr_port(to, (char *)ip_str);
                LOCK_SAL_CORE;
                err = salnetconn_connect(pstsalsock->conn, ip_str, remote_port);
                UNLOCK_SAL_CORE;

                if (ERR_OK != err) {
                    SAL_ERROR("sal_sendto fail to connect socket %d\n", s);
                    return err;
                }
            }
        } else {
            if (pstsalsock->conn->state == NETCONN_NONE) {
                SAL_ERROR("sal_sendto  socket %d is not connected and "
                          "input addr is null, cannot send packet\n", s);
                return ERR_ARG;
            }
        }
    }

#if SAL_PACKET_SEND_MODE_ASYNC
    sal_outputbuf_t *buf = NULL;
    buf = (sal_outputbuf_t *)aos_zalloc(sizeof(sal_outputbuf_t));
    if (NULL == buf) {
        SAL_ERROR("memory is not enough, malloc size %d fail\n", sizeof(sal_outputbuf_t));
        return -1;
    }

    buf->payload = aos_malloc(size);
    if (NULL == buf->payload) {
        free(buf);
        SAL_ERROR("memory is no enough, malloc size %d fail\n", size);
        return -1;
    }

    buf->len = size;
    memcpy(buf->payload, data, size);

    if (sal_mbox_trypost(&pstsalsock->conn->sendmbox, buf) != ERR_OK) {
        free(buf->payload);
        free(buf);
        sock_set_errno(pstsalsock, EAGAIN);
        SAL_ERROR("%s try post output packet fail \n", __FUNCTION__);
        //return -1;
    } else {
        sal_deal_event(s, NETCONN_EVT_SENDMINUS);
    }

#else
    sal_deal_event(s, NETCONN_EVT_SENDMINUS);
    LOCK_SAL_CORE;
    if (sal_module_send(s, (uint8_t *)data, size, NULL, -1, pstsalsock->conn->send_timeout)) {
        SAL_ERROR("socket %d fail to send packet, do nothing for now \r\n", s);
        UNLOCK_SAL_CORE;
        return -1;
    }
    UNLOCK_SAL_CORE;
    sal_deal_event(s, NETCONN_EVT_SENDPLUS);
#endif

    return size;
}

ssize_t sal_send(int s, const void *data, size_t size, int flags)
{
    SAL_DEBUG("sal_send(%d, flags=0x%x)\n", s, flags);

    return sal_sendto(s, data, size, flags, NULL, 0);
}

int sal_write(int s, const void *data, int size)
{
    struct sal_event *event = tryget_event(s);

    if (event) {
        SAL_ARCH_DECL_PROTECT(lev);

        if (size != sizeof(uint64_t)) {
            return -1;
        }

        SAL_ARCH_PROTECT(lev);
        event->counts += *(uint64_t *)data;

        if (event->counts) {
            event->reads = event->counts;

            if (event->psem) {
                sal_sem_signal(event->psem);
            }
        }

        SAL_ARCH_UNPROTECT(lev);
        return size;
    }

    return sal_send(s, data, size, 0);
}

static int salnetconn_packet_input(sal_netconn_t *conn, void *data, size_t len, char remote_ip[16], uint16_t remote_port)
{
    sal_netbuf_t *buf;

    if (NULL == conn || !sal_mbox_valid(&conn->recvmbox)) {
        SAL_ERROR("invalid connection to input packet\n");
        return -1;
    }

    buf = (sal_netbuf_t *)aos_zalloc(sizeof(sal_netbuf_t));
    if (NULL == buf) {
        SAL_ERROR("memory is not enough, malloc size %d fail\n", sizeof(sal_netbuf_t));
        return -1;
    }


    buf->payload = aos_malloc(len);
    if (NULL == buf->payload) {
        free(buf);
        SAL_ERROR("memory is no enough, malloc size %d fail\n", len);
        return -1;
    }

    memcpy(buf->payload, data, len);
    buf->len = len;
#if LWIP_IPV4 && LWIP_IPV6
    buf->addr.type = IPADDR_TYPE_V4;
#endif

    if (NULL != remote_ip) {
        //ipstr_to_u32(remote_ip, &buf->addr.u_addr.ip4.addr);
        ip4addr_aton(remote_ip, (ip4_addr_t *)ip_2_ip4(&buf->addr));
    }

    buf->port = remote_port;

    if (sal_mbox_trypost(&conn->recvmbox, buf) != ERR_OK) {
        free(buf->payload);
        free(buf);
        SAL_ERROR("try post recv packet fail \n");
        return -1;
    }

    return 0;
}

#ifdef SAL_SERVER
int sal_client_status_notify(int fd, client_status_t status, char remote_ip[16], uint16_t remote_port)
{
    struct sal_sock *sock = NULL;
    sal_netconn_t *newconn = NULL;

    LOG("in sal_client_status_notify\n");
    sock = get_socket(fd);

    if (NULL == sock) {
        SAL_ERROR("low level invalid input: socket %d doesn't exist\n", fd);
        return -1;
    }

    if (status != CLIENT_CONNECTED && status != CLIENT_CLOSED) {
        SAL_ERROR("invalid remote client status %d\n", status);
        return -1;
    }

    if (sock->conn == NULL) {
        SAL_ERROR("socket %d invalid for haven't creat connnet yet\n", fd);
        return -1;
    }

    if (sock->conn->type != NETCONN_TCP) {
        SAL_ERROR("socket %d invalid connection type %d\n", fd, sock->conn->type);
        return -1;
    }

    if (!sal_mbox_valid(&sock->conn->acceptmbox)) {
        SAL_ERROR("socket %d invalid accept mbox\n", fd);
        return -1;
    }

    LOGD(SAL_TAG, "%s low layer notify client %s port %d status %d\n", __func__, remote_ip, remote_port, status);

    newconn = salnetconn_new(NETCONN_TCP, true);

    if (NULL == newconn) {
        SAL_ERROR("socket %d fail to allocate new netconn\n", fd);
        return -1;
    }

    if (NULL == newconn->pcb.tcp) {
        salnetconn_delete(newconn);
        SAL_ERROR("socket %d fail to allocate pcb for a new netconn\n", fd);
        return -1;
    }

    if (NULL != remote_ip) {
        ipstr_to_u32(remote_ip, &newconn->pcb.tcp->remote_ip.u_addr.ip4.addr);
    }

    newconn->pcb.tcp->remote_port = remote_port;
    newconn->pcb.tcp->remote_ip.type = IPADDR_TYPE_V4;

    if (sal_mbox_trypost(&sock->conn->acceptmbox, newconn) != ERR_OK) {
        salnetconn_delete(newconn);
        SAL_ERROR("try post accept fail\n");
        return -1;
    }

    sal_deal_event(fd, NETCONN_EVT_RCVPLUS);

    LOGD(SAL_TAG, "%s low layer notify newconn post from ip %u port %u!\n", __func__,
         newconn->pcb.tcp->remote_ip.u_addr.ip4.addr, newconn->pcb.tcp->remote_port);

    return ERR_OK;
}

static int find_sockfd_by_type_remote_ip_port(enum netconn_type type,
        char remote_ip[16],
        uint16_t remote_port)
{
    int i;
    uint32_t ip32;
    bool found = false;
    SAL_ARCH_DECL_PROTECT(lev);

    if (NULL == remote_ip) {
        LOG("type: %d remote port %d\n", type, remote_port);
        return -1;
    }

    ipstr_to_u32(remote_ip, &ip32);

    for (i = 0; i < NUM_SOCKETS; i++) {
        /* Protect socket array */
        SAL_ARCH_PROTECT(lev);

        if (sockets[i].conn && sockets[i].conn->type == type) {
            if (NETCONN_TCP == type) {
                if (sockets[i].conn->pcb.tcp->remote_ip.u_addr.ip4.addr == ip32 &&
                    sockets[i].conn->pcb.tcp->remote_port == remote_port) {
                    found = true;
                }
            } else if (NETCONN_UDP == type) {
                if (sockets[i].conn->pcb.udp->remote_ip.u_addr.ip4.addr == ip32 &&
                    sockets[i].conn->pcb.udp->remote_port == remote_port) {
                    found = true;
                }
            }
        }

        SAL_ARCH_UNPROTECT(lev);

        if (found) {
            return i + SAL_SOCKET_OFFSET;
        }
    }

    return -1;
}
#endif

int sal_packet_input(int s, void *data, size_t len, char remote_ip[16], uint16_t remote_port)
{
    struct sal_sock *sock = NULL;
    int             ret = 0;
#ifdef SAL_SERVER
    struct sal_sock *client_sock = NULL;
    int sock_fd = -1, client_sock_fd = -1;
#endif

    if (NULL == data || 0 == len) {
        SAL_ERROR("low level invalid input data\n");
        return -1;
    }

    sock = get_socket(s);

    if (NULL == sock) {
        SAL_ERROR("low level invalid input , socket %d doesn't exist\n", s);
        return -1;
    }

    if (sock->conn->state == NETCONN_CLOSE) {
        return 0;
    }

    if (sock->conn == NULL) {
        SAL_ERROR("socket %d invalid for haven't creat connnet yet\n");
        return -1;
    }

#ifdef SAL_SERVER
    sock_fd = s;

    if (NETCONN_TCP == sock->conn->type) {
        // TODO: hard code here
        client_sock_fd = 1;
        client_sock = get_socket(client_sock_fd);

        /*
        client_sock_fd = find_sockfd_by_type_remote_ip_port(NETCONN_TCP, remote_ip, remote_port);
        client_sock = get_socket(client_sock_fd);

        if (NULL == client_sock) {
            SAL_ERROR("socket %d does not match type, remote ip and port\n", client_sock_fd);
            return -1;
        }

        if (NULL == client_sock->conn) {
            SAL_ERROR("socket %d invalid for haven't creat connnet yet\n", client_sock_fd);
            return -1;
        }*/

        if (client_sock_fd != sock_fd) {
            sock_fd = client_sock_fd;
            sock = client_sock;
            LOGD(SAL_TAG, "%s %d socket %d type %d passes handle to socket %d type %d\n",
                 __func__, __LINE__, sock_fd, sock->conn->type, client_sock_fd, client_sock->conn->type);
        }
    }

#endif

    ret = salnetconn_packet_input(sock->conn, data, len, remote_ip, remote_port);

    if (ret) {
        SAL_ERROR("sal packet input fail\n");
        return -1;
    }

#ifdef SAL_SERVER
    sal_deal_event(sock_fd, NETCONN_EVT_RCVPLUS);
#else
    sal_deal_event(s, NETCONN_EVT_RCVPLUS);
#endif

    return ret;
}

int sal_packet_close(int s)
{
    struct sal_sock *sock = NULL;

    sock = get_socket(s);
    sock->conn->state = NETCONN_CLOSE;

    // sal_deal_event(s, NETCONN_EVT_RCVPLUS);

    return 0;
}

void sal_deal_event(int s, enum netconn_evt evt)
{
    struct sal_select_cb *scb;
    int last_select_cb_ctr;
    struct sal_sock *sock = tryget_socket(s);

    if (!sock) {
        return;
    }

    SAL_ARCH_DECL_PROTECT(lev);
    SAL_ARCH_PROTECT(lev);

    /* Set event as required */
    switch (evt) {
    case NETCONN_EVT_RCVPLUS:
        sock->rcvevent++;
        break;

    case NETCONN_EVT_RCVMINUS:
        sock->rcvevent--;
        break;

    case NETCONN_EVT_SENDPLUS:
        sock->sendevent++;
        break;

    case NETCONN_EVT_SENDMINUS:
        sock->sendevent--;
        break;

    case NETCONN_EVT_ERROR:
        sock->errevent = 1;
        break;

    default:
        SAL_ASSERT("unknown event", 0);
        break;
    }

    if (sock->select_waiting == 0) {
        /* noone is waiting for this socket, no need to check select_cb_list */
        SAL_ARCH_UNPROTECT(lev);
        return;
    }

    /* Now decide if anyone is waiting for this socket */
    /* NOTE: This code goes through the select_cb_list list multiple times
       ONLY IF a select was actually waiting. We go through the list the number
       of waiting select calls + 1. This list is expected to be small. */

    /* At this point, SAL_ARCH is still protected! */
again:

    for (scb = select_cb_list; scb != NULL; scb = scb->next) {
        /* remember the state of select_cb_list to detect changes */
        last_select_cb_ctr = select_cb_ctr;

        if (scb->sem_signalled == 0) {
            /* semaphore not signalled yet */
            int do_signal = 0;

            /* Test this select call for our socket */
            if (sock->rcvevent > 0) {
                if (scb->readset && FD_ISSET(s, scb->readset)) {
                    do_signal = 1;
                }
            }

            if (sock->sendevent != 0) {
                if (!do_signal && scb->writeset && \
                    FD_ISSET(s, scb->writeset)) {
                    do_signal = 1;
                }
            }

            if (sock->errevent != 0) {
                if (!do_signal && scb->exceptset && \
                    FD_ISSET(s, scb->exceptset)) {
                    do_signal = 1;
                }
            }

            if (do_signal) {
                scb->sem_signalled = 1;
                /* Don't call SAL_ARCH_UNPROTECT() before signaling the
                   semaphore, as this might lead to the select thread taking
                   itself off the list, invalidating the semaphore. */
                sal_sem_signal(/*SELECT_SEM_PTR*/(scb->psem));
            }
        }

        /* unlock interrupts with each step */
        SAL_ARCH_UNPROTECT(lev);
        /* this makes sure interrupt protection time is short */
        SAL_ARCH_PROTECT(lev);

        if (last_select_cb_ctr != select_cb_ctr) {
            /* someone has changed select_cb_list, restart at the beginning */
            goto again;
        }
    }

    SAL_ARCH_UNPROTECT(lev);

}

/**
 * Allocate a new socket for a given netconn.
 *
 * @param newconn the netconn for which to allocate a socket
 * @param accepted 1 if socket has been created by accept(),
 *                 0 if socket has been created by socket()
 * @return the index of the new socket; -1 on error
 */
static int alloc_socket(sal_netconn_t *newconn, int accepted)
{
    int i;
    SAL_ARCH_DECL_PROTECT(lev);

    /* allocate a new socket identifier */
    for (i = 0; i < NUM_SOCKETS; ++i) {
        /* Protect socket array */
        SAL_ARCH_PROTECT(lev);

        if (!sockets[i].conn) {
            sockets[i].conn       = newconn;
            /* The socket is not yet known to anyone, so no need to protect
               after having marked it as used. */
            SAL_ARCH_UNPROTECT(lev);
            sockets[i].lastdata   = NULL;
            sockets[i].lastoffset = 0;
            sockets[i].rcvevent   = 0;
            /* TCP sendbuf is empty, but the socket is not yet writable
               until connected (unless it has been created by accept()). */
            sockets[i].sendevent  = (NETCONNTYPE_GROUP(newconn->type) == \
                                     NETCONN_TCP ? (accepted != 0) : 1);
            sockets[i].errevent   = 0;
            sockets[i].err        = 0;
            sockets[i].select_waiting = 0;
            return i + SAL_SOCKET_OFFSET;
        }

        SAL_ARCH_UNPROTECT(lev);
    }

    return -1;
}

int sal_socket(int domain, int type, int protocol)
{
    sal_netconn_t *conn;
    int i;
    // #if !SAL_IPV6
    //   SAL_UNUSED_ARG(domain); /* @todo: check this */
    // #endif /* SAL_IPV6 */
    LOCK_SAL_CORE;

    /* create a netconn */
    switch (type) {
    case SOCK_RAW://暂不支持
        set_errno(EINVAL);
        UNLOCK_SAL_CORE;
        return -1;

    case SOCK_DGRAM:
#ifdef SAL_SERVER
        conn = salnetconn_new(NETCONN_UDP, false);
#else
        conn = salnetconn_new(NETCONN_UDP);
#endif
        break;

    case SOCK_STREAM:
#ifdef SAL_SERVER
        conn = salnetconn_new(NETCONN_TCP, false);
#else
        conn = salnetconn_new(NETCONN_TCP);
#endif
        break;

    default:
        set_errno(EINVAL);
        UNLOCK_SAL_CORE;
        return -1;
    }

    if (!conn) {
        UNLOCK_SAL_CORE;
        set_errno(ENOBUFS);
        return -1;
    }

    i = alloc_socket(conn, 0);

    if (i == -1) {
        salnetconn_delete(conn);
        set_errno(ENFILE);
        UNLOCK_SAL_CORE;
        return -1;
    }

    conn->socket = i;
    UNLOCK_SAL_CORE;
    SAL_DEBUG("sal_socket new fd %d\r\n", i);
    set_errno(0);
    return i;
}

#if SAL_PACKET_SEND_MODE_ASYNC

#define SAL_ASYNC_TASK_SLEEP_TIME_PER_SOCKET   5
static void sal_packet_output(void *arg)
{
    int fd = 0;
    sal_outputbuf_t *outputmem = NULL;
    struct sal_sock *pstsalsock = NULL;

    while (sal_send_async_running) {
        for (fd = 0; fd < MEMP_NUM_NETCONN; fd++) {
            pstsalsock = get_socket(fd);

            if (NULL == pstsalsock || NULL == pstsalsock->conn || !sal_mbox_valid(&pstsalsock->conn->sendmbox)) {
                aos_msleep(SAL_ASYNC_TASK_SLEEP_TIME_PER_SOCKET);
                continue;
            }

            /* TODO : here need to do some protection to against conn_drain */
            if (sal_arch_mbox_fetch(&pstsalsock->conn->sendmbox, (void **)&outputmem,
                                    SAL_ASYNC_TASK_SLEEP_TIME_PER_SOCKET) != SAL_ARCH_TIMEOUT) {
                if (outputmem == NULL) {
                    continue;
                }

                sal_deal_event(fd, NETCONN_EVT_SENDPLUS);

                /* sal module send need timeout to support send timeout */
                if (sal_module_send(fd, outputmem->payload, outputmem->len, NULL, -1, 0)) {
                    SAL_ERROR("socket %d fail to send packet, do nothing for now \r\n", fd);
                }

                free(outputmem->payload);
                free(outputmem);
                outputmem = NULL;
            }
        }
    }

    return ;
}
#endif

/* Call this during the init process. */
int sal_init(void)
{
    if (sal_init_done) {
        SAL_ERROR("sal have already init done\n");
        return 0;
    }

    SAL_DEBUG("Initializing SAL ...");
    sal_mutex_arch_init();

    if (sal_mutex_new(&lock_sal_core) != ERR_OK) {
        SAL_ERROR("failed to creat lock_sal_core \n");
        sal_mutex_arch_free();
        return -1;
    }

#if SAL_PACKET_SEND_MODE_ASYNC
    aos_task_t  task;

    sal_send_async_running = 1;
    if (aos_task_new_ext(&task, "sal_xmit", sal_packet_output, &sal_send_async_running, 2048, AOS_DEFAULT_APP_PRI - 4)) {
        sal_mutex_arch_free();
        sal_mutex_free(&lock_sal_core);
        SAL_ERROR("fail to creat sal xmit task \r\n");
        return -1;
    }

#endif

    if (sal_module_register_netconn_data_input_cb(&sal_packet_input) != ERR_OK) {
        SAL_ERROR("failed to reg sal packet input cb\n");
        sal_mutex_arch_free();
        sal_mutex_free(&lock_sal_core);
        return -1;
    }

    if (sal_module_register_netconn_close_cb(&sal_packet_close) != ERR_OK) {
        SAL_ERROR("failed to reg sal packet input cb\n");
        sal_mutex_arch_free();
        sal_mutex_free(&lock_sal_core);
        return -1;
    }

#ifdef SAL_SERVER

    if (sal_module_register_client_status_notify_cb(&sal_client_status_notify) != ERR_OK) {
        SAL_ERROR("failed to reg client status cb\n");
        sal_mutex_arch_free();
        sal_mutex_free(&lock_sal_core);
        return -1;
    }

#endif

    /* Low level init. */
    if (sal_module_init() != ERR_OK) {
        SAL_ERROR("sal low level init fail\n");
        sal_mutex_arch_free();
        sal_mutex_free(&lock_sal_core);
        return -1;
    }

    sal_init_done = 1;
    return 0 ;
}

int sal_deinit(void)
{
    if (!sal_init_done) {
        SAL_ERROR("sal have not inited\n");
        return 0;
    }

    SAL_DEBUG("Deinitializing SAL ...");
    sal_mutex_arch_free();

    sal_mutex_free(&lock_sal_core);

#if SAL_PACKET_SEND_MODE_ASYNC
///////
    sal_send_async_running = 0;
#endif

    /* Low level init. */
    sal_module_deinit();

    sal_init_done = 0;
    return 0 ;
}

int sal_listen(int s, int backlog)
{
#ifndef SAL_SERVER
    set_errno(ENOTSUP);
    return -1;
#else
    struct sal_sock *sock;
    err_t           err = ERR_OK;

    /*there is nothing to do at sal for now */
    SAL_DEBUG("sal_listen(%d, backlog=%d)\n", s, backlog);

    sock = get_socket(s);

    if (!sock) {
        return -1;
    }

    if (NULL == sock->conn) {
        SAL_ERROR("sal listen fail to get socket %d conn info\n.", s);
        sock_set_errno(sock, err_to_errno(ERR_ARG));
        return -1;
    }

    LOCK_SAL_CORE;
    err = salnetconn_listen(sock->conn);
    UNLOCK_SAL_CORE;

    if (err != ERR_OK) {
        SAL_ERROR("sal_listen %d failed, err=%d\n", s, err);
        sock_set_errno(sock, err_to_errno(err));
        return -1;
    }

    sock_set_errno(sock, 0);
    return 0;
#endif
}

int sal_bind(int s, const struct sockaddr *name, socklen_t namelen)
{
    struct sal_sock *sock = NULL;
    sal_netconn_t   *pstconn = NULL;
    ip_addr_t       local_addr;
    u16_t           local_port;
    err_t           err = ERR_OK;

    /*TODO check family and alignment of 'name' */
    if (NULL == name || !IS_SOCK_ADDR_LEN_VALID(namelen)) {
        SAL_ERROR("sal_bind invalid input arg\n");
        return -1;
    }

    sock = get_socket(s);

    if (NULL == sock) {
        SAL_ERROR("sal bind get socket failed.");
        return ERR_ARG;
    }

    pstconn = sock->conn;

    if (NULL == pstconn) {
        SAL_ERROR("sal bind fail to get socket %d conn info\n.", s);
        sock_set_errno(sock, err_to_errno(ERR_VAL));
        return ERR_ARG;
    }

    sockaddr_to_ipaddr_port(name, &local_addr, &local_port);
    LOCK_SAL_CORE;
    err = salnetconn_bind(pstconn, &local_addr, local_port);
    UNLOCK_SAL_CORE;

    if (ERR_OK != err) {
        SAL_ERROR("sal_bind %d failed, err=%d\n", s, err);
        sock_set_errno(sock, err_to_errno(err));
        return -1;
    }

    SAL_DEBUG("sal_bind(%d) succeeded\n", s);
    sock_set_errno(sock, 0);
    return 0;
}

int sal_connect(int s, const struct sockaddr *name, socklen_t namelen)
{
    struct sal_sock *sock = NULL;
    ip_addr_t remote_addr;
    u16_t     remote_port;
    int8_t    ip_str[SAL_SOCKET_IP4_ADDR_LEN] = {0};
    err_t err = ERR_OK;


    sock = get_socket(s);

    if (!sock) {
        SAL_ERROR("get_socket failed.");
        return -1;
    }

    if (!sock->conn) {
        SAL_ERROR("fail to get socket %d conn info\n.", s);
        sock_set_errno(sock, err_to_errno(ERR_VAL));
        return -1;
    }

    /* Only IPv4 is supported. */
    if (name->sa_family != AF_INET) {
        SAL_ERROR("Not supported (only IPv4 for now)!");
        sock_set_errno(sock, err_to_errno(ERR_VAL));
        return -1;
    }

    /* Check size */
    if (namelen != sizeof(struct sockaddr_in)) {
        SAL_ERROR("sal_connect: invalid address");
        sock_set_errno(sock, err_to_errno(ERR_ARG));
        return -1;
    }

    sockaddr_to_ipaddr_port(name, &remote_addr, &remote_port);
    LOGD(SAL_TAG, "remote_port -- : %d", remote_port);
    ip4_sockaddr_to_ipstr_port(name, (char *)ip_str);
    LOCK_SAL_CORE;
    err = salnetconn_connect(sock->conn, ip_str, remote_port);
    UNLOCK_SAL_CORE;

    if (ERR_OK != err) {
        SAL_ERROR("sal_connect failed, err=%d\n", err);
        sock_set_errno(sock, err_to_errno(err));
        return -1;
    }

    SAL_DEBUG("sal_connect(%d) succeeded\n", s);
    sock_set_errno(sock, 0);
    return err;
}

/**
 * Free a socket. The socket's netconn must have been
 * delete before!
 *
 * @param sock the socket to free
 * @param is_tcp != 0 for TCP sockets, used to free lastdata
 */
static void free_socket(struct sal_sock *sock)
{
    sock->lastdata   = NULL;
    sock->lastoffset = 0;
    sock->err        = 0;

    /* Protect socket array */
    SAL_ARCH_SET(sock->conn, NULL);
    /* don't use 'sock' after this line, as another task
       might have allocated it */
}

int sal_close(int s)
{
    struct sal_sock *sock;
    struct sal_event *event;
    err_t err;

    SAL_DEBUG("sal_close(%d)\r\n", s);
    event = tryget_event(s);
    if (event) {
        event->used = 0;
        return 0;
    }

    sock = get_socket(s);
    if (!sock) {
        return -1;
    }

    if (!sock->conn) {
        return -1;
    }

#if SAL_PACKET_SEND_MODE_ASYNC
    int wait_send_timeout = 0;

    if (sal_mbox_valid(&sock->conn->sendmbox)) {
        while (wait_send_timeout < SAL_DRAIN_SENDMBOX_WAIT_TIME) {
            if (sock->sendevent == 0) {
                break;
            }

            aos_msleep(10);
            wait_send_timeout++;
        }
    }

#endif
    //del state check, if socket create, must close module.
    //if ((sock->conn->state == NETCONN_CONNECT) || (sock->conn->state == NETCONN_CLOSE)) {
        LOCK_SAL_CORE;
        if (sal_module_close(s, -1) != 0) {
            SAL_DEBUG("sal_module_close failed.");
        }
        UNLOCK_SAL_CORE;
    //}

    sal_deal_event(s, NETCONN_EVT_ERROR);
    LOCK_SAL_CORE;
    err = salnetconn_delete(sock->conn);
    UNLOCK_SAL_CORE;

    if (err != ERR_OK) {
        SAL_ERROR("salnetconn_delete failed in %s.", __func__);
        sock_set_errno(sock, err_to_errno(err));
        return -1;
    }

    free_socket(sock);
    set_errno(0);
    return 0;
}

struct hostent *sal_gethostbyname(const char *name)
{
    ip_addr_t addr;
    char ip_str[16] = {0};

    static struct hostent s_hostent;
    static char *s_aliases;
    static ip_addr_t s_hostent_addr;
    static ip_addr_t *s_phostent_addr[2];
    static char s_hostname[DNS_MAX_NAME_LENGTH + 1];

    if (!name) {
        SAL_ERROR("%s failed, invalid argument.", __func__);
        return NULL;
    }

    LOCK_SAL_CORE;
    if (sal_module_domain_to_ip((char *)name, ip_str) != 0) {
        SAL_ERROR("domain to ip failed.");
        UNLOCK_SAL_CORE;
        return NULL;
    }
    UNLOCK_SAL_CORE;

#if LWIP_IPV4 && LWIP_IPV6
    addr.type = IPADDR_TYPE_V4;
#endif

    if (ip4addr_aton(ip_str, (ip4_addr_t *)ip_2_ip4(&addr)) == 0) {
        SAL_ERROR("ip_2_u32 failed");
        return NULL;
    }

    /* fill hostent */
    s_hostent_addr = addr;
    s_phostent_addr[0] = &s_hostent_addr;
    s_phostent_addr[1] = NULL;
    strncpy(s_hostname, name, DNS_MAX_NAME_LENGTH);
    s_hostname[DNS_MAX_NAME_LENGTH] = 0;
    s_hostent.h_name = s_hostname;
    s_aliases = NULL;
    s_hostent.h_aliases = &s_aliases;
    s_hostent.h_addrtype = AF_INET;
    s_hostent.h_length = sizeof(ip_addr_t);
    s_hostent.h_addr_list = (char **)&s_phostent_addr;

    /* not thread safe, <TODO> */
    return &s_hostent;
}

int sal_getsockopt(int s, int level, int optname,
                   void *optval, socklen_t *optlen)
{
    u8_t err = 0;
    struct sal_sock *sock = get_socket(s);

    if (!sock) {
        return -1;
    }

    if ((NULL == optval) || (NULL == optlen)) {
        sock_set_errno(sock, EFAULT);
        return -1;
    }

    /* Only support SOL_SOCKET/SO_ERROR for now. */
    switch (level) {
    case SOL_SOCKET:
        switch (optname) {
        case SO_ERROR:
            if (*optlen < sizeof(int)) {
                return EINVAL;
            }

            /* only overwrite ERR_OK or temporary errors */
            if (((sock->err == 0) || (sock->err == EINPROGRESS)) &&
                (sock->conn != NULL)) {
                sock_set_errno(sock, err_to_errno(sock->conn->pending_err));
            }

            *(int *)optval = (sock->err == 0xFF ? (int) - 1 : (int)sock->err);
            sock->err = 0;
            SAL_DEBUG("sal_getsockopt(%d, SOL_SOCKET, SO_ERROR) = %d\n",
                      s, *(int *)optval);
            break;

        default:
            SAL_DEBUG("sal_getsockopt(%d, SOL_SOCKET, UNIMPL: "
                      "optname=0x%x, ..)\n", s, optname);
            err = ENOPROTOOPT;
            break;
        }

        break;

    default:
        SAL_DEBUG("sal_getsockopt(%d, level=0x%x, UNIMPL: optname=0x%x, ..)\n",
                  s, level, optname);
        err = ENOPROTOOPT;
        break;
    }

    sock_set_errno(sock, err);
    return err ? -1 : 0;
}

int sal_setsockopt(int s, int level, int optname,
                   const void *optval, socklen_t optlen)
{
    u8_t err = 0;
    struct sal_sock *sock = get_socket(s);

    if (!sock) {
        return -1;
    }

    if (NULL == optval) {
        sock_set_errno(sock, EFAULT);
        return -1;
    }

    switch (level) {
    case SOL_SOCKET:
        switch (optname) {
        case SO_RCVTIMEO:
            sock->conn->recv_timeout = SAL_SO_SNDRCVTIMEO_GET_MS(optval);
            break;

        case SO_SNDTIMEO:
            sock->conn->send_timeout = SAL_SO_SNDRCVTIMEO_GET_MS(optval);
            break;

        case SO_REUSEADDR:
            break;

        default:
            SAL_DEBUG("sal_setsockopt(%d, SOL_SOCKET:, UNIMPL: "
                      "optname=0x%x, ..)\n", s, optname);
            err = ENOPROTOOPT;
            break;
        }

        break;

    case IPPROTO_IP:
        switch (optname) {
        case IP_MULTICAST_IF:
            break;

        case IP_MULTICAST_LOOP:
            break;

        case IP_ADD_MEMBERSHIP:
            break;

        default:
            SAL_DEBUG("sal_setsockopt(%d, SOL_SOCKET:, UNIMPL: "
                      "optname=0x%x, ..)\n", s, optname);
            err = ENOPROTOOPT;
            break;
        }

        break;

    default:
        SAL_DEBUG("sal_setsockopt(%d, level=0x%x, UNIMPL: optname=0x%x, ..)\n",
                  s, level, optname);
        err = ENOPROTOOPT;
        break;
    }

    sock_set_errno(sock, err);
    return err ? -1 : 0;
}

/** A minimal implementation of fcntl.
 * Currently only the commands F_GETFL and F_SETFL are implemented.
 * Only the flag O_NONBLOCK is implemented.
 */
int sal_fcntl(int s, int cmd, ...)
{
    va_list ap;

    struct sal_sock *sock = get_socket(s);
    int ret = -1;

    if (!sock) {
        return -1;
    }

    va_start(ap, cmd);
    int val = va_arg(ap, int);

    switch (cmd) {
    case F_GETFL:
        ret = netconn_is_nonblocking(sock->conn) ? O_NONBLOCK : 0;
        sock_set_errno(sock, 0);
        break;

    case F_SETFL:
        if ((val & ~O_NONBLOCK) == 0) {
            /* only O_NONBLOCK, all other bits are zero */
            netconn_set_nonblocking(sock->conn, val & O_NONBLOCK);
            ret = 0;
            sock_set_errno(sock, 0);
        } else {
            sock_set_errno(sock, ENOSYS); /* not yet implemented */
        }

        break;

    default:
        SAL_DEBUG("sal_fcntl(%d, UNIMPL: %d, %d)\n", s, cmd, val);
        sock_set_errno(sock, ENOSYS); /* not yet implemented */
        break;
    }

    va_end(ap);

    return ret;
}

int sal_shutdown(int s, int how)
{
    SAL_DEBUG("%s call stub for now\n", __func__);
    return 0;
}

int sal_getaddrinfo(const char *nodename, const char *servname,
                    const struct addrinfo *hints, struct addrinfo **res)
{
    //err_t err;
    ip_addr_t addr;
    struct addrinfo *ai;
    struct sockaddr_storage *sa = NULL;
    int port_nr = 0;
    size_t total_size;
    size_t namelen = 0;
    int ai_family;

    if (res == NULL) {
        return EAI_FAIL;
    }

    *res = NULL;

    if ((nodename == NULL) && (servname == NULL)) {
        return EAI_NONAME;
    }

    if (hints != NULL) {
        ai_family = hints->ai_family;

        if ((ai_family != AF_UNSPEC)
            && (ai_family != AF_INET)
           ) {
            return EAI_FAMILY;
        }
    } else {
        ai_family = AF_UNSPEC;
    }

    if (servname != NULL) {
        /* service name specified: convert to port number
         * @todo?: currently, only ASCII integers (port numbers) are supported (AI_NUMERICSERV)! */
        port_nr = atoi(servname);

        if ((port_nr <= 0) || (port_nr > 0xffff)) {
            return EAI_SERVICE;
        }
    }

    if (nodename != NULL) {
        /* service location specified, try to resolve */
        if ((hints != NULL) && (hints->ai_flags & AI_NUMERICHOST)) {
            /* no DNS lookup, just parse for an address string */
            if (!ipaddr_aton(nodename, (ip4_addr_t *)&addr)) {
                return EAI_NONAME;
            }

            if ((IP_IS_V6_VAL(addr) && ai_family == AF_INET) ||
                (IP_IS_V4_VAL(addr) && ai_family == AF_INET6)) {
                return EAI_NONAME;
            }
        } else {
            //ip_addr_t addr;
            char ip_str[16] = {0};

            LOCK_SAL_CORE;
            if (sal_module_domain_to_ip((char *)nodename, ip_str) != 0) {
                SAL_ERROR("domain to ip failed.");
                UNLOCK_SAL_CORE;
                return EAI_FAIL;
            }
            UNLOCK_SAL_CORE;
            // Currently only v4 is supported by AT firmware
#if LWIP_IPV4 && LWIP_IPV6
            addr.type = IPADDR_TYPE_V4;
#endif

            if (ip4addr_aton(ip_str, (ip4_addr_t *)ip_2_ip4(&addr)) == 0) {
                SAL_ERROR("ip_2_u32 failed");
                return EAI_FAIL;
            }
        }
    } else {
        /* to do service location specified, use loopback address */
    }

    total_size = sizeof(struct addrinfo) + sizeof(struct sockaddr_storage);

    if (nodename != NULL) {
        namelen = strlen(nodename);

        if (namelen > DNS_MAX_NAME_LENGTH) {
            /* invalid name length */
            return EAI_FAIL;
        }

        SAL_ASSERT("namelen is too long", total_size + namelen + 1 > total_size);
        total_size += namelen + 1;
    }

    /* If this fails, please report to lwip-devel! :-) */
    SAL_ASSERT("total_size <= NETDB_ELEM_SIZE: please report this!",
               total_size <= NETDB_ELEM_SIZE);
    ai = (struct addrinfo *)aos_zalloc(total_size);

    if (ai == NULL) {
        return EAI_MEMORY;
    }

    /* cast through void* to get rid of alignment warnings */
    sa = (struct sockaddr_storage *)(void *)((u8_t *)ai + sizeof(struct addrinfo));
    struct sockaddr_in *sa4 = (struct sockaddr_in *)sa;
    /* set up sockaddr */
    inet_addr_from_ip4addr(&sa4->sin_addr, ip_2_ip4(&addr));
    sa4->sin_family = AF_INET;
    sa4->sin_len = sizeof(struct sockaddr_in);
    sa4->sin_port = sal_htons((u16_t)port_nr);
    ai->ai_family = AF_INET;

    /* set up addrinfo */
    if (hints != NULL) {
        /* copy socktype & protocol from hints if specified */
        ai->ai_socktype = hints->ai_socktype;
        ai->ai_protocol = hints->ai_protocol;
    }

    if (nodename != NULL) {
        /* copy nodename to canonname if specified */
        ai->ai_canonname = ((char *)ai + sizeof(struct addrinfo) + \
                            sizeof(struct sockaddr_storage));
        memcpy(ai->ai_canonname, nodename, namelen);
        ai->ai_canonname[namelen] = 0;
    }

    ai->ai_addrlen = sizeof(struct sockaddr_storage);
    ai->ai_addr = (struct sockaddr *)sa;

    *res = ai;

    return 0;
}

void sal_freeaddrinfo(struct addrinfo *ai)
{
    if (ai != NULL) {
        free(ai);
    }
}
#endif

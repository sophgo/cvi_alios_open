/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <lwip/netdb.h>
#include <unistd.h>
#include <iot_import.h>
// #include "imports/iot_import_coap.h"
#include <ulog/ulog.h>

#define TAG "IOT_HAL"

#define TRANSPORT_ADDR_LEN 16

#define NETWORK_ADDR_LEN (16)

intptr_t HAL_UDP_create_without_connect(const char *host, unsigned short port)
{
    int                flag      = 1;
    int                ret       = -1;
    int                socket_id = -1;
    struct sockaddr_in local_addr; /*local addr*/

    if ((socket_id = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        LOGE(TAG, "socket create failed");
        return (intptr_t)-1;
    }

    ret = setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    if (ret < 0) {
        close(socket_id);
        LOGE(TAG, "setsockopt SO_REUSEADDR failed");
        return (intptr_t)-1;
    }

    flag = 1;
#ifdef IP_RECVPKTINFO
    if ((ret = setsockopt(socket_id, IPPROTO_IP, IP_RECVPKTINFO, &flag,
                          sizeof(flag))) < 0)
#else  /* IP_RECVPKTINFO */
    if ((ret = setsockopt(socket_id, IPPROTO_IP, IP_MULTICAST_IF, &flag,
                          sizeof(flag))) < 0)
#endif /* IP_RECVPKTINFO */
        if (ret < 0) {
            close(socket_id);
            LOGE(TAG, "setsockopt IP_MULTICAST_IF failed");
            return (intptr_t)-1;
        }


    memset(&local_addr, 0x00, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    if (NULL != host) {
        inet_aton(host, &local_addr.sin_addr);
    } else {
        local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    local_addr.sin_port = htons(port);
    ret = bind(socket_id, (struct sockaddr *)&local_addr, sizeof(local_addr));

    // fprintf(stderr,"\r\n[%s LINE #%d]  Create socket port %d fd %d ret
    // %d\r\n",
    //                    __FILE__, __LINE__, port, socket_id, ret);
    return (intptr_t)socket_id;
}

int HAL_UDP_close_without_connect(_IN_ intptr_t sockfd)
{
    return close((int)sockfd);
}

intptr_t HAL_UDP_create(_IN_ char *host, _IN_ unsigned short port)
{
    int                     rc = -1;
    long                    socket_id = -1;
    char                    port_ptr[6] = {0};
    struct addrinfo         hints;
    char                    addr[NETWORK_ADDR_LEN] = {0};
    struct addrinfo        *res, *ainfo;
    struct sockaddr_in     *sa = NULL;

    if (NULL == host) {
        return (-1);
    }

    sprintf(port_ptr, "%u", port);
    memset((char *)&hints, 0x00, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_INET;
    hints.ai_protocol = IPPROTO_UDP;

    rc = getaddrinfo(host, port_ptr, &hints, &res);
    if (0 != rc) {
        LOGE(TAG, "getaddrinfo error: %d", rc);
        return (-1);
    }

    for (ainfo = res; ainfo != NULL; ainfo = ainfo->ai_next) {
        if (AF_INET == ainfo->ai_family) {
            sa = (struct sockaddr_in *)ainfo->ai_addr;
            inet_ntop(AF_INET, &sa->sin_addr, addr, NETWORK_ADDR_LEN);

            socket_id = socket(ainfo->ai_family, ainfo->ai_socktype, ainfo->ai_protocol);
            if (socket_id < 0) {
                LOGE(TAG, "create socket error");
                continue;
            }
            if (0 == connect(socket_id, ainfo->ai_addr, ainfo->ai_addrlen)) {
                break;
            }

            close(socket_id);
        }
    }
    freeaddrinfo(res);

    return socket_id;
}

 void HAL_UDP_close(_IN_ intptr_t p_socket)
{
    long socket_id = -1;

    socket_id = (long)p_socket;
    close(socket_id);
}

int HAL_UDP_write(intptr_t p_socket, const unsigned char *p_data,
                  unsigned int datalen)
{
    int  rc        = -1;
    long socket_id = -1;

    socket_id = (long)p_socket;
    rc        = send(socket_id, (char *)p_data, (int)datalen, 0);
    if (-1 == rc) {
        return -1;
    }

    return rc;
}


int HAL_UDP_read(_IN_ intptr_t       p_socket,
                 _OU_ unsigned char *p_data,
                 _OU_ unsigned int   datalen)
{
    long socket_id = -1;
    int count = -1;

    if(NULL == p_data || 0 == p_socket){
        return -1;
    }

    socket_id = (long)p_socket;
    count = (int)recv(socket_id, p_data, datalen, 0);

    return count;
}



int HAL_UDP_readTimeout(intptr_t p_socket, unsigned char *p_data,
                        unsigned int datalen, unsigned int timeout)
{
    int            ret;
    struct timeval tv;
    fd_set         read_fds;
    long           socket_id = -1;

    if (0 == p_socket || NULL == p_data) {
        return -1;
    }
    socket_id = (long)p_socket;

    if (socket_id < 0) {
        return -1;
    }

    FD_ZERO(&read_fds);
    FD_SET(socket_id, &read_fds);

    tv.tv_sec  = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    ret =
      select(socket_id + 1, &read_fds, NULL, NULL, timeout == 0 ? NULL : &tv);

    /* Zero fds ready means we timed out */
    if (ret == 0) {
        return -2; /* receive timeout */
    }

    if (ret < 0) {
        if (errno == EINTR) {
            return -3; /* want read */
        }

        return -4; /* receive failed */
    }

    /* This call will not block */
    return  read((long)p_socket, p_data, datalen);

}


int HAL_UDP_resolveAddress(const char *p_host,  char addr[NETWORK_ADDR_LEN])
{
    struct addrinfo *res, *ainfo;
    struct addrinfo hints;
    int error = -1;
    struct sockaddr_in *sa = NULL;

    memset ((char *)&hints, 0x00, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_INET;
    hints.ai_protocol = IPPROTO_UDP;

    error = getaddrinfo(p_host, NULL, &hints, &res);

    if (error != 0)
    {
        fprintf(stderr, "getaddrinfo error\n");
        return error;
    }

    for (ainfo = res; ainfo != NULL; ainfo = ainfo->ai_next)
    {
        switch (ainfo->ai_family)
        {
            case AF_INET:
                sa = (struct sockaddr_in *)ainfo->ai_addr;
                inet_ntop(AF_INET, &sa->sin_addr, addr, NETWORK_ADDR_LEN);
                break;
            default:
                ;
        }
    }

    freeaddrinfo(res);
    return 0;
}
int HAL_UDP_connect(_IN_ intptr_t sockfd,
                    _IN_ const char *host,
                    _IN_ unsigned short port)
{
    int                     rc = -1;
    char                    port_ptr[6] = {0};
    struct addrinfo         hints;
    struct addrinfo        *res, *ainfo;

    if (NULL == host) {
        return -1;
    }

    LOGI(TAG, "HAL_UDP_connect, host=%s, port=%d", host, port);
    sprintf(port_ptr, "%u", port);
    memset((char *)&hints, 0x00, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_INET;
    hints.ai_protocol = IPPROTO_UDP;

    rc = getaddrinfo(host, port_ptr, &hints, &res);
    if (0 != rc) {
        LOGE(TAG, "getaddrinfo error");
        return -1;
    }

    for (ainfo = res; ainfo != NULL; ainfo = ainfo->ai_next) {
        if (AF_INET == ainfo->ai_family) {
            if (0 == connect(sockfd, ainfo->ai_addr, ainfo->ai_addrlen)) {
                freeaddrinfo(res);
                return 0;
            }
        }
    }
    freeaddrinfo(res);

    return -1;
}

int HAL_UDP_recvfrom(intptr_t sockfd, NetworkAddr *p_remote,
                     unsigned char *p_data, unsigned int datalen,
                     unsigned int timeout_ms)
{
    int             socket_id = -1;
    struct sockaddr from;
    int             count = -1, ret = -1;
    socklen_t       addrlen = 0;
    struct timeval  tv;
    fd_set          read_fds;

    if (NULL == p_remote || NULL == p_data) {
        return -1;
    }

    socket_id = (int)sockfd;

    FD_ZERO(&read_fds);
    FD_SET(socket_id, &read_fds);

    tv.tv_sec  = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    ret = select(socket_id + 1, &read_fds, NULL, NULL,
                 timeout_ms == 0 ? NULL : &tv);

    /* Zero fds ready means we timed out */
    if (ret == 0) {
        return -2; /* receive timeout */
    }

    if (ret < 0) {
        if (errno == EINTR) {
            return -3; /* want read */
        }

        return -4; /* receive failed */
    }

    addrlen = sizeof(struct sockaddr);
    count   = recvfrom(socket_id, p_data, (size_t)datalen, 0, &from, &addrlen);
    if (-1 == count) {
        return -1;
    }
    if (from.sa_family == AF_INET) {
        struct sockaddr_in *sin = (struct sockaddr_in *)&from;
        inet_ntop(AF_INET, &sin->sin_addr, (char *)p_remote->addr,
                  NETWORK_ADDR_LEN);
        p_remote->port = ntohs(sin->sin_port);
    }
    return count;
}


int HAL_UDP_sendto(intptr_t sockfd, const NetworkAddr *p_remote,
                   const unsigned char *p_data, unsigned int datalen,
                   unsigned int timeout_ms)
{
    int                rc        = -1;
    int                socket_id = -1;
    struct sockaddr_in remote_addr;

    if (NULL == p_remote || NULL == p_data) {
        return -1;
    }

    socket_id              = (int)sockfd;
    remote_addr.sin_family = AF_INET;
    if (1 !=
        (rc = inet_pton(remote_addr.sin_family, (const char *)p_remote->addr,
                        &remote_addr.sin_addr.s_addr))) {
        return -1;
    }
    remote_addr.sin_port = htons(p_remote->port);
    rc                   = sendto(socket_id, p_data, (size_t)datalen, 0,
                (const struct sockaddr *)&remote_addr, sizeof(remote_addr));
    if (-1 == rc) {
        return -1;
    }
    return rc;
}


int HAL_UDP_joinmulticast(intptr_t sockfd, char *p_group)
{
    int err       = -1;
    int socket_id = -1;

    if (NULL == p_group) {
        return -1;
    }

    /*set loopback*/
    int loop  = 1;
    socket_id = (int)sockfd;
    err =
      setsockopt(socket_id, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
    if (err < 0) {
        printf("setsockopt():IP_MULTICAST_LOOP failed\r\n");
        return err;
    }

    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(p_group);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY); /*default networt interface*/

    /*join to the mutilcast group*/
    err =
      setsockopt(socket_id, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
    if (err < 0) {
        printf("setsockopt():IP_ADD_MEMBERSHIP failed\r\n");
        return err;
    }

    return 0;
}



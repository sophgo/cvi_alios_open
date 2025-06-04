/*
 * Copyright (C) 2021 Alibaba Group Holding Limited
 */

#ifndef _POLL_H
#define _POLL_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef POLLIN
#define POLLIN     0x001
#endif
#ifndef POLLOUT
#define POLLOUT    0x004
#endif
#ifndef POLLERR
#define POLLERR    0x008
#endif

typedef unsigned int nfds_t;

struct pollfd {
    int fd;        /* file descriptor */
    short events;  /* requested events */
    short revents; /* returned events */
};

int poll(struct pollfd fds[], nfds_t nfds, int timeout);

#ifdef __cplusplus
}
#endif

#endif /*_POLL_H*/

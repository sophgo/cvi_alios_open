/*
 * Copyright (C) 2015-2021 Alibaba Group Holding Limited
 */

#ifndef _MQUEUE_H
#define _MQUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>
#include <time.h>
#include <aos/kernel.h>

#define DEFAULT_MQUEUE_SIZE  10240
#define DEFAULT_MAX_MSG_SIZE 1024

typedef void *mqd_t;

struct mq_attr {
    long mq_flags;    /* message queue flags */
    long mq_maxmsg;   /* maximum number of messages */
    long mq_msgsize;  /* maximum message size */
    long mq_curmsgs;  /* number of messages currently queued */
};

mqd_t   mq_open(const char *name, int oflag, ...);
int     mq_close(mqd_t mqdes);
ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned *msg_prio);
int     mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned msg_prio);
int     mq_setattr(mqd_t mqdes, const struct mq_attr *mqstat, struct mq_attr *omqstat);
int     mq_getattr(mqd_t mqdes, struct mq_attr *mqstat);
ssize_t mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned *msg_prio, const struct timespec *abs_timeout);
int     mq_timedsend(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned msg_prio, const struct timespec *abs_timeout);
int     mq_unlink(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* _MQUEUE_H */

/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */

#include <stdint.h>
#include "aos/kernel.h"

void *hcc_malloc(uint32_t size)
{
    return aos_malloc(size);
}

void hcc_free(void *ptr)
{
    aos_free(ptr);
}

void *hcc_mutex_new(void)
{
    aos_mutex_t mutex;
    if (0 != aos_mutex_new(&mutex)) {
        return NULL;
    }

    return mutex;
}

void hcc_mutex_free(void *mutex)
{
    if (NULL != mutex) {
        aos_mutex_free((aos_mutex_t *)&mutex);
    }
}

void hcc_mutex_lock(void *mutex)
{
    if (NULL != mutex) {
        aos_mutex_lock((aos_mutex_t *)&mutex, AOS_WAIT_FOREVER);
    }
}

void hcc_mutex_unlock(void *mutex)
{
    if (NULL != mutex) {
        aos_mutex_unlock((aos_mutex_t *)&mutex);
    }
}

void *hcc_sem_new(void)
{
    aos_sem_t sem;

    if (0 != aos_sem_new(&sem, 0)) {
        return NULL;
    }

    return sem;
}

void hcc_sem_free(void *sem)
{
    aos_sem_free((aos_sem_t *)&sem);
}

void hcc_sem_signal(void *sem)
{
    aos_sem_signal((aos_sem_t *)&sem);
}

int hcc_sem_wait(void *sem, uint32_t timeout_ms)
{
    return aos_sem_wait((aos_sem_t *)&sem, timeout_ms);
}

int hcc_task_new_ext(void *task, char *name, void (*fn)(void *),
                       void *arg, int stack_size, int prio)
{
    if (task == NULL)
        return -1;

    return aos_task_new_ext((aos_task_t *)&task, name, fn, arg, stack_size,
                            prio);
}

void hcc_sleep_ms(const unsigned int millisec)
{
    aos_msleep(millisec);
}


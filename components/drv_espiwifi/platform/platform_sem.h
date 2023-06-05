#ifndef __PLATFORM_SEM_H__
#define __PLATFORM_SEM_H__

#include <aos/kernel.h>
#include <stddef.h>

typedef struct {
    aos_sem_t sem;
} platform_sem_t;

platform_sem_t *platform_sem_create(size_t maxcount, size_t initalcount);
void platform_sem_post(platform_sem_t *sem);
int platform_sem_wait(platform_sem_t *sem, size_t timeout_ms);
void platform_sem_destroy(platform_sem_t *sem);

#endif

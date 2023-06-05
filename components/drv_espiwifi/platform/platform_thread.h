#ifndef __PLATFORM_THREAD_H__
#define __PLATFORM_THREAD_H__

#include <aos/kernel.h>

#define PLATFORM_DEFAULT_NAME_LEN 32
#define PLATFORM_DEFAULT_STACK_LEN 4096

typedef enum {
    PLATFORM_THREAD_INIT,
    PLATFORM_THREAD_RUNNING,
    PLATFORM_THREAD_STOP
} platform_thread_state_e;

typedef struct {
    char name[PLATFORM_DEFAULT_NAME_LEN];
    aos_task_t handle;
    unsigned int state;
} platform_thread_t;

platform_thread_t *platform_thread_init(const char *name, unsigned int pir,
    unsigned int stack_size, void (*entry)(void *), void *args);
void platform_thread_stop(platform_thread_t *thread);
void platform_thread_start(platform_thread_t *thread);
void platform_thread_kill(platform_thread_t *thread);
void platform_thread_destory(platform_thread_t *thread);

#endif

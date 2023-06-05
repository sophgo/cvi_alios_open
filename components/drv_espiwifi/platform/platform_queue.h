#ifndef __PLATFORM_QUEUE_H__
#define __PLATFORM_QUEUE_H__

#include <stddef.h>
#include <aos/kernel.h>

typedef struct {
      aos_queue_t handle;
} platform_queue_t;

platform_queue_t *platform_queue_create(const char *name, size_t num, size_t size, size_t dummy);
int platform_queue_send(platform_queue_t *queue_handle, char *msg, size_t msglen, size_t timeout_ms);
int platform_queue_receive(platform_queue_t *queue_handle, char *msg, size_t msglen, size_t timeout_ms);
void platform_queue_destory(platform_queue_t *queue_handle);

#endif

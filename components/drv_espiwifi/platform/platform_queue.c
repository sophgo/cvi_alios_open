#include "platform_os.h"
#include <string.h>

platform_queue_t *platform_queue_create(const char *name, size_t num, size_t size, size_t dummy)
{
    void *msgbuff = NULL;
    platform_queue_t *queue = platform_memory_alloc(sizeof(platform_queue_t));
    if (queue == NULL) {
        platform_log_e("%s[%d] alloc queue failed\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    msgbuff = (void *)platform_memory_alloc(num * size);
    if (msgbuff == NULL) {
        platform_log_e("%s[%d] alloc queue buff failed\n", __FUNCTION__, __LINE__);
        platform_memory_free(queue);
        return NULL;
    }

    memset(queue, 0, sizeof(platform_queue_t));
    if (aos_queue_new(&queue->handle, msgbuff, num * size, size) != 0) {
        platform_log_e("%s[%d] create queue failed\n", __FUNCTION__, __LINE__);
        platform_memory_free(queue);
        platform_memory_free(msgbuff);
        return NULL;
    }

    return queue;
}

int platform_queue_send(platform_queue_t *queue, char *msg, size_t msglen, size_t timeout_ms)
{
    if (queue == NULL || msg == NULL || msglen == 0) {
        platform_log_e("%s[%d] invalid input params\n", __FUNCTION__, __LINE__);
        return -1;
    }

    int ret = aos_queue_send(&queue->handle, (void*)msg, msglen);
    if (ret != 0) {
        // platform_log_e("%s[%d] queue send error\n", __FUNCTION__, __LINE__);
        return -1;
    }

    return 0;
}

int platform_queue_receive(platform_queue_t *queue, char *msg, size_t msglen, size_t timeout_ms)
{
    int ret;

    if (queue == NULL || msg == NULL) {
        platform_log_e("%s[%d] invalid input params\n", __FUNCTION__, __LINE__);
        return -1;
    }

    ret = aos_queue_recv(&queue->handle, timeout_ms, msg, &msglen);
    if (ret != 0) {
        platform_log_e("%s[%d] queue recv error\n", __FUNCTION__, __LINE__);
        return -1;
    }

    return 0;
}

void platform_queue_destory(platform_queue_t *queue)
{
    aos_queue_free(&queue->handle);
    platform_memory_free(queue);
}

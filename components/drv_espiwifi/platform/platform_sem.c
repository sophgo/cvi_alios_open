#include "platform_os.h"

platform_sem_t *platform_sem_create(size_t maxcount, size_t initalcount)
{
    platform_sem_t *sem = platform_memory_alloc(sizeof(platform_sem_t));
    int ret;

    if (sem == NULL) {
        platform_log_e("%s[%d] alloc sem failed\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    ret = aos_sem_new(&(sem->sem), initalcount);
    if (ret != 0) {
        platform_log_e("%s[%d] create sem failed\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    return sem;
}

void platform_sem_post(platform_sem_t *sem)
{
    aos_sem_signal(&(sem->sem));
}

int platform_sem_wait(platform_sem_t *sem, size_t timeout_ms)
{
    return aos_sem_wait(&(sem->sem), timeout_ms);
}

void platform_sem_destroy(platform_sem_t *sem)
{
    aos_sem_free(&(sem->sem));
    platform_memory_free(sem);
}

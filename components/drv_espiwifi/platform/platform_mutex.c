#include "platform_mutex.h"

void platform_mutex_init(platform_mutex_t *m)
{
    aos_mutex_new(&(m->mutex));
}

int platform_mutex_lock(platform_mutex_t *m)
{
    return aos_mutex_lock(&(m->mutex), AOS_WAIT_FOREVER);
}

int platform_mutex_trylock(platform_mutex_t *m)
{
    return aos_mutex_lock(&(m->mutex), AOS_WAIT_FOREVER);
}

void platform_mutex_unlock(platform_mutex_t *m)
{
    aos_mutex_unlock(&(m->mutex));
}

void platform_mutex_destroy(platform_mutex_t *m)
{
    aos_mutex_free(&(m->mutex));
}

#include "platform_memory.h"
#include <stdlib.h>
#include <string.h>

void platform_memory_set(void *ptr, int c, size_t size)
{
    memset(ptr, c, size);
}

void *platform_memory_alloc(size_t size)
{
    return malloc(size);
}

void *platform_memory_calloc(size_t num, size_t size)
{
    return calloc(num, size);
}

void *platform_memory_realloc(void *ptr, size_t size)
{
    if (ptr == NULL) {
        return malloc(size);
    } else {
        return realloc(ptr, size);
    }
}

void platform_memory_free(void *ptr)
{
    free(ptr);
}

void platform_memory_copy(void *dst, void *src, size_t len)
{
    memcpy(dst, src, len);
}


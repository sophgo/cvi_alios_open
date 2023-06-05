#ifndef __PLATFORM_MEMORY_H__
#define __PLATFORM_MEMORY_H__

#include <stddef.h>

void platform_memory_set(void *ptr, int c, size_t size);
void *platform_memory_alloc(size_t size);
void *platform_memory_calloc(size_t num, size_t size);
void *platform_memory_realloc(void *ptr, size_t size);
void platform_memory_free(void *ptr);
void platform_memory_copy(void *dst, void *src, size_t len);

#endif

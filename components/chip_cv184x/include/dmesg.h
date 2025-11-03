#ifndef __DMESG_H__
#define __DMESG_H__

#include <stdarg.h>

void dmesg_init(void);
void dmesg_deinit(void);
int dmesg_printk(const char *fmt, ...);
int vprintk_func(const char *fmt, va_list args);

#endif

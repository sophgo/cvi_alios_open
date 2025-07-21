#ifndef __DMESG_H__
#define __DMESG_H__



void dmesg_init(void);
void dmesg_deinit(void);
int dmesg_printk(const char *fmt, ...);

#endif

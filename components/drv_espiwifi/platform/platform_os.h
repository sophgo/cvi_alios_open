#ifndef __PLATFORM_OS_H__
#define __PLATFORM_OS_H__

#include "platform_sem.h"
#include "platform_thread.h"
//#include "platform_timer.h"
//#include "platform_mutex.h"
//#include "platform_list.h"
#include "platform_memory.h"
#include "platform_queue.h"
#include "stdio.h"

#define platform_log_e printf
#define platform_log_i printf
#define platform_log_d printf
#define PLATFORM_WAIT_FOREVER AOS_WAIT_FOREVER
#define platform_msleep(x) aos_msleep(x)
#define BIT16 (1<<16)
#define BIT15 (1<<15)

unsigned int platform_irq_save(void);
void platform_irq_restore(unsigned int flag);

#endif


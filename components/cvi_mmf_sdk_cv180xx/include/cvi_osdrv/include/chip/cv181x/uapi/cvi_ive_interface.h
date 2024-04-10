/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: cvi_ive_interface.h
 * Description: ive driver interface header file
 */

#ifndef __CVI_IVE_INTERFACE_H__
#define __CVI_IVE_INTERFACE_H__

#include <semaphore.h>
#include "rtos_types.h"
#include "pthread.h"

struct cvi_ive_device {
	uintptr_t ive_base; /* NULL if not initialized. */
	unsigned int ive_irq; /* alarm and periodic irq */
	pthread_mutex_t close_lock;
	sem_t frame_done;
	sem_t op_done;
	int cur_optype;
	int tile_num;
	int total_tile;
	int use_count;
};

struct ive_profiling_info {
	char op_name[16];
	int tile_num;

	uint64_t time_ioctl_start;
	uint64_t time_ioctl_end;
	uint64_t time_vld_start;
	uint64_t time_vld_end;

	uint32_t time_ioctl_diff_us;
	uint32_t time_vld_diff_us[6];
	uint32_t time_tile_diff_us;
};

void start_vld_time(int optype);
void stop_vld_time(int optype, int tile_num);
int cvi_ive_init(void);
int cvi_ive_deinit(void);
int cvi_ive_open(void);
int cvi_ive_close(void);
int cvi_ive_ioctl(unsigned int cmd, void *arg);

#endif /* __CVI_IVE_INTERFACE_H__ */

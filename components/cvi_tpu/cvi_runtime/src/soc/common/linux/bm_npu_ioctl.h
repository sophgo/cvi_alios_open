/*
 * Bitmain SoC NPU Controller Driver
 *
 * Copyright (c) 2018 Bitmain Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __BM_NPU_IOCTL_H__
#define __BM_NPU_IOCTL_H__

#include <stdint.h>
#include <sys/ioctl.h>
#include <rtos_types.h>

struct bm_cache_op_arg {
  unsigned long long paddr;
  unsigned long long size;
  int dma_fd;
};

struct bm_submit_dma_arg {
  unsigned long long paddr;
  unsigned int seq_no;
};

struct bm_wait_dma_arg {
  unsigned int seq_no;
  int ret;
};

struct bm_pio_mode {
  unsigned long long cmdbuf;
  unsigned long long sz;
};

struct cvi_load_tee_arg {
  //ree domain
  unsigned long long cmdbuf_addr_ree;
  unsigned int cmdbuf_len_ree;
  unsigned long long weight_addr_ree;
  unsigned int weight_len_ree;
  unsigned long long neuron_addr_ree;

  //tee domain
  unsigned long long dmabuf_addr_tee;
};

struct cvi_submit_tee_arg {
  unsigned long long dmabuf_tee_addr;
  unsigned long long gaddr_base2;
  unsigned long long gaddr_base3;
  unsigned long long gaddr_base4;
  unsigned long long gaddr_base5;
  unsigned long long gaddr_base6;
  unsigned long long gaddr_base7;
  unsigned int seq_no;
};

struct cvi_unload_tee_arg {
  unsigned long long paddr;
  unsigned long long size;
};

#define CVITPU_SUBMIT_DMABUF      0x01
#define CVITPU_DMABUF_FLUSH_FD    0x02
#define CVITPU_DMABUF_INVLD_FD    0x03
#define CVITPU_DMABUF_FLUSH       0x04
#define CVITPU_DMABUF_INVLD       0x05
#define CVITPU_WAIT_DMABUF        0x06
#define CVITPU_PIO_MODE           0x07

#define CVITPU_LOAD_TEE           0x08
#define CVITPU_SUBMIT_TEE         0x09
#define CVITPU_UNLOAD_TEE         0x0A
#define CVITPU_SUBMIT_PIO         0x0B
#define CVITPU_WAIT_PIO           0x0C

#endif /* __BM_NPU_IOCTL_H__ */

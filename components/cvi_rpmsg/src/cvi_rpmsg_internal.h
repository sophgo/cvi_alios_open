/*
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Internal (private) declarations shared among the cvi_rpmsg source files.
 */

#ifndef CVI_RPMSG_INTERNAL_H
#define CVI_RPMSG_INTERNAL_H

#include <stdint.h>
#include <aos/kernel.h>
#include <metal/io.h>
#include <metal/sys.h>
#include <openamp/remoteproc.h>
#include <openamp/remoteproc_virtio.h>
#include <openamp/rpmsg_virtio.h>
#include "cvi_rpmsg.h"
#include "cvi_rpmsg_rsc_table.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Mailbox hardware (local copy of the register layout; no MMU, the
 * physical register base 0x01900000 is also the virtual address) ---- */

#define CVI_RPMSG_MBOX_REG_BASE       0x01900000UL
#define CVI_RPMSG_MBOX_CONTEXT_OFFSET 0x0400
#define CVI_RPMSG_MBOX_MAX_SLOT       8

/* Mailbox CPU index.  AliOS C906L = 2, Linux A53 = 0 (matches the existing
 * cvitek-mailbox.c: LOCAL_CPU_ID=0, REMOTE_CPU_ID=2, and the AliOS IPCM
 * RECEIVE_CPU=CPU_C960L=2).  AliOS receives on its own index (2) and sends
 * to Linux index (0). */
#define CVI_RPMSG_LOCAL_CPU_ID        2
#define CVI_RPMSG_REMOTE_CPU_ID       0

/* AliOS mailbox interrupt number (PLIC). */
#define CVI_RPMSG_MBOX_IRQ            61

/*used task for irq */
#define CVI_RPMSG_IRQ_IN_TASK

union cvi_mbox_info { uint8_t mbox_info; uint32_t reserved; };
union cvi_mbox_int_clr { uint8_t mbox_int_clr; uint32_t reserved; };
union cvi_mbox_int_mask { uint8_t mbox_int_mask; uint32_t reserved; };
union cvi_mbox_int_val { uint8_t mbox_int; uint32_t reserved; };
union cvi_mbox_int_raw { uint8_t mbox_int_raw; uint32_t reserved; };
union cvi_mbox_set { uint8_t mbox_set; uint32_t reserved; };

struct cvi_mbox_int {
	union cvi_mbox_int_clr  int_clr;
	union cvi_mbox_int_mask int_mask;
	union cvi_mbox_int_val  int_status;
	union cvi_mbox_int_raw  int_raw;
};

struct cvi_mailbox_set_register {
	union cvi_mbox_info  mbox_en[4];          /* 0x00..0x0c */
	struct cvi_mbox_int  mbox_set[4];         /* 0x10..0x4c */
	uint32_t             reserved[4];         /* 0x50..0x5c */
	union cvi_mbox_set   mbox_set_reg;        /* 0x60 */
	uint32_t             pad[3];
};

/* ---- Platform instance ---- */

struct cvi_rpmsg_inst {
	/* metal I/O regions over the shared-memory sub-areas.  Each region's
	 * default ops invalidate d-cache before reads and flush after writes,
	 * which is required for coherency with the Linux big core. */
	struct metal_io_region rsc_io;
	struct metal_io_region vring0_io;
	struct metal_io_region vring1_io;
	struct metal_io_region shm_io;
	metal_phys_addr_t      rsc_phys;
	metal_phys_addr_t      vring0_phys;
	metal_phys_addr_t      vring1_phys;
	metal_phys_addr_t      shm_phys;

	struct remoteproc              rproc;
	struct virtio_device          *vdev;
	struct rpmsg_virtio_device     rvdev;
	struct rpmsg_virtio_shm_pool   shpool;

	/* remoteproc memory regions covering the shared-memory sub-areas.
	 * remoteproc_set_rsc_table() and the virtio layer look up the I/O
	 * region for an address through rproc->mems, so every sub-area the
	 * firmware touches must be registered here. */
	struct remoteproc_mem rsc_mem;
	struct remoteproc_mem vring0_mem;
	struct remoteproc_mem vring1_mem;
	struct remoteproc_mem shm_mem;

	cvi_rpmsg_ns_bind_cb_t ns_bind_cb;

	volatile struct cvi_mailbox_set_register *mbox_regs;
	volatile uint32_t                        *mbox_context;

	/* Set only after rpmsg_init_vdev() has created the virtqueues.  The
	 * mailbox ISR must not call rproc_virtio_notified() before the vqs
	 * exist, otherwise virtqueue_notification(NULL) crashes — and Linux
	 * kicks us (host-online) right after setting DRIVER_OK, which races
	 * with our rpmsg_init_vdev() that follows wait_remote_ready(). */
	volatile int rpmsg_ready;

	/* Vring processing (rproc_virtio_notified -> virtqueue_notification
	 * -> rpmsg rx callback) takes mutexes, may wait for TX buffers, and
	 * has a deep call chain.  That is illegal in hard-IRQ context
	 * (scheduling while atomic / stack overflow / deadlock).  So the ISR
	 * only acks the hardware and records the pending notify id here, then
	 * signals notify_sem; a worker task runs the actual processing. */
	aos_sem_t notify_sem;
	aos_task_t task;  //CVI_RPMSG_IRQ_IN_TASK
	volatile unsigned int notify_pending;

	int inited;
};


/* ---- notify.c ---- */
int  cvi_rpmsg_mailbox_init(struct cvi_rpmsg_inst *inst);
void cvi_rpmsg_mailbox_deinit(struct cvi_rpmsg_inst *inst);
int  cvi_rpmsg_notify(void *priv, uint32_t id);   /* rpvdev_notify_func */
void cvi_rpmsg_notify_worker(void *arg);          /* worker task: runs vring processing */

/* ---- rproc_ops.c ---- */
extern const struct remoteproc_ops cvi_rpmsg_rproc_ops;

/* ---- rsc_table.c ---- */
int  cvi_rpmsg_rsc_table_init(void *va_base);

#ifdef __cplusplus
}
#endif

#endif /* CVI_RPMSG_INTERNAL_H */

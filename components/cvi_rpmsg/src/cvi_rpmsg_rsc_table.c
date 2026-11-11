/*
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Build the resource table in shared memory.
 *
 * AliOS (REMOTE) owns the resource table: it writes it once at init time,
 * flushes its d-cache so the bytes reach DRAM, and the Linux host then reads
 * it through the remoteproc attach path (get_loaded_rsc_table).  After init
 * the only field that changes is vdev.status, which Linux writes and AliOS
 * polls through metal_io (cache invalidate) in rproc_virtio_wait_remote_ready.
 */

#include <string.h>
#include <metal/cache.h>
#include "cvi_rpmsg_internal.h"

int cvi_rpmsg_rsc_table_init(void *va_base)
{
	struct cvi_rpmsg_rsc_table *rsc;
	uintptr_t base = (uintptr_t)va_base;

	if (!va_base)
		return -1;

	rsc = (struct cvi_rpmsg_rsc_table *)base;
	memset(rsc, 0, sizeof(*rsc));

	rsc->ver   = 1;
	rsc->num   = 1;
	rsc->reserved[0] = 0;
	rsc->reserved[1] = 0;
	rsc->offset[0]   = offsetof(struct cvi_rpmsg_rsc_table, vdev);

	/* virtio rpmsg device entry */
	rsc->vdev.type          = RSC_VDEV;
	rsc->vdev.id            = VIRTIO_ID_RPMSG;
	rsc->vdev.notifyid      = CVI_RPMSG_VDEV_NOTIFYID;
	rsc->vdev.dfeatures     = (1U << VIRTIO_RPMSG_F_NS); /* name service */
	rsc->vdev.gfeatures     = 0;
	rsc->vdev.config_len    = 0;
	rsc->vdev.status        = 0;
	rsc->vdev.num_of_vrings = 2;
	rsc->vdev.reserved[0]   = 0;
	rsc->vdev.reserved[1]   = 0;

	/* vring0: AliOS -> Linux (notifyid 0) */
	rsc->vring[0].da       = (uint32_t)(base + CVI_RPMSG_VRING0_OFFSET);
	rsc->vring[0].align    = CVI_RPMSG_VRING_ALIGN;
	rsc->vring[0].num      = CVI_RPMSG_VRING_DESC_NUM;
	rsc->vring[0].notifyid = CVI_RPMSG_VRING0_NOTIFYID;
	rsc->vring[0].reserved = 0;

	/* vring1: Linux -> AliOS (notifyid 1) */
	rsc->vring[1].da       = (uint32_t)(base + CVI_RPMSG_VRING1_OFFSET);
	rsc->vring[1].align    = CVI_RPMSG_VRING_ALIGN;
	rsc->vring[1].num      = CVI_RPMSG_VRING_DESC_NUM;
	rsc->vring[1].notifyid = CVI_RPMSG_VRING1_NOTIFYID;
	rsc->vring[1].reserved = 0;

	/* Zero the vring and buffer areas so no stale descriptors survive. */
	memset((void *)(base + CVI_RPMSG_VRING0_OFFSET), 0, CVI_RPMSG_VRING0_SIZE);
	memset((void *)(base + CVI_RPMSG_VRING1_OFFSET), 0, CVI_RPMSG_VRING1_SIZE);
	memset((void *)(base + CVI_RPMSG_BUFFER_OFFSET), 0, CVI_RPMSG_BUFFER_SIZE);

	/* Push all writes out of the C906L d-cache so the A53 can see them. */
	metal_cache_flush((void *)base, CVI_RPMSG_RSC_TABLE_SIZE);
	metal_cache_flush((void *)(base + CVI_RPMSG_VRING0_OFFSET),
			  CVI_RPMSG_VRING0_SIZE);
	metal_cache_flush((void *)(base + CVI_RPMSG_VRING1_OFFSET),
			  CVI_RPMSG_VRING1_SIZE);
	metal_cache_flush((void *)(base + CVI_RPMSG_BUFFER_OFFSET),
			  CVI_RPMSG_BUFFER_SIZE);

	return 0;
}

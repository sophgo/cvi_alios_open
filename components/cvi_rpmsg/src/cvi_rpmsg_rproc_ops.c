/*
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * remoteproc operations for the AliOS (C906L) side of the cvitek rpmsg link.
 *
 * AliOS is the REMOTE/DEVICE end and is already running when these ops are
 * used, so start/stop/config are no-ops.  The only op that does real work is
 * notify(), which kicks the Linux host through the mailbox.  mmap() returns
 * the matching metal_io_region for a shared-memory physical address (no MMU,
 * so phys == virt on C906L).
 */

#include <metal/sys.h>
#include "cvi_rpmsg_internal.h"

static struct remoteproc *cvi_rpmsg_rproc_init(struct remoteproc *rproc,
					       const struct remoteproc_ops *ops,
					       void *arg)
{
	/* remoteproc_init() already filled rproc->ops/priv; nothing more to
	 * do for this platform. */
	(void)ops;
	(void)arg;
	return rproc;
}

static void cvi_rpmsg_rproc_remove(struct remoteproc *rproc)
{
	(void)rproc;
}

static void *cvi_rpmsg_rproc_mmap(struct remoteproc *rproc,
				  metal_phys_addr_t *pa,
				  metal_phys_addr_t *da,
				  size_t size,
				  unsigned int attribute,
				  struct metal_io_region **io)
{
	struct cvi_rpmsg_inst *inst = (struct cvi_rpmsg_inst *)rproc->priv;
	metal_phys_addr_t addr;

	(void)attribute;
	(void)size;

	/* On C906L (no MMU) the device address equals the physical address. */
	addr = (da && *da != METAL_BAD_PHYS) ? *da :
	       (pa ? *pa : METAL_BAD_PHYS);

	/* Hand back the I/O region that covers this address so the virtio
	 * layer's cache flush/invalidate ops apply to the right area. */
	if (io) {
		if (addr >= inst->shm_phys &&
		    addr < inst->shm_phys + CVI_RPMSG_BUFFER_SIZE)
			*io = &inst->shm_io;
		else if (addr >= inst->vring0_phys &&
			 addr < inst->vring0_phys + CVI_RPMSG_VRING0_SIZE)
			*io = &inst->vring0_io;
		else if (addr >= inst->vring1_phys &&
			 addr < inst->vring1_phys + CVI_RPMSG_VRING1_SIZE)
			*io = &inst->vring1_io;
		else
			*io = &inst->rsc_io;
	}

	return (void *)(uintptr_t)addr;
}

static int cvi_rpmsg_rproc_start(struct remoteproc *rproc)
{
	/* AliOS is already running; nothing to boot. */
	(void)rproc;
	return 0;
}

static int cvi_rpmsg_rproc_stop(struct remoteproc *rproc)
{
	(void)rproc;
	return 0;
}

static int cvi_rpmsg_rproc_config(struct remoteproc *rproc, void *data)
{
	(void)rproc;
	(void)data;
	return 0;
}

static int cvi_rpmsg_rproc_notify(struct remoteproc *rproc, uint32_t id)
{
	return cvi_rpmsg_notify(rproc->priv, id);
}

const struct remoteproc_ops cvi_rpmsg_rproc_ops = {
	.init    = cvi_rpmsg_rproc_init,
	.remove  = cvi_rpmsg_rproc_remove,
	.mmap    = cvi_rpmsg_rproc_mmap,
	.start   = cvi_rpmsg_rproc_start,
	.stop    = cvi_rpmsg_rproc_stop,
	.config  = cvi_rpmsg_rproc_config,
	.notify  = cvi_rpmsg_rproc_notify,
};

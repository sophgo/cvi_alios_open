/*
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Main initialization and public API for the cvitek rpmsg platform on AliOS
 * (C906L, REMOTE/DEVICE side).
 *
 * AliOS builds the resource table and vrings in shared memory, creates the
 * virtio device in the DEVICE role, then blocks until the Linux host has
 * brought its side up (VIRTIO_CONFIG_STATUS_DRIVER_OK).  After that the rpmsg
 * bus is live and endpoints can be created.
 */

#include <string.h>
#include <stdio.h>
#include <metal/sys.h>
#include <metal/io.h>
#include <metal/log.h>
#include <metal/utilities.h>
#include <metal/cache.h>
#include <openamp/remoteproc.h>
#include <openamp/remoteproc_virtio.h>
#include <openamp/rpmsg.h>
#include <openamp/rpmsg_virtio.h>
#include <openamp/virtio.h>
#include "cvi_rpmsg.h"
#include "cvi_rpmsg_internal.h"


static struct cvi_rpmsg_inst g_inst;
static struct cvi_rpmsg_inst *g_cvi_rpmsg;

/* ---- Endpoint pool: multiple concurrent endpoints for multi-channel comms.
 * Each wraps an openamp rpmsg_endpoint (the storage openamp needs) plus the
 * user callback/priv.  The struct is opaque to callers (cvi_rpmsg_ept_t). ---- */

struct cvi_rpmsg_ept {
	struct rpmsg_endpoint ept;   /* openamp storage; must stay first */
	cvi_rpmsg_rx_cb_t          cb;
	cvi_rpmsg_ns_unbind_cb_t   unbind_cb;
	void                      *priv;
	int                        in_use;
};

static struct cvi_rpmsg_ept s_ept_pool[CVI_RPMSG_MAX_ENDPOINTS];
static aos_mutex_t         s_ept_lock;

static struct cvi_rpmsg_ept *cvi_rpmsg_ept_alloc(void)
{
	struct cvi_rpmsg_ept *cep = NULL;
	int i;

	aos_mutex_lock(&s_ept_lock, AOS_WAIT_FOREVER);
	for (i = 0; i < CVI_RPMSG_MAX_ENDPOINTS; i++) {
		if (!s_ept_pool[i].in_use) {
			s_ept_pool[i].in_use = 1;
			cep = &s_ept_pool[i];
			break;
		}
	}
	aos_mutex_unlock(&s_ept_lock);
	return cep;
}

static void cvi_rpmsg_ept_free(struct cvi_rpmsg_ept *cep)
{
	if (!cep)
		return;
	aos_mutex_lock(&s_ept_lock, AOS_WAIT_FOREVER);
	cep->in_use = 0;
	cep->cb = NULL;
	cep->unbind_cb = NULL;
	cep->priv = NULL;
	aos_mutex_unlock(&s_ept_lock);
}

/*
 * openamp rx callback adapter: recover the wrapping cvi_rpmsg_ept from the
 * openamp endpoint (which is its first member) and forward to the user cb.
 */
static int cvi_rpmsg_ept_rx_cb(struct rpmsg_endpoint *ept, void *data,
			       size_t len, uint32_t src, void *priv)
{
	struct cvi_rpmsg_ept *cep = metal_container_of(ept, struct cvi_rpmsg_ept, ept);
	(void)priv;
	if (cep->cb)
		return cep->cb((cvi_rpmsg_ept_t *)cep, data, len, src, cep->priv);
	return -1;
}

/*
 * openamp ns_unbind callback adapter: the remote destroyed the service bound
 * to this endpoint.  Recover the wrapper and forward to the user's unbind cb.
 */
static void cvi_rpmsg_ept_ns_unbind_cb(struct rpmsg_endpoint *ept)
{
	struct cvi_rpmsg_ept *cep = metal_container_of(ept, struct cvi_rpmsg_ept, ept);
	if (cep->unbind_cb)
		cep->unbind_cb((cvi_rpmsg_ept_t *)cep);
}

/*
 * openamp ns_bind callback adapter: forward to the user's cvi_rpmsg_ns_bind_cb_t
 * (which takes only name + remote addr, no openamp types).
 */
static void cvi_rpmsg_ns_bind_adapter(struct rpmsg_device *rdev,
				      const char *name, uint32_t dest)
{
	(void)rdev;
	if (g_cvi_rpmsg && g_cvi_rpmsg->ns_bind_cb)
		g_cvi_rpmsg->ns_bind_cb(name, dest);
}

static void cvi_rpmsg_io_init(struct cvi_rpmsg_inst *inst)
{
	uintptr_t base = CVI_RPMSG_SHM_BASE;

	inst->rsc_phys    = (metal_phys_addr_t)(base + CVI_RPMSG_RSC_TABLE_OFFSET);
	inst->vring0_phys = (metal_phys_addr_t)(base + CVI_RPMSG_VRING0_OFFSET);
	inst->vring1_phys = (metal_phys_addr_t)(base + CVI_RPMSG_VRING1_OFFSET);
	inst->shm_phys    = (metal_phys_addr_t)(base + CVI_RPMSG_BUFFER_OFFSET);

	/* Default ops invalidate d-cache before reads and flush after writes,
	 * which is mandatory for coherency with the A53 big core. */
	metal_io_init(&inst->rsc_io,
		      (void *)(base + CVI_RPMSG_RSC_TABLE_OFFSET),
		      &inst->rsc_phys, CVI_RPMSG_RSC_TABLE_SIZE,
		      12, 0, NULL);
	metal_io_init(&inst->vring0_io,
		      (void *)(base + CVI_RPMSG_VRING0_OFFSET),
		      &inst->vring0_phys, CVI_RPMSG_VRING0_SIZE,
		      12, 0, NULL);
	metal_io_init(&inst->vring1_io,
		      (void *)(base + CVI_RPMSG_VRING1_OFFSET),
		      &inst->vring1_phys, CVI_RPMSG_VRING1_SIZE,
		      12, 0, NULL);
	metal_io_init(&inst->shm_io,
		      (void *)(base + CVI_RPMSG_BUFFER_OFFSET),
		      &inst->shm_phys, CVI_RPMSG_BUFFER_SIZE,
		      12, 0, NULL);
}

/*
 * Register the shared-memory sub-areas as remoteproc memory regions.
 * remoteproc_set_rsc_table() resolves the rsc-table address through
 * rproc->mems (remoteproc_get_io_with_va), and the virtio layer later
 * resolves vring/buffer addresses the same way — so every sub-area must be
 * registered here, with the metal_io_region (and its cache ops) attached.
 * On C906L (no MMU) pa == da == va.
 */
static void cvi_rpmsg_mem_init(struct cvi_rpmsg_inst *inst)
{
	uintptr_t base = CVI_RPMSG_SHM_BASE;

	remoteproc_init_mem(&inst->rsc_mem, "rsc_table",
			    (metal_phys_addr_t)(base + CVI_RPMSG_RSC_TABLE_OFFSET),
			    (metal_phys_addr_t)(base + CVI_RPMSG_RSC_TABLE_OFFSET),
			    CVI_RPMSG_RSC_TABLE_SIZE, &inst->rsc_io);
	remoteproc_add_mem(&inst->rproc, &inst->rsc_mem);

	remoteproc_init_mem(&inst->vring0_mem, "vdev0vring0",
			    (metal_phys_addr_t)(base + CVI_RPMSG_VRING0_OFFSET),
			    (metal_phys_addr_t)(base + CVI_RPMSG_VRING0_OFFSET),
			    CVI_RPMSG_VRING0_SIZE, &inst->vring0_io);
	remoteproc_add_mem(&inst->rproc, &inst->vring0_mem);

	remoteproc_init_mem(&inst->vring1_mem, "vdev0vring1",
			    (metal_phys_addr_t)(base + CVI_RPMSG_VRING1_OFFSET),
			    (metal_phys_addr_t)(base + CVI_RPMSG_VRING1_OFFSET),
			    CVI_RPMSG_VRING1_SIZE, &inst->vring1_io);
	remoteproc_add_mem(&inst->rproc, &inst->vring1_mem);

	remoteproc_init_mem(&inst->shm_mem, "vdev0buffer",
			    (metal_phys_addr_t)(base + CVI_RPMSG_BUFFER_OFFSET),
			    (metal_phys_addr_t)(base + CVI_RPMSG_BUFFER_OFFSET),
			    CVI_RPMSG_BUFFER_SIZE, &inst->shm_io);
	remoteproc_add_mem(&inst->rproc, &inst->shm_mem);
}

int cvi_rpmsg_init(cvi_rpmsg_ns_bind_cb_t ns_bind_cb)
{
	struct metal_init_params params = METAL_INIT_DEFAULTS;
	struct cvi_rpmsg_inst *inst = &g_inst;
	struct cvi_rpmsg_rsc_table *rsc;
	int ret;

	if (inst->inited)
		return 0;

	inst->ns_bind_cb = ns_bind_cb;

	/* Endpoint pool for multi-channel communication. */
	aos_mutex_new(&s_ept_lock);
	memset(s_ept_pool, 0, sizeof(s_ept_pool));

	/* 1. libmetal */
	ret = metal_init(&params);
	if (ret) {
		metal_err("[cvi_rpmsg] step1 metal_init FAILED: %d", ret);
		return ret;
	}

#ifdef CVI_RPMSG_IRQ_IN_TASK
	/* 1b. notification worker (must exist before the mailbox ISR, which
	 * signals notify_sem).  Vring processing runs in this task, not in
	 * the ISR, so it can take mutexes / wait for TX buffers safely. */
	aos_sem_new(&inst->notify_sem, 0);
	aos_task_new_ext(&inst->task, "cvi_rpmsg_worker", cvi_rpmsg_notify_worker, inst, 8192, 15);
#endif

	/* 2. mailbox hardware + ISR */
	metal_info("[cvi_rpmsg] mailbox_init (irq=%d local_cpu=%d remote_cpu=%d)",
	       CVI_RPMSG_MBOX_IRQ, CVI_RPMSG_LOCAL_CPU_ID, CVI_RPMSG_REMOTE_CPU_ID);
	ret = cvi_rpmsg_mailbox_init(inst);
	if (ret) {
		metal_err("[cvi_rpmsg] step2 mailbox_init FAILED: %d (irq %d already taken by IPCM?)",
		       ret, CVI_RPMSG_MBOX_IRQ);
		goto err_metal;
	}

	/* 3. resource table in shared memory */
	metal_info("[cvi_rpmsg] rsc_table_init @0x%lx", (unsigned long)CVI_RPMSG_SHM_BASE);
	ret = cvi_rpmsg_rsc_table_init((void *)CVI_RPMSG_SHM_BASE);
	if (ret) {
		metal_err("[cvi_rpmsg] step3 rsc_table_init FAILED: %d", ret);
		goto err_mbox;
	}

	/* 4. metal I/O regions over the shared-memory sub-areas */
	cvi_rpmsg_io_init(inst);

	/* 5. remoteproc instance */
	if (remoteproc_init(&inst->rproc, &cvi_rpmsg_rproc_ops, inst) == NULL) {
		metal_err("[cvi_rpmsg] step5 remoteproc_init FAILED (NULL)");
		ret = -1;
		goto err_mbox;
	}

	/* 5b. register shared-memory sub-areas as remoteproc mem regions so
	 * that set_rsc_table / virtio can resolve addresses through rproc->mems */
	cvi_rpmsg_mem_init(inst);

	/* 6. register the resource table with the remoteproc core */
	rsc = (struct cvi_rpmsg_rsc_table *)CVI_RPMSG_SHM_BASE;
	ret = remoteproc_set_rsc_table(&inst->rproc,
				       (struct resource_table *)rsc,
				       sizeof(*rsc));
	if (ret) {
		metal_err("[cvi_rpmsg] step6 remoteproc_set_rsc_table FAILED: %d (rsc table malformed?)", ret);
		goto err_mbox;
	}

	/* 7. create the virtio device in DEVICE (REMOTE) role */
	inst->vdev = rproc_virtio_create_vdev(VIRTIO_DEV_DEVICE,
					      CVI_RPMSG_VDEV_NOTIFYID,
					      &rsc->vdev, &inst->rsc_io,
					      inst, cvi_rpmsg_notify, NULL);
	if (!inst->vdev) {
		metal_err("[cvi_rpmsg] step7 rproc_virtio_create_vdev FAILED (NULL)");
		ret = -1;
		goto err_mbox;
	}

	/* 8. initialize the two vrings */
	metal_info("[cvi_rpmsg] init_vring0 @0x%lx notifyid=%d",
	       (unsigned long)(CVI_RPMSG_SHM_BASE + CVI_RPMSG_VRING0_OFFSET),
	       CVI_RPMSG_VRING0_NOTIFYID);
	ret = rproc_virtio_init_vring(inst->vdev, 0,
				      CVI_RPMSG_VRING0_NOTIFYID,
				      (void *)(uintptr_t)(CVI_RPMSG_SHM_BASE +
							   CVI_RPMSG_VRING0_OFFSET),
				      &inst->vring0_io,
				      CVI_RPMSG_VRING_DESC_NUM,
				      CVI_RPMSG_VRING_ALIGN);
	if (ret) {
		metal_err("[cvi_rpmsg] step8 init_vring0 FAILED: %d", ret);
		goto err_mbox;
	}
	metal_info("[cvi_rpmsg] init_vring1 @0x%lx notifyid=%d",
	       (unsigned long)(CVI_RPMSG_SHM_BASE + CVI_RPMSG_VRING1_OFFSET),
	       CVI_RPMSG_VRING1_NOTIFYID);
	ret = rproc_virtio_init_vring(inst->vdev, 1,
				      CVI_RPMSG_VRING1_NOTIFYID,
				      (void *)(uintptr_t)(CVI_RPMSG_SHM_BASE +
							   CVI_RPMSG_VRING1_OFFSET),
				      &inst->vring1_io,
				      CVI_RPMSG_VRING_DESC_NUM,
				      CVI_RPMSG_VRING_ALIGN);
	if (ret) {
		metal_err("[cvi_rpmsg] step8 init_vring1 FAILED: %d", ret);
		goto err_mbox;
	}

	/* 9. wait for the Linux host to set VIRTIO_CONFIG_STATUS_DRIVER_OK.
	 * The status field lives in the resource table, which the host writes;
	 * rproc_virtio_get_status() invalidates the d-cache before reading. */
	metal_info("[cvi_rpmsg] wait_remote_ready (blocking until Linux DRIVER_OK)...");
	rproc_virtio_wait_remote_ready(inst->vdev);
	metal_info("[cvi_rpmsg] host ready (DRIVER_OK seen)");

	/* 10. shared buffer pool + rpmsg device */
	metal_info("[cvi_rpmsg] shm pool @0x%lx size=0x%x",
	       (unsigned long)(CVI_RPMSG_SHM_BASE + CVI_RPMSG_BUFFER_OFFSET),
	       CVI_RPMSG_BUFFER_SIZE);
	rpmsg_virtio_init_shm_pool(&inst->shpool,
				   (void *)(uintptr_t)(CVI_RPMSG_SHM_BASE +
						       CVI_RPMSG_BUFFER_OFFSET),
				   CVI_RPMSG_BUFFER_SIZE);

	ret = rpmsg_init_vdev(&inst->rvdev, inst->vdev, cvi_rpmsg_ns_bind_adapter,
			      &inst->shm_io, &inst->shpool);
	if (ret) {
		metal_err("[cvi_rpmsg] step10 rpmsg_init_vdev FAILED: %d", ret);
		goto err_mbox;
	}
	/* Virtqueues are created now — it is safe to service mailbox kicks. */
	inst->rpmsg_ready = 1;

	inst->inited = 1;
	g_cvi_rpmsg = inst;
	metal_info("[cvi_rpmsg] init OK");
	return 0;

err_mbox:
	cvi_rpmsg_mailbox_deinit(inst);
err_metal:
	metal_finish();
	return ret;
}

void cvi_rpmsg_deinit(void)
{
	struct cvi_rpmsg_inst *inst = &g_inst;

	if (!inst->inited)
		return;
#ifdef CVI_RPMSG_IRQ_IN_TASK
	aos_task_delete(&inst->task);
#endif
	rpmsg_deinit_vdev(&inst->rvdev);
	rproc_virtio_remove_vdev(inst->vdev);
	remoteproc_remove(&inst->rproc);
	cvi_rpmsg_mailbox_deinit(inst);
	metal_finish();

	inst->inited = 0;
	g_cvi_rpmsg = NULL;
}

int cvi_rpmsg_is_ready(void)
{
	return (g_cvi_rpmsg && g_cvi_rpmsg->inited) ? 1 : 0;
}

cvi_rpmsg_ept_t *cvi_rpmsg_create_ept(const char *name, uint32_t src,
				       uint32_t dest, cvi_rpmsg_rx_cb_t cb,
				       cvi_rpmsg_ns_unbind_cb_t unbind_cb,
				       void *priv)
{
	struct cvi_rpmsg_ept *cep;
	struct rpmsg_device *rdev;
	int ret;

	if (!g_cvi_rpmsg || !g_cvi_rpmsg->inited || !name || !cb)
		return NULL;

	rdev = rpmsg_virtio_get_rpmsg_device(&g_cvi_rpmsg->rvdev);
	if (!rdev)
		return NULL;

	cep = cvi_rpmsg_ept_alloc();
	if (!cep)
		return NULL;

	memset(&cep->ept, 0, sizeof(cep->ept));
	cep->cb = cb;
	cep->unbind_cb = unbind_cb;
	cep->priv = priv;

	ret = rpmsg_create_ept(&cep->ept, rdev, name, src, dest,
			       cvi_rpmsg_ept_rx_cb, cvi_rpmsg_ept_ns_unbind_cb);
	if (ret) {
		cvi_rpmsg_ept_free(cep);
		return NULL;
	}
	return (cvi_rpmsg_ept_t *)cep;
}

void cvi_rpmsg_destroy_ept(cvi_rpmsg_ept_t *ept)
{
	struct cvi_rpmsg_ept *cep = (struct cvi_rpmsg_ept *)ept;

	if (!cep)
		return;
	rpmsg_destroy_ept(&cep->ept);
	cvi_rpmsg_ept_free(cep);
}

int cvi_rpmsg_send(cvi_rpmsg_ept_t *ept, const void *data, int len)
{
	struct cvi_rpmsg_ept *cep = (struct cvi_rpmsg_ept *)ept;

	if (!cep)
		return -1;
	return rpmsg_send(&cep->ept, data, len);
}

int cvi_rpmsg_sendto(cvi_rpmsg_ept_t *ept, const void *data, int len,
		     uint32_t dst)
{
	struct cvi_rpmsg_ept *cep = (struct cvi_rpmsg_ept *)ept;

	if (!cep)
		return -1;
	return rpmsg_sendto(&cep->ept, data, len, dst);
}

int cvi_rpmsg_trysend(cvi_rpmsg_ept_t *ept, const void *data, int len)
{
	struct cvi_rpmsg_ept *cep = (struct cvi_rpmsg_ept *)ept;

	if (!cep)
		return -1;
	return rpmsg_trysend(&cep->ept, data, len);
}

int cvi_rpmsg_trysendto(cvi_rpmsg_ept_t *ept, const void *data, int len,
			uint32_t dst)
{
	struct cvi_rpmsg_ept *cep = (struct cvi_rpmsg_ept *)ept;

	if (!cep)
		return -1;
	return rpmsg_trysendto(&cep->ept, data, len, dst);
}

void *cvi_rpmsg_get_tx_payload_buffer(cvi_rpmsg_ept_t *ept, uint32_t *len,
				      int wait)
{
	struct cvi_rpmsg_ept *cep = (struct cvi_rpmsg_ept *)ept;

	if (!cep || !len)
		return NULL;
	return rpmsg_get_tx_payload_buffer(&cep->ept, len, wait);
}

int cvi_rpmsg_send_nocopy(cvi_rpmsg_ept_t *ept, const void *data, int len)
{
	struct cvi_rpmsg_ept *cep = (struct cvi_rpmsg_ept *)ept;

	if (!cep)
		return -1;
	return rpmsg_send_nocopy(&cep->ept, data, len);
}

int cvi_rpmsg_sendto_nocopy(cvi_rpmsg_ept_t *ept, const void *data, int len,
			    uint32_t dst)
{
	struct cvi_rpmsg_ept *cep = (struct cvi_rpmsg_ept *)ept;

	if (!cep)
		return -1;
	return rpmsg_sendto_nocopy(&cep->ept, data, len, dst);
}

void cvi_rpmsg_hold_rx_buffer(cvi_rpmsg_ept_t *ept, void *rxbuf)
{
	struct cvi_rpmsg_ept *cep = (struct cvi_rpmsg_ept *)ept;

	if (cep)
		rpmsg_hold_rx_buffer(&cep->ept, rxbuf);
}

void cvi_rpmsg_release_rx_buffer(cvi_rpmsg_ept_t *ept, void *rxbuf)
{
	struct cvi_rpmsg_ept *cep = (struct cvi_rpmsg_ept *)ept;

	if (cep)
		rpmsg_release_rx_buffer(&cep->ept, rxbuf);
}

int cvi_rpmsg_ept_ready(cvi_rpmsg_ept_t *ept)
{
	struct cvi_rpmsg_ept *cep = (struct cvi_rpmsg_ept *)ept;

	if (!cep)
		return 0;
	return (int)is_rpmsg_ept_ready(&cep->ept);
}

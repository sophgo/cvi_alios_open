/*
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Mailbox notification (kick) layer for cvitek rpmsg on AliOS (C906L).
 *
 * The mailbox hardware only carries a notification (which vring has pending
 * work); all rpmsg payload travels through the vrings in shared memory.  A
 * kick is a single mailbox slot write keyed by the vring notify id:
 *
 *   AliOS -> Linux : write slot <notifyid>, trigger CPU index 0 (A53)
 *   Linux -> AliOS : (symmetric on the Linux side) triggers CPU index 2
 *
 * On reception AliOS decodes the slot index and feeds it to
 * rproc_virtio_notified(), which dispatches to the matching virtqueue.
 */

#include <drv/cvi_irq.h>
#include <stdio.h>
#include "cvi_rpmsg_internal.h"


/*
 * Mailbox ISR.  Registered for PLIC IRQ 61 (MBOX_INT_C906_2ND).  AliOS receives
 * on its own mailbox CPU index (LOCAL_CPU_ID = 2).
 */
static void cvi_rpmsg_mailbox_isr(int irq, void *dev_id)
{
	struct cvi_rpmsg_inst *inst = (struct cvi_rpmsg_inst *)dev_id;
	volatile struct cvi_mailbox_set_register *regs;
	uint8_t set_val;
	int i;

	(void)irq;
	if (!inst || !inst->mbox_regs)
		return;

	regs = inst->mbox_regs;
	set_val = regs->mbox_set[CVI_RPMSG_LOCAL_CPU_ID].int_status.mbox_int;

	if (!set_val)
		return;

	for (i = 0; i < CVI_RPMSG_MBOX_MAX_SLOT && set_val; i++) {
		uint8_t mask = (uint8_t)(set_val & (1 << i));

		if (!mask)
			continue;

		/* Clear the interrupt for this slot on our CPU. */
		regs->mbox_set[CVI_RPMSG_LOCAL_CPU_ID].int_clr.mbox_int_clr = mask;
		/* Disable the slot's enable bit on our CPU. */
		regs->mbox_en[CVI_RPMSG_LOCAL_CPU_ID].mbox_info &= ~mask;
		/* Clear the mailbox context slot. */
		inst->mbox_context[i] = 0;
#ifdef CVI_RPMSG_IRQ_IN_TASK
		/* Queue the vring notify id (slot index == notify id, see
		 * cvi_rpmsg_rsc_table.c) and wake the worker task to run
		 * rproc_virtio_notified() in task context.  The actual vring
		 * processing takes mutexes / may wait for TX buffers / has a
		 * deep call chain, so it must NOT run here in hard-IRQ context.
		 *
		 * Always queue, even before rpmsg_ready: Linux kicks slot 0
		 * immediately after setting DRIVER_OK (virtio_rpmsg_bus), which
		 * races with our rpmsg_init_vdev() that follows wait_remote_ready.
		 * Gating on rpmsg_ready here would drop that first kick and the
		 * worker would never wake.  The worker defers the actual
		 * rproc_virtio_notified() until the virtqueues exist. */
		__sync_or_and_fetch(&inst->notify_pending, 1U << i);
		aos_sem_signal(&inst->notify_sem);
#else
		rproc_virtio_notified(inst->vdev, (uint32_t)i);
#endif
		/* metal_dbg("[cvi_rpmsg] isr slot=%d", i); */
	}
}

#ifdef CVI_RPMSG_IRQ_IN_TASK
/*
 * Worker task: consume the pending notify ids and run the virtio vring
 * processing in a sleepable, full-stack task context.
 */
void cvi_rpmsg_notify_worker(void *arg)
{
	struct cvi_rpmsg_inst *inst = (struct cvi_rpmsg_inst *)arg;
	unsigned int pending;
	int i;

	while (1) {
		aos_sem_wait(&inst->notify_sem, AOS_WAIT_FOREVER);

		/* Atomically grab and clear the pending bitmask so an ISR
		 * arriving between read and clear can't lose a bit. */
		pending = __sync_fetch_and_and(&inst->notify_pending, 0);
		if (!pending)
			continue;

		/* The kick may have arrived during the DRIVER_OK / rpmsg_init_vdev
		 * race window, before the virtqueues exist.  Don't drop it — put
		 * the bits back, briefly yield, and re-wake ourselves until we're
		 * ready, so the first (and any early) kick is serviced once the
		 * vqs are up instead of being lost. */
		if (!inst->vdev || !inst->rpmsg_ready) {
			__sync_or_and_fetch(&inst->notify_pending, pending);
			aos_msleep(1);
			aos_sem_signal(&inst->notify_sem);
			continue;
		}

		for (i = 0; i < CVI_RPMSG_MBOX_MAX_SLOT; i++) {
			if (pending & (1U << i)) {
				rproc_virtio_notified(inst->vdev, (uint32_t)i);
			}
		}
		/* Loop: aos_sem_wait blocks until the next kick. */
	}
}
#endif

/*
 * Kick the Linux host: write slot <id> and trigger an interrupt on CPU 0.
 * This is the rpvdev_notify_func handed to rproc_virtio_create_vdev().
 */
int cvi_rpmsg_notify(void *priv, uint32_t id)
{
	struct cvi_rpmsg_inst *inst = (struct cvi_rpmsg_inst *)priv;
	volatile struct cvi_mailbox_set_register *regs;
	uint8_t mask;

	if (!inst || !inst->mbox_regs || id >= CVI_RPMSG_MBOX_MAX_SLOT)
		return -1;

	/* metal_dbg("[cvi_rpmsg] notify: kick id=%lu", (unsigned long)id); */

	regs = inst->mbox_regs;
	mask = (uint8_t)(1 << id);

	/* Write the slot index into the mailbox context (informational; the
	 * slot bit itself is what the host decodes). */
	inst->mbox_context[id] = id;

	/* Clear any pending interrupt for this slot on the target (Linux) CPU. */
	regs->mbox_set[CVI_RPMSG_REMOTE_CPU_ID].int_clr.mbox_int_clr = mask;
	/* Enable the slot on the target CPU. */
	regs->mbox_en[CVI_RPMSG_REMOTE_CPU_ID].mbox_info |= mask;
	/* Trigger the mailbox interrupt on the target CPU. */
	regs->mbox_set_reg.mbox_set = mask;

	return 0;
}

int cvi_rpmsg_mailbox_init(struct cvi_rpmsg_inst *inst)
{
	uintptr_t base = (uintptr_t)CVI_RPMSG_MBOX_REG_BASE;
	int ret;

	inst->mbox_regs    = (volatile struct cvi_mailbox_set_register *)base;
	inst->mbox_context = (volatile uint32_t *)(base +
			      CVI_RPMSG_MBOX_CONTEXT_OFFSET);

	/* Clear all mailbox context slots and any pending local interrupts. */
	for (int i = 0; i < CVI_RPMSG_MBOX_MAX_SLOT; i++)
		inst->mbox_context[i] = 0;
	inst->mbox_regs->mbox_set[CVI_RPMSG_LOCAL_CPU_ID].int_clr.mbox_int_clr =
		(uint8_t)0xff;

	ret = request_irq(CVI_RPMSG_MBOX_IRQ, cvi_rpmsg_mailbox_isr, 0,
			  "cvi_rpmsg", inst);
	if (ret) {
		return ret;
	}

	return 0;
}

void cvi_rpmsg_mailbox_deinit(struct cvi_rpmsg_inst *inst)
{
	(void)inst;
	/* AliOS does not provide a free_irq wrapper; the ISR stays registered
	 * for the lifetime of the system. */
}

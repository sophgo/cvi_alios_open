/*
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Resource table and shared memory layout for cvitek dual-core rpmsg.
 *
 * The shared memory is physically shared between the Linux big core (A53,
 * HOST/DRIVER) and the AliOS small core (C906L, REMOTE/DEVICE).  AliOS builds
 * the resource table at the beginning of the shared memory region; Linux reads
 * it through the remoteproc "attach" path (get_loaded_rsc_table).
 *
 * Data-structure note: every field stored in shared memory uses a fixed-width
 * type (uint32_t / the open_amp fw_rsc_* structs which are packed).  This keeps
 * the layout identical across the 64-bit AliOS (C906L, unsigned long = 8 bytes)
 * and the 32-bit Linux (A53) worlds — no `unsigned long` or pointer is ever
 * stored in shared memory.
 */

#ifndef CVI_RPMSG_RSC_TABLE_H
#define CVI_RPMSG_RSC_TABLE_H

#include <stdint.h>
#include <openamp/remoteproc.h>
#include "cvi_board_memmap.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Shared memory base.  This must match CVIMMAP_SHARE_MEM_ADDR used by the Linux
 * DTS (cv184x_default_memmap.dtsi).  AliOS on C906L has no MMU, so the physical
 * address is also the virtual address.
 */
#define CVI_RPMSG_SHM_BASE           CVIMMAP_SHARE_MEM_ADDR
#define CVI_RPMSG_SHM_SIZE            0x00018000UL  /* 96 KiB used of 128 KiB   */

/* Sub-region offsets within the shared memory.  These match the three reserved
 * memory regions declared in the Linux DTS:
 *   vdev0vring0_reserved : SHM + 0x0000, 0x4000  (holds rsc table + vring0)
 *   vdev0vring1_reserved : SHM + 0x4000, 0x4000  (holds vring1)
 *   vdev0buffer_reserved : SHM + 0x8000, 0x10000 (holds rpmsg buffer pool)
 */
#define CVI_RPMSG_RSC_TABLE_OFFSET    0x0000
#define CVI_RPMSG_RSC_TABLE_SIZE      0x1000

#define CVI_RPMSG_VRING0_OFFSET       0x1000
#define CVI_RPMSG_VRING0_SIZE         0x3000

#define CVI_RPMSG_VRING1_OFFSET       0x4000
#define CVI_RPMSG_VRING1_SIZE         0x4000

#define CVI_RPMSG_BUFFER_OFFSET       0x8000
#define CVI_RPMSG_BUFFER_SIZE         0x10000

/* virtio vring parameters */
#define CVI_RPMSG_VRING_DESC_NUM      16
#define CVI_RPMSG_VRING_ALIGN         0x1000

/* Notification ids.  These are written into the resource table by AliOS and
 * read back by Linux, so both sides agree on which mailbox slot carries which
 * vring's kick.  The vdev notify id (used for status kicks) is distinct from
 * the vring notify ids so the remoteproc id allocator never sees a conflict. */
#define CVI_RPMSG_VDEV_NOTIFYID       2
#define CVI_RPMSG_VRING0_NOTIFYID     0
#define CVI_RPMSG_VRING1_NOTIFYID     1

/* virtio ids / features */
#ifndef VIRTIO_ID_RPMSG
#define VIRTIO_ID_RPMSG               7
#endif
#ifndef VIRTIO_RPMSG_F_NS
#define VIRTIO_RPMSG_F_NS             0
#endif

/*
 * The resource table.  The layout is:
 *   resource_table header (ver, num, reserved[2], offset[1])
 *   one fw_rsc_vdev entry (type, id, notifyid, ... two vrings)
 *
 * offset[0] points at the fw_rsc_vdev.  The fw_rsc_vdev's first field is
 * `type` which doubles as the fw_rsc_hdr.type the parser reads — this matches
 * the on-disk layout Linux expects (struct fw_rsc_vdev in the Linux uapi omits
 * `type` because it lives in the wrapping fw_rsc_hdr, but the bytes are the
 * same: [type][id][notifyid]...).
 */
struct cvi_rpmsg_rsc_table {
	uint32_t ver;
	uint32_t num;
	uint32_t reserved[2];
	uint32_t offset[1];            /* one resource entry */
	struct fw_rsc_vdev vdev;
	struct fw_rsc_vdev_vring vring[2];
	/* config space follows vring[]; config_len is 0, so nothing here */
} __attribute__((packed));

/*
 * Return the address (== physical address on AliOS) of the resource table.
 */
static inline void *cvi_rpmsg_rsc_table_addr(void)
{
	return (void *)(uintptr_t)CVI_RPMSG_SHM_BASE;
}

/*
 * Build the resource table in shared memory.  Returns 0 on success.
 */
int cvi_rpmsg_rsc_table_init(void *va_base);

#ifdef __cplusplus
}
#endif

#endif /* CVI_RPMSG_RSC_TABLE_H */

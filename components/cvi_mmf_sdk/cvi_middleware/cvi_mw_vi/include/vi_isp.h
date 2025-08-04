#ifndef __U_VI_ISP_H__
#define __U_VI_ISP_H__

#include "osal_types.h"
#include "vi_snsr.h"

enum ISP_SCENE_INFO {
	FE_ON_POST_OFF_SC,
	FE_OFF_POST_OFF_SC,
	FE_SLICE_POST_OFF_SC,
	FE_ON_POST_ON_SC,
	FE_OFF_POST_ON_SC,
	FE_SLICE_POST_ON_SC,
};

enum sop_isp_source {
	ISP_SOURCE_DEV = 0,
	ISP_SOURCE_FE,
	ISP_SOURCE_POST,
	ISP_SOURCE_MAX,
};

/* struct sop_vip_memblock
 * @base: the address of the memory allocated.
 * @size: Size in bytes of the memblock.
 */
struct sop_vip_memblock {
	__u8  raw_num;
	__u64 phy_addr;
	void *vir_addr;
#ifdef __arm__
	__u32 padding;
#endif
	__u32 size;
};

struct sop_vip_isp_raw_blk {
	struct sop_vip_memblock raw_dump;
	__u32 frm_num;
	__u32 time_out;//msec
	__u16 src_w;
	__u16 src_h;
	__u16 crop_x;
	__u16 crop_y;
	__u8  is_b_not_rls;
	__u8  is_timeout;
	__u8  is_sig_int;
};

struct sop_vip_isp_smooth_raw_param {
	__u64 raw_blk_phyaddr;
	__u8 raw_num;
	__u8 frm_num;
};

struct sop_isp_sts_mem {
	__u8			raw_num;
	struct sop_vip_memblock ae_le;
	struct sop_vip_memblock ae_se;
	struct sop_vip_memblock af;
	struct sop_vip_memblock lsc_hist;
	struct sop_vip_memblock drc_hist;
	struct sop_vip_memblock ldci_hist;
};

struct isp_proc_cfg {
	void *buffer;
#ifdef __arm__
	__u32 padding;
#endif
	size_t buffer_size;
};

#endif // __U_VI_ISP_H__

#ifndef __LDC_CB_H__
#define __LDC_CB_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include <base_ctx.h>

#define DEFAULT_MESH_PADDR	0x80000000
#define GDC_SUPPORT_FMT(fmt)                                                   \
	((fmt == PIXEL_FORMAT_NV12) || (fmt == PIXEL_FORMAT_NV21) ||           \
	 (fmt == PIXEL_FORMAT_YUV_400) || (fmt == PIXEL_FORMAT_RGB_888_PLANAR) ||    \
	 (fmt == PIXEL_FORMAT_BGR_888_PLANAR) || (fmt == PIXEL_FORMAT_YUV_PLANAR_444) || \
	 (fmt == PIXEL_FORMAT_YUV_PLANAR_420))

enum GDC_USAGE {
	GDC_USAGE_ROTATION,
	GDC_USAGE_FISHEYE,
	GDC_USAGE_LDC,
	GDC_USAGE_MAX
};

typedef CVI_VOID (*gdc_cb)(CVI_VOID *, VB_BLK);

struct mesh_gdc_cfg {
	enum GDC_USAGE usage;
	const CVI_VOID *pUsageParam;
	struct vb_s *vb_in;
	PIXEL_FORMAT_E enPixFormat;
	CVI_U64 mesh_addr;
	CVI_BOOL sync_io;
	CVI_VOID *pcbParam;
	CVI_U32 cbParamSize;
	ROTATION_E enRotation;
};

struct ldc_op_done_cfg {
	CVI_VOID *pParam;
	VB_BLK blk;
};

enum LDC_CB_CMD {
	LDC_CB_MESH_GDC_OP,
	LDC_CB_VPSS_SBM_DONE,
	LDC_CB_GDC_OP_DONE = 100,	/* Skip VI/VPSS/VO self cmd */
	LDC_CB_MAX
};

#ifdef __cplusplus
}
#endif

#endif /* __LDC_CB_H__ */

#include "cvi_debug.h"
#include "osal.h"
#include "driver_base.h"
#include "vb_ioctl.h"


#define VB_CTRL_PTR(_cfg, _ioctl)\
	do {\
		struct vb_ext_control ec1;\
		int ret;\
		osal_memset(&ec1, 0, sizeof(ec1));\
		ec1.id = _ioctl;\
		ec1.ptr = (void *)_cfg;\
		ret = driver_base_ioctl(BASE_VB_CMD, (unsigned long)&ec1);\
		if (ret < 0)\
			CVI_TRACE_VB(CVI_DBG_ERR, "BASE_VB_CMD - %s NG\n", __func__);\
		return ret;\
	} while (0)

#define VB_CTRL_S_VALUE(_cfg, _ioctl)\
	do {\
		struct vb_ext_control ec1;\
		int ret;\
		osal_memset(&ec1, 0, sizeof(ec1));\
		ec1.id = _ioctl;\
		ec1.value = _cfg;\
		ret = driver_base_ioctl(BASE_VB_CMD, (unsigned long)&ec1);\
		if (ret < 0)\
			CVI_TRACE_VB(CVI_DBG_ERR, "BASE_VB_CMD - %s NG\n", __func__);\
		return ret;\
	} while (0)

#define VB_CTRL_S_VALUE64(_cfg, _ioctl)\
	do {\
		struct vb_ext_control ec1;\
		int ret;\
		osal_memset(&ec1, 0, sizeof(ec1));\
		ec1.id = _ioctl;\
		ec1.value64 = _cfg;\
		ret = driver_base_ioctl(BASE_VB_CMD, (unsigned long)&ec1);\
		if (ret < 0)\
			CVI_TRACE_VB(CVI_DBG_ERR, "BASE_VB_CMD - %s NG\n", __func__);\
		return ret;\
	} while (0)

#define VB_CTRL_G_VALUE(_out, _ioctl)\
	do {\
		struct vb_ext_control ec1;\
		int ret;\
		osal_memset(&ec1, 0, sizeof(ec1));\
		ec1.id = _ioctl;\
		ec1.value = 0;\
		ret = driver_base_ioctl(BASE_VB_CMD, (unsigned long)&ec1);\
		if (ret < 0)\
			CVI_TRACE_VB(CVI_DBG_ERR, "BASE_VB_CMD - %s NG\n", __func__);\
		*_out = ec1.value;\
		return ret;\
	} while (0)

int vb_ioctl_set_config(struct vb_cfg *cfg)
{
	VB_CTRL_PTR(cfg, VB_IOCTL_SET_CONFIG);
}

int vb_ioctl_get_config(struct vb_cfg *cfg)
{
	VB_CTRL_PTR(cfg, VB_IOCTL_GET_CONFIG);
}

int vb_ioctl_init(void)
{
	VB_CTRL_PTR(NULL, VB_IOCTL_INIT);
}

int vb_ioctl_exit(void)
{
	VB_CTRL_PTR(NULL, VB_IOCTL_EXIT);
}

int vb_ioctl_create_pool(struct vb_pool_cfg *cfg)
{
	VB_CTRL_PTR(cfg, VB_IOCTL_CREATE_POOL);
}

int vb_ioctl_create_ex_pool(struct vb_pool_ex_cfg *cfg)
{
	VB_CTRL_PTR(cfg, VB_IOCTL_CREATE_EX_POOL);
}

int vb_ioctl_destroy_pool(VB_POOL poolId)
{
	VB_CTRL_S_VALUE(poolId, VB_IOCTL_DESTROY_POOL);
}

int vb_ioctl_phys_to_handle(struct vb_blk_info *blk_info)
{
	VB_CTRL_PTR(blk_info, VB_IOCTL_PHYS_TO_HANDLE);
}

int vb_ioctl_get_blk_info(struct vb_blk_info *blk_info)
{
	VB_CTRL_PTR(blk_info, VB_IOCTL_GET_BLK_INFO);
}

int vb_ioctl_get_pool_cfg(struct vb_pool_cfg *pool_cfg)
{
	VB_CTRL_PTR(pool_cfg, VB_IOCTL_GET_POOL_CFG);
}

int vb_ioctl_get_block(struct vb_blk_cfg *blk_cfg)
{
	VB_CTRL_PTR(blk_cfg, VB_IOCTL_GET_BLOCK);
}

int vb_ioctl_release_block(VB_BLK blk)
{
	VB_CTRL_S_VALUE64(blk, VB_IOCTL_RELEASE_BLOCK);
}

int vb_ioctl_get_pool_max_cnt(unsigned int *vb_max_pools)
{
	VB_CTRL_G_VALUE(vb_max_pools, VB_IOCTL_GET_POOL_MAX_CNT);
}

int vb_ioctl_print_pool(VB_POOL poolId)
{
	VB_CTRL_S_VALUE(poolId, VB_IOCTL_PRINT_POOL);
}


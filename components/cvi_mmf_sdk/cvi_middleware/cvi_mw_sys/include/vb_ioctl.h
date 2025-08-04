#ifndef MODULES_VB_IOCTL_H_
#define MODULES_VB_IOCTL_H_

#include "cvi_comm_vb.h"
#include "base_uapi.h"

int vb_ioctl_set_config(struct vb_cfg *cfg);
int vb_ioctl_get_config(struct vb_cfg *cfg);
int vb_ioctl_init(void);
int vb_ioctl_exit(void);
int vb_ioctl_create_pool(struct vb_pool_cfg *cfg);
int vb_ioctl_create_ex_pool(struct vb_pool_ex_cfg *cfg);
int vb_ioctl_destroy_pool(VB_POOL poolId);
int vb_ioctl_phys_to_handle(struct vb_blk_info *blk_info);
int vb_ioctl_get_blk_info(struct vb_blk_info *blk_info);
int vb_ioctl_get_pool_cfg(struct vb_pool_cfg *pool_cfg);
int vb_ioctl_get_block(struct vb_blk_cfg *blk_cfg);
int vb_ioctl_release_block(VB_BLK blk);
int vb_ioctl_get_pool_max_cnt(unsigned int *vb_max_pools);
int vb_ioctl_print_pool(VB_POOL poolId);

#endif // MODULES_VB_IOCTL_H_

#ifndef __U_VI_UAPI_H__
#define __U_VI_UAPI_H__

#ifdef __cplusplus
	extern "C" {
#endif

#include <time.h>
#include <cvi_comm_vi.h>
#include <cvi_comm_sys.h>

#define VI_IOC_BASE		0x20

#define VI_IOC_G_CTRL		VI_IOC_BASE
#define VI_IOC_S_CTRL		(VI_IOC_BASE + 1)

enum VI_IOCTL {
	VI_IOCTL_ONLINE,
	VI_IOCTL_HDR,
	VI_IOCTL_3DNR,
	VI_IOCTL_TILE,
	VI_IOCTL_COMPRESS_EN,
	VI_IOCTL_STS_MEM,
	VI_IOCTL_STS_GET,
	VI_IOCTL_STS_PUT,
	VI_IOCTL_POST_STS_GET,
	VI_IOCTL_POST_STS_PUT,
	VI_IOCTL_USR_PIC_CFG,
	VI_IOCTL_USR_PIC_ONOFF,
	VI_IOCTL_USR_PIC_PUT,
	VI_IOCTL_AE_CFG,
	VI_IOCTL_AWB_CFG,
	VI_IOCTL_AF_CFG,
	VI_IOCTL_USR_PIC_TIMING,
	VI_IOCTL_GET_LSC_PHY_BUF,
	VI_IOCTL_CSIBDG_CFG,
	VI_IOCTL_GET_TUN_ADDR,
	VI_IOCTL_SET_SNR_INFO,
	VI_IOCTL_SET_SNR_CFG_NODE,
	VI_IOCTL_GET_PIPE_DUMP,
	VI_IOCTL_PUT_PIPE_DUMP,
	VI_IOCTL_SET_RGBMAP_IDX,
	VI_IOCTL_HDR_DETAIL_EN,
	VI_IOCTL_YUV_BYPASS_PATH,
	VI_IOCTL_BE_ONLINE,
	VI_IOCTL_SUBLVDS_PATH,
	VI_IOCTL_GET_IP_INFO,
	VI_IOCTL_TRIG_PRERAW,
	VI_IOCTL_SET_PROC_CONTENT,
	VI_IOCTL_SC_ONLINE,
	VI_IOCTL_MMAP_GRID_SIZE,
	VI_IOCTL_RGBIR,
	VI_IOCTL_AWB_STS_GET,
	VI_IOCTL_AWB_STS_PUT,
	VI_IOCTL_GET_FSWDR_PHY_BUF,
	VI_IOCTL_GET_SCENE_INFO,
	VI_IOCTL_CLK_CTRL,
	VI_IOCTL_GET_BUF_SIZE,
	VI_IOCTL_SET_DMA_BUF_INFO,
	VI_IOCTL_ENQ_BUF,
	VI_IOCTL_DQEVENT,
	VI_IOCTL_START_STREAMING,
	VI_IOCTL_STOP_STREAMING,
	VI_IOCTL_SET_SLICE_BUF_EN,
	VI_IOCTL_GET_CLUT_TBL_IDX,
	VI_IOCTL_SDK_CTRL,
	VI_IOCTL_GET_SHARE_MEM,
	VI_IOCTL_ENABLE_PATTERN,
	VI_IOCTL_QUERY_ISP_EVENT,
	VI_IOCTL_CHECK_ISP_EVENT,
	VI_IOCTL_TUNING_PARAM,
	VI_IOCTL_MAX,
};

enum VI_SDK_CTRL {
	VI_SDK_SET_DEVATTR,
	VI_SDK_GET_DEVATTR,
	VI_SDK_ENABLE_DEV,
	VI_SDK_DISABLE_DEV,
	VI_SDK_CREATE_PIPE,
	VI_SDK_DESTROY_PIPE,
	VI_SDK_SET_PIPEATTR,
	VI_SDK_GET_PIPEATTR,
	VI_SDK_START_PIPE,
	VI_SDK_STOP_PIPE,
	VI_SDK_SET_CHNATTR,
	VI_SDK_GET_CHNATTR,
	VI_SDK_ENABLE_CHN,
	VI_SDK_DISABLE_CHN,
	VI_SDK_SET_MOTION_LV,
	VI_SDK_ENABLE_DIS,
	VI_SDK_DISABLE_DIS,
	VI_SDK_SET_DIS_INFO,
	VI_SDK_SET_PIPE_FRM_SRC,
	VI_SDK_SEND_PIPE_RAW,
	VI_SDK_SET_DEV_TIMING_ATTR,
	VI_SDK_GET_CHN_FRAME,
	VI_SDK_RELEASE_CHN_FRAME,
	VI_SDK_SET_CHN_CROP,
	VI_SDK_GET_CHN_CROP,
	VI_SDK_GET_PIPE_FRAME,
	VI_SDK_RELEASE_PIPE_FRAME,
	VI_SDK_START_SMOOTH_RAWDUMP,
	VI_SDK_STOP_SMOOTH_RAWDUMP,
	VI_SDK_GET_SMOOTH_RAWDUMP,
	VI_SDK_PUT_SMOOTH_RAWDUMP,
	VI_SDK_SET_CHN_ROTATION,
	VI_SDK_SET_CHN_LDC,
	VI_SDK_REG_SYNC_TASK,
	VI_SDK_UNREG_SYNC_TASK,
};

/*
 * Events
 */
enum VI_EVENT {
	VI_EVENT_BASE,
	VI_EVENT_PRE0_SOF,
	VI_EVENT_PRE1_SOF,
	VI_EVENT_PRE0_EOF,
	VI_EVENT_PRE1_EOF,
	VI_EVENT_POST_EOF,
	VI_EVENT_POST1_EOF,
	VI_EVENT_ISP_PROC_READ,
	VI_EVENT_AWB0_DONE,
	VI_EVENT_AWB1_DONE,
	VI_EVENT_MAX,
};

struct _vi_sdk_cfg {
	__s32 dev;
	__s32 pipe;
	__s32 chn;
	void *ptr;
	__s32 val;
};

struct vi_ext_control {
	__u32 id;
	__u32 sdk_id;
	struct _vi_sdk_cfg sdk_cfg;
	union {
		__s32 value;
		__s64 value64;
		void *ptr;
	};
} __attribute__ ((packed));

struct vi_plane {
	__u64 addr;
};

/*
 * @index:
 * @length: length of planes
 * @planes: to describe buf
 * @reserved
 */
struct vi_buffer {
	__u32 index;
	__u32 length;
	struct vi_plane planes[3];
	__u32 reserved;
};

struct vi_event {
	__u32			dev_id;
	__u32			type;
	__u32			frame_sequence;
	struct timespec		timestamp;
};

#define MO_TBL_SIZE 256

struct mlv_info_s {
	__u8	sensor_num;
	__u32	frm_num;
	__u8	mlv;
	__u8	mtable[MO_TBL_SIZE];
};

struct crop_size_s {
	__u16 start_x;
	__u16 start_y;
	__u16 end_x;
	__u16 end_y;
};

struct dis_info_s {
	__u8   sensor_num;
	__u32  frm_num;
	struct crop_size_s dis_i;
};

struct vi_chn_rot_cfg {
	VI_CHN ViChn;
	ROTATION_E enRotation;
};

struct vi_chn_ldc_cfg {
	VI_CHN ViChn;
	ROTATION_E enRotation;
	VI_LDC_ATTR_S stLDCAttr;
	CVI_U64 meshHandle;
};

int vi_open(void);
int vi_close(void);
long vi_ioctl(u_int cmd, void *arg);
int vi_core_init(void);
int vi_core_deinit(void);

#ifdef __cplusplus
	}
#endif

#endif /* __U_VI_UAPI_H__ */

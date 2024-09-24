#ifndef __U_VI_UAPI_H__
#define __U_VI_UAPI_H__

#ifdef __cplusplus
	extern "C" {
#endif

#ifdef CONFIG_KERNEL_RHINO
#include <time.h>
#else
#include <linux/types.h>
#endif
#include <cvi_comm_vi.h>
#include <cvi_comm_sys.h>

#define VI_IOC_BASE		0x20

#define VI_IOC_G_CTRL		VI_IOC_BASE
#define VI_IOC_S_CTRL		(VI_IOC_BASE + 1)

/*
 * Enumeration of IOCTL commands for video input operations.
 * These commands control various features and settings of the video input system.
 */
enum VI_IOCTL {
	VI_IOCTL_ONLINE,                /* Enable online mode */
	VI_IOCTL_HDR,                   /* Enable HDR (High Dynamic Range) */
	VI_IOCTL_3DNR,                  /* Enable 3D Noise Reduction */
	VI_IOCTL_TILE,                  /* Enable tiling */
	VI_IOCTL_COMPRESS_EN,           /* Enable compression */
	VI_IOCTL_STS_MEM,               /* Status memory operation */
	VI_IOCTL_STS_GET,               /* Get status */
	VI_IOCTL_STS_PUT,               /* Put status */
	VI_IOCTL_POST_STS_GET,          /* Get post status */
	VI_IOCTL_POST_STS_PUT,          /* Put post status */
	VI_IOCTL_USR_PIC_CFG,           /* User picture configuration */
	VI_IOCTL_USR_PIC_ONOFF,         /* Enable/disable user picture */
	VI_IOCTL_USR_PIC_PUT,           /* Put user picture */
	VI_IOCTL_AE_CFG,                /* Auto Exposure configuration */
	VI_IOCTL_AWB_CFG,               /* Auto White Balance configuration */
	VI_IOCTL_AF_CFG,                /* Auto Focus configuration */
	VI_IOCTL_USR_PIC_TIMING,        /* User picture timing configuration */
	VI_IOCTL_GET_LSC_PHY_BUF,       /* Get LSC physical buffer */
	VI_IOCTL_CSIBDG_CFG,            /* CSI bridge configuration */
	VI_IOCTL_GET_TUN_ADDR,          /* Get tuning address */
	VI_IOCTL_SET_SNR_INFO,          /* Set sensor information */
	VI_IOCTL_SET_SNR_CFG_NODE,      /* Set sensor configuration node */
	VI_IOCTL_GET_PIPE_DUMP,         /* Get pipe dump */
	VI_IOCTL_PUT_PIPE_DUMP,         /* Put pipe dump */
	VI_IOCTL_SET_RGBMAP_IDX,        /* Set RGB map index */
	VI_IOCTL_HDR_DETAIL_EN,         /* Enable HDR details */
	VI_IOCTL_YUV_BYPASS_PATH,       /* Enable YUV bypass path */
	VI_IOCTL_BE_ONLINE,             /* Enable backend online mode */
	VI_IOCTL_SUBLVDS_PATH,          /* Set sub LVDS path */
	VI_IOCTL_GET_IP_INFO,           /* Get IP information */
	VI_IOCTL_TRIG_PRERAW,           /* Trigger pre-raw data */
	VI_IOCTL_SET_PROC_CONTENT,       /* Set processing content */
	VI_IOCTL_SC_ONLINE,             /* Enable smart capture online mode */
	VI_IOCTL_MMAP_GRID_SIZE,        /* Get memory map grid size */
	VI_IOCTL_RGBIR,                 /* RGBIR configuration */
	VI_IOCTL_AWB_STS_GET,           /* Get AWB status */
	VI_IOCTL_AWB_STS_PUT,           /* Put AWB status */
	VI_IOCTL_GET_FSWDR_PHY_BUF,     /* Get FSWDR physical buffer */
	VI_IOCTL_GET_SCENE_INFO,        /* Get scene information */
	VI_IOCTL_CLK_CTRL,              /* Clock control */
	VI_IOCTL_GET_BUF_SIZE,          /* Get buffer size */
	VI_IOCTL_SET_DMA_BUF_INFO,      /* Set DMA buffer information */
	VI_IOCTL_ENQ_BUF,               /* Enqueue buffer */
	VI_IOCTL_DQEVENT,               /* Dequeue event */
	VI_IOCTL_START_STREAMING,       /* Start streaming */
	VI_IOCTL_STOP_STREAMING,        /* Stop streaming */
	VI_IOCTL_SET_SLICE_BUF_EN,      /* Enable slice buffer */
	VI_IOCTL_GET_CLUT_TBL_IDX,      /* Get CLUT table index */
	VI_IOCTL_SDK_CTRL,              /* SDK control commands */
	VI_IOCTL_GET_RGBMAP_LE_PHY_BUF, /* Get RGB map left physical buffer */
	VI_IOCTL_GET_RGBMAP_SE_PHY_BUF, /* Get RGB map right physical buffer */
	VI_IOCTL_GET_SHARE_MEM,         /* Get shared memory */
	VI_IOCTL_ENABLE_PATTERN,         /* Enable pattern generation */
	VI_IOCTL_QUERY_ISP_EVENT,       /* Query ISP event */
	VI_IOCTL_CHECK_ISP_EVENT,       /* Check ISP event status */
	VI_IOCTL_TUNING_PARAM,          /* Tuning parameters */
	VI_IOCTL_MAX,                   /* Maximum value for enumeration */
};

/*
 * Enumeration of SDK control commands for video input operations.
 * These commands manage device attributes, pipe operations, and channel settings.
 */
enum VI_SDK_CTRL {
	VI_SDK_SET_DEVATTR,             /* Set device attributes */
	VI_SDK_GET_DEVATTR,             /* Get device attributes */
	VI_SDK_ENABLE_DEV,              /* Enable device */
	VI_SDK_DISABLE_DEV,             /* Disable device */
	VI_SDK_CREATE_PIPE,             /* Create a processing pipe */
	VI_SDK_DESTROY_PIPE,            /* Destroy a processing pipe */
	VI_SDK_SET_PIPEATTR,            /* Set pipe attributes */
	VI_SDK_GET_PIPEATTR,            /* Get pipe attributes */
	VI_SDK_START_PIPE,              /* Start processing pipe */
	VI_SDK_STOP_PIPE,               /* Stop processing pipe */
	VI_SDK_SET_CHNATTR,             /* Set channel attributes */
	VI_SDK_GET_CHNATTR,             /* Get channel attributes */
	VI_SDK_ENABLE_CHN,              /* Enable channel */
	VI_SDK_DISABLE_CHN,             /* Disable channel */
	VI_SDK_SET_MOTION_LV,           /* Set motion level */
	VI_SDK_ENABLE_DIS,              /* Enable digital image stabilization */
	VI_SDK_DISABLE_DIS,             /* Disable digital image stabilization */
	VI_SDK_SET_DIS_INFO,            /* Set digital image stabilization information */
	VI_SDK_SET_PIPE_FRM_SRC,        /* Set pipe frame source */
	VI_SDK_SEND_PIPE_RAW,           /* Send raw data to pipe */
	VI_SDK_SET_DEV_TIMING_ATTR,     /* Set device timing attributes */
	VI_SDK_GET_DEV_TIMING_ATTR,     /* Get device timing attributes */
	VI_SDK_GET_CHN_FRAME,           /* Get channel frame */
	VI_SDK_RELEASE_CHN_FRAME,       /* Release channel frame */
	VI_SDK_SET_CHN_CROP,            /* Set channel crop parameters */
	VI_SDK_GET_CHN_CROP,            /* Get channel crop parameters */
	VI_SDK_GET_PIPE_FRAME,           /* Get pipe frame */
	VI_SDK_RELEASE_PIPE_FRAME,       /* Release pipe frame */
	VI_SDK_START_SMOOTH_RAWDUMP,    /* Start smooth raw dump */
	VI_SDK_STOP_SMOOTH_RAWDUMP,     /* Stop smooth raw dump */
	VI_SDK_GET_SMOOTH_RAWDUMP,       /* Get smooth raw dump */
	VI_SDK_PUT_SMOOTH_RAWDUMP,       /* Put smooth raw dump */
	VI_SDK_SET_CHN_ROTATION,        /* Set channel rotation */
	VI_SDK_SET_CHN_LDC,             /* Set lens distortion correction for channel */
	VI_SDK_GET_DEV_STATUS,          /* Get device status */
	VI_SDK_ATTACH_VB_POOL,          /* Attach video buffer pool */
	VI_SDK_DETACH_VB_POOL,          /* Detach video buffer pool */
};

/*
 * Enumeration of events for video input operations.
 * These events represent various synchronization points and processing states.
 */
enum VI_EVENT {
	VI_EVENT_BASE,                  /* Base event */
	VI_EVENT_PRE0_SOF,             /* Pre-frame 0 start of frame event */
	VI_EVENT_PRE1_SOF,             /* Pre-frame 1 start of frame event */
	VI_EVENT_PRE2_SOF,             /* Pre-frame 2 start of frame event */
	VI_EVENT_VIRT_PRE3_SOF,        /* Virtual pre-frame 3 start of frame event */
	VI_EVENT_PRE0_EOF,             /* Pre-frame 0 end of frame event */
	VI_EVENT_PRE1_EOF,             /* Pre-frame 1 end of frame event */
	VI_EVENT_PRE2_EOF,             /* Pre-frame 2 end of frame event */
	VI_EVENT_VIRT_PRE3_EOF,        /* Virtual pre-frame 3 end of frame event */
	VI_EVENT_POST_EOF,             /* Post-frame end of frame event */
	VI_EVENT_POST1_EOF,            /* Post-frame 1 end of frame event */
	VI_EVENT_POST2_EOF,            /* Post-frame 2 end of frame event */
	VI_EVENT_VIRT_POST3_EOF,       /* Virtual post-frame 3 end of frame event */
	VI_EVENT_ISP_PROC_READ,        /* ISP processing read event */
	VI_EVENT_AWB0_DONE,            /* AWB (Auto White Balance) 0 completion event */
	VI_EVENT_AWB1_DONE,            /* AWB (Auto White Balance) 1 completion event */
	VI_EVENT_MAX,                  /* Maximum value for enumeration */
};

/*
 * Structure representing video input SDK configuration.
 * Contains device, pipe, channel information, and a pointer for additional data.
 */
struct _vi_sdk_cfg {
	__s32 dev;          /* Device identifier */
	__s32 pipe;         /* Pipe identifier */
	__s32 chn;          /* Channel identifier */
	void *ptr;          /* Pointer to additional configuration data */
	__s32 val;          /* Additional value for configuration */
};

/*
 * Structure for extended control operations in the video input SDK.
 * Contains an ID, SDK ID, configuration, and a union for different value types.
 */
struct vi_ext_control {
	__u32 id;                     /* Control ID */
	__u32 sdk_id;                 /* SDK ID for extended control */
	struct _vi_sdk_cfg sdk_cfg;   /* SDK configuration structure */
	union {
		__s32 value;              /* 32-bit value */
		__s64 value64;            /* 64-bit value */
		void *ptr;                /* Pointer to additional data */
	};
} __attribute__ ((packed));       /* Ensure no padding in structure */

/*
 * Structure representing a memory plane for video input buffers.
 * Contains the address of the memory plane.
 */
struct vi_plane {
	__u64 addr;                   /* Address of the memory plane */
};

/*
 * Structure representing a video input buffer.
 * Contains index, length, planes, and reserved space.
 */
struct vi_buffer {
	__u32 index;                  /* Buffer index */
	__u32 length;                 /* Length of the buffer */
	struct vi_plane planes[3];    /* Array of planes (up to 3) */
	__u32 reserved;               /* Reserved for future use */
};

/*
 * Structure representing an event in the video input system.
 * Contains device ID, event type, frame sequence, and timestamp.
 */
struct vi_event {
	__u32 dev_id;                 /* Device identifier */
	__u32 type;                   /* Event type */
	__u32 frame_sequence;         /* Frame sequence number */
	struct timespec timestamp;    /* Timestamp of the event */
};

/*
 * Size of motion table for MLV (Multi-Layer Video) information.
 */
#define MO_TBL_SIZE 2048

/*
 * Structure representing MLV information.
 * Contains sensor number, frame number, and a motion table.
 */
struct mlv_info_s {
	__u8 sensor_num;              /* Number of sensors */
	__u32 frm_num;                /* Frame number */
	__u8 mlv;                     /* MLV flag */
	__u8 mtable[MO_TBL_SIZE];     /* Motion table */
};

/*
 * Structure representing crop size information.
 * Contains start and end coordinates for cropping.
 */
struct crop_size_s {
	__u16 start_x;                /* X coordinate of the start point */
	__u16 start_y;                /* Y coordinate of the start point */
	__u16 end_x;                  /* X coordinate of the end point */
	__u16 end_y;                  /* Y coordinate of the end point */
};

/*
 * Structure representing digital image stabilization (DIS) information.
 * Contains sensor number, frame number, and crop size for stabilization.
 */
struct dis_info_s {
	__u8 sensor_num;              /* Number of sensors */
	__u32 frm_num;                /* Frame number */
	struct crop_size_s dis_i;     /* Crop size information for DIS */
};

/*
 * Structure representing channel rotation configuration.
 * Contains channel identifier and rotation setting.
 */
struct vi_chn_rot_cfg {
	VI_CHN ViChn;                 /* Channel identifier */
	ROTATION_E enRotation;        /* Rotation setting */
};

/*
 * Structure representing lens distortion correction (LDC) configuration for a channel.
 * Contains channel identifier, rotation setting, LDC attributes, and mesh handle.
 */
struct vi_chn_ldc_cfg {
	VI_CHN ViChn;                 /* Channel identifier */
	ROTATION_E enRotation;        /* Rotation setting */
	VI_LDC_ATTR_S stLDCAttr;      /* LDC attributes */
	CVI_U64 meshHandle;           /* Mesh handle for LDC */
};

/*
 * Structure representing video buffer pool configuration.
 * Contains pipe and channel identifiers, and buffer pool information.
 */
struct vi_vb_pool_cfg {
	VI_PIPE ViPipe;               /* Pipe identifier */
	VI_CHN ViChn;                 /* Channel identifier */
	__u32 VbPool;                 /* Video buffer pool identifier */
};


#ifdef CONFIG_KERNEL_RHINO
int vi_open(void);
int vi_close(void);
long vi_ioctl(u_int cmd, void *arg);
int vi_core_init(void);
int vi_core_deinit(void);
#endif

#ifdef __cplusplus
	}
#endif

#endif /* __U_VI_UAPI_H__ */

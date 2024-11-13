/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: include/cvi_comm_sys.h
 * Description:
 *   The common sys type defination.
 */

#ifndef __CVI_COMM_SYS_H__
#define __CVI_COMM_SYS_H__

#include <cvi_comm_video.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#define BIND_DEST_MAXNUM 32
#define BIND_NODE_MAXNUM 64

/**
 * total_size: total size for ion
 * free_size: total free size for ion
 * max_avail_size: max available size for ion
 */
typedef struct _ION_MEM_STATE_S {
	CVI_U64 total_size;
	CVI_U64 free_size;
	CVI_U64 max_avail_size;
} ION_MEM_STATE_S;

/**
 * u32Num: Number of bound destinations.
 * astMmfChn: Channel structure array for binding purposes.
*/
typedef struct _MMF_BIND_DEST_S {
	CVI_U32   u32Num;
	MMF_CHN_S astMmfChn[BIND_DEST_MAXNUM];
} MMF_BIND_DEST_S;

/**
 * bUsed: is bind node used.
 * src: bind node src chn.
 * dsts: bind node dest chns.
*/
typedef struct _BIND_NODE_S {
	CVI_BOOL bUsed;
	MMF_CHN_S src;
	MMF_BIND_DEST_S dsts;
} BIND_NODE_S;

/**
 * VI_OFFLINE_VPSS_OFFLINE: vi offline vpss offline.
 * VI_OFFLINE_VPSS_ONLINE: vi offline vpss online.
 * VI_ONLINE_VPSS_OFFLINE: vi online vpss offline.
 * VI_ONLINE_VPSS_ONLINE: vi online vpss online.
 * VI_BE_OFL_POST_OL_VPSS_OFL: vi be offline post online vpss offline.
 * VI_BE_OFL_POST_OFL_VPSS_OFL: vi be offline post offline vpss offline.
 * VI_BE_OL_POST_OFL_VPSS_OFL: vi be online post offline vpss offline.
 * VI_BE_OL_POST_OL_VPSS_OFL: vi be online post online vpss offline.
*/
typedef enum _VI_VPSS_MODE_E {
	VI_OFFLINE_VPSS_OFFLINE = 0,
	VI_OFFLINE_VPSS_ONLINE,
	VI_ONLINE_VPSS_OFFLINE,
	VI_ONLINE_VPSS_ONLINE,
	VI_BE_OFL_POST_OL_VPSS_OFL,
	VI_BE_OFL_POST_OFL_VPSS_OFL,
	VI_BE_OL_POST_OFL_VPSS_OFL,
	VI_BE_OL_POST_OL_VPSS_OFL,
	VI_VPSS_MODE_BUTT
} VI_VPSS_MODE_E;

/**
 * aenMode: vi_vpss modes.
*/
typedef struct _VI_VPSS_MODE_S {
	VI_VPSS_MODE_E aenMode[VI_MAX_PIPE_NUM];
} VI_VPSS_MODE_S;

/**
 * VPSS_MODE_SINGLE: VPSS works as a single hardware entity
 * VPSS_MODE_DUAL: VPSS works in two hardware entities
 * VPSS_MODE_RGNEX: not supported now.
*/
typedef enum _VPSS_MODE_E {
	VPSS_MODE_SINGLE = 0,
	VPSS_MODE_DUAL,
	VPSS_MODE_RGNEX,
	VPSS_MODE_BUTT
} VPSS_MODE_E;

/**
 * VPSS_INPUT_MEM: VPSS input is DDR.
 * VPSS_INPUT_ISP: VPSS works online, and input is ISP.
*/
typedef enum _VPSS_INPUT_E {
	VPSS_INPUT_MEM = 0,
	VPSS_INPUT_ISP,
	VPSS_INPUT_BUTT
} VPSS_INPUT_E;

/*
 * enMode: decide vpss work as 1/2 device.
 * aenInput: decide the input of each vpss device.
 * ViPipe: only meaningful if enInput is ISP.
 */
typedef struct _VPSS_MODE_S {
	VPSS_MODE_E enMode;
	VPSS_INPUT_E aenInput[VPSS_IP_NUM];
	VI_PIPE ViPipe[VPSS_IP_NUM];
} VPSS_MODE_S;

/**
 * paddr_src: physical address of src mem.
 * paddr_dst: physical address of dst mem.
 * w_bytes: width of mem.
 * h: height of mem.
 * stride_bytes_src: row span of src mem.
 * stride_bytes_dst: row span of dst mem.
*/
typedef struct _CVI_TDMA_2D_S {
	uint64_t paddr_src;
	uint64_t paddr_dst;
	uint32_t w_bytes;
	uint32_t h;
	uint32_t stride_bytes_src;
	uint32_t stride_bytes_dst;
} CVI_TDMA_2D_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif  /* __CVI_COMM_SYS_H__ */


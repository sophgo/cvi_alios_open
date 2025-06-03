#ifndef __CVI_VENCCFG_H_
#define __CVI_VENCCFG_H_
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "cvi_comm_video.h"
#include "cvi_comm_sys.h"
#include "cvi_comm_venc.h"
//#include "cvi_venc.h"
//多级别结构 确定使用的多级数组加number作标定 (多级限定场景 串行结构 不允许存在同一级别有多个多级存在) 大结构必须精简尽量使用指针

typedef struct _PARAM_GOP_PARAM_S {
    CVI_U16 u16gopMode;
    CVI_S8 s8IPQpDelta;/* RW; Range:[-10,30]; QP variance between P frame and I frame */
} PARAM_GOP_PARAM_S;

typedef struct _PARAM_RC_PARAM_S {
    CVI_U16 u16Gop;
    CVI_U8  u8SrcFrameRate;
    CVI_U8  u8DstFrameRate;
    CVI_U8  u8Qfactor;/* RW; Range:[1,99]; Qfactor value, 50 = user q-table */
    CVI_U16 u16BitRate;
    CVI_U32 u32MaxBitRate;
    CVI_U8 u8VariFpsEn;
    CVI_U8 u8StartTime;
    CVI_U16 u16RcMode;
    CVI_U16 u16FirstFrmstartQp;
    CVI_U16 u16InitialDelay; // RW; Range:[10, 3000]; Rate control initial delay (ms).
    CVI_U16 u16ThrdLv; /*RW; Range:[0, 4]; Mad threshold for controlling the macroblock-level bit rate */
    CVI_U16 u16BgDeltaQp; /* RW; Range:[-51, 51]; Backgournd Qp Delta */
    CVI_U8 u8MinIprop;
    CVI_U8 u8MaxIprop;
    CVI_U8 u8MinIqp;//0-51
    CVI_U8 u8MaxIqp;//0-51
    CVI_U8 u8MinQp;//0-51
    CVI_U8 u8MaxQp;//0-51
    CVI_U8 u8MaxReEncodeTimes; /* RW; Range:[0, 3]; Range:max number of re-encode times.*/
	CVI_U8 u8QpMapEn; /* RW; Range:[0, 1]; enable qpmap.*/
    CVI_U8 u8ChangePos; //VBR使用
    CVI_BOOL bSingleCore;
    CVI_U32 u32FixedIQp;
    CVI_U32 u32FixedPQp;
    CVI_U32 u32Duration;
    CVI_S32 s32MinStillPercent;
    CVI_U32 u32MaxStillQP;
    CVI_U32 u32MinStillPSNR;
    CVI_U32 u32MotionSensitivity;
    CVI_S32	s32AvbrFrmLostOpen;
    CVI_S32 s32AvbrFrmGap;
    CVI_S32 s32AvbrPureStillThr;
    CVI_S16 s16Quality;
    CVI_S16 s16MCUPerECS;
	CVI_U8 u8MinQfactor; /* RW; Range:[1,99]; minQfactor value*/
	CVI_U8 u8MaxQfactor; /* RW; Range:[1,99]; maxQfactor value*/
} PARAM_RC_PARAM_S;

typedef struct _PARAM_ROI_PARAM_S {
    CVI_U32 u8Index; /* RW; Range:[0, 7]; Index of an ROI. The system supports indexes ranging from 0 to 7 */
    CVI_BOOL bEnable; /* RW; Range:[0, 1]; Whether to enable this ROI */
    VENC_CHN VencChn;
    CVI_BOOL bAbsQp; // RW; Range:[0, 1]; QP mode of an ROI. CVI_FALSE: relative QP. CVI_TRUE: absolute QP
    CVI_S16 s32Qp; /* RW; Range:[-51, 51]; QP value,only relative mode can QP value less than 0. */
    CVI_S32 s32X;
    CVI_S32 s32Y;
    CVI_U32 u32Width;
    CVI_U32 u32Height;
} PARAM_ROI_PARAM_S;


typedef struct _PARAM_VENCCHN_PARAM_S {
    //基本属性
    CVI_BOOL bStart;
    CVI_U8 u8VencChn;
    CVI_U16 u16EnType;//PAYLOAD_TYPE_E
    CVI_U8 u8Profile;
    CVI_U16 u16Width;
    CVI_U16 u16Height;
    CVI_U32 u32BitStreamBufSize;
    CVI_U8 u8EsBufQueueEn; ///< Use es buffer queue
    CVI_U8 u8EntropyEncModeI;//encode 编码属性 base/high
    CVI_U8 u8EntropyEncModeP;//encode 编码属性 base/high
    CVI_BOOL bEnable;
    CVI_U32 StreamTo; //send to RTSP:0x01; save to falsh:0x10; RTSP and flash both:0x11;
    CVI_U32 enBindMode; //0: VENC_BIND_DISABLE, 2: VENC_BIND_VPSS
    MMF_CHN_S astChn[2];
}PARAM_VENCCHN_PARAM_S;

typedef struct PARAM_VENC_CHN_CFG_T {
    PARAM_VENCCHN_PARAM_S stChnParam;
    PARAM_RC_PARAM_S stRcParam;
    PARAM_GOP_PARAM_S stGopParam;
} PARAM_VENC_CHN_CFG_S;


typedef struct _PARAM_VENC_CFG_S {
    CVI_S32 s32VencChnCnt;
    PARAM_VENC_CHN_CFG_S * pstVencChnCfg;
    CVI_S8 s8RoiNumber;//-1代表没有ROI range:[0-7]
    PARAM_ROI_PARAM_S * pstRoiParam;
} PARAM_VENC_CFG_S;



#endif

/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2021. All rights reserved.
 *
 * File Name: include/af_motor.h
 * Description:
 *
 */

#ifndef __AF_MOTOR_H__
#define __AF_MOTOR_H__

#include "cvi_comm_isp.h"
#include "cvi_comm_3a.h"
#include "cvi_af_comm.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

// #define ENABLE_AF_LIB (1)
//The customer can deisgin the control of the motor by themself,
//and sophgo also provides the public practice for customer reference
//please contact sophgo get motor ko source code
//design it please flow cb func format
//cb sample start
CVI_S32 SAMPLE_COMM_ISP_Motor_SetFocusInCb(VI_PIPE ViPipe, CVI_U8 step);
CVI_S32 SAMPLE_COMM_ISP_Motor_SetFocusOutCb(VI_PIPE ViPipe, CVI_U8 step);
CVI_S32 SAMPLE_COMM_ISP_Motor_SetZoomSpeedCb(VI_PIPE ViPipe, CVI_U8 speed);
CVI_S32 SAMPLE_COMM_ISP_Motor_SetFocusSpeedCb(VI_PIPE ViPipe, CVI_U8 speed);
CVI_S32 SAMPLE_COMM_ISP_Motor_SetZoomInCb(VI_PIPE ViPipe, CVI_U8 step);
CVI_S32 SAMPLE_COMM_ISP_Motor_SetZoomOutCb(VI_PIPE ViPipe, CVI_U8 step);
CVI_S32 SAMPLE_COMM_ISP_Motor_GetLensInfoCb(VI_PIPE ViPipe, ISP_AF_LEN_INFO_S *info);
CVI_S32 SAMPLE_COMM_ISP_Motor_SetZoomAndFocusCb(VI_PIPE ViPipe, AF_DIRECTION eDirz, AF_DIRECTION eDirf,
													CVI_U8 zoomStep, CVI_U8 focusStep);
//cb sample end

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* __AF_MOTOR_H__ */

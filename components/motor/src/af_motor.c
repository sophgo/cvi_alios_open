#include "cvi_af.h"
#include "cvi_ae.h"
#include "cvi_isp.h"
#include "af_motor.h"
#include "motor.h"


CVI_S32 SAMPLE_COMM_ISP_Motor_SetFocusInCb(VI_PIPE ViPipe, CVI_U8 step)
{
	return motor_cw(MOTOR_TYPE, step);
}

CVI_S32 SAMPLE_COMM_ISP_Motor_SetFocusOutCb(VI_PIPE ViPipe, CVI_U8 step)
{
	return motor_ccw(MOTOR_TYPE, step);
}

CVI_S32 SAMPLE_COMM_ISP_Motor_SetZoomSpeedCb(VI_PIPE ViPipe, CVI_U8 speed)
{
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_ISP_Motor_SetFocusSpeedCb(VI_PIPE ViPipe, CVI_U8 speed)
{
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_ISP_Motor_SetZoomInCb(VI_PIPE ViPipe, CVI_U8 step)
{

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_ISP_Motor_SetZoomOutCb(VI_PIPE ViPipe, CVI_U8 step)
{

	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_ISP_Motor_SetZoomAndFocusCb(VI_PIPE ViPipe, AF_DIRECTION eDirz, AF_DIRECTION eDirf,
												CVI_U8 zoomStep, CVI_U8 focusStep)
{
	if (eDirf == AF_DIR_NEAR) {
		motor_ccw(MOTOR_TYPE, focusStep);
	} else {
		motor_cw(MOTOR_TYPE, focusStep);
	}
	return CVI_SUCCESS;
}

CVI_S32 SAMPLE_COMM_ISP_Motor_GetLensInfoCb(VI_PIPE ViPipe, ISP_AF_LEN_INFO_S *info)
{
	info->zoom_time_cost_one_step = 1;
	info->focus_time_cost_one_step = 1;
	info->zoom_backlash = 0;
	info->focus_backlash = 0;
	info->motor_type = AF_MOTOR_VCM;
	info->zoom_max_step = 255;
	info->focus_max_step = 255;
	info->zoom_offset = 0;
	info->focus_offset = 0;
	info->zoom_range = 0;
	info->focus_range = 1023;

	return CVI_SUCCESS;
}

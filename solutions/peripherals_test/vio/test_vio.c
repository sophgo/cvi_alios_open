#include <aos/kernel.h>
#include <stdio.h>
#include <ulog/ulog.h>
#include <unistd.h>
#include <aos/cli.h>
#include "fatfs_vfs.h"
#include "vfs.h"

#include "cvi_vi.h"
#include "media_video.h"
#include "cvi_param.h"
#include "common_vi.h"

static int vio_init_flag;

static int test_vio_init(void)
{
	//return CVI_SUCCESS;
	PARAM_SYS_CFG_S *pstSysCtx = PARAM_getSysCtx();
	PARAM_VI_CFG_S *pstViCfg = PARAM_getViCtx();
	PARAM_VPSS_CFG_S *pstVpssCtx = PARAM_getVpssCtx();
	PARAM_VO_CFG_S *pstVoCtx = PARAM_getVoCtx();

	//media vi init
	MEDIA_CHECK_RET(MEDIA_VIDEO_SysVbInit(pstSysCtx), "MEDIA_VIDEO_SysVbInit failed");
	MEDIA_CHECK_RET(MEDIA_VIDEO_ViInit(pstViCfg), "MEDIA_VIDEO_ViInit failed");
	MEDIA_CHECK_RET(MEDIA_VIDEO_VpssInit(pstVpssCtx), "MEDIA_VIDEO_VpssInit failed");
	MEDIA_CHECK_RET(MEDIA_VIDEO_VoInit(pstVoCtx), "MEDIA_VIDEO_VoInit failed");

	return CVI_SUCCESS;
}

static int test_vio_deinit(void)
{
	PARAM_VI_CFG_S *pstViCfg = PARAM_getViCtx();
	PARAM_VO_CFG_S *pstVoCtx = PARAM_getVoCtx();
	PARAM_VPSS_CFG_S *pstVpssCtx = PARAM_getVpssCtx();

	MEDIA_CHECK_RET(MEDIA_VIDEO_VoDeinit(pstVoCtx), "MEDIA_VIDEO_VoDeinit failed");
	MEDIA_CHECK_RET(MEDIA_VIDEO_VpssDeinit(pstVpssCtx), "MEDIA_VIDEO_VpssDeinit failed");
	MEDIA_VIDEO_ViDeinit(pstViCfg);
	MEDIA_VIDEO_SysVbDeinit();
	return CVI_SUCCESS;
}

void start_vio(int32_t argc, char **argv)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (vio_init_flag == 1) {
		aos_cli_printf("vio already init \r\n");
		return;
	}

	s32Ret = test_vio_init();
	if (s32Ret != CVI_SUCCESS) {
		aos_cli_printf("vio_init: fail\r\n");
		return;
	}
	vio_init_flag = 1;
}

void stop_vio(int32_t argc, char **argv)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	if (vio_init_flag != 1) {
		aos_cli_printf("vio not init \r\n");
		return;
	}

	s32Ret = test_vio_deinit();
	if (s32Ret != CVI_SUCCESS) {
		aos_cli_printf("vio_deinit: fail\r\n");
		return;
	}
	vio_init_flag = 0;
}

ALIOS_CLI_CMD_REGISTER(start_vio, start_vio, start sensor to panel);
ALIOS_CLI_CMD_REGISTER(stop_vio, stop_vio, stop sensor to panel);

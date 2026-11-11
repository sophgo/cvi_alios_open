/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */
#include <aos/kernel.h>
#include <stdio.h>
#include <ulog/ulog.h>
#include <unistd.h>
#include "common_yocsystem.h"
#include "platform.h"
#include "custom_event.h"
#include "cvi_param.h"
#include "app_param.h"
#include <drv/pwm.h>
#include "media_driver.h"
#include "rpmsg_service.h"
#include "cvi_msg_server.h"
#include "media_video.h"
#include "module.h"
#ifdef CONFIG_AUD_DRV_SEL
#include "media_audio.h"
#endif
#if CONFIG_PQTOOL_SUPPORT == 1
#include "cvi_ispd2.h"
#endif

#define TAG "app"

int main(int argc, char *argv[])
{
	//board pinmux init
	PLATFORM_IoInit();
	PLATFORM_PanelInit();

	YOC_SYSTEM_Init();
	//cli and ulog init
	YOC_SYSTEM_ToolInit();

	//rpmsg init
	cvi_rpmsg_service_start();

	//driver init
	module_init();

	//msg server start
	CVI_MSG_Init();

	//Fs init
	//YOC_SYSTEM_FsVfsInit();

	//load cfg
	PARAM_LoadCfg();

#ifdef CONFIG_RTOS_PARSE_PARAM
	//APP_PARAM_Parse();
#endif

#ifdef CONFIG_RTOS_INIT_MEDIA
	//custom_evenet_pre
	MEDIA_VIDEO_Init(0);
	printf("MEDIA_VIDEO_Init ...\n");
#endif
	//start_pwm();
#ifdef CONFIG_AUD_DRV_SEL
	//audio driver init
	MEDIA_AUDIO_Init();
	printf("MEDIA_AUDIO_Init ...\n");
#endif

	LOGI(TAG, "app start........\n");
	while (1) {
		aos_msleep(3000);
	};
}

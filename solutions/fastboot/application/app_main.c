/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */
#include <aos/kernel.h>
#include <stdio.h>
#include <ulog/ulog.h>
#include <unistd.h>
#include "common_yocsystem.h"
#include "media_video.h"
#include "media_audio.h"
#include "platform.h"
#include "custom_event.h"
#include "cvi_param.h"
#include "cvi_msg_server.h"
#include "app_param.h"
#include <drv/tick.h>

#include "media_driver.h"
#if CONFIG_PQTOOL_SUPPORT == 1
#include "cvi_ispd2.h"
#endif

#define TAG "app"

int main(int argc, char *argv[])
{
	uint64_t t1, t2, t3, t4, t5, t6, t7;

	t1 = csi_tick_get_us();
	//board pinmux init
	PLATFORM_IoInit();
	PLATFORM_PanelInit();

	YOC_SYSTEM_Init();

	//cli and ulog init
	YOC_SYSTEM_ToolInit();
	t2 = csi_tick_get_us();

	//msg server start
	CVI_MSG_Init();
	t3 = csi_tick_get_us();

	//Fs init
	//YOC_SYSTEM_FsVfsInit();

	//load cfg
	PARAM_LoadCfg();
	t4 = csi_tick_get_us();

	//video driver init
	media_driver_init();
	t5 = csi_tick_get_us();

#ifdef CONFIG_RTOS_PARSE_PARAM
		APP_PARAM_Parse();
#endif

	t6 = csi_tick_get_us();

#ifdef CONFIG_RTOS_INIT_MEDIA
	//start video
	MEDIA_VIDEO_Init(0);
	t7 = csi_tick_get_us();
#endif

#ifdef CONFIG_AUD_DRV_SEL
	//audio driver init
	MEDIA_AUDIO_Init();
#endif

#if (CONFIG_CV184X_SLT)
	MEDIA_VIO_VCODEC_SLT(1);
#endif

	printf("bootup timestamp: t1=%ld(us)\n", t1);
	printf("media init done timestamp: t7=%ld(us)\n", t7);

	printf("\n===The duration of each stage===\n");
	printf("yoc system init: t2-t1=%ld(us)\n", t2 - t1);
	printf("CVI_MSG_Init: t3-t2=%ld(us)\n", t3 - t2);
	printf("PARAM_LoadCfg: t4-t3=%ld(us)\n", t4 - t3);
	printf("media_driver_init: t5-t4=%ld(us)\n", t5 - t4);
	printf("App param parse: t6-t5=%ld(us)\n", t6 - t5);
	printf("MEDIA_VIDEO_Init: t7-t6=%ld(us)\n", t7 - t6);

	while (1) {
		aos_msleep(3000);
	};
}

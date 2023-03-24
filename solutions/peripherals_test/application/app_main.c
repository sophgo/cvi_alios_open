/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */
#include <aos/kernel.h>
#include <stdio.h>
#include <ulog/ulog.h>
#include <unistd.h>
#include <aos/cli.h>
#include "common_yocsystem.h"
#include "media_video.h"
#include "gui_display.h"
#include "media_audio.h"
#include "platform.h"
#include "custom_event.h"
#include "cvi_param.h"
#include "wifi_if.h"
#include "ethernet_init.h"


#define TAG "app"


int main(int argc, char *argv[])
{
	YOC_SYSTEM_Init();
	//board pinmux init
	PLATFORM_IoInit();
	//Fs init
	YOC_SYSTEM_FsVfsInit();
	//load cfg
	PARAM_LoadCfg();
	//media video sys init
	MEDIA_VIDEO_SysInit();
	//network
	#if (CONFIG_APP_ETHERNET_SUPPORT == 1)
	ethernet_init();
	#endif
	#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1)
	APP_WifiInit();
	#endif
	//cli and ulog init
	YOC_SYSTEM_ToolInit();
	LOGI(TAG, "app start........\n");
	// APP_CustomEventStart();
	while (1) {
		aos_msleep(3000);
	};
}

/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */
#include <aos/kernel.h>
#include <stdio.h>
#include <ulog/ulog.h>
#include <unistd.h>
#include "common_yocsystem.h"
#include "media_video.h"
#include "gui_display.h"
#include "media_audio.h"
#include "platform.h"
#include "custom_event.h"
#include "cvi_param.h"
#include "wifi_if.h"
#include "ethernet_init.h"
#include "media_nightvision.h"
#include "yoc/netmgr.h"
#include "uservice/uservice.h"
#include "yoc/netmgr_service.h"

#if CONFIG_PQTOOL_SUPPORT == 1
#include "cvi_ispd2.h"
#include "raw_dump.h"
#endif

#if CONFIG_RNDIS_DEVICE_ETH
#include "drv_usbd_class.h"
#endif

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
	//custom_evenet_pre
	//media video
	MEDIA_VIDEO_Init();
	//media_audio
	MEDIA_AUDIO_Init();

	#if CONFIG_NIGHT_VISION_SUPPORT
	//night_vision
	MEDIA_NightVisionInit();
	#endif
	//network
	#if (CONFIG_APP_ETHERNET_SUPPORT == 1)
	ethernet_init();
	#endif
	#if (CONFIG_APP_WIFI_SUPPORT == 1)
	APP_WifiInit();
	#endif
	//cli and ulog init
	YOC_SYSTEM_ToolInit();

	#if (CONFIG_RNDIS_DEVICE_ETH == 1)
	rndis_device_init();
	#endif

	#if (CONFIG_PQTOOL_SUPPORT == 1)
	usleep(12 * 1000);
	isp_daemon2_init(5566);
	cvi_raw_dump_init();
	#endif
	LOGI(TAG, "app start........\n");
	APP_CustomEventStart();
	while (1) {
		aos_msleep(3000);
	};
}

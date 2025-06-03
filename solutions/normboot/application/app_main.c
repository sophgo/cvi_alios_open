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
#include "cvi_ipcm.h"
#include "ipcm_custom.h"
#include "media_driver.h"
#include "cvi_msg_server.h"
#ifdef CONFIG_AUD_DRV_SEL
#include "media_audio.h"
#endif
#if CONFIG_PQTOOL_SUPPORT == 1
#include "cvi_ispd2.h"
#endif

#define TAG "app"

extern void ipcm_driver_test_init(void);
int main(int argc, char *argv[])
{
	PoolConfig *cust_config;
	//board pinmux init
	PLATFORM_IoInit();

	CVI_IPCM_SetRtosSysBootStat();

	YOC_SYSTEM_Init();
	//cli and ulog init
	YOC_SYSTEM_ToolInit();

	//msg server start
	CVI_MSG_Init();

	//Fs init
	//YOC_SYSTEM_FsVfsInit();

	//load cfg
	PARAM_LoadCfg();

	//video driver init
	media_driver_init();

#ifdef CONFIG_RTOS_PARSE_PARAM
	//APP_PARAM_Parse();
#endif
	cust_config = (PoolConfig *)malloc(sizeof(PoolConfig) + 3 * sizeof(BlockConfig));
	cust_config->num = 3;
	cust_config->blk_conf[0].size = 64;
	cust_config->blk_conf[0].num = 2;
	cust_config->blk_conf[1].size = 128;
	cust_config->blk_conf[1].num = 3;
	cust_config->blk_conf[2].size = 256;
	cust_config->blk_conf[2].num = 4;
    ipcm_cust_srv_init(cust_config);
	free(cust_config);
	cust_config = NULL;
	ipcm_driver_test_init();
	//parse_PqParam();

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
#if 0
	//network
	#if (CONFIG_APP_ETHERNET_SUPPORT == 1)
	ethernet_init();
	#endif
	#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1)
	APP_WifiInit();
	#endif
	//cli and ulog init
	// YOC_SYSTEM_ToolInit();
	#if (CONFIG_PQTOOL_SUPPORT == 1)
	usleep(1000);
	isp_daemon2_init(5566);
	#endif
	LOGI(TAG, "app start........\n");
	APP_CustomEventStart();
#endif
	LOGI(TAG, "app start........\n");
	while (1) {
		aos_msleep(3000);
	};
}

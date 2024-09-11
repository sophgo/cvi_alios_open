/*
 * Copyright (C) 2015-2020 Alibaba Group Holding Limited
 */
#include <aos/kernel.h>
#include <stdio.h>
#include <ulog/ulog.h>
#include <unistd.h>
#include <drv/tick.h>
#include "common_yocsystem.h"
#include "media_video.h"
#include "media_audio.h"

#define TAG "app"


int main(int argc, char *argv[])
{

	YOC_SYSTEM_Init();
	//board pinmux init
	PLATFORM_IoInit();
	//Fs init
	YOC_SYSTEM_FsVfsInit();

	//media video sys init
	MEDIA_VIDEO_SysInit();

	//cli and ulog init
	YOC_SYSTEM_ToolInit();

	LOGI(TAG, "app start........\n");

	while (1) {
		aos_msleep(3000);
	};
}

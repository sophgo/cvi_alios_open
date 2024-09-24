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
#include "app_anonmsg.h"
#include "app_param.h"
#include <drv/pwm.h>

#if CONFIG_PQTOOL_SUPPORT == 1
#include "cvi_ispd2.h"
#endif

extern void ipcm_driver_test_init(void);

#define TAG "app"

#include "cvi_comm_ipcm.h"
#include "cvi_ipcm.h"
#include <console_uart.h>
#include <drv/tick.h>

static int _anon_vuart_process(CVI_VOID *priv, IPCM_ANON_MSG_S *data)
{
	unsigned char port_id, msg_id, data_type;
	unsigned int data_len;
	int ret = 0;

	if (data == NULL) {
		printf("%s fail, handle data is null.\n", __func__);
		return -1;
	}

	port_id = data->u8PortID;
	msg_id = data->u8MsgID;
	data_type = data->u8DataType;
	data_len = data->stData.u32Size;

	if ((port_id == CVI_IPCM_PORT_ANON_VUART) && (msg_id == 0) && (data_type == IPCM_MSG_TYPE_SHM)) {
		console_push_data_to_ringbuffer(data->stData.pData, data_len);
	} else {
		printf("%s port_id:%d msg_id:%d data_type:%d not handled.\n", __func__, port_id, msg_id, data_type);
	}

	return ret;
}

#ifdef CONFIG_RTOS_ANNON_MSG
static int _anon_msg_process(CVI_VOID *priv, IPCM_ANON_MSG_S *data)
{
	if (data == NULL) {
		printf("%s fail, handle data is null.\n", __func__);
		return -1;
	}

	return APP_ANONMSG_process(priv, data);
}
#endif

static int ipcm_anon_init(void)
{
	int ret = 0;

	ret = CVI_IPCM_AnonInit();
	if (ret) {
		printf("%s CVI_IPCM_AnonInit fail:%d.\n", __func__, ret);
		return ret;
	}

	ret = CVI_IPCM_RegisterAnonHandle(CVI_IPCM_PORT_ANON_VUART, _anon_vuart_process, NULL);
	if (ret) {
		printf("%s CVI_IPCM_RegisterAnonHandle(%d) fail:%d.\n", __func__, CVI_IPCM_PORT_ANON_VUART, ret);
		CVI_IPCM_AnonUninit();
		return ret;
	}

#ifdef CONFIG_RTOS_ANNON_MSG
	ret = CVI_IPCM_RegisterAnonHandle(CVI_IPCM_PORT_ANON_MSG, _anon_msg_process, NULL);
	if (ret) {
		printf("%s CVI_IPCM_RegisterAnonHandle(%d) fail:%d.\n", __func__, CVI_IPCM_PORT_ANON_MSG, ret);
		CVI_IPCM_AnonUninit();
		return ret;
	}
#endif

	return ret;
}

static int start_pwm(void)
{
    PARAM_SYS_CFG_S * pstSysCtx = PARAM_getSysCtx();
    if (0 == pstSysCtx->stSwitchCfg.bMipiSwitchEnable) {
        return 0;
    }
	uint32_t bank, channel, period_ns, duty_ns, polarity;
	csi_pwm_t pwm;

	bank = 2;
	channel = 3;
	period_ns = 33333333;
	duty_ns = 16666666;
	polarity = PWM_POLARITY_LOW;

	csi_pwm_init(&pwm, bank);
	csi_pwm_out_stop(&pwm, channel);
	csi_pwm_out_config(&pwm, channel, period_ns, duty_ns,
						polarity ? PWM_POLARITY_HIGH : PWM_POLARITY_LOW);
	csi_pwm_out_start(&pwm, channel);
	return 0;
}

int main(int argc, char *argv[])
{
	//board pinmux init
	PLATFORM_IoInit();

    CVI_IPCM_SetRtosSysBootStat();

	YOC_SYSTEM_Init();

	//cli and ulog init
	YOC_SYSTEM_ToolInit();

	//Fs init
	//YOC_SYSTEM_FsVfsInit();
	//load cfg
	PARAM_LoadCfg();
	//video driver init
	MEDIA_VIDEO_SysInit();
#ifdef CONFIG_AUD_DRV_SEL
	//audio driver init
	MEDIA_AUDIO_Init();
#endif
	ipcm_driver_test_init();

	CVI_MSG_Init();

	ipcm_anon_init();
#ifdef CONFIG_RTOS_PARSE_PARAM
	APP_PARAM_Parse();
#endif
  	start_pwm();
#ifdef CONFIG_RTOS_INIT_MEDIA
	//custom_evenet_pre
	//media video
	MEDIA_VIDEO_Init(0);
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
	while (1) {
		aos_msleep(3000);
	};
}

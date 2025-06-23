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

#define PQ_DEBUG 1

#define CVI_IPCM_ANON_STRESS_TEST_PORT 63

static int _anon_stress_test_process(CVI_VOID *priv, IPCM_ANON_MSG_S *data)
{
	unsigned char port_id, msg_id;
	unsigned int param;
	int ret = 0;

	if (data == NULL) {
		printf("%s fail, handle data is null.\n", __func__);
		return -1;
	}

	port_id = data->u8PortID;
	msg_id = data->u8MsgID;
	param = data->u32Param;

	if (port_id != CVI_IPCM_ANON_STRESS_TEST_PORT) {
		printf("%s port_id:%d msg_id:%d not handled.\n", __func__, port_id, msg_id);
	}

	ret = CVI_IPCM_AnonSendParam(
		CVI_IPCM_ANON_STRESS_TEST_PORT, msg_id + 1, param + 1);
	if (ret) {
		printf("%s CVI_IPCM_AnonSendParam fail:%d.\n", __func__, ret);
	}

	return ret;
}

static int ipcm_stress_test_init(void)
{
	int ret = 0;
	ret = CVI_IPCM_RegisterAnonHandle(CVI_IPCM_ANON_STRESS_TEST_PORT, _anon_stress_test_process, NULL);
	if (ret) {
		printf("%s CVI_IPCM_RegisterAnonHandle(%d) fail:%d.\n", __func__, CVI_IPCM_ANON_STRESS_TEST_PORT, ret);
		return ret;
	}
	return ret;
}

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

#if !CONFIG_CV1811C_JD9165
static int start_pwm(void)
{
    PARAM_SYS_CFG_S * pstSysCtx = PARAM_getSysCtx();
    if (0 == pstSysCtx->stSwitchCfg.bMipiSwitchEnable) {
        return 0;
    }
	uint32_t bank, channel, period_ns, duty_ns, polarity;
	csi_pwm_t pwm;

	bank = 0;
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

static int parse_PqParam()
{
    PARAM_VI_CFG_S * pstViCtx = PARAM_GET_VI_CFG();
    if (pstViCtx == NULL) {
        aos_debug_printf("PARAM_VI_CFG_S is null!\r\n");
        return -1;
    }

    #if PQ_DEBUG
    aos_debug_printf("CVI_IPCM_GetPQBinQddr() 0x%x\r\n", CVI_IPCM_GetPQBinQddr());
    #endif
    PARTITION_CHECK_HAED_S * pstPqParam = (PARTITION_CHECK_HAED_S *)((unsigned long)CVI_IPCM_GetPQBinQddr());
    if (_param_check_head((unsigned char *)pstPqParam) != 0) {
        return -1;
    }
    unsigned char * pPqBuffer = (unsigned char *)pstPqParam + sizeof(PARTITION_CHECK_HAED_S);
    for (unsigned int i = 0; i < pstPqParam->package_number; i++) {
        pstViCtx->pstIspCfg[i / 2].stPQBinDes[i % 2].pIspBinData = pPqBuffer;
        pstViCtx->pstIspCfg[i / 2].stPQBinDes[i % 2].u32IspBinDataLen = pstPqParam->package_length[i];
        #if PQ_DEBUG
        aos_debug_printf("pPqBuffer %p\r\n", pPqBuffer);
        aos_debug_printf("pstViCtx->pstIspCfg[%d].stPQBinDes[%d].pIspBinData %x\r\n", i/2, i %2, *pPqBuffer);
        aos_debug_printf("pstViCtx->pstIspCfg[%d].stPQBinDes[%d].u32IspBinDataLen %d\r\n", i/2, i %2, pstViCtx->pstIspCfg[i / 2].stPQBinDes[i % 2].u32IspBinDataLen);
        #endif
        pPqBuffer += pstPqParam->package_length[i];
    }

    return 0;
}
#endif

int main(int argc, char *argv[])
{
	//board pinmux init
	PLATFORM_IoInit();

    CVI_IPCM_SetRtosSysBootStat();

	YOC_SYSTEM_Init();

	//cli and ulog init
	YOC_SYSTEM_ToolInit();

	CVI_MSG_Init();

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

	ipcm_anon_init();

	ipcm_stress_test_init();

#ifdef CONFIG_RTOS_PARSE_PARAM
	APP_PARAM_Parse();
#endif

#if !CONFIG_CV1811C_JD9165
    parse_PqParam();
#endif
#ifdef CONFIG_RTOS_INIT_MEDIA
	//custom_evenet_pre
	//media video
	MEDIA_VIDEO_Init(0);
#endif
#if !CONFIG_CV1811C_JD9165
  	start_pwm();
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

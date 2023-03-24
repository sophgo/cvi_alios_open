/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: wifi_hostmsgpack.c
 * Description:
 *   ....
 */

#include "wifi_hostmsgpack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wifi_dispatch.h"
#include "cvi_type.h"
#include "wifi_common.h"
#include "wifi_if.h"


#define UPLOADFUN(COMMCMD) UPLOADFUN_##COMMCMD

CVI_S32 WIFI_DATATOPACK(CVI_U8 *DATA, CVI_U16 Datalen, WIFI_MSGPACKINFO_S *Pack)
{
	if (!Pack) {
		printf("%s err\n", __func__);
		return CVI_FAILURE;
	}
	Pack->MultipleData[Pack->MultipleDataNumber].data = DATA;
	Pack->MultipleData[Pack->MultipleDataNumber].Len = Datalen;
	Pack->MultipleData[Pack->MultipleDataNumber].Type = Pack->MultipleDataNumber + 1;
	Pack->MultipleDataNumber++;
	return CVI_SUCCESS;
}

CVI_S32 H2W_PACKMSG_SEND(WIFI_MSGPACKINFO_S *Pack)
{
	if (!Pack) {
		printf("%s not pack err\n", __func__);
		return CVI_FAILURE;
	}
	//结构转字节流
	//计算总长度 申请字节
	//一个Pack = Cmd(1 Byte) + MultipleDataNumber * 3字节（type+len）+Datalen各自
	CVI_U8 *SendData = NULL;
	CVI_U32 SendLen  = 1;
	CVI_U32 SeekLen  = 0;
	CVI_S32 i = 0;
	if (Pack->MultipleData) {
		SendLen += Pack->MultipleDataNumber * 3;
		for (i = 0; i < Pack->MultipleDataNumber; i++) {
			SendLen += Pack->MultipleData[i].Len;
		}
	}
	SendData = (CVI_U8 *)malloc(SendLen);
	//调用sdio发送接口
	if (SendData) {
		//拼接数据
		memcpy(SendData, &Pack->CMD, 1);
		SeekLen += 1;
		if (Pack->MultipleData) {
			for (i = 0; i < Pack->MultipleDataNumber; i++) {
				memcpy(SendData + SeekLen, &Pack->MultipleData[i].Type, 1);
				SeekLen += 1;
				memcpy(SendData + SeekLen, &Pack->MultipleData[i].Len, 2);
				SeekLen += 2;
				if (Pack->MultipleData[i].data) {
					memcpy(SendData + SeekLen, Pack->MultipleData[i].data, Pack->MultipleData[i].Len);
				}
				SeekLen += Pack->MultipleData[i].Len;
			}
		}
		Wifi_DispatchSendMsgData(SendData, SendLen);
		free(SendData);
	}
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_HOST_CMD_GET_MAC(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_HOST_CMD_GET_MAC;
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_HOST_CMD_GET_IP(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_HOST_CMD_GET_IP;
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_HOST_CMD_SET_FILTER(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_HOST_CMD_SET_FILTER;
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_HOST_CMD_GET_RSSI(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_HOST_CMD_GET_RSSI;
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_HOST_CMD_CONNECT(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_HOST_CMD_CONNECT;
	WIFI_DATATOPACK(Info->ConnectInfo.SSID, Info->ConnectInfo.SSIDLEN, &Pack);
	WIFI_DATATOPACK(Info->ConnectInfo.ENC, Info->ConnectInfo.ENCLEN, &Pack);
	WIFI_DATATOPACK(Info->ConnectInfo.KEY, Info->ConnectInfo.KEYLEN, &Pack);
	//WIFI_DATATOPACK(Info->ConnectInfo.BSSID, Info->ConnectInfo.BSSIDLEN, &Pack);
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_HOST_CMD_REPORT_WPA_EVENT(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = W2H_HOST_CMD_REPORT_WPA_EVENT;
	WIFI_DATATOPACK(Info->EventInfo.EventID, Info->EventInfo.EventIDLen, &Pack);
	if (Info->EventInfo.EventData && Info->EventInfo.EventDataLen > 0)
		WIFI_DATATOPACK(Info->EventInfo.EventData, Info->EventInfo.EventDataLen, &Pack);
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_HOST_CMD_START_STA(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_HOST_CMD_START_STA;
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_HOST_CMD_STOP_STA(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_HOST_CMD_STOP_STA;
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_HOST_CMD_CLEAN_WIFI(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_HOST_CMD_CLEAN_WIFI;
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_HOST_CMD_CLEAN_DEVICEINFO(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_HOST_CMD_CLEAN_DEVICEINFO;
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_HOST_CMD_CLEAN_ALL(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_HOST_CMD_CLEAN_ALL;
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_IOT_SET_DEVICEINFO(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_IOT_SET_DEVICEINFO;
	WIFI_DATATOPACK(Info->DeviceInfo.DeviceName, Info->DeviceInfo.DeviceNameLen, &Pack);
	WIFI_DATATOPACK(Info->DeviceInfo.Device_Secret, Info->DeviceInfo.Device_SecretLen, &Pack);
	WIFI_DATATOPACK(Info->DeviceInfo.Product_Key, Info->DeviceInfo.Product_KeyLen, &Pack);
	WIFI_DATATOPACK(Info->DeviceInfo.Product_Secret, Info->DeviceInfo.Product_SecretLen, &Pack);
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_IOT_GET_DEVICEINFO(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_IOT_GET_DEVICEINFO;
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_IOT_REPORT_PROPERTY(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_IOT_REPORT_PROPERTY;
	WIFI_DATATOPACK(Info->IotReportInfo.Attribute, Info->IotReportInfo.AttributeLen, &Pack);
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_IOT_EVENT_NOTIFY(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_IOT_EVENT_NOTIFY;
	WIFI_DATATOPACK(Info->EventInfo.EventID, Info->EventInfo.EventIDLen, &Pack);
	WIFI_DATATOPACK(Info->EventInfo.EventData, Info->EventInfo.EventDataLen, &Pack);
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_IOT_USER_NOTIFY_EVENT(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_IOT_USER_NOTIFY_EVENT;
	WIFI_DATATOPACK(Info->EventInfo.EventID, Info->EventInfo.EventIDLen, &Pack);
	WIFI_DATATOPACK(Info->EventInfo.EventData, Info->EventInfo.EventDataLen, &Pack);
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_LINK_VISUAL_PUBLISH(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_LINK_VISUAL_PUBLISH;
	WIFI_DATATOPACK(Info->LVPublishInfo.Index, Info->LVPublishInfo.IndexLen, &Pack);
	WIFI_DATATOPACK(Info->LVPublishInfo.Topic, Info->LVPublishInfo.TopicLen, &Pack);
	WIFI_DATATOPACK(Info->LVPublishInfo.Message, Info->LVPublishInfo.MessaegLen, &Pack);
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_LINK_VISUAL_INIT_SUCCESS(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_LINK_VISUAL_INIT_SUCCESS;
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_HOST_CMD_GET_VERSION(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_HOST_CMD_GET_VERSION;
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_HOST_CMD_SET_VERSION(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_HOST_CMD_SET_VERSION;
	WIFI_DATATOPACK(Info->VersionInfo.LockVer, Info->VersionInfo.LockVerLen, &Pack);
	WIFI_DATATOPACK(Info->VersionInfo.InLockVer, Info->VersionInfo.InLockVerLen, &Pack);
	WIFI_DATATOPACK(Info->VersionInfo.HostVer, Info->VersionInfo.HostVerLen, &Pack);
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_HOST_CMD_WANK_UP(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_HOST_CMD_WANK_UP;
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_HOST_CMD_WIFI_OTA_START(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_HOST_CMD_WIFI_OTA_START;
	WIFI_DATATOPACK(Info->DataInfo.Data, Info->DataInfo.DataLen, &Pack);
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_HOST_CMD_WIFI_OTA(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_HOST_CMD_WIFI_OTA;
	WIFI_DATATOPACK(Info->DataInfo.Data, Info->DataInfo.DataLen, &Pack);
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_HOST_CMD_ALL_OTA(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_HOST_CMD_ALL_OTA;
	WIFI_DATATOPACK(Info->DataInfo.Data, Info->DataInfo.DataLen, &Pack);
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_HOST_CMD_WIFI_SLEEP_TIME(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_HOST_CMD_WIFI_SLEEP_TIME;
	WIFI_DATATOPACK((CVI_U8 *)&Info->TimeInfo.SleepTime, Info->TimeInfo.SleepTimeLen, &Pack);
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_HOST_TEST_RSSI(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_HOST_TEST_RSSI;
	WIFI_DATATOPACK(Info->ConnectInfo.SSID, Info->ConnectInfo.SSIDLEN, &Pack);
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_HOST_CMD_HEART(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_HOST_CMD_HEART;
	WIFI_DATATOPACK(Info->HeartInfo.SleepEnable, Info->HeartInfo.SleepEnableLen, &Pack);
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_HOST_CMD_ZEROCONFIG(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_HOST_CMD_ZEROCONFIG;
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_HOST_CMD_REBOOT(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_HOST_CMD_REBOOT;
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_HOST_CMD_UPDATEPROGRESS(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_HOST_CMD_UPDATEPROGRESS;
	WIFI_DATATOPACK(Info->UpgradeInfo.UpgradeID, Info->UpgradeInfo.UpgradeIDLen, &Pack);
	WIFI_DATATOPACK(Info->UpgradeInfo.Progress, Info->UpgradeInfo.ProgressLen, &Pack);
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}


CVI_S32 UPLOADFUN_W2H_HOST_CMD_ALL_OTA_START(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = W2H_HOST_CMD_ALL_OTA_START;
	WIFI_DATATOPACK(Info->DataInfo.Data, Info->DataInfo.DataLen, &Pack);
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 UPLOADFUN_H2W_HOST_CMD_UPDATEVERSION(const WIFI_MSGINFO_S *Info)
{
	WIFI_MSGPACKINFO_S Pack = {0};

	Pack.CMD = H2W_HOST_CMD_UPDATEVERSION;
	WIFI_DATATOPACK(Info->DataInfo.Data, Info->DataInfo.DataLen, &Pack);
	H2W_PACKMSG_SEND(&Pack);
	return CVI_SUCCESS;
}

CVI_S32 Wifi_HostMsgUpLoad(WIFI_MSG_E Cmd, const WIFI_MSGINFO_S *Info)
{
	switch (Cmd) {
	case H2W_HOST_CMD_GET_MAC:
		UPLOADFUN(H2W_HOST_CMD_GET_MAC)(Info);
		break;
	case H2W_HOST_CMD_GET_IP:
		UPLOADFUN(H2W_HOST_CMD_GET_IP)(Info);
		break;
	case H2W_HOST_CMD_SET_FILTER:
		UPLOADFUN(H2W_HOST_CMD_SET_FILTER)(Info);
		break;
	case H2W_HOST_CMD_GET_RSSI:
		UPLOADFUN(H2W_HOST_CMD_GET_RSSI)(Info);
		break;
	case H2W_HOST_CMD_CONNECT:
		UPLOADFUN(H2W_HOST_CMD_CONNECT)(Info);
		break;
	case H2W_HOST_CMD_START_STA:
		UPLOADFUN(H2W_HOST_CMD_START_STA)(Info);
		break;
	case H2W_HOST_CMD_STOP_STA:
		UPLOADFUN(H2W_HOST_CMD_STOP_STA)(Info);
		break;
	case H2W_HOST_CMD_CLEAN_WIFI:
		UPLOADFUN(H2W_HOST_CMD_CLEAN_WIFI)(Info);
		break;
	case H2W_HOST_CMD_CLEAN_DEVICEINFO:
		UPLOADFUN(H2W_HOST_CMD_CLEAN_DEVICEINFO)(Info);
		break;
	case H2W_HOST_CMD_CLEAN_ALL:
		UPLOADFUN(H2W_HOST_CMD_CLEAN_ALL)(Info);
		break;
	case H2W_IOT_SET_DEVICEINFO:
		UPLOADFUN(H2W_IOT_SET_DEVICEINFO)(Info);
		break;
	case H2W_IOT_GET_DEVICEINFO:
		UPLOADFUN(H2W_IOT_GET_DEVICEINFO)(Info);
		break;
	case H2W_IOT_REPORT_PROPERTY:
		UPLOADFUN(H2W_IOT_REPORT_PROPERTY)(Info);
		break;
	case H2W_IOT_EVENT_NOTIFY:
		UPLOADFUN(H2W_IOT_EVENT_NOTIFY)(Info);
		break;
	case H2W_IOT_USER_NOTIFY_EVENT:
		UPLOADFUN(H2W_IOT_USER_NOTIFY_EVENT)(Info);
		break;
	case H2W_LINK_VISUAL_PUBLISH:
		UPLOADFUN(H2W_LINK_VISUAL_PUBLISH)(Info);
		break;
	case H2W_LINK_VISUAL_INIT_SUCCESS:
		UPLOADFUN(H2W_LINK_VISUAL_INIT_SUCCESS)(Info);
		break;
	case H2W_HOST_CMD_GET_VERSION:
		UPLOADFUN(H2W_HOST_CMD_GET_VERSION)(Info);
		break;
	case H2W_HOST_CMD_SET_VERSION:
		UPLOADFUN(H2W_HOST_CMD_SET_VERSION)(Info);
		break;
	case H2W_HOST_CMD_WANK_UP:
		UPLOADFUN(H2W_HOST_CMD_WANK_UP)(Info);
		break;
	case H2W_HOST_CMD_WIFI_OTA_START:
		UPLOADFUN(H2W_HOST_CMD_WIFI_OTA_START)(Info);
		break;
	case H2W_HOST_CMD_WIFI_OTA:
		UPLOADFUN(H2W_HOST_CMD_WIFI_OTA)(Info);
		break;
	case H2W_HOST_CMD_ALL_OTA:
		UPLOADFUN(H2W_HOST_CMD_ALL_OTA)(Info);
		break;
	case H2W_HOST_CMD_WIFI_SLEEP_TIME:
		UPLOADFUN(H2W_HOST_CMD_WIFI_SLEEP_TIME)(Info);
		break;
	case H2W_HOST_TEST_RSSI:
		UPLOADFUN(H2W_HOST_TEST_RSSI)(Info);
		break;
	case H2W_HOST_CMD_HEART:
		UPLOADFUN(H2W_HOST_CMD_HEART)(Info);
		break;
	case H2W_HOST_CMD_ZEROCONFIG:
		UPLOADFUN(H2W_HOST_CMD_ZEROCONFIG)(Info);
		break;
	case H2W_HOST_CMD_REBOOT:
		UPLOADFUN(H2W_HOST_CMD_REBOOT)(Info);
		break;
	case H2W_HOST_CMD_UPDATEPROGRESS:
		UPLOADFUN(H2W_HOST_CMD_UPDATEPROGRESS)(Info);
		break;
	case W2H_HOST_CMD_ALL_OTA_START:
		UPLOADFUN(W2H_HOST_CMD_ALL_OTA_START)(Info);
		break;
	case H2W_HOST_CMD_UPDATEVERSION:
		UPLOADFUN(H2W_HOST_CMD_UPDATEVERSION)(Info);
		break;
	case W2H_HOST_CMD_REPORT_WPA_EVENT:
	case W2H_IOT_SET_PROPERTY:
	case W2H_IOT_SET_SERVICE:
	case W2H_IOT_REPORT_SERVICE:
	case W2H_IOT_USER_SET_SERVICE:
	case W2H_LINK_VISUAL_SERVICE:
	case W2H_CMD_CONNECT_WIFI_FIRST:
	case W2H_LINK_VIUSAL_CONNECT:
	case W2H_LINK_VISUAL_USER:
	case W2H_LINK_VIUSAL_DISCONNECT:
	case W2H_IOT_DEVICE_CONNECT:
	case W2H_HOST_CMD_UPDATEINFO:
	default:
		printf("%s unsupport Upload Cmd [%d]\n", __func__, Cmd);
		break;
	}
	return CVI_SUCCESS;
}
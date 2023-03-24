/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name:
 * Description:
 *   ....
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <aos/kernel.h>
#include <aos/cli.h>
#include "cvi_type.h"
#include "cvi_venc.h"
#include "cvi_vi.h"
#include "cvi_sys.h"
#include "cvi_vpss.h"
#include "event_dispatch.h"
#include <aos/kv.h>
#include "fatfs_vfs.h"
#if (CONFIG_APP_HI3861_WIFI_SUPPORT ==1)
#include "wifi_if.h"
#include "wifi_dispatch.h"
#include "wifi_hostmsgprase.h"
#endif
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1) && (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
#include "recordmanager.h"
#include "cx_func.h"
#include "linkkit_client.h"
#include "link_visual_api.h"
#include "link_visual_enum.h"
#include "exports/iot_export_linkkit.h"
#include "link_visual_struct.h"
#include "iot_export.h"
#include "iot_import.h"
#include "sdk_assistant.h"
#ifdef DUMMY_IPC
#include "dummy_ipc.h"
#else
#include "normal_ipc.h"
#endif
#endif

/** 产品功能描述
 * Host(H) : 门锁模块
 * Module(M) : 人脸模块(可视门铃不带人脸识别功能)
 **/

/*--------------------------------------------------------------------*/
typedef struct {
	CVI_S32 s32DevChn;
	CVI_U32 u32MaxWidth;
	CVI_U32 u32MaxHeight;
	CVI_U32 u32Width;
	CVI_U32 u32Height;
	CVI_U32 u32EncType;
	CVI_U32 uBindMode;
	CVI_U32 u32FrameRate;
} Venc_attr_s;

#define USER_UARTMAINVIDEO 0 //分辨率切换需要使用主VENC这里开启。cli不使用start_venc
#define UPLOAD_MODE 0
#define DUMP_BS 0
#define SNAP_GRPINDEX 1
#define SNAP_CHNINDEX 0
#define SNAP_VENCINDEX 1
#define VENC_DEVINDEX 0

static CVI_S32 MainVencRunStatus;
static pthread_t MainVencPthreadId;
static CVI_S32 StreamStatus;
static CVI_U8 TalkStatus;
static CVI_U8 NetStatus = NET_NOT_CONFIG;
static CVI_U8 LockRawData[256] = {0};
static CVI_U8 WIFI_SSID[32] = {0};       //保存的SSID
static CVI_U8 WIFI_PASSWD[24] = {0};     //保存的PASSWD
static CVI_U8 WifiMac[6] = {0};
static CVI_U8 FaceModuleMac[6] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
static AUDIO_CONFIG_INFO_S AudioParam = {0};
static ALIYUN_PARAM_S AliYunParm = {0};
#if DUMP_BS
static CVI_S32 FileNumber;
#endif
static Venc_attr_s Vencattr = {
	.s32DevChn = 0,
	.u32MaxWidth = 1280,
	.u32MaxHeight = 720,
	.u32Width = 1280,
	.u32Height = 720,
	.uBindMode = 0,
	.u32EncType = PT_H265,
	.u32FrameRate = 25,
};

void Upload_Ack_AttributePack();
void Upload_Ack_ServerPack();
void Upload_Ack_Event();
void Upload_Ack_ShadowDevice();
void Upload_Ack_ShadowDevice_Update();



CVI_S32 Host2Module_SetNetConfig(MODULE_PARAM_S *Parm)
{
	EVENT_DISPATCH_DEBUG(">>>>%s\n", __func__);
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1)
	/*
	CVI_U8 SSID[33] = {0};
	CVI_U8 Passwd[65] = {0};
	CVI_S32 SSIDLen = 0;
	CVI_S32 PasswdLen = 0;
	*/
	CVI_U8 enc_c[20] = {"c6"};
	if (Parm->NetCfg.ConfigMode == 1) {
		/*
		SSIDLen = strlen((CVI_CHAR *)Parm->NetCfg.SSID);
		PasswdLen = strlen((CVI_CHAR *)Parm->NetCfg.Password);
		SSIDLen = SSIDLen > 32 ? 32 : SSIDLen;
		PasswdLen = PasswdLen > 64 ? 64 : PasswdLen;
		WifiConfig_Write(Parm->NetCfg.SSID, SSIDLen, Parm->NetCfg.Password, PasswdLen);
		WifiConnect(Parm->NetCfg.SSID, SSIDLen, Parm->NetCfg.Password, PasswdLen);
		*/
		WIFI_MSGINFO_S Info = {0};
		Info.ConnectInfo.SSID = Parm->NetCfg.SSID;
		Info.ConnectInfo.SSIDLEN = strlen((CVI_CHAR *)Parm->NetCfg.SSID);;
		Info.ConnectInfo.KEY = Parm->NetCfg.Password;
		Info.ConnectInfo.KEYLEN = strlen((CVI_CHAR *)Parm->NetCfg.Password);
		//force fill enc data
		Info.ConnectInfo.ENC = enc_c;
		Info.ConnectInfo.ENCLEN = strlen((CVI_CHAR *)enc_c);
		Wifi_HostMsgUpLoad(H2W_HOST_CMD_CONNECT, &Info);

	} else if (Parm->NetCfg.ConfigMode == 0) {
		/*
		if (WifiConfig_Read(SSID, &SSIDLen, Passwd, &PasswdLen) == CVI_SUCCESS) {
			WifiConnect(SSID, SSIDLen, Passwd, PasswdLen);
		}*/

		WIFI_MSGINFO_S Info = {0};
		Wifi_HostMsgUpLoad(H2W_HOST_CMD_REBOOT, &Info);
	} else {
		return CVI_FAILURE;
	}
#endif
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_SetVideoInfo(MODULE_PARAM_S *Parm)
{
	EVENT_DISPATCH_DEBUG(">>>>%s\n", __func__);
	VENC_CHN_ATTR_S Venc_stChnAttr = {0};

	if (CVI_VENC_GetChnAttr(VENC_DEVINDEX, &Venc_stChnAttr) != CVI_SUCCESS) {
		EVENT_DISPATCH_DEBUG("<<<<CVI_VENC_GetChnAttr  error\n");
		return RES_ERRORCODE_1;
	}
#if USER_UARTMAINVIDEO
	CVI_U32 DstFrameRate = 25;

	switch (Parm->VideoParm.Framerate) {
	case VIDEO_FRAME_RATE_15FPS:
		DstFrameRate = 15;
		break;
	case VIDEO_FRAME_RATE_25FPS:
		DstFrameRate = 25;
		break;
	case VIDEO_FRAME_RATE_30FPS:
		DstFrameRate = 30;
		break;
	default:
		EVENT_DISPATCH_DEBUG("<<<<%s FPS error\n", __func__);
		return RES_ERRORCODE_2;
		break;
	}
#endif
	switch (Parm->VideoParm.Video_Encode) {
	case VIDEO_ENCODE_TYPE_H264:
		Venc_stChnAttr.stVencAttr.enType = PT_H264;
		break;
	case VIDEO_ENCODE_TYPE_H265:
		Venc_stChnAttr.stVencAttr.enType = PT_H265;
		break;
	case VIDEO_ENCODE_TYPE_MJPEG:
		Venc_stChnAttr.stVencAttr.enType = PT_MJPEG;
		break;
	default:
		EVENT_DISPATCH_DEBUG("<<<<%s Video_Encode error\n", __func__);
		return RES_ERRORCODE_1;
		break;
	}
	switch (Parm->VideoParm.Video_Size) {
	case VIDEO_SIZE_1200x1600:
		Venc_stChnAttr.stVencAttr.u32PicWidth = 1600;
		Venc_stChnAttr.stVencAttr.u32PicHeight = 1200;
		break;
	case VIDEO_SIZE_900x1200:
		Venc_stChnAttr.stVencAttr.u32PicWidth = 1200;
		Venc_stChnAttr.stVencAttr.u32PicHeight = 900;
		break;
	case VIDEO_SIZE_720x960:
		Venc_stChnAttr.stVencAttr.u32PicWidth = 960;
		Venc_stChnAttr.stVencAttr.u32PicHeight = 720;
		break;
	case VIDEO_SIZE_600x800:
		Venc_stChnAttr.stVencAttr.u32PicWidth = 800;
		Venc_stChnAttr.stVencAttr.u32PicHeight = 600;
		break;
	case VIDEO_SIZE_360x480:
		Venc_stChnAttr.stVencAttr.u32PicWidth = 480;
		Venc_stChnAttr.stVencAttr.u32PicHeight = 360;
		break;
	case VIDEO_SIZE_300x400:
		Venc_stChnAttr.stVencAttr.u32PicWidth = 400;
		Venc_stChnAttr.stVencAttr.u32PicHeight = 300;
		break;
	case VIDEO_SIZE_320x480:
		Venc_stChnAttr.stVencAttr.u32PicWidth = 480;
		Venc_stChnAttr.stVencAttr.u32PicHeight = 320;
		break;
	case VIDEO_SIZE_720x1280:
		Venc_stChnAttr.stVencAttr.u32PicWidth = 1280;
		Venc_stChnAttr.stVencAttr.u32PicHeight = 720;
		break;
	default:
		EVENT_DISPATCH_DEBUG("<<<<%s size error\n", __func__);
		return RES_ERRORCODE_3;
		break;
	}
	if (Venc_stChnAttr.stVencAttr.u32PicWidth > Venc_stChnAttr.stVencAttr.u32MaxPicWidth
	    || Venc_stChnAttr.stVencAttr.u32PicHeight > Venc_stChnAttr.stVencAttr.u32MaxPicHeight) {
		EVENT_DISPATCH_DEBUG("<<<<%s size error\n", __func__);
		return RES_ERRORCODE_3;
	}
#if USER_UARTMAINVIDEO
	MainVideo_VencDestory(VENC_DEVINDEX);
	Vencattr.u32Height = Venc_stChnAttr.stVencAttr.u32PicHeight;
	Vencattr.u32Width = Venc_stChnAttr.stVencAttr.u32PicWidth;
	Vencattr.u32EncType = Venc_stChnAttr.stVencAttr.enType;
	switch (Venc_stChnAttr.stVencAttr.enType) {
	case PT_H264:
		Vencattr.u32FrameRate = Venc_stChnAttr.stRcAttr.stH264Cbr.fr32DstFrameRate = DstFrameRate;
		break;
	case PT_H265:
		Vencattr.u32FrameRate = Venc_stChnAttr.stRcAttr.stH265Cbr.fr32DstFrameRate = DstFrameRate;
		break;
	case PT_MJPEG:
		Vencattr.u32FrameRate = Venc_stChnAttr.stRcAttr.stMjpegCbr.fr32DstFrameRate = DstFrameRate;
		break;
	default:
		EVENT_DISPATCH_DEBUG("<<<<%s size error\n", __func__);
		return RES_ERRORCODE_2;
		break;
	}
	MainVideo_VencInit(&Vencattr);
#endif
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_SetPictureInfo(MODULE_PARAM_S *Parm)
{
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1) && (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
	//只有JPEG
	CVI_U32 Width = 0;
	CVI_U32 Height = 0;

	//switch (Parm->picparm.Picture_Type) {
	//case PICTURE_TYPE_JPEG:
	//	Venc_stChnAttr.stVencAttr.enType = PT_JPEG;
	//	break;
	//case PICTURE_TYPE_YVU420SP:
	//	Vpss_stChnAttr.enPixelFormat = PIXEL_FORMAT_NV21;
	//	break;
	//default:
	//	EVENT_DISPATCH_DEBUG("%s: RES_ERRORCODE_1\n", __func__);
	//	return RES_ERRORCODE_1;
	//	break;
	//}

	switch (Parm->picparm.Picture_Size) {
	case VIDEO_SIZE_1200x1600:
		Width = 1200;
		Height = 1600;
		break;
	case VIDEO_SIZE_900x1200:
		Width = 900;
		Height = 1200;
		break;
	case VIDEO_SIZE_720x960:
		Width = 720;
		Height = 960;
		break;
	case VIDEO_SIZE_600x800:
		Width = 600;
		Height = 800;
		break;
	case VIDEO_SIZE_360x480:
		Width = 360;
		Height = 480;
		break;
	case VIDEO_SIZE_300x400:
		Width = 300;
		Height = 400;
		break;
	case VIDEO_SIZE_320x480:
		Width = 320;
		Height = 480;
		break;
	case VIDEO_SIZE_720x1280:
		Width = 720;
		Height = 1280;
		break;
	default:
		EVENT_DISPATCH_DEBUG("RES_ERRORCODE_2\n");
		return RES_ERRORCODE_2;
		break;
	}
	VPSS_CHN_ATTR_S stChnAttr = {0};
	venChnAttr stSnapShotAttr = {0};
	if (cx_get_snapshot_attr(&stSnapShotAttr) == CVI_SUCCESS) {
		if (CVI_VPSS_GetChnAttr(stSnapShotAttr.vpssGrp, stSnapShotAttr.vpssChn, &stChnAttr) == CVI_SUCCESS) {
			if (Width > stChnAttr.u32Width || Height > stChnAttr.u32Height) {
				EVENT_DISPATCH_DEBUG("Host2Module_SetPictureInfo size err\n");
				return RES_ERRORCODE_2;
			}
			stSnapShotAttr.width = Width;
			stSnapShotAttr.height = Height;
			cx_set_snapshot_attr(&stSnapShotAttr);
		} else
			return RES_ERRORCODE_2;
	} else
		return RES_ERRORCODE_2;
	EVENT_DISPATCH_DEBUG("Host2Module_SetPictureInfo Width:%d Height:%d\n", Width, Height);
#endif
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_SetAudioInfo(MODULE_PARAM_S *Parm)
{
	EVENT_DISPATCH_DEBUG(">>>>%s\n", __func__);
	AudioParam.Audio_EncType = Parm->AudioParm.Audio_EncType;
	AudioParam.Sample_Rate = Parm->AudioParm.Sample_Rate;
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_SetAliYunParm(MODULE_PARAM_S *Parm)
{
	AliYunParm.Product_Secret_Len = Parm->AliyunParm.Product_Secret_Len;
	AliYunParm.Product_Key_Len = Parm->AliyunParm.Product_Key_Len;
	AliYunParm.Device_Name_Len = Parm->AliyunParm.Device_Name_Len;
	AliYunParm.Device_Secret_Len = Parm->AliyunParm.Device_Secret_Len;
	EVENT_DISPATCH_DEBUG("AliyunParm.Device_Name is %s Device_Name_Len [%d]\n", Parm->AliyunParm.Device_Name,
	       Parm->AliyunParm.Device_Name_Len);
	memcpy(AliYunParm.Product_Secret, Parm->AliyunParm.Product_Secret, AliYunParm.Product_Secret_Len);
	memcpy(AliYunParm.Product_Key, Parm->AliyunParm.Product_Key, AliYunParm.Product_Key_Len);
	memcpy(AliYunParm.Device_Name, Parm->AliyunParm.Device_Name, AliYunParm.Device_Name_Len);
	memcpy(AliYunParm.Device_Secret, Parm->AliyunParm.Device_Secret, AliYunParm.Device_Secret_Len);
	EVENT_DISPATCH_DEBUG("AliYundParm.Device_Name is %s Device_Name_Len [%d]\n", AliYunParm.Device_Name,
	       AliYunParm.Device_Name_Len);
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1)
	WIFI_MSGINFO_S Info = {0};
	Info.DeviceInfo.DeviceName = AliYunParm.Device_Name;
	Info.DeviceInfo.DeviceNameLen = AliYunParm.Device_Name_Len;
	Info.DeviceInfo.Device_Secret = AliYunParm.Device_Secret;
	Info.DeviceInfo.Device_SecretLen = AliYunParm.Device_Secret_Len;
	Info.DeviceInfo.Product_Key = AliYunParm.Product_Key;
	Info.DeviceInfo.Product_KeyLen = AliYunParm.Product_Key_Len;
	Info.DeviceInfo.Product_Secret = AliYunParm.Product_Secret;
	Info.DeviceInfo.Product_SecretLen = AliYunParm.Product_Secret_Len;
	Wifi_HostMsgUpLoad(H2W_IOT_SET_DEVICEINFO, &Info);
#if (CONFIG_SUPPORT_NORFLASH == 1)
	CVI_S32 s32Ret = 0;
	s32Ret = aos_kv_set("dn", Info.DeviceInfo.DeviceName, Info.DeviceInfo.DeviceNameLen, 1);
	if (s32Ret != 0) {
		EVENT_DISPATCH_DEBUG("%s aos_kv_set dn err\n", __func__);
	}
	s32Ret = aos_kv_set("ds", Info.DeviceInfo.Device_Secret, Info.DeviceInfo.Device_SecretLen, 1);
	if (s32Ret != 0) {
		EVENT_DISPATCH_DEBUG("%s aos_kv_set dn err\n", __func__);
	}
	s32Ret = aos_kv_set("pk", Info.DeviceInfo.Product_Key, Info.DeviceInfo.Product_KeyLen, 1);
	if (s32Ret != 0) {
		EVENT_DISPATCH_DEBUG("%s aos_kv_set dn err\n", __func__);
	}
	s32Ret = aos_kv_set("ps", Info.DeviceInfo.Product_Secret, Info.DeviceInfo.Product_SecretLen, 1);
	if (s32Ret != 0) {
		EVENT_DISPATCH_DEBUG("%s aos_kv_set dn err\n", __func__);
	}
#endif
#endif
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_CleanNetConfig()
{
	EVENT_DISPATCH_DEBUG(">>>>%s\n", __func__);
	memset(WIFI_SSID, 0, sizeof(WIFI_SSID));
	memset(WIFI_PASSWD, 0, sizeof(WIFI_PASSWD));
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1)
	WifiConfig_Clean();
	WifiDisConnect();
	//调用WIFI SDIO接口
	WIFI_MSGINFO_S Info = {0};
	Wifi_HostMsgUpLoad(H2W_HOST_CMD_CLEAN_WIFI, &Info);
#endif
	return CVI_SUCCESS;
}

CVI_S32 HostModule_FactoryReset()
{
	EVENT_DISPATCH_DEBUG(">>>>%s\n", __func__);
	//恢复默认配置
#if (CONFIG_APP_HI3861_WIFI_SUPPORT ==1)
	WifiConfig_Clean();
	WIFI_MSGINFO_S Info = {0};
	Wifi_HostMsgUpLoad(H2W_HOST_CMD_CLEAN_ALL, &Info);
#endif
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_TransferLockMac(MODULE_PARAM_S *Parm)
{
	memcpy(WifiMac, Parm->MAC, 6);
	EVENT_DISPATCH_DEBUG("<<<< LockMac is %s\n", WifiMac);
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_TransferLocktmpPw(MODULE_PARAM_S *Parm)
{
	//上报锁临时密码数据
	CVI_U16 i;
	CVI_U16 keyNum = Parm->TmpPasswdInfo.tempPassWordNum;
	TMP_PWD_INFO_NODE_S *pkeyNode = Parm->TmpPasswdInfo.Node;

	for (i = 0; i < keyNum; i++) {
		CVI_U8 tempPassWordCntent[7] = {0};
		memcpy(tempPassWordCntent, pkeyNode[i].tempPassWordCntent, 6);
		EVENT_DISPATCH_DEBUG("[%d] tempPassWordId [%d] Status [%d] CntTent [%s]\n", i, pkeyNode[i].tempPassWordId,
		       pkeyNode[i].tempPassWordStatus, tempPassWordCntent);
	}
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_AckAttributeSetting(MODULE_PARAM_S *Parm)
{
	switch (Parm->ErrCode) {
	case RES_ERRORCODE_CMD_OK:
		EVENT_DISPATCH_DEBUG("%s : Requst Success\n", __func__);
		break;
	case RES_ERRORCODE_1:
		EVENT_DISPATCH_DEBUG("%s : Requst ParmErr\n", __func__);
		break;
	case RES_ERRORCODE_2:
		EVENT_DISPATCH_DEBUG("%s : Requst DataErr\n", __func__);
		break;
	default:
		EVENT_DISPATCH_DEBUG("%s : UnKnow Code %d\n", __func__, Parm->ErrCode);
		break;
	}
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_AttributePack(MODULE_PARAM_S *Parm)
{
	EVENT_DISPATCH_DEBUG("%s: Datalen [%d] \n String is %s\n", __func__, Parm->RawData.Datalen, (CVI_CHAR *)Parm->RawData.Data);
#if (CONFIG_APP_HI3861_WIFI_SUPPORT ==1)
	WIFI_MSGINFO_S Info = {0};

	Info.IotReportInfo.Attribute = Parm->RawData.Data;
	Info.IotReportInfo.AttributeLen = Parm->RawData.Datalen;
	Wifi_HostMsgUpLoad(H2W_IOT_REPORT_PROPERTY, &Info);//上报锁端保存的配置
	WifiMsg_IOT_SET_PROPERTY((CVI_CHAR *)Parm->RawData.Data);//拿出可以同步的配置设置flash
#endif
	Upload_Ack_AttributePack(); //注意这里收到包后需要进行ACK回复 否则会引起重发机制
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_AckServerCommand(MODULE_PARAM_S *Parm)
{
	switch (Parm->ErrCode) {
	case RES_ERRORCODE_CMD_OK:
		EVENT_DISPATCH_DEBUG("%s : Requst Success\n", __func__);
		break;
	case RES_ERRORCODE_1:
		EVENT_DISPATCH_DEBUG("%s : Requst ParmErr\n", __func__);
		break;
	case RES_ERRORCODE_2:
		EVENT_DISPATCH_DEBUG("%s : Requst DataErr\n", __func__);
		break;
	default:
		EVENT_DISPATCH_DEBUG("%s : UnKnow Code %d\n", __func__, Parm->ErrCode);
		break;
	}
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_ReportServerPack(MODULE_PARAM_S *Parm)
{
	EVENT_DISPATCH_DEBUG("%s: packet %d packet_number %d packet_Seqnumber %d key %s len %d\n", __func__, Parm->PackParm.packet,
	       Parm->PackParm.packet_number, Parm->PackParm.packet_Seqnumber, Parm->PackParm.key, Parm->PackParm.len);
	EVENT_DISPATCH_DEBUG("PackString %s\n", Parm->PackString);
	Upload_Ack_ServerPack();//接收数据后需要回复
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_ReportEvent(MODULE_PARAM_S *Parm)
{
	CVI_U8 Data[512] = {0};
	memcpy(Data, Parm->PackString, Parm->PackParm.len > sizeof(Data) ? sizeof(Data) : Parm->PackParm.len);
	EVENT_DISPATCH_DEBUG("%s : Key %s len %d Data %s \n", __func__, Parm->PackParm.key,
	       Parm->PackParm.len, Data);
	//校验数据正常后回复
	Upload_Ack_Event();
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_AckUpgrade(MODULE_PARAM_S *Parm)
{
	switch (Parm->ErrCode) {
	case RES_ERRORCODE_CMD_OK:
		EVENT_DISPATCH_DEBUG("%s : AckSuccess\n", __func__);
		break;
	case RES_ERRORCODE_1:
		EVENT_DISPATCH_DEBUG("%s : MCU Not Ready\n", __func__);
		break;
	default:
		EVENT_DISPATCH_DEBUG("%s : Default Code\n", __func__);
		break;
	}
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_AckUpgradePack(MODULE_PARAM_S *Parm)
{
	switch (Parm->ErrCode) {
	case RES_ERRORCODE_CMD_OK:
		EVENT_DISPATCH_DEBUG("%s : AckSuccess\n", __func__);
		break;
	case RES_ERRORCODE_1:
		EVENT_DISPATCH_DEBUG("%s : MCU Need To ReUpLoad\n", __func__);
		break;
	default:
		EVENT_DISPATCH_DEBUG("%s : Default Code\n", __func__);
		break;
	}
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_ReportShadowDevice(MODULE_PARAM_S *Parm)
{
	EVENT_DISPATCH_DEBUG("%s DeltaUpdate:%d len :%d\n", __func__, Parm->ShadowDeviceInfo.DeltaUpdate, Parm->ShadowDeviceInfo.len);
	EVENT_DISPATCH_DEBUG("String : %s\n", Parm->PackString);
	//确认数据正常则回复
	Upload_Ack_ShadowDevice();
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_ReportShadowDeviceUpdate(MODULE_PARAM_S *Parm)
{
	EVENT_DISPATCH_DEBUG("%s :ShadownUpdate\n", __func__);
	//确认数据正常则回复
	Upload_Ack_ShadowDevice_Update();
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_TransferLockInfo(MODULE_PARAM_S *Parm)
{
	//上报锁密码数据
	CVI_U16 i;
	CVI_U16 keyNum = Parm->LockKeyInfo.keyNum;
	LOCK_KEY_INFO_NODE_S *pkeyNode = Parm->LockKeyInfo.keyNode;

	for (i = 0; i < keyNum; i++) {
		CVI_U8 TempKeyAlias[33] = {0};
		memcpy(TempKeyAlias, pkeyNode[i].KeyAlias, 32);
		EVENT_DISPATCH_DEBUG("[%d] keyID [%d] lockType [%d] userLimit [%d] KeyAlias [%s]\n", i, pkeyNode[i].keyID,
		       pkeyNode[i].lockType, pkeyNode[i].userLimit, TempKeyAlias);
	}
	return CVI_SUCCESS;
}

void Module_NetStatusSet(NET_STATUS_E bSet)
{
	NetStatus = bSet;
}

CVI_S32 Host2Module_GetNetStatus(MODULE_PARAM_S *Parm)
{
	EVENT_DISPATCH_DEBUG(">>>>%s NetStatus is %d\n", __func__, NetStatus);
	Parm->NetStatus = NetStatus;
	return CVI_SUCCESS;
}

static CVI_S32 Module_TransforFrameRate(const VENC_CHN_ATTR_S *attr)
{
	switch (attr->stVencAttr.enType) {
	case PT_H264:
		switch (attr->stRcAttr.stH264Cbr.fr32DstFrameRate) {
		case 15:
			return VIDEO_FRAME_RATE_15FPS;
		case 25:
			return VIDEO_FRAME_RATE_25FPS;
		case 30:
			return VIDEO_FRAME_RATE_30FPS;
		default:
			return CVI_FAILURE;
		}
		break;
	case PT_H265:
		switch (attr->stRcAttr.stH265Cbr.fr32DstFrameRate) {
		case 15:
			return VIDEO_FRAME_RATE_15FPS;
		case 25:
			return VIDEO_FRAME_RATE_25FPS;
		case 30:
			return VIDEO_FRAME_RATE_30FPS;
		default:
			return CVI_FAILURE;
		}
		break;
	case PT_MJPEG:
		switch (attr->stRcAttr.stH264Cbr.fr32DstFrameRate) {
		case 15:
			return VIDEO_FRAME_RATE_15FPS;
		case 25:
			return VIDEO_FRAME_RATE_25FPS;
		case 30:
			return VIDEO_FRAME_RATE_30FPS;
		default:
			return CVI_FAILURE;
		}
		break;
	default:
		return CVI_FAILURE;
	}
	return CVI_FAILURE;
}


CVI_S32 Host2Module_GetVideoInfo(MODULE_PARAM_S *Parm)
{
	EVENT_DISPATCH_DEBUG(">>>>%s\n", __func__);
	VENC_CHN_ATTR_S Venc_stChnAttr = {0};

	if (CVI_VENC_GetChnAttr(VENC_DEVINDEX, &Venc_stChnAttr) != CVI_SUCCESS) {
		EVENT_DISPATCH_DEBUG("<<<CVI_VENC_GetChnAttr error\n");
		return RES_ERRORCODE_1;
	}
	switch (Venc_stChnAttr.stVencAttr.enType) {
	case PT_H264:
		Parm->VideoParm.Video_Encode = VIDEO_ENCODE_TYPE_H264;
		break;
	case PT_H265:
		Parm->VideoParm.Video_Encode = VIDEO_ENCODE_TYPE_H265;
		break;
	case PT_MJPEG:
		Parm->VideoParm.Video_Encode = VIDEO_ENCODE_TYPE_MJPEG;
		break;
	default:
		return RES_ERRORCODE_1;
	}
	Parm->VideoParm.Framerate = Module_TransforFrameRate(&Venc_stChnAttr);
	if (Parm->VideoParm.Framerate == CVI_FAILURE)
		return RES_ERRORCODE_1;
	switch (Venc_stChnAttr.stVencAttr.u32PicWidth * Venc_stChnAttr.stVencAttr.u32PicHeight) {
	case 1200*1600:
		Parm->VideoParm.Video_Size = VIDEO_SIZE_1200x1600;
		break;
	case 900*1200:
		Parm->VideoParm.Video_Size = VIDEO_SIZE_900x1200;
		break;
	case 720*960:
		Parm->VideoParm.Video_Size = VIDEO_SIZE_720x960;
		break;
	case 600*800:
		Parm->VideoParm.Video_Size = VIDEO_SIZE_600x800;
		break;
	case 360*480:
		Parm->VideoParm.Video_Size = VIDEO_SIZE_360x480;
		break;
	case 300*400:
		Parm->VideoParm.Video_Size = VIDEO_SIZE_300x400;
		break;
	case 320*480:
		Parm->VideoParm.Video_Size = VIDEO_SIZE_320x480;
		break;
	case 720*1280:
		Parm->VideoParm.Video_Size = VIDEO_SIZE_720x1280;
		break;
	default:
		return RES_ERRORCODE_3;
	}
	EVENT_DISPATCH_DEBUG("u32PicWidth [%d] u32PicHeight [%d]\n", Venc_stChnAttr.stVencAttr.u32PicWidth,
	       Venc_stChnAttr.stVencAttr.u32PicHeight);
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_GetPictureInfo(MODULE_PARAM_S *Parm)
{
	CVI_U32 u32PicWidth = 0;
	CVI_U32 u32PicHeight = 0;
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1) && (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
	venChnAttr stSnapShotAttr = {0};
	if (cx_get_snapshot_attr(&stSnapShotAttr) == CVI_SUCCESS) {
		u32PicWidth = stSnapShotAttr.width;
		u32PicHeight = stSnapShotAttr.height;
	} else {
		return RES_ERRORCODE_2;
	}
#endif
	EVENT_DISPATCH_DEBUG("Host2Module_GetPictureInfo u32PicWidth:%d u32PicHeight:%d\n", u32PicWidth, u32PicHeight);
	switch (u32PicWidth) {
	case 1200:
		Parm->picparm.Picture_Size = VIDEO_SIZE_1200x1600;
		break;
	case 900:
		Parm->picparm.Picture_Size = VIDEO_SIZE_900x1200;
		break;
	case 720:
		if (u32PicHeight > 960) {
			Parm->picparm.Picture_Size = VIDEO_SIZE_720x1280;
		} else {
			Parm->picparm.Picture_Size = VIDEO_SIZE_720x960;
		}
		break;
	case 600:
		Parm->picparm.Picture_Size = VIDEO_SIZE_600x800;
		break;
	case 360:
		Parm->picparm.Picture_Size = VIDEO_SIZE_360x480;
		break;
	case 300:
		Parm->picparm.Picture_Size = VIDEO_SIZE_300x400;
		break;
	case 320:
		Parm->picparm.Picture_Size = VIDEO_SIZE_320x480;
		break;
	default:
		return RES_ERRORCODE_2;
	}
	Parm->picparm.Picture_Type = PICTURE_TYPE_JPEG;
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_GetAudioInfo(MODULE_PARAM_S *Parm)
{
	EVENT_DISPATCH_DEBUG(">>>>%s\n", __func__);
	Parm->AudioParm.Audio_EncType = AUDIO_ENCODE_FORMATE_G711A;
	Parm->AudioParm.Sample_Rate = AUDIO_SAMPLERATE_48K;
	return CVI_SUCCESS;
}


CVI_S32 Host2Module_GetStreamStaus(MODULE_PARAM_S *Parm)
{
	EVENT_DISPATCH_DEBUG(">>>>%s\n", __func__);
	Parm->StreamStaus = StreamStatus;
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_GetIpcTalkStatus(MODULE_PARAM_S *Parm)
{
	EVENT_DISPATCH_DEBUG(">>>>%s\n", __func__);
	Parm->TalkStatus = TalkStatus;
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_GetModuleMac(MODULE_PARAM_S *Parm)
{
	EVENT_DISPATCH_DEBUG(">>>>%s\n", __func__);
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1)
	WifiGetMac(FaceModuleMac);
	EVENT_DISPATCH_DEBUG("%s Mac:%02x:%02x:%02x:%02x:%02x:%02x \n", __func__, FaceModuleMac[0],
	       FaceModuleMac[1], FaceModuleMac[2], FaceModuleMac[3], FaceModuleMac[4], FaceModuleMac[5]);
#endif
	memcpy(Parm->MAC, FaceModuleMac, 6);
	return CVI_SUCCESS;
}


CVI_S32 Host2Module_GetAliYunParm(MODULE_PARAM_S *Parm)
{
	EVENT_DISPATCH_DEBUG(">>>>%s\n", __func__);
	memcpy(&Parm->AliyunParm, &AliYunParm, sizeof(ALIYUN_PARAM_S));
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1)
	WIFI_MSGINFO_S Info = {0};
	Wifi_HostMsgUpLoad(H2W_IOT_GET_DEVICEINFO, &Info);
#endif
	/*
	#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1) && (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
		cx_cloudkvDeviceInfo_Get((CVI_CHAR *)Parm->AliyunParm.Product_Key,sizeof(Parm->AliyunParm.Product_Key),
		(CVI_CHAR *)Parm->AliyunParm.Product_Secret,sizeof(Parm->AliyunParm.Product_Secret),
		(CVI_CHAR *)Parm->AliyunParm.Device_Name,sizeof(Parm->AliyunParm.Device_Name),
		(CVI_CHAR *)Parm->AliyunParm.Device_Secret,sizeof(Parm->AliyunParm.Device_Secret));
	#endif
	*/
	return CVI_SUCCESS;
}

CVI_S32 StartStream(MODULE_PARAM_S *Info)
{
	EVENT_DISPATCH_DEBUG(">>>>%s\n", __func__);
	StreamStatus = 1;
	Info->Duration = 0;
	return CVI_SUCCESS;
}

CVI_S32 StopStream(void)
{
	StreamStatus = 0;
	return CVI_SUCCESS;
}

static CVI_CHAR *g_picture_buf = NULL;
static CVI_S32  g_snapsize = 0;

CVI_S32 SnapShot_Ready()
{
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1) && (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
	cx_pic_type_e pic_type;
	CVI_S32 SnapSize = 500 * 1024;
	if (g_picture_buf) {
		free(g_picture_buf);
		g_picture_buf = NULL;
		g_snapsize = 0;
	}
	g_picture_buf = (CVI_CHAR *)malloc(SnapSize * sizeof(CVI_CHAR));
	if (!g_picture_buf) {
		EVENT_DISPATCH_DEBUG("SnapShot_Ready malloc %d err\n", SnapSize);
		return CVI_FAILURE;
	}
	memset(g_picture_buf, 0, SnapSize);
	g_snapsize = app_hal_get_one_picture((CVI_CHAR *)g_picture_buf, SnapSize, &pic_type);
	if (g_snapsize <= 0) {
		free(g_picture_buf);
		g_picture_buf = NULL;
		g_snapsize = 0;
		return CVI_FAILURE;
	}
#endif
	return CVI_SUCCESS;
}



CVI_S32 CapturePicture(MODULE_PARAM_S *Info)
{
	EVENT_DISPATCH_DEBUG(">>>>%s\n", __func__);
	//CVI_U8 *SnapPic_Data = NULL;
	CVI_S32 ret = CVI_SUCCESS;
	CVI_U32 *pu32CaptureType = 0;
	CVI_U64 isRecord = CVI_FALSE;
	pu32CaptureType = (CVI_U32 *)Info->CaptureInfo.CaptureType;
	EVENT_DISPATCH_DEBUG("pu32CaptureType is %u \n", *pu32CaptureType);

	if (*pu32CaptureType == TAMPER_ALARM ||
	    *pu32CaptureType == DOORBELL_EVENT ||
	    *pu32CaptureType == STAY_ALARM ||
	    *pu32CaptureType == OPEN_DOOR_ALARM ||
	    *pu32CaptureType == FINGERPRINT_ERRO_ALARM ||
	    *pu32CaptureType == PASSWORD_ERROR_ALRAM) {
		isRecord = CVI_TRUE;
	}
	/*
	#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1) && (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
	Wifi_SetHeadCount(8000);
	CVI_S32 SnapSize = 0;
	if( isRecord == CVI_FALSE)
	{
		//venChnAttr stSnapShotAttr = {0};
		//if(cx_get_snapshot_attr(&stSnapShotAttr) != CVI_SUCCESS)
		//{
		//	EVENT_DISPATCH_DEBUG("CapturePicture cx_get_snapshot_attr err\n");
		//	return CVI_FAILURE;
		//}
		cx_pic_type_e pic_type;
		SnapSize = 500 *1024;
		SnapPic_Data = (CVI_U8 *)malloc(SnapSize *sizeof(CVI_U8));
		if(!SnapPic_Data) {
			EVENT_DISPATCH_DEBUG("CapturePicture SnapPic_Data malloc %d err\n",SnapSize);
			return CVI_FAILURE;
		}
		SnapSize = app_hal_get_one_picture((CVI_CHAR *)SnapPic_Data,SnapSize,&pic_type);
	}
	#endif

	#if DUMP_BS
	if (SnapPic_Data) {
		static int FileNumber = 0;
		char FileName[64] = {0};
		int filefd = 0;

		snEVENT_DISPATCH_DEBUG(FileName, sizeof(FileName), SD_FATFS_MOUNTPOINT"/CapturePic%d", FileNumber);
		filefd = aos_open(FileName, O_CREAT | O_TRUNC | O_RDWR);
		aos_write(filefd, SnapPic_Data, SnapSize);
		aos_sync(filefd);
		aos_close(filefd);
		FileNumber++;
	}
	#endif
	*/
#if (CONFIG_APP_HI3861_WIFI_SUPPORT == 1) && (CONFIG_APP_CX_CLOUD_SUPPORT == 1)
	EVENT_DISPATCH_DEBUG("SnapType is %d SnapSize %d\n", *pu32CaptureType, g_snapsize);
	lv_device_auth_s auth = {0};
	if (cx_cloudGetAuth(&auth.product_key, NULL, &auth.device_name, &auth.device_secret) >= 0) {
		lv_alarm_event_param_s param;
		int service_id;
		memset(&param, 0, sizeof(lv_alarm_event_param_s));
		param.type = *pu32CaptureType;
		param.media.p = g_picture_buf;
		param.media.len = g_snapsize;
		param.addition_string.p = (char *)Info->CaptureInfo.Data;
		param.addition_string.len = Info->CaptureInfo.Datalen;
		EVENT_DISPATCH_DEBUG("The CapturePicture len %d data is %s \n", Info->CaptureInfo.Datalen, Info->CaptureInfo.Data);
		ret = lv_post_alarm_image(&auth, &param, &service_id);
	}
#endif
	if (g_picture_buf) {
		free(g_picture_buf);
		g_picture_buf = NULL;
		g_snapsize = 0;
	}
	if (isRecord == CVI_TRUE) {
#if (CONFIG_APP_RECORD_SUPPORT == 1)
		APP_Record_StartNotify(10);
#endif
	}
	return ret;
}

CVI_S32 StartTalkPlay(MODULE_PARAM_S *Info)
{
	//启动对讲 Out:返回对讲时长 Return: 0:Success 1:已在对讲，2推流失败，3拉流失败
	TalkStatus = 1;
	Info->Duration = 0x00;
	return CVI_SUCCESS;
}

CVI_S32 StopTalkPlay(void)
{
	EVENT_DISPATCH_DEBUG(">>>>%s\n", __func__);
	//停止对讲
	TalkStatus = 0;
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_TransRawData(MODULE_PARAM_S *Info)
{
	CVI_U32 Datalen = Info->RawData.Datalen;
	memcpy(LockRawData, Info->RawData.Data, Datalen > sizeof(LockRawData) ? sizeof(LockRawData) : Datalen);
	EVENT_DISPATCH_DEBUG("The Host2Module_TransRawData is %s\n", LockRawData);
#if (CONFIG_APP_HI3861_WIFI_SUPPORT ==1)
	WIFI_MSGINFO_S WifiInfo = {0};

	WifiInfo.EventInfo.EventID = (CVI_U8 *)"lock_notify";
	WifiInfo.EventInfo.EventIDLen = strlen("lock_notify");
	WifiInfo.EventInfo.EventData = Info->RawData.Data;
	WifiInfo.EventInfo.EventDataLen = Info->RawData.Datalen;
	Wifi_HostMsgUpLoad(H2W_IOT_USER_NOTIFY_EVENT, &WifiInfo);
#endif
	return CVI_SUCCESS;
}

void UploadTalkStatus(void)
{
	MODULE_UPLOAD_S Info = {0};
	Info.Comm_Cmd = COMM_CMD_M2H_REPORT_IPC_TALK_STATUS;
	Info.StreamStatus = TalkStatus;
	Module_EventUpLoad(&Info);
}

void UploadSteamStatus(void)
{
	MODULE_UPLOAD_S Info = {0};
	Info.Comm_Cmd = COMM_CMD_M2H_REPORT_STREAM_STATUS;
	Info.StreamStatus = StreamStatus;
	Module_EventUpLoad(&Info);
}

void UploadServerRawData(void)
{
	CVI_U8 Data[64] = {0};
	MODULE_UPLOAD_S Info = {0};

	Info.Comm_Cmd = COMM_CMD_M2H_DATA_TRANSFER;
	Info.RawData.Data = Data;
	Info.RawData.Datalen = sizeof(Data);
	Module_EventUpLoad(&Info);
}

void UploadGetKeyInfo(void)
{
	//查询钥匙信息
	MODULE_UPLOAD_S Info = {0};

	Info.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Info.Aliyun_Cmd = M2H_QUERY_KEYINFO;
	Info.KeyType = LOCK_KEY_PWD; //查询的钥匙类型
	Module_EventUpLoad(&Info);
}

void UploadAddKey(void)
{
	//添加钥匙
	MODULE_UPLOAD_S Info = {0};

	Info.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Info.Aliyun_Cmd = M2H_ADD_KEY;
	Info.KeyCfg.lockType = LOCK_KEY_FINGERPRINT; //添加普通指纹用户
	Info.KeyCfg.userLimit = LOCK_KEY_LIMIT_USER;
	Module_EventUpLoad(&Info);
}

void UploadDeleteKey(void)
{
	//删除钥匙
	MODULE_UPLOAD_S Info = {0};

	Info.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Info.Aliyun_Cmd = M2H_DELETE_KEY;
	Info.KeyCfg.keyID = 1;  //删除1号指纹用户
	Info.KeyCfg.lockType = LOCK_KEY_FINGERPRINT;
	Module_EventUpLoad(&Info);
}

void UploadGetTmpPasswd(void)
{
	//查询临时密码
	//从设置接口上报密码
	MODULE_UPLOAD_S Info = {0};

	Info.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Info.Aliyun_Cmd = M2H_QUERY_TMP_PWD;
	Module_EventUpLoad(&Info);
}

void UploadTmpPasswd(void)
{
	//更新临时密码状态
	MODULE_UPLOAD_S Info = {0};

	Info.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Info.Aliyun_Cmd = M2H_UPDATE_GENCFG;
	Info.PassST.tempPassWordId = 1;
	Info.PassST.tempPassWordStatus = LOCK_STATUS_NORMAL;
	Module_EventUpLoad(&Info);
}

void UploadUpdataGenCfg(void)
{
	//请求重新生成临时密码
	MODULE_UPLOAD_S Info = {0};

	Info.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Info.Aliyun_Cmd = M2H_SET_KEY_PER;
	Info.GenCfg = 1;
	Module_EventUpLoad(&Info);
}

void UploadSetKeyCfg(void)
{
	//设置钥匙权限
	MODULE_UPLOAD_S Info = {0};

	Info.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Info.Aliyun_Cmd = M2H_SET_KEY_PER;
	Info.KeyCfg.keyID = 1; //修改1号密码开锁用户为Root
	Info.KeyCfg.lockType = LOCK_KEY_PWD;
	Info.KeyCfg.userLimit = LOCK_KEY_LIMIT_ROOT;
	Module_EventUpLoad(&Info);
}

void UploadUpdataKeyInfo(void)
{
	//修改钥匙信息
	MODULE_UPLOAD_S Info = {0};
	Info.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Info.Aliyun_Cmd = M2H_MODIFY_KEY_INFO;

	Info.KeyNodeInfo.keyID = 1;
	Info.KeyNodeInfo.lockType = LOCK_KEY_PWD;
	memcpy(Info.KeyNodeInfo.KeyAlias, "CVI_Lock", strlen("CVI_Lock")); //设置别名
	Module_EventUpLoad(&Info); //修改1号密码开锁用户别名为CVI_LOCK
}

void UploadPowerOn()
{
	MODULE_UPLOAD_S Info = {0};

	Info.Comm_Cmd = COMM_CMD_M2H_REPORT_POWERON;
	Module_EventUpLoad(&Info);
}

void Upload_Set_Attribute_Setting()
{
	MODULE_UPLOAD_S Info = {0};
	CVI_U8 Data[32] = {[0 ... 31] = 0x01};
	Info.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Info.Aliyun_Cmd = M2H_SET_ATTRIBUTE_SETTING;

	Info.RawData.Data = Data;
	Info.RawData.Datalen = sizeof(Data);
	Module_EventUpLoad(&Info);
}

void Upload_Ack_AttributePack()
{
	MODULE_UPLOAD_S Info = {0};

	Info.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Info.Aliyun_Cmd = M2H_ACK_UPDATE_ATTRIBUTEPACK;
	Info.ErrorCode = RES_ERRORCODE_CMD_OK;
	Module_EventUpLoad(&Info);
}

void Upload_Set_ServerCommand()
{

	MODULE_UPLOAD_S Info = {0};

	Info.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Info.Aliyun_Cmd = M2H_SET_SERVER_COMMOAND;
	Info.RawData.Data = (CVI_U8 *)"SERVER";
	Info.RawData.Datalen = strlen("SERVER");
	Module_EventUpLoad(&Info);
}

void Upload_Ack_ServerPack()
{
	MODULE_UPLOAD_S Info = {0};

	Info.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Info.Aliyun_Cmd = M2H_ACK_SERVER_PACK;
	Info.ErrorCode = RES_ERRORCODE_CMD_OK; //0数据正确 1参数错误 2数据不完全需要重发
	Module_EventUpLoad(&Info);
}

void Upload_Ack_Event()
{
	MODULE_UPLOAD_S Info = {0};

	Info.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Info.Aliyun_Cmd = M2H_ACK_EVENT;
	Info.ErrorCode = RES_ERRORCODE_CMD_OK; //0数据正确 1参数错误 2数据不完全需要重发
	Module_EventUpLoad(&Info);
}

void Upload_Set_Upgrade()
{
	MODULE_UPLOAD_S Info = {0};

	Info.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Info.Aliyun_Cmd = M2H_SET_UPGRADE;
	Info.RawData.Datalen = 10 * 1024; //OTA bin包大小
	Module_EventUpLoad(&Info);
}

void Upload_Update_UpgradePack()
{
	MODULE_UPLOAD_S Info = {0};
	CVI_U8 Buf[512] = {0};

	Info.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Info.Aliyun_Cmd = M2H_UPDATE_UPGRADEPACK;
	Info.PackParm.packet_number = 1;
	Info.PackParm.packet_Seqnumber = 1; //大于512 请进行切包操作
	Info.PackParm.len = sizeof(Buf);
	Info.PackString = Buf;
	Module_EventUpLoad(&Info);
}

void Upload_Ack_ShadowDevice()
{
	MODULE_UPLOAD_S Info = {0};

	Info.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Info.Aliyun_Cmd = M2H_ACK_SHADOWDEVICE;
	Info.ErrorCode = RES_ERRORCODE_CMD_OK; //0数据正确 1未准备好
	Module_EventUpLoad(&Info);
}

void Upload_Ack_ShadowDevice_Update()
{
	MODULE_UPLOAD_S Info = {0};

	Info.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Info.Aliyun_Cmd = M2H_ACK_SHADOWDEVICE_UPDATE;
	Info.ErrorCode = RES_ERRORCODE_CMD_OK; //0数据正确 1需要重发
	Module_EventUpLoad(&Info);
}

void Upload_Set_ShadowDevice_Info()
{
	MODULE_UPLOAD_S Info = {0};
	CVI_U8 Data[512] = {0};

	Info.Comm_Cmd = COMM_CMD_H2M_RAWDATA_TRANSFER;
	Info.Aliyun_Cmd = M2H_SET_SHADOWDEVICE_INFO;
	Info.PackParm.packet = 1;//完整包
	Info.PackParm.packet_number = 1;
	Info.PackParm.packet_Seqnumber = 1;
	Info.PackParm.len = sizeof(Data);
	Info.PackString = Data;
	Module_EventUpLoad(&Info);
}

CVI_S32 ExVideo_VpssInit(void)
{

	VPSS_GRP_ATTR_S stGrpAttr = {0};
	VPSS_CHN_ATTR_S stChnAttr = {0};

	stGrpAttr.u32MaxW = 1920;
	stGrpAttr.u32MaxH = 1080;
	stGrpAttr.enPixelFormat = PIXEL_FORMAT_NV21;
	stGrpAttr.stFrameRate.s32SrcFrameRate = -1;
	stGrpAttr.stFrameRate.s32DstFrameRate = -1;
	stGrpAttr.u8VpssDev = 0;
	stChnAttr.u32Width                    = 1280;
	stChnAttr.u32Height                   = 720;
	stChnAttr.enVideoFormat               = VIDEO_FORMAT_LINEAR;
	stChnAttr.enPixelFormat               = PIXEL_FORMAT_NV21;
	stChnAttr.stFrameRate.s32SrcFrameRate = -1;
	stChnAttr.stFrameRate.s32DstFrameRate = -1;
	stChnAttr.bMirror                     = CVI_FALSE;
	stChnAttr.bFlip                       = CVI_FALSE;
	stChnAttr.u32Depth                    = 0;
	stChnAttr.stAspectRatio.enMode        = ASPECT_RATIO_NONE;
	stChnAttr.stNormalize.bEnable         = CVI_FALSE;
	if (CVI_VPSS_CreateGrp(SNAP_GRPINDEX, &stGrpAttr) != CVI_SUCCESS)
		EVENT_DISPATCH_DEBUG("CVI_VPSS_CreateGrp failed\n");
	if (CVI_VPSS_SetChnAttr(SNAP_GRPINDEX, SNAP_CHNINDEX, &stChnAttr) != CVI_SUCCESS)
		EVENT_DISPATCH_DEBUG("CVI_VPSS_SetChnAttr failed\n");
	if (CVI_VPSS_EnableChn(SNAP_GRPINDEX, SNAP_CHNINDEX) != CVI_SUCCESS)
		EVENT_DISPATCH_DEBUG("CVI_VPSS_EnableChn failed\n");
	if (CVI_VPSS_StartGrp(SNAP_GRPINDEX) != CVI_SUCCESS)
		EVENT_DISPATCH_DEBUG("CVI_VPSS_StartGrp failed\n");
	MMF_CHN_S stSrcChn;
	MMF_CHN_S stDestChn;
	VI_DEV ViDev = 0;
	VI_CHN ViChn = 0;

	stSrcChn.enModId = CVI_ID_VI;
	stSrcChn.s32DevId = ViDev;
	stSrcChn.s32ChnId = ViChn;

	stDestChn.enModId = CVI_ID_VPSS;
	stDestChn.s32DevId = SNAP_GRPINDEX;
	stDestChn.s32ChnId = SNAP_CHNINDEX;
	if (CVI_SYS_Bind(&stSrcChn, &stDestChn) != CVI_SUCCESS)
		EVENT_DISPATCH_DEBUG("CVI_SYS_Bind(VI-VPSS)\n");

	return CVI_SUCCESS;
}

CVI_S32 ExVideo_VencInit(const Venc_attr_s *attr)
{
	VENC_CHN_ATTR_S stAttr = {0};
	VENC_PARAM_MOD_S stModParam = {0};
	VENC_RC_PARAM_S stRcParam = {0};
	VENC_REF_PARAM_S stRefParam = {0};
	VENC_CHN_PARAM_S stChnParam = {0};
	VENC_ROI_ATTR_S stRoiAttr = {0};
	VENC_FRAMELOST_S stFrmLostParam = {0};
	VENC_SUPERFRAME_CFG_S stSuperFrmParam = {0};
	VENC_CU_PREDICTION_S stCuPrediction = {0};
	VENC_H265_TRANS_S stH265Trans = {0};
	VENC_H265_VUI_S stH265Vui = {0};
	VENC_H264_TRANS_S stH264Trans = {0};
	VENC_H264_ENTROPY_S stH264EntropyEnc = {0};
	VENC_H264_VUI_S stH264Vui = {0};
	VENC_RECV_PIC_PARAM_S stRecvParam = {0};
	MMF_CHN_S stSrcChn;
	MMF_CHN_S stDestChn;

	//stAttr.stVencAttr.enType = PT_JPEG,
	//stAttr.stVencAttr.u32MaxPicWidth = 1920;
	//stAttr.stVencAttr.u32MaxPicHeight = 1080;
	stAttr.stVencAttr.enType = attr->u32EncType;
	stAttr.stVencAttr.u32MaxPicWidth = attr->u32MaxWidth;
	stAttr.stVencAttr.u32MaxPicHeight = attr->u32MaxHeight;
	stAttr.stVencAttr.u32BufSize = 1024 * 1024;
	stAttr.stVencAttr.u32Profile = 0;
	stAttr.stVencAttr.bByFrame = CVI_TRUE;
	stAttr.stVencAttr.u32PicWidth = attr->u32Width;
	stAttr.stVencAttr.u32PicHeight = attr->u32Height;
	stAttr.stVencAttr.bSingleCore = 0;
	stAttr.stVencAttr.bEsBufQueueEn = 0;
	if (stAttr.stVencAttr.enType == PT_H264) {
		stAttr.stVencAttr.stAttrH264e.bSingleLumaBuf = 0;
		stAttr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
		stAttr.stRcAttr.stH264Cbr.bVariFpsEn = 0;
		stAttr.stRcAttr.stH264Cbr.fr32DstFrameRate = 25;
		stAttr.stRcAttr.stH264Cbr.u32BitRate = 128;
		stAttr.stRcAttr.stH264Cbr.u32Gop = 25;
		stAttr.stRcAttr.stH264Cbr.u32StatTime = 2;
	} else if (stAttr.stVencAttr.enType == PT_H265) {
		stAttr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
		stAttr.stRcAttr.stH265Cbr.bVariFpsEn = 0;
		stAttr.stRcAttr.stH265Cbr.fr32DstFrameRate = 25;
		stAttr.stRcAttr.stH265Cbr.u32BitRate = 128;
		stAttr.stRcAttr.stH265Cbr.u32Gop = 25;
		stAttr.stRcAttr.stH265Cbr.u32StatTime = 2;
	} else if (stAttr.stVencAttr.enType == PT_MJPEG) {
		stAttr.stRcAttr.enRcMode = VENC_RC_MODE_MJPEGCBR;
		stAttr.stRcAttr.stMjpegCbr.bVariFpsEn = 0;
		stAttr.stRcAttr.stMjpegCbr.fr32DstFrameRate = 25;
		stAttr.stRcAttr.stMjpegCbr.u32BitRate = 128;
		stAttr.stRcAttr.stMjpegCbr.u32StatTime = 2;
	}
	stAttr.stGopAttr.enGopMode = 0;
	stAttr.stGopAttr.stNormalP.s32IPQpDelta = 0;
	if (CVI_VENC_CreateChn(attr->s32DevChn, &stAttr))
		EVENT_DISPATCH_DEBUG("CVI_VENC_CreateChn\n");
	if (CVI_VENC_GetModParam(&stModParam))
		EVENT_DISPATCH_DEBUG("CVI_VENC_GetModParam\n");
	if (CVI_VENC_SetModParam(&stModParam))
		EVENT_DISPATCH_DEBUG("CVI_VENC_SetModParam\n");
	if (CVI_VENC_GetRcParam(attr->s32DevChn, &stRcParam))
		EVENT_DISPATCH_DEBUG("CVI_VENC_GetRcParam\n");
	stRcParam.s32FirstFrameStartQp = 30;
	stRcParam.s32InitialDelay = CVI_INITIAL_DELAY_DEFAULT;
	stRcParam.u32ThrdLv = 2;
	stRcParam.s32BgDeltaQp = 0;
	if (stAttr.stVencAttr.enType == PT_H264) {
		stRcParam.stParamH264Cbr.bQpMapEn = CVI_FALSE;
		stRcParam.stParamH264Cbr.s32MaxReEncodeTimes = 0;
		stRcParam.stParamH264Cbr.u32MaxIprop = 100;
		stRcParam.stParamH264Cbr.u32MinIprop = 1;
		stRcParam.stParamH264Cbr.u32MaxIQp = CVI_H26X_MAXQP_DEFAULT;
		stRcParam.stParamH264Cbr.u32MaxQp = CVI_H26X_MAXQP_DEFAULT;
		stRcParam.stParamH264Cbr.u32MinIQp = CVI_H26X_MINQP_DEFAULT;
		stRcParam.stParamH264Cbr.u32MinQp = CVI_H26X_MINQP_DEFAULT;
	} else if (stAttr.stVencAttr.enType == PT_H265) {
		stRcParam.stParamH265Cbr.bQpMapEn = CVI_FALSE;
		stRcParam.stParamH265Cbr.s32MaxReEncodeTimes = 0;
		stRcParam.stParamH265Cbr.u32MaxIprop = 100;
		stRcParam.stParamH265Cbr.u32MinIprop = 1;
		stRcParam.stParamH265Cbr.u32MaxIQp = CVI_H26X_MAXQP_DEFAULT;
		stRcParam.stParamH265Cbr.u32MaxQp = CVI_H26X_MAXQP_DEFAULT;
		stRcParam.stParamH265Cbr.u32MinIQp =  CVI_H26X_MINQP_DEFAULT;
		stRcParam.stParamH265Cbr.u32MinQp = CVI_H26X_MINQP_DEFAULT;
	} else if (stAttr.stVencAttr.enType == PT_MJPEG) {
		stRcParam.stParamMjpegCbr.u32MaxQfactor = 99;
		stRcParam.stParamMjpegCbr.u32MinQfactor = 1;
	}
	if (CVI_VENC_SetRcParam(attr->s32DevChn, &stRcParam) != CVI_SUCCESS) {
		EVENT_DISPATCH_DEBUG("CVI_VENC_SetRcParam\n");
		return -1;
	}
	if (CVI_VENC_GetRefParam(attr->s32DevChn, &stRefParam) != CVI_SUCCESS)
		EVENT_DISPATCH_DEBUG("CVI_VENC_GetRefParam\n");
	stRefParam.bEnablePred = 0;
	stRefParam.u32Base = 0;
	stRefParam.u32Enhance = 0;
	if (CVI_VENC_SetRefParam(attr->s32DevChn, &stRefParam) != CVI_SUCCESS)
		EVENT_DISPATCH_DEBUG("CVI_VENC_SetRefParam\n");
	if (CVI_VENC_GetChnParam(attr->s32DevChn, &stChnParam) != CVI_SUCCESS)
		EVENT_DISPATCH_DEBUG("CVI_VENC_GetChnParam\n");
	stChnParam.stFrameRate.s32SrcFrmRate = 25;
	stChnParam.stFrameRate.s32DstFrmRate = 25;
	if (CVI_VENC_SetChnParam(attr->s32DevChn, &stChnParam) != CVI_SUCCESS)
		EVENT_DISPATCH_DEBUG("CVI_VENC_SetChnParam\n");

	if (CVI_VENC_GetRoiAttr(attr->s32DevChn, 0, &stRoiAttr) != CVI_SUCCESS)
		EVENT_DISPATCH_DEBUG("CVI_VENC_GetRoiAttr\n");
	stRoiAttr.bEnable = CVI_FALSE;
	stRoiAttr.bAbsQp = CVI_FALSE;
	stRoiAttr.s32Qp = -2;
	stRoiAttr.u32Index = 0;
	if (CVI_VENC_SetRoiAttr(attr->s32DevChn, &stRoiAttr) != CVI_SUCCESS)
		EVENT_DISPATCH_DEBUG("CVI_VENC_SetRoiAttr\n");

	if (CVI_VENC_GetFrameLostStrategy(attr->s32DevChn, &stFrmLostParam) != CVI_SUCCESS)
		EVENT_DISPATCH_DEBUG("CVI_VENC_GetFrameLostStrategy\n");

	stFrmLostParam.bFrmLostOpen = 0;
	stFrmLostParam.enFrmLostMode = FRMLOST_PSKIP;
	stFrmLostParam.u32EncFrmGaps = 0;
	stFrmLostParam.u32FrmLostBpsThr = 0;
	if (CVI_VENC_SetFrameLostStrategy(attr->s32DevChn, &stFrmLostParam) != CVI_SUCCESS)
		EVENT_DISPATCH_DEBUG("CVI_VENC_SetFrameLostStrategy\n");

	if (CVI_VENC_GetSuperFrameStrategy(attr->s32DevChn, &stSuperFrmParam) != CVI_SUCCESS)
		EVENT_DISPATCH_DEBUG("CVI_VENC_GetSuperFrameStrategy\n");
	stSuperFrmParam.enSuperFrmMode = 0;
	if (CVI_VENC_SetSuperFrameStrategy(attr->s32DevChn, &stSuperFrmParam) != CVI_SUCCESS)
		EVENT_DISPATCH_DEBUG("CVI_VENC_SetSuperFrameStrategy\n");

	if (CVI_VENC_GetCuPrediction(attr->s32DevChn, &stCuPrediction) != CVI_SUCCESS)
		EVENT_DISPATCH_DEBUG("CVI_VENC_GetCuPrediction\n");
	stCuPrediction.u32IntraCost = 0;
	if (CVI_VENC_SetCuPrediction(attr->s32DevChn, &stCuPrediction) != CVI_SUCCESS)
		EVENT_DISPATCH_DEBUG("CVI_VENC_SetCuPrediction\n");

	if (stAttr.stVencAttr.enType == PT_H265) {
		if (CVI_VENC_GetH265Trans(attr->s32DevChn, &stH265Trans) != CVI_SUCCESS)
			EVENT_DISPATCH_DEBUG("CVI_VENC_GetH265Trans\n");
		stH265Trans.cb_qp_offset = 0;
		stH265Trans.cr_qp_offset = 0;
		if (CVI_VENC_SetH265Trans(attr->s32DevChn, &stH265Trans) != CVI_SUCCESS)
			EVENT_DISPATCH_DEBUG("CVI_VENC_SetH265Trans\n");

		if (CVI_VENC_GetH265Vui(attr->s32DevChn, &stH265Vui) != CVI_SUCCESS)
			EVENT_DISPATCH_DEBUG("CVI_VENC_GetH265Vui\n");
		stH265Vui.stVuiTimeInfo.timing_info_present_flag = 0;
		stH265Vui.stVuiAspectRatio.aspect_ratio_info_present_flag = 0;
		stH265Vui.stVuiBitstreamRestric.bitstream_restriction_flag = 0;
		stH265Vui.stVuiVideoSignal.video_signal_type_present_flag = 0;
		if (CVI_VENC_SetH265Vui(attr->s32DevChn, &stH265Vui) != CVI_SUCCESS)
			EVENT_DISPATCH_DEBUG("CVI_VENC_SetH265Vui\n");
	} else if (stAttr.stVencAttr.enType == PT_H264) {
		if (CVI_VENC_GetH264Trans(attr->s32DevChn, &stH264Trans) != CVI_SUCCESS)
			EVENT_DISPATCH_DEBUG("CVI_VENC_GetH264Trans\n");
		stH264Trans.chroma_qp_index_offset = 0;
		if (CVI_VENC_SetH264Trans(attr->s32DevChn, &stH264Trans) != CVI_SUCCESS)
			EVENT_DISPATCH_DEBUG("CVI_VENC_SetH264Trans\n");
		if (CVI_VENC_GetH264Entropy(attr->s32DevChn, &stH264EntropyEnc) != CVI_SUCCESS)
			EVENT_DISPATCH_DEBUG("CVI_VENC_GetH264Entropy\n");
		stH264EntropyEnc.u32EntropyEncModeI = stH264EntropyEnc.u32EntropyEncModeP = 0;
		if (CVI_VENC_SetH264Entropy(attr->s32DevChn, &stH264EntropyEnc) != CVI_SUCCESS)
			EVENT_DISPATCH_DEBUG("CVI_VENC_SetH264Entropy\n");

		if (CVI_VENC_GetH264Vui(attr->s32DevChn, &stH264Vui) != CVI_SUCCESS)
			EVENT_DISPATCH_DEBUG("CVI_VENC_GetH264Vui\n");
		stH264Vui.stVuiTimeInfo.timing_info_present_flag = 0;
		stH264Vui.stVuiAspectRatio.aspect_ratio_info_present_flag = 0;
		stH264Vui.stVuiBitstreamRestric.bitstream_restriction_flag = 0;
		stH264Vui.stVuiVideoSignal.video_signal_type_present_flag = 0;
		if (CVI_VENC_SetH264Vui(attr->s32DevChn, &stH264Vui) != CVI_SUCCESS)
			EVENT_DISPATCH_DEBUG("CVI_VENC_SetH264Vui\n");
	}
	if (attr->uBindMode == 1 && attr->s32DevChn == VENC_DEVINDEX) {
		stSrcChn.enModId = CVI_ID_VPSS;
		stSrcChn.s32ChnId = 0;
		stSrcChn.s32DevId = 0;
		stDestChn.enModId = CVI_ID_VENC;
		stDestChn.s32DevId = attr->s32DevChn;
		stDestChn.s32ChnId = 0;
		if (CVI_SYS_Bind(&stSrcChn, &stDestChn) != CVI_SUCCESS) {
			EVENT_DISPATCH_DEBUG("CVI_SYS_Bind\n");
			return -1;
		}
	}
	stRecvParam.s32RecvPicNum = -1;
	if (CVI_VENC_StartRecvFrame(attr->s32DevChn, &stRecvParam) != CVI_SUCCESS) {
		EVENT_DISPATCH_DEBUG("CVI_VENC_StartRecvFrame");
		return -1;
	}

	return CVI_SUCCESS;
}

void *MVencThread(void *argc)
{
	VIDEO_FRAME_INFO_S stFrameInfo = {0};
	VENC_CHN_STATUS_S stStatus = {0};
	VENC_STREAM_S stStream = {0};
	CVI_S32 ret = CVI_SUCCESS;

	prctl(PR_SET_NAME, "MVencThread", 0, 0, 0);
#if DUMP_BS
	int  i = 0;
	int  fd = -1;
	char FileName[64] = {0};
	VENC_PACK_S *ppack;
	static int FileCount = 0;

	snEVENT_DISPATCH_DEBUG(FileName, sizeof(FileName), SD_FATFS_MOUNTPOINT"/uartvideo%d", FileNumber);
	FileNumber++;
	fd = aos_open(FileName, O_CREAT | O_TRUNC | O_RDWR);
#endif
	while (MainVencRunStatus) {
		if (Vencattr.uBindMode == 0) {

			if (CVI_VPSS_GetChnFrame(0, 0, &stFrameInfo, 2000) != CVI_SUCCESS) {
				EVENT_DISPATCH_DEBUG("%s: VpssGetChnFrame Err\n", __func__);
				continue;
			}
			if (CVI_VENC_SendFrame(Vencattr.s32DevChn, &stFrameInfo, 2000) != CVI_SUCCESS) {
				EVENT_DISPATCH_DEBUG("%s: VENC_SendFrame Err\n", __func__);
				goto RELEASEVPSSFRAME;
			}
		}
		if (CVI_VENC_QueryStatus(Vencattr.s32DevChn, &stStatus) != CVI_SUCCESS) {
			EVENT_DISPATCH_DEBUG("%s: CVI_VENC_QueryStatus Err\n", __func__);
			goto RELEASEVPSSFRAME;
		}
		stStream.pstPack = malloc(sizeof(VENC_PACK_S) * stStatus.u32CurPacks);
		if (stStream.pstPack == NULL) {
			EVENT_DISPATCH_DEBUG("%s malloc pstPack memory failed\n", __func__);
			goto RELEASEVPSSFRAME;
		}
GET_STREAM:
		ret = CVI_VENC_GetStream(Vencattr.s32DevChn, &stStream, 2000);
		if (ret == CVI_ERR_VENC_BUSY) {
			goto GET_STREAM;
		} else if (ret != CVI_SUCCESS) {
			EVENT_DISPATCH_DEBUG("%s: VencGetStream ret %d\n", __func__, ret);
			goto RELEASEVPSSFRAME;
		}

RELEASEVPSSFRAME:
		if (Vencattr.uBindMode == 0)
			CVI_VPSS_ReleaseChnFrame(0, 0, &stFrameInfo);
#if DUMP_BS
		for (i = 0; i < stStream.u32PackCount; i++) {
			ppack = &stStream.pstPack[i];
			aos_write(fd, ppack->pu8Addr + ppack->u32Offset, ppack->u32Len - ppack->u32Offset);
		}
		if (FileCount % 50 == 0)
			EVENT_DISPATCH_DEBUG("Media Write\n");
		FileCount++;
#endif
		if (CVI_VENC_ReleaseStream(Vencattr.s32DevChn, &stStream) != CVI_SUCCESS)
			EVENT_DISPATCH_DEBUG("%s:CVI_VENC_ReleaseStream Err\n", __func__);
		if (stStream.pstPack != NULL) {
			free(stStream.pstPack);
			stStream.pstPack = NULL;
		}
	}
#if DUMP_BS
	EVENT_DISPATCH_DEBUG("VencMedia Close\n");
	aos_sync(fd);
	aos_close(fd);
#endif
	return CVI_SUCCESS;
}

CVI_S32 MainVideo_VencDestory(CVI_S32 DevChn)
{
	if (DevChn == 0 && MainVencRunStatus == 1) {
		MainVencRunStatus = 0;
		pthread_join(MainVencPthreadId, NULL);
		if (CVI_VENC_StopRecvFrame(DevChn) != CVI_SUCCESS) {
			EVENT_DISPATCH_DEBUG("%s:VencStopRecvFrame Err\n", __func__);
			return CVI_FAILURE;
		}
		if (CVI_VENC_ResetChn(DevChn) != CVI_SUCCESS) {
			EVENT_DISPATCH_DEBUG("%s:Venc_ResrtChn Err\n", __func__);
			return CVI_FAILURE;
		}
		if (CVI_VENC_DestroyChn(DevChn) != CVI_SUCCESS) {
			EVENT_DISPATCH_DEBUG("%s:Venc_DestroyChn Err\n", __func__);
			return CVI_FAILURE;
		}
	}
	return CVI_SUCCESS;
}

CVI_S32 MainVideo_VencInit(const Venc_attr_s *Vencattr)
{
	//设定主码流通道
	struct sched_param Schparm;
	pthread_attr_t pthread_attr;

	if (ExVideo_VencInit(Vencattr) != CVI_SUCCESS) {
		EVENT_DISPATCH_DEBUG("%s: VencInit Err\n", __func__);
		goto VIDEO_ERREXIT;
	}
	if (MainVencRunStatus == 0) {
		Schparm.sched_priority = 40;
		pthread_attr_init(&pthread_attr);
		pthread_attr_setschedpolicy(&pthread_attr, SCHED_RR);
		pthread_attr_setschedparam(&pthread_attr, &Schparm);
		pthread_attr_setinheritsched(&pthread_attr, PTHREAD_EXPLICIT_SCHED);
		pthread_attr_setstacksize(&pthread_attr, 8192);
		MainVencRunStatus = 1;
		if (pthread_create(&MainVencPthreadId, &pthread_attr, MVencThread, NULL) != CVI_SUCCESS) {
			EVENT_DISPATCH_DEBUG("%s:CreateThread Err\n", __func__);
			goto THREAD_ERREXIT;
		}
	}
	return CVI_SUCCESS;
THREAD_ERREXIT:
	pthread_attr_destroy(&pthread_attr);
VIDEO_ERREXIT:
	MainVideo_VencDestory(Vencattr->s32DevChn);
	return CVI_FAILURE;
}

void ExVideoInit(void)
{
	Venc_attr_s attr;
	attr.s32DevChn = 1;
	attr.u32EncType = PT_JPEG;
	attr.u32MaxWidth = 1920;
	attr.u32MaxHeight = 1080;
	attr.u32Width = 1280;
	attr.u32Height = 720;
	attr.uBindMode = 0;

	ExVideo_VpssInit();
	ExVideo_VencInit(&attr);
#if USER_UARTMAINVIDEO
	MainVideo_VencInit(&Vencattr);
#endif
}

void ExVideoDestroy(void)
{
	MMF_CHN_S stSrcChn;
	MMF_CHN_S stDestChn;
	VI_DEV ViDev = 0;
	VI_CHN ViChn = 0;

	CVI_VENC_StopRecvFrame(SNAP_VENCINDEX);
	CVI_VENC_ResetChn(SNAP_VENCINDEX);
	CVI_VENC_DestroyChn(SNAP_VENCINDEX);
	stSrcChn.enModId = CVI_ID_VI;
	stSrcChn.s32DevId = ViDev;
	stSrcChn.s32ChnId = ViChn;
	stDestChn.enModId = CVI_ID_VPSS;
	stDestChn.s32DevId = SNAP_GRPINDEX;
	stDestChn.s32ChnId = SNAP_CHNINDEX;
	if (CVI_SYS_UnBind(&stSrcChn, &stDestChn) != CVI_SUCCESS)
		EVENT_DISPATCH_DEBUG("%s : sysubind eer\n", __func__);
	if (CVI_VPSS_DisableChn(SNAP_GRPINDEX, SNAP_CHNINDEX) != CVI_SUCCESS)
		EVENT_DISPATCH_DEBUG("%s : VPSS_DisableChn eer\n", __func__);
	if (CVI_VPSS_DestroyGrp(SNAP_GRPINDEX) != CVI_SUCCESS)
		EVENT_DISPATCH_DEBUG("%s : VPSS_DestroyGrp eer\n", __func__);
#if USER_UARTMAINVIDEO
	MainVideo_VencDestory(VENC_DEVINDEX);
#endif
}

CVI_S32 Host2Module_Ack_ShadowDevice_Info(MODULE_PARAM_S *Parm)
{
	switch (Parm->ErrCode) {
	case RES_ERRORCODE_CMD_OK:
		EVENT_DISPATCH_DEBUG("%s : Requst Success\n", __func__);
		break;
	case RES_ERRORCODE_1:
		EVENT_DISPATCH_DEBUG("%s : Requst Err\n", __func__);
		break;
	default:
		EVENT_DISPATCH_DEBUG("%s : Default ErrCode %d\n", __func__, Parm->ErrCode);
		break;
	}
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_AliyunDataPrase(MODULE_PARAM_S *Parm)
{
	EVENT_DISPATCH_DEBUG(">>>>%s Cmd %d\n", __func__, Parm->Aliyun_Cmd);
	switch (Parm->Aliyun_Cmd) {
	case H2M_REPORT_KEYINFO:
		return Host2Module_TransferLockInfo(Parm);
	case H2M_REPORT_TMP_PWD:
		return Host2Module_TransferLocktmpPw(Parm);
	case H2M_ACK_ATTRIBUTE_SETTING:
		return Host2Module_AckAttributeSetting(Parm);
	case H2M_REPORT_ATTRIBUTEPACK:
		return Host2Module_AttributePack(Parm);
	case H2M_ACK_SERVER_COMMOAND:
		return Host2Module_AckServerCommand(Parm);
	case H2M_REPORT_SERVER_PACK:
		return Host2Module_ReportServerPack(Parm);
	case H2M_REPORT_EVENT:
		return Host2Module_ReportEvent(Parm);
	case H2M_ACK_UPGRADE:
		return Host2Module_AckUpgrade(Parm);
	case H2M_ACK_UPGRADEPACK:
		return Host2Module_AckUpgradePack(Parm);
	case H2M_REPORT_SHADOWDEVICE:
		return Host2Module_ReportShadowDevice(Parm);
	case H2M_REPORT_SHADOWDEVICE_UPDATE:
		return Host2Module_ReportShadowDeviceUpdate(Parm);
	case H2M_ACK_SHADOWDEVICE_INFO:
		return Host2Module_Ack_ShadowDevice_Info(Parm);
	default:
		break;
	}
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_Query_Software_Ver(MODULE_PARAM_S *Parm)
{
	EVENT_DISPATCH_DEBUG(">>>>%s\n", __func__);
#define Software_MAIN_VER 2
#define Software_SUBVER 9
	Parm->VerInfo.Main_Ver = Software_MAIN_VER;
	Parm->VerInfo.Sub_Ver = Software_SUBVER;
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_Query_Agreemt_Ver(MODULE_PARAM_S *Parm)
{
	EVENT_DISPATCH_DEBUG(">>>>%s\n", __func__);
#define Agreemt_PLATFOMR_VER 0x01
#define Agreemt_MAIN_VER 0x02
#define Agreemt_SUBVER 0x09
	Parm->VerInfo.Platform = Agreemt_PLATFOMR_VER;
	Parm->VerInfo.Main_Ver = Agreemt_MAIN_VER;
	Parm->VerInfo.Sub_Ver = Agreemt_SUBVER;
	return CVI_SUCCESS;
}





CVI_S32 Host2Module_RebootWifi()
{
#if (CONFIG_APP_HI3861_WIFI_SUPPORT ==1)
	WIFI_MSGINFO_S WifiInfo = {0};
	Wifi_HostMsgUpLoad(H2W_HOST_CMD_REBOOT, &WifiInfo);
#endif
	return CVI_SUCCESS;
}

CVI_S32 Host2Module_DataCallBack(MODULE_PARAM_S *Parm)
{
	switch (Parm->Comm_Cmd) {
	case COMM_CMD_H2M_QUERY_AGREEMT_VER:
		return Host2Module_Query_Agreemt_Ver(Parm);
	case COMM_CMD_H2M_SET_NETCONFIG:
		return Host2Module_SetNetConfig(Parm);
	case COMM_CMD_H2M_GET_NETCONFIG:
		return Host2Module_GetNetStatus(Parm);
	case COMM_CMD_H2M_GET_VIDEOINFO:
		return Host2Module_GetVideoInfo(Parm);
	case COMM_CMD_H2M_SET_VIDEOINFO:
		return Host2Module_SetVideoInfo(Parm);
	case COMM_CMD_H2M_GET_PICTUREINFO:
		return Host2Module_GetPictureInfo(Parm);
	case COMM_CMD_H2M_SET_PICTUREINFO:
		return  Host2Module_SetPictureInfo(Parm);
	case COMM_CMD_H2M_GET_AUDIOINFO:
		return Host2Module_GetAudioInfo(Parm);
	case COMM_CMD_H2M_SET_AUDIOINFO:
		return Host2Module_SetAudioInfo(Parm);
	case COMM_CMD_H2M_START_STREAM:
		return StartStream(Parm);
	case COMM_CMD_H2M_STOP_STREAM:
		return StopStream();
	case COMM_CMD_H2M_GET_STREAM_STATUS:
		return Host2Module_GetStreamStaus(Parm);
	case COMM_CMD_H2M_START_IPC_TALK:
		return StartTalkPlay(Parm);
	case COMM_CMD_H2M_STOP_IPC_TALK:
		return StopTalkPlay();
	case COMM_CMD_H2M_GET_IPC_TALK_STATUS:
		return Host2Module_GetIpcTalkStatus(Parm);
	case COMM_CMD_H2M_PIC_CAPTURE:
		return CapturePicture(Parm);
	case COMM_CMD_H2M_DATA_TRANSFER:
		return Host2Module_TransRawData(Parm);
	case COMM_CMD_H2M_GET_WIFIMAC:
		return Host2Module_GetModuleMac(Parm);
	case COMM_CMD_H2M_GET_ALIYUN_PARAM:
		return Host2Module_GetAliYunParm(Parm);
	case COMM_CMD_H2M_SET_ALIYUN_PARAM:
		return Host2Module_SetAliYunParm(Parm);
	case COMM_CMD_H2M_CLEAR_NETCONFIG:
		return Host2Module_CleanNetConfig();
	case COMM_CMD_H2M_FACTORY_RESET:
		return HostModule_FactoryReset();
	case COMM_CMD_H2M_RAWDATA_TRANSFER:
		return Host2Module_AliyunDataPrase(Parm);
	case COMM_CMD_H2M_QUERY_SOFTWARE_VER:
		return Host2Module_Query_Software_Ver(Parm);
	case COMM_CMD_H2M_START_SNAP_PICTURE:
		return SnapShot_Ready();
	case COMM_CMD_H2M_STOP_SMARTCONFIG:
		return Host2Module_RebootWifi();
	case COMM_CMD_H2M_SAVE_LOCKKEYLIST:

		return 1;
	default:
		break;
	}
	return CVI_SUCCESS;
}

void DoorbellModule_EventDispatchStart(void)
{
	MainVencRunStatus = 0;
	StreamStatus = 0;
	TalkStatus = 0;
	NetStatus = NET_NOT_CONFIG;
	//ExVideoInit();
	if (Event_dispatch_Init() == CVI_FAILURE) {
		EVENT_DISPATCH_DEBUG("Event_dispatch_Init error\n");
		return;
	}
	UploadPowerOn();
#if UPLOAD_MODE
	UploadServerRawData();
	UploadSteamStatus();
	UploadTalkStatus();
	UploadGetKeyInfo();
	UploadAddKey();
	UploadDeleteKey();
	UploadGetTmpPasswd();
	UploadTmpPasswd();
	UploadUpdataGenCfg();
	UploadSetKeyCfg();
	UploadUpdataKeyInfo();
	Upload_Set_Attribute_Setting();
	Upload_Set_ServerCommand();
	Upload_Set_Upgrade();
	Upload_Update_UpgradePack();
	Upload_Set_ShadowDevice_Info();
#endif
}

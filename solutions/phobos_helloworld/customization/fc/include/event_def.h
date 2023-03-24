/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name:
 * Description:
 *   ....
 */
#ifndef _EVENT_COMMON_DEFINE_H_
#define _EVENT_COMMON_DEFINE_H_
#include "cvi_type.h"

#define COMM_PACKET_HEAD_FIRST		(0xAA) // 帧第一标识
#define COMM_PACKET_HEAD_SECOND		(0x75) // 帧第二标识
#define COMM_PACKET_PID_H2M 	(0x01) //包标识 H2M
#define COMM_PACKET_PID_M2H 	(0x07) //包标识 M2H
#define COMM_PACKET_LEN			(2) //包长度
#define COMM_PACKET_CHECKSUM	(2) //校验和
#define COMM_PACKET_SIZE	(sizeof(H_M_COMM_CMD_PACK_T))

typedef enum {
	COMM_CMD_M2H_REPORT_POWERON = 0x01,              //启动上报
	COMM_CMD_H2M_QUERY_CATEYE_VER = 0x51,         //查询1820软件版本
	COMM_CMD_H2M_QUERY_AGREEMT_VER = 0xa0,            //查询平台对接协议版本
	COMM_CMD_H2M_SET_NETCONFIG = 0xa1,               // 配置网络参数
	COMM_CMD_H2M_GET_NETCONFIG = 0xa2,                // 获取网络状态
	COMM_CMD_H2M_GET_VIDEOINFO = 0xa3,                // 获取视频信息
	COMM_CMD_H2M_SET_VIDEOINFO = 0xa4,                // 设置视频信息
	COMM_CMD_H2M_GET_PICTUREINFO = 0xa5,              // 获取图片参数
	COMM_CMD_H2M_SET_PICTUREINFO = 0xa6,              // 设置图片参数
	COMM_CMD_H2M_GET_AUDIOINFO = 0xa7,                // 获取音频参数
	COMM_CMD_H2M_SET_AUDIOINFO = 0xa8,                // 设置音频参数
	COMM_CMD_H2M_PUT_HOSTID = 0xa9,            	// 传递门锁设备参数(废弃)
	COMM_CMD_H2M_START_STREAM = 0xaa,             // 锁端通知人脸模块启动推流
	COMM_CMD_H2M_STOP_STREAM = 0xab,              // 锁端通知人脸模块停止推流
	COMM_CMD_H2M_GET_STREAM_STATUS = 0xac,              // 查询推流状态
	COMM_CMD_M2H_REPORT_STREAM_STATUS = 0xad,    // 上报推流状态 FaceModule To Looker
	COMM_CMD_H2M_START_IPC_TALK = 0xae,                // 启动对讲
	COMM_CMD_H2M_STOP_IPC_TALK = 0xaf,                 // 停止对讲
	COMM_CMD_H2M_GET_IPC_TALK_STATUS = 0xb0,            // 查询对讲状态
	COMM_CMD_M2H_REPORT_IPC_TALK_STATUS = 0xb1,   // 上报对讲状态
	COMM_CMD_H2M_PIC_CAPTURE = 0xb2,                     //  抓拍图片
	COMM_CMD_H2M_DATA_TRANSFER = 0xb3,               //  锁端透传数据上报
	COMM_CMD_M2H_DATA_TRANSFER = 0xb4,            // 人脸接收服务器数据透传给门锁
	COMM_CMD_H2M_GET_WIFIMAC = 0xb5,            // 查询WIFI的MAC地址
	COMM_CMD_H2M_GET_ALIYUN_PARAM = 0xb6,             // 查询模块阿里云对接参数
	COMM_CMD_H2M_SET_ALIYUN_PARAM = 0xb7,             // 设置模块阿里云对接参数
	COMM_CMD_H2M_CLEAR_NETCONFIG = 0xb8, 		//清除wifi联网信息
	COMM_CMD_H2M_FACTORY_RESET = 0xb9,			//恢复出厂设置
	COMM_CMD_H2M_RAWDATA_TRANSFER = 0xbf,        // 指芯对接阿里平台半透传模块指令

	COMM_CMD_H2M_START_SNAP_PICTURE = 0xc0,
	COMM_CMD_H2M_STOP_SMARTCONFIG = 0xc1,
	COMM_CMD_H2M_SAVE_LOCKKEYLIST = 0xc2,

	COMM_CMD_H2M_QUERY_SOFTWARE_VER  = 0xf0    // 查询核心协议版本 即对接SmartLock版本 目前为2.9
} HOST_MODULE_COMM_CMD_E;

typedef enum {
	M2H_QUERY_KEYINFO = 0x01,  //查询门锁钥匙信息
	H2M_REPORT_KEYINFO = 0x02, //门锁钥匙信息上报
	M2H_ADD_KEY = 0x03, //添加钥匙
	M2H_DELETE_KEY = 0x04, //删除钥匙
	M2H_QUERY_TMP_PWD = 0x05, //查询临时密码
	H2M_REPORT_TMP_PWD = 0x06, //临时密码上报
	M2H_UPDATE_TMP_PWD_STATUS = 0x07, //更新临时密码的状态
	M2H_UPDATE_GENCFG   = 0X08, //重新生成临时密码
	M2H_SET_KEY_PER = 0x09, //设置钥匙的开锁权限
	M2H_MODIFY_KEY_INFO = 0x0a, //修改钥匙信息
	M2H_SET_ATTRIBUTE_SETTING = 0x0b, //下发属性设置
	H2M_ACK_ATTRIBUTE_SETTING = 0x0c, //下发属性回复
	H2M_REPORT_ATTRIBUTEPACK = 0x0d, //属性打包上传
	M2H_ACK_UPDATE_ATTRIBUTEPACK = 0x0e, //属性打包上传回复
	M2H_SET_SERVER_COMMOAND = 0X0f, //服务命令转发
	H2M_ACK_SERVER_COMMOAND   = 0x10, //服务命令回复
	H2M_REPORT_SERVER_PACK    = 0x11, //服务打包上传
	M2H_ACK_SERVER_PACK       = 0x12, //服务打包上传回复
	H2M_REPORT_EVENT          = 0x13, //事件上传
	M2H_ACK_EVENT             = 0x14, //事件上传回复
	M2H_SET_UPGRADE           = 0x15, //升级包确认
	H2M_ACK_UPGRADE           = 0x16, //升级包确认回复
	M2H_UPDATE_UPGRADEPACK    = 0x17, //升级包发送
	H2M_ACK_UPGRADEPACK       = 0x18, //升级包确认
	H2M_REPORT_SHADOWDEVICE   = 0x19, //影子设备上传更新
	M2H_ACK_SHADOWDEVICE      = 0x1a, //影子设备上传回复
	H2M_REPORT_SHADOWDEVICE_UPDATE = 0x1b, //影子设备更新
	M2H_ACK_SHADOWDEVICE_UPDATE = 0x1c, //影子设备更新回复
	M2H_SET_SHADOWDEVICE_INFO  = 0x1d, //影子设备数据更新转发
	H2M_ACK_SHADOWDEVICE_INFO  = 0x1e, //影子设备数据更新转发回复
} ALIYUN_CMD_TYPE_E;

typedef struct _AliYunCmdPack_s {
	CVI_U8 HeadReserver;
	CVI_U8 AliHeadCmd;
	CVI_U8 Reserver[2];
} __attribute__((packed)) ALIYUN_CMD_PACK_S;

typedef struct _H_M_CommCmdPack_s {
	CVI_U8 FrameHead[2];
	CVI_U8 CMD;
	CVI_U8 PackIdentification;
	CVI_U16 Length;
	CVI_U16 Sum;
} __attribute__((packed)) H_M_COMM_CMD_PACK_T;

CVI_S32 GetAudioInfoAck(void);
CVI_S32 GetPictureInfoAck(void);
CVI_S32 SetVideoInfoAck(CVI_U8 RetCode);
CVI_S32 GetVideoInfoAck(void);
CVI_S32 GetNetStatusAck(void);
CVI_S32 Query_AgreemtVer_Prase();
CVI_S32 GetSoftwareVerPrase();
CVI_S32 CleanNetconfigPrase();
CVI_S32 FactoryResetPrase();
CVI_S32 Get_CatEye_Version();
CVI_S32 SetPictureInfoPrase(CVI_U8 *PackBuf, CVI_U32 PackBufLength);
CVI_S32 SetAudioInfoPrase(CVI_U8 *PackBuf, CVI_U32 PackBufLength);
CVI_S32 SetVideoInfoPrase(CVI_U8 *PackBuf, CVI_U32 PackBufLength);
CVI_S32 ConfigNetCmdPrase(CVI_U8 *PackBuf, CVI_U16 PackBufLen);
CVI_S32 TransferLockMacPrase(CVI_U8 *PackBuf, CVI_U32 PackBufLength);
CVI_S32 StartStreamPushPrase(void);
CVI_S32 StopStreamPushPrase(void);
CVI_S32 GetStreamStausPrase(void);
CVI_S32 StartIpcTalkPrase(void);
CVI_S32 StopIpcTalkPrase(void);
CVI_S32 GetIpcTalkStatusPrase(void);
CVI_S32 Capture_Picture_Prase(CVI_U8 *PackBuf, CVI_U32 PackBufLength);
CVI_S32 DataTransferPrase(CVI_U8 *PackBuf, CVI_U32 PackBufLength);
CVI_S32 GetFaceModuleMacPrase(void);
CVI_S32 GetAliYunParmPrase(void);
CVI_S32 SetAliYunParmPrase(CVI_U8 *PackBuf, CVI_U32 PackBufLength);
CVI_S32 Get_SoftwareVer_Prase(void);
CVI_S32 TransferRawDataPrase(CVI_U8 *PackBuf, CVI_U32 PackBufLength);

CVI_S32 Snap_Picture();
CVI_S32 Reboot_wifi();



#endif



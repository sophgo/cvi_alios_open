/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: event_emulator.c
 * Description:
 *   ....
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <aos/kernel.h>
#include <aos/cli.h>
#include "event_def.h"
#include "hostmcu_emulator.h"
#include "event_dispatch.h"

#define RECVBUFFERMAXSIZE 20

typedef struct {
	CVI_U8 *Data;
	CVI_S32 DataLen;
} EMULATOR_TASKNODE_S;

typedef struct {
	EMULATOR_TASKNODE_S Node[RECVBUFFERMAXSIZE];
	CVI_S32 front;
	CVI_S32 near;
	pthread_mutex_t mutex_lock;
} EMULATOR_TASKINFO_S;
static EMULATOR_TASKINFO_S s_HostMcu_DataQueue;
static CVI_S32 s_HostMcu_EmulatorRecvRunStaus;
CVI_U8 McuEvent_COMM_CMD_M2H_REPORT_POWERON[] = {0x00}; //启动上报
CVI_U8 McuEvent_COMM_CMD_H2M_QUERY_AGREEMT_VER[] = {0xAA, 0x75, 0xa0, 0x01, 0x00, 0x00, 0xb5, 0x88}; //查询平台对接协议版本
CVI_U8 McuEvent_COMM_CMD_H2M_SET_NETCONFIG[] = {0xAA, 0x75, 0xA1, 0x01, 0x39, 0x00, 0x11, 0xDF, 0x01,
						0x54, 0x50, 0x5F, 0x4C, 0x49, 0x4E, 0x4B, 0x5F, 0x46, 0x37, 0x34, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7A, 0x68, 0x69, 0x78,
						0x69, 0x6E, 0x31, 0x32, 0x33, 0x34, 0x35, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						0x00, 0x00
					       };// 配置网络参数
CVI_U8 McuEvent_COMM_CMD_H2M_GET_NETCONFIG[] = {0xAA, 0x75, 0xa2, 0x01, 0x00, 0x00, 0xb3, 0x88}; // 获取网络状态
CVI_U8 McuEvent_COMM_CMD_H2M_GET_VIDEOINFO[] = {0xAA, 0x75, 0xA3, 0x01, 0x00, 0x00, 0xb2, 0x88}; // 获取视频信息
CVI_U8 McuEvent_COMM_CMD_H2M_SET_VIDEOINFO[] = {0xAA, 0x75, 0xA4, 0x01, 0x03, 0x00, 0xad, 0x88, 0x00, 0x00, 0x01}; // 设置视频信息
CVI_U8 McuEvent_COMM_CMD_H2M_GET_PICTUREINFO[] = {0xAA, 0x75, 0xA5, 0x01, 0x00, 0x00, 0xb0, 0x88}; // 获取图片参数
CVI_U8 McuEvent_COMM_CMD_H2M_SET_PICTUREINFO[] = {0xAA, 0x75, 0xA6, 0x01, 0x02, 0x00, 0xad, 0x87, 0x00, 0x01}; // 设置图片参数
CVI_U8 McuEvent_COMM_CMD_H2M_GET_AUDIOINFO[] = {0xAA, 0x75, 0xA7, 0x01, 0x00, 0x00, 0xae, 0x88}; // 获取音频参数
CVI_U8 McuEvent_COMM_CMD_H2M_SET_AUDIOINFO[] = {0xAA, 0x75, 0xA8, 0x01, 0x02, 0x00, 0xaa, 0x87, 0x01, 0x01}; // 设置音频参数
CVI_U8 McuEvent_COMM_CMD_H2M_PUT_HOSTID[] = {0x00}; // 传递门锁设备参数(废弃)
CVI_U8 McuEvent_COMM_CMD_H2M_START_STREAM[] = {0xAA, 0x75, 0xaa, 0x01, 0x00, 0x00, 0xab, 0x88}; // 锁端通知人脸模块启动推流
CVI_U8 McuEvent_COMM_CMD_H2M_STOP_STREAM[] = {0xAA, 0x75, 0xab, 0x01, 0x00, 0x00, 0xaa, 0x88}; // 锁端通知人脸模块停止推流
CVI_U8 McuEvent_COMM_CMD_H2M_GET_STREAM_STATUS[] = {0xAA, 0x75, 0xac, 0x01, 0x00, 0x00, 0xa9, 0x88}; // 查询推流状态
CVI_U8 McuEvent_COMM_CMD_M2H_REPORT_STREAM_STATUS[] = {0x00}; // 上报推流状态 FaceModule To Looker
CVI_U8 McuEvent_COMM_CMD_H2M_START_IPC_TALK[] = {0xAA, 0x75, 0xae, 0x01, 0x00, 0x00, 0xa7, 0x88}; // 启动对讲
CVI_U8 McuEvent_COMM_CMD_H2M_STOP_IPC_TALK[] = {0xAA, 0x75, 0xaf, 0x01, 0x00, 0x00, 0xa6, 0x88}; // 停止对讲
CVI_U8 McuEvent_COMM_CMD_H2M_GET_IPC_TALK_STATUS[] = {0xAA, 0x75, 0xb0, 0x01, 0x00, 0x00, 0xa5, 0x88}; // 查询对讲状态
CVI_U8 McuEvent_COMM_CMD_M2H_REPORT_IPC_TALK_STATUS[] = {0x00}; // 上报对讲状态
CVI_U8 McuEvent_COMM_CMD_H2M_PIC_CAPTURE[] = {0xAA, 0x75, 0xb2, 0x01, 0x08, 0x00, 0x6a, 0x54, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x31, 0x31}; //  抓拍图片
CVI_U8 McuEvent_COMM_CMD_H2M_DATA_TRANSFER[] = {0xAA, 0x75, 0xB3, 0x01, 0x4D, 0x00, 0x8A, 0x89, 0x7B, 0x22, 0x70, 0x72, 0x6F, 0x74, 0x6F, 0x63, 0x6F, 0x6C, 0x22, 0x3A, 0x22, 0x46, 0x44, 0x30, 0x32, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31, 0x46, 0x30, 0x30, 0x36, 0x43, 0x35, 0x43, 0x33, 0x44, 0x42, 0x32, 0x30, 0x30, 0x43, 0x36, 0x33, 0x30, 0x35, 0x37, 0x35, 0x35, 0x45, 0x36, 0x41, 0x39, 0x35, 0x46, 0x42, 0x39, 0x31, 0x42, 0x42, 0x34, 0x31, 0x39, 0x32, 0x30, 0x41, 0x39, 0x37, 0x37, 0x35, 0x32, 0x30, 0x33, 0x32, 0x33, 0x43, 0x35, 0x46, 0x45, 0x22, 0x7D}; //  锁端透传数据上报
CVI_U8 McuEvent_COMM_CMD_M2H_DATA_TRANSFER[] = {0x00}; // 人脸接收服务器数据透传给门锁
CVI_U8 McuEvent_COMM_CMD_H2M_GET_WIFIMAC[] = {0xAA, 0x75, 0xb5, 0x01, 0x00, 0x00, 0xa0, 0x88}; // 查询WIFI的MAC地址
CVI_U8 McuEvent_COMM_CMD_H2M_GET_ALIYUN_PARAM[] = {0xAA, 0x75, 0xb6, 0x01, 0x00, 0x00, 0x9f, 0x88}; // 查询模块阿里云对接参数
CVI_U8 McuEvent_COMM_CMD_H2M_SET_ALIYUN_PARAM[] = {0xAA, 0x75, 0xb7, 0x01, 0x0c, 0x00, 0xbb, 0xb1, 0x02, 0x31, 0x32, 0x02, 0x33, 0x34, 0x02, 0x35, 0x36, 0x02, 0x37, 0x38, 0x00, 0x00, 0x00
						   , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
						   , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
						   , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
						   , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
						   , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
						   , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
						  }; // 设置模块阿里云对接参数
CVI_U8 McuEvent_COMM_CMD_H2M_CLEAR_NETCONFIG[] = {0xAA, 0x75, 0xb8, 0x01, 0x00, 0x00, 0x9d, 0x88}; //清除wifi联网信息
CVI_U8 McuEvent_COMM_CMD_H2M_FACTORY_RESET[] = {0xAA, 0x75, 0xb9, 0x01, 0x00, 0x00, 0x9c, 0x88}; //恢复出厂设置
CVI_U8 McuEvent_COMM_CMD_H2M_RAWDATA_TRANSFER[] = {0x00}; // 指芯对接阿里平台半透传模块指令
CVI_U8 McuEvent_COMM_CMD_H2M_QUERY_SOFTWARE_VER[] = {0xAA, 0x75, 0xf0, 0x01, 0x00, 0x00, 0x65, 0x88}; // 查询核心协议版本 即对接SmartLock版本 目前为2.9

CVI_U8 McuEvent_M2H_QUERY_KEYINFO[] = {0x00};//查询门锁钥匙信息
CVI_U8 McuEvent_H2M_REPORT_KEYINFO[] = {0xAA, 0x75, 0xbf, 0x01, 0x4E, 0x00, 0x2c, 0x6c, 0x00, 0x02, 0x00, 0x00,
					0x02, 0x00, 0x01, 0x00, 0x01, 0x02, 0x41, 0x42, 0x43, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x02, 0x01, 0x46, 0x47, 0x48, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00
				       };//门锁钥匙信息上报
CVI_U8 McuEvent_M2H_ADD_KEY[] = {0x00};//添加钥匙
CVI_U8 McuEvent_M2H_DELETE_KEY[] = {0x00};//删除钥匙
CVI_U8 McuEvent_M2H_QUERY_TMP_PWD[] = {0x00};//查询临时密码
CVI_U8 McuEvent_H2M_REPORT_TMP_PWD[] = {0xAA, 0x75, 0xbf, 0x01, 0x15, 0x00, 0x42, 0x49,
					0x00, 0x06, 0x00, 0x00, 0x02, 0x01, 0x00, 0x31, 0x32, 0x33, 0x34,
					0x35, 0x36, 0x02, 0x01, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
				       }; //临时密码上报
CVI_U8 McuEvent_M2H_UPDATE_TMP_PWD_STATUS[] = {0x00}; //更新临时密码的状态
CVI_U8 McuEvent_M2H_UPDATE_GENCFG[] = {0x00};  //重新生成临时密码
CVI_U8 McuEvent_M2H_SET_KEY_PER[] = {0x00}; //设置钥匙的开锁权限
CVI_U8 McuEvent_M2H_MODIFY_KEY_INFO[] = {0x00}; //修改钥匙信息
CVI_U8 McuEvent_M2H_SET_ATTRIBUTE_SETTING[] = {0x00}; //下发属性设置
CVI_U8 McuEvent_H2M_ACK_ATTRIBUTE_SETTING[] = {0xAA, 0x75, 0xBF, 0x01, 0x05, 0x00, 0x91, 0x7c, 0x00, 0x0C, 0x00, 0x00, 0x00}; //下发属性回复
CVI_U8 McuEvent_H2M_REPORT_ATTRIBUTEPACK[] = {0xAA, 0x75, 0xBF, 0x01, 0x0A, 0x00, 0x24, 0x15, 0x00, 0x0D, 0x00, 0x00, 0x04, 0x00, 0x31, 0x32, 0x33, 0x34}; //属性打包上传
CVI_U8 McuEvent_M2H_ACK_UPDATE_ATTRIBUTEPACK[] = {0x00};//属性打包上传回复
CVI_U8 McuEvent_M2H_SET_SERVER_COMMOAND[] = {0x00}; //服务命令转发
CVI_U8 McuEvent_H2M_ACK_SERVER_COMMOAND[] = {0xAA, 0x75, 0xBF, 0x01, 0x05, 0x00, 0x91, 0x78, 0x00, 0x10, 0x00, 0x00, 0x00}; //服务命令回复
CVI_U8 McuEvent_H2M_REPORT_SERVER_PACK[] = {0xAA, 0x75, 0xBF, 0x01, 0x23, 0x00, 0xfa, 0xfe, 0x00, 0x11, 0x00, 0x00,
					    0x01, 0x00, 0x00, 0x00, 0x00, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x31,
					    0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x31, 0x32, 0x04, 0x00, 0x31, 0x32, 0x33, 0x34
					   };//服务打包上传
CVI_U8 McuEvent_M2H_ACK_SERVER_PACK[] = {0x00}; //服务打包上传回复
CVI_U8 McuEvent_H2M_REPORT_EVENT[] = {0xAA, 0x75, 0xBF, 0x01, 0x0A, 0x00, 0x26, 0x11, 0x00, 0x13, 0x00, 0x00, 0x04, 0x00, 0x31, 0x32, 0x31, 0x32}; //事件上传
CVI_U8 McuEvent_M2H_ACK_EVENT[] = {0x00}; //事件上传回复
CVI_U8 McuEvent_M2H_SET_UPGRADE[] = {0x00}; //升级包确认
CVI_U8 McuEvent_H2M_ACK_UPGRADE[] = {0xAA, 0x75, 0xBF, 0x01, 0x05, 0x00, 0x91, 0x72, 0x00, 0x16, 0x00, 0x00, 0x00}; //升级包确认回复
CVI_U8 McuEvent_M2H_UPDATE_UPGRADEPACK[] = {0x00}; //升级包发送
CVI_U8 McuEvent_H2M_ACK_UPGRADEPACK [] = {0xAA, 0x75, 0xBF, 0x01, 0x05, 0x00, 0x91, 0x70, 0x00, 0x18, 0x00, 0x00, 0x00}; //升级包确认
CVI_U8 McuEvent_H2M_REPORT_SHADOWDEVICE[] = {0xAA, 0x75, 0xBF, 0x01, 0x0B, 0x00, 0x24, 0x07, 0x00,
					     0x19, 0x00, 0x00, 0x01, 0x04, 0x00, 0x31, 0x32, 0x33, 0x34
					    }; //影子设备上传回复
CVI_U8 McuEvent_H2M_REPORT_SHADOWDEVICE_UPDATE[] = {0xAA, 0x75, 0xBF, 0x01, 0x04, 0x00, 0x92, 0x6d, 0x00, 0x1B, 0x00, 0x00}; //影子设备更新
CVI_U8 McuEvent_M2H_ACK_SHADOWDEVICE_UPDATE[] = {0x00}; //影子设备更新回复
CVI_U8 McuEvent_M2H_SET_SHADOWDEVICE_INFO[] = {0x00}; //影子设备数据更新转发
CVI_U8 McuEvent_H2M_ACK_SHADOWDEVICE_INFO[] = {0xAA, 0x75, 0xBF, 0x01, 0x05, 0x00, 0x91, 0x6a, 0x00, 0x1E, 0x00, 0x00, 0x00}; //影子设备数据更新转发回复

#define MCU_RECVCHECK(COMM_CMD) EmuRecvCheck_##COMM_CMD
#define MCU_EVENT(COMM_CMD) McuEvent_##COMM_CMD

CVI_U8 *McuEvent_GrpAliyunCmd(ALIYUN_CMD_TYPE_E EventId, CVI_S32 *pLen)
{
	CVI_U8 *pRet = NULL;
	CVI_S32 len = 0;
	switch (EventId) {
	case H2M_REPORT_KEYINFO:
		pRet = MCU_EVENT(H2M_REPORT_KEYINFO);
		len = sizeof(MCU_EVENT(H2M_REPORT_KEYINFO));
		break;
	case H2M_REPORT_TMP_PWD:
		pRet = MCU_EVENT(H2M_REPORT_TMP_PWD);
		len = sizeof(MCU_EVENT(H2M_REPORT_TMP_PWD));
		break;
	case H2M_ACK_ATTRIBUTE_SETTING:
		pRet = MCU_EVENT(H2M_ACK_ATTRIBUTE_SETTING);
		len = sizeof(MCU_EVENT(H2M_ACK_ATTRIBUTE_SETTING));
		break;
	case H2M_REPORT_ATTRIBUTEPACK:
		pRet = MCU_EVENT(H2M_REPORT_ATTRIBUTEPACK);
		len = sizeof(MCU_EVENT(H2M_REPORT_ATTRIBUTEPACK));
		break;
	case H2M_ACK_SERVER_COMMOAND:
		pRet = MCU_EVENT(H2M_ACK_SERVER_COMMOAND);
		len = sizeof(MCU_EVENT(H2M_ACK_SERVER_COMMOAND));
		break;
	case H2M_REPORT_SERVER_PACK:
		pRet = MCU_EVENT(H2M_REPORT_SERVER_PACK);
		len = sizeof(MCU_EVENT(H2M_REPORT_SERVER_PACK));
		break;
	case H2M_REPORT_EVENT:
		pRet = MCU_EVENT(H2M_REPORT_EVENT);
		len = sizeof(MCU_EVENT(H2M_REPORT_EVENT));
		break;
	case H2M_ACK_UPGRADE:
		pRet = MCU_EVENT(H2M_ACK_UPGRADE);
		len = sizeof(MCU_EVENT(H2M_ACK_UPGRADE));
		break;
	case H2M_ACK_UPGRADEPACK:
		pRet = MCU_EVENT(H2M_ACK_UPGRADEPACK);
		len = sizeof(MCU_EVENT(H2M_ACK_UPGRADEPACK));
		break;
	case H2M_REPORT_SHADOWDEVICE:
		pRet = MCU_EVENT(H2M_REPORT_SHADOWDEVICE);
		len = sizeof(MCU_EVENT(H2M_REPORT_SHADOWDEVICE));
		break;
	case H2M_REPORT_SHADOWDEVICE_UPDATE:
		pRet = MCU_EVENT(H2M_REPORT_SHADOWDEVICE_UPDATE);
		len = sizeof(MCU_EVENT(H2M_REPORT_SHADOWDEVICE_UPDATE));
		break;
	case H2M_ACK_SHADOWDEVICE_INFO:
		pRet = MCU_EVENT(H2M_ACK_SHADOWDEVICE_INFO);
		len = sizeof(MCU_EVENT(H2M_ACK_SHADOWDEVICE_INFO));
		break;
	case M2H_ACK_SHADOWDEVICE:
	case M2H_ACK_SHADOWDEVICE_UPDATE:
	case M2H_SET_SHADOWDEVICE_INFO:
	case M2H_UPDATE_UPGRADEPACK:
	case M2H_SET_UPGRADE:
	case M2H_ACK_EVENT:
	case M2H_ACK_SERVER_PACK:
	case M2H_SET_SERVER_COMMOAND:
	case M2H_ACK_UPDATE_ATTRIBUTEPACK:
	case M2H_QUERY_KEYINFO:
	case M2H_ADD_KEY:
	case M2H_DELETE_KEY:
	case M2H_QUERY_TMP_PWD:
	case M2H_UPDATE_TMP_PWD_STATUS:
	case M2H_UPDATE_GENCFG:
	case M2H_SET_KEY_PER:
	case M2H_MODIFY_KEY_INFO:
	case M2H_SET_ATTRIBUTE_SETTING:
	default:
		printf("Mcu Aliyun Not This Cmd\n");
		break;
	}
	*pLen = len;
	return pRet;
}

CVI_U8 *McuEvent_Grp(HOST_MODULE_COMM_CMD_E EventId, CVI_S32 *pLen)
{
	CVI_U8 *pRet = NULL;
	CVI_S32 len = 0;

	if (pLen == NULL)
		return NULL;
	switch (EventId) {
	case COMM_CMD_H2M_QUERY_AGREEMT_VER: //查询平台对接协议版本
		pRet = MCU_EVENT(COMM_CMD_H2M_QUERY_AGREEMT_VER);
		len = sizeof(MCU_EVENT(COMM_CMD_H2M_QUERY_AGREEMT_VER));
		break;
	case COMM_CMD_H2M_SET_NETCONFIG: // 配置网络参数
		pRet = MCU_EVENT(COMM_CMD_H2M_SET_NETCONFIG);
		len = sizeof(MCU_EVENT(COMM_CMD_H2M_SET_NETCONFIG));
		break;
	case COMM_CMD_H2M_GET_NETCONFIG: // 获取网络状态
		pRet = MCU_EVENT(COMM_CMD_H2M_GET_NETCONFIG);
		len = sizeof(MCU_EVENT(COMM_CMD_H2M_GET_NETCONFIG));
		break;
	case COMM_CMD_H2M_GET_VIDEOINFO: // 获取视频信息
		pRet = MCU_EVENT(COMM_CMD_H2M_GET_VIDEOINFO);
		len = sizeof(MCU_EVENT(COMM_CMD_H2M_GET_VIDEOINFO));
		break;
	case COMM_CMD_H2M_SET_VIDEOINFO: // 设置视频信息
		pRet = MCU_EVENT(COMM_CMD_H2M_SET_VIDEOINFO);
		len = sizeof(MCU_EVENT(COMM_CMD_H2M_SET_VIDEOINFO));
		break;
	case COMM_CMD_H2M_GET_PICTUREINFO: // 获取图片参数
		pRet = MCU_EVENT(COMM_CMD_H2M_GET_PICTUREINFO);
		len = sizeof(MCU_EVENT(COMM_CMD_H2M_GET_PICTUREINFO));
		break;
	case COMM_CMD_H2M_SET_PICTUREINFO: // 设置图片参数
		pRet = MCU_EVENT(COMM_CMD_H2M_SET_PICTUREINFO);
		len = sizeof(MCU_EVENT(COMM_CMD_H2M_SET_PICTUREINFO));
		break;
	case COMM_CMD_H2M_GET_AUDIOINFO: // 获取音频参数
		pRet = MCU_EVENT(COMM_CMD_H2M_GET_AUDIOINFO);
		len = sizeof(MCU_EVENT(COMM_CMD_H2M_GET_AUDIOINFO));
		break;
	case COMM_CMD_H2M_SET_AUDIOINFO: // 设置音频参数
		pRet = MCU_EVENT(COMM_CMD_H2M_SET_AUDIOINFO);
		len = sizeof(MCU_EVENT(COMM_CMD_H2M_SET_AUDIOINFO));
		break;
	case COMM_CMD_H2M_START_STREAM: // 锁端通知人脸模块启动推流
		pRet = MCU_EVENT(COMM_CMD_H2M_START_STREAM);
		len = sizeof(MCU_EVENT(COMM_CMD_H2M_START_STREAM));
		break;
	case COMM_CMD_H2M_STOP_STREAM: // 锁端通知人脸模块停止推流
		pRet = MCU_EVENT(COMM_CMD_H2M_STOP_STREAM);
		len = sizeof(MCU_EVENT(COMM_CMD_H2M_STOP_STREAM));
		break;
	case COMM_CMD_H2M_GET_STREAM_STATUS: // 查询推流状态
		pRet = MCU_EVENT(COMM_CMD_H2M_GET_STREAM_STATUS);
		len = sizeof(MCU_EVENT(COMM_CMD_H2M_GET_STREAM_STATUS));
		break;
	case COMM_CMD_H2M_START_IPC_TALK: // 启动对讲
		pRet = MCU_EVENT(COMM_CMD_H2M_START_IPC_TALK);
		len = sizeof(MCU_EVENT(COMM_CMD_H2M_START_IPC_TALK));
		break;
	case COMM_CMD_H2M_STOP_IPC_TALK: // 停止对讲
		pRet = MCU_EVENT(COMM_CMD_H2M_STOP_IPC_TALK);
		len = sizeof(MCU_EVENT(COMM_CMD_H2M_STOP_IPC_TALK));
		break;
	case COMM_CMD_H2M_GET_IPC_TALK_STATUS: // 查询对讲状态
		pRet = MCU_EVENT(COMM_CMD_H2M_GET_IPC_TALK_STATUS);
		len = sizeof(MCU_EVENT(COMM_CMD_H2M_GET_IPC_TALK_STATUS));
		break;
	case COMM_CMD_H2M_PIC_CAPTURE: //  抓拍图片
		pRet = MCU_EVENT(COMM_CMD_H2M_PIC_CAPTURE);
		len = sizeof(MCU_EVENT(COMM_CMD_H2M_PIC_CAPTURE));
		break;
	case COMM_CMD_H2M_DATA_TRANSFER: //  锁端透传数据上报
		pRet = MCU_EVENT(COMM_CMD_H2M_DATA_TRANSFER);
		len = sizeof(MCU_EVENT(COMM_CMD_H2M_DATA_TRANSFER));
		break;
	case COMM_CMD_H2M_GET_WIFIMAC: // 查询WIFI的MAC地址
		pRet = MCU_EVENT(COMM_CMD_H2M_GET_WIFIMAC);
		len = sizeof(MCU_EVENT(COMM_CMD_H2M_GET_WIFIMAC));
		break;
	case COMM_CMD_H2M_GET_ALIYUN_PARAM: // 查询模块阿里云对接参数
		pRet = MCU_EVENT(COMM_CMD_H2M_GET_ALIYUN_PARAM);
		len = sizeof(MCU_EVENT(COMM_CMD_H2M_GET_ALIYUN_PARAM));
		break;
	case COMM_CMD_H2M_SET_ALIYUN_PARAM: // 设置模块阿里云对接参数
		pRet = MCU_EVENT(COMM_CMD_H2M_SET_ALIYUN_PARAM);
		len = sizeof(MCU_EVENT(COMM_CMD_H2M_SET_ALIYUN_PARAM));
		break;
	case COMM_CMD_H2M_CLEAR_NETCONFIG: //清除wifi联网信息
		pRet = MCU_EVENT(COMM_CMD_H2M_CLEAR_NETCONFIG);
		len = sizeof(MCU_EVENT(COMM_CMD_H2M_CLEAR_NETCONFIG));
		break;
	case COMM_CMD_H2M_FACTORY_RESET: //恢复出厂设置
		pRet = MCU_EVENT(COMM_CMD_H2M_FACTORY_RESET);
		len = sizeof(MCU_EVENT(COMM_CMD_H2M_FACTORY_RESET));
		break;
	case COMM_CMD_H2M_RAWDATA_TRANSFER: // 指芯对接阿里平台半透传模块指令 指令太多这条暂时不用
		pRet = MCU_EVENT(COMM_CMD_H2M_RAWDATA_TRANSFER);
		len = sizeof(MCU_EVENT(COMM_CMD_H2M_RAWDATA_TRANSFER));
		break;
	case COMM_CMD_H2M_QUERY_SOFTWARE_VER: // 查询核心协议版本 即对接SmartLock版本 目前为2.9
		pRet = MCU_EVENT(COMM_CMD_H2M_QUERY_SOFTWARE_VER);
		len = sizeof(MCU_EVENT(COMM_CMD_H2M_QUERY_SOFTWARE_VER));
		break;
	case COMM_CMD_M2H_REPORT_POWERON: //上报类的方向为M2H 启动上报
	case COMM_CMD_M2H_REPORT_STREAM_STATUS:
	case COMM_CMD_M2H_REPORT_IPC_TALK_STATUS:
	case COMM_CMD_M2H_DATA_TRANSFER:
	case COMM_CMD_H2M_PUT_HOSTID: // 传递门锁设备参数(废弃)
	default:
		break;
	}
	*pLen = len;
	return pRet;
}

static CVI_S32 do_event_send_inputpro(HOST_MODULE_COMM_CMD_E EventId, ALIYUN_CMD_TYPE_E AliyunEventId, int argc,
				      char **argv, CVI_S32 *Len)
{
	CVI_U8 *PackData = NULL;
	if (argc < 3)
		return CVI_FAILURE;
	switch (EventId) {
	case COMM_CMD_H2M_SET_NETCONFIG:
		if (argc >= 5) {
			if (strlen(argv[3]) >= 32 || strlen(argv[3]) <= 0) {
				printf("%s ssid err use defalut\n", __func__);
				return CVI_FAILURE;
			} else if (strlen(argv[4]) >= 24) {
				printf("%s passwd err use defalut\n", __func__);
				return CVI_FAILURE;
			}
			PackData = &McuEvent_COMM_CMD_H2M_SET_NETCONFIG[8];
			CVI_U16 *pSum = (CVI_U16 *)&McuEvent_COMM_CMD_H2M_SET_NETCONFIG[6];
			memset(&McuEvent_COMM_CMD_H2M_SET_NETCONFIG[8], 0x00, sizeof(McuEvent_COMM_CMD_H2M_SET_NETCONFIG) - 8);
			PackData[0] = (CVI_U8) atoi(argv[2]); //configmode
			memcpy(&PackData[1], argv[3], strlen(argv[3]));
			memcpy(&PackData[33], argv[4], strlen(argv[4]));
			*pSum = 0;
			*pSum = Module_CheckSum(McuEvent_COMM_CMD_H2M_SET_NETCONFIG, sizeof(McuEvent_COMM_CMD_H2M_SET_NETCONFIG));
		}
		break;
	case COMM_CMD_H2M_SET_VIDEOINFO:
		if (argc >= 5) {
			if (strlen(argv[2]) <= 0 || strlen(argv[3]) <= 0 || strlen(argv[4]) <= 0) {
				printf("%s parm input err\n", __func__);
				return CVI_FAILURE;
			}
			PackData = &McuEvent_COMM_CMD_H2M_SET_VIDEOINFO[8];
			CVI_U16 *pSum = (CVI_U16 *)&McuEvent_COMM_CMD_H2M_SET_VIDEOINFO[6];
			memset(&McuEvent_COMM_CMD_H2M_SET_VIDEOINFO[8], 0x00, sizeof(McuEvent_COMM_CMD_H2M_SET_VIDEOINFO) - 8);
			PackData[0] = (CVI_U8) atoi(argv[2]);//video_encode
			PackData[1] = (CVI_U8) atoi(argv[3]);//framerate
			PackData[2] = (CVI_U8) atoi(argv[4]);//video_size
			*pSum = 0;
			*pSum = Module_CheckSum(McuEvent_COMM_CMD_H2M_SET_VIDEOINFO, sizeof(McuEvent_COMM_CMD_H2M_SET_VIDEOINFO));
		}
		break;
	case COMM_CMD_H2M_SET_AUDIOINFO:
		if (argc >= 4) {
			if (strlen(argv[2]) <= 0 || strlen(argv[3]) <= 0) {
				printf("%s parm input err\n", __func__);
				return CVI_FAILURE;
			}
			PackData = &McuEvent_COMM_CMD_H2M_SET_AUDIOINFO[8];
			CVI_U16 *pSum = (CVI_U16 *)&McuEvent_COMM_CMD_H2M_SET_AUDIOINFO[6];
			memset(&McuEvent_COMM_CMD_H2M_SET_AUDIOINFO[8], 0x00, sizeof(McuEvent_COMM_CMD_H2M_SET_AUDIOINFO) - 8);
			PackData[0] = (CVI_U8) atoi(argv[2]);//Audio_Enc
			PackData[1] = (CVI_U8) atoi(argv[3]);//Sample_Rate
			*pSum = 0;
			*pSum = Module_CheckSum(McuEvent_COMM_CMD_H2M_SET_AUDIOINFO, sizeof(McuEvent_COMM_CMD_H2M_SET_AUDIOINFO));
		}
		break;
	case COMM_CMD_H2M_SET_PICTUREINFO:
		if (argc >= 4) {
			if (strlen(argv[2]) <= 0 || strlen(argv[3]) <= 0) {
				printf("%s parm input err\n", __func__);
				return CVI_FAILURE;
			}
			PackData = &McuEvent_COMM_CMD_H2M_SET_PICTUREINFO[8];
			CVI_U16 *pSum = (CVI_U16 *)&McuEvent_COMM_CMD_H2M_SET_PICTUREINFO[6];
			memset(&McuEvent_COMM_CMD_H2M_SET_PICTUREINFO[8], 0x00, sizeof(McuEvent_COMM_CMD_H2M_SET_PICTUREINFO) - 8);
			PackData[0] = (CVI_U8) atoi(argv[2]);//Picture_Type
			PackData[1] = (CVI_U8) atoi(argv[3]);//Picture_Size
			*pSum = 0;
			*pSum = Module_CheckSum(McuEvent_COMM_CMD_H2M_SET_PICTUREINFO, sizeof(McuEvent_COMM_CMD_H2M_SET_PICTUREINFO));
		}
		break;
	case COMM_CMD_H2M_SET_ALIYUN_PARAM:
		if (argc >= 6) {
			*Len = 0;
			if (strlen(argv[2]) <= 0 || strlen(argv[3]) <= 0 || strlen(argv[4]) <= 0 || strlen(argv[5]) <= 0) {
				printf("%s parm input err\n", __func__);
				return CVI_FAILURE;
			}
			if (strlen(argv[2]) + strlen(argv[3]) + strlen(argv[4]) + strlen(argv[5]) + 4 >= sizeof(
				    McuEvent_COMM_CMD_H2M_SET_ALIYUN_PARAM) - 8) {
				printf("%s input size over Memory\n", __func__);
				return CVI_FAILURE;
			}
			CVI_U16 *pPackLen = (CVI_U16 *)&McuEvent_COMM_CMD_H2M_SET_ALIYUN_PARAM[4];
			PackData = &McuEvent_COMM_CMD_H2M_SET_ALIYUN_PARAM[8];
			CVI_U16 *pSum = (CVI_U16 *)&McuEvent_COMM_CMD_H2M_SET_ALIYUN_PARAM[6];
			memset(&McuEvent_COMM_CMD_H2M_SET_ALIYUN_PARAM[8], 0x00, sizeof(McuEvent_COMM_CMD_H2M_SET_ALIYUN_PARAM) - 8);
			CVI_U8 *pProduct_Secret = (CVI_U8 *)argv[2];
			CVI_U8 *pProduct_Key = (CVI_U8 *)argv[3];
			CVI_U8 *pDevice_Name = (CVI_U8 *)argv[4];
			CVI_U8 *pDevice_Secret = (CVI_U8 *)argv[5];
			CVI_U8 *pTmpPoint = PackData;
			CVI_U8 Product_SecretLen = (CVI_U8)strlen(argv[2]);
			CVI_U8 Product_KeyLen = (CVI_U8)strlen(argv[3]);
			CVI_U8 Device_NameLen = (CVI_U8)strlen(argv[4]);
			CVI_U8 Device_SecretLen = (CVI_U8)strlen(argv[5]);
			pTmpPoint[0] = Product_SecretLen + 1;
			memcpy(&pTmpPoint[1], pProduct_Secret, Product_SecretLen);
			pTmpPoint += Product_SecretLen + 2;
			*Len += Product_SecretLen + 1 + 1;

			pTmpPoint[0] = Product_KeyLen + 1;
			memcpy(&pTmpPoint[1], pProduct_Key, Product_KeyLen);
			pTmpPoint += Product_KeyLen + 2;
			*Len += Product_KeyLen + 1 + 1;

			pTmpPoint[0] = Device_NameLen + 1;
			memcpy(&pTmpPoint[1], pDevice_Name, Device_NameLen);
			pTmpPoint += Device_NameLen + 2;
			*Len += Device_NameLen + 1 + 1;

			pTmpPoint[0] = Device_SecretLen + 2;
			memcpy(&pTmpPoint[1], pDevice_Secret, Device_SecretLen);
			pTmpPoint += Device_SecretLen + 2;
			*Len += Device_SecretLen + 1 + 1;
			*pPackLen = *Len;
			*Len += COMM_PACKET_SIZE;
			*pSum = 0;
			*pSum = Module_CheckSum(McuEvent_COMM_CMD_H2M_SET_ALIYUN_PARAM, *Len);
		}
		break;
	case COMM_CMD_H2M_PIC_CAPTURE:
		if (argc == 3) {
			CVI_U32 *pEvent = (CVI_U32 *)&McuEvent_COMM_CMD_H2M_PIC_CAPTURE[8];
			CVI_U16 *pSum = (CVI_U16 *)&McuEvent_COMM_CMD_H2M_PIC_CAPTURE[6];
			*pEvent = atoi(argv[2]);
			*pSum = 0;
			*pSum = Module_CheckSum(McuEvent_COMM_CMD_H2M_PIC_CAPTURE, sizeof(McuEvent_COMM_CMD_H2M_PIC_CAPTURE));
			*Len = sizeof(McuEvent_COMM_CMD_H2M_PIC_CAPTURE);
		}
		break;
	default:
		break;
	}
	return CVI_SUCCESS;
}

static CVI_S32 inline do_event_send(HOST_MODULE_COMM_CMD_E EventId, ALIYUN_CMD_TYPE_E AliyunEventId, int argc,
				    char **argv)
{
	CVI_S32 Len = 0;
	CVI_U8 *GrpPoint = McuEvent_Grp(EventId, &Len);

	if (EventId == COMM_CMD_H2M_RAWDATA_TRANSFER && AliyunEventId != 0x00) {
		GrpPoint = McuEvent_GrpAliyunCmd(AliyunEventId, &Len);
	}
	if (GrpPoint) {
		do_event_send_inputpro(EventId, AliyunEventId, argc, argv, &Len);
		printf("do_event_send\r\n");
		return Module_ReadCallBack(GrpPoint, Len);
	} else
		printf("%s EventId : 0x%02x not used \n", __func__, EventId);
	return CVI_FAILURE;
}

CVI_S32 HostMcuEmu_InfoQueueIsEmpty()
{
	if (s_HostMcu_DataQueue.front == s_HostMcu_DataQueue.near)
		return CVI_SUCCESS;
	return CVI_FAILURE;
}

CVI_S32 HostMcuEmu_InfoQueueIsMax()
{
	if (s_HostMcu_DataQueue.front == (s_HostMcu_DataQueue.near + 1) % RECVBUFFERMAXSIZE)
		return CVI_SUCCESS;
	return CVI_FAILURE;
}

CVI_S32 HostMcuEmu_InfoQueuePushBack(CVI_U8 *buf, CVI_S32 buflen)
{
	if (s_HostMcu_EmulatorRecvRunStaus == CVI_SUCCESS)
		return CVI_SUCCESS;
	if (HostMcuEmu_InfoQueueIsMax() == CVI_SUCCESS)
		return CVI_FAILURE;
	pthread_mutex_lock(&s_HostMcu_DataQueue.mutex_lock);
	if (s_HostMcu_DataQueue.Node[s_HostMcu_DataQueue.near].Data) {
		free(s_HostMcu_DataQueue.Node[s_HostMcu_DataQueue.near].Data);
	}
	s_HostMcu_DataQueue.Node[s_HostMcu_DataQueue.near].Data = (CVI_U8 *)malloc(buflen);
	if (!s_HostMcu_DataQueue.Node[s_HostMcu_DataQueue.near].Data) {
		pthread_mutex_unlock(&s_HostMcu_DataQueue.mutex_lock);
		return CVI_FAILURE;
	}
	memcpy(s_HostMcu_DataQueue.Node[s_HostMcu_DataQueue.near].Data, buf, buflen);
	s_HostMcu_DataQueue.Node[s_HostMcu_DataQueue.near].DataLen = buflen;
	s_HostMcu_DataQueue.near = (s_HostMcu_DataQueue.near + 1) % RECVBUFFERMAXSIZE;
	pthread_mutex_unlock(&s_HostMcu_DataQueue.mutex_lock);
	return CVI_SUCCESS;
}

CVI_S32 HostMcuEmu_InfoQueuePop(CVI_U8 **buf, CVI_S32 *buflen)
{
	if ((HostMcuEmu_InfoQueueIsEmpty() == CVI_SUCCESS) || !buf)
		return CVI_FAILURE;
	pthread_mutex_lock(&s_HostMcu_DataQueue.mutex_lock);
	*buf  = s_HostMcu_DataQueue.Node[s_HostMcu_DataQueue.front].Data;
	*buflen = s_HostMcu_DataQueue.Node[s_HostMcu_DataQueue.front].DataLen;
	s_HostMcu_DataQueue.Node[s_HostMcu_DataQueue.front].Data = NULL;
	s_HostMcu_DataQueue.front = (s_HostMcu_DataQueue.front + 1) % RECVBUFFERMAXSIZE;
	pthread_mutex_unlock(&s_HostMcu_DataQueue.mutex_lock);
	return CVI_SUCCESS;
}

void EmuRecvCheck_COMM_CMD_H2M_QUERY_AGREEMT_VER(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData,
		CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];
	CVI_U8 *Data = &RecvData[1];
	printf("%s Code:0x%02x Data:0x%02x \n", __func__, *Code, *Data);
}

void EmuRecvCheck_COMM_CMD_M2H_REPORT_POWERON(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];
	CVI_U8 *Data = &RecvData[1];
	printf("%s Code:0x%02x Data:0x%02x \n", __func__, *Code, *Data);
}

void EmuRecvCheck_COMM_CMD_H2M_SET_NETCONFIG(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];
	printf("%s Code:0x%02x\n", __func__, *Code);
}

void EmuRecvCheck_COMM_CMD_H2M_GET_NETCONFIG(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];
	if (*Code == CVI_SUCCESS) {
		CVI_U8 *Netstatus = &RecvData[1];
		printf("%s Code:0x%02x Netstatus:0x%02x\n", __func__, *Code, *Netstatus);
	} else {
		printf("%s Code:0x%02x\n", __func__, *Code);
	}

}

void EmuRecvCheck_COMM_CMD_H2M_GET_VIDEOINFO(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];
	if (Code == CVI_SUCCESS) {
		VIDEO_CONFIG_INFO_S *pVideoCfg = (VIDEO_CONFIG_INFO_S *) &RecvData[1];
		printf("%s Code:0x%02x VideoCfg Encode :0x%02x Framerate: 0x%02x Video_Size: 0x%02x\n", __func__,
		       *Code, pVideoCfg->Video_Encode, pVideoCfg->Framerate, pVideoCfg->Video_Size);
	} else {
		printf("%s Code:0x%02x\n", __func__, *Code);
	}
}

void EmuRecvCheck_COMM_CMD_H2M_SET_VIDEOINFO(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];
	printf("%s Code:0x%02x\n", __func__, *Code);
}

void EmuRecvCheck_COMM_CMD_H2M_GET_PICTUREINFO(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s no data err\n", __func__);
	}
	CVI_U8 *Code = &RecvData[0];
	if (Code == CVI_SUCCESS) {
		PICTURE_CONFIG_INFO_E *pPicCfg = (PICTURE_CONFIG_INFO_E *) &RecvData[1];
		printf("%s Code:0x%02x PictureCfg Type :0x%02x Picture_Size: 0x%02x\n", __func__,
		       *Code, pPicCfg->Picture_Type, pPicCfg->Picture_Size);
	} else {
		printf("%s Code:0x%02x\n", __func__, *Code);
	}
}

void EmuRecvCheck_COMM_CMD_H2M_SET_PICTUREINFO(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];
	printf("%s Code:0x%02x\n", __func__, *Code);
}

void EmuRecvCheck_COMM_CMD_H2M_GET_AUDIOINFO(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];
	if (Code == CVI_SUCCESS) {
		AUDIO_CONFIG_INFO_S *pAudioCfg = (AUDIO_CONFIG_INFO_S *) &RecvData[1];
		printf("%s Code:0x%02x AudioCfg EncType :0x%02x Sample_Rate: 0x%02x\n", __func__,
		       *Code, pAudioCfg->Audio_EncType, pAudioCfg->Sample_Rate);
	} else {
		printf("%s Code:0x%02x\n", __func__, *Code);
	}
}

void EmuRecvCheck_COMM_CMD_H2M_SET_AUDIOINFO(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];
	printf("%s Code:0x%02x\n", __func__, *Code);
}

void EmuRecvCheck_COMM_CMD_H2M_START_STREAM(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];
	CVI_U16 *Duration = (CVI_U16 *)&RecvData[1];
	printf("%s Code:0x%02x Duration:0x%04x\n", __func__, *Code, *Duration);
}

void EmuRecvCheck_COMM_CMD_H2M_STOP_STREAM(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];
	printf("%s Code:0x%02x\n", __func__, *Code);
}

void EmuRecvCheck_COMM_CMD_H2M_GET_STREAM_STATUS(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData,
		CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];
	CVI_U8 *Status = &RecvData[1];
	printf("%s Code:0x%02x Status:0x%02x\n", __func__, *Code, *Status);
}

void EmuRecvCheck_COMM_CMD_M2H_REPORT_STREAM_STATUS(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData,
		CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];
	CVI_U8 *Status = &RecvData[1];
	printf("%s Code:0x%02x Status:0x%02x\n", __func__, *Code, *Status);
}

void EmuRecvCheck_COMM_CMD_H2M_START_IPC_TALK(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];
	CVI_U16 *Duration = (CVI_U16 *)&RecvData[1];
	printf("%s Code:0x%02x Duration:0x%04x\n", __func__, *Code, *Duration);
}

void EmuRecvCheck_COMM_CMD_H2M_STOP_IPC_TALK(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];
	printf("%s Code:0x%02x\n", __func__, *Code);
}

void EmuRecvCheck_COMM_CMD_H2M_GET_IPC_TALK_STATUS(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData,
		CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];
	CVI_U8 *Status = &RecvData[1];
	printf("%s Code:0x%02x Status:0x%02x\n", __func__, *Code, *Status);
}

void EmuRecvCheck_COMM_CMD_M2H_REPORT_IPC_TALK_STATUS(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData,
		CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];
	CVI_U8 *Status = &RecvData[1];
	printf("%s Code:0x%02x Status:0x%02x\n", __func__, *Code, *Status);
}

void EmuRecvCheck_COMM_CMD_H2M_PIC_CAPTURE(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];
	printf("%s Code:0x%02x\n", __func__, *Code);
}

void EmuRecvCheck_COMM_CMD_H2M_DATA_TRANSFER(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];
	printf("%s Code:0x%02x\n", __func__, *Code);
}

void EmuRecvCheck_COMM_CMD_M2H_DATA_TRANSFER(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];

	printf("%s Code:0x%02x\n", __func__, *Code);
	if (RecvDataLen >= 2) {
		CVI_U8 *pRawData = (CVI_U8 *)malloc(RecvDataLen + 1);

		if (pRawData) {
			memset(pRawData, 0, RecvDataLen + 1);
			memcpy(pRawData, RecvData + 1, RecvDataLen - 1);
			printf("%s RawData : %s\n", __func__, pRawData);
			free(pRawData);
		}
	}
	fflush(stdout);
}

void EmuRecvCheck_COMM_CMD_H2M_GET_WIFIMAC(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];

	if (*Code == CVI_SUCCESS) {
		CVI_U8 *Mac = &RecvData[1];
		printf("%s Code:0x%02x Mac :", __func__, *Code);
		for (int i = 0; i < RecvDataLen - 1; i++) {
			printf(" 0x%02x ", Mac[i]);
		}
		printf("\n");
	} else {
		printf("%s Code:0x%02x\n", __func__, *Code);
	}
	fflush(stdout);
}

void EmuRecvCheck_COMM_CMD_H2M_GET_ALIYUN_PARAM(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData,
		CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];

	if (*Code == CVI_SUCCESS) {
		CVI_CHAR Product_Secret[65] = {0};
		CVI_CHAR Device_Secret[65] = {0};
		CVI_CHAR Device_Name[65] = {0};
		CVI_CHAR Product_Key[65] = {0};
		CVI_U8 Product_Secret_Len = RecvData[1];
		CVI_U8 Product_Key_Len = RecvData[1 + 1 + Product_Secret_Len];
		CVI_U8 Device_Name_Len = RecvData[1 + 1 + Product_Secret_Len + 1 + Product_Key_Len];
		CVI_U8 Device_Secret_Len = RecvData[1 + 1 + Product_Secret_Len + 1 + Product_Key_Len + 1 + Device_Name_Len];

		memcpy(Product_Secret, &RecvData[1 + 1], Product_Secret_Len);
		memcpy(Product_Key, &RecvData[1 + 1 + Product_Secret_Len + 1], Product_Key_Len);
		memcpy(Device_Name, &RecvData[1 + 1 + Product_Secret_Len + 1 + Product_Key_Len + 1], Device_Name_Len);
		memcpy(Device_Secret, &RecvData[1 + 1 + Product_Secret_Len + 1 + Product_Key_Len + 1 + Device_Name_Len + 1],
		       Device_Secret_Len);
		printf("%s Code:0x%02x Product_Secret_Len: %d Product_Secret %s \n", __func__, *Code,
		       Product_Secret_Len, Product_Secret);
		printf("Device_Secret_Len %d Device_Secret %s Device_Name_Len %d Device_Name %s \n",
		       Device_Secret_Len,
		       Device_Secret, Device_Name_Len, Device_Name);
		printf("Product_Key_Len %d Product_Key %s\n", Product_Key_Len, Product_Key);
	} else {
		printf("%s Code:0x%02x\n", __func__, *Code);
	}
}

void EmuRecvCheck_COMM_CMD_H2M_SET_ALIYUN_PARAM(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData,
		CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];

	printf("%s Code:0x%02x\n", __func__, *Code);
}

void EmuRecvCheck_COMM_CMD_H2M_CLEAR_NETCONFIG(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];

	printf("%s Code:0x%02x\n", __func__, *Code);
}

void EmuRecvCheck_COMM_CMD_H2M_FACTORY_RESET(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];

	printf("%s Code:0x%02x\n", __func__, *Code);
}

void EmuRecvCheck_M2H_QUERY_KEYINFO(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *LockType = &RecvData[0];

	printf("%s LockType is 0x%02x\n", __func__, *LockType);
}

void EmuRecvCheck_M2H_ADD_KEY(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *LockType = &RecvData[0];
	CVI_U8 *userLimit = &RecvData[1];

	printf("%s LockType is 0x%02x userLimit 0x%02x\n", __func__, *LockType, *userLimit);
}

void EmuRecvCheck_M2H_DELETE_KEY(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U16 *keyID = (CVI_U16 *)&RecvData[0];
	CVI_U8 *userLimit = &RecvData[2];

	printf("%s keyID is 0x%04x userLimit 0x%02x\n", __func__, *keyID, *userLimit);
}

void EmuRecvCheck_M2H_QUERY_TMP_PWD(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	printf("%s\n", __func__);
}

void EmuRecvCheck_M2H_UPDATE_TMP_PWD_STATUS(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *tempPassWordId = &RecvData[0];
	CVI_U8 *tempPassWordStatus = &RecvData[1];

	printf("%s tempPassWordId 0x%02x tempPassWordStatus 0x%02x \n", __func__, *tempPassWordId, *tempPassWordStatus);
}

void EmuRecvCheck_M2H_UPDATE_GENCFG(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *tempPassWordNumber = &RecvData[0];

	printf("%s tempPassWordNumber 0x%02x\n", __func__, *tempPassWordNumber);
}

void EmuRecvCheck_M2H_SET_KEY_PER(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U16 *keyID = (CVI_U16 *)&RecvData[0];
	CVI_U8 *lockType = &RecvData[2];
	CVI_U8 *userLimit = &RecvData[3];

	printf("%s keyID 0x%02x lockType 0x%02x userLimit 0x%02x\n", __func__, *keyID, *lockType, *userLimit);
}

void EmuRecvCheck_M2H_MODIFY_KEY_INFO(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U16 *KeyID = (CVI_U16 *)&RecvData[0];
	CVI_U8 *lockType = &RecvData[2];
	CVI_U8 *KeyAlias = &RecvData[3];

	printf("%s keyID 0x%02x lockType 0x%02x KeyAlias %s\n", __func__,
	       *KeyID, *lockType, KeyAlias);
}

void EmuRecvCheck_M2H_SET_ATTRIBUTE_SETTING(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U16 *Len = (CVI_U16 *)&RecvData[0];
	CVI_CHAR *String = (CVI_CHAR *)&RecvData[2];
	printf("%s Len 0x%04x String :%s\n", __func__, *Len, String);
}

void EmuRecvCheck_M2H_ACK_UPDATE_ATTRIBUTEPACK(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];
	printf("%s Code 0x%02x\n", __func__, *Code);
}

void EmuRecvCheck_M2H_SET_SERVER_COMMOAND(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U16 *Len = (CVI_U16 *)&RecvData[0];
	CVI_CHAR *String = (CVI_CHAR *)&RecvData[2];
	printf("%s Len 0x%04x String :%s\n", __func__, *Len, String);
}

void EmuRecvCheck_M2H_ACK_SERVER_PACK(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];
	printf("%s Code 0x%02x\n", __func__, *Code);
}

void EmuRecvCheck_M2H_ACK_EVENT(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];
	printf("%s Code 0x%02x\n", __func__, *Code);
}

void EmuRecvCheck_M2H_SET_UPGRADE(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U16 *Len = (CVI_U16 *)&RecvData[0];
	printf("%s OTALen 0x%02x\n", __func__, *Len);
}

void EmuRecvCheck_M2H_UPDATE_UPGRADEPACK(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U16 *PackNumber = (CVI_U16 *)&RecvData[0];
	CVI_U16 *PackSeq = (CVI_U16 *)&RecvData[2];
	CVI_U16 *PackLen = (CVI_U16 *)&RecvData[4];
	printf("%s PackNumber 0x%04x PackSeq 0x%04x PackLen 0x%04x\n", __func__,
	       *PackNumber, *PackSeq, *PackLen);
}

void EmuRecvCheck_M2H_ACK_SHADOWDEVICE(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];
	printf("%s Code 0x%02x\n", __func__, *Code);
}

void EmuRecvCheck_M2H_ACK_SHADOWDEVICE_UPDATE(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *Code = &RecvData[0];
	printf("%s Code 0x%02x\n", __func__, *Code);
}

void EmuRecvCheck_M2H_SET_SHADOWDEVICE_INFO(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData, CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s Data Err\n", __func__);
		return ;
	}
	CVI_U8 *IsOnePack = &RecvData[0];
	CVI_U16 *pack_number = (CVI_U16 *)&RecvData[1];
	CVI_U16 *pack_seq = (CVI_U16 *)&RecvData[3];
	CVI_U16 *packlen = (CVI_U16 *)&RecvData[5];
	printf("%s IsOnePack 0x%02x pack_number 0x%04x pack_number 0x%04x pack_number 0x%04x\n", __func__,
	       *IsOnePack, *pack_number, *pack_seq, *packlen);
}

void EmuRecvCheck_COMM_CMD_H2M_RAWDATA_TRANSFER(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData,
		CVI_S32 SendLen)
{
	CVI_U8 *PackBuf = NULL;
	CVI_S32 PackBufLen = RecvDataLen - sizeof(ALIYUN_CMD_PACK_S);

	if (!RecvData) {
		printf("%s no data err\n", __func__);
	}
	//阿里解析
	ALIYUN_CMD_PACK_S *AliYunPack = (ALIYUN_CMD_PACK_S *)RecvData;
	PackBuf = RecvData + sizeof(ALIYUN_CMD_PACK_S);
	switch (AliYunPack->AliHeadCmd) {
	case M2H_QUERY_KEYINFO:
		MCU_RECVCHECK(M2H_QUERY_KEYINFO)(PackBuf, PackBufLen, SendData, SendLen);
		break;
	case M2H_ADD_KEY:
		MCU_RECVCHECK(M2H_ADD_KEY)(PackBuf, PackBufLen, SendData, SendLen);
		break;
	case M2H_DELETE_KEY:
		MCU_RECVCHECK(M2H_DELETE_KEY)(PackBuf, PackBufLen, SendData, SendLen);
		break;
	case M2H_QUERY_TMP_PWD:
		MCU_RECVCHECK(M2H_QUERY_TMP_PWD)(PackBuf, PackBufLen, SendData, SendLen);
		break;
	case M2H_UPDATE_TMP_PWD_STATUS:
		MCU_RECVCHECK(M2H_UPDATE_TMP_PWD_STATUS)(PackBuf, PackBufLen, SendData, SendLen);
		break;
	case M2H_UPDATE_GENCFG:
		MCU_RECVCHECK(M2H_UPDATE_GENCFG)(PackBuf, PackBufLen, SendData, SendLen);
		break;
	case M2H_SET_KEY_PER:
		MCU_RECVCHECK(M2H_SET_KEY_PER)(PackBuf, PackBufLen, SendData, SendLen);
		break;
	case M2H_MODIFY_KEY_INFO:
		MCU_RECVCHECK(M2H_MODIFY_KEY_INFO)(PackBuf, PackBufLen, SendData, SendLen);
		break;
	case M2H_SET_ATTRIBUTE_SETTING:
		MCU_RECVCHECK(M2H_SET_ATTRIBUTE_SETTING)(PackBuf, PackBufLen, SendData, SendLen);
		break;
	case M2H_ACK_UPDATE_ATTRIBUTEPACK:
		MCU_RECVCHECK(M2H_ACK_UPDATE_ATTRIBUTEPACK)(PackBuf, PackBufLen, SendData, SendLen);
		break;
	case M2H_SET_SERVER_COMMOAND:
		MCU_RECVCHECK(M2H_SET_SERVER_COMMOAND)(PackBuf, PackBufLen, SendData, SendLen);
		break;
	case M2H_ACK_SERVER_PACK:
		MCU_RECVCHECK(M2H_ACK_SERVER_PACK)(PackBuf, PackBufLen, SendData, SendLen);
		break;
	case M2H_ACK_EVENT:
		MCU_RECVCHECK(M2H_ACK_EVENT)(PackBuf, PackBufLen, SendData, SendLen);
		break;
	case M2H_SET_UPGRADE:
		MCU_RECVCHECK(M2H_SET_UPGRADE)(PackBuf, PackBufLen, SendData, SendLen);
		break;
	case M2H_UPDATE_UPGRADEPACK:
		MCU_RECVCHECK(M2H_UPDATE_UPGRADEPACK)(PackBuf, PackBufLen, SendData, SendLen);
		break;
	case M2H_ACK_SHADOWDEVICE:
		MCU_RECVCHECK(M2H_ACK_SHADOWDEVICE)(PackBuf, PackBufLen, SendData, SendLen);
		break;
	case M2H_ACK_SHADOWDEVICE_UPDATE:
		MCU_RECVCHECK(M2H_ACK_SHADOWDEVICE_UPDATE)(PackBuf, PackBufLen, SendData, SendLen);
		break;
	case M2H_SET_SHADOWDEVICE_INFO:
		MCU_RECVCHECK(M2H_SET_SHADOWDEVICE_INFO)(PackBuf, PackBufLen, SendData, SendLen);
		break;
	case H2M_ACK_SHADOWDEVICE_INFO:
	case H2M_REPORT_SHADOWDEVICE_UPDATE:
	case H2M_REPORT_SHADOWDEVICE:
	case H2M_ACK_UPGRADEPACK:
	case H2M_ACK_UPGRADE:
	case H2M_REPORT_EVENT:
	case H2M_REPORT_SERVER_PACK:
	case H2M_ACK_SERVER_COMMOAND:
	case H2M_REPORT_ATTRIBUTEPACK:
	case H2M_ACK_ATTRIBUTE_SETTING:
	case H2M_REPORT_TMP_PWD:
	case H2M_REPORT_KEYINFO:
	default:
		printf("Module Not This Cmd\n");
		break;
	}
}

void EmuRecvCheck_COMM_CMD_H2M_QUERY_SOFTWARE_VER(CVI_U8 *RecvData, CVI_S32 RecvDataLen, CVI_U8 *SendData,
		CVI_S32 SendLen)
{
	if (!RecvData) {
		printf("%s no data err\n", __func__);
	}
	CVI_U8 *Code = &RecvData[0];
	CVI_U8 *MainVer = &RecvData[1];
	CVI_U8 *SubVer = &RecvData[2];
	printf("%s Code:0x%02x MainVer:0x%02x  SubVer:0x%02x\n", __func__, *Code, *MainVer, *SubVer);
}

CVI_S32 HostEmu_Ack_Check(CVI_U8 *AckData, CVI_S32 DataLen)
{
	CVI_U8 *SendBuf = NULL;
	CVI_S32 SendLen = 0;
	CVI_U8 *PackBuf = NULL;
	CVI_S32 PackBufLen = 0;
	//校验校验和
	if (Module_CheckSum(AckData, DataLen) != CVI_SUCCESS) {
		printf("%s CheckSum err\n", __func__);
		return CVI_FAILURE;
	}
	//打印回复错误码带参数则打印参数和是否匹配
	H_M_COMM_CMD_PACK_T *PackHead = (H_M_COMM_CMD_PACK_T *)AckData;
	PackBuf = AckData + sizeof(H_M_COMM_CMD_PACK_T);
	PackBufLen = PackHead->Length;
	if (PackHead->FrameHead[0] != COMM_PACKET_HEAD_FIRST || PackHead->FrameHead[1] != COMM_PACKET_HEAD_SECOND) {
		printf("%s PackHead err\n", __func__);
		return CVI_FAILURE;
	}
	if (PackHead->PackIdentification != COMM_PACKET_PID_M2H) {
		printf("%s PackIdentification err\n", __func__);
		return CVI_FAILURE;
	}
	SendBuf = McuEvent_Grp(PackHead->CMD, &SendLen);
	printf("%s Cmd: 0x%02x Check\n", __func__, PackHead->CMD);
	switch (PackHead->CMD) {
	case COMM_CMD_M2H_REPORT_POWERON:
		MCU_RECVCHECK(COMM_CMD_M2H_REPORT_POWERON)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_H2M_QUERY_AGREEMT_VER:
		MCU_RECVCHECK(COMM_CMD_H2M_QUERY_AGREEMT_VER)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_H2M_SET_NETCONFIG:
		MCU_RECVCHECK(COMM_CMD_H2M_SET_NETCONFIG)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_H2M_GET_NETCONFIG:
		MCU_RECVCHECK(COMM_CMD_H2M_GET_NETCONFIG)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_H2M_GET_VIDEOINFO:
		MCU_RECVCHECK(COMM_CMD_H2M_GET_VIDEOINFO)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_H2M_SET_VIDEOINFO:
		MCU_RECVCHECK(COMM_CMD_H2M_SET_VIDEOINFO)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_H2M_GET_PICTUREINFO:
		MCU_RECVCHECK(COMM_CMD_H2M_GET_PICTUREINFO)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_H2M_SET_PICTUREINFO:
		MCU_RECVCHECK(COMM_CMD_H2M_SET_PICTUREINFO)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_H2M_GET_AUDIOINFO:
		MCU_RECVCHECK(COMM_CMD_H2M_GET_AUDIOINFO)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_H2M_SET_AUDIOINFO:
		MCU_RECVCHECK(COMM_CMD_H2M_SET_AUDIOINFO)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_H2M_START_STREAM:
		MCU_RECVCHECK(COMM_CMD_H2M_START_STREAM)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_H2M_STOP_STREAM:
		MCU_RECVCHECK(COMM_CMD_H2M_STOP_STREAM)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_H2M_GET_STREAM_STATUS:
		MCU_RECVCHECK(COMM_CMD_H2M_GET_STREAM_STATUS)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_M2H_REPORT_STREAM_STATUS:
		MCU_RECVCHECK(COMM_CMD_M2H_REPORT_STREAM_STATUS)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_H2M_START_IPC_TALK:
		MCU_RECVCHECK(COMM_CMD_H2M_START_IPC_TALK)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_H2M_STOP_IPC_TALK:
		MCU_RECVCHECK(COMM_CMD_H2M_STOP_IPC_TALK)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_H2M_GET_IPC_TALK_STATUS:
		MCU_RECVCHECK(COMM_CMD_H2M_GET_IPC_TALK_STATUS)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_M2H_REPORT_IPC_TALK_STATUS:
		MCU_RECVCHECK(COMM_CMD_M2H_REPORT_IPC_TALK_STATUS)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_H2M_PIC_CAPTURE:
		MCU_RECVCHECK(COMM_CMD_H2M_PIC_CAPTURE)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_H2M_DATA_TRANSFER:
		MCU_RECVCHECK(COMM_CMD_H2M_DATA_TRANSFER)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_M2H_DATA_TRANSFER:
		MCU_RECVCHECK(COMM_CMD_M2H_DATA_TRANSFER)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_H2M_GET_WIFIMAC:
		MCU_RECVCHECK(COMM_CMD_H2M_GET_WIFIMAC)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_H2M_GET_ALIYUN_PARAM:
		MCU_RECVCHECK(COMM_CMD_H2M_GET_ALIYUN_PARAM)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_H2M_SET_ALIYUN_PARAM:
		MCU_RECVCHECK(COMM_CMD_H2M_SET_ALIYUN_PARAM)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_H2M_CLEAR_NETCONFIG:
		MCU_RECVCHECK(COMM_CMD_H2M_CLEAR_NETCONFIG)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_H2M_FACTORY_RESET:
		MCU_RECVCHECK(COMM_CMD_H2M_FACTORY_RESET)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_H2M_RAWDATA_TRANSFER:
		MCU_RECVCHECK(COMM_CMD_H2M_RAWDATA_TRANSFER)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	case COMM_CMD_H2M_QUERY_SOFTWARE_VER:
		MCU_RECVCHECK(COMM_CMD_H2M_QUERY_SOFTWARE_VER)(PackBuf, PackBufLen, SendBuf, SendLen);
		break;
	default:
		break;
	}
	//对于OTA包这些 只校验回复校验和 内容无法进行校验
	return CVI_SUCCESS;
}

void *Emu_recv_task(void *argc)
{
	prctl(PR_SET_NAME, "Emu_recv_task");
	CVI_U8 *buf = NULL;
	CVI_S32 buflen = 0;

	while (s_HostMcu_EmulatorRecvRunStaus) {
		if (HostMcuEmu_InfoQueuePop(&buf, &buflen) == CVI_SUCCESS) {
			if (buf) {
				HostEmu_Ack_Check(buf, buflen);
				free(buf);
				buf = NULL;
			}
		}
		usleep(5 * 1000);
	}
	return NULL;
}

void Start_emu_recv_task()
{
	printf(">>>>%s\n", __func__);
	s_HostMcu_EmulatorRecvRunStaus = 1;
	pthread_t EmuRecv_Handle;
	pthread_attr_t pthread_attr;
	pthread_attr_init(&pthread_attr);
	pthread_attr_setinheritsched(&pthread_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setstacksize(&pthread_attr, 4096);
	pthread_create(&EmuRecv_Handle, &pthread_attr, Emu_recv_task, NULL);
}

void HostMcuEmu_Recv(int argc, char **argv)
{
	static CVI_S32 _emu_recv_tskinit = false;
	if (!_emu_recv_tskinit) {
		memset(&s_HostMcu_DataQueue, 0, sizeof(EMULATOR_TASKINFO_S));
		pthread_mutex_init(&s_HostMcu_DataQueue.mutex_lock, NULL);
		Start_emu_recv_task();
		_emu_recv_tskinit = true;
	} else {
		printf("emu recv task already init.\n");
	}
}

void HostMcuEmu_Send(int argc, char **argv)
{
	if (argc < 2) {
		printf("HostMcuEmu_Send Please Enter argc\n");
		printf("COMM_CMD_M2H_REPORT_POWERON :1 no input\n");
		printf("COMM_CMD_H2M_QUERY_AGREEMT_VER :160 no input\n");
		printf("COMM_CMD_H2M_SET_NETCONFIG :161 input: [ConfigMode] [SSID] [PASSWD]\n");
		printf("COMM_CMD_H2M_GET_NETCONFIG :162 no input\n");
		printf("COMM_CMD_H2M_GET_VIDEOINFO :163 no input\n");
		printf("COMM_CMD_H2M_SET_VIDEOINFO :164 input: [Video_Encode] [Framerate] [Video_Size]\n");
		printf("COMM_CMD_H2M_GET_PICTUREINFO :165 no input\n");
		printf("COMM_CMD_H2M_SET_PICTUREINFO :166 input :[Picture_Type] [Picture_Size] \n");
		printf("COMM_CMD_H2M_GET_AUDIOINFO :167 no input\n");
		printf("COMM_CMD_H2M_SET_AUDIOINFO :168 no input\n");
		printf("COMM_CMD_H2M_START_STREAM :170 no input\n");
		printf("COMM_CMD_H2M_STOP_STREAM  :171 no input\n");
		printf("COMM_CMD_H2M_GET_STREAM_STATUS :172 no input\n");
		printf("COMM_CMD_M2H_REPORT_STREAM_STATUS:173 no input\n");
		printf("COMM_CMD_H2M_START_IPC_TALK :174 no input\n");
		printf("COMM_CMD_H2M_STOP_IPC_TALK :175 no input\n");
		printf("COMM_CMD_H2M_GET_IPC_TALK_STATUS :176 no input\n");
		printf("COMM_CMD_H2M_PIC_CAPTURE :178 input: [event_type] record:60001 60002 60004 70002 70006 70007 other:capture picture\n");
		printf("COMM_CMD_H2M_DATA_TRANSFER: 179 no input\n");
		printf("COMM_CMD_H2M_GET_WIFIMAC :181 no input\n");
		printf("COMM_CMD_H2M_GET_ALIYUN_PARAM :182 no input\n");
		printf("COMM_CMD_H2M_SET_ALIYUN_PARAM :183 input [Product Secret] [Product Key] [Device Name] [Device Secret]\n");
		printf("COMM_CMD_H2M_CLEAR_NETCONFIG :184 no input\n");
		printf("COMM_CMD_H2M_FACTORY_RESET :185 no input\n");
		printf("COMM_CMD_H2M_RAWDATA_TRANSFER :191 no input\n");
		printf("COMM_CMD_H2M_QUERY_SOFTWARE_VER :240 no input\n");
		return;
	}
	CVI_U32 uVal = 0x00;
	CVI_U32 uAliyunVal = 0x00;
	uVal = atoi(argv[1]);
	if (argc == 3) {
		uAliyunVal = atoi(argv[2]);
	}
	do_event_send(uVal, uAliyunVal, argc, argv);
}

ALIOS_CLI_CMD_REGISTER(HostMcuEmu_Recv, host_mcu_recv, host mcu emu receive test);
ALIOS_CLI_CMD_REGISTER(HostMcuEmu_Send, host_mcu_send, host mcu emu send test);
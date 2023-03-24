/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name:
 * Description:
 *   ....
 */
#ifndef _UART_COMMUNICATION_H_
#define _UART_COMMUNICATION_H_
#include "cvi_type.h"

#define COMM_UART_IDX 1
#define COMM_UART_BAUDRATE 115200
#define COMM_UART_BLOCKTIME 40

typedef enum {
	UART_LOCKER = 0,
	UART_RECVCB_MAX
} UART_COMM_RECVCB_NAME_E;

CVI_S32 UartComm_Init(void);
CVI_S32 UartComm_Destroy(void);
CVI_S32 UartComm_Send(CVI_U8 *Buf, CVI_U32 BufLen, CVI_U32 u32BlockTimeMs);
CVI_S32 UartComm_RecvCbAttach(UART_COMM_RECVCB_NAME_E RecevierName, CVI_S32(*pfun)(CVI_U8 *, CVI_U32));
CVI_S32 UartComm_RecvCbUnattach(UART_COMM_RECVCB_NAME_E RecevierName);

#endif










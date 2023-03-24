
/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name:
 * Description:
 *   ....
 */
#include "mmio.h"
#include <pthread.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <drv/dma.h>
#include <drv/irq.h>
#include <drv/gpio.h>
#include <drv/pin.h>
#include <pin.h>
#include <drv/porting.h>
#include <soc.h>
#include <drv/tick.h>
#include "aos/hal/uart.h"
#include "uart_communication.h"


static uart_dev_t s_UartHandle;

static pthread_t s_RunTask_ThreadId;
static CVI_S32 s_RunStatus;
CVI_S32(*g_Uart_RecvCb[UART_RECVCB_MAX])(CVI_U8 *, CVI_U32);
#define EVBBOARD 0

CVI_S32 UartComm_RecvCbAttach(UART_COMM_RECVCB_NAME_E RecevierName, CVI_S32(*pfun)(CVI_U8 *, CVI_U32))
{
	if (RecevierName < 0 || RecevierName >= UART_RECVCB_MAX) {
		printf("%s : RecevierName err\n", __func__);
		return CVI_FAILURE;
	}
	if (!g_Uart_RecvCb[RecevierName]) {
		g_Uart_RecvCb[RecevierName] = pfun;
		return CVI_SUCCESS;
	}
	return CVI_FAILURE;
}

CVI_S32 UartComm_RecvCbUnattach(UART_COMM_RECVCB_NAME_E RecevierName)
{
	if (RecevierName < 0 || RecevierName >= UART_RECVCB_MAX) {
		printf("%s : RecevierName err\n", __func__);
		return CVI_SUCCESS;
	}
	if (g_Uart_RecvCb[RecevierName]) {
		g_Uart_RecvCb[RecevierName] = NULL;
		return CVI_SUCCESS;
	}
	return CVI_FAILURE;
}

void *Uart_RecvPro(void *args)
{
	prctl(PR_SET_NAME, "Uart_RecvPro");
	CVI_U8 ReadBuf[128] = {0};
	CVI_S32 i = 0;
	CVI_U32 ReadBufLen = 0;

	while (s_RunStatus) {
		ReadBufLen = 0;
		hal_uart_recv_II(&s_UartHandle, ReadBuf, sizeof(ReadBuf), &ReadBufLen, COMM_UART_BLOCKTIME);
		if (ReadBufLen > 0) {
			for (i = 0; i < UART_RECVCB_MAX; i++) {
				if (g_Uart_RecvCb[i])
					g_Uart_RecvCb[i](ReadBuf, ReadBufLen);
			}
		} else
			usleep(5 * 1000);
	}
	return 0;
}

CVI_S32 UartComm_PinMuxInit()
{
	return 0;
}

CVI_S32 UartComm_Init(void)
{
	struct sched_param sch_param;
	pthread_attr_t pthread_attr;

	//初始化 Uart1
	if (s_RunStatus == 0) {
		PINMUX_CONFIG(JTAG_CPU_TMS,UART1_TX);
		PINMUX_CONFIG(JTAG_CPU_TCK,UART1_RX);
		s_UartHandle.port                = COMM_UART_IDX;
		s_UartHandle.config.baud_rate    = COMM_UART_BAUDRATE;
		s_UartHandle.config.mode         = MODE_TX_RX;
		s_UartHandle.config.flow_control = FLOW_CONTROL_DISABLED;
		s_UartHandle.config.stop_bits    = STOP_BITS_1;
		s_UartHandle.config.parity       = NO_PARITY;
		s_UartHandle.config.data_width   = DATA_WIDTH_8BIT;

		if (hal_uart_init(&s_UartHandle) != CSI_OK) {
			printf("%s: hal_uart_init err\n", __func__);
			return CVI_FAILURE;
		}

		s_RunStatus = 1;
		sch_param.sched_priority = 40;
		pthread_attr_init(&pthread_attr);
		pthread_attr_setschedpolicy(&pthread_attr, SCHED_RR);
		pthread_attr_setschedparam(&pthread_attr, &sch_param);
		pthread_attr_setinheritsched(&pthread_attr, PTHREAD_EXPLICIT_SCHED);
		if (pthread_create(&s_RunTask_ThreadId, &pthread_attr, Uart_RecvPro, NULL) != CVI_SUCCESS) {
			printf("%s: threadcreate err\n", __func__);
			s_RunStatus = 0;
			return CVI_FAILURE;
		}
	}
	return CVI_SUCCESS;
}

CVI_S32 UartComm_Destroy(void)
{
	CVI_S32 i = 0;
	if (s_RunStatus == 1) {
		s_RunStatus = 0;
		pthread_join(s_RunTask_ThreadId, NULL);
		hal_uart_finalize(&s_UartHandle);
		for (i = 0; i < UART_RECVCB_MAX; i++) {
			if (g_Uart_RecvCb[i]) {
				g_Uart_RecvCb[i] = NULL;
				break;
			}
		}
	}
	return CVI_SUCCESS;
}

CVI_S32 UartComm_Send(CVI_U8 *Buf, CVI_U32 BufLen, CVI_U32 u32BlockTimeMs)
{
	return hal_uart_send(&s_UartHandle, Buf, BufLen, u32BlockTimeMs);
}

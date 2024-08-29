
/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name:
 * Description:
 *   ....
 */
#include "uart_communication.h"
#include <drv/dma.h>
#include <drv/gpio.h>
#include <drv/irq.h>
#include <drv/pin.h>
#include <drv/porting.h>
#include <drv/tick.h>
#include <pin.h>
#include <pthread.h>
#include <soc.h>
#include <sys/prctl.h>
#include <unistd.h>
#include "aos/hal/uart.h"
#include "mmio.h"

static uart_dev_t s_UartHandle;

static pthread_t s_RunTask_ThreadId;
static CVI_S32 s_RunStatus;
CVI_S32 (*g_Uart_RecvCb[UART_RECVCB_MAX])(CVI_U8*, CVI_U32);
#define EVBBOARD 0

CVI_S32 UartComm_RecvCbAttach(UART_COMM_RECVCB_NAME_E RecevierName,
                              CVI_S32 (*pfun)(CVI_U8*, CVI_U32))
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

void* Uart_RecvPro(void* args)
{
    prctl(PR_SET_NAME, "Uart_RecvPro");
    CVI_U8 ReadBuf[256] = {0};
    CVI_S32 i           = 0;
    CVI_U32 ReadBufLen  = 0;

    while (s_RunStatus) {
        ReadBufLen = 0;
        hal_uart_recv_II(&s_UartHandle, ReadBuf, sizeof(ReadBuf), &ReadBufLen, COMM_UART_BLOCKTIME);
        if (ReadBufLen > 0) {
            for (i = 0; i < UART_RECVCB_MAX; i++) {
                if (g_Uart_RecvCb[i]) {
                    g_Uart_RecvCb[i](ReadBuf, ReadBufLen);
                    memset(ReadBuf, 0, ReadBufLen);
                }
            }
        } else
            usleep(5 * 1000);
    }
    return 0;
}

CVI_S32 UartComm_PinMuxInit()
{
    // PINMUX_CONFIG(JTAG_CPU_TMS,UART1_TX);
    // PINMUX_CONFIG(JTAG_CPU_TCK,UART1_RX);
    PINMUX_CONFIG(IIC0_SCL, UART1_TX);
    PINMUX_CONFIG(IIC0_SDA, UART1_RX);
    return 0;
}

extern int aos_debug_printf(const char* fmt, ...);
CVI_S32 UartComm_Printf(CVI_U8* data, CVI_U32 len)
{
    aos_debug_printf("data(%d): ", len);
    for (int i = 0; i < len; i++)
        aos_debug_printf("0x%x ", data[i]);
    aos_debug_printf("\n");

    return 0;
}

CVI_S32 UartComm_Init(datalink_init_param_t* preq)
{
    struct sched_param sch_param;
    pthread_attr_t pthread_attr;

    // 初始化 Uart1
    if (s_RunStatus == 0) {
        // UartComm_PinMuxInit();
        s_UartHandle.port                = preq->id;
        s_UartHandle.config.baud_rate    = preq->baudrate;
        s_UartHandle.config.mode         = MODE_TX_RX;
        s_UartHandle.config.flow_control = FLOW_CONTROL_DISABLED;
        s_UartHandle.config.stop_bits    = STOP_BITS_1;
        s_UartHandle.config.parity       = NO_PARITY;
        s_UartHandle.config.data_width   = DATA_WIDTH_8BIT;

        if (hal_uart_init(&s_UartHandle) != CSI_OK) {
            printf("%s: hal_uart_init err\n", __func__);
            return CVI_FAILURE;
        }
        // UartComm_RecvCbAttach(UART_LOCKER, UartComm_Printf);
        s_RunStatus              = 1;
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

CVI_S32 UartComm_Send(CVI_U8* Buf, CVI_U32 BufLen, CVI_U32 u32BlockTimeMs)
{
    return hal_uart_send(&s_UartHandle, Buf, BufLen, u32BlockTimeMs);
}

void uart_send(int32_t argc, char** argv)
{
    if (argc <= 1 || argc > 128) {
        printf("invailed param\nusage: %s data[0-127]\n", argv[0]);
        return;
    }

    UartComm_Send((CVI_U8*)argv[1], argc * 2, COMM_UART_BLOCKTIME);
}

datalink_handles_t get_datalink_handles(void)
{
    datalink_handles_t uart_handles;
    uart_handles.init                   = UartComm_Init;
    uart_handles.deinit                 = UartComm_Destroy;
    uart_handles.send                   = UartComm_Send;
    uart_handles.register_recv_handle   = UartComm_RecvCbAttach;
    uart_handles.unregister_recv_handle = UartComm_RecvCbUnattach;
    return uart_handles;
}

ALIOS_CLI_CMD_REGISTER(uart_send, uart_send, uart_send);
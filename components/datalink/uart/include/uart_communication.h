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

#define COMM_UART_BLOCKTIME 100

typedef enum { UART_LOCKER = 0, UART_RECVCB_MAX } UART_COMM_RECVCB_NAME_E;

typedef struct __datalink_init_param_t {
    uint32_t id;
    uint32_t baudrate;
} datalink_init_param_t;

typedef struct __datalink_handles_t {
    /* datalink的初始化接口 */
    int32_t (*init)(datalink_init_param_t* preq);
    /* datalink的反初始化接口 */
    int32_t (*deinit)(void);
    /* 向datalink发送数据的接口 */
    int32_t (*send)(uint8_t* buf, uint32_t buf_len, uint32_t timeout);
    /* datalink需要实现给协议层注册接受数据的回调函数接口, id表示回调函数的槽的索引 */
    int32_t (*register_recv_handle)(uint32_t id, int32_t (*pfun)(uint8_t*, uint32_t));
    int32_t (*unregister_recv_handle)(uint32_t id);
} datalink_handles_t;

datalink_handles_t get_datalink_handles(void);

// CVI_S32 UartComm_Init(void);
// CVI_S32 UartComm_Destroy(void);
// CVI_S32 UartComm_Send(CVI_U8 *Buf, CVI_U32 BufLen, CVI_U32 u32BlockTimeMs);
// CVI_S32 UartComm_RecvCbAttach(UART_COMM_RECVCB_NAME_E RecevierName, CVI_S32(*pfun)(CVI_U8 *,
// CVI_U32)); CVI_S32 UartComm_RecvCbUnattach(UART_COMM_RECVCB_NAME_E RecevierName);

#endif
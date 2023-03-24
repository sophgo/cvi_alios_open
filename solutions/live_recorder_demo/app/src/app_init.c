/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */
#include <stdbool.h>
#include <board.h>
#include <board_config.h>
#include <yoc/partition.h>
#include <yoc/init.h>
#include <uservice/uservice.h>
#include <cvi_misc.h>
#include "app_main.h"
#include <drv/dma.h>

#define TAG "init"
#define CONFIG_CONSOLE_UART_BUFSIZE (512)

static void stduart_init(void)
{
    extern void console_init(int idx, uint32_t baud, uint16_t buf_size);
    console_init(CONSOLE_UART_IDX, CONFIG_CLI_USART_BAUD, CONFIG_CONSOLE_UART_BUFSIZE);
}

void board_yoc_init(void)
{
    int ret;

    extern void  cxx_system_init(void);
    cxx_system_init();

    stduart_init();
    board_init();

    ulog_init();

#ifdef CONFIG_DEBUG
    aos_set_log_level(AOS_LL_DEBUG);
#else
    aos_set_log_level(AOS_LL_WARN);
#endif

    CVI_EFUSE_EnableFastBoot();
    ret = CVI_EFUSE_IsFastBootEnabled();
    if (ret == 1) {
        LOGD(TAG, "fast boot enabled");
    }

    ret = partition_init();
    if (ret <= 0) {
        LOGE(TAG, "partition_init failed(%d).\n", ret);
        return;
    }
    event_service_init(NULL);
    return;
}

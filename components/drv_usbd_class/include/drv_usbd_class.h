/*
 * Copyright (C) 2022 Alibaba Group Holding Limited
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Register rndis device driver
 * @return      void
 */
void rndis_device_init();

/**
 * init cdc acm uart device driver
 * @return      void
 */
void cdc_acm_uart_drv_init(uint8_t idx);

/**
 * uninit cdc acm uart device driver
 * @return      void
 */
void cdc_acm_uart_drv_uninit();

#ifdef __cplusplus
}
#endif
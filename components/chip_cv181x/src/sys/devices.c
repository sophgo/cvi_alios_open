/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     devices.c
 * @brief    source file for the devices
 * @version  V1.0
 * @date     2019-12-18
******************************************************************************/

#include <stdio.h>
#include <csi_config.h>
#include <soc.h>
#include <drv/uart.h>
#include <drv/timer.h>
#include <drv/dma.h>
#include <drv/pin.h>
#include <drv/i2s.h>
#include <cvi_dma_ll.h>
#include <dw_timer_ll.h>
#include <cvi_rtc.h>

const csi_perip_info_t g_soc_info[] = {
    {DW_GPIO0_BASE,            DW_GPIO0_IRQn,            0,    DEV_DW_GPIO_TAG},
    {DW_GPIO1_BASE,            DW_GPIO1_IRQn,            1,    DEV_DW_GPIO_TAG},
    {DW_GPIO2_BASE,            DW_GPIO2_IRQn,            2,    DEV_DW_GPIO_TAG},
    {DW_GPIO3_BASE,            DW_GPIO3_IRQn,            3,    DEV_DW_GPIO_TAG},
    {DW_RTC_GPIO_BASE,         DW_RTC_GPIO_IRQn,         4,    DEV_DW_GPIO_TAG},
    {DW_UART0_BASE,            DW_UART0_IRQn,            0,    DEV_DW_UART_TAG},
    {DW_UART1_BASE,            DW_UART1_IRQn,            1,    DEV_DW_UART_TAG},
    {DW_UART2_BASE,            DW_UART2_IRQn,            2,    DEV_DW_UART_TAG},
    {DW_UART3_BASE,            DW_UART3_IRQn,            3,    DEV_DW_UART_TAG},
    {DW_UART4_BASE,            DW_UART4_IRQn,            4,    DEV_DW_UART_TAG},
    {DW_DMA_BASE,              DW_DMA_IRQn,              0,    DEV_DW_DMA_TAG},
    {DW_TIMER0_BASE,           TIMER_INTR_0,             0,    DEV_DW_TIMER_TAG},
    {DW_TIMER1_BASE,           TIMER_INTR_1,             1,    DEV_DW_TIMER_TAG},
    {DW_TIMER2_BASE,           TIMER_INTR_2,             2,    DEV_DW_TIMER_TAG},
    {DW_TIMER3_BASE,           TIMER_INTR_3,             3,    DEV_DW_TIMER_TAG},
    {DW_TIMER4_BASE,           TIMER_INTR_4,             4,    DEV_DW_TIMER_TAG},
    {DW_TIMER5_BASE,           TIMER_INTR_5,             5,    DEV_DW_TIMER_TAG},
    {DW_TIMER6_BASE,           TIMER_INTR_6,             6,    DEV_DW_TIMER_TAG},
    {DW_TIMER7_BASE,           TIMER_INTR_7,             7,    DEV_DW_TIMER_TAG},
    {CVI_RTC_BASE,             RTC_ALARM_O,              0,    DEV_CVI_RTC_TAG},
    {DW_IIC0_BASE,             DW_IIC0_IRQn,             0,    DEV_DW_IIC_TAG},
    {DW_IIC1_BASE,             DW_IIC1_IRQn,             1,    DEV_DW_IIC_TAG},
    {DW_IIC2_BASE,             DW_IIC2_IRQn,             2,    DEV_DW_IIC_TAG},
    {DW_IIC3_BASE,             DW_IIC3_IRQn,             3,    DEV_DW_IIC_TAG},
    {DW_IIC4_BASE,             DW_IIC4_IRQn,             4,    DEV_DW_IIC_TAG},
	{DW_PWR_IIC_BASE,          DW_PWR_IIC_IRQn,          5,    DEV_DW_IIC_TAG},
	{DW_SPI0_BASE,             DW_SPI0_IRQn,             0,    DEV_DW_SPI_TAG},
	{DW_SPI1_BASE,             DW_SPI1_IRQn,             1,    DEV_DW_SPI_TAG},
	{DW_SPI2_BASE,             DW_SPI2_IRQn,             2,    DEV_DW_SPI_TAG},
	{DW_SPI3_BASE,             DW_SPI3_IRQn,             3,    DEV_DW_SPI_TAG},
    {CVI_PWM0_BASE,            0,                        0,    DEV_CVI_PWM_TAG},
    {CVI_PWM1_BASE,            0,                        1,    DEV_CVI_PWM_TAG},
    {CVI_PWM2_BASE,            0,                        2,    DEV_CVI_PWM_TAG},
    {CVI_PWM3_BASE,            0,                        3,    DEV_CVI_PWM_TAG},
    {DW_MAC_BASE,              DW_MAC_IRQn,              0,    DEV_DW_MAC_TAG},
    {ADC_BASE,                 ADC_IRQn,                 0,    DEV_CVI_ADC_TAG},
    {RTC_ADC_BASE,             0,                        1,    DEV_CVI_ADC_TAG},
    {DW_SDIO0_BASE,            SD0_INTR,                 0,    DEV_DW_SDMMC_TAG,},
    {DW_SDIO1_BASE,            SD1_INTR,                 1,    DEV_DW_SDMMC_TAG,},
    {CVI_WDT0_BASE,             0,                        0,    DEV_DW_WDT_TAG},
    {CVI_WDT1_BASE,             WDT1_INTR,                 1,    DEV_DW_WDT_TAG},
    {CVI_WDT2_BASE,             0,                 2,    DEV_DW_WDT_TAG},
    {CVI_RTC_WDT_BASE,          0,                 3,    DEV_DW_WDT_TAG},
    {0, 0, 0, 0}
};

const uint8_t g_dma_chnum[] = {16};

/* DMA handshake number */
/* The member of uart_tx_hs_num is the handshake number for ETB */
const uint16_t uart_tx_hs_num[] = {19};
const uint16_t uart_rx_hs_num[] = {18};
const uint16_t iic_tx_hs_num[]  = {21, 23};
const uint16_t iic_rx_hs_num[]  = {20, 22};
const uint16_t i2s_tx_hs_num[]  = {9, 11, 13, 36, 37, 38, 39};
const uint16_t i2s_rx_hs_num[]  = {8, 10, 12, 14, 15, 16, 17};
const uint16_t spdif_tx_hs_num[]  = {25, 27};
const uint16_t spdif_rx_hs_num[]  = {24, 26};
const uint16_t tdm_rx_hs_num[]  = {28, 29, 30, 31, 32, 33, 34, 35};
const uint16_t vad_rx_hs_num[]  = {0, 1, 2, 3, 4, 5, 6, 7};

const csi_dma_ch_desc_t uart0_dma_ch_list[] = {
    {0, 0}, {0, 1}, {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},
    {0, 8}, {0, 9}, {0, 10},  {0, 11},  {0, 12},  {0, 13},  {0, 14},  {0, 15},
    {0xff, 0xff}
};

const csi_dma_ch_desc_t iic0_dma_ch_list[] = {
    {0, 0}, {0, 1}, {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},
    {0, 8}, {0, 9}, {0, 10},  {0, 11},  {0, 12},  {0, 13},  {0, 14},  {0, 15},
    {0xff, 0xff}
};

const csi_dma_ch_desc_t iic1_dma_ch_list[] = {
    {0, 0}, {0, 1}, {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},
    {0, 8}, {0, 9}, {0, 10},  {0, 11},  {0, 12},  {0, 13},  {0, 14},  {0, 15},
    {0xff, 0xff}
};

const csi_dma_ch_desc_t i2s0_dma_ch_list[] = {
    {0, 0}, {0, 1}, {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},
    {0, 8}, {0, 9}, {0, 10},  {0, 11},  {0, 12},  {0, 13},  {0, 14},  {0, 15},
    {0xff, 0xff}
};

const csi_dma_ch_desc_t i2s1_dma_ch_list[] = {
    {0, 0}, {0, 1}, {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},
    {0, 8}, {0, 9}, {0, 10},  {0, 11},  {0, 12},  {0, 13},  {0, 14},  {0, 15},
    {0xff, 0xff}
};

const csi_dma_ch_desc_t i2s2_dma_ch_list[] = {
    {0, 0}, {0, 1}, {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},
    {0, 8}, {0, 9}, {0, 10},  {0, 11},  {0, 12},  {0, 13},  {0, 14},  {0, 15},
    {0xff, 0xff}
};

const csi_dma_ch_desc_t i2s3_dma_ch_list[] = {
    {0, 0}, {0, 1}, {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},
    {0, 8}, {0, 9}, {0, 10},  {0, 11},  {0, 12},  {0, 13},  {0, 14},  {0, 15},
    {0xff, 0xff}
};

const csi_dma_ch_desc_t spdif0_dma_ch_list[] = {
    {0, 0}, {0, 1}, {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},
    {0, 8}, {0, 9}, {0, 10},  {0, 11},  {0, 12},  {0, 13},  {0, 14},  {0, 15},
    {0xff, 0xff}
};

const csi_dma_ch_desc_t spdif1_dma_ch_list[] = {
    {0, 0}, {0, 1}, {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},
    {0, 8}, {0, 9}, {0, 10},  {0, 11},  {0, 12},  {0, 13},  {0, 14},  {0, 15},
    {0xff, 0xff}
};

const csi_dma_ch_desc_t tdm_dma_ch_list[] = {
    {0, 0}, {0, 1}, {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},
    {0, 8}, {0, 9}, {0, 10},  {0, 11},  {0, 12},  {0, 13},  {0, 14},  {0, 15},
    {0xff, 0xff}
};

const csi_dma_ch_desc_t vad_dma_ch_list[] = {
    {0, 0}, {0, 1}, {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},
    {0, 8}, {0, 9}, {0, 10},  {0, 11},  {0, 12},  {0, 13},  {0, 14},  {0, 15},
    {0xff, 0xff}
};

const csi_dma_ch_desc_t spi3_dma_ch_list[] = {
    {0, 0}, {0, 1}, {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},
    {0, 8}, {0, 9}, {0, 10},  {0, 11},  {0, 12},  {0, 13},  {0, 14},  {0, 15},
    {0xff, 0xff}
};

const csi_dma_ch_desc_t spi0_dma_ch_list[] = {
    {0, 0}, {0, 1}, {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},
    {0, 8}, {0, 9}, {0, 10},  {0, 11},  {0, 12},  {0, 13},  {0, 14},  {0, 15},
    {0xff, 0xff}
};

const csi_dma_ch_spt_list_t dma_spt_list[] = {
    {DEV_DW_UART_TAG,   0, uart0_dma_ch_list},
    {DEV_DW_IIC_TAG,    0, iic0_dma_ch_list},
    {DEV_DW_IIC_TAG,    1, iic1_dma_ch_list},
    {DEV_WJ_I2S_TAG,    0, i2s0_dma_ch_list},
    {DEV_WJ_I2S_TAG,    1, i2s1_dma_ch_list},
    {DEV_WJ_I2S_TAG,    2, i2s2_dma_ch_list},
    {DEV_WJ_I2S_TAG,    3, i2s3_dma_ch_list},
    {DEV_WJ_SPDIF_TAG,  0, spdif0_dma_ch_list},
    {DEV_WJ_SPDIF_TAG,  1, spdif1_dma_ch_list},
    {DEV_WJ_TDM_TAG,    0, tdm_dma_ch_list},
    {DEV_WJ_VAD_TAG,    0, vad_dma_ch_list},
    {DEV_DW_SPI_TAG,    2, spi0_dma_ch_list}, /* spi2 */
    {0xFFFFU,         0xFFU,         NULL},
};



const cvi_pinmap_t uart_pinmap[] = {
    // {PA4,             0,     PIN_UART_RX,    PA4_UART_RX  },
    // {PA5,             0,     PIN_UART_TX,    PA5_UART_TX  },
    // {PA6,             0,     PIN_UART_RX,    PA6_UART_RX  },
    // {PA7,             0,     PIN_UART_TX,    PA7_UART_TX  },
    // {PA12,            0,     PIN_UART_RX,    PA12_UART_TX },
    // {PA13,            0,     PIN_UART_TX,    PA13_UART_RX },
    // {PA23,            0,     PIN_UART_RX,    PA23_UART_RX },
    // {PA24,            0,     PIN_UART_TX,    PA24_UART_TX },
    {0xFFFFFFFFU, 0xFFU, 0xFFU,      0xFFFFFFFFU  },
};

const cvi_pinmap_t iic_pinmap[] = {
    {0xFFFFFFFFU, 0xFFU,       0xFFU,   0xFFFFFFFFU },
};

const cvi_pinmap_t i2s_pinmap[] = {
    {0xFFFFFFFFU, 0xFFU, 0xFFU, 0xFFFFFFFFU  },
};

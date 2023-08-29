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
    {DW_SDIO2_BASE,            SD2_INTR,                 2,    DEV_DW_SDMMC_TAG,},
    {CVI_WDT0_BASE,             0,                        0,    DEV_DW_WDT_TAG},
    {CVI_WDT1_BASE,             WDT1_INTR,                 1,    DEV_DW_WDT_TAG},
    {CVI_WDT2_BASE,             0,                 2,    DEV_DW_WDT_TAG},
    {CVI_RTC_WDT_BASE,          0,                 3,    DEV_DW_WDT_TAG},
    {CVI_TEMPSEN_BASE,          TEMPSEN_INTR,             0,    DEV_DW_TEMPSEN_TAG},
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

const csi_dma_ch_desc_t spi2_dma_ch_list[] = {
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
    {DEV_DW_SPI_TAG,    2, spi2_dma_ch_list}, /* spi2 */
    {0xFFFFU,         0xFFU,         NULL},
};

const csi_pinmap_t gpio_pinmap[] = {
    // {PB0,    0,  0,           PIN_FUNC_GPIO },
    // {PB1,    0,  1,           PIN_FUNC_GPIO },
    // {PB2,    0,  2,           PIN_FUNC_GPIO },
    // {PB3,    0,  3,           PIN_FUNC_GPIO },
    // {PB4,    0,  4,           PIN_FUNC_GPIO },
    // {PB5,    0,  5,           PIN_FUNC_GPIO },
    // {PB6,    0,  6,           PIN_FUNC_GPIO },
    // {PB7,    0,  7,           PIN_FUNC_GPIO },
    // {PB8 ,   0,  8,           PIN_FUNC_GPIO },
    // {PB9 ,   0,  9,           PIN_FUNC_GPIO },
    // {PB10,   0,  10,          PIN_FUNC_GPIO },
    // {PB11,   0,  11,          PIN_FUNC_GPIO },
    // {PB12,   0,  12,          PIN_FUNC_GPIO },
    // {PC0,    0,  0,           PIN_FUNC_GPIO },
    // {PC1,    0,  1,           PIN_FUNC_GPIO },
    // {PC2,    0,  2,           PIN_FUNC_GPIO },
    // {PC3,    0,  3,           PIN_FUNC_GPIO },
    // {PC4,    0,  4,           PIN_FUNC_GPIO },
    // {PC5,    0,  5,           PIN_FUNC_GPIO },
    // {PC6,    0,  6,           PIN_FUNC_GPIO },
    // {PC7,    0,  7,           PIN_FUNC_GPIO },
    // {PD0,    0,  0,           PIN_FUNC_GPIO },
    // {PD1,    0,  1,           PIN_FUNC_GPIO },
    // {PD2,    0,  2,           PIN_FUNC_GPIO },
    // {PD3,    0,  3,           PIN_FUNC_GPIO },
    // {PD4,    0,  4,           PIN_FUNC_GPIO },
    // {PD5,    0,  5,           PIN_FUNC_GPIO },
    // {PD6,    0,  6,           PIN_FUNC_GPIO },
    // {PD7,    0,  7,           PIN_FUNC_GPIO },
    // {PD8,    0,  8,           PIN_FUNC_GPIO },
    // {PD9,    0,  9,           PIN_FUNC_GPIO },
    // {PD10,   0,  10,          PIN_FUNC_GPIO },
    // {PD11,   0,  11,          PIN_FUNC_GPIO },
    // {PD12,   0,  12,          PIN_FUNC_GPIO },
    // {PD13,   0,  13,          PIN_FUNC_GPIO },
    // {PD14,   0,  14,          PIN_FUNC_GPIO },
    // {PD15,   0,  15,          PIN_FUNC_GPIO },
    // {PD16,   0,  16,          PIN_FUNC_GPIO },
    // {PD17,   0,  17,          PIN_FUNC_GPIO },
    // {PD18,   0,  18,          PIN_FUNC_GPIO },
    // {PD19,   0,  19,          PIN_FUNC_GPIO },
    // {PD20,   0,  20,          PIN_FUNC_GPIO },
    // {PD21,   0,  21,          PIN_FUNC_GPIO },
    // {PD22,   0,  22,          PIN_FUNC_GPIO },
    // {PE0 ,   0,  0 ,          PIN_FUNC_GPIO },
    // {PE1 ,   0,  1 ,          PIN_FUNC_GPIO },
    // {PE2 ,   0,  2 ,          PIN_FUNC_GPIO },
    // {PE3 ,   0,  3 ,          PIN_FUNC_GPIO },
    // {PE4 ,   0,  4 ,          PIN_FUNC_GPIO },
    // {PE5 ,   0,  5 ,          PIN_FUNC_GPIO },
    // {PE6 ,   0,  6 ,          PIN_FUNC_GPIO },
    // {PE7 ,   0,  7 ,          PIN_FUNC_GPIO },
    // {PE8 ,   0,  8 ,          PIN_FUNC_GPIO },
    // {PE9 ,   0,  9 ,          PIN_FUNC_GPIO },
    // {PE10,   0,  10,          PIN_FUNC_GPIO },
    // {PE11,   0,  11,          PIN_FUNC_GPIO },
    // {PE12,   0,  12,          PIN_FUNC_GPIO },
    // {PE13,   0,  13,          PIN_FUNC_GPIO },
    // {PE14,   0,  14,          PIN_FUNC_GPIO },
    // {PE15,   0,  15,          PIN_FUNC_GPIO },
    // {PE16,   0,  16,          PIN_FUNC_GPIO },
    // {PE17,   0,  17,          PIN_FUNC_GPIO },
    // {PF0 ,   0,  0 ,          PIN_FUNC_GPIO },
    // {PF1 ,   0,  1 ,          PIN_FUNC_GPIO },
    // {PF2 ,   0,  2 ,          PIN_FUNC_GPIO },
    // {PF3 ,   0,  3 ,          PIN_FUNC_GPIO },
    // {PF4 ,   0,  4 ,          PIN_FUNC_GPIO },
    // {PF5 ,   0,  5 ,          PIN_FUNC_GPIO },
    // {PF6 ,   0,  6 ,          PIN_FUNC_GPIO },
    // {PG0 ,   0,  0 ,          PIN_FUNC_GPIO },
    // {PG1 ,   0,  1 ,          PIN_FUNC_GPIO },
    // {PG2 ,   0,  2 ,          PIN_FUNC_GPIO },
    // {PG3 ,   0,  3 ,          PIN_FUNC_GPIO },
    // {PG4 ,   0,  4 ,          PIN_FUNC_GPIO },
    // {PG5 ,   0,  5 ,          PIN_FUNC_GPIO },
    // {PG6 ,   0,  6 ,          PIN_FUNC_GPIO },
    // {PG7 ,   0,  7 ,          PIN_FUNC_GPIO },
    // {PG8 ,   0,  8 ,          PIN_FUNC_GPIO },
    // {PG9 ,   0,  9 ,          PIN_FUNC_GPIO },
    // {PG10,   0,  10,          PIN_FUNC_GPIO },
    // {PG11,   0,  11,          PIN_FUNC_GPIO },
    // {PG12,   0,  12,          PIN_FUNC_GPIO },
    // {PG13,   0,  13,          PIN_FUNC_GPIO },
    // {PG14,   0,  14,          PIN_FUNC_GPIO },
    // {PG15,   0,  15,          PIN_FUNC_GPIO },
    // {PG16,   0,  16,          PIN_FUNC_GPIO },
    // {PG17,   0,  17,          PIN_FUNC_GPIO },
    // {PG18,   0,  18,          PIN_FUNC_GPIO },
    {0xFFFFFFFFU,   0xFFU, 0xFFU,       0xFFFFFFFFU   },
};

const csi_pinmap_t uart_pinmap[] = {
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

const csi_pinmap_t iic_pinmap[] = {
    {0xFFFFFFFFU, 0xFFU,       0xFFU,   0xFFFFFFFFU },
};

const csi_pinmap_t i2s_pinmap[] = {
    {0xFFFFFFFFU, 0xFFU, 0xFFU, 0xFFFFFFFFU  },
};

const csi_clkmap_t clk_map[] = {
    {TIM0_CLK,   DEV_DW_TIMER_TAG,      0},
    {TIM1_CLK,   DEV_DW_TIMER_TAG,      1},
    {RTC0_CLK,   DEV_WJ_RTC_TAG,        0},
    {WDT_CLK,    DEV_DW_WDT_TAG,        0},
    {SPI0_CLK,   DEV_DW_SPI_TAG,        0},
    {UART0_CLK,  DEV_DW_UART_TAG,       0},
    {IIC0_CLK,   DEV_DW_IIC_TAG,        0},
    {PWM_CLK,    DEV_WJ_PWM_TAG,        0},
    {QSPI0_CLK,  DEV_CD_QSPI_TAG,       0},
    {PWMR_CLK,   DEV_BLANK_TAG,         0},
    {EFUSE_CLK,  DEV_WJ_EFUSE_TAG,      0},
    {I2S0_CLK,   DEV_WJ_I2S_TAG,        0},
    {I2S1_CLK,   DEV_WJ_I2S_TAG,        1},
    {GPIO0_CLK,  DEV_DW_GPIO_TAG,       0},
    {TIM2_CLK,   DEV_DW_TIMER_TAG,      2},
    {TIM3_CLK,   DEV_DW_TIMER_TAG,      3},
    {SPI1_CLK,   DEV_DW_SPI_TAG,        1},
    {UART1_CLK,  DEV_DW_UART_TAG,       1},
    {I2S567_CLK, DEV_WJ_I2S_TAG,        5},
    {I2S567_CLK, DEV_WJ_I2S_TAG,        6},
    {I2S567_CLK, DEV_WJ_I2S_TAG,        7},
    {ADC_CLK,    DEV_WJ_ADC_TAG,        0},
    {ETB_CLK,    DEV_WJ_ETB_TAG,        0},
    {I2S2_CLK,   DEV_WJ_I2S_TAG,        2},
    {I2S3_CLK,   DEV_WJ_I2S_TAG,        3},
    {IOC_CLK,    DEV_BLANK_TAG,         0},
    {CODEC_CLK,  DEV_RCHBAND_CODEC_TAG, 0},
    {0xFFFFFFFFU, 0xFFFFU,          0xFFU}
};

const csi_pinmap_t adc_pinmap[] = {
    {0xFFFFFFFFU, 0xFFU,       0xFFU,   0xFFFFFFFFU },
};

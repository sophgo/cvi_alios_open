/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     soc.h
 * @brief    CSI Core Peripheral Access Layer Header File for
 *           CSKYSOC Device Series
 * @version  V1.0
 * @date     7. April 2020
 ******************************************************************************/

#ifndef _SOC_H_
#define _SOC_H_

#include <stdint.h>
#include <csi_core.h>
#include <sys_clk.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef EHS_VALUE
#define EHS_VALUE               20000000U
#endif

#ifndef ELS_VALUE
#define ELS_VALUE               32768U
#endif

#ifndef IHS_VALUE
#define IHS_VALUE               24000000U
#endif

#ifndef ILS_VALUE
#define ILS_VALUE               32768U
#endif

typedef enum {
    Supervisor_Software_IRQn        =  1U,
    Machine_Software_IRQn           =  3U,
    Supervisor_Timer_IRQn           =  5U,
    CORET_IRQn                      =  7U,
    Supervisor_External_IRQn        =  9U,
    Machine_External_IRQn           =  11U,

    DW_GPIO0_IRQn                   =  60U,
    DW_GPIO1_IRQn                   =  61U,
    DW_GPIO2_IRQn                   =  62U,
    DW_GPIO3_IRQn                   =  63U,
    DW_RTC_GPIO_IRQn                =  70U,
    DW_UART0_IRQn                   =  44U,
    DW_UART1_IRQn                   =  45U,
    DW_UART2_IRQn                   =  46U,
    DW_UART3_IRQn                   =  47U,
    DW_UART4_IRQn                   =  48U,
    DW_IIC0_IRQn                    =  49U,
    DW_IIC1_IRQn                    =  50U,
    DW_IIC2_IRQn                    =  51U,
    DW_IIC3_IRQn                    =  52U,
    DW_IIC4_IRQn                    =  53U,
	DW_IIC5_IRQn                    =  59U,
    DW_SPI0_IRQn                    =  54U,
    DW_SPI1_IRQn                    =  55U,
    DW_SPI2_IRQn                    =  56U,
    DW_SPI3_IRQn                    =  57U,
	DW_MAC_IRQn                   	=  31U,
    ADC_IRQn                        =  100U,
    SD0_INTR                        =  36U,
    SD1_INTR                        =  38U,
    SD2_INTR                        =  34U,
    WDT1_INTR                       =  58U,
	RTC_WDT_INTR		    =  74U,
    TEMPSEN_INTR                    =  16U,
} irqn_type_t;

typedef enum {
    SD0_CLK,
    SD0_CMD,
    SD0_D0,
    SD0_D1,
    SD0_D2,
    SD0_D3,
    SD0_CD,
    SD0_PWR_EN,
    SPK_EN,
    UART0_TX,
    UART0_RX,
    SPINOR_HOLD_X,
    SPINOR_SCK,
    SPINOR_MOSI,
    SPINOR_WP_X,
    SPINOR_MISO,
    SPINOR_CS_X,
    JTAG_CPU_TMS,
    JTAG_CPU_TCK,
    IIC0_SCL,
    IIC0_SDA,
    AUX0,
    GPIO_ZQ,
    PWR_VBAT_DET,
    PWR_RSTN,
    PWR_SEQ1,
    PWR_SEQ2,
    PWR_WAKEUP0,
    PWR_BUTTON1,
    XTAL_XIN,
    PWR_GPIO0,
    PWR_GPIO1,
    PWR_GPIO2,
	SD1_GPIO1,
	SD1_GPIO0,
    SD1_D3,
    SD1_D2,
    SD1_D1,
    SD1_D0,
    SD1_CMD,
    SD1_CLK,
    PWM0_BUCK,
    ADC1,
    USB_VBUS_DET,
    PAD_ETH_TXP,
    PAD_ETH_TXM,
    PAD_ETH_RXP,
    PAD_ETH_RXM,
    GPIO_RTX,
    PAD_MIPIRX4N,
    PAD_MIPIRX4P,
    PAD_MIPIRX3N,
    PAD_MIPIRX3P,
    PAD_MIPIRX2N,
    PAD_MIPIRX2P,
    PAD_MIPIRX1N,
    PAD_MIPIRX1P,
    PAD_MIPIRX0N,
    PAD_MIPIRX0P,
    PAD_MIPI_TXM2,
    PAD_MIPI_TXP2,
    PAD_MIPI_TXM1,
    PAD_MIPI_TXP1,
    PAD_MIPI_TXM0,
    PAD_MIPI_TXP0,
    PAD_AUD_AINL_MIC,
    PAD_AUD_AOUTR,
    MUX_SPI1_MISO,
    MUX_SPI1_MOSI,
    MUX_SPI1_CS,
    MUX_SPI1_SCK,
    PIN_NUM,
} pin_name_t;


typedef enum {
    SD0_CLK__SDIO0_CLK = 0U,
    SD0_CLK__IIC1_SDA = 1U,
    SD0_CLK__SPI0_SCK = 2U,
    SD0_CLK__XGPIOA_7 = 3U,
    SD0_CLK__PWM_15 = 5U,
    SD0_CLK__EPHY_LNK_LED = 6U,
    SD0_CLK__DBG_0 = 7U,
    SD0_CMD__SDIO0_CMD = 0U,
    SD0_CMD__IIC1_SCL = 1U,
    SD0_CMD__SPI0_SDO = 2U,
    SD0_CMD__XGPIOA_8 = 3U,
    SD0_CMD__PWM_14 = 5U,
    SD0_CMD__EPHY_SPD_LED = 6U,
    SD0_CMD__DBG_1 = 7U,
    SD0_D0__SDIO0_D_0 = 0U,
    SD0_D0__CAM_MCLK1 = 1U,
    SD0_D0__SPI0_SDI = 2U,
    SD0_D0__XGPIOA_9 = 3U,
    SD0_D0__UART3_TX = 4U,
    SD0_D0__PWM_13 = 5U,
    SD0_D0__WG0_D0 = 6U,
    SD0_D0__DBG_2 = 7U,
    SD0_D1__SDIO0_D_1 = 0U,
    SD0_D1__IIC1_SDA = 1U,
    SD0_D1__AUX0 = 2U,
    SD0_D1__XGPIOA_10 = 3U,
    SD0_D1__UART1_TX = 4U,
    SD0_D1__PWM_12 = 5U,
    SD0_D1__WG0_D1 = 6U,
    SD0_D1__DBG_3 = 7U,
    SD0_D2__SDIO0_D_2 = 0U,
    SD0_D2__IIC1_SCL = 1U,
    SD0_D2__AUX1 = 2U,
    SD0_D2__XGPIOA_11 = 3U,
    SD0_D2__UART1_RX = 4U,
    SD0_D2__PWM_11 = 5U,
    SD0_D2__WG1_D0 = 6U,
    SD0_D2__DBG_4 = 7U,
    SD0_D3__SDIO0_D_3 = 0U,
    SD0_D3__CAM_MCLK0 = 1U,
    SD0_D3__SPI0_CS_X = 2U,
    SD0_D3__XGPIOA_12 = 3U,
    SD0_D3__UART3_RX = 4U,
    SD0_D3__PWM_10 = 5U,
    SD0_D3__WG1_D1 = 6U,
    SD0_D3__DBG_5 = 7U,
    SD0_CD__SDIO0_CD = 0U,
    SD0_CD__XGPIOA_13 = 3U,
    SD0_PWR_EN__SDIO0_PWR_EN = 0U,
    SD0_PWR_EN__XGPIOA_14 = 3U,
    SPK_EN__XGPIOA_15 = 3U,
    UART0_TX__UART0_TX = 0U,
    UART0_TX__CAM_MCLK1 = 1U,
    UART0_TX__PWM_4 = 2U,
    UART0_TX__XGPIOA_16 = 3U,
    UART0_TX__UART1_TX = 4U,
    UART0_TX__AUX1 = 5U,
    UART0_TX__JTAG_TMS = 6U,
    UART0_TX__DBG_6 = 7U,
    UART0_RX__UART0_RX = 0U,
    UART0_RX__CAM_MCLK0 = 1U,
    UART0_RX__PWM_5 = 2U,
    UART0_RX__XGPIOA_17 = 3U,
    UART0_RX__UART1_RX = 4U,
    UART0_RX__AUX0 = 5U,
    UART0_RX__JTAG_TCK = 6U,
    UART0_RX__DBG_7 = 7U,
    SPINOR_HOLD_X__SPINOR_HOLD_X = 1U,
    SPINOR_HOLD_X__SPINAND_HOLD = 2U,
    SPINOR_HOLD_X__XGPIOA_26 = 3U,
    SPINOR_SCK__SPINOR_SCK = 1U,
    SPINOR_SCK__SPINAND_CLK = 2U,
    SPINOR_SCK__XGPIOA_22 = 3U,
    SPINOR_MOSI__SPINOR_MOSI = 1U,
    SPINOR_MOSI__SPINAND_MOSI = 2U,
    SPINOR_MOSI__XGPIOA_25 = 3U,
    SPINOR_WP_X__SPINOR_WP_X = 1U,
    SPINOR_WP_X__SPINAND_WP = 2U,
    SPINOR_WP_X__XGPIOA_27 = 3U,
    SPINOR_MISO__SPINOR_MISO = 1U,
    SPINOR_MISO__SPINAND_MISO = 2U,
    SPINOR_MISO__XGPIOA_23 = 3U,
    SPINOR_CS_X__SPINOR_CS_X = 1U,
    SPINOR_CS_X__SPINAND_CS = 2U,
    SPINOR_CS_X__XGPIOA_24 = 3U,
    JTAG_CPU_TMS__JTAG_TMS = 0U,
    JTAG_CPU_TMS__CAM_MCLK0 = 1U,
    JTAG_CPU_TMS__PWM_7 = 2U,
    JTAG_CPU_TMS__XGPIOA_19 = 3U,
    JTAG_CPU_TMS__UART1_RTS = 4U,
    JTAG_CPU_TMS__AUX0 = 5U,
    JTAG_CPU_TMS__UART1_TX = 6U,
    JTAG_CPU_TCK__JTAG_TCK = 0U,
    JTAG_CPU_TCK__CAM_MCLK1 = 1U,
    JTAG_CPU_TCK__PWM_6 = 2U,
    JTAG_CPU_TCK__XGPIOA_18 = 3U,
    JTAG_CPU_TCK__UART1_CTS = 4U,
    JTAG_CPU_TCK__AUX1 = 5U,
    JTAG_CPU_TCK__UART1_RX = 6U,
    IIC0_SCL__JTAG_TDI = 0U,
    IIC0_SCL__UART1_TX = 1U,
    IIC0_SCL__UART2_TX = 2U,
    IIC0_SCL__XGPIOA_28 = 3U,
    IIC0_SCL__IIC0_SCL = 4U,
    IIC0_SCL__WG0_D0 = 5U,
    IIC0_SCL__DBG_10 = 7U,
    IIC0_SDA__JTAG_TDO = 0U,
    IIC0_SDA__UART1_RX = 1U,
    IIC0_SDA__UART2_RX = 2U,
    IIC0_SDA__XGPIOA_29 = 3U,
    IIC0_SDA__IIC0_SDA = 4U,
    IIC0_SDA__WG0_D1 = 5U,
    IIC0_SDA__WG1_D0 = 6U,
    IIC0_SDA__DBG_11 = 7U,
    AUX0__AUX0 = 0U,
    AUX0__XGPIOA_30 = 3U,
    AUX0__IIS1_MCLK = 4U,
    AUX0__WG1_D1 = 6U,
    AUX0__DBG_12 = 7U,
    GPIO_ZQ__PWR_GPIO_24 = 3U,
    GPIO_ZQ__PWM_2 = 4U,
    PWR_VBAT_DET__PWR_VBAT_DET = 0U,
    PWR_RSTN__PWR_RSTN = 0U,
    PWR_SEQ1__PWR_SEQ1 = 0U,
    PWR_SEQ1__PWR_GPIO_3 = 3U,
    PWR_SEQ2__PWR_SEQ2 = 0U,
    PWR_SEQ2__PWR_GPIO_4 = 3U,
    PWR_WAKEUP0__PWR_WAKEUP0 = 0U,
    PWR_WAKEUP0__PWR_IR0 = 1U,
    PWR_WAKEUP0__PWR_UART0_TX = 2U,
    PWR_WAKEUP0__PWR_GPIO_6 = 3U,
    PWR_WAKEUP0__UART1_TX = 4U,
    PWR_WAKEUP0__IIC4_SCL = 5U,
    PWR_WAKEUP0__EPHY_LNK_LED = 6U,
    PWR_WAKEUP0__WG2_D0 = 7U,
    PWR_BUTTON1__PWR_BUTTON1 = 0U,
    PWR_BUTTON1__PWR_GPIO_8 = 3U,
    PWR_BUTTON1__UART1_RX = 4U,
    PWR_BUTTON1__IIC4_SDA = 5U,
    PWR_BUTTON1__EPHY_SPD_LED = 6U,
    PWR_BUTTON1__WG2_D1 = 7U,
    XTAL_XIN__PWR_XTAL_CLKIN = 0U,
    PWR_GPIO0__PWR_GPIO_0 = 0U,
    PWR_GPIO0__UART2_TX = 1U,
    PWR_GPIO0__PWR_UART0_RX = 2U,
    PWR_GPIO0__PWM_8 = 4U,
    PWR_GPIO1__PWR_GPIO_1 = 0U,
    PWR_GPIO1__UART2_RX = 1U,
    PWR_GPIO1__EPHY_LNK_LED = 3U,
    PWR_GPIO1__PWM_9 = 4U,
    PWR_GPIO1__PWR_IIC_SCL = 5U,
    PWR_GPIO1__IIC2_SCL = 6U,
    PWR_GPIO1__IIC0_SDA = 7U,
    PWR_GPIO2__PWR_GPIO_2 = 0U,
    PWR_GPIO2__PWR_SECTICK = 2U,
    PWR_GPIO2__EPHY_SPD_LED = 3U,
    PWR_GPIO2__PWM_10 = 4U,
    PWR_GPIO2__PWR_IIC_SDA = 5U,
    PWR_GPIO2__IIC2_SDA = 6U,
    PWR_GPIO2__IIC0_SCL = 7U,
    SD1_GPIO1__UART4_TX = 1U,
    SD1_GPIO1__PWR_GPIO_26 = 3U,
    SD1_GPIO1__PWM_10 = 7U,
    SD1_GPIO0__UART4_RX = 1U,
    SD1_GPIO0__PWR_GPIO_25 = 3U,
    SD1_GPIO0__PWM_11 = 7U,
    SD1_D3__PWR_SD1_D3 = 0U,
    SD1_D3__SPI2_CS_X = 1U,
    SD1_D3__IIC1_SCL = 2U,
    SD1_D3__PWR_GPIO_18 = 3U,
    SD1_D3__CAM_MCLK0 = 4U,
    SD1_D3__UART3_CTS = 5U,
    SD1_D3__PWR_SPINOR1_CS_X = 6U,
    SD1_D3__PWM_4 = 7U,
    SD1_D2__PWR_SD1_D2 = 0U,
    SD1_D2__IIC1_SCL = 1U,
    SD1_D2__UART2_TX = 2U,
    SD1_D2__PWR_GPIO_19 = 3U,
    SD1_D2__CAM_MCLK0 = 4U,
    SD1_D2__UART3_TX = 5U,
    SD1_D2__PWR_SPINOR1_HOLD_X = 6U,
    SD1_D2__PWM_5 = 7U,
    SD1_D1__PWR_SD1_D1 = 0U,
    SD1_D1__IIC1_SDA = 1U,
    SD1_D1__UART2_RX = 2U,
    SD1_D1__PWR_GPIO_20 = 3U,
    SD1_D1__CAM_MCLK1 = 4U,
    SD1_D1__UART3_RX = 5U,
    SD1_D1__PWR_SPINOR1_WP_X = 6U,
    SD1_D1__PWM_6 = 7U,
    SD1_D0__PWR_SD1_D0 = 0U,
    SD1_D0__SPI2_SDI = 1U,
    SD1_D0__IIC1_SDA = 2U,
    SD1_D0__PWR_GPIO_21 = 3U,
    SD1_D0__CAM_MCLK1 = 4U,
    SD1_D0__UART3_RTS = 5U,
    SD1_D0__PWR_SPINOR1_MISO = 6U,
    SD1_D0__PWM_7 = 7U,
    SD1_CMD__PWR_SD1_CMD = 0U,
    SD1_CMD__SPI2_SDO = 1U,
    SD1_CMD__IIC3_SCL = 2U,
    SD1_CMD__PWR_GPIO_22 = 3U,
    SD1_CMD__CAM_VS0 = 4U,
    SD1_CMD__EPHY_LNK_LED = 5U,
    SD1_CMD__PWR_SPINOR1_MOSI = 6U,
    SD1_CMD__PWM_8 = 7U,
    SD1_CLK__PWR_SD1_CLK = 0U,
    SD1_CLK__SPI2_SCK = 1U,
    SD1_CLK__IIC3_SDA = 2U,
    SD1_CLK__PWR_GPIO_23 = 3U,
    SD1_CLK__CAM_HS0 = 4U,
    SD1_CLK__EPHY_SPD_LED = 5U,
    SD1_CLK__PWR_SPINOR1_SCK = 6U,
    SD1_CLK__PWM_9 = 7U,
    PWM0_BUCK__PWM_0 = 0U,
    PWM0_BUCK__XGPIOB_0 = 3U,
    ADC1__XGPIOB_3 = 3U,
    ADC1__KEY_COL2 = 4U,
    ADC1__PWM_3 = 6U,
    USB_VBUS_DET__USB_VBUS_DET = 0U,
    USB_VBUS_DET__XGPIOB_6 = 3U,
    USB_VBUS_DET__CAM_MCLK0 = 4U,
    USB_VBUS_DET__CAM_MCLK1 = 5U,
    USB_VBUS_DET__PWM_4 = 6U,
    MUX_SPI1_MISO__UART3_RTS = 1U,
    MUX_SPI1_MISO__IIC1_SDA = 2U,
    MUX_SPI1_MISO__XGPIOB_8 = 3U,
    MUX_SPI1_MISO__PWM_9 = 4U,
    MUX_SPI1_MISO__KEY_COL1 = 5U,
    MUX_SPI1_MISO__SPI1_SDI = 6U,
    MUX_SPI1_MISO__DBG_14 = 7U,
    MUX_SPI1_MOSI__UART3_RX = 1U,
    MUX_SPI1_MOSI__IIC1_SCL = 2U,
    MUX_SPI1_MOSI__XGPIOB_7 = 3U,
    MUX_SPI1_MOSI__PWM_8 = 4U,
    MUX_SPI1_MOSI__KEY_COL0 = 5U,
    MUX_SPI1_MOSI__SPI1_SDO = 6U,
    MUX_SPI1_MOSI__DBG_13 = 7U,
    MUX_SPI1_CS__UART3_CTS = 1U,
    MUX_SPI1_CS__CAM_MCLK0 = 2U,
    MUX_SPI1_CS__XGPIOB_10 = 3U,
    MUX_SPI1_CS__PWM_11 = 4U,
    MUX_SPI1_CS__KEY_ROW3 = 5U,
    MUX_SPI1_CS__SPI1_CS_X = 6U,
    MUX_SPI1_CS__DBG_16 = 7U,
    MUX_SPI1_SCK__UART3_TX = 1U,
    MUX_SPI1_SCK__CAM_MCLK1 = 2U,
    MUX_SPI1_SCK__XGPIOB_9 = 3U,
    MUX_SPI1_SCK__PWM_10 = 4U,
    MUX_SPI1_SCK__KEY_ROW2 = 5U,
    MUX_SPI1_SCK__SPI1_SCK = 6U,
    MUX_SPI1_SCK__DBG_15 = 7U,
    PAD_ETH_TXP__UART3_RX = 1U,
    PAD_ETH_TXP__IIC1_SCL = 2U,
    PAD_ETH_TXP__XGPIOB_25 = 3U,
    PAD_ETH_TXP__PWM_13 = 4U,
    PAD_ETH_TXP__CAM_MCLK0 = 5U,
    PAD_ETH_TXP__SPI1_SDO = 6U,
    PAD_ETH_TXP__IIS2_LRCK = 7U,
    PAD_ETH_TXM__UART3_RTS = 1U,
    PAD_ETH_TXM__IIC1_SDA = 2U,
    PAD_ETH_TXM__XGPIOB_24 = 3U,
    PAD_ETH_TXM__PWM_12 = 4U,
    PAD_ETH_TXM__CAM_MCLK1 = 5U,
    PAD_ETH_TXM__SPI1_SDI = 6U,
    PAD_ETH_TXM__IIS2_BCLK = 7U,
    PAD_ETH_RXP__UART3_TX = 1U,
    PAD_ETH_RXP__CAM_MCLK1 = 2U,
    PAD_ETH_RXP__XGPIOB_27 = 3U,
    PAD_ETH_RXP__PWM_15 = 4U,
    PAD_ETH_RXP__CAM_HS0 = 5U,
    PAD_ETH_RXP__SPI1_SCK = 6U,
    PAD_ETH_RXP__IIS2_DO = 7U,
    PAD_ETH_RXM__UART3_CTS = 1U,
    PAD_ETH_RXM__CAM_MCLK0 = 2U,
    PAD_ETH_RXM__XGPIOB_26 = 3U,
    PAD_ETH_RXM__PWM_14 = 4U,
    PAD_ETH_RXM__CAM_VS0 = 5U,
    PAD_ETH_RXM__SPI1_CS_X = 6U,
    PAD_ETH_RXM__IIS2_DI = 7U,
    GPIO_RTX__VI0_D_15 = 1U,
    GPIO_RTX__XGPIOB_23 = 3U,
    GPIO_RTX__PWM_1 = 4U,
    GPIO_RTX__CAM_MCLK0 = 5U,
    GPIO_RTX__IIS2_MCLK = 7U,
    PAD_MIPIRX4N__VI0_CLK = 1U,
    PAD_MIPIRX4N__IIC0_SCL = 2U,
    PAD_MIPIRX4N__XGPIOC_2 = 3U,
    PAD_MIPIRX4N__IIC1_SDA = 4U,
    PAD_MIPIRX4N__CAM_MCLK0 = 5U,
    PAD_MIPIRX4N__KEY_ROW0 = 6U,
    PAD_MIPIRX4N__MUX_SPI1_SCK = 7U,
    PAD_MIPIRX4P__VI0_D_0 = 1U,
    PAD_MIPIRX4P__IIC0_SDA = 2U,
    PAD_MIPIRX4P__XGPIOC_3 = 3U,
    PAD_MIPIRX4P__IIC1_SCL = 4U,
    PAD_MIPIRX4P__CAM_MCLK1 = 5U,
    PAD_MIPIRX4P__KEY_ROW1 = 6U,
    PAD_MIPIRX4P__MUX_SPI1_CS = 7U,
    PAD_MIPIRX3N__VI0_D_1 = 1U,
    PAD_MIPIRX3N__XGPIOC_4 = 3U,
    PAD_MIPIRX3N__CAM_MCLK0 = 4U,
    PAD_MIPIRX3N__MUX_SPI1_MISO = 7U,
    PAD_MIPIRX3P__VI0_D_2 = 1U,
    PAD_MIPIRX3P__XGPIOC_5 = 3U,
    PAD_MIPIRX3P__MUX_SPI1_MOSI = 7U,
    PAD_MIPIRX2N__VI0_D_3 = 1U,
    PAD_MIPIRX2N__XGPIOC_6 = 3U,
    PAD_MIPIRX2N__IIC4_SCL = 5U,
    PAD_MIPIRX2N__DBG_6 = 7U,
    PAD_MIPIRX2P__VI0_D_4 = 1U,
    PAD_MIPIRX2P__XGPIOC_7 = 3U,
    PAD_MIPIRX2P__IIC4_SDA = 5U,
    PAD_MIPIRX2P__DBG_7 = 7U,
    PAD_MIPIRX1N__VI0_D_5 = 1U,
    PAD_MIPIRX1N__XGPIOC_8 = 3U,
    PAD_MIPIRX1N__KEY_ROW3 = 6U,
    PAD_MIPIRX1N__DBG_8 = 7U,
    PAD_MIPIRX1P__VI0_D_6 = 1U,
    PAD_MIPIRX1P__XGPIOC_9 = 3U,
    PAD_MIPIRX1P__IIC1_SDA = 4U,
    PAD_MIPIRX1P__KEY_ROW2 = 6U,
    PAD_MIPIRX1P__DBG_9 = 7U,
    PAD_MIPIRX0N__VI0_D_7 = 1U,
    PAD_MIPIRX0N__XGPIOC_10 = 3U,
    PAD_MIPIRX0N__IIC1_SCL = 4U,
    PAD_MIPIRX0N__CAM_MCLK1 = 5U,
    PAD_MIPIRX0N__DBG_10 = 7U,
    PAD_MIPIRX0P__VI0_D_8 = 1U,
    PAD_MIPIRX0P__XGPIOC_11 = 3U,
    PAD_MIPIRX0P__CAM_MCLK0 = 4U,
    PAD_MIPIRX0P__DBG_11 = 7U,
    PAD_MIPI_TXM2__VI0_D_13 = 1U,
    PAD_MIPI_TXM2__IIC0_SDA = 2U,
    PAD_MIPI_TXM2__XGPIOC_16 = 3U,
    PAD_MIPI_TXM2__IIC1_SDA = 4U,
    PAD_MIPI_TXM2__PWM_8 = 5U,
    PAD_MIPI_TXM2__SPI0_SCK = 6U,
    PAD_MIPI_TXP2__VI0_D_14 = 1U,
    PAD_MIPI_TXP2__IIC0_SCL = 2U,
    PAD_MIPI_TXP2__XGPIOC_17 = 3U,
    PAD_MIPI_TXP2__IIC1_SCL = 4U,
    PAD_MIPI_TXP2__PWM_9 = 5U,
    PAD_MIPI_TXP2__SPI0_CS_X = 6U,
    PAD_MIPI_TXP2__IIS1_MCLK = 7U,
    PAD_MIPI_TXM1__SPI3_SDO = 0U,
    PAD_MIPI_TXM1__VI0_D_11 = 1U,
    PAD_MIPI_TXM1__IIS1_LRCK = 2U,
    PAD_MIPI_TXM1__XGPIOC_14 = 3U,
    PAD_MIPI_TXM1__IIC2_SDA = 4U,
    PAD_MIPI_TXM1__PWM_10 = 5U,
    PAD_MIPI_TXM1__SPI0_SDO = 6U,
    PAD_MIPI_TXM1__DBG_14 = 7U,
    PAD_MIPI_TXP1__SPI3_SDI = 0U,
    PAD_MIPI_TXP1__VI0_D_12 = 1U,
    PAD_MIPI_TXP1__IIS1_DO = 2U,
    PAD_MIPI_TXP1__XGPIOC_15 = 3U,
    PAD_MIPI_TXP1__IIC2_SCL = 4U,
    PAD_MIPI_TXP1__PWM_11 = 5U,
    PAD_MIPI_TXP1__SPI0_SDI = 6U,
    PAD_MIPI_TXP1__DBG_15 = 7U,
    PAD_MIPI_TXM0__SPI3_SCK = 0U,
    PAD_MIPI_TXM0__VI0_D_9 = 1U,
    PAD_MIPI_TXM0__IIS1_DI = 2U,
    PAD_MIPI_TXM0__XGPIOC_12 = 3U,
    PAD_MIPI_TXM0__CAM_MCLK1 = 4U,
    PAD_MIPI_TXM0__PWM_14 = 5U,
    PAD_MIPI_TXM0__CAM_VS0 = 6U,
    PAD_MIPI_TXM0__DBG_12 = 7U,
    PAD_MIPI_TXP0__SPI3_CS_X = 0U,
    PAD_MIPI_TXP0__VI0_D_10 = 1U,
    PAD_MIPI_TXP0__IIS1_BCLK = 2U,
    PAD_MIPI_TXP0__XGPIOC_13 = 3U,
    PAD_MIPI_TXP0__CAM_MCLK0 = 4U,
    PAD_MIPI_TXP0__PWM_15 = 5U,
    PAD_MIPI_TXP0__CAM_HS0 = 6U,
    PAD_MIPI_TXP0__DBG_13 = 7U,
    PAD_AUD_AINL_MIC__XGPIOC_23 = 3U,
    PAD_AUD_AINL_MIC__IIS1_BCLK = 4U,
    PAD_AUD_AINL_MIC__IIS2_BCLK = 5U,
    PAD_AUD_AOUTR__XGPIOC_24 = 3U,
    PAD_AUD_AOUTR__IIS1_DI = 4U,
    PAD_AUD_AOUTR__IIS2_DO = 5U,
    PAD_AUD_AOUTR__IIS1_DO = 6U,
    MUX_UART0_IP_SEL__SELECT_UART_IP_0 = 0,
	MUX_UART0_IP_SEL__SELECT_UART_IP_1 = 1,
	MUX_UART0_IP_SEL__SELECT_UART_IP_2 = 2,
	MUX_UART0_IP_SEL__SELECT_UART_IP_3 = 3,
	MUX_UART0_IP_SEL__SELECT_UART_IP_4 = 4,
    MUX_UART1_IP_SEL__SELECT_UART_IP_0 = 0,
	MUX_UART1_IP_SEL__SELECT_UART_IP_1 = 1,
	MUX_UART1_IP_SEL__SELECT_UART_IP_2 = 2,
	MUX_UART1_IP_SEL__SELECT_UART_IP_3 = 3,
	MUX_UART1_IP_SEL__SELECT_UART_IP_4 = 4,
    MUX_UART2_IP_SEL__SELECT_UART_IP_0 = 0,
	MUX_UART2_IP_SEL__SELECT_UART_IP_1 = 1,
	MUX_UART2_IP_SEL__SELECT_UART_IP_2 = 2,
	MUX_UART2_IP_SEL__SELECT_UART_IP_3 = 3,
	MUX_UART2_IP_SEL__SELECT_UART_IP_4 = 4,
    MUX_UART3_IP_SEL__SELECT_UART_IP_0 = 0,
	MUX_UART3_IP_SEL__SELECT_UART_IP_1 = 1,
	MUX_UART3_IP_SEL__SELECT_UART_IP_2 = 2,
	MUX_UART3_IP_SEL__SELECT_UART_IP_3 = 3,
	MUX_UART3_IP_SEL__SELECT_UART_IP_4 = 4,
    MUX_UART4_IP_SEL__SELECT_UART_IP_0 = 0,
	MUX_UART4_IP_SEL__SELECT_UART_IP_1 = 1,
	MUX_UART4_IP_SEL__SELECT_UART_IP_2 = 2,
	MUX_UART4_IP_SEL__SELECT_UART_IP_3 = 3,
	MUX_UART4_IP_SEL__SELECT_UART_IP_4 = 4,

    PIN_FUNC_GPIO                    = 3U,
    PIN_FUNC_INVALID                 = 0xff,
} pin_func_t;

typedef struct {
    pin_name_t  pin_name;
    uint8_t     idx;        ///< ctrl idx.    e.g: ADC0 channel 1, idx = 0, channel = 1
    uint8_t     channel;    ///< channel idx. e.g: same as the previous line
    pin_func_t  pin_func;
} cvi_pinmap_t;

typedef struct {
    pin_name_t pin_name;
    uint16_t cfg_reg_offset;
    uint16_t mux_reg_offset;
} pin_reg_offset_t;


typedef enum {
    CVI_OK          =  0,
    CVI_ERROR       = -1,
    CVI_BUSY        = -2,
    CVI_TIMEOUT     = -3,
    CVI_UNSUPPORTED = -4
} cvi_error_t;

typedef struct {
    unsigned long reg_base;
    uint8_t       irq_num;
    uint8_t       idx;
    void          (*irq_handler)(uint32_t irqn,void *arg);
    void           *arg;
} cvi_dev_t;

#define GET_DEV_REG_BASE(handle)     (handle->dev.reg_base)
#define GET_DEV_IDX(handle)      (handle->dev.idx)
#define GET_DEV_IRQ_NUM(handle)      (handle->dev.irq_num)
#define GET_DEV_IRQ_HANDLER(handle)  (handle->dev.irq_handler)


#define CONFIG_GPIO_NUM             2
#define CONFIG_IRQ_NUM              1023U
#define CONFIG_REAL_IRQ_CNT         105

#define PIN_MUX_BASE            0x03001000UL
#define PIN_CFG_BASE            0x03001000UL
#define PWR_PIN_CFG_BASE        0x05027000UL

#define DW_GPIO0_BASE               0x03020000UL
#define DW_GPIO1_BASE               0x03021000UL
#define DW_GPIO2_BASE               0x03022000UL
#define DW_GPIO3_BASE               0x03023000UL
#define DW_RTC_GPIO_BASE            0x05021000UL

#define CVI_WDT0_BASE                0x03010000
#define CVI_WDT1_BASE                0x03011000
#define CVI_WDT2_BASE                0x03012000
#define CVI_RTC_WDT_BASE             0x0502D000
#define CVI_WDT_SIZE                0x1000

#define CVI_TEMPSEN_BASE			0x030E0000
#define CVI_TEMPSEN_SIZE			0x10000

#define DW_UART0_BASE               0x04140000UL
#define DW_UART0_SIZE               0x1000U

#define DW_UART1_BASE               0x04150000UL
#define DW_UART1_SIZE               0x1000U

#define DW_UART2_BASE               0x04160000UL
#define DW_UART2_SIZE               0x1000U

#define DW_UART3_BASE               0x04170000UL
#define DW_UART3_SIZE               0x1000U

#define DW_UART4_BASE               0x041C0000UL
#define DW_UART4_SIZE               0x1000U

#define DW_IIC0_BASE               0x04000000UL
#define DW_IIC0_SIZE               0x1000U

#define DW_IIC1_BASE               0x04010000UL
#define DW_IIC1_SIZE               0x1000U

#define DW_IIC2_BASE               0x04020000UL
#define DW_IIC2_SIZE               0x1000U

#define DW_IIC3_BASE               0x04030000UL
#define DW_IIC3_SIZE               0x1000U

#define DW_IIC4_BASE               0x04040000UL
#define DW_IIC4_SIZE               0x1000U

#define DW_IIC5_BASE               0x0502B000UL
#define DW_IIC5_SIZE               0x1000U

#define DW_SPI_REG_SIZE             (0x10000UL)
#define DW_SPI0_BASE                (0x04180000UL)
#define DW_SPI1_BASE                (DW_SPI0_BASE + 1 * DW_SPI_REG_SIZE)
#define DW_SPI2_BASE                (DW_SPI0_BASE + 2 * DW_SPI_REG_SIZE)
#define DW_SPI3_BASE                (DW_SPI0_BASE + 3 * DW_SPI_REG_SIZE)

#define DW_MAC_BASE                0x04070000UL
#define DW_MAC_SIZE                0x10000U

#define ADC_BASE                    0x030F0000
#define RTC_ADC_BASE                0x0502C000
#define ADC_SIZE                    0x1000

#define CVI_PWM0_BASE               0x03060000
#define CVI_PWM1_BASE               0x03061000
#define CVI_PWM2_BASE               0x03062000
#define CVI_PWM3_BASE               0x03063000

#define PLIC_BASE           (0x70000000UL)
#define CORET_BASE          (PLIC_BASE + 0x4000000UL)               /*!< CORET Base Address */
#define PLIC                ((PLIC_Type *)PLIC_BASE)

#define DW_SDIO0_BASE               0x04320000
#define DW_SDIO1_BASE               0x04310000
#define DW_SDIO2_BASE               0x04300000

/* ================================================================================ */
/* ================                  otp declaration               ================ */
/* ================================================================================ */
#define CONFIG_OTP_BASE_ADDR 0U
#define CONFIG_OTP_BANK_SIZE 256U    // Bytes

extern const pin_reg_offset_t cvi_pin_reg_offset[];

#ifdef __cplusplus
}
#endif

#endif  /* _SOC_H_ */

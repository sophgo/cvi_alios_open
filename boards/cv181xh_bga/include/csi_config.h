#ifndef __CSI_CONFIG_H__
#define __CSI_CONFIG_H__

/*
 * defination for the UART can be used by test AT command Console
 * define uart pins and it pin functions accordingly
 * undefine TEST_UART_PINMUX_INIT if this UART not support pinmux
 */

#define TEST_UART_PINMUX_INIT

#ifdef TEST_UART_PINMUX_INIT
#define CONSOLE_TXD             PIN0
#define CONSOLE_RXD             PIN1
#define CONSOLE_TXD_FUNC        PIN0_UART0_TX
#define CONSOLE_RXD_FUNC        PIN1_UART0_RX
#endif

/*
 *  CSI Interface Configuration File
 *  Undefine optional unimplemented CSI functions
 *  for the optional functions that being implemented,
 *  chnage the corrspoding line to define
*/

/* CSI GPIO and optional implemented functions */
#undef CSI_GPIO_DEBONCE

/* CSI GPIO_PIN optional implemented functions */
#undef CSI_GPIO_PIN_DEBONCE

/* CSI PIN_MUX optional implemented functions */
#undef CSI_PIN_SPEED
#undef CSI_PIN_WEAKUP
#undef CSI_PIN_DRIVE
#undef CSI_PIN_GET_DEVIDX
#undef CSI_PIN_GET_CHANNEL
#undef CSI_PIN_GET_PINNAME

/* CSI ADC optional implemented functions */
#undef CSI_ADC_CHANNEL_SAMPLING_TIME
#undef CSI_ADC_GET_STATE
#undef CSI_ADC_LINK_DMA

/* CSI PWM optional implemented functions */
#undef CSI_PWM_CAPTURE
#undef CSI_PWM_CALLBACK

/* CSI TIMER optional implemented functions */
#undef CSI_TIMER_IS_RUNNING

/* CSI RTC optional implemented functions */
#undef CSI_RTC_IS_RUNNING

/* CSI WDT optional implemented functions */
#undef CSI_WDT_STOP
#undef CSI_WDT_GET_REMAINING_TIME
#undef CSI_WDT_IS_RUNNING
#undef CSI_WDT_CALLBACK

/* CSI UART optional implemented functions */
#undef CSI_UART_FLOWCTRL
#undef CSI_UART_LINK_DMA
#undef CSI_UART_GET_STATE

/* CSI SPI optional implemented functions */
#undef CSI_SPI_SELECT_SLAVE
#undef CSI_SPI_LINK_DMA
#undef CSI_SPI_GET_STATE

/* CSI SPIFLASH optional implemented functions */
#define CSI_SPIFLASH_SPI
#undef CSI_SPIFLASH_QSPI
#undef CSI_SPIFLASH_LOCK
#undef CSI_SPIFLASH_READ_REG

/* CSI CODEC optional implemented functions */
#undef CSI_CODEC_FLOWCTRL
#undef CSI_CODEC_LINK_DMA
#undef CSI_CODEC_GET_STATE

/* CSI AES optional implemented functions */
#undef CSI_AES_CFB
#undef CSI_AES_OFB
#undef CSI_AES_CTR

/* CSI SHA optional implemented functions */
#undef CSI_SHA_ASYNC
#undef CSI_SHA_GET_STATE

#endif /* __CSI_CONFIG_H__ */
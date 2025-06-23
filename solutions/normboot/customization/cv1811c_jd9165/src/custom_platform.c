#include "platform.h"
#include <drv/pin.h>
#include <pinctrl-mars.h>
#include "cvi_type.h"
#include <aos/kernel.h>

#define GPIO_PIN_MASK(_gpio_num) (1 << _gpio_num)

void _GPIOSetValue(u8 gpio_grp, u8 gpio_num, u8 level)
{
	csi_error_t ret;
	csi_gpio_t gpio = {0};

	ret = csi_gpio_init(&gpio, gpio_grp);
	if(ret != CSI_OK) {
		printf("csi_gpio_init failed\r\n");
		return;
	}
	// gpio write
	ret = csi_gpio_dir(&gpio , GPIO_PIN_MASK(gpio_num), GPIO_DIRECTION_OUTPUT);

	if(ret != CSI_OK) {
		printf("csi_gpio_dir failed\r\n");
		return;
	}
	csi_gpio_write(&gpio , GPIO_PIN_MASK(gpio_num), level);
	//printf("test pin end and success.\r\n");
}
void PLATFORM_PanelOn(int value)
{
	u8 gpio_rst_grp = 1;
	u8 gpio_rst_num = 3;

	u8 gpio_slp_grp = 1;
	u8 gpio_slp_num = 0;

	u8 gpio_pwr_grp = 1;
	u8 gpio_pwr_num = 23;

	_GPIOSetValue(gpio_rst_grp, gpio_rst_num, 0);
	aos_msleep(1);
	_GPIOSetValue(gpio_rst_grp, gpio_rst_num, 1);

	if(value)
	_GPIOSetValue(gpio_slp_grp, gpio_slp_num, 1);
	else
	_GPIOSetValue(gpio_slp_grp, gpio_slp_num, 0);

	if(value)
	_GPIOSetValue(gpio_pwr_grp, gpio_pwr_num, 1);
	else
	_GPIOSetValue(gpio_pwr_grp, gpio_pwr_num, 0);
}
void PLATFORM_SpkMute(int value)
{
	u8 gpio_spken_r_grp = 4;
	u8 gpio_spken_r_num = 2;
#if defined (CONFIG_CHIP_cv1811h) || defined (CONFIG_CHIP_cv1812h) || (CONFIG_CHIP_cv1811ha) || (CONFIG_CHIP_cv1812ha) || (CONFIG_CHIP_cv1813h)
	u8 gpio_spken_l_grp;
	u8 gpio_spken_l_num;
#endif

#if defined (CONFIG_CHIP_cv1811c) || defined (CONFIG_CHIP_cv1801c) || defined (CONFIG_CHIP_cv1810c) || defined (CONFIG_CHIP_cv1812cp)
	gpio_spken_r_grp = 0;
	gpio_spken_r_num = 15;
#endif
#if defined (CONFIG_CHIP_cv1801b) || defined (CONFIG_CHIP_cv180zb)
	gpio_spken_r_grp = 0;
	gpio_spken_r_num = 15;
#endif
#if defined (CONFIG_CHIP_cv1811h) || defined (CONFIG_CHIP_cv1812h) || (CONFIG_CHIP_cv1811ha) || (CONFIG_CHIP_cv1812ha) || (CONFIG_CHIP_cv1813h)
	gpio_spken_r_grp = 0;
	gpio_spken_r_num = 30;
	gpio_spken_l_grp = 0;
	gpio_spken_l_num = 15;
#endif
//0静音 ，1非静音
    if(value){
        _GPIOSetValue(gpio_spken_r_grp, gpio_spken_r_num, 1);
#if defined (CONFIG_CHIP_cv1811h) || defined (CONFIG_CHIP_cv1812h) || (CONFIG_CHIP_cv1811ha) || (CONFIG_CHIP_cv1812ha) || (CONFIG_CHIP_cv1813h)
        _GPIOSetValue(gpio_spken_l_grp, gpio_spken_l_num, 1);
#endif
    }else{
        _GPIOSetValue(gpio_spken_r_grp, gpio_spken_r_grp, 0);
#if defined (CONFIG_CHIP_cv1811h) || defined (CONFIG_CHIP_cv1812h) || (CONFIG_CHIP_cv1811ha) || (CONFIG_CHIP_cv1812ha) || (CONFIG_CHIP_cv1813h)
        _GPIOSetValue(gpio_spken_l_grp, gpio_spken_l_num, 0);
#endif
    }
}
static void _AudioPinmux(void)
{
#if defined (CONFIG_CHIP_cv1811c) || defined (CONFIG_CHIP_cv1801c) || defined (CONFIG_CHIP_cv1810c) || defined (CONFIG_CHIP_cv1812cp)
	PINMUX_CONFIG(SPK_EN, XGPIOA_15);
#endif
#if defined (CONFIG_CHIP_cv1811h) || defined (CONFIG_CHIP_cv1812h) || (CONFIG_CHIP_cv1811ha) || (CONFIG_CHIP_cv1812ha) || (CONFIG_CHIP_cv1813h)
    PINMUX_CONFIG(SPK_EN, XGPIOA_15);
    PINMUX_CONFIG(AUX0, XGPIOA_30);
#endif
#if defined (CONFIG_CHIP_cv1801b) || defined (CONFIG_CHIP_cv180zb)
    PINMUX_CONFIG(SPK_EN, XGPIOA_15);
#endif
    PLATFORM_SpkMute(1);
}
static void _UartPinmux()
{
	return;

	// uart1 pinmux
	PINMUX_CONFIG(IIC0_SCL, UART1_TX);
	PINMUX_CONFIG(IIC0_SDA, UART1_RX);

	// PINMUX_CONFIG(UART0_TX, UART0_TX);
	// PINMUX_CONFIG(UART0_RX, UART0_RX);
}

static void _SensorPinmux()
{
	return;
	//Sensor Pinmux
#if defined (CONFIG_CHIP_cv1811c) || defined (CONFIG_CHIP_cv1801c) || defined (CONFIG_CHIP_cv1812cp)
	PINMUX_CONFIG(PAD_MIPI_TXP1, IIC2_SCL);
	PINMUX_CONFIG(PAD_MIPI_TXM1, IIC2_SDA);
	PINMUX_CONFIG(PAD_MIPI_TXM0, CAM_MCLK1);
#endif
#if defined (CONFIG_CHIP_cv1811h) || defined (CONFIG_CHIP_cv1812h) || defined (CONFIG_CHIP_cv1811ha)|| defined (CONFIG_CHIP_cv1812ha) || defined (CONFIG_CHIP_cv1813h)
	// PINMUX_CONFIG(IIC2_SCL, IIC2_SCL);
	// PINMUX_CONFIG(IIC2_SDA, IIC2_SDA);

	// PINMUX_CONFIG(IIC3_SCL, IIC3_SCL);
	// PINMUX_CONFIG(IIC3_SDA, IIC3_SDA);
	PINMUX_CONFIG(CAM_MCLK0, CAM_MCLK0);

	// PINMUX_CONFIG(IIC2_SCL, IIC2_SCL);
	// PINMUX_CONFIG(IIC2_SDA, IIC2_SDA);
	PINMUX_CONFIG(CAM_RST0, XGPIOA_2);
	PINMUX_CONFIG(IIC3_SCL, IIC3_SCL);
	PINMUX_CONFIG(IIC3_SDA, IIC3_SDA);
#endif
#if defined (CONFIG_CHIP_cv1801b) || defined (CONFIG_CHIP_cv180zb)
	PINMUX_CONFIG(PAD_MIPIRX0N, IIC1_SCL);
	PINMUX_CONFIG(PAD_MIPIRX1P, IIC1_SDA);

#if 0 //evb mipi switch
	PINMUX_CONFIG(PAD_MIPIRX1N, XGPIOC_8);
#endif

	PINMUX_CONFIG(PAD_MIPIRX0P, CAM_MCLK0);
#endif
}

static void _MipiRxPinmux(void)
{
	return;

//mipi rx pinmux
#if 0 //need porting for cv180x
    PINMUX_CONFIG(PAD_MIPIRX4P, XGPIOC_3);
    PINMUX_CONFIG(PAD_MIPIRX4N, XGPIOC_2);
#endif
}

static void _MipiTxPinmux(void)
{
//mipi tx pinmux
#if CONFIG_PANEL_ILI9488
	PINMUX_CONFIG(PAD_MIPI_TXM1, XGPIOC_14);
	PINMUX_CONFIG(PAD_MIPI_TXP1, XGPIOC_15);
	PINMUX_CONFIG(PAD_MIPI_TXM2, XGPIOC_16);
	PINMUX_CONFIG(PAD_MIPI_TXP2, XGPIOC_17);
	PINMUX_CONFIG(IIC0_SCL, XGPIOA_28);
#elif (CONFIG_PANEL_HX8394 || CONFIG_PANEL_JD9165)
#if CONFIG_BOARD_CV181XC
	PINMUX_CONFIG(PAD_MIPI_TXM0, XGPIOC_12);
	PINMUX_CONFIG(PAD_MIPI_TXP0, XGPIOC_13);
	PINMUX_CONFIG(PAD_MIPI_TXM1, XGPIOC_14);
	PINMUX_CONFIG(PAD_MIPI_TXP1, XGPIOC_15);
	PINMUX_CONFIG(PAD_MIPI_TXM2, XGPIOC_16);
	PINMUX_CONFIG(PAD_MIPI_TXP2, XGPIOC_17);
	PINMUX_CONFIG(ADC1, XGPIOB_3);//RST
	PINMUX_CONFIG(PWM0_BUCK, XGPIOB_0);//STABDBY
#elif (defined(__CV181X__))
	PINMUX_CONFIG(PAD_MIPI_TXM0, XGPIOC_12);
	PINMUX_CONFIG(PAD_MIPI_TXP0, XGPIOC_13);
	PINMUX_CONFIG(PAD_MIPI_TXM1, XGPIOC_14);
	PINMUX_CONFIG(PAD_MIPI_TXP1, XGPIOC_15);
	PINMUX_CONFIG(PAD_MIPI_TXM2, XGPIOC_16);
	PINMUX_CONFIG(PAD_MIPI_TXP2, XGPIOC_17);
	PINMUX_CONFIG(PAD_MIPI_TXM3, XGPIOC_20);
	PINMUX_CONFIG(PAD_MIPI_TXP3, XGPIOC_21);
	PINMUX_CONFIG(PAD_MIPI_TXM4, XGPIOC_18);
	PINMUX_CONFIG(PAD_MIPI_TXP4, XGPIOC_19);
#endif
#endif
	PLATFORM_PanelOn(1);
}

#if (CONFIG_APP_DEBUG_JTAG == 1)
void JTAG_PinmuxIn()
{
    PINMUX_CONFIG(IIC0_SDA, CV_SDA0__CR_4WTDO);
    PINMUX_CONFIG(IIC0_SCL, CV_SCL0__CR_4WTDI);
}
#endif

void PLATFORM_IoInit(void)
{
//pinmux 切换接口
    _UartPinmux();//void
    _MipiRxPinmux();//void
    _MipiTxPinmux();
    _SensorPinmux();//void
    _AudioPinmux();
	#if (CONFIG_APP_DEBUG_JTAG == 1)
	JTAG_PinmuxIn();
	#endif

#if 0 //evb mipi switch
	PINMUX_CONFIG(SD1_CMD, IIC3_SCL);
	PINMUX_CONFIG(SD1_CLK, IIC3_SDA);
	PINMUX_CONFIG(ADC1, PWM_3);
#endif
}

void PLATFORM_PowerOff(void)
{
//下电休眠前调用接口
}

int PLATFORM_PanelInit(void)
{
    return CVI_SUCCESS;
}

void PLATFORM_PanelBacklightCtl(int level)
{

}

int PLATFORM_IrCutCtl(int duty)
{
    return 0;
}

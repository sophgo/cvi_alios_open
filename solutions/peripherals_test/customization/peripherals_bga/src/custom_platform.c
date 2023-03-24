#include "platform.h"
#include <drv/pin.h>
#include <pinctrl-mars.h>
#include "cvi_type.h"

static void _SensorPinmux()
{
    //Sensor Pinmux
	PINMUX_CONFIG(IIC3_SCL, IIC3_SCL);
	PINMUX_CONFIG(IIC3_SDA, IIC3_SDA);
	PINMUX_CONFIG(CAM_MCLK0, CAM_MCLK0);
	PINMUX_CONFIG(CAM_RST0,	XGPIOA_2);
}

static void _MipiRxPinmux(void)
{
//mipi rx pinmux
    PINMUX_CONFIG(PAD_MIPIRX4P, XGPIOC_3);
    PINMUX_CONFIG(PAD_MIPIRX4N, XGPIOC_2);
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
#elif (CONFIG_PANEL_HX8394)
	PINMUX_CONFIG(PAD_MIPI_TXM0, XGPIOC_12);
	PINMUX_CONFIG(PAD_MIPI_TXP0, XGPIOC_13);
	PINMUX_CONFIG(PAD_MIPI_TXM1, XGPIOC_14);
	PINMUX_CONFIG(PAD_MIPI_TXP1, XGPIOC_15);
	PINMUX_CONFIG(PAD_MIPI_TXM2, XGPIOC_16);
	PINMUX_CONFIG(PAD_MIPI_TXP2, XGPIOC_17);
	PINMUX_CONFIG(JTAG_CPU_TCK, XGPIOA_18);
	PINMUX_CONFIG(JTAG_CPU_TMS, XGPIOA_19);
	PINMUX_CONFIG(SPK_EN, XGPIOA_15);
#endif
}

#if (CONFIG_SUPPORT_USB_HC || CONFIG_SUPPORT_USB_DC)

#define GPIO_PIN_MASK(_gpio_num) (1 << _gpio_num)

static void _GPIOSetValue(u8 gpio_grp, u8 gpio_num, u8 level)
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
	csi_gpio_uninit(&gpio);
}

static void _UsbPinmux(void)
{
	// SOC_PORT_SEL
	PINMUX_CONFIG(ADC1, XGPIOB_3);
	PINMUX_CONFIG(ADC2, XGPIOB_2);
}

static void _UsbIoInit(void)
{
#if CONFIG_SUPPORT_USB_HC
	_GPIOSetValue(1, 3, 0);
	_GPIOSetValue(1, 2, 0);
#elif CONFIG_SUPPORT_USB_DC
	_GPIOSetValue(1, 3, 1);
	_GPIOSetValue(1, 2, 1);
#endif
}

#endif

void PLATFORM_IoInit(void)
{
//pinmux 切换接口
#if (CONFIG_SUPPORT_USB_HC || CONFIG_SUPPORT_USB_DC)
	_UsbPinmux();
	_UsbIoInit();
#endif
    _MipiRxPinmux();
    _MipiTxPinmux();
    _SensorPinmux();
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

void PLATFORM_SpkMute(int value)
{
//0静音 ，1非静音
}

int PLATFORM_IrCutCtl(int duty)
{
    return 0;
}

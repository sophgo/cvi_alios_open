#include "platform.h"
#include <drv/pin.h>
#include <pinctrl-mars.h>
#include "cvi_type.h"
#define GPIO_SPKEN_GRP 0
#define GPIO_SPKEN_NUM 15
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

static void _TouchPanlePinmux()
{
	PINMUX_CONFIG(IIC3_SCL, XGPIOA_5);
	PINMUX_CONFIG(IIC3_SDA, XGPIOA_6);
	PINMUX_CONFIG(VIVO_D1, IIC3_SDA);
	PINMUX_CONFIG(VIVO_D0, IIC3_SCL);

	PINMUX_CONFIG(JTAG_CPU_TMS, XGPIOA_19);
	PINMUX_CONFIG(JTAG_CPU_TCK, XGPIOA_18);
}

static void _SensorPinmux()
{
	//Sensor IIC
	PINMUX_CONFIG(IIC2_SDA, IIC2_SDA);
	PINMUX_CONFIG(IIC2_SCL, IIC2_SCL);

	//Sensor PWDN PIN
	PINMUX_CONFIG(CAM_PD0, XGPIOA_1);
	PINMUX_CONFIG(CAM_PD1, XGPIOA_4);

	mmio_write_32(0x03020004, mmio_read_32(0x03020004) | 0x2);
	mmio_write_32(0x03020000, mmio_read_32(0x03020000) | 0x2);

	mmio_write_32(0x03020004, mmio_read_32(0x03020004) | 0x10);
	mmio_write_32(0x03020000, mmio_read_32(0x03020000) | 0x10);
	//Sensor Reset PIN
	PINMUX_CONFIG(CAM_RST0, XGPIOA_2);
	PINMUX_CONFIG(IIC3_SDA, XGPIOA_6);
	//Sensor Clock PIN
	PINMUX_CONFIG(CAM_MCLK0, CAM_MCLK0);
	PINMUX_CONFIG(CAM_MCLK1, CAM_MCLK1);
}

static void _MipiRxPinmux(void)
{
//mipi rx pinmux
    // PINMUX_CONFIG(PAD_MIPIRX4P, XGPIOC_3);
    // PINMUX_CONFIG(PAD_MIPIRX4N, XGPIOC_2);
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
#elif (CONFIG_PANEL_ST7701S)
	PINMUX_CONFIG(PAD_MIPI_TXM0, XGPIOC_12);
 	PINMUX_CONFIG(PAD_MIPI_TXP0, XGPIOC_13);
 	PINMUX_CONFIG(PAD_MIPI_TXM1, XGPIOC_14);
 	PINMUX_CONFIG(PAD_MIPI_TXP1, XGPIOC_15);
 	PINMUX_CONFIG(PAD_MIPI_TXM2, XGPIOC_16);
 	PINMUX_CONFIG(PAD_MIPI_TXP2, XGPIOC_17);

	PINMUX_CONFIG(PWR_GPIO2, PWR_GPIO_2);//LED_RST
	PINMUX_CONFIG(PWR_GPIO0, PWR_GPIO_0);//LED_PWM
#endif
}

static void _AudI2S1Pinmux(void)
{
    PINMUX_CONFIG(UART2_TX, IIS2_BCLK);
	PINMUX_CONFIG(UART2_RX, IIS2_DI);

	// PINMUX_CONFIG(UART2_TX, PWR_GPIO_14);
	// PINMUX_CONFIG(UART2_RX, PWR_GPIO_16);
    //speaker mute
    PINMUX_CONFIG(SPK_EN, XGPIOA_15);
    PINMUX_CONFIG(AUX0, XGPIOA_30);
}

void PLATFORM_IoInit(void)
{
//pinmux 切换接口
    _MipiRxPinmux();
    _MipiTxPinmux();
    _SensorPinmux();
	_TouchPanlePinmux();
	_AudI2S1Pinmux();
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
    if(value){
        _GPIOSetValue(GPIO_SPKEN_GRP, GPIO_SPKEN_NUM, 1);
        _GPIOSetValue(GPIO_SPKEN_GRP, 30, 1);
    }else{
        _GPIOSetValue(GPIO_SPKEN_GRP, GPIO_SPKEN_NUM, 0);
        _GPIOSetValue(GPIO_SPKEN_GRP, 30, 0);
    }
}

int PLATFORM_IrCutCtl(int duty)
{
    return 0;
}
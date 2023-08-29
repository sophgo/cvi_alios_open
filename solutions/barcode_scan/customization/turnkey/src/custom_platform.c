#include "platform.h"
#include <drv/pin.h>
#include <pinctrl-mars.h>
#include "cvi_type.h"

static void _SensorPinmux()
{
    //Sensor Pinmux
	PINMUX_CONFIG(PAD_MIPIRX0P, CAM_MCLK0); // MCLK0
	PINMUX_CONFIG(PAD_MIPIRX1P, IIC1_SDA); // I2C 2
	PINMUX_CONFIG(PAD_MIPIRX0N, IIC1_SCL);
 	printf("--- sensor pinmux111 ---\n");
}

static void _MipiRxPinmux(void)
{
//mipi rx pinmux
    PINMUX_CONFIG(PAD_MIPIRX4P, XGPIOC_3);
    PINMUX_CONFIG(PAD_MIPIRX4N, XGPIOC_2);
    PINMUX_CONFIG(PAD_MIPIRX3P, XGPIOC_5);
    PINMUX_CONFIG(PAD_MIPIRX3N, XGPIOC_4);
    PINMUX_CONFIG(PAD_MIPIRX2P, XGPIOC_7);
    PINMUX_CONFIG(PAD_MIPIRX2N, XGPIOC_6);
}

static void _MipiTxPinmux(void)
{
//mipi tx pinmux
}

void PLATFORM_IoInit(void)
{
//pinmux 切换接口
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
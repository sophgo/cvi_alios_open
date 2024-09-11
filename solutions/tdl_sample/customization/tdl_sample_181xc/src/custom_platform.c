#include <drv/pin.h>
#include <pinctrl-mars.h>
#include "cvi_type.h"

static void _SensorPinmux()
{
    PINMUX_CONFIG(PWR_GPIO1, IIC2_SCL);
    PINMUX_CONFIG(PWR_GPIO2, IIC2_SDA);
    PINMUX_CONFIG(SD1_D2, CAM_MCLK0);
    PINMUX_CONFIG(SD1_D3, PWR_GPIO_18);
}

static void _MipiRxPinmux(void)
{
    PINMUX_CONFIG(PAD_MIPIRX4P, XGPIOC_3);
    PINMUX_CONFIG(PAD_MIPIRX4N, XGPIOC_2);
}

static void _MipiTxPinmux(void)
{

}

void PLATFORM_SpkMute(int value)
{

}

int PLATFORM_IrCutCtl(int duty)
{
    return CVI_SUCCESS;
}

void PLATFORM_IoInit(void)
{
    _MipiRxPinmux();
    _MipiTxPinmux();
    _SensorPinmux();
}
#include <drv/pin.h>
#include <pinctrl-mars.h>
#include "cvi_type.h"

static void _SensorPinmux()
{

}

static void _MipiRxPinmux(void)
{

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
#include <drv/pin.h>
#include <pinctrl-mars.h>
#include "cvi_type.h"
#include "platform.h"

#define GPIO_SPKEN_GRP 0
#define GPIO_SPKEN_NUM 15

static void _SensorPinmux()
{
    // Sensor Pinmux
    PINMUX_CONFIG(PAD_MIPIRX0P, CAM_MCLK0);
    PINMUX_CONFIG(PAD_MIPIRX1P, IIC1_SDA);
    PINMUX_CONFIG(PAD_MIPIRX0N, IIC1_SCL);
    PINMUX_CONFIG(PAD_MIPIRX1N, XGPIOC_8);
}

static void _MipiRxPinmux(void)
{
    // mipi rx pinmux
    PINMUX_CONFIG(PAD_MIPIRX4P, XGPIOC_3);
    PINMUX_CONFIG(PAD_MIPIRX4N, XGPIOC_2);
    PINMUX_CONFIG(PAD_MIPIRX3P, XGPIOC_5);
    PINMUX_CONFIG(PAD_MIPIRX3N, XGPIOC_4);
    PINMUX_CONFIG(PAD_MIPIRX2P, XGPIOC_7);
    PINMUX_CONFIG(PAD_MIPIRX2N, XGPIOC_6);
}

static void _MipiTxPinmux(void)
{
    // mipi tx pinmux
}

#if (CONFIG_SUPPORT_USB_HC || CONFIG_SUPPORT_USB_DC)

#define GPIO_PIN_MASK(_gpio_num) (1 << _gpio_num)

static void _GPIOSetValue(u8 gpio_grp, u8 gpio_num, u8 level)
{
    csi_error_t ret;
    csi_gpio_t gpio = {0};

    ret = csi_gpio_init(&gpio, gpio_grp);
    if (ret != CSI_OK) {
        printf("csi_gpio_init failed\r\n");
        return;
    }
    // gpio write
    ret = csi_gpio_dir(&gpio, GPIO_PIN_MASK(gpio_num), GPIO_DIRECTION_OUTPUT);

    if (ret != CSI_OK) {
        printf("csi_gpio_dir failed\r\n");
        return;
    }
    csi_gpio_write(&gpio, GPIO_PIN_MASK(gpio_num), level);
    // printf("test pin end and success.\r\n");
    csi_gpio_uninit(&gpio);
}

static void _UsbPinmux(void)
{
    // SOC_PORT_SEL
    PINMUX_CONFIG(SD1_GPIO0, PWR_GPIO_25);
    PINMUX_CONFIG(SD1_GPIO1, PWR_GPIO_26);
}

static void _UsbIoInit(void)
{
#if CONFIG_SUPPORT_USB_HC
    _GPIOSetValue(4, 25, 1);
    _GPIOSetValue(4, 26, 1);
#elif CONFIG_SUPPORT_USB_DC
    _GPIOSetValue(4, 25, 0);
    _GPIOSetValue(4, 26, 0);
#endif
}

#endif

void PLATFORM_IoInit(void)
{
// pinmux 切换接口
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
    // 下电休眠前调用接口
}

int PLATFORM_PanelInit(void)
{
    return CVI_SUCCESS;
}

void PLATFORM_PanelBacklightCtl(int level) {}

void PLATFORM_SpkMute(int value)
{
    // 0静音 ，1非静音
    printf("set spkMute = %d\r\n", value);
    PINMUX_CONFIG(SPK_EN, XGPIOA_15);
    if (value == 0) {
        _GPIOSetValue(GPIO_SPKEN_GRP, GPIO_SPKEN_NUM, 0);
    } else {
        _GPIOSetValue(GPIO_SPKEN_GRP, GPIO_SPKEN_NUM, 1);
    }
}

int PLATFORM_IrCutCtl(int duty)
{
    return 0;
}
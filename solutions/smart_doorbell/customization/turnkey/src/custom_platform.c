#include <aos/cli.h>
#include <drv/pin.h>
#include <mmio.h>
#include <pinctrl-mars.h>
#include <stdint.h>
#include "cvi_param.h"
#include "cvi_type.h"
#include "gpio_util.h"
#include "platform.h"

#define GPIO_SPKEN_GRP 0
#define GPIO_SPKEN_NUM 15

static void _SensorPinmux()
{
    /* Sensor pinmux */
    PINMUX_CONFIG(PAD_MIPIRX2N, XGPIOC_6);
    PINMUX_CONFIG(PAD_MIPIRX2P, XGPIOC_7);
    PINMUX_CONFIG(PAD_MIPIRX1P, IIC1_SDA);
    PINMUX_CONFIG(PAD_MIPIRX0N, IIC1_SCL);
    PINMUX_CONFIG(PAD_MIPIRX0P, CAM_MCLK0);

    cvi_gpio_set_val(2, 8, 1);  // XSHUTDOWN
    cvi_gpio_set_val(2, 7, 1);  // XSHUTDOWN
}

static void _MipiRxPinmux(void)
{
    /* Mipi rx pinmux */
    PINMUX_CONFIG(PAD_MIPIRX4P, XGPIOC_3);
    PINMUX_CONFIG(PAD_MIPIRX4N, XGPIOC_2);
    PINMUX_CONFIG(PAD_MIPIRX3P, XGPIOC_5);
    PINMUX_CONFIG(PAD_MIPIRX3N, XGPIOC_4);
}

static void _UartPinmux()
{
    PINMUX_CONFIG(IIC0_SDA, UART1_RX);
    PINMUX_CONFIG(IIC0_SCL, UART1_TX);
}

static void _UsbPinmux(void)
{
#if (CONFIG_SUPPORT_USB_HC || CONFIG_SUPPORT_USB_DC)
    /* SOC_PORT_SEL */
    PINMUX_CONFIG(PWR_GPIO0, PWR_GPIO_0);
    PINMUX_CONFIG(PWR_GPIO1, PWR_GPIO_1);
#endif
}

static void _UsbIoInit(void)
{
#if CONFIG_SUPPORT_USB_HC
    cvi_gpio_set_val(4, 0, 0);
    cvi_gpio_set_val(4, 1, 0);
#elif CONFIG_SUPPORT_USB_DC
    cvi_gpio_set_val(4, 0, 1);
    cvi_gpio_set_val(4, 1, 1);
#endif
}

static void _LedPinmux(void)
{
    PINMUX_CONFIG(SD1_GPIO1, PWR_GPIO_26);  // IR LED
    PINMUX_CONFIG(SD1_GPIO0, PWR_GPIO_25);  // White LED
}

void PLATFORM_IoInit(void)
{
    /* Pin Function Configuration */
    _UsbPinmux();
    _UsbIoInit();

    _MipiRxPinmux();
    _SensorPinmux();
    _UartPinmux();

    _LedPinmux();
}

void PLATFORM_PowerOff(void)
{
    /* Call the interface before powering off and sleeping */
}

int PLATFORM_PanelInit(void)
{
    return CVI_SUCCESS;
}

void PLATFORM_PanelBacklightCtl(int level) {}

void PLATFORM_SpkMute(int value)
{
    // 0 Mute, 1 Non-Mute
    if (value == 0) {
        cvi_gpio_set_val(GPIO_SPKEN_GRP, GPIO_SPKEN_NUM, 0);
    } else {
        cvi_gpio_set_val(GPIO_SPKEN_GRP, GPIO_SPKEN_NUM, 1);
    }
}

int PLATFORM_IrCutCtl(int duty)
{
    return 0;
}

void PLATFORM_CLK_AXI4_Restore()
{
    // restore axi4 to 300M
    if (mmio_read_32(0x030020B8) != 0x00050009) {
        mmio_write_32(0x030020B8, 0x00050009);
    }
}

void PLATFORM_LightCtl(int type, int value)
{
    if (type) {
        cvi_gpio_set_val(4, 26, !!value);
    } else {
        cvi_gpio_set_val(4, 25, !!value);
    }
}
void led_switch(int argc, char** argv)
{
    if (argc < 3) {
        printf("led_switch 0/1(rgb/ir) 0/1(off/on)");
        return;
    }
    int type  = atoi(argv[1]);
    int value = atoi(argv[2]);
    PLATFORM_LightCtl(type, value);
}
ALIOS_CLI_CMD_REGISTER(led_switch, led_switch, led_switch);

extern uint8_t isp_param_ir_face[];
extern uint32_t isp_param_ir_face_len;

uint8_t isp_param_ir_palm[174025];
uint32_t isp_param_ir_palm_len;
extern uint32_t isp_palm_diff_len;
extern uint16_t isp_palm_diff_offset[];
extern uint8_t isp_palm_diff_value[];

uint8_t isp_param_rgb[174025];
uint32_t isp_param_rgb_len;
extern uint32_t isp_rgb_diff_len;
extern uint8_t isp_rgb_diff_offset[];
extern uint8_t isp_rgb_diff_value[];

void cvi_cal_isp_param()
{
    uint32_t u32_offset = 0;

    isp_param_rgb_len = isp_param_ir_face_len;
    memcpy(isp_param_rgb, isp_param_ir_face, isp_param_ir_face_len);
    for (int i = 0; i < isp_rgb_diff_len; ++i) {
        u32_offset += isp_rgb_diff_offset[i];
        isp_param_rgb[u32_offset] = isp_rgb_diff_value[i];
    }

    u32_offset = 0;
    isp_param_ir_palm_len = isp_param_ir_face_len;
    memcpy(isp_param_ir_palm, isp_param_ir_face, isp_param_ir_face_len);

    for (int i = 0; i < isp_palm_diff_len; ++i) {
        u32_offset += isp_palm_diff_offset[i];
        isp_param_ir_palm[u32_offset] = isp_palm_diff_value[i];
    }

    return;
}
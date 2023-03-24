#include <drv/pin.h>
#include <pinctrl-mars.h>
#include <drv/pwm.h>
#include "platform.h"
#include "cvi_type.h"
#include "fc_custom_media.h"
#include <aos/cli.h>

#define GPIO_LCD_BLEN_GRP 1
#define GPIO_LCD_BLEN_NUM 6
#define GPIO_SPKEN_GRP 0
#define GPIO_SPKEN_NUM 15
#define PWM_IRLED_BANK 2
#define PWM_IRLED_CHN 0

#define GPIO_PIN_MASK(_gpio_num) (1 << _gpio_num)

static void _IrCutCtl_Pinmux()
{
    PINMUX_CONFIG(PWR_GPIO0, PWM_8);//切 IRLED pinmux
}

static void _SensorPinmux()
{
    //Sensor Pinmux
    PINMUX_CONFIG(ADC1, XGPIOB_3); // RGB PWDN
    PINMUX_CONFIG(PWR_BUTTON1, PWR_GPIO_8); // IR PWDN
    PINMUX_CONFIG(PAD_MIPI_TXP0, CAM_MCLK0); // MCLK0
    PINMUX_CONFIG(PWR_GPIO1, IIC2_SCL); // I2C 2
    PINMUX_CONFIG(PWR_GPIO2, IIC2_SDA);
    PINMUX_CONFIG(PAD_MIPIRX2N, IIC4_SCL); // I2C 4
    PINMUX_CONFIG(PAD_MIPIRX2P, IIC4_SDA);
    PINMUX_CONFIG(PWR_GPIO0, PWR_GPIO_0); // ir led
}

static void _MipiRxPinmux(void)
{
    //mipi rx pinmux
    PINMUX_CONFIG(PAD_MIPIRX4N, XGPIOC_2); // IR DATA LANE
    PINMUX_CONFIG(PAD_MIPIRX4P, XGPIOC_3);
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
	csi_gpio_uninit(&gpio);
}

int PLATFORM_PanelInit(void)
{
	return CVI_SUCCESS;
}

void PLATFORM_PanelBacklightCtl(int level)
{
    PINMUX_CONFIG(USB_VBUS_DET, XGPIOB_6);
    if(level > 0) {
        _GPIOSetValue(GPIO_LCD_BLEN_GRP,GPIO_LCD_BLEN_NUM,1);
    } else {
        _GPIOSetValue(GPIO_LCD_BLEN_GRP,GPIO_LCD_BLEN_NUM,0);
    }
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
    _MipiRxPinmux();
    _MipiTxPinmux();
    _SensorPinmux();
    _IrCutCtl_Pinmux();
    CustomEvent_IRGpioSet(0);
#if (CONFIG_APP_DEBUG_JTAG == 1)
    JTAG_PinmuxIn();
#endif
    PLATFORM_PanelBacklightCtl(100);
}

void PLATFORM_PowerOff(void)
{
//下电休眠前调用接口
}

void PLATFORM_SpkMute(int value)
{
//0静音 ，1非静音
    if(value == 0) {
        _GPIOSetValue(GPIO_SPKEN_GRP, GPIO_SPKEN_NUM, 0);
    } else {
        _GPIOSetValue(GPIO_SPKEN_GRP, GPIO_SPKEN_NUM, 1);
    }
}

int PLATFORM_IrCutCtl(int duty)
{
    if(duty < 0) {
        return -1;
    }
    csi_pwm_t pwm;
    if(csi_pwm_init(&pwm, PWM_IRLED_BANK) == 0 ){
        csi_pwm_out_stop(&pwm, PWM_IRLED_CHN);
        if(duty != 0) {
            //printf("PLATFORM_IrCutCtl duty %d \n",duty);
            csi_pwm_out_config(&pwm, PWM_IRLED_CHN, 10000, duty, PWM_POLARITY_HIGH);
            return csi_pwm_out_start(&pwm, PWM_IRLED_CHN);
        } else {
            return 0;
        }
    } else {
        printf("csi_pwm_init err \n");
        return -1;
    }
    return -1;
}

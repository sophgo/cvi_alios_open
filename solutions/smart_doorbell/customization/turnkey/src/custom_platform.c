#include "platform.h"
#include <drv/pin.h>
#include <pinctrl-mars.h>
#include "cvi_type.h"
#include <mmio.h>
#include <stdint.h>
#include <aos/cli.h>
#include "cvi_param.h"

#define GPIO_SPKEN_GRP 0
#define GPIO_SPKEN_NUM 15

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

static void _SensorPinmux()
{
#if (CONFIG_QFN_PANEL_REWORK == 1)
    //改板
    PINMUX_CONFIG(SD1_D3, PWR_GPIO_18);
    PINMUX_CONFIG(SD1_D2, CAM_MCLK0);
    PINMUX_CONFIG(PWR_GPIO1, IIC2_SCL); // I2C 2
    PINMUX_CONFIG(PWR_GPIO2, IIC2_SDA);
#else
    // Sensor Pinmux
    PINMUX_CONFIG(PAD_MIPIRX2N, XGPIOC_6);
    PINMUX_CONFIG(PAD_MIPIRX2P, XGPIOC_7);
    PINMUX_CONFIG(PAD_MIPIRX1P, IIC1_SDA);
    PINMUX_CONFIG(PAD_MIPIRX0N, IIC1_SCL);
    PINMUX_CONFIG(PAD_MIPIRX0P, CAM_MCLK0);

    _GPIOSetValue(2, 6, 1); // XSHUTDOWN
    _GPIOSetValue(2, 7, 1); // XSHUTDOWN
#endif
}

static void _MipiRxPinmux(void)
{
//mipi rx pinmux
    PINMUX_CONFIG(PAD_MIPIRX4P, XGPIOC_3);
    PINMUX_CONFIG(PAD_MIPIRX4N, XGPIOC_2);
    PINMUX_CONFIG(PAD_MIPIRX3P, XGPIOC_5);
    PINMUX_CONFIG(PAD_MIPIRX3N, XGPIOC_4);
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
#elif (CONFIG_PANEL_GC9503CV)
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

static void _UartPinmux()
{
    PINMUX_CONFIG(IIC0_SDA, UART1_RX);
    PINMUX_CONFIG(IIC0_SCL, UART1_TX);
}
#if (CONFIG_SUPPORT_USB_HC || CONFIG_SUPPORT_USB_DC)

static void _UsbPinmux(void)
{
    // SOC_PORT_SEL
    PINMUX_CONFIG(PWR_GPIO0, PWR_GPIO_0);
    PINMUX_CONFIG(PWR_GPIO1, PWR_GPIO_1);
}

static void _UsbIoInit(void)
{
#if CONFIG_SUPPORT_USB_HC
    _GPIOSetValue(4, 0, 0);
    _GPIOSetValue(4, 1, 0);
#elif CONFIG_SUPPORT_USB_DC
    _GPIOSetValue(4, 0, 1);
    _GPIOSetValue(4, 1, 1);
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
    _UartPinmux();
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
    if(value == 0) {
        _GPIOSetValue(GPIO_SPKEN_GRP, GPIO_SPKEN_NUM, 0);
    } else {
        _GPIOSetValue(GPIO_SPKEN_GRP, GPIO_SPKEN_NUM, 1);
    }
}

int PLATFORM_IrCutCtl(int duty)
{
    return 0;
}

#if CONFIG_QUICK_STARTUP_SUPPORT
void PLATFORM_CLK_AXI4_Restore()
{
    // restore axi4 to 300M
    if (mmio_read_32(0x030020B8) != 0x00050009) {
        mmio_write_32(0x030020B8, 0x00050009);
    }
}
#endif

void PLATFORM_LightCtl(int type, int value)
{


    if (type) {
        PINMUX_CONFIG(SD1_GPIO1, PWR_GPIO_26);
        _GPIOSetValue(4, 26, !!value);
    }
    else {
        PINMUX_CONFIG(SD1_GPIO0, PWR_GPIO_25);
        _GPIOSetValue(4, 25, !!value);
    }
}
void led_switch(int argc, char** argv)
{
    if (argc < 3) {
        printf("led_switch 0/1(rgb/ir) 0/1(off/on)");
        return;
    }
    int type = atoi(argv[1]);
    int value = atoi(argv[2]);
    PLATFORM_LightCtl(type, value);
}
ALIOS_CLI_CMD_REGISTER(led_switch, led_switch, led_switch);

extern unsigned char cvi_turnkey_cv180zb_gc02m1_ir_pq_param_face[];
extern unsigned int cvi_turnkey_cv180zb_gc02m1_ir_pq_param_face_length;
extern unsigned char cvi_turnkey_cv180zb_gc02m1_ir_pq_param_palm[];
extern unsigned int cvi_turnkey_cv180zb_gc02m1_ir_pq_param_palm_length;
void PLATFORM_ISPSWITCH(int type, int value)
{
    if (type) {
        //IR
        if(value == 0){
            //face
            printf("face ir isp\n");
            PARAM_VI_CFG_S * pstViCfg = PARAM_getViCtx();
            PARAM_PQ_BIN_DESC_S * pqbindes = NULL;
            int scene_mode = 0;
            CVI_U8 devNum = 1;
            pqbindes = &(pstViCfg->pstIspCfg[scene_mode].astPQBinDes[devNum]);
            pqbindes->pIspBinData = cvi_turnkey_cv180zb_gc02m1_ir_pq_param_face;
            pqbindes->u32IspBinDataLen = cvi_turnkey_cv180zb_gc02m1_ir_pq_param_face_length;
            if (pqbindes->pIspBinData && pqbindes->u32IspBinDataLen > 0) {
                int ret = CVI_BIN_LoadParamFromBinEx(pqbindes->binID,pqbindes->pIspBinData,pqbindes->u32IspBinDataLen);
                if(ret != 0){
                    printf("CVI_BIN_LoadParamFromBinEx(%d) fail", pqbindes->binID);
                }
            }
            printf("face ir isp change finish\n");
        }
        else if(value == 1) {
            //palm
            printf("palm ir isp\n");
            PARAM_VI_CFG_S * pstViCfg = PARAM_getViCtx();
            PARAM_PQ_BIN_DESC_S * pqbindes = NULL;
            int scene_mode = 0;
            CVI_U8 devNum = 1;
            pqbindes = &(pstViCfg->pstIspCfg[scene_mode].astPQBinDes[devNum]);
            pqbindes->pIspBinData = cvi_turnkey_cv180zb_gc02m1_ir_pq_param_palm;
            pqbindes->u32IspBinDataLen = cvi_turnkey_cv180zb_gc02m1_ir_pq_param_palm_length;
            if (pqbindes->pIspBinData && pqbindes->u32IspBinDataLen > 0) {
                int ret = CVI_BIN_LoadParamFromBinEx(pqbindes->binID, pqbindes->pIspBinData, pqbindes->u32IspBinDataLen);
                if(ret != 0){
                    printf("CVI_BIN_LoadParamFromBinEx(%d) fail", pqbindes->binID);
                }
            }
            printf("palm ir isp change finish\n");
        }
        else{
            printf("do nothing\n");
        }
    }
    else {
        //RGB
        printf("Currently, RGB cannot change ISP\n");
    }
}

void isp_switch(int argc, char** argv)
{
    if (argc < 3) {
        printf("ir_param_switch 0/1(rgb/ir) 0/1(param1/param2)\n");
        return;
    }
    int type = atoi(argv[1]);
    int value = atoi(argv[2]);
    PLATFORM_ISPSWITCH(type, value);
}
ALIOS_CLI_CMD_REGISTER(isp_switch, isp_switch, isp_switch);
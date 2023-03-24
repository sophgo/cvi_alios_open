#include <aos/cli.h>
#include "fc_custom_media.h"
#include "platform.h"
#include "ir_custom_param.h"
#include "cvi_param.h"
#include "media_video.h"
#include "cvi_buffer.h"
#include "cvi_sys.h"
#include "cvi_vb.h"
#include "cvi_vi.h"
#include "cvi_bin.h"

#define IR_PWDN_GRP 4
#define IR_PWDN_NUM 8
#define RGB_PWDN_GRP 1
#define RGB_PWDN_NUM 3
//#define IR_LED_GRP 4
//#define IR_LED_NUM 0
csi_gpio_t g_irgpio = {0};
csi_gpio_t g_rgbgpio = {0};
unsigned int g_gpio_init = 0;

void CustomEvent_IRGpioSet(int status)
{
    if(g_gpio_init == 0)  {
        //csi_gpio_t _irledgpio;
        if (csi_gpio_init(&g_irgpio, IR_PWDN_GRP) != CSI_OK) {
            printf("%s gpio init err \n",__func__);
            g_gpio_init = 0;
            return ;
        }
        if (csi_gpio_init(&g_rgbgpio, RGB_PWDN_GRP) != CSI_OK) {
            csi_gpio_uninit(&g_irgpio);
            g_gpio_init = 0;
            printf("%s gpio init err \n",__func__);
            return ;
        }
        g_gpio_init = 1;
    }

    csi_gpio_dir(&g_irgpio , (1 << IR_PWDN_NUM) , GPIO_DIRECTION_OUTPUT);
    csi_gpio_dir(&g_rgbgpio , (1 << RGB_PWDN_NUM) , GPIO_DIRECTION_OUTPUT);
    //csi_gpio_init(&_irledgpio, IR_LED_GRP);
    //csi_gpio_dir(&_irledgpio , (1 << IR_LED_NUM) , GPIO_DIRECTION_OUTPUT);
    if(status == 1) {
        //初始化IR GPIO 置高
        csi_gpio_write(&g_irgpio , (1 << IR_PWDN_NUM), 1);
        csi_gpio_write(&g_rgbgpio , (1 << RGB_PWDN_NUM), 0);
        //csi_gpio_write(&_irledgpio , (1 << IR_LED_NUM), 1);//开启IR_LED灯
        PLATFORM_IrCutCtl(10000);
    } else 
    {
        //切换RGB Sensor出流
        csi_gpio_write(&g_irgpio , (1 << IR_PWDN_NUM), 0);
        csi_gpio_write(&g_rgbgpio , (1 << RGB_PWDN_NUM), 1);
        //csi_gpio_write(&_irledgpio , (1 << IR_LED_NUM), 0);//关闭IR_LED灯
        PLATFORM_IrCutCtl(0);
    }
}

/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: HMAC module HCC layer adaptation.
 * Author: kongcheng
 * Create: 2018-08-04
 */

/* $)AM7ND<~0|:, */
#include "hi_types_base.h"
#include "oam_ext_if.h"
#include "wal_net.h"
#include "hcc_adapt.h"
//#include "wal_netlink.h"
//#include "pin_name.h"
//#include "drv_gpio.h"
//#include "pinmux.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
/*
void hcc_gpio_set(pin_name_e gpio_idx, bool value)
{ 
    gpio_pin_handle_t pin = NULL;
    pin = csi_gpio_pin_initialize(gpio_idx, NULL);
    csi_gpio_pin_config_mode(pin, GPIO_MODE_PULLUP);
    
    csi_gpio_pin_config_direction(pin, GPIO_DIRECTION_OUTPUT);
    csi_gpio_pin_write(pin, value);   
}

#define GPIOC20_3V3_WIFI_DSI             PC20

void hcc_adapt_power_on(void)
{
    drv_pinmux_config(SDSLV_MUX,GPIO_MUX);
    hcc_gpio_set(GPIOC20_3V3_WIFI_DSI, 0);
    aos_msleep(10);
    hcc_gpio_set(GPIOC20_3V3_WIFI_DSI, 1);
    aos_msleep(2000);
}
*/
extern void test_sdio_do_isr();

/* $)A:/J}6(Re */
/* insmod hichannel.ko $)AHk?Z:/J} */
hi_s32 wlan_init(void)
{
    hi_s32 ret;
    oam_warning_log0("hichannel 2020-11-10 17:00:00\n");
    // hcc_adapt_power_on();
#if 0 
    /* $)ASC;'L,:MDZ:KL,=(A"5DA,=S */
    ret = oal_netlink_init();
    if (ret != HI_SUCCESS) {
        oam_error_log1("wlan_init:: oal_netlink_init FAILED[%d]", ret);
        goto fail;
    }
#endif

#if 0
    /* $)AN,2b3uJ<;/ */
    ret = oam_main_init();
    if (ret != HI_SUCCESS) {
        oam_error_log1("wlan_init:: oam_main_init FAILED[%d]", ret);
        goto oam_main_init_fail;
    }
#endif
    //aos_task_t oal_isr_task_thread;
    //aos_task_new_ext(&oal_isr_task_thread, "oal_sdio_task", test_sdio_do_isr, NULL, 1024*1024, 20);

    wal_irq_config();
    //aos_msleep(100);
    ret = hcc_adapt_init();
    if (ret != HI_SUCCESS) {
        goto hcc_host_init_fail;
    }
#if 0
    ret = netdev_register();
    if (ret != HI_SUCCESS) {
        oam_error_log0("wlan_init:: netdev_register failed");
        return ret;
    }
#endif
    //wal_irq_config();
    //aos_task_t oal_isr_task_thread;
    //aos_task_new_ext(&oal_isr_task_thread, "oal_sdio_task", test_sdio_do_isr, NULL, 1024*1024, 20);

    oam_warning_log0("wlan drv insmod SUCCESSFULLY\n");
    return HI_SUCCESS;

hcc_host_init_fail:
    //oam_main_exit();
// oam_main_init_fail:
    //oal_netlink_deinit();
// fail:
    return -HI_FAILURE;
}

/* rmmdo hichannel.ko $)AHk?Z:/J} */
// static void wlan_deinit(void)
// {
//     //netdev_unregister();
//     hcc_adapt_exit();
//     //oam_main_exit();
//     //oal_netlink_deinit();
//     return;
// }

#ifdef __cplusplus
#if __cplusplus
    }
#endif
#endif

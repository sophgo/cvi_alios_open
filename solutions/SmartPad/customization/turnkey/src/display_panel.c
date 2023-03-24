#include <stdio.h>
#include <pthread.h>
#include <aos/kernel.h>
#include <aos/cli.h>
#include <ulog/ulog.h>
#include <lvgl.h>
#include <devices/display.h>
#include "display_panel.h"
#include "st7701s_devops.h"
#include "gui_guider.h"
#include "sys/prctl.h"

lv_ui guider_ui;
extern void lv_port_disp_init(void);
extern void lv_port_indev_init(void);

void lvgl_gui_init()
{
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
    setup_ui(&guider_ui);
}

void * _display_hander(void * args)
{
    prctl(PR_SET_NAME, "_gui_hander");
    /* handle the tasks of LVGL */
    while(1) {
        lv_task_handler();
        aos_msleep(1);
    }
}

void Display_Panel_Init()
{
    st7701s_gpio_pin _gpio_pin = {
        .blank_pwm_bank = 2,
        .blank_pwm_chn = 0,
    };
    drv_st7701s_display_register(&_gpio_pin);
    lvgl_gui_init();
    pthread_t disp_threadid;
    pthread_attr_t attr = {0};
    struct sched_param param;
    pthread_attr_init(&attr);
    param.sched_priority = 31;
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setstacksize(&attr, 8 * 1024);
    pthread_create(&disp_threadid, &attr, _display_hander, NULL);
}

void Display_Panel_Destory()
{
}

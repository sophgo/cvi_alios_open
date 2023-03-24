#include <devices/input.h>
#include "touchscreen.h"
#include "gsl1680_devops.h"

#if 1
static rvm_dev_t * touch_dev;

static void touchscreen_event(rvm_dev_t *dev, int event_id, void *arg)
{
    int res = 0;
    rvm_hal_input_event_t   tevent;
    static uint8_t  down   = false;
    static uint8_t  moving = false;
    static uint16_t x      = 0;
    static uint16_t y      = 0;

    while(1) {
        res = rvm_hal_input_read(dev, &tevent, sizeof(tevent), 0);
        if (res < (int)sizeof(tevent)) {
            break;
        }
        if (tevent.type == EV_ABS) {
            if (tevent.code == ABS_X) {
                x = tevent.value;
            } else if (tevent.code == ABS_Y) {
                y = tevent.value;
            }
        } else if (tevent.type == EV_SYN && tevent.code == SYN_REPORT) {
            if (down) {
                down   = false;
                moving = true;
                printf("touch pressed x: %d y: %d\n", x, y);
            }
            if (moving) {
                printf("touch move x: %d y: %d \n", x, y);
            }
        } else if (tevent.type == EV_KEY && tevent.code == BTN_TOUCH) {
            if (tevent.value == 1) {
                down = true;
            } else if (tevent.value == 0) {
                down   = false;
                moving = false;
                printf("touch unpressed x: %d y: %d\n", x, y);
            }
        }
    }
}
#endif
int Touchscreen_Init()
{
    gsl1680_gpio_pin _gpio_pin = {
        .iic_id = 3,
        .reset_gpio_polarity = 1,
        .reset_gpio_group = 0,
        .reset_gpio_chn = 18,
        .interrupt_gpio_group = 0,
        .interrupt_gpio_chn = 19,
    };
    drv_gsl1680_devops_register(&_gpio_pin);
    touch_dev = rvm_hal_input_open("input");
    rvm_hal_input_set_event(touch_dev, touchscreen_event, NULL);
    return 0;
}








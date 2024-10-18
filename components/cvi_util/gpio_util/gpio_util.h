#ifndef __GPIO_UTIL_H__
#define __GPIO_UTIL_H__

#include <drv/gpio.h>
#include <stdint.h>

typedef enum {
    GPIO_LEVEL_LOW,
    GPIO_LEVEL_HIGH,
} cvi_gpio_level_e;

void cvi_gpio_set_val(uint8_t u8_gpio_grp, uint8_t u8_gpio_num, cvi_gpio_level_e e_level);
void cvi_gpio_get_val(uint8_t u8_gpio_grp, uint8_t u8_gpio_num, cvi_gpio_level_e* e_level);

#endif
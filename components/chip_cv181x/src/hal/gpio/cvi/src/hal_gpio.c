/*
* Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
*/

#include <stdbool.h>
#include <stdint.h>
#include "hal_gpio.h"

void hal_gpio_uninit(unsigned long reg_base)
{
    /* reset all related register*/
    //dw_gpio_set_port_irq(reg_base, 0U);
    //dw_gpio_set_port_msk(reg_base, 0U);
    //dw_gpio_set_port_irq_sensitive(reg_base, 0U);
    //dw_gpio_set_port_irq_polarity(reg_base, 0U);
}
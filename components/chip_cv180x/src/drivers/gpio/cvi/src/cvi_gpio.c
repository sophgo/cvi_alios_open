#include "hal_gpio.h"
#include "cvi_gpio.h"
#include "cvi_pin.h"
#include <drv/cvi_irq.h>

static void cvi_gpio_irqhandler(unsigned int irqn, void *args)
{
    cvi_gpio_t *gpio = (cvi_gpio_t *)args;
    uint32_t bitmask;
    unsigned long reg_base = GET_DEV_REG_BASE(gpio);

    bitmask = dw_gpio_read_port_int_status(reg_base);

    /* clear all interrput */
    dw_gpio_clr_port_irq(reg_base, bitmask);

    /* execute the callback function */
    if (gpio->callback) {
        gpio->callback(gpio, bitmask, gpio->args);
    }
}

cvi_error_t cvi_gpio_init(cvi_gpio_t *gpio)
{
	unsigned long reg_base = GET_DEV_REG_BASE(gpio);
	uint8_t irqn = GET_DEV_IRQ_NUM(gpio);

	if (reg_base && irqn)
	{
		pr_debug("gpio init ok! reg_base: 0x%x, bank: %d, irq_num: %d\n", reg_base, GET_DEV_IDX(gpio), irqn);
		return CVI_OK;
	}
	else
	{
		pr_err("gpio init failed! reg_base: 0x%x, bank: %d, irq_num: %d\n", reg_base, GET_DEV_IDX(gpio), irqn);
		return CVI_ERROR;
	}
}

cvi_error_t cvi_gpio_init_one(cvi_gpio_t *gpio)
{
	unsigned long reg_base = GET_DEV_REG_BASE(gpio);
	uint8_t irqn = GET_DEV_IRQ_NUM(gpio);

	if (reg_base && irqn)
	{
		pr_debug("gpio init ok! reg_base: 0x%x, pin: %d, irq_num: %d\n", reg_base, GET_DEV_IDX(gpio), irqn);
		return CVI_OK;
	}
	else
	{
		pr_err("gpio init failed! reg_base: 0x%x, pin: %d, irq_num: %d\n", reg_base, GET_DEV_IDX(gpio), irqn);
		return CVI_ERROR;
	}
}

void cvi_gpio_uninit(cvi_gpio_t *gpio)
{
    /* reset all related register*/
	hal_gpio_uninit(GET_DEV_REG_BASE(gpio));
}

cvi_error_t cvi_gpio_set_dir(cvi_gpio_t *gpio, uint32_t pin_mask, cvi_gpio_dir_t dir)
{
	int ret = CVI_OK;
	unsigned long reg_base = GET_DEV_REG_BASE(gpio);

    uint32_t tmp = dw_gpio_read_port_direction(reg_base);

    switch (dir) {
        case GPIO_DIR_INPUT:
            dw_gpio_set_port_direction(reg_base, tmp & (~pin_mask));
            break;

        case GPIO_DIR_OUTPUT:
            dw_gpio_set_port_direction(reg_base, tmp | pin_mask);
            break;

        default:
            ret = CVI_UNSUPPORTED;
			pr_err("gpio reg_base 0x%x set dir %d error\n", reg_base, dir);
            break;
    }

	return ret;
}

cvi_error_t cvi_gpio_set_mode(cvi_gpio_t *gpio, uint32_t pin_mask, cvi_pin_mode_t mode)
{
    cvi_error_t ret = CSI_OK;
    cvi_error_t temp;
    uint8_t offset = 0U;
    pin_name_t pin_name = 0;

    /* set pin mode */
    while (pin_mask) {
        if (pin_mask & 0x01U) {
            pin_name = cvi_pin_get_pinname_by_gpio(gpio->dev.idx, offset);

            if ((uint8_t)pin_name != 0xFFU) {
                temp = cvi_pin_set_mode(pin_name, mode);
                if (temp == CVI_ERROR) {
                    ret = CVI_ERROR;
                    pr_err("gpio %d set mode %d failed\n", pin_name, mode);
                    break;
                } else if (temp == CVI_UNSUPPORTED) {
                    pr_err("gpio %d mode %d unsupported\n", pin_name, mode);
                    ret = CVI_UNSUPPORTED;
                }
            }
        }

        pin_mask >>= 1U;
        offset++;
    }

    pr_debug("gpio %d number: %d\n", gpio->dev.idx, cvi_pin_get_gpio_pinnum(pin_name));

    return ret;
}

cvi_error_t cvi_gpio_irq_mode(cvi_gpio_t *gpio, uint32_t pin_mask, cvi_gpio_irq_flag_t flag)
{
	cvi_error_t ret = CVI_OK;
    uint32_t senstive, polarity;
	unsigned long reg_base = GET_DEV_REG_BASE(gpio);

    senstive = dw_gpio_read_port_irq_sensitive(reg_base);
    polarity = dw_gpio_read_port_irq_polarity(reg_base);

    switch (flag) {
        /* rising edge interrupt mode */
        case GPIO_IRQ_FLAG_RISING_EDGE:
            dw_gpio_set_port_irq_sensitive(reg_base, senstive | pin_mask);
            dw_gpio_set_port_irq_polarity(reg_base, polarity | pin_mask);
            break;

        /* falling edge interrupt mode */
        case GPIO_IRQ_FLAG_FALLING_EDGE:
            dw_gpio_set_port_irq_sensitive(reg_base, senstive | pin_mask);
            dw_gpio_set_port_irq_polarity(reg_base, polarity & (~pin_mask));
            break;

        /* low level interrupt mode */
        case GPIO_IRQ_FLAG_LOW_LEVEL:
            dw_gpio_set_port_irq_sensitive(reg_base, senstive & (~pin_mask));
            dw_gpio_set_port_irq_polarity(reg_base, polarity & (~pin_mask));
            break;

        /* high level interrupt mode */
        case GPIO_IRQ_FLAG_HIGH_LEVEL:
            dw_gpio_set_port_irq_sensitive(reg_base, senstive & (~pin_mask));
            dw_gpio_set_port_irq_polarity(reg_base, polarity | pin_mask);
            break;

        /* double edge interrupt mode */
        case GPIO_IRQ_FLAG_BOTH_EDGE:
        default:
            ret = CVI_UNSUPPORTED;
			pr_err("gpio reg_base 0x%x set irq flag %d error\n", reg_base, flag);
            break;
    }

    return ret;
}

cvi_error_t cvi_gpio_debounce(cvi_gpio_t *gpio, uint32_t pin_mask, bool enable)
{
	unsigned long reg_base = GET_DEV_REG_BASE(gpio);
    uint32_t temp = dw_gpio_get_debounce(reg_base);

    if (enable)
        dw_gpio_set_debounce(reg_base, temp | pin_mask);
    else
        dw_gpio_set_debounce(reg_base, temp & (~pin_mask));

    return CVI_OK;
}

uint32_t cvi_gpio_read(cvi_gpio_t *gpio, uint32_t pin_mask)
{
    return dw_gpio_read_input_port(GET_DEV_REG_BASE(gpio)) & pin_mask;
}

void cvi_gpio_write(cvi_gpio_t *gpio, uint32_t pin_mask, cvi_gpio_state_t value)
{
	unsigned long reg_base = GET_DEV_REG_BASE(gpio);
    uint32_t tmp = dw_gpio_read_output_port(reg_base);

    if (value == 1) {
        dw_gpio_write_output_port(reg_base, tmp | pin_mask);
    } else {
        dw_gpio_write_output_port(reg_base, tmp & (~pin_mask));
    }
}

void cvi_gpio_toggle(cvi_gpio_t *gpio, uint32_t pin_mask)
{
	unsigned long reg_base = GET_DEV_REG_BASE(gpio);
    uint32_t tmp = dw_gpio_read_output_port(reg_base);
    dw_gpio_write_output_port(reg_base, tmp ^ pin_mask);
}

void cvi_gpio_irq_enable(cvi_gpio_t *gpio, uint32_t pin_mask, bool enable)
{
	unsigned long reg_base = GET_DEV_REG_BASE(gpio);
    uint32_t temp = dw_gpio_read_port_irq(reg_base);

    if (enable) {
        dw_gpio_set_port_irq(reg_base, temp | pin_mask);
    } else {
        dw_gpio_set_port_irq(reg_base, temp & (~pin_mask));
    }
}

void cvi_gpio_configure_irq(cvi_gpio_t *gpio, void *callback, void *args)
{
	gpio->callback = callback;
	gpio->args = args;

	if (callback)
	{
		/* clear interrput status before enable irq */
		dw_gpio_clr_port_irq(GET_DEV_REG_BASE(gpio), 0U);
		request_irq(gpio->dev.irq_num, &cvi_gpio_irqhandler, 0, "gpio int", gpio);
	}
	else
	{
		csi_irq_disable((uint32_t)gpio->dev.irq_num);
	}
}



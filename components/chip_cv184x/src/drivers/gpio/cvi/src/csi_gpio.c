/*
* Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
*/

#include <drv/gpio.h>
#include "cvi_gpio.h"
#include <drv/cvi_irq.h>
#include <drv/pin.h>
#include "aos/cli.h"

static void dw_gpio_irqhandler(unsigned int irqn, void *args)
{
    csi_gpio_t *handle = (csi_gpio_t *)args;
    uint32_t bitmask;
	struct gpio_irq_list_node *list = NULL;

    unsigned long reg_base = HANDLE_REG_BASE(handle);
    bitmask = dw_gpio_read_port_int_status(reg_base);

	list = csi_gpio_irq_list_each(handle, bitmask);
	/* now alios will not clear irq if the irq unregister*/
	if (list)
		dw_gpio_clr_port_irq(reg_base, bitmask);
	else
		return;
	/* execute the callback function */
	if (list->callback)
		list->callback(handle, bitmask, handle->arg);
	else if (handle->callback)
		handle->callback(handle, bitmask, handle->arg);
}

csi_error_t csi_gpio_init(csi_gpio_t *gpio, uint32_t port_idx)
{
    CSI_PARAM_CHK(gpio, CSI_ERROR);

    csi_error_t ret = CSI_OK;

    if (target_get(DEV_DW_GPIO_TAG, port_idx, &gpio->dev) != CSI_OK) {
        pr_err("gpio bank %d init failed!\n", port_idx);
        ret = CSI_ERROR;
    }

    pr_debug("gpio %d reg_base 0x%x\n", gpio->dev.idx, gpio->dev.reg_base);
	return ret;
}

void csi_gpio_uninit(csi_gpio_t *gpio)
{
    CSI_PARAM_CHK_NORETVAL(gpio);

    /* unregister irq */
    csi_irq_disable((uint32_t)gpio->dev.irq_num);
    csi_irq_detach((uint32_t)gpio->dev.irq_num);

    /* release handle */
    memset(gpio, 0, sizeof(csi_gpio_t));
}

csi_error_t csi_gpio_dir(csi_gpio_t *gpio, uint32_t pin_mask, csi_gpio_dir_t dir)
{
    CSI_PARAM_CHK(gpio, CSI_ERROR);
    CSI_PARAM_CHK(pin_mask, CSI_ERROR);

    csi_error_t ret = CSI_OK;

    unsigned long reg_base = HANDLE_REG_BASE(gpio);
    uint32_t tmp = dw_gpio_read_port_direction(reg_base);

    switch (dir) {
        case GPIO_DIRECTION_INPUT:
            dw_gpio_set_port_direction(reg_base, tmp & (~pin_mask));
            break;

        case GPIO_DIRECTION_OUTPUT:
            dw_gpio_set_port_direction(reg_base, tmp | pin_mask);
            break;

        default:
            ret = CSI_UNSUPPORTED;
            break;
    }

    return ret;
}

csi_error_t csi_gpio_mode(csi_gpio_t *gpio, uint32_t pin_mask, csi_gpio_mode_t mode)
{
    CSI_PARAM_CHK(gpio, CSI_ERROR);
    CSI_PARAM_CHK(pin_mask, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    csi_error_t temp;
    uint8_t offset = 0U;
    pin_name_t pin_name = 0;

    /* set pin mode */
    while (pin_mask) {
        if (pin_mask & 0x01U) {
            pin_name = csi_pin_get_pinname_by_gpio(gpio->dev.idx, offset);

            if ((uint8_t)pin_name != 0xFFU) {
                temp = csi_pin_mode(pin_name, mode);
                if (temp == CSI_ERROR) {
                    ret = CSI_ERROR;
                    pr_err("gpio %d set mode %d failed\n", pin_name, mode);
                    break;
                } else if (temp == CSI_UNSUPPORTED) {
                    pr_err("gpio %d mode %d unsupported\n", pin_name, mode);
                    ret = CSI_UNSUPPORTED;
                }
            }
        }

        pin_mask >>= 1U;
        offset++;
    }

    pr_debug("gpio %d number: %d\n", gpio->dev.idx, csi_pin_get_gpio_channel(pin_name));

    return ret;
}

csi_error_t csi_gpio_irq_mode(csi_gpio_t *gpio, uint32_t pin_mask, csi_gpio_irq_mode_t mode)
{
    CSI_PARAM_CHK(gpio, CSI_ERROR);

    uint32_t senstive, polarity;
    csi_error_t    ret = CSI_OK;
    unsigned long reg_base = HANDLE_REG_BASE(gpio);

    senstive = dw_gpio_read_port_irq_sensitive(reg_base);
    polarity = dw_gpio_read_port_irq_polarity(reg_base);

    switch (mode) {
        /* rising edge interrupt mode */
        case GPIO_IRQ_MODE_RISING_EDGE:
            dw_gpio_set_port_irq_sensitive(reg_base, senstive | pin_mask);
            dw_gpio_set_port_irq_polarity(reg_base, polarity | pin_mask);
            break;

        /* falling edge interrupt mode */
        case GPIO_IRQ_MODE_FALLING_EDGE:
            dw_gpio_set_port_irq_sensitive(reg_base, senstive | pin_mask);
            dw_gpio_set_port_irq_polarity(reg_base, polarity & (~pin_mask));
            break;

        /* low level interrupt mode */
        case GPIO_IRQ_MODE_LOW_LEVEL:
            dw_gpio_set_port_irq_sensitive(reg_base, senstive & (~pin_mask));
            dw_gpio_set_port_irq_polarity(reg_base, polarity & (~pin_mask));
            break;

        /* high level interrupt mode */
        case GPIO_IRQ_MODE_HIGH_LEVEL:
            dw_gpio_set_port_irq_sensitive(reg_base, senstive & (~pin_mask));
            dw_gpio_set_port_irq_polarity(reg_base, polarity | pin_mask);
            break;

        /* double edge interrupt mode */
        case GPIO_IRQ_MODE_BOTH_EDGE:
        default:
            ret = CSI_UNSUPPORTED;
            break;
    }

    return ret;
}

csi_error_t csi_gpio_irq_enable(csi_gpio_t *gpio, uint32_t pin_mask, bool enable)
{
    CSI_PARAM_CHK(gpio, CSI_ERROR);
    CSI_PARAM_CHK(pin_mask, CSI_ERROR);

    unsigned long reg_base = HANDLE_REG_BASE(gpio);
    uint32_t temp = dw_gpio_read_port_irq(reg_base);

    if (enable) {
        dw_gpio_set_port_irq(reg_base, temp | pin_mask);
    } else {
        dw_gpio_set_port_irq(reg_base, temp & (~pin_mask));
    }

    return CSI_OK;
}

csi_error_t csi_gpio_debounce(csi_gpio_t *gpio, uint32_t pin_mask, bool enable)
{
    CSI_PARAM_CHK(gpio, CSI_ERROR);
    CSI_PARAM_CHK(pin_mask, CSI_ERROR);

    unsigned long reg_base = HANDLE_REG_BASE(gpio);
    uint32_t temp = dw_gpio_get_debounce(reg_base);

    if (enable) {
        dw_gpio_set_debounce(reg_base, temp | pin_mask);
    } else {
        dw_gpio_set_debounce(reg_base, temp & (~pin_mask));
    }
    return CSI_OK;
}

void csi_gpio_write(csi_gpio_t *gpio, uint32_t pin_mask, csi_gpio_pin_state_t value)
{
    CSI_PARAM_CHK_NORETVAL(gpio);
    CSI_PARAM_CHK_NORETVAL(pin_mask);

    unsigned long reg_base = HANDLE_REG_BASE(gpio);
    uint32_t tmp = dw_gpio_read_output_port(reg_base);

    if (value == 1) {
        dw_gpio_write_output_port(reg_base, tmp | pin_mask);
    } else {
        dw_gpio_write_output_port(reg_base, tmp & (~pin_mask));
    }
}

uint32_t csi_gpio_read(csi_gpio_t *gpio, uint32_t pin_mask)
{
    CSI_PARAM_CHK(gpio, CSI_ERROR);
    CSI_PARAM_CHK(pin_mask, CSI_ERROR);

    unsigned long reg_base = HANDLE_REG_BASE(gpio);
    return dw_gpio_read_input_port(reg_base) & pin_mask;
}

void csi_gpio_toggle(csi_gpio_t *gpio, uint32_t pin_mask)
{
    CSI_PARAM_CHK_NORETVAL(gpio);
    CSI_PARAM_CHK_NORETVAL(pin_mask);

    unsigned long reg_base = HANDLE_REG_BASE(gpio);
    uint32_t tmp = dw_gpio_read_output_port(reg_base);

    dw_gpio_write_output_port(reg_base, tmp ^ pin_mask);
}

csi_error_t csi_gpio_attach_callback(csi_gpio_t *gpio, void *callback, void *arg)
{

    CSI_PARAM_CHK(gpio, CSI_ERROR);
    //CSI_PARAM_CHK(callback, CSI_ERROR);

    unsigned long reg_base = HANDLE_REG_BASE(gpio);

    /* clear interrput status before enable irq */
    dw_gpio_clr_port_irq(reg_base, 0U);
	if (callback)
		gpio->callback = callback;
	gpio->arg = arg;

#if 0
    csi_irq_attach((uint32_t)gpio->dev.irq_num, &dw_gpio_irqhandler, gpio);
#else
    request_irq((uint32_t)(gpio->dev.irq_num), &dw_gpio_irqhandler, 0, "gpio int", gpio);
#endif

    csi_irq_enable((uint32_t)gpio->dev.irq_num);

    return CSI_OK;
}

void csi_gpio_detach_callback(csi_gpio_t *gpio)
{
    CSI_PARAM_CHK_NORETVAL(gpio);

    gpio->callback = NULL;
    gpio->arg      = NULL;
}



#define  csi_gpio_irq_node_alloc  (struct gpio_irq_list_node *)malloc(sizeof(struct gpio_irq_list_node))

csi_error_t csi_gpio_irq_list_del(csi_gpio_t *gpio, struct gpio_irq_list_node **list)
{
	struct gpio_irq_list_node *head = gpio->irq_list_head;

	if (*list == head) {
		gpio->irq_list_head = aos_container_of(head->node.next, struct gpio_irq_list_node, node);
		if (head == gpio->irq_list_head)
			gpio->irq_list_head = NULL;
	}
	dlist_del(&(*list)->node);
	free(*list);
	return CSI_OK;
}

csi_error_t csi_gpio_irq_list_add(csi_gpio_t *gpio, struct gpio_irq_list_node **list)
{
	struct gpio_irq_list_node *head = gpio->irq_list_head;
	*list = csi_gpio_irq_node_alloc;
	if (!(*list))
		return CSI_ERROR;
	(*list)->pin_mask = 0xffff;
	dlist_add_tail(&(*list)->node, &head->node);
	return CSI_OK;
}

csi_error_t csi_gpio_irq_list_init(csi_gpio_t *gpio)
{
	struct gpio_irq_list_node *head = csi_gpio_irq_node_alloc;

	if (!head)
		return CSI_ERROR;
	head->pin_mask = 0xffff;
	dlist_init(&head->node);
	gpio->irq_list_head = head;
	return CSI_OK;
}

struct gpio_irq_list_node *csi_gpio_irq_list_each(csi_gpio_t *gpio, uint32_t pin_mask)
{
	struct gpio_irq_list_node *list = NULL;
	struct gpio_irq_list_node *head = gpio->irq_list_head;

	dlist_for_each_entry(&head->node, list, struct gpio_irq_list_node, node) {
		if (list->pin_mask == pin_mask)
			return list;
	}
	return NULL;
}


csi_error_t csi_gpio_irq_register(csi_gpio_t *gpio, uint32_t pin_mask, void *callback, void *arg)
{
	struct gpio_irq_list_node *list = NULL;
	struct gpio_irq_list_node *head = gpio->irq_list_head;

	CSI_PARAM_CHK(gpio, CSI_ERROR);
	CSI_PARAM_CHK(callback, CSI_ERROR);
	if (!head) {
		if (csi_gpio_irq_list_init(gpio))
			return CSI_ERROR;
		list = head;
	} else {
		if (csi_gpio_irq_list_add(gpio, &list))
			return CSI_ERROR;
	}
	list->pin_mask = pin_mask;
	list->callback = callback;
	return csi_gpio_attach_callback(gpio, NULL, arg);
}

csi_error_t csi_gpio_irq_unregister(csi_gpio_t *gpio, uint32_t pin_mask, void *arg)
{
	struct gpio_irq_list_node *list = NULL;

	CSI_PARAM_CHK(gpio, CSI_ERROR);
	list = csi_gpio_irq_list_each(gpio, pin_mask);
	if (!list)
		return CSI_ERROR;
	csi_gpio_irq_list_del(gpio, &list);
	return CSI_OK;
}


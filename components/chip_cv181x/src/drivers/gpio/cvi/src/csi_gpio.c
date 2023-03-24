/*
* Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
*/

#include <drv/gpio.h>
#include "cvi_gpio.h"
#include <drv/cvi_irq.h>
#include <drv/pin.h>
#include "aos/cli.h"

csi_error_t csi_gpio_init(csi_gpio_t *gpio, uint32_t port_idx)
{
    CSI_PARAM_CHK(gpio, CSI_ERROR);

    csi_error_t ret = CSI_OK;

    if (target_get(DEV_DW_GPIO_TAG, port_idx, &gpio->dev) != CSI_OK) {
        aos_cli_printf("gpio bank %d init failed!\n", port_idx);
        ret = CSI_ERROR;
    }
    if(gpio->priv != NULL) {
        cvi_gpio_uninit(gpio->priv);
        free(gpio->priv);
        gpio->priv = NULL;
    }
    cvi_gpio_t *cvi_gpio = (cvi_gpio_t *)malloc(sizeof(*gpio));
    if (!cvi_gpio)
    {
        aos_cli_printf("gpio malloc failed! reg_base: 0x%x, idx: 0x%x\n", GET_DEV_REG_BASE(gpio), port_idx);
        return CSI_ERROR;
    }

    cvi_gpio->dev.reg_base = GET_DEV_REG_BASE(gpio);
    cvi_gpio->dev.idx = GET_DEV_IDX(gpio);
    cvi_gpio->dev.irq_num = GET_DEV_IRQ_NUM(gpio);
    ret = (csi_error_t)cvi_gpio_init(cvi_gpio);

    gpio->priv = cvi_gpio;

    return ret;
}

void csi_gpio_uninit(csi_gpio_t *gpio)
{
    CSI_PARAM_CHK_NORETVAL(gpio);
    if(gpio->priv == NULL) {
        return ;
    }
    /* reset all related register*/
    cvi_gpio_uninit(gpio->priv);
    free(gpio->priv);

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

	if (cvi_gpio_set_dir(gpio->priv, pin_mask, dir) < 0)
		ret = CSI_UNSUPPORTED;

    return ret;
}

csi_error_t csi_gpio_mode(csi_gpio_t *gpio, uint32_t pin_mask, csi_gpio_mode_t mode)
{
    CSI_PARAM_CHK(gpio, CSI_ERROR);
    CSI_PARAM_CHK(pin_mask, CSI_ERROR);
	return cvi_gpio_set_mode(gpio->priv, pin_mask, mode);
}

csi_error_t csi_gpio_irq_mode(csi_gpio_t *gpio, uint32_t pin_mask, csi_gpio_irq_mode_t mode)
{
    CSI_PARAM_CHK(gpio, CSI_ERROR);
    csi_error_t    ret = CSI_OK;

	if (cvi_gpio_irq_mode(gpio->priv, pin_mask, mode) < 0)
		ret = CSI_UNSUPPORTED;

	return ret;
}

csi_error_t csi_gpio_irq_enable(csi_gpio_t *gpio, uint32_t pin_mask, bool enable)
{
    CSI_PARAM_CHK(gpio, CSI_ERROR);
    CSI_PARAM_CHK(pin_mask, CSI_ERROR);

	cvi_gpio_irq_enable(gpio->priv, pin_mask, enable);

    return CSI_OK;
}

csi_error_t csi_gpio_debounce(csi_gpio_t *gpio, uint32_t pin_mask, bool enable)
{
    CSI_PARAM_CHK(gpio, CSI_ERROR);
    CSI_PARAM_CHK(pin_mask, CSI_ERROR);
    return CSI_UNSUPPORTED;
}

void csi_gpio_write(csi_gpio_t *gpio, uint32_t pin_mask, csi_gpio_pin_state_t value)
{
    CSI_PARAM_CHK_NORETVAL(gpio);
    CSI_PARAM_CHK_NORETVAL(pin_mask);
	cvi_gpio_write(gpio->priv, pin_mask, value);
}

uint32_t csi_gpio_read(csi_gpio_t *gpio, uint32_t pin_mask)
{
    CSI_PARAM_CHK(gpio, CSI_ERROR);
    CSI_PARAM_CHK(pin_mask, CSI_ERROR);
    return cvi_gpio_read(gpio->priv, pin_mask);
}

void csi_gpio_toggle(csi_gpio_t *gpio, uint32_t pin_mask)
{
    CSI_PARAM_CHK_NORETVAL(gpio);
    CSI_PARAM_CHK_NORETVAL(pin_mask);
    cvi_gpio_toggle(gpio->priv, pin_mask);
}

csi_error_t csi_gpio_attach_callback(csi_gpio_t *gpio, void *callback, void *arg)
{
    CSI_PARAM_CHK(gpio, CSI_ERROR);
    CSI_PARAM_CHK(callback, CSI_ERROR);
    cvi_gpio_configure_irq(gpio->priv, callback, arg);
    return CSI_OK;
}

void csi_gpio_detach_callback(csi_gpio_t *gpio)
{
    CSI_PARAM_CHK_NORETVAL(gpio);
    cvi_gpio_configure_irq(gpio->priv, NULL, NULL);
}

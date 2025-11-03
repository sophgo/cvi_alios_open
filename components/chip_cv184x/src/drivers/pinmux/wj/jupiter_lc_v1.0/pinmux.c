/*
 * Copyright (C) 2017-2019 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     pinmux.c
 * @brief    source file for the pinmux
 * @version  V1.0
 * @date     02. June 2017
 ******************************************************************************/

#include <stdint.h>
#include <stddef.h>
#include <drv/pin.h>
#include <drv/gpio.h>
#include "soc.h"
#include "wj_pinmux_ll.h"

/*******************************************************************************
 * function: danica_ioreuse_inital
 *
 * description:
 *   initial danica_pinmux
 *******************************************************************************/
csi_error_t csi_pin_set_mux(pin_name_t pin_name, pin_func_t pin_func)
{
    uint32_t val = 0U;
    uint32_t reg_val = 0U;
    uint32_t reg_mux0 = 0U;
    csi_error_t res = CSI_OK;

    if (pin_func == PIN_FUNC_GPIO) {
        val = readl(HOBBIT_GIPO0_PORTCTL_REG);
        val &= ~((uint32_t)1U << (uint32_t)pin_name);
        writel(val, HOBBIT_GIPO0_PORTCTL_REG);
        res =  CSI_OK;
    } else {
        val = readl(HOBBIT_GIPO0_PORTCTL_REG);
        val |= ((uint32_t)1U << (uint32_t)pin_name);
        writel(val, HOBBIT_GIPO0_PORTCTL_REG);

        if (pin_name >= PA16) {
            reg_mux0 = HOBBIT_IOMUX0H_REG;
            pin_name = pin_name - (pin_name_t)16;
        } else {
            reg_mux0 = HOBBIT_IOMUX0L_REG;
        }

        reg_val = ((uint32_t)0x3U << ((uint32_t)pin_name * 2U));
        /* reuse function select */
        val = readl(reg_mux0);
        val &= ~(reg_val);
        val |= ((uint32_t)pin_func << (2U * (uint32_t)pin_name));
        writel(val, reg_mux0);
        res =  CSI_OK;
    }

    return res;
}


csi_error_t csi_pin_mode(pin_name_t pin_name, csi_gpio_mode_t mode)
{
    uint32_t reg = 0U;
    uint32_t val = 0U;
    csi_error_t ret = CSI_OK;

    switch (mode) {
        case GPIO_MODE_PULLNONE:
        case GPIO_MODE_OPEN_DRAIN:
        case GPIO_MODE_PUSH_PULL:
            ret = CSI_OK;
            break;

        case GPIO_MODE_PULLUP:
            val = readl(HOBBIT_IOPD0_REG);

            if (val & ((uint32_t)1U << (uint32_t)pin_name)) {
                val &= ~((uint32_t)1U << (uint32_t)pin_name);
                writel(val, HOBBIT_IOPD0_REG);
            }

            reg = HOBBIT_IOPU0_REG;
            break;

        case GPIO_MODE_PULLDOWN:
            val = readl(HOBBIT_IOPU0_REG);

            if (val & ((uint32_t)1U << (uint32_t)pin_name)) {
                val &= ~((uint32_t)1U << (uint32_t)pin_name);
                writel(val, HOBBIT_IOPU0_REG);
            }

            reg = HOBBIT_IOPD0_REG;
            break;

        default:
            ret =  CSI_ERROR;
            break;
    }

    if(reg != 0U){
        val = readl(reg);
        val |= ((uint32_t)1U << (uint32_t)pin_name);
        writel(val, reg);
    }

    return ret;
}

csi_error_t csi_pin_wakeup(pin_name_t pin_name, bool enable)
{
    uint32_t val = 0U;
    csi_error_t ret = CSI_OK;

    val = readl(HOBBIT_WAKEUP_POLAR0_REG);
    val |= ((uint32_t)1U << (uint32_t)pin_name);
    writel(val, HOBBIT_WAKEUP_POLAR0_REG);

    if (enable) {
        val = readl(HOBBIT_WAKEUP0_REG);
        val |= ((uint32_t)1U << (uint32_t)pin_name);
        writel(val, HOBBIT_WAKEUP0_REG);
    } else {
        val = readl(HOBBIT_WAKEUP0_REG);
        val &= ~((uint32_t)1U << (uint32_t)pin_name);
        writel(val, HOBBIT_WAKEUP0_REG);
    }

    return ret;
}

/**
  \brief       get the pin function.
  \param[in]   pin       refs to pin_name_e.
  \return      pin function count
*/
pin_func_t csi_pin_get_mux(pin_name_t pin_name)
{
    uint32_t val = 0U;
    uint32_t reg_val = 0U;
    uint32_t reg_mux0 = 0U;
    uint8_t offset = (uint8_t)pin_name;
    uint32_t ret = 0U;

    /* gpio data source select */
    val = readl(HOBBIT_GIPO0_PORTCTL_REG);
    val &= ((uint32_t)1U << (uint32_t)offset);

    if (val != ((uint32_t)1U << (uint32_t)offset)) {
        ret = (uint32_t)PIN_FUNC_GPIO;
    } else {
        if (pin_name >= PA16) {
            offset = (uint8_t)pin_name - 16U;
            reg_mux0 = HOBBIT_IOMUX0H_REG;
        } else {
            offset = (uint8_t)pin_name;
            reg_mux0 = HOBBIT_IOMUX0L_REG;
        }

        reg_val = ((uint32_t)0x3U << (offset * 2U));
        /* reuse function select */
        val = readl(reg_mux0);
        val &= (reg_val);
        ret = ((uint32_t)val >> (2U * offset));
    }

    return (pin_func_t)ret;
}

/**
  \brief       set pin speed
  \param[in]   pin_name pin name, defined in soc.h.
  \param[in]   speed    io speed
  \return      error code
*/
csi_error_t csi_pin_speed(pin_name_t pin_name, csi_pin_speed_t speed)
{
    return CSI_OK;
}


/**
  \brief       set pin drive
  \param[in]   pin_name pin name, defined in soc.h.
  \param[in]   drive    io drive
  \return      error code
*/
csi_error_t csi_pin_drive(pin_name_t pin_name, csi_pin_drive_t drive)
{
    uint32_t val = 0U;
    uint32_t reg_val = 0U;
    uint32_t reg_mux0 = 0U;

    if (pin_name >= PA16) {
        reg_mux0 = HOBBIT_IODS0H_REG;
        pin_name = pin_name - (pin_name_t)16;
    } else {
        reg_mux0 = HOBBIT_IODS0L_REG;
    }

    reg_val = ((uint32_t)0x3U << ((uint32_t)pin_name * 2U));
    /* reuse function select */
    val = readl(reg_mux0);
    val &= ~(reg_val);
    val |= ((uint32_t)drive << (2U * (uint32_t)pin_name));
    writel(val, reg_mux0);

    return CSI_OK;
}

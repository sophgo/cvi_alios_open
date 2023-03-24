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

#ifdef CONFIG_XIP
#define ATTRIBUTE_DATA __attribute__((section(".ram.code")))
#else
#define ATTRIBUTE_DATA
#endif

#define SPARROW_GPIO_SEL_REG    (WJ_IOC_BASE + 0x00)
#define SPARROW_AFL_REG         (WJ_IOC_BASE + 0x04)
#define SPARROW_AFH_REG         (WJ_IOC_BASE + 0x08)
#define IO_PAD_00_CFG_CFG       (WJ_IOC_BASE + 0x0c)

#define IO_PULL_UP_VALUE        0x30
#define IO_PULL_DOWN_VALUE      0x10
#define IO_DRIVE_LV0_VALUE      0x00
#define IO_DRIVE_LV1_VALUE      0x07
#define IO_DRIVE_LV2_VALUE      0x0b
#define IO_DRIVE_LV3_VALUE      0x0f

#define readl(addr) \
    ({ unsigned int __v = (*(volatile uint32_t *) (addr)); __v; })

#define writel(b,addr) ((*(volatile uint32_t *) (addr)) = (b))



/*******************************************************************************
 * function: danica_ioreuse_inital
 *
 * description:
 *   initial sparrow_pinmux
 *******************************************************************************/
csi_error_t csi_pin_set_mux(pin_name_t pin_name, pin_func_t pin_func)
{
    csi_error_t ret = CSI_OK;
    uint64_t regs = 0U;
    uint32_t reg_val = 0U;
    volatile uint32_t val = 0U;
    uint32_t AF_H_L = 0U;

    if (pin_func == PIN_FUNC_GPIO) {
        val = readl(SPARROW_GPIO_SEL_REG);
        val &= ~((uint32_t)0x1 << (uint32_t)pin_name);
        writel(val, SPARROW_GPIO_SEL_REG);
        ret = CSI_OK;
    } else {
        if (pin_name < PA16) {
            regs = SPARROW_AFL_REG;
            AF_H_L = 0U;
        } else {
            regs = SPARROW_AFH_REG;
            AF_H_L = 1U;
            pin_name = pin_name - (pin_name_t)16U;
        }

        val = readl(regs);
        reg_val = (uint32_t)0x3 << ((uint32_t)pin_name * 2U);
        val &= ~(reg_val);
        val |= ((uint32_t)pin_func << ((uint32_t)pin_name * 2));
        writel(val, regs);
        val = readl(SPARROW_GPIO_SEL_REG);
        val |= ((uint32_t)0x1 << ((uint32_t)pin_name + 16U * AF_H_L));
        writel(val, SPARROW_GPIO_SEL_REG);
        ret = CSI_OK;
    }

    return ret;
}

csi_error_t csi_pin_mode(pin_name_t pin_name, csi_gpio_mode_t mode)
{
    csi_error_t ret = CSI_OK;
    uint32_t val = 0U;

    switch (mode) {
        case GPIO_MODE_PULLNONE:
        case GPIO_MODE_OPEN_DRAIN:
        case GPIO_MODE_PUSH_PULL:
            ret = CSI_OK;
            break;

        case GPIO_MODE_PULLUP:
            val = readl(IO_PAD_00_CFG_CFG + (uint32_t)((pin_name/2) * 4));
            if(pin_name % 2) {
                val &= 0xffcfffff;
                val |= IO_PULL_UP_VALUE << 16;
            } else {
                val &= 0xffffffcf;
                val |= IO_PULL_UP_VALUE;
            }
            writel(val, IO_PAD_00_CFG_CFG + (uint32_t)((pin_name/2) * 4));
            ret = CSI_OK;
            break;

        case GPIO_MODE_PULLDOWN:
            val = readl(IO_PAD_00_CFG_CFG + (uint32_t)((pin_name/2) * 4));
            if(pin_name % 2) {
                val &= 0xffcfffff;
                val |= IO_PULL_DOWN_VALUE << 16;
            } else {
                val &= 0xffffffcf;
                val |= IO_PULL_DOWN_VALUE;
            }
            writel(val, IO_PAD_00_CFG_CFG + (uint32_t)((pin_name/2) * 4));
            ret = CSI_OK;
            break;

        default:
            ret = CSI_ERROR;
            break;
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
    uint32_t ret = 0;
    uint32_t val = 0U;
    uint64_t regs = 0U;

    val = readl(SPARROW_GPIO_SEL_REG);
    val &= ((uint32_t)0x1 << (uint32_t)pin_name);

    if (val != ((uint32_t)0x1 << (uint32_t)pin_name)) {
        ret = (uint32_t)PIN_FUNC_GPIO;
    } else {
        if (pin_name < PA16) {
            regs = SPARROW_AFL_REG;
        } else {
            regs = SPARROW_AFH_REG;
            pin_name = pin_name - (pin_name_t)16U;
        }

        val = readl(regs);
        val &= ((uint32_t)0x3 << ((uint32_t)pin_name * 2U));
        ret = (val >> ((uint32_t)pin_name * 2U));
    }

    return (pin_name_t)ret;
}

/**
  \brief       set pin speed
  \param[in]   pin_name pin name, defined in soc.h.
  \param[in]   speed    io speed
  \return      error code
*/
csi_error_t csi_pin_speed(pin_name_t pin_name, csi_pin_speed_t speed)
{
    csi_error_t ret = CSI_OK;
    return ret;
}

csi_error_t csi_pin_wakeup(pin_name_t pin_name, bool enable)
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
    csi_error_t ret = CSI_OK;

    switch (drive) {
        case PIN_DRIVE_LV0:
            val = readl(IO_PAD_00_CFG_CFG + (uint32_t)((pin_name/2) * 4));
            if (pin_name % 2) {
                val &= 0xfff0ffff;
                val |= IO_DRIVE_LV0_VALUE << 16;
            } else {
                val &= 0xfffffff0;
                val |= IO_DRIVE_LV0_VALUE;
            }
            writel(val, IO_PAD_00_CFG_CFG + (uint32_t)((pin_name/2) * 4));
            break;

        case PIN_DRIVE_LV1:
            val = readl(IO_PAD_00_CFG_CFG + (uint32_t)((pin_name/2) * 4));
            if (pin_name % 2) {
                val &= 0xfff0ffff;
                val |= IO_DRIVE_LV1_VALUE << 16;
            } else {
                val &= 0xfffffff0;
                val |= IO_DRIVE_LV1_VALUE;
            }
            writel(val, IO_PAD_00_CFG_CFG + (uint32_t)((pin_name/2) * 4));
            break;

        case PIN_DRIVE_LV2:
            val = readl(IO_PAD_00_CFG_CFG + (uint32_t)((pin_name/2) * 4));
            if (pin_name % 2) {
                val &= 0xfff0ffff;
                val |= IO_DRIVE_LV2_VALUE << 16;
            } else {
                val &= 0xfffffff0;
                val |= IO_DRIVE_LV3_VALUE;
            }
            writel(val, IO_PAD_00_CFG_CFG + (uint32_t)((pin_name/2) * 4));
            break;

        case PIN_DRIVE_LV3:
            val = readl(IO_PAD_00_CFG_CFG + (uint32_t)((pin_name/2) * 4));
            if (pin_name % 2) {
                val &= 0xfff0ffff;
                val |= IO_DRIVE_LV3_VALUE << 16;
            } else {
                val &= 0xfffffff0;
                val |= IO_DRIVE_LV3_VALUE;
            }
            writel(val, IO_PAD_00_CFG_CFG + (uint32_t)((pin_name/2) * 4));
            break;

        default:
            break;
    }

    return ret;
}

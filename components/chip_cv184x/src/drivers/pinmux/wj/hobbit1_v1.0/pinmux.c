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

#define GPIO_SET_BIT16 0x00010000U
#define GPIO_SET_BIT17 0x00020000U

/******************************************************************************
 * CH2201 gpio control and gpio reuse function
 * selecting regester adddress
 ******************************************************************************/

#define CH2201_GIPO0_PORTCTL_REG 0x50006008U
#define CH2201_GIPO1_PORTCTL_REG 0x50009008U
#define CH2201_IOMUX0L_REG       0x50006100U
#define CH2201_IOMUX0H_REG       0x50006104U
#define CH2201_IOMUX1L_REG       0x50006108U
#define CH2201_IOPU0_REG         0x50006124U
#define CH2201_IOPU1_REG         0x50006128U
#define CH2201_IOPD0_REG         0x5000612CU
#define CH2201_IOPD1_REG         0x50006130U
#define CH2201_IOOD0_REG         0x5000611CU
#define CH2201_IOOD1_REG         0x50006120U
#define CH2201_IOSR0_REG         0x50006134U
#define CH2201_IOSR1_REG         0x50006138U
#define CH2201_IODR0_REG         0x5000613CU
#define CH2201_IODR1_REG         0x50006140U


#define readl(addr) \
    ({ unsigned int __v = (*(volatile unsigned int *) (addr)); __v; })

#define writel(b,addr) ((*(volatile unsigned int *) (addr)) = (b))


/**
  \brief       config the pin function.
  \param[in]   pin       refs to pin_idx_t.
  \param[in]   pin_func  refs to pin_func_t.
  \return      0-success or -1-failure
*/
csi_error_t csi_pin_set_mux(pin_name_t pin, pin_func_t pin_func)
{
    uint32_t val = 0U;
    uint32_t reg_val = 0U;
    uint8_t offset;
    csi_error_t ret = CSI_OK;

    if (pin_func == 10) {
        /* PA10 PA12 PA14 SYS_WKUP */
        if ((pin == PA10) || (pin == PA12)) {
            csi_vic_set_wakeup_irq(14);
            csi_vic_set_wakeup_irq(31);
        } else if (pin == PA14) {
            csi_vic_set_wakeup_irq(14);
            csi_vic_set_wakeup_irq(30);
        }
        ret = CSI_OK;
    }else if(pin_func > 3) {
        /* GPIO FUNC */
        if (pin <= PB3) {
            if (pin <= PA5) {
                offset = (uint8_t)pin;
                /* gpio data source select */
                val = readl(CH2201_GIPO0_PORTCTL_REG);
                val &= ~((uint32_t)1U << (uint32_t)offset);
                writel(val, CH2201_GIPO0_PORTCTL_REG);
            } else if (pin >= PB0) {
                offset = (uint8_t)pin - 6U;
                /* gpio data source select */
                val = readl(CH2201_GIPO1_PORTCTL_REG);
                val &= ~((uint32_t)1U << (uint32_t)offset);
                writel(val, CH2201_GIPO1_PORTCTL_REG);
            }
        }

        if ((pin >= PA6) && (pin <= PA27)) {
            offset = (uint8_t)pin - 4U;
            /* gpio data source select */
            val = readl(CH2201_GIPO0_PORTCTL_REG);
            val &= ~((uint32_t)1U << (uint32_t)offset);
            writel(val, CH2201_GIPO0_PORTCTL_REG);
        }
        ret = CSI_OK;
    }else{
        if ((pin >= PA6) && (pin <= PA27)) {
            offset = (uint8_t)pin - 4U;

            /* gpio data source select */
            val = readl(CH2201_GIPO0_PORTCTL_REG);
            val |= ((uint32_t)1U << (uint32_t)offset);
            writel(val, CH2201_GIPO0_PORTCTL_REG);

            if (pin <= PA11) {
                offset = (uint8_t)pin;
                reg_val = (uint32_t)0x3U << (offset * 2U);
                /* reuse function select */
                val = readl(CH2201_IOMUX0L_REG);
                val &= ~(reg_val);
                val |= (uint32_t)pin_func << (2U * offset);
                writel(val, CH2201_IOMUX0L_REG);
            } else {
                offset = (uint8_t)pin - 16U;
                reg_val = (uint32_t)0x3U << (offset * 2U);
                /* reuse function select */
                val = readl(CH2201_IOMUX0H_REG);
                val &= ~(reg_val);
                val |= (uint32_t)pin_func << (2U * offset);
                writel(val, CH2201_IOMUX0H_REG);
            }
        }

        if ((pin >= PA0) && (pin <= PB3)) {
            if (pin >= PB0) {
                offset = (uint8_t)pin - 6U;
                val = readl(CH2201_GIPO1_PORTCTL_REG);
                val |= (uint32_t)1U << offset;
                writel(val, CH2201_GIPO1_PORTCTL_REG);

                offset = (uint8_t)pin;
                reg_val = (uint32_t)0x3U << (offset * 2U);
                /* reuse function select */
                val = readl(CH2201_IOMUX0L_REG);
                val &= ~(reg_val);
                val |= (uint32_t)pin_func << (2U * offset);
                writel(val, CH2201_IOMUX0L_REG);
            }

            if (pin <= PA5) {
                offset = (uint8_t)pin;
                /* gpio data source select */
                val = readl(CH2201_GIPO0_PORTCTL_REG);
                val |= ((uint32_t)1 << (uint32_t)offset);
                writel(val, CH2201_GIPO0_PORTCTL_REG);

                reg_val = ((uint32_t)0x3U << (offset * 2U));
                /* reuse function select */
                val = readl(CH2201_IOMUX0L_REG);
                val &= ~(reg_val);
                val |= ((uint32_t)pin_func << (2U * offset));
                writel(val, CH2201_IOMUX0L_REG);
            }
        }

        if (pin > PA27) {
            offset = (uint8_t)pin - (uint8_t)PC0;
            reg_val = ((uint32_t)0x3U << (offset * 2U));
            val = readl(CH2201_IOMUX1L_REG);
            val &= ~(reg_val);
            val |= ((uint32_t)pin_func << (2U * offset));
            writel(val, CH2201_IOMUX1L_REG);

        }
         ret = CSI_OK;
    }

    return ret;
}



/**
  \brief       get the pin function.
  \param[in]   pin_name       refs to pin_name_e.
  \return      pin function count
*/

pin_func_t csi_pin_get_mux(pin_name_t   pin_name)
{
    uint32_t val = 0U;
    uint32_t reg_val = 0U;
    uint8_t offset;
    uint32_t  res = 0U;

    if (pin_name <= PB3) {
        if (pin_name <= PA5) {
            offset = (uint8_t)pin_name;
            /* gpio data source select */
            val = readl(CH2201_GIPO0_PORTCTL_REG);
            val &= ((uint32_t)1U << (uint32_t)offset);

            if (val != ((uint32_t)1U << (uint32_t)offset)) {
                res = (uint32_t)PIN_FUNC_GPIO;
            }
        } else if (pin_name >= PB0) {
            offset = (uint8_t)pin_name - 6U;
            /* gpio data source select */
            val = readl(CH2201_GIPO1_PORTCTL_REG);
            val &= ((uint32_t)1U << (uint32_t)offset);

            if (val != ((uint32_t)1U << (uint32_t)offset)) {
                res = (uint32_t)PIN_FUNC_GPIO;
            }
        }
    }

    if ((pin_name >= PA6) && (pin_name <= PA27)) {
        offset = (uint8_t)pin_name - 4U;
        /* gpio data source select */
        val = readl(CH2201_GIPO0_PORTCTL_REG);
        val &= ((uint32_t)1U << (uint32_t)offset);

        if (val != ((uint32_t)1U << (uint32_t)offset)) {
            res = (uint32_t)PIN_FUNC_GPIO;
        }
    }

    if(res != (uint32_t)PIN_FUNC_GPIO)
    {
        if ((pin_name >= PA6) && (pin_name <= PA27)) {
            offset = (uint8_t)pin_name - 4U;

            if (pin_name <= PA11) {
                offset = (uint8_t)pin_name;
                reg_val = ((uint32_t)0x3U << (offset * 2U));
                /* reuse function select */
                val = readl(CH2201_IOMUX0L_REG);
                val &= (reg_val);
                res = (val >> (2U * offset));
            } else {
                offset = (uint8_t)pin_name - 16U;
                reg_val = ((uint32_t)0x3U << (offset * 2U));
                /* reuse function select */
                val = readl(CH2201_IOMUX0H_REG);
                val &= (reg_val);
                res = val >> (2U * offset);
            }
        }

        if ((pin_name >= PA0) && (pin_name <= PB3)) {
            if (pin_name >= PB0) {
                offset = (uint8_t)pin_name;
                reg_val = ((uint32_t)0x3U << (offset * 2U));
                /* reuse function select */
                val = readl(CH2201_IOMUX0L_REG);
                val &= (reg_val);
                res = val >> (2U * offset);
            }

            if (pin_name <= PA5) {
                offset = (uint8_t)pin_name;

                reg_val = ((uint32_t)0x3U << (offset * 2U));
                /* reuse function select */
                val = readl(CH2201_IOMUX0L_REG);
                val &= (reg_val);
                res = val >> (2U * offset);
            }
        }

        if (pin_name > PA27) {
            offset = (uint8_t)pin_name - (uint8_t)PC0;
            reg_val = ((uint32_t)0x3U << (offset * 2U));
            val = readl(CH2201_IOMUX1L_REG);
            val &= (reg_val);
            res = val >> (2U * offset);
        }
    }
    return (pin_func_t)res;
}

csi_error_t csi_pin_wakeup(pin_name_t pin_name, bool enable)
{
    return CSI_OK;
}

/**
  \brief       set pin mode
  \param[in]   pin_name pin name, defined in soc.h.
  \param[in]   mode     push/pull mode
  \return      error code
*/
csi_error_t csi_pin_mode(pin_name_t pin_name, csi_gpio_mode_t mode)
{
    unsigned int val = 0U;
    uint32_t reg = 0U;
    csi_error_t ret = CSI_OK;

    switch (mode) {
        case GPIO_MODE_PULLUP:
            if (pin_name >= PC0) {
                reg = CH2201_IOPU1_REG;
                val = readl(CH2201_IOPD1_REG);

                /* disable Portc corresponding pin pulldown status */
                if (val & ((uint32_t)1U << ((uint32_t)pin_name - (uint32_t)PC0))) {
                    val &= ~((uint32_t)1U << ((uint32_t)pin_name - (uint32_t)PC0));
                    writel(val, CH2201_IOPD1_REG);
                }
            } else {
                /* disable PortA&PortB corresponding pin pulldown status */
                reg = CH2201_IOPU0_REG;
                val = readl(CH2201_IOPD0_REG);

                if (val & ((uint32_t)1U << (uint32_t)pin_name)) {
                    val &= ~((uint32_t)1U << (uint32_t)pin_name);
                    writel(val, CH2201_IOPD0_REG);
                }
            }
            ret = CSI_OK;
            break;

        case GPIO_MODE_PULLDOWN:
            if (pin_name >= PC0) {
                reg = CH2201_IOPD1_REG;
                val = readl(CH2201_IOPU1_REG);

                /* disable Portc corresponding pin pullup status */
                if (val & ((uint32_t)1U << ((uint32_t)pin_name - (uint32_t)PC0))) {
                    val &= ~((uint32_t)1U << ((uint32_t)pin_name - (uint32_t)PC0));
                    writel(val, CH2201_IOPU1_REG);
                }
            } else {
                reg = CH2201_IOPD0_REG;
                val = readl(CH2201_IOPU0_REG);

                /* disable PortA&PortB corresponding pin pullup status */
                if (val & ((uint32_t)1U << (uint32_t)pin_name)) {
                    val &= ~((uint32_t)1U << (uint32_t)pin_name);
                    writel(val, CH2201_IOPU0_REG);
                }
            }
            ret = CSI_OK;
            break;

        case GPIO_MODE_OPEN_DRAIN:
            if (pin_name >= PC0) {
                reg = CH2201_IOOD1_REG;
            } else {
                reg = CH2201_IOOD0_REG;
            }
            ret = CSI_OK;
            break;

        case GPIO_MODE_PUSH_PULL:
             ret = CSI_UNSUPPORTED;
             break;
        case GPIO_MODE_PULLNONE:
            if (pin_name >= PC0) {
                val = readl(CH2201_IOPD1_REG);

                /* disable Portc corresponding pin pulldown status */
                if (val & ((uint32_t)1U << ((uint32_t)pin_name - (uint32_t)PC0))) {
                    val &= ~((uint32_t)1U << ((uint32_t)pin_name - (uint32_t)PC0));
                    writel(val, CH2201_IOPD1_REG);
                }

                val = readl(CH2201_IOPU1_REG);

                /* disable Portc corresponding pin pullup status */
                if (val & ((uint32_t)1U << ((uint32_t)pin_name - (uint32_t)PC0))) {
                    val &= ~((uint32_t)1U << ((uint32_t)pin_name - (uint32_t)PC0));
                    writel(val, CH2201_IOPU1_REG);
                }

            } else {
                /* disable PortA&PortB corresponding pin pulldown status */
                val = readl(CH2201_IOPD0_REG);

                if (val & ((uint32_t)1U << (uint32_t)pin_name)) {
                    val &= ~((uint32_t)1U << (uint32_t)pin_name);
                    writel(val, CH2201_IOPD0_REG);
                }

                val = readl(CH2201_IOPU0_REG);

                /* disable PortA&PortB corresponding pin pullup status */
                if (val & ((uint32_t)1U << (uint32_t)pin_name)) {
                    val &= ~((uint32_t)1U << (uint32_t)pin_name);
                    writel(val, CH2201_IOPU0_REG);
                }
            }

             ret = CSI_OK;
             break;

        default:
            ret = CSI_UNSUPPORTED;
            break;
    }
    if((ret == CSI_OK) && (mode != GPIO_MODE_PULLNONE ))
    {
        val = readl(reg);

        if (pin_name >= PC0) {
            val |= ((uint32_t)1U << ((uint32_t)pin_name - (uint32_t)PC0));
        } else {
            val |= ((uint32_t)1U << (uint32_t)pin_name);
        }

        writel(val, reg);
    }
    return ret;
}

/**
  \brief       set pin speed
  \param[in]   pin_name pin name, defined in soc.h.
  \param[in]   speed    io speed
  \return      error code
*/
csi_error_t csi_pin_speed(pin_name_t pin_name, csi_pin_speed_t speed)
{
    uint32_t val = 0U;
    csi_error_t ret = CSI_OK;

    if (speed > PIN_SPEED_LV1) {
        ret = CSI_UNSUPPORTED;
    }else{
        switch (speed) {
            case PIN_SPEED_LV0:
                if (pin_name >= PC0) {
                    val = readl(CH2201_IOSR1_REG);

                    if (val & ((uint32_t)1U << ((uint32_t)pin_name - (uint32_t)PC0))) {
                        val &= ~((uint32_t)1U << ((uint32_t)pin_name - (uint32_t)PC0));
                        writel(val, CH2201_IOSR1_REG);
                    }
                } else {
                    val = readl(CH2201_IOSR0_REG);

                    if (val & ((uint32_t)1U << (uint32_t)pin_name)) {
                        val &= ~((uint32_t)1U << (uint32_t)pin_name);
                        writel(val, CH2201_IOSR0_REG);
                    }
                }
                ret = CSI_OK;
                break;

            case PIN_SPEED_LV1:
                if (pin_name >= PC0) {
                    val = readl(CH2201_IOSR1_REG);
                    val |= (uint32_t)1U << ((uint32_t)pin_name - (uint32_t)PC0);
                    writel(val, CH2201_IOSR1_REG);
                } else {
                    val = readl(CH2201_IOSR0_REG);
                    val |= (uint32_t)1U << (uint32_t)pin_name;
                    writel(val, CH2201_IOSR0_REG);
                }
                ret = CSI_OK;
                break;

            default:
                ret = CSI_ERROR;
                break;
        }
    }

    return ret;
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

    if (drive > PIN_DRIVE_LV1) {
        ret = CSI_UNSUPPORTED;
    }else{
        switch (drive) {
            case PIN_DRIVE_LV0:
                if (pin_name >= PC0) {
                    val = readl(CH2201_IODR1_REG);

                    if (val & ((uint32_t)1U << ((uint32_t)pin_name - (uint32_t)PC0))) {
                        val &= ~(uint32_t)1U << ((uint32_t)pin_name - (uint32_t)PC0);
                        writel(val, CH2201_IODR1_REG);
                    }
                } else {
                    val = readl(CH2201_IODR0_REG);

                    if (val & ((uint32_t)1U << (uint32_t)pin_name)) {
                        val &= ~(uint32_t)1U << (uint32_t)pin_name;
                        writel(val, CH2201_IODR0_REG);
                    }
                }
                ret = CSI_OK;
                break;

            case PIN_DRIVE_LV1:
                if (pin_name >= PC0) {
                    val = readl(CH2201_IODR1_REG);
                    val |= (uint32_t)1U << ((uint32_t)pin_name - (uint32_t)PC0);
                    writel(val, CH2201_IODR1_REG);
                } else {
                    val = readl(CH2201_IODR0_REG);
                    val |= (uint32_t)1U << (uint32_t)pin_name;
                    writel(val, CH2201_IODR0_REG);
                }
                ret = CSI_OK;
                break;

            default:
                ret = CSI_ERROR;
                break;
        }
    }
    return   ret;
}

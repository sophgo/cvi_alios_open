/*
* Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
*/

#include <drv/pin.h>
#include "cvi_pin.h"
#include "pin.h"

#define CHECK_PIN_NAME_VALID(PIN_NAME)                        \
    do                                                  \
    {                                                   \
        if (PIN_NAME >= PIN_NUM) \
        {                                               \
            pr_err("pin name %d is invalid\n", PIN_NAME);	\
            return (CVI_ERROR);                               \
        }                                               \
    } while (0)

pin_name_t cvi_gpio_to_pin(uint8_t gpio_bank, uint8_t pin_num, const cvi_pinmap_t *pinmap)
{
    const cvi_pinmap_t *map = pinmap;
    pin_name_t ret = (pin_name_t)0xFFFFFFFFU;

    while ((uint32_t)map->pin_name != 0xFFFFFFFFU) {
        if ((map->idx == gpio_bank) && (map->channel == pin_num)) {
            ret = map->pin_name;
            break;
        }
        map++;
    }
    return ret;
}

uint32_t cvi_pin_to_channel(pin_name_t pin_name, const cvi_pinmap_t *pinmap)
{
    const cvi_pinmap_t *map = pinmap;
    uint32_t ret = 0xFFFFFFFFU;

    while ((uint32_t)map->pin_name != 0xFFFFFFFFU) {
        if (map->pin_name == pin_name) {
            ret = (uint32_t)map->channel;
            break;
        }
        map++;
    }
    return ret;
}

static uint16_t pin_to_mux_offset(pin_name_t pin_name)
{
    const pin_reg_offset_t *map = cvi_pin_reg_offset;
    pin_name_t ret = (pin_name_t)0xFFFF;

    while ((uint32_t)map->pin_name != 0xFFFFFFFFU) {
        if (map->pin_name == pin_name) {
            ret = map->mux_reg_offset;
            break;
        }
        map++;
    }

    return ret;
}

static uint16_t pin_to_cfg_offset(pin_name_t pin_name)
{
    const pin_reg_offset_t *map = cvi_pin_reg_offset;
    pin_name_t ret = (pin_name_t)0xFFFF;

    while ((uint32_t)map->pin_name != 0xFFFFFFFFU) {
        if (map->pin_name == pin_name) {
            ret = map->cfg_reg_offset;
            break;
        }
        map++;
    }

    return ret;
}

cvi_error_t cvi_pin_set_mux(pin_name_t pin_name, pin_func_t pin_func)
{
#if 1
	/* please usb marco PINMUX_COMFIG replace this function */
	return CVI_UNSUPPORTED;
#else
    uint16_t offset;
    cvi_error_t ret = CSI_OK;
    unsigned long mux_reg_base = PIN_MUX_BASE;

	CHECK_PIN_NAME_VALID(pin_name);

    offset = pin_to_mux_offset(pin_name);
    if (offset == 0xFFFF)
    {
        pr_err("pin name %d is unsupported to mux\n", pin_name);
        ret = CVI_UNSUPPORTED;
        goto done;
    }

    pin_set_mux(mux_reg_base + offset, pin_func);

done:
    return ret;
#endif
}

pin_func_t cvi_pin_get_mux(pin_name_t pin_name)
{
    uint16_t offset;
    cvi_error_t ret = PIN_FUNC_INVALID;
    unsigned long mux_reg_base = PIN_MUX_BASE;

	CHECK_PIN_NAME_VALID(pin_name);

    offset = pin_to_mux_offset(pin_name);
    if (offset == 0xFFFF)
    {
        pr_err("pin name %d is unsupported to mux\n", pin_name);
        ret = CVI_UNSUPPORTED;
        goto done;
    }

    ret = pin_get_mux(mux_reg_base + offset);

done:
    return ret;
}


cvi_error_t cvi_pin_set_mode(pin_name_t pin_name, cvi_pin_mode_t mode)
{
    uint16_t offset, value;
    csi_error_t ret = CSI_OK;
    unsigned long cfg_reg_base = PIN_CFG_BASE;

	CHECK_PIN_NAME_VALID(pin_name);

    offset = pin_to_cfg_offset(pin_name);
    if (offset == 0xFFFF)
    {
        pr_err("pin name %d is unsupported to set mode\n", pin_name);
        ret = CVI_UNSUPPORTED;
        goto done;
    }

    if (pin_name >= PWR_VBAT_DET)
        cfg_reg_base = PWR_PIN_CFG_BASE;

    switch (mode)
    {
        case PIN_MODE_PULL_NONE:
            value = PIN_MODE_NONE;
            break;
        case PIN_MODE_PULL_UP:
            value = PIN_MODE_PULLUP;
            break;
        case PIN_MODE_PULL_DOWN:
            value = PIN_MODE_PULLDOWN;
            break;
        default:
            pr_err("pin name %d is unsupported to set mode %d\n", pin_name, mode);
            ret = CVI_UNSUPPORTED;
            goto done;
    }

    pin_set_mode(cfg_reg_base + offset, value);

done:
    return ret;
}

cvi_error_t cvi_pin_set_speed(pin_name_t pin_name, cvi_pin_speed_t speed)
{
    uint16_t offset, value;
    cvi_error_t ret = CSI_OK;
    unsigned long cfg_reg_base = PIN_CFG_BASE;

	CHECK_PIN_NAME_VALID(pin_name);

    offset = pin_to_cfg_offset(pin_name);
    if (offset == 0xFFFF)
    {
        pr_err("pin name %d is unsupported to set speed\n", pin_name);
        ret = CSI_UNSUPPORTED;
        goto done;
    }

	if (pin_name >= PWR_VBAT_DET)
        cfg_reg_base = PWR_PIN_CFG_BASE;

    switch (speed)
    {
        case CVI_PIN_SPEED_SLOW:
            value = PIN_SPEED_SLOW;
            break;
        case CVI_PIN_SPEED_FAST:
            value = PIN_SPEED_FAST;
            break;
        default:
            pr_err("pin name %d is unsupported to set speed %d\n", pin_name, speed);
            ret = CVI_UNSUPPORTED;
            goto done;
    }

    pin_set_speed(cfg_reg_base + offset, value);

done:
    return ret;
}

cvi_error_t cvi_pin_set_drive(pin_name_t pin_name, cvi_pin_drive_t drive)
{
    uint16_t offset, value;
    csi_error_t ret = CSI_OK;
    unsigned long cfg_reg_base = PIN_CFG_BASE;

	CHECK_PIN_NAME_VALID(pin_name);

    offset = pin_to_cfg_offset(pin_name);
    if (offset == 0xFFFF)
    {
        pr_err("pin name %d is unsupported to set drive\n", pin_name);
        ret = CSI_UNSUPPORTED;
        goto done;
    }

	if (pin_name >= PWR_VBAT_DET)
        cfg_reg_base = PWR_PIN_CFG_BASE;

    switch (drive)
    {
        case PIN_DRIVE_STRENGTH0:
            value = PIN_DRIVE_STRENGTH_0;
            break;
        case PIN_DRIVE_STRENGTH1:
            value = PIN_DRIVE_STRENGTH_1;
            break;
        default:
            pr_err("pin name %d is unsupported to set drive %d\n", pin_name, drive);
            ret = CVI_UNSUPPORTED;
            goto done;
    }

    pin_set_drive(cfg_reg_base + offset, value);

done:
    return ret;
}


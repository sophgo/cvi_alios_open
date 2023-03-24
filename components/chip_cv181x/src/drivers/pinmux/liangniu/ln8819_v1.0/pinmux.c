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
#include "reg_iomux.h"


typedef union
{
    uint32_t val;
    struct
    {
        uint32_t    func_sel_0          : 7  ; // 6  : 0
        uint32_t    reserved_3          : 1  ; // 7  : 7
        uint32_t    func_sel_1          : 7  ; // 14 : 8
        uint32_t    reserved_2          : 1  ; // 15 : 15
        uint32_t    func_sel_2          : 7  ; // 22 : 16
        uint32_t    reserved_1          : 1  ; // 23 : 23
        uint32_t    func_sel_3          : 7  ; // 30 : 24
        uint32_t    reserved_0          : 1  ; // 31 : 31
    } bit_field;
} t_iomux_func_isel;


typedef struct
{
    volatile        uint32_t func_isel[8];  // 0x0 ~ 0x1c
    volatile        t_iomux_io_en                            io_en                                  ; // 0x20
    volatile        t_iomux_dsel_en                          dsel_en                                ; // 0x24
    volatile        t_iomux_pull_up0                         pull_up0                               ; // 0x28
    volatile        t_iomux_pull_up1                         pull_up1                               ; // 0x2c
    volatile        t_iomux_pull_down0                       pull_down0                             ; // 0x30
    volatile        t_iomux_pull_down1                       pull_down1                             ; // 0x34
    volatile        t_iomux_pad_hp0                          pad_hp0                                ; // 0x38
    volatile        t_iomux_pad_hp1                          pad_hp1                                ; // 0x3c
} ln_pinmux_regs_t;


csi_error_t csi_pin_set_mux(pin_name_t pin_name, pin_func_t pin_func)
{
    if (pin_func == PIN_FUNC_GPIO) {
        ;//TODO
    }

    if (pin_name >= PA13 && pin_name <= PB5) {
        pin_name -= 1;
    }

    if (pin_name >= PB10 && pin_name <= PC4) {
        pin_name -= 5;
    }

    ln_pinmux_regs_t *regbase = (ln_pinmux_regs_t *)WJ_IOC_BASE;
    int32_t idx = pin_name / 4;
    uint32_t regval = regbase->func_isel[idx];
    uint8_t offset = (pin_name % 4) * 8;
    regval &= ~(0xff << offset);
    regval |= pin_func << offset;
    regbase->func_isel[idx] = regval;

    return CSI_OK;
}

pin_func_t csi_pin_get_mux(pin_name_t pin_name)
{
    ln_pinmux_regs_t *regbase = (ln_pinmux_regs_t *)WJ_IOC_BASE;
    int32_t idx = pin_name / 4;
    uint32_t regval = regbase->func_isel[idx];
    uint8_t offset = (pin_name % 4) * 8;

    regval = (regval >> offset) & 0xff;
    if (regval == 0xff) {
        return PIN_FUNC_GPIO;
    }

    return regval;
}

csi_error_t csi_pin_mode(pin_name_t pin_name, csi_pin_mode_t mode)
{
    /*ToDo*/
    return CSI_OK;
}

csi_error_t csi_pin_speed(pin_name_t pin_name, csi_pin_speed_t speed)
{
    /*ToDo*/
    return CSI_OK;
}

csi_error_t csi_pin_drive(pin_name_t pin_name, csi_pin_drive_t drive)
{
    /*ToDo*/
    return CSI_OK;
}

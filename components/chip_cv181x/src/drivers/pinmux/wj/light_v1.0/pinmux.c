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

#define readl(addr) \
    ({ unsigned int __v = (*(volatile uint32_t *) (addr)); __v; })

#define writel(b,addr) ((*(volatile uint32_t *) (addr)) = (b))

/*******************************************************************************
 * function: danica_ioreuse_inital
 *
 * description:
 *   initial danica_pinmux
 *******************************************************************************/
ATTRIBUTE_DATA csi_error_t csi_pin_set_mux(pin_name_t pin_name, pin_func_t pin_func)
{
    uint64_t reg_base;		
    uint32_t shift;		
    uint32_t mask,val,temp;		
    if(pin_name >= PA0 && pin_name < PB0)		
    {		
      reg_base = WJ_IOC_BASE + 0x404;		
      shift    = (pin_name % 8 );		
      		
      mask      = 0xF << (shift<<2);		
      val       = pin_func << (shift<<2);		
      reg_base += (pin_name>>3) <<2;		
      		
      temp = readl(reg_base);		
      temp &= ~mask;		
      temp |= val;		
      writel(temp,reg_base);		
    }		
    else		
    {		
      return CSI_UNSUPPORTED;		
    }		
    //to be continuce 
    return CSI_OK;
}


csi_error_t csi_pin_mode(pin_name_t pin_name, csi_gpio_mode_t mode)
{


    return CSI_OK;
}

csi_error_t csi_pin_wakeup(pin_name_t pin_name, bool enable)
{
    return CSI_OK;
}

/**
  \brief       get the pin function.
  \param[in]   pin       refs to pin_name_e.
  \return      pin function count
*/
pin_func_t csi_pin_get_mux(pin_name_t pin_name)
{
    uint32_t ret = 0;
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


/**
  \brief       set pin drive
  \param[in]   pin_name pin name, defined in soc.h.
  \param[in]   drive    io drive
  \return      error code
*/
csi_error_t csi_pin_drive(pin_name_t pin_name, csi_pin_drive_t drive)
{
    csi_error_t ret = CSI_OK;
    return ret;
}

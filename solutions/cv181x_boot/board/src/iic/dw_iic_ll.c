/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */
#include <unistd.h>
#include "dw_iic_ll.h"
// #include <aos/cli.h>
// #include <posix/timer.h>

uint32_t dw_iic_wait_for_bb(dw_iic_regs_t *iic_base)
{
	uint16_t    timeout = 0;
	while ((iic_base->IC_STATUS & DW_IIC_MST_ACTIVITY_STATE) ||
	       !(iic_base->IC_STATUS & DW_IIC_TXFIFO_EMPTY_STATE)) {

		/* Evaluate timeout */
		udelay(5);
		timeout++;
		if (timeout > 200) {/* exceed 1 ms */
            log("Timed out waiting for bus busy\n");
			return 1;
		}
	}

	return 0;
}

void dw_iic_set_reg_address(dw_iic_regs_t *iic_base, uint32_t addr, uint8_t addr_len)
{
	while (addr_len) {
		addr_len--;
		/* high byte address going out first */
		dw_iic_transmit_data(iic_base, (addr >> (addr_len * 8)) & 0xff);
	}
}

void dw_iic_set_target_address(dw_iic_regs_t *iic_base, uint32_t address)
{
    uint32_t iic_status;
    iic_status = dw_iic_get_iic_status(iic_base);
    dw_iic_disable(iic_base);
    iic_base->IC_TAR = (iic_base->IC_TAR & ~0x3ff) | address; /* this register can be written only when the I2C is disabled*/

    if (iic_status == DW_IIC_EN) {
        dw_iic_enable(iic_base);
    }
}

int32_t dw_iic_xfer_init(dw_iic_regs_t *iic_base, uint32_t dev_addr,
                         uint16_t reg_addr, uint8_t reg_addr_len)
{
    if (dw_iic_wait_for_bb(iic_base)) {
        return CSI_ERROR;
    }

    dw_iic_set_target_address(iic_base, dev_addr);
    dw_iic_enable(iic_base);
    dw_iic_set_reg_address(iic_base, reg_addr, reg_addr_len);

    return CSI_OK;
}

int32_t dw_iic_xfer_finish(dw_iic_regs_t *iic_base)
{
	uint32_t timeout = 0;
	while (1) {
		if (iic_base->IC_RAW_INTR_STAT & DW_IIC_RAW_STOP_DET) {
			iic_base->IC_CLR_STOP_DET;
			break;
		} else {
			timeout++;
			udelay(5);
			if (timeout > 1000000) {
				log("xfer finish tiemout\n");
				break;
			}
		}
	}

	if (dw_iic_wait_for_bb(iic_base)) {
		return CSI_ERROR;
	}

	dw_iic_flush_rxfifo(iic_base);

	return CSI_OK;
}

void dw_iic_set_transfer_speed_high(dw_iic_regs_t *iic_base)
{
    uint32_t speed_config = iic_base->IC_CON;
    speed_config &= ~(DW_IIC_CON_SPEEDL_EN | DW_IIC_CON_SPEEDH_EN);
    speed_config |= DW_IIC_CON_SPEEDL_EN | DW_IIC_CON_SPEEDH_EN;
    iic_base->IC_CON = speed_config;
}

void dw_iic_set_transfer_speed_fast(dw_iic_regs_t *iic_base)
{
    uint32_t speed_config = iic_base->IC_CON;
    speed_config &= ~(DW_IIC_CON_SPEEDL_EN | DW_IIC_CON_SPEEDH_EN);
    speed_config |= DW_IIC_CON_SPEEDH_EN;
    iic_base->IC_CON = speed_config;
}

void dw_iic_set_transfer_speed_standard(dw_iic_regs_t *iic_base)
{
    uint32_t speed_config = iic_base->IC_CON;
    speed_config &= ~(DW_IIC_CON_SPEEDL_EN | DW_IIC_CON_SPEEDH_EN);
    speed_config |= DW_IIC_CON_SPEEDL_EN;
    iic_base->IC_CON = speed_config;
}

void dw_iic_set_slave_mode(dw_iic_regs_t *iic_base)
{
    uint32_t iic_status;
    iic_status = dw_iic_get_iic_status(iic_base);
    dw_iic_disable(iic_base);
    uint32_t val = DW_IIC_CON_MASTER_EN | DW_IIC_CON_SLAVE_EN;
    iic_base->IC_CON &= ~val;  ///< set 0 to disabled master mode; set 0 to enabled slave mode

    if (iic_status == DW_IIC_EN) {
        dw_iic_enable(iic_base);
    }
}

void dw_iic_set_master_mode(dw_iic_regs_t *iic_base)
{
    uint32_t iic_status;
    iic_status = dw_iic_get_iic_status(iic_base);
    dw_iic_disable(iic_base);
    uint32_t val = DW_IIC_CON_MASTER_EN | DW_IIC_CON_SLAVE_EN; ///< set 1 to enabled master mode; set 1 to disabled slave mode
    iic_base->IC_CON |= val;

    if (iic_status == DW_IIC_EN) {
        dw_iic_enable(iic_base);
    }
}

uint32_t dw_iic_find_max_prime_num(uint32_t num)
{
    uint32_t i = 0U;

    for (i = 8U; i > 0U; i --) {
        if (!num) {
            i = 1U;
            break;
        }

        if (!(num % i)) {
            break;
        }

    }

    return i;
}

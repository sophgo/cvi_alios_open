/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     crc.c
 * @brief    CSI Source File for CRC Driver
 * @version  V1.0
 * @date     19. Feb 2020
 ******************************************************************************/
#include <drv/cvi_irq.h>
#include <stdio.h>
#include <string.h>
#include <wj_crc_ll.h>
#include <drv/crc.h>
#include <drv/common.h>
#include	<assert.h>

/**
  \brief       Initialize CRC Interface. 1. Initializes the resources needed for the CRC interface 2.registers event callback function
  \param[in]   crc  crc handle to operate.
  \param[in]   idx  device id
  \return      \ref csi_error_t
*/
csi_error_t drv_crc_init(csi_crc_t *crc, uint32_t idx)
{
    CSI_PARAM_CHK(crc, CSI_ERROR);
    target_get(DEV_WJ_CRC_TAG, idx, &crc->dev);

    return CSI_OK;
}

/**
  \brief       De-initialize CRC Interface. stops operation and releases the software resources used by the interface
  \param[in]   crc  crc handle to operate.
  \return      none
*/
void drv_crc_uninit(csi_crc_t *crc)
{

}
/**
  \brief       config crc mode.
  \param[in]   crc       crc handle to operate.
  \param[in]   config    \ref csi_crc_config_t
  \return      \ref csi_error_t
*/
csi_error_t drv_crc_config(csi_crc_t *crc, csi_crc_config_t *config)
{
    ck_crc_reg_t *addr;
    csi_error_t ret = CSI_OK;
    CSI_PARAM_CHK(crc, CSI_ERROR);

    addr = (ck_crc_reg_t *)HANDLE_REG_BASE(crc);
    crc->config = *config;

    switch (config->standard) {
        case CRC_STANDARD_MODBUS:

            wj_crc_set_crc_sel_poly_sel(addr, WJ_CRC_SEL_POLY_SEL_x8005);
            wj_crc_set_polarity_sel_positive(addr);
            wj_crc_set_init_value(addr, 0xffffU);
            break;

        case CRC_STANDARD_IBM:

            wj_crc_set_crc_sel_poly_sel(addr, WJ_CRC_SEL_POLY_SEL_x8005);
            wj_crc_set_polarity_sel_positive(addr);
            wj_crc_set_init_value(addr, 0U);
            break;

        case CRC_STANDARD_MAXIM:
            wj_crc_set_crc_sel_poly_sel(addr, WJ_CRC_SEL_POLY_SEL_x8005);
            wj_crc_set_polarity_sel_negative(addr);
            wj_crc_set_init_value(addr, 0U);
            break;

        case CRC_STANDARD_USB:
            wj_crc_set_crc_sel_poly_sel(addr, WJ_CRC_SEL_POLY_SEL_x8005);
            wj_crc_set_polarity_sel_negative(addr);
            wj_crc_set_init_value(addr, 0xffffU);
            break;

        case CRC_STANDARD_CCITT:
            wj_crc_set_crc_sel_poly_sel(addr, WJ_CRC_SEL_POLY_SEL_x1021);
            wj_crc_set_polarity_sel_positive(addr);
            wj_crc_set_init_value(addr, 0U);
            break;

        case CRC_STANDARD_CCITT_FALSE:
            ret = CSI_ERROR;
            break;

        case CRC_STANDARD_X25:
            wj_crc_set_crc_sel_poly_sel(addr, WJ_CRC_SEL_POLY_SEL_x1021);
            wj_crc_set_polarity_sel_negative(addr);
            wj_crc_set_init_value(addr, 0xffffU);
            break;

        case CRC_STANDARD_CRC8_31:
            wj_crc_set_crc_sel_poly_sel(addr, WJ_CRC_SEL_POLY_SEL_x31);
            wj_crc_set_polarity_sel_positive(addr);
            break;

        case CRC_STANDARD_CRC8_7:
            wj_crc_set_crc_sel_poly_sel(addr, WJ_CRC_SEL_POLY_SEL_x07);
            wj_crc_set_polarity_sel_positive(addr);

            break;


        case CRC_STANDARD_XMODEM:
            ret = CSI_ERROR;
            break;


        case CRC_STANDARD_DNP:
            ret = CSI_ERROR;
            break;

        default:
            ret = CSI_ERROR;
    }

    return ret;

}

/**
  \brief       accumulate crc. Computes the 32-bit CRC of 32-bit data buffer using combination of the previous CRC value and the new one.
  \param[in]   crc     crc handle to operate.
  \param[in]   data    Pointer to the input data
  \param[in]   size    intput data size.
  \return      \ref calculate value
*/
uint32_t drv_crc_accumulate(csi_crc_t *crc, const void *data, uint32_t size)
{
    ck_crc_reg_t *addr;
    uint32_t ret = 0U;
    CSI_PARAM_CHK(crc, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);

    if (size <= 0U) {
        ret = 0U;
    } else {
        addr = (ck_crc_reg_t *)HANDLE_REG_BASE(crc);

        crc->state.busy = 1U;

        /* put the data int the register */
        uint32_t cur;
        uint8_t *p = (uint8_t *)data;

        for (cur = 0U; cur < (size - 3U); cur += 4U, p += 4U) {
            wj_crc_set_data(addr, (uint32_t)(p[0])
                            | ((uint32_t)(p[1]) << 8U)
                            | ((uint32_t)(p[2]) << 16U)
                            | ((uint32_t)(p[3]) << 24U));
        }

        uint32_t data1 = 0U;
        uint8_t i;

        if (cur < size) {
            for (i = 0U; i < (size - cur); i++) {
                data1 |= ((uint32_t)(p[i]) << (i * 8U));
            }

            wj_crc_set_data(addr, data1);
        }

        crc->state.busy = 0U;
        ret = wj_crc_get_data(addr);
    }

    return ret;

}

/**
  \brief       calculate crc. Computes the 32-bit CRC of 32-bit data buffer independently of the previous CRC value.
  \param[in]   crc     crc handle to operate.
  \param[in]   data    Pointer to the input data
  \param[in]   size    intput data size.
  \return      \ref calculate value
*/
uint32_t drv_crc_calculate(csi_crc_t *crc, const void *data, uint32_t size)
{
    ck_crc_reg_t *addr;
    uint32_t ret = 0U;

    CSI_PARAM_CHK(crc, CSI_ERROR);
    CSI_PARAM_CHK(data, CSI_ERROR);

    if (size <= 0U) {
        ret = 0U;
    } else {

        addr = (ck_crc_reg_t *)HANDLE_REG_BASE(crc);

        crc->state.busy = 1U;
        /* put the data int the register */
        uint32_t cur;
        uint8_t *p = (uint8_t *)data;

        for (cur = 0U; cur < (size - 3U); cur += 4U, p += 4U) {
            wj_crc_set_data(addr, (uint32_t)(p[0])
                            | ((uint32_t)(p[1]) << 8U)
                            | ((uint32_t)(p[2]) << 16U)
                            | ((uint32_t)(p[3]) << 24U));
        }

        uint32_t data1 = 0U;
        uint8_t i;

        if (cur < size) {
            for (i = 0U; i < (size - cur); i++) {
                data1 |= ((uint32_t)(p[i]) << (i * 8U));
            }

            wj_crc_set_data(addr, data1);
        }

        crc->state.busy = 0U;
        //  while((wj_crc_get_init_value(addr)&0xffff)!=0xffff);
        ret = wj_crc_get_data(addr);
    }

    return ret;
}

/**
  \brief       Get CRC state.
  \param[in]   crc  crc handle to operate.
  \return      CRC state \ref  csi_crc_state_t
*/
csi_crc_state_t drv_crc_get_state(csi_crc_t *crc)
{

    return  crc->state;
}

/**
 * \brief Compute the CRC-7 checksum of a buffer.
 *
 * See JESD84-A441.  Used by the MMC protocol.  Uses 0x09 as the
 * polynomial with no reflection.  The CRC is left
 * justified, so bit 7 of the result is bit 6 of the CRC.
 * init = 0; poly = 0x09 refin = 0 refout = 0 xorout = 0
 * \param[in] crc      crc init value or crc immediate result
 * \param[in] data     data buf to be calculate
 * \param[in] size     data size
 *
 * \return    The computed CRC7 value
 */
uint8_t csi_crc7_be(uint8_t crc, uint8_t *data, uint32_t size)
{
    return 0;
}

/**
 * \brief Compute the CRC-8 checksum of a buffer.
 *  init = 0 or 0xff; poly = 0x07 refin = 0 refout = 0 xorout = 0
 * \param[in] crc      crc init value or crc immediate result
 * \param[in] data     data buf to be calculate
 * \param[in] size     data size
 * \return    The computed CRC8 value
 */
uint8_t csi_crc8(uint8_t crc, uint8_t *data, uint32_t size)
{
    csi_crc_state_t status;
    csi_crc_config_t config;
    static csi_crc_t crc1;
    csi_crc_t *pt = &crc1;
    uint8_t out;
    ck_crc_reg_t *addr;

    status.busy = 1U;

    drv_crc_init(&crc1, 0U);

    addr = (ck_crc_reg_t *)HANDLE_REG_BASE(pt);

    config.mode = CRC_MODE_CRC8;
    config.standard = CRC_STANDARD_CRC8_7;
    drv_crc_config(&crc1, &config);
    wj_crc_set_init_value(addr, crc);
    out = (uint16_t)(drv_crc_calculate(&crc1, data, size));

    while (status.busy == 1U) {
        status = drv_crc_get_state(&crc1);
    }

    drv_crc_uninit(&crc1);
    return out;
}

/**
 * \brief Compute the CRC-8 checksum of a buffer.
 *  init = 0; poly = 0x31 refin = 1 refout = 1 xorout = 0
 * \param[in] crc      crc init value or crc immediate result
 * \param[in] data     data buf to be calculate
 * \param[in] size     data size
 * \return    The computed CRC8 value
 */
uint8_t csi_crc8_maxim(uint8_t crc, uint8_t *data, size_t size)
{
    csi_crc_state_t status;
    csi_crc_config_t config;
    static csi_crc_t crc1;
    csi_crc_t *pt = &crc1;
    uint8_t out;
    ck_crc_reg_t *addr;

    status.busy = 1U;

    drv_crc_init(&crc1, 0U);

    addr = (ck_crc_reg_t *)HANDLE_REG_BASE(pt);

    config.mode = CRC_MODE_CRC8;
    config.standard = CRC_STANDARD_CRC8_31;
    drv_crc_config(&crc1, &config);
    wj_crc_set_init_value(addr, crc);
    out = (uint16_t)(drv_crc_calculate(&crc1, data, size));

    while (status.busy == 1U) {
        status = drv_crc_get_state(&crc1);
    }

    drv_crc_uninit(&crc1);
    return out;

}

/**
 * \brief Compute the CRC-16 checksum of a buffer.
 *   init = 0 or 0xffff; poly = 0x8005 refin = 1 refout = 1 xorout = 0
 * \param[in] crc      crc init value or crc immediate result
 * \param[in] data     data buf to be calculate
 * \param[in] size     data size
 * \return    The computed CRC16 without xorout
 */
uint16_t csi_crc16(uint16_t crc, uint8_t *data, uint32_t size)
{
    csi_crc_state_t status;
    csi_crc_t crc1;
    csi_crc_config_t config;
    uint16_t out;
    status.busy = 1U;

    if ((crc == 0U) || (crc == 0xffffU)) {

        drv_crc_init(&crc1, 0U);

        config.mode = CRC_MODE_CRC16;

        if (crc == 0xffffU) {
            config.standard = CRC_STANDARD_MODBUS;
        } else {
            config.standard = CRC_STANDARD_IBM;
        }

        drv_crc_config(&crc1, &config);
        out = (uint16_t)(drv_crc_calculate(&crc1, data, size));

        while (status.busy == 1U) {
            status = drv_crc_get_state(&crc1);
        }

        drv_crc_uninit(&crc1);
    } else {
        out = 0U; /* code */

    }

    return out;
}

/**
 * \brief Compute the CRC-16 checksum of a buffer.
 *  init = 0; poly = 0x1021 refin = 1 refout = 1 xorout = 0
 * \param[in] crc      crc init value or crc immediate result
 * \param[in] data     data buf to be calculate
 * \param[in] size     data size
 * \return    The computed CRC16 without xorout
 */
uint16_t csi_crc16_ccitt(uint16_t crc, uint8_t *data, uint32_t size)
{
    csi_crc_state_t status;
    csi_crc_config_t config;
    static csi_crc_t crc1;
    uint16_t out;


    status.busy = 1U;

    drv_crc_init(&crc1, 0U);
    config.mode = CRC_MODE_CRC16;
    config.standard = CRC_STANDARD_CCITT;
    drv_crc_config(&crc1, &config);
    out = (uint16_t)(drv_crc_calculate(&crc1, data, size));

    while (status.busy == 1U) {
        status = drv_crc_get_state(&crc1);
    }

    drv_crc_uninit(&crc1);

    return out;
}

/**
 * \brief Compute the CRC-16 checksum of a buffer.
 *  init = 0; poly = 0x3d65 refin = 1 refout = 1 xorout = 0xffff
 * \param[in] init_value      crc init value
 * \param[in] data            data buf to be calculate
 * \param[in] size            data size
 * \return    The computed CRC16 with xorout
 */
uint16_t csi_crc16_dnp(uint16_t init_value, uint8_t *data, uint32_t size)
{
    return 0;
}

/**
 * \brief Compute the CRC-16 checksum of a buffer.
 *  init = 0; poly = 0x1021 refin = 0 refout = 0 xorout = 0
 * \param[in] crc      crc init value or crc immediate result
 * \param[in] data     data buf to be calculate
 * \param[in] size     data size
 * \return    The computed CRC16 without xorout
 */
uint16_t csi_crc16_itu(uint16_t crc, uint8_t *data, uint32_t size)
{
    return 0;

}

uint32_t csi_crc32_le(uint32_t crc, uint8_t *data, uint32_t size)
{
    return 0;
}

/**
 * \brief Compute the CRC-32 checksum of a buffer.
 *  init = 0; poly = 0x04c11db7 refin = 0 refout = 0 xorout = 0
 * \param[in] crc      crc init value or crc immediate result
 * \param[in] data     data buf to be calculate
 * \param[in] size     data size
 * \return    The computed CRC32 without xorout
 */
uint32_t csi_crc32_be(uint32_t crc, uint8_t *data, uint32_t size)
{
    return 0;
}

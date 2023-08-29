/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     sensor.c
 * @brief    source file for sensor csi driver
 * @version  V1.0
 * @date     15. June 2020
 ******************************************************************************/
#include <string.h>
#include <drv/tick.h>
#include <drv/cvi_irq.h>
#include "drv/sensor.h"
#include "wj_sensor_ll.h"
/**
  \brief       Initialize Sensor VD
  \param[in]   vds   Voltage range
  \param[in]   vhs   Voltage high threshold
  \param[in]   vls   Voltage low threshold
  \param[in]   vtm   Used to modify the threshold value of the voltage detection point
  \return      Error code
*/
csi_error_t drv_sensor_vd_init(drv_sensor_vds_t vds, drv_sensor_vhs_t vhs, drv_sensor_vls_t vls, uint8_t vtm)
{
    wj_sensor_regs_t *sensor_base = (wj_sensor_regs_t *)WJ_SENSOR_BASE;

    wj_sensor_vd_set_vtm(sensor_base, vtm);
    wj_sensor_vd_set_vds(sensor_base, vds);
    wj_sensor_vd_set_vhs(sensor_base, vhs);
    wj_sensor_vd_set_vls(sensor_base, vls);
    wj_sensor_vd_enble_ana(sensor_base);
    mdelay(2);
    wj_sensor_vd_enble(sensor_base);
    return CSI_OK;
}

/**
  \brief       Get vd  warn
  \param[in]   warn  Vd warn select
  \return      fd warn code
*/
uint32_t drv_sensor_vd_get_warn(drv_sensor_warn_t warn)
{
    uint32_t ret = 0;
    wj_sensor_regs_t *sensor_base = (wj_sensor_regs_t *)WJ_SENSOR_BASE;

    switch (warn) {
        case SENSOR_WARN_H:
            ret = wj_sensor_vd_get_vh_status(sensor_base);
            break;

        case SENSOR_WARN_L:
            ret = wj_sensor_vd_get_vl_status(sensor_base);
            break;

        case SENSOR_WARN_RST:
            ret = wj_sensor_vd_get_vrst_status(sensor_base);
            break;

        default:
            break;
    }

    return ret;
}

/**
  \brief       Initialize Sensor FD
  \param[in]   hs   Temp high threshold
  \param[in]   ls   Temp low threshold
  \return      Error code
*/
csi_error_t drv_sensor_fd_init(drv_sensor_freq_t freq, drv_sensor_fhs_t fhs, drv_sensor_fls_t fls)
{
    wj_sensor_regs_t *sensor_base = (wj_sensor_regs_t *)(WJ_SENSOR_BASE + 0x100U);
    wj_sensor_fd_set_rfdtm(sensor_base, ((uint8_t)freq % 4));

    if (freq > SENSOR_FHS_FREQ_12M) {
        wj_sensor_fd_set_enble_tri(sensor_base);
    } else {
        wj_sensor_fd_set_disenble_tri(sensor_base);
    }

    wj_sensor_fd_set_fhs(sensor_base, fhs);
    wj_sensor_fd_set_fls(sensor_base, fls);
    wj_sensor_fd_enble_ana(sensor_base);
    mdelay(2);
    wj_sensor_fd_enble(sensor_base);
    return CSI_OK;
}

/**
  \brief       Get fd  warn
  \param[in]   warn  Fd warn select
  \return      fd warn code
*/
uint32_t drv_sensor_fd_get_warn(drv_sensor_warn_t warn)
{
    uint32_t ret = 0;
    wj_sensor_regs_t *sensor_base = (wj_sensor_regs_t *)(WJ_SENSOR_BASE + 0x100U);

    switch (warn) {
        case SENSOR_WARN_H:
            ret = wj_sensor_fd_get_htd_status(sensor_base);
            break;

        case SENSOR_WARN_L:
            ret = wj_sensor_fd_get_ltd_status(sensor_base);
            break;

        case SENSOR_WARN_RST:
            ret = wj_sensor_fd_get_reset_status(sensor_base);
            break;

        default:
            break;
    }

    return ret;
}

/**
  \brief       Initialize Sensor td
  \param[in]   hs   Temp high threshold
  \param[in]   ls   Temp low threshold
  \return      Error code
*/
csi_error_t drv_sensor_td_init(drv_sensor_tdhs_t hs, drv_sensor_tdls_t ls)
{
    wj_sensor_regs_t *sensor_base = (wj_sensor_regs_t *)(WJ_SENSOR_BASE + 0x200U);
    wj_sensor_td_set_bgtc(sensor_base, 15);
    wj_sensor_td_set_hs(sensor_base, hs);
    wj_sensor_td_set_ls(sensor_base, ls);
    wj_sensor_td_enble_ana(sensor_base);
    mdelay(2);
    wj_sensor_td_enble(sensor_base);
    return CSI_OK;
}

/**
  \brief       Get td  warn
  \param[in]   warn  Td warn select
  \return      Td warn code
*/
uint32_t drv_sensor_td_get_warn(drv_sensor_warn_t warn)
{
    uint32_t ret = 0;
    wj_sensor_regs_t *sensor_base = (wj_sensor_regs_t *)(WJ_SENSOR_BASE + 0x200U);

    switch (warn) {
        case SENSOR_WARN_H:
            ret = wj_sensor_td_get_htd_status(sensor_base);
            break;

        case SENSOR_WARN_L:
            ret = wj_sensor_td_get_ltd_status(sensor_base);
            break;

        case SENSOR_WARN_RST:
            ret = wj_sensor_td_get_trst_status(sensor_base);
            break;

        default:
            break;
    }

    return ret;
}

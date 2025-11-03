/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     ck_trng.c
 * @brief
 * @version
 * @date     27. April 2020
 ******************************************************************************/

#include <drv/rng.h>
#include <drv/cvi_irq.h>
#include "wj_trng_ll.h"
#include <drv/tick.h>
#include <string.h>

#define DEFAULT_OSR_TRNG_TIMEOUT 1000000U

/**
  \brief       Get data from the TRNG engine
  \param[out]  data  Pointer to buffer with data get from TRNG
  \param[in]   num   Number of data items,uinit in uint32
  \return      error code
*/
csi_error_t csi_rng_get_multi_word(uint32_t *data, uint32_t num)
{
    uint32_t timestart;
    csi_error_t ret = CSI_OK;
    wj_trng_regs_t *trng_base = (wj_trng_regs_t *)WJ_RNG_BASE;

    CSI_PARAM_CHK(data, CSI_ERROR);
    if (num == 0U) {
        ret = CSI_ERROR;
    }else{
        timestart = csi_tick_get();
        int i = 0;
        while (num) {
            wj_trng_enable_module(trng_base);

            while(!wj_trng_data_valid(trng_base)) {
                if ((csi_tick_get() - timestart) > DEFAULT_OSR_TRNG_TIMEOUT) {
                    ret = CSI_TIMEOUT;
                    return ret;
                }
            }
            
            data[i] = wj_trng_get_data(trng_base);
            i++;
            num--;
        }
    }
    return ret;
}

/**
  \brief       Get data from the TRNG engine
  \return      error code
*/
csi_error_t csi_rng_get_single_word(uint32_t* data)
{
    uint32_t timestart;
    csi_error_t ret = CSI_OK;
    static wj_trng_regs_t *trng_base = (wj_trng_regs_t *)WJ_RNG_BASE;
    CSI_PARAM_CHK(data, CSI_ERROR);

    timestart = csi_tick_get();
    wj_trng_enable_module(trng_base);

    while(!wj_trng_data_valid(trng_base)) {
        if ((csi_tick_get() - timestart) > DEFAULT_OSR_TRNG_TIMEOUT) {
            ret = CSI_TIMEOUT;
            return ret;
        }
    }

    *data = wj_trng_get_data(trng_base);

    return  ret;
}

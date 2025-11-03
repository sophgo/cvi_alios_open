/*
 * Copyright (C) 2019-2022 Cvitek Group Holding Limited
 */

/******************************************************************************
 * @file     trng.c
 * @brief
 * @version
 * @date     2022-09-05
 ******************************************************************************/
#include <string.h>
#include <drv/tick.h>
#include <drv/rng.h>

#include "mmio.h"

#define SEC_BASE            0x02000000
#define TRNG_BASE           (SEC_BASE + 0x00070000)

#define BIT(n) (UINT32_C(1) << (n))

#define TRNG_RAND0_REG_ADDR 0x24
#define TRNG_DATA_WIDTH 32
#define TRNG_ADDR_WIDTH 8

#define GetTrngAddr(addrOff)  (addrOff+TRNG_BASE)

static csi_error_t _csi_rng_init()
{
    uint32_t  data;
    static int init = 0;

    if (init == 1) {
        return CSI_OK;
    }

    init = 1;

    // wait idle
    do {
        data = mmio_read_32(GetTrngAddr(0xc));
    } while((data & 0x80000000) == 0x80000000);

    printf("start seed generation\n");
    mmio_write_32(GetTrngAddr(0x00), 0x1);

    do {
        data = mmio_read_32(GetTrngAddr(0x14));
    } while(!(data & 0x10));
    // ack and clear the done bit flag
    mmio_write_32(GetTrngAddr(0x14), 0x10);

    printf("create state\n");
    mmio_write_32(GetTrngAddr(0x00), 0x3);

    do {
        data = mmio_read_32(GetTrngAddr(0x14));
    } while(!(data & 0x10));
    // ack and clear the done bit flag
    mmio_write_32(GetTrngAddr(0x14), 0x10);

    return CSI_OK;
}

/**
  \brief       Get data from the TRNG engine
  \param[out]  data  Pointer to buffer with data get from TRNG
  \param[in]   num   Number of data items,uinit in uint32
  \return      error code
*/
csi_error_t csi_rng_get_multi_word(uint32_t *data, uint32_t num)
{
    CSI_PARAM_CHK(data, CSI_ERROR);
    if (num == 0) return CSI_ERROR;

    int i, j = 0;
    uint32_t count = (num + 0x3) >> 2;
    csi_error_t ret = CSI_OK;
    uint32_t  status;

    _csi_rng_init();

    while (count--) {

        // generation random number
        mmio_write_32(GetTrngAddr(0x00), 0x6);

        do {
            status = mmio_read_32(GetTrngAddr(0x14));
        } while(!(status & 0x10));
        // ack and clear the done bit flag
        mmio_write_32(GetTrngAddr(0x14), 0x10);

        for (i = 0; i < (TRNG_DATA_WIDTH/TRNG_ADDR_WIDTH); i++) {
            data[j++] = mmio_read_32(GetTrngAddr(0x24)+(4*i));
            if (j >= num) {
                break;
            }
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
    CSI_PARAM_CHK(data, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    uint32_t  status;

    _csi_rng_init();

    // generation random number
    mmio_write_32(GetTrngAddr(0x00), 0x6);

    do {
        status = mmio_read_32(GetTrngAddr(0x14));
    } while(!(status & 0x10));
    // ack and clear the done bit flag
    mmio_write_32(GetTrngAddr(0x14), 0x10);

    data[0] = mmio_read_32(GetTrngAddr(0x24));

    return  ret;
}

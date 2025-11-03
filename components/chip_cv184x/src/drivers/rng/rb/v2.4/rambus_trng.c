/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     trng.c
 * @brief
 * @version
 * @date     2020-09-28
 ******************************************************************************/
#include <string.h>
#include <stdint.h>
#include <drv/tick.h>
#include <drv/rng.h>
#include "eip76_internal.h"
#include <eip76_read.h>
#include "eip76_init.h"
#include <unistd.h>
#include "rambus.h"

#define DRIVER_HRNG_RANDOM_BUFFER_LEN 16
#define DEFAULT_TIMEOUT 1000U

#define EIP76_ALARM_EVENTS 0
#define EIP76_ALARM_EVENTS 0
#define EIP76_FATAL_EVENTS (EIP76_STUCK_OUT_EVENT | EIP76_NOISE_FAIL_EVENT)

#define EIP76_IMPFRO_ENABLED_MASK 0x00000F

static uint32_t trng_initialized = 0;

#define ADAPTER_EIP76_PS_WORD32_COUNT 12

static uint32_t Adapter_EIP76_PS_Data[ADAPTER_EIP76_PS_WORD32_COUNT] = {
    0x64299d83, 0xc34d7098, 0x5bd1f51d, 0xddccfdc1, 0xdd0455b7, 0x166279e5,
    0x0974cb1b, 0x2f2cd100, 0x59a5060a, 0xca79940d, 0xd4e29a40, 0x56b7b779};

/*----------------------------------------------------------------------------
 * Adapter_SleepMS
 */
void Adapter_SleepMS(const unsigned int Duration_ms)
{
    //usleep(Duration_ms * 1000);
    udelay(Duration_ms * 1000); // Wait what's left from it
}

csi_error_t trng_init(void)
{
    EIP76_Status_t  ret;
    EIP76_IOArea_t  io;
    Device_Handle_t handle = NULL;
    if (trng_initialized) {
        TRACE();
        return CSI_OK;
    }
    ret = EIP76_Initialize(&io, handle, Adapter_EIP76_PS_Data, ADAPTER_EIP76_PS_WORD32_COUNT);
    CHECK_RET_WITH_RET(ret == EIP76_NO_ERROR, SC_DRV_FAILED);
    trng_initialized = 1;

    return CSI_OK;
}

void trng_uninit(void)
{
    /* EIP76_Shutdown(); */
}

static int s_event_check(EIP76_EventStatus_t events)
{
    if (events & EIP76_FATAL_EVENTS) {
        TRACE_ERR(SC_DRV_FAILED);
        return SC_DRV_FAILED;
    }

    if (events & (EIP76_APROP_FAIL_EVENT | EIP76_REPCNT_FAIL_EVENT | EIP76_STUCK_NRBG_EVENT)) {
        TRACE_ERR(SC_DRV_FAILED);
        return SC_DRV_FAILED;
    }

    if (events & EIP76_ALARM_EVENTS) {
        TRACE_ERR(SC_DRV_FAILED);
        return SC_DRV_FAILED;
    }

    return 0;
}

static int s_get_random_data_once(uint8_t *buf)
{
    EIP76_Status_t      res;
    EIP76_EventStatus_t events = 0;
    unsigned int        actual = 0;
    EIP76_IOArea_t      ioarea;
    uint32_t            timestart;
    uint32_t            rt  = 0;
    csi_error_t         ret = CSI_OK;

    timestart = csi_tick_get();
    rt        = timestart;
    while ((rt - timestart) < DEFAULT_TIMEOUT) {
        EIP76_Request_Random_Data(&ioarea, 16);
        res = EIP76_Random_IsBusy(&ioarea, &events);
        if (res != EIP76_NO_ERROR) {
            rt  = csi_tick_get();
            ret = SC_DRV_FAILED;
            continue;
        }
        /* 1 block */
        res = EIP76_Random_Get(&ioarea, (EIP76_Random128_t *)buf, 1, &actual, &events);
        /* retry */
        if (res == EIP76_BUSY_RETRY_LATER) {
            rt  = csi_tick_get();
            ret = SC_DRV_FAILED;
            continue;
        }
        /* err */
        if (res != EIP76_NO_ERROR) {
            TRACE_ERR(res);
            return SC_DRV_FAILED;
        }
        /* retry with wrong len */
        if (actual == 0) {
            rt  = csi_tick_get();
            ret = SC_DRV_FAILED;
            continue;
        }
        /* check event */
        return s_event_check(events);
    }

    return ret;
}

static int s_get_random_data(uint8_t *buf, const int count)
{
    EIP76_Status_t res   = CSI_ERROR;
    uint32_t       times = count / DRIVER_HRNG_RANDOM_BUFFER_LEN;
    uint32_t       rm    = count % DRIVER_HRNG_RANDOM_BUFFER_LEN;
    uint8_t *      p     = buf;

    while (times > 0) {
        res = s_get_random_data_once(p);
        CHECK_RET_WITH_RET(res == EIP76_NO_ERROR, res);
        p += DRIVER_HRNG_RANDOM_BUFFER_LEN;
        times--;
    }

    if (rm > 0) {
        EIP76_Random128_t tmp_buf;

        /* Use a temporary buffer of a single random block. Only copy as
        many bytes to the caller buffer as desired. */
        res = s_get_random_data_once((uint8_t *)&tmp_buf);
        CHECK_RET_WITH_RET(res == EIP76_NO_ERROR, res);
        memcpy(p, &tmp_buf, rm);
        memset(&tmp_buf, 0, DRIVER_HRNG_RANDOM_BUFFER_LEN);
        return res;
    }

    return res;
}

/**
  \brief       Get data from the TRNG engine
  \param[out]  data  Pointer to buffer with data get from TRNG
  \param[in]   num   Number of data items,uinit in uint32
  \return      error code
*/
csi_error_t csi_rng_get_multi_word(uint32_t *data, uint32_t num)
{
    CHECK_RET_WITH_RET(data, SC_PARAM_INV);
    CHECK_RET_WITH_RET(num > 0, SC_PARAM_INV);

    uint32_t ret = trng_init();
    CHECK_RET_WITH_RET(ret == CSI_OK, ret);

    RB_PERF_START_POINT();
    ret = s_get_random_data((uint8_t *)data, num * sizeof(uint32_t));

    RB_PERF_END_POINT();
    trng_uninit();

    return ret;
}

/**
  \brief       Get data from the TRNG engine
  \return      error code
*/
csi_error_t csi_rng_get_single_word(uint32_t *data)
{
    CHECK_RET_WITH_RET(data, SC_PARAM_INV);

    uint32_t ret = trng_init();
    CHECK_RET_WITH_RET(ret == CSI_OK, ret);

    ret = s_get_random_data((uint8_t *)data, sizeof(uint32_t));
    trng_uninit();

    return ret;
}

/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file       rambus_sm3.c
 * @brief      Header File for SM3 Driver
 * @version    V2.0
 * @date       9. DEC 2020
 * @model      SM3
 ******************************************************************************/

#include <drv/sm3.h>
#include <drv/cvi_irq.h>
#include <basic_defs.h>
#include "device_rw.h"
#include "soc.h"
#include "rambus.h"

// Function documentation

/**
  \brief       Initialize SM3 Interface. Initializes the resources needed for the SM3 interface
  \param[in]   sm3  operate handle.
  \param[in]   idx index of sm3
  \return      error code \ref uint32_t
*/
csi_error_t csi_sm3_init(csi_sm3_t *sm3, uint32_t idx)
{
        CHECK_RET_WITH_RET(sm3, SC_PARAM_INV);

        csi_error_t ret = SC_OK;
        if (0 == target_get(DEV_RAMBUS_120SI_TAG, idx, &sm3->dev)) {
                ret = SC_OK;
        } else {
                ret = CSI_ERROR;
        }

        return ret;
}

/**
  \brief       De-initialize SM3 Interface. stops operation and releases the software resources used by the interface
  \param[in]   sm3  sm3 handle to operate.
  \return      none
*/
void csi_sm3_uninit(csi_sm3_t *sm3)
{
        CHECK_RET(sm3);
}

/**
  \brief       Attach the callback handler to SM3
  \param[in]   sm3         Handle to operate
  \param[in]   callback    Callback function
  \param[in]   arg         Callback's param
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_sm3_attach_callback(csi_sm3_t *sm3, void *callback, void *arg)
{
        return CSI_UNSUPPORTED;
}

/**
  \brief       Detach the callback handler
  \param[in]   sm3    Handle to operate
  \return      None
*/
void csi_sm3_detach_callback(csi_sm3_t *sm3)
{
}

/* len ali 4 */
static void rb_sm3_process(Device_Handle_t *dev_sm3, uint8_t *src, uint32_t len,
                           uint32_t last, uint32_t tlen)
{
        uint32_t *ps        = (uint32_t *)src;
        uint32_t  i         = 0;
        uint32_t  datas[16] = {
            EIP120_HASH_DATA_IN_0,  EIP120_HASH_DATA_IN_1,
            EIP120_HASH_DATA_IN_2,  EIP120_HASH_DATA_IN_3,
            EIP120_HASH_DATA_IN_4,  EIP120_HASH_DATA_IN_5,
            EIP120_HASH_DATA_IN_6,  EIP120_HASH_DATA_IN_7,
            EIP120_HASH_DATA_IN_8,  EIP120_HASH_DATA_IN_9,
            EIP120_HASH_DATA_IN_10, EIP120_HASH_DATA_IN_11,
            EIP120_HASH_DATA_IN_12, EIP120_HASH_DATA_IN_13,
            EIP120_HASH_DATA_IN_14, EIP120_HASH_DATA_IN_15,
        };
        //Wait for rfd_in=1
        /* DEBUG try time */
        while ((Device_Read32(dev_sm3, EIP120_HASH_IO_BUF_CTRL) & 0x00000004) !=
               0x00000004) {
                ;
        }
        while (len > 0) {
                Device_Write32(dev_sm3, datas[i], *ps);
                i++;
                ps++;
                len -= 4;
        }
        if (last) {
                Device_Write32(dev_sm3, EIP120_HASH_LENGTH_IN_L,
                               tlen * 8); //bits
                Device_Write32(dev_sm3, EIP120_HASH_LENGTH_IN_H, 0);
                Device_Write32(dev_sm3, EIP120_HASH_IO_BUF_CTRL,
                               0x00000022); //pad && datain
        } else {
                Device_Write32(dev_sm3, EIP120_HASH_IO_BUF_CTRL, 0x00000002);
        }
        /* NOTE set unused data in 0? */
}

/* len ali 4 */
static void rb_sm3_output(Device_Handle_t *dev_sm3, uint8_t *dst, uint32_t len)
{
        uint32_t *ps       = (uint32_t *)dst;
        uint32_t  i        = 0;
        uint32_t  datas[8] = {
            EIP120_HASH_DIGEST_INOUT_A, EIP120_HASH_DIGEST_INOUT_B,
            EIP120_HASH_DIGEST_INOUT_C, EIP120_HASH_DIGEST_INOUT_D,
            EIP120_HASH_DIGEST_INOUT_E, EIP120_HASH_DIGEST_INOUT_F,
            EIP120_HASH_DIGEST_INOUT_G, EIP120_HASH_DIGEST_INOUT_H,
        };
        //Wait for rfd_in=1
        /* DEBUG try time */
        while ((Device_Read32(dev_sm3, EIP120_HASH_IO_BUF_CTRL) & 0x00000001) !=
               0x00000001) //output_full
        {
                ;
        }
        while (len > 0) {
                *ps = Device_Read32(dev_sm3, datas[i]);
                ps++;
                i++;
                len -= 4;
        }
        Device_Write32(dev_sm3, EIP120_HASH_IO_BUF_CTRL, 0x00000001);
}

/**
  \brief       start the engine
  \param[in]   sm3     sm3 handle to .operate
  \param[in]   context Pointer to the sm3 context \ref csi_sm3_context_t
  \return      error code \ref uint32_t
*/
csi_error_t csi_sm3_start(csi_sm3_t *sm3, csi_sm3_context_t *context)
{
        CHECK_RET_WITH_RET(sm3, SC_PARAM_INV);
        CHECK_RET_WITH_RET(context, SC_PARAM_INV);
        csi_error_t     ret     = SC_OK;
        Device_Handle_t dev_sm3 = (Device_Handle_t)HANDLE_REG_BASE(sm3);

        LOG_INFO("===%s, %d\n", __FUNCTION__, __LINE__);
        Device_Write32(dev_sm3, EIP120_HASH_MODE_IN,
                       0x00000001); //sm3 new hash
        Device_Write32(dev_sm3, EIP120_HASH_IO_BUF_CTRL, 0x00000001); //1
        Device_Write32(dev_sm3, EIP120_CTRL_ALG_SEL, 0x00000000);
        memset(context, 0, sizeof(csi_sm3_context_t));

        return ret;
}

/**
  \brief       update the engine
  \param[in]   sm3     sm3 handle to operate.
  \param[in]   context Pointer to the sm3 context \ref csi_sm3_context_t
  \param[in]   input   Pointer to the Source data
  \param[in]   size    the data size
  \return      error code \ref uint32_t
*/
csi_error_t csi_sm3_update(csi_sm3_t *sm3, csi_sm3_context_t *context,
                           const uint8_t *input, uint32_t size)
{
        CHECK_RET_WITH_RET(sm3, SC_PARAM_INV);
        CHECK_RET_WITH_RET(context, SC_PARAM_INV);
        csi_error_t     ret     = SC_OK;
        Device_Handle_t dev_sm3 = (Device_Handle_t)HANDLE_REG_BASE(sm3);

        LOG_INFO("===%s, %d, len:%d\n", __FUNCTION__, __LINE__, size);
        size_t   fill;
        uint32_t left;
        uint8_t *pin = (uint8_t *)input;

        if (size == 0)
                return SC_OK;

        sm3->state.busy = 1U;
        left            = context->total[1];

        if (left == 64) {
                rb_sm3_process(dev_sm3, context->buffer, 64, 0, 0);
                left = 0;
        }
        fill = 64 - left;

        context->total[0] += (uint32_t)size;

        if (left && size >= fill) {
                /* fill to 64 */
                memcpy((void *)(context->buffer + left), pin, fill);
                LOG_INFO("===%s, %d, len:%d\n", __FUNCTION__, __LINE__, size);
                pin += fill;
                size -= fill;
                left = 64;
        }

        RB_PERF_START_POINT();
        while (size > 64) {
                if (left == 64) {
                        rb_sm3_process(dev_sm3, context->buffer, 64, 0, 0);
                        left = 0;
                }
                LOG_INFO("===%s, %d, len:%d\n", __FUNCTION__, __LINE__, size);
                rb_sm3_process(dev_sm3, pin, 64, 0, 0);
                pin += 64;
                size -= 64;
        }
        RB_PERF_END_POINT();

        /* <= 64 */
        if (size > 0) {
                if (left == 64) {
                        rb_sm3_process(dev_sm3, context->buffer, 64, 0, 0);
                        left = 0;
                }
                LOG_INFO("===%s, %d, len:%d\n", __FUNCTION__, __LINE__, size);
                memcpy((void *)(context->buffer + left), pin, size);
                left += size;
        }
        LOG_INFO("===%s, %d, len:%d\n", __FUNCTION__, __LINE__, size);
        context->total[1] = left;

        return ret;
}

/**
  \brief       Accumulate the engine (async mode)
  \param[in]   sm3        Handle to operate
  \param[in]   context    Pointer to the SM3 context \ref csi_sm3_context_t
  \param[in]   input      Pointer to the Source data
  \param[in]   size       The data size
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_sm3_update_async(csi_sm3_t *sm3, csi_sm3_context_t *context,
                                 const uint8_t *input, uint32_t size)
{
        return CSI_UNSUPPORTED;
}

/**
  \brief       finish the engine
  \param[in]   sm3      sm3 handle to operate.
  \param[in]   context  Pointer to the sm3 context \ref csi_sm3_context_t
  \param[out]  output   Pointer to the result data
  \param[out]  out_size Pointer to the result data size(bytes)
  \return      error code \ref uint32_t
*/
csi_error_t csi_sm3_finish(csi_sm3_t *sm3, csi_sm3_context_t *context,
                           uint8_t *output, uint32_t *out_size)
{
        CHECK_RET_WITH_RET(sm3, SC_PARAM_INV);
        CHECK_RET_WITH_RET(context, SC_PARAM_INV);
        csi_error_t     ret = CSI_OK;
        uint32_t        t;
        uint32_t        left    = context->total[1];
        Device_Handle_t dev_sm3 = (Device_Handle_t)HANDLE_REG_BASE(sm3);

        if (left < 64) {
                LOG_INFO("===%s, %d, :%d\n", __FUNCTION__, __LINE__, left);
                memset(context->buffer + left, 0, 64 - left);
        }
        t = (left + 3) & (~3);
        LOG_INFO("===%s, %d, :%d, %d, %d\n", __FUNCTION__, __LINE__,
                 context->total[0], t, left);
        rb_sm3_process(dev_sm3, context->buffer, t, 1, context->total[0]);
        /* out */
        rb_sm3_output(dev_sm3, output, 32);
        *out_size = 32;

        LOG_INFO("===%s, %d\n", __FUNCTION__, __LINE__);
        memset((uint8_t *)context, 0, sizeof(csi_sm3_context_t));
        sm3->state.busy = 0U;

        return ret;
}

/**
  \brief       Get SM3 state
  \param[in]   sm3      Handle to operate
  \param[out]  state    SM3 state \ref csi_sm3_state_t
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_sm3_get_state(csi_sm3_t *sm3, csi_sm3_state_t *state)
{
        return CSI_UNSUPPORTED;
}

/**
  \brief       Enable SM3 power manage
  \param[in]   sm3     Handle to operate
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_sm3_enable_pm(csi_sm3_t *sm3)
{
        return CSI_UNSUPPORTED;
}

/**
  \brief       Disable SM3 power manage
  \param[in]   sm3    Handle to operate
  \return      None
*/
void csi_sm3_disable_pm(csi_sm3_t *sm3)
{
}

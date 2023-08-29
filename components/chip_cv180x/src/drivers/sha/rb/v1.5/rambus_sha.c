/*
 *	Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/*******************************************************
 * @file 	sha.c
 * @brief	source file for sha csi driver
 * @version V2.0
 * @date	14. Nov 2020
 * ******************************************************/

#include <drv/sha.h>
#include <drv/cvi_irq.h>
#include <basic_defs.h>
#include "device_rw.h"
#include "soc.h"
#include "rambus.h"
#include "drv/tick.h"


/*<! Public function documentation */

/**
  \brief       Initialize SHA Interface.
  \param[in]   sha  operate handle.
  \param[in]   idx index of sha
  \return      \ref csi_error_t
*/
csi_error_t csi_sha_init(csi_sha_t *sha, uint32_t idx)
{
    CHECK_RET_WITH_RET(sha, SC_PARAM_INV);
    csi_error_t ret = CSI_OK;

    memset(sha, 0, sizeof(csi_sha_t));
    if (0 == target_get(DEV_RAMBUS_120SII_TAG, idx, &sha->dev))
    {
        ret = CSI_OK;
    }
    else
    {
        ret = CSI_ERROR;
    }

    return ret;
}

/**
  \brief       De-initialize SHA Interface.
  \param[in]   sha  sha handle to operate.
  \return      none
*/
void csi_sha_uninit(csi_sha_t *sha)
{
    CHECK_RET(sha);
}

/**
  \brief       attach the callback handler to SHA
  \param[in]   sha          operate handle.
  \param[in]   callback     callback function
  \param[in]   arg          callback's param
  \return      error code
*/
csi_error_t csi_sha_attach_callback(csi_sha_t *sha, void *callback, void *arg)
{
    CHECK_RET_WITH_RET(sha, SC_PARAM_INV);

    csi_error_t ret = CSI_UNSUPPORTED;

    return ret;
}

/**
  \brief       detach the callback handler
  \param[in]   sha  operate handle.
*/
void csi_sha_detach_callback(csi_sha_t *sha)
{
    CHECK_RET(sha);
}

#define EIP120SII ((volatile eip120si_reg_tt *)RB_EIP120SII_BASE)

/* len ali 4 */
static void rb_sha256_process(Device_Handle_t *dev_sha, uint8_t *src, uint32_t len, uint32_t last, uint32_t tlen)
{
    uint32_t *ps = (uint32_t *)src;
    uint32_t i = 0;
    uint32_t datas[16] =
        {
            EIP120_HASH_DATA_IN_0,
            EIP120_HASH_DATA_IN_1,
            EIP120_HASH_DATA_IN_2,
            EIP120_HASH_DATA_IN_3,
            EIP120_HASH_DATA_IN_4,
            EIP120_HASH_DATA_IN_5,
            EIP120_HASH_DATA_IN_6,
            EIP120_HASH_DATA_IN_7,
            EIP120_HASH_DATA_IN_8,
            EIP120_HASH_DATA_IN_9,
            EIP120_HASH_DATA_IN_10,
            EIP120_HASH_DATA_IN_11,
            EIP120_HASH_DATA_IN_12,
            EIP120_HASH_DATA_IN_13,
            EIP120_HASH_DATA_IN_14,
            EIP120_HASH_DATA_IN_15,
        };
    //Wait for rfd_in=1
    /* TODO try time */
    while ((Device_Read32(dev_sha, EIP120_HASH_IO_BUF_CTRL) & 0x00000004) != 0x00000004)
    {
        ;
    }
    while (len > 0)
    {
        Device_Write32(dev_sha, datas[i], *ps);
        i++;
        ps++;
        len -= 4;
    }
    if (last)
    {
        
        Device_Write32(dev_sha, EIP120_HASH_LENGTH_IN_L, tlen * 8);//bits
        Device_Write32(dev_sha, EIP120_HASH_LENGTH_IN_H, 0);
        Device_Write32(dev_sha, EIP120_HASH_IO_BUF_CTRL, 0x00000022); //pad && datain
    }
    else
    {
        Device_Write32(dev_sha, EIP120_HASH_IO_BUF_CTRL, 0x00000002);
    }
    /* NOTE set unused data in 0? */
}

/* len ali 4 */
static void rb_sha256_output(Device_Handle_t *dev_sha, uint8_t *dst, uint32_t len)
{
    uint32_t *ps = (uint32_t *)dst;
    uint32_t i = 0;
    uint32_t datas[8] =
        {
            EIP120_HASH_DIGEST_INOUT_A,
            EIP120_HASH_DIGEST_INOUT_B,
            EIP120_HASH_DIGEST_INOUT_C,
            EIP120_HASH_DIGEST_INOUT_D,
            EIP120_HASH_DIGEST_INOUT_E,
            EIP120_HASH_DIGEST_INOUT_F,
            EIP120_HASH_DIGEST_INOUT_G,
            EIP120_HASH_DIGEST_INOUT_H,
        };
    //Wait for rfd_in=1
    /* TODO try time */
    while ((Device_Read32(dev_sha, EIP120_HASH_IO_BUF_CTRL) & 0x00000001) != 0x00000001) //output_full
    {
        ;
    }
    while (len > 0)
    {
        *ps = Device_Read32(dev_sha, datas[i]);
        ps++;
        i++;
        len -= 4;
    }
    Device_Write32(dev_sha, EIP120_HASH_IO_BUF_CTRL, 0x00000001);
}

/**
  \brief       config sha mode.
  \param[in]   sha     sha handle to operate.
  \param[in]   context Pointer to the sha context
  \param[in]   mode    sha mode \ref csi_sha_mode_t
  \return      \ref csi_error_t
*/
csi_error_t csi_sha_start(csi_sha_t *sha, csi_sha_context_t *context, csi_sha_mode_t mode)
{
    CHECK_RET_WITH_RET(sha, SC_PARAM_INV);
    CHECK_RET_WITH_RET(context, SC_PARAM_INV);
    csi_error_t ret = CSI_OK;
    Device_Handle_t dev_sha = (Device_Handle_t)HANDLE_REG_BASE(sha);

    LOG_INFO("===%s, %d\n", __FUNCTION__, __LINE__);
    switch (mode)
    {
    case SHA_MODE_256:
        LOG_INFO("===%s, %d\n", __FUNCTION__, __LINE__);
        Device_Write32(dev_sha, EIP120_HASH_MODE_IN, 0x00000009);//sha_256 new hash
        Device_Write32(dev_sha, EIP120_HASH_IO_BUF_CTRL, 0x00000001); //1
        Device_Write32(dev_sha, EIP120_CTRL_ALG_SEL, 0x00000000);
        memset(context, 0, sizeof(csi_sha_context_t));
        context->mode = SHA_MODE_256;
        break;
    case SHA_MODE_1:
    case SHA_MODE_224:
    case SHA_MODE_512:
    case SHA_MODE_384:
    case SHA_MODE_512_256:
    case SHA_MODE_512_224:
        ret = CSI_UNSUPPORTED;
        break;

    default:
        ret = SC_PARAM_INV;
        break;
    }

    return ret;
}

/**
  \brief       update the engine
  \param[in]   sha     sha handle to operate.
  \param[in]   context Pointer to the sha context
  \param[in]   input   Pointer to the Source data
  \param[in]   size    the data size
  \return      \ref csi_error_t
*/
csi_error_t csi_sha_update(csi_sha_t *sha, csi_sha_context_t *context, const void *input, uint32_t size)
{
    CHECK_RET_WITH_RET(sha, SC_PARAM_INV);
    CHECK_RET_WITH_RET(context, SC_PARAM_INV);
    csi_error_t ret = CSI_OK;
    Device_Handle_t dev_sha = (Device_Handle_t)HANDLE_REG_BASE(sha);

    LOG_INFO("===%s, %d, len:%d\n", __FUNCTION__, __LINE__, size);
    size_t fill;
    uint32_t left;
    uint8_t *pin = (uint8_t *)input;

    if (size == 0)
        return CSI_OK;

    sha->state.busy = 1U;
    left = context->total[1];

    if (left == 64) {
        rb_sha256_process(dev_sha, context->buffer, 64, 0, 0);
        left  = 0;
    }
    fill = 64 - left;

    context->total[0] += (uint32_t)size;

    if (left && size >= fill)
    {
        /* fill to 64 */
        memcpy((void *)(context->buffer + left), pin, fill);
        LOG_INFO("===%s, %d, len:%d\n", __FUNCTION__, __LINE__, size);
        pin += fill;
        size -= fill;
        left = 64;
    }


    RB_PERF_START_POINT();
    while (size > 64)
    {
        if (left == 64) {
            rb_sha256_process(dev_sha, context->buffer, 64, 0, 0);
            left  = 0;
        }
        LOG_INFO("===%s, %d, len:%d\n", __FUNCTION__, __LINE__, size);
        rb_sha256_process(dev_sha, pin, 64, 0, 0);
        pin += 64;
        size -= 64;
    }

    RB_PERF_END_POINT();
    /* <= 64 */
    if (size > 0) {
        if (left == 64) {
            rb_sha256_process(dev_sha, context->buffer, 64, 0, 0);
            left  = 0;
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
  \brief       accumulate the engine (async mode)
  \param[in]   sha     sha handle to operate.
  \param[in]   context Pointer to the sha context
  \param[in]   input   Pointer to the Source data
  \param[in]   size    the data size
  \return      \ref csi_error_t
*/
csi_error_t csi_sha_update_async(csi_sha_t *sha, csi_sha_context_t *context, const void *input, uint32_t size)
{
    CHECK_RET_WITH_RET(sha, SC_PARAM_INV);
    CHECK_RET_WITH_RET(context, SC_PARAM_INV);

    csi_error_t ret = CSI_UNSUPPORTED;

    return ret;
}

/**
  \brief       finish the engine
  \param[in]   sha      sha handle to operate.
  \param[in]   context Pointer to the sha context
  \param[out]  output   Pointer to the result data
  \param[out]  out_size Pointer to the result data size(bytes)
  \return      \ref csi_error_t
*/
csi_error_t csi_sha_finish(csi_sha_t *sha, csi_sha_context_t *context, void *output, uint32_t *out_size)
{
    CHECK_RET_WITH_RET(sha, SC_PARAM_INV);
    CHECK_RET_WITH_RET(context, SC_PARAM_INV);
    csi_error_t ret = CSI_OK;
    uint32_t t;
    uint32_t left = context->total[1];
    Device_Handle_t dev_sha = (Device_Handle_t)HANDLE_REG_BASE(sha);

    if (left < 64)
    {
        LOG_INFO("===%s, %d, :%d\n", __FUNCTION__, __LINE__, left);
        memset(context->buffer + left, 0, 64 - left);
    }
    t = (left + 3) & (~3);
    LOG_INFO("===%s, %d, :%d, %d, %d\n", __FUNCTION__, __LINE__, context->total[0], t, left);
    rb_sha256_process(dev_sha, context->buffer, t, 1, context->total[0]);
    /* out */
    rb_sha256_output(dev_sha, output, 32);
    *out_size = 32;

    LOG_INFO("===%s, %d\n", __FUNCTION__, __LINE__);
    memset((uint8_t *)context, 0, sizeof(csi_sha_context_t));
    sha->state.busy = 0U;

    return ret;
}

/**
  \brief       Get SHA state.
  \param[in]   sha  sha handle to operate.
  \param[out]  state    sha state \ref csi_sha_state_t.
  \return      \ref csi_error_t
*/
csi_error_t csi_sha_get_state(csi_sha_t *sha, csi_sha_state_t *state)
{
    CHECK_RET_WITH_RET(sha, SC_PARAM_INV);
    CHECK_RET_WITH_RET(state, SC_PARAM_INV);

    state->busy = sha->state.busy;
    state->error = sha->state.error;

    return CSI_OK;
}

csi_error_t csi_sha_enable_pm(csi_sha_t *sha)
{
    CHECK_RET_WITH_RET(sha, SC_PARAM_INV);
    csi_error_t ret = CSI_UNSUPPORTED;

    return ret;
}

void csi_sha_disable_pm(csi_sha_t *sha)
{
    CHECK_RET(sha);
}

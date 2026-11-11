/*
 *	Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/*******************************************************
 * @file 	wj_sha.c
 * @brief	source file for sha csi driver
 * @version V1.0
 * @date	20. Aug 2020
 * ******************************************************/

#include <drv/sha.h>
#include <drv/cvi_irq.h>
#include "wj_sha_ll.h"

#define CONFIG_SHA_SUPPORT_MUL_THREAD            (1U)


/**
 * Different modes get different plaintext blocks
 */
#define WJ_SHA_GET_BOLOCK_SIZE_BYTES(_mod_)      (((_mod_) < SHA_MODE_512) ? (uint32_t)64 : (uint32_t)128)

/**
 * Different modes get different data lengths
 * \note SHA-1\SHA-224\SHA-256 need 2 * 4 = 8 bytes
 *       SHA-512\SHA-384       need 4 * 4 = 16 bytes
 */
#define WJ_SHA_GET_MSGLEN_TAIL_4BYTES(_mod_)    (((_mod_) < SHA_MODE_512) ? (uint32_t)2 : (uint32_t)4)

/**
 * Number of result message digest bytes retrieved by sha mode
 */
#define WJ_SHA_GET_MSGDIGEST_BYTES(_mod_)       (((_mod_) == SHA_MODE_1  ) ? (uint32_t)20 :    \
        ((_mod_) == SHA_MODE_256) ? (uint32_t)32 :    \
        ((_mod_) == SHA_MODE_224) ? (uint32_t)28 :    \
        ((_mod_) == SHA_MODE_512) ? (uint32_t)64 :    \
        ((_mod_) == SHA_MODE_384) ? (uint32_t)48 :    \
        (uint32_t)20)

#define WJ_SHA_ALG_SWAP(_x_, _y_)               {   \
        uint8_t z = (uint8_t)*_x_;                  \
        *_x_ = (uint8_t)*_y_;                       \
        *_y_ = z;                                   \
    }

#define WJ_SHA_ALG_ALIGN_4BYTE(_len_)           (((_len_) % 4U) ? ((((_len_) >> 2U) << 2U) + 4U) : (_len_))


#define WJ_SHA_ALG_SELECT_MIN(_x_, _y_)         (((_x_) < (_y_)) ? (_x_) : (_y_))

#define WJ_SHA_WAIT_WRITED_10S                                                        (10000U)

#define WJ_SHA_WAIT_IS_TIMEOUT(_time_ms_, _result_)                                   {        \
        do {                                                                                   \
            if (_time_ms_ >= WJ_SHA_WAIT_WRITED_10S) {                                         \
                _result_ = -1;                                                                 \
            }                                                                                  \
        } while(0);                                                                            \
    }

/*<! Private function documentation */

/**
  \brief       SHA data flipping
  \param[in]   p       operate start address
  \param[in]   n       operate data length
  \return      null
*/
void wj_sha_alg_reverse(uint8_t *p, uint8_t n)
{
    if (n) {
        WJ_SHA_ALG_SWAP((p + 0U), (p + 3U));        ///< BYTE0 <<---SWAP--->> BYTE3
        WJ_SHA_ALG_SWAP((p + 1U), (p + 2U));        ///< BYTE1 <<---SWAP--->> BYTE2

        wj_sha_alg_reverse((p + 4U), (n - 1U));     ///< offset next address(uint32_t) and reduce operate data count
    }
}

/**
  \brief       SHA interrupt handling function
  \param[in]   arg        Callback function member variables
  \return      null
*/
void wj_sha_irq_handler(void *arg)
{
    csi_sha_t *sha = (csi_sha_t *)arg;
    wj_sha_regs_t *sha_base = (wj_sha_regs_t *)sha->dev.reg_base;

    if (wj_sha_state_get_int_done(sha_base)) {
        if (sha->callback) {
            sha->callback(sha, SHA_EVENT_COMPLETE, sha->arg);
        }

        wj_sha_state_clr_int_done(sha_base);
    }
}

/*<! Public function documentation */

/**
  \brief       Initialize SHA Interface. 1. Initializes the resources needed for the SHA interface 2.registers event callback function
  \param[in]   sha  operate handle.
  \param[in]   idx index of sha
  \return      \ref csi_error_t
*/
csi_error_t csi_sha_init(csi_sha_t *sha, uint32_t idx)
{
    CSI_PARAM_CHK(sha, CSI_ERROR);
    csi_error_t ret = CSI_OK;
    wj_sha_regs_t *sha_base = (wj_sha_regs_t *)HANDLE_REG_BASE(sha);

    if (0 == target_get(DEV_WJ_SHA_TAG, idx, &sha->dev)) {
        sha_base = (wj_sha_regs_t *)HANDLE_REG_BASE(sha);
        memset((void *)sha_base, 0, sizeof(wj_sha_regs_t));

        sha->state.busy = 0U;
    } else {
        ret = CSI_ERROR;
    }

    return ret;
}

/**
  \brief       De-initialize SHA Interface. stops operation and releases the software resources used by the interface
  \param[in]   sha  sha handle to operate.
  \return      none
*/
void csi_sha_uninit(csi_sha_t *sha)
{
    CSI_PARAM_CHK_NORETVAL(sha);
    wj_sha_regs_t *sha_base = (wj_sha_regs_t *)HANDLE_REG_BASE(sha);

    memset((void *)sha_base, 0, sizeof(wj_sha_regs_t));
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
    CSI_PARAM_CHK(sha, CSI_ERROR);

    sha->callback = callback;
    sha->arg = arg;
    csi_irq_attach((uint32_t)sha->dev.irq_num, &wj_sha_irq_handler, &sha->dev);
    csi_irq_enable((uint32_t)sha->dev.irq_num);

    return CSI_OK;
}

/**
  \brief       detach the callback handler
  \param[in]   sha  operate handle.
*/
void csi_sha_detach_callback(csi_sha_t *sha)
{
    CSI_PARAM_CHK_NORETVAL(sha);
    sha->callback = NULL;
    sha->arg = NULL;
    csi_irq_disable((uint32_t)sha->dev.irq_num);
    csi_irq_detach((uint32_t)sha->dev.irq_num);
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
    CSI_PARAM_CHK(sha, CSI_ERROR);
    CSI_PARAM_CHK(context, CSI_ERROR);
    wj_sha_regs_t *sha_base = (wj_sha_regs_t *)HANDLE_REG_BASE(sha);
    csi_error_t ret = CSI_OK;

    switch (mode) {
        case SHA_MODE_1:
        case SHA_MODE_256:
        case SHA_MODE_224:
        case SHA_MODE_512:
        case SHA_MODE_384:
            wj_sha_mode_sel(sha_base, (uint32_t)mode);
            wj_sha_write_base_addr(sha_base, (uint32_t *)context->buffer);
            wj_sha_write_dest_addr(sha_base, context->state);

            memset((void *)&context->mode, 0, sizeof(csi_sha_context_t));
            context->mode = mode;
            break;

        case SHA_MODE_512_256:
        case SHA_MODE_512_224:
            ret = CSI_UNSUPPORTED;
            break;

        default:
            ret = CSI_ERROR;
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
    CSI_PARAM_CHK(sha, CSI_ERROR);
    CSI_PARAM_CHK(context, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    uint8_t *p_data_in = (uint8_t *)input;
    uint32_t msg_length;
    uint32_t block_size;
    uint32_t timecount = 0U;
    uint32_t count[4] = {0U};
    uint32_t left;
    uint32_t length;

    wj_sha_regs_t *sha_base = (wj_sha_regs_t *)HANDLE_REG_BASE(sha);

    sha->state.busy = 1U;

    block_size = WJ_SHA_GET_BOLOCK_SIZE_BYTES(context->mode);
    msg_length = WJ_SHA_GET_MSGLEN_TAIL_4BYTES(context->mode);

    context->total[0] += size;
    left = strlen((const char *)context->buffer);               ///< Get unused message

    /**
     * If there is any unused message, it will be added to the new message for calculation
    */
    if (left) {
        memcpy((uint8_t *)context->buffer + left, p_data_in, WJ_SHA_ALG_SELECT_MIN(block_size - left, size)); ///< pad input message to complete block
        size += left;                                           ///< input message size need add original message size(only unused)
    } else {
        memcpy((uint8_t *)context->buffer, p_data_in, WJ_SHA_ALG_SELECT_MIN(block_size, size));
    }

    length = size + (msg_length << 2U);                          ///< message size + extra length
    count[0] = (block_size << 3U);                               ///< 512bit or 1024bit

    if (length > block_size) {                                   ///< if length > block size, need accumulate two times least
        wj_sha_mode_en_without_count(sha_base);

        if (size == context->total[0]) {
            wj_sha_mode_dis_initial(sha_base);                   ///< No initialization required for the first time
        }

        do {

            wj_sha_write_count(sha_base, count);

            wj_sha_ctrl_start_cal(sha_base);

            mdelay(1U);                                          ///<! Must be kept, otherwise the data is prone to error

            wj_sha_mode_en_initial(sha_base);

            length -= block_size;
            p_data_in += (block_size - left);                    ///< input address offset
            left = 0U;

            memcpy((uint8_t *)context->buffer, p_data_in, block_size);

            WJ_SHA_WAIT_IS_TIMEOUT(++timecount, ret);

            if (ret != CSI_OK) {
                break;
            }
        } while (length > block_size);

        memset((uint8_t *)context->buffer, 0, block_size);

        memcpy((uint8_t *)context->buffer, p_data_in, (size & (block_size - 1U)));

    }

    sha->state.busy = 0U;

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
    CSI_PARAM_CHK(sha, CSI_ERROR);
    CSI_PARAM_CHK(context, CSI_ERROR);

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
    CSI_PARAM_CHK(sha, CSI_ERROR);
    CSI_PARAM_CHK(context, CSI_ERROR);
    csi_error_t ret = CSI_OK;

    uint8_t *p_data_in = (uint8_t *)context->buffer;
    uint8_t *out_buf = (uint8_t *)output;
    uint32_t i;
    uint32_t msg_length;
    uint32_t block_size;
    uint32_t length;
    uint32_t size = context->total[0];
    uint32_t pad_buf[4] = {0U};
    uint32_t cnt_buf[4] = {0U};
    uint64_t pad_bit_len;
    uint64_t cnt_bit_len;

    wj_sha_regs_t *sha_base = (wj_sha_regs_t *)HANDLE_REG_BASE(sha);

    block_size = WJ_SHA_GET_BOLOCK_SIZE_BYTES(context->mode);
    msg_length = WJ_SHA_GET_MSGLEN_TAIL_4BYTES(context->mode);

    pad_bit_len = (uint64_t)size << 3U;                                    ///< write message length into memory behind message
    cnt_bit_len = ((uint64_t)size % block_size) << 3U;                     ///< write the last block size into count register
    cnt_buf[0] = (block_size << 3U);

    length = size + (msg_length << 2U);                          ///< message size + extra length

    wj_sha_mode_dis_without_count(sha_base);

    /**
     * If message exceed block size, is need enter circle mode to use block size message
     */
    if ((context->total[0] + (msg_length << 2U)) > block_size) {
        wj_sha_mode_en_initial(sha_base);                       ///< cycle calculation (n * block) times, this finish is the last time
    } else {
        wj_sha_mode_dis_initial(sha_base);                      ///< the message less than block size, only one time to calculation
    }

    /**
     * padding length is big endian
     * count length is small endian
    */
    for (i = 0U; i < msg_length; i++) {
        cnt_buf[i] = (uint32_t)cnt_bit_len;
        pad_buf[msg_length - i - 1U] = (uint32_t)pad_bit_len;
        cnt_bit_len >>= 32U;
        pad_bit_len >>= 32U;
    }

    memcpy(p_data_in + WJ_SHA_ALG_ALIGN_4BYTE(size % block_size), (uint8_t *)pad_buf, (msg_length << 2U)); ///< add tail(msg bit length)

    memcpy((uint8_t *)context->buffer, p_data_in, length);      //< import to baseaddr

    wj_sha_write_count(sha_base, cnt_buf);

    wj_sha_ctrl_start_cal(sha_base);

    mdelay(1U);                                                 ///<! Must be kept, otherwise the data is prone to error

    *out_size = WJ_SHA_GET_MSGDIGEST_BYTES(context->mode);

    memcpy(out_buf, context->state, *out_size);

    wj_sha_alg_reverse(out_buf, ((uint8_t)*out_size) >> 2U);   ///< Flip the last result data

    /**
     * clean cache
    */
    memset((uint8_t *)context->total, 0, sizeof(context->total));
    memset((uint8_t *)context->state, 0, sizeof(context->state));
    memset((uint8_t *)context->buffer, 0, sizeof(context->buffer));

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
    CSI_PARAM_CHK(sha, CSI_ERROR);
    CSI_PARAM_CHK(state, CSI_ERROR);

    state = &sha->state;

    return CSI_OK;
}

#ifdef CONFIG_PM
csi_error_t wj_sha_pm_action(csi_dev_t *dev, csi_pm_dev_action_t action)
{
    CSI_PARAM_CHK(dev, CSI_ERROR);

    csi_error_t ret = CSI_OK;
    csi_pm_dev_t *pm_dev = &dev->pm_dev;

    switch (action) {
        case PM_DEV_SUSPEND:
            csi_pm_dev_save_regs(pm_dev->reten_mem, (uint32_t *)dev->reg_base, 25U);
            break;

        case PM_DEV_RESUME:
            csi_pm_dev_restore_regs(pm_dev->reten_mem, (uint32_t *)dev->reg_base, 25U);
            break;

        default:
            ret = CSI_ERROR;
            break;
    }

    return ret;
}

csi_error_t csi_sha_enable_pm(csi_sha_t *sha)
{
    return csi_pm_dev_register(&sha->dev, wj_sha_pm_action, 25U, 0U);
}

void csi_sha_disable_pm(csi_sha_t *sha)
{
    csi_pm_dev_unregister(&sha->dev);
}
#endif

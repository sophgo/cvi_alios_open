/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */

#ifndef CONFIG_SEC_CRYPTO_SHA_SW
#ifdef CONFIG_CSI_V1
#include "sec_mcu.h"
#include "sec_crypto_common.h"

/**
  \brief       Initialize SHA Interface. Initializes the resources needed for the SHA interface
*/
uint32_t sc_sha_init(sc_sha_t *sha, uint32_t idx)
{
    CHECK_PARAM(sha, SC_PARAM_INV);

    sha->handle = csi_sha_initialize(idx, &sha->ctx, NULL);
    if (!sha->handle) {
        return SC_DRV_FAILED;
    }

    return SC_OK;
}

/**
  \brief       De-initialize SHA Interface. stops operation and releases the software resources used by the interface
*/
void sc_sha_uninit(sc_sha_t *sha)
{
    if (sha && sha->handle) {
        csi_sha_uninitialize(sha->handle);
        memset(sha, 0, sizeof(sc_sha_t));
    }
}

/**
  \brief       attach the callback handler to SHA
*/
uint32_t sc_sha_attach_callback(sc_sha_t *sha, void *callback, void *arg)
{
    return SC_NOT_SUPPORT;
}

/**
  \brief       detach the callback handler
*/
void sc_sha_detach_callback(sc_sha_t *sha)
{
    return;
}

/**
  \brief       start the engine
*/
uint32_t sc_sha_start(sc_sha_t *sha, sc_sha_context_t *context, sc_sha_mode_t mode)
{
    uint32_t   ret;
    sha_mode_e m;
    CHECK_PARAM(context, SC_PARAM_INV);
    CHECK_PARAM(sha, SC_PARAM_INV);

    if (mode == SC_SHA_MODE_256) {
        m = SHA_MODE_256;
    } else if (mode == SC_SHA_MODE_224) {
        m = SHA_MODE_224;
    } else if (mode == SC_SHA_MODE_1) {
        m = SHA_MODE_1;
    } else {
        return SC_NOT_SUPPORT;
    }
    csi_sha_config(sha->handle, m, SHA_ENDIAN_MODE_BIG);
    sha->mode = mode;

    ret = csi_sha_start(sha->handle, NULL);
    if (ret) {
        return SC_CRYPT_FAIL;
    }

    return SC_OK;
}

/**
  \brief       update the engine
*/
uint32_t sc_sha_update(sc_sha_t *sha, sc_sha_context_t *context, const void *input, uint32_t size)
{
    uint32_t ret;
    CHECK_PARAM(sha, SC_PARAM_INV);
    CHECK_PARAM(context, SC_PARAM_INV);
    CHECK_PARAM(input, SC_PARAM_INV);

    ret = csi_sha_update(sha->handle, NULL, input, size);
    if (ret) {
        return SC_CRYPT_FAIL;
    }

    return SC_OK;
}

/**
  \brief       accumulate the engine (async mode)
*/
uint32_t sc_sha_update_async(sc_sha_t *sha, sc_sha_context_t *context, const void *input,
                             uint32_t size)
{
    return SC_NOT_SUPPORT;
}

/**
  \brief       finish the engine
*/
uint32_t sc_sha_finish(sc_sha_t *sha, sc_sha_context_t *context, void *output, uint32_t *out_size)
{
    uint32_t ret;
    CHECK_PARAM(sha, SC_PARAM_INV);
    CHECK_PARAM(context, SC_PARAM_INV);
    CHECK_PARAM(output, SC_PARAM_INV);

    ret = csi_sha_finish(sha->handle, NULL, output);
    if (ret) {
        return SC_CRYPT_FAIL;
    }
    if (sha->mode == SC_SHA_MODE_256) {
        *out_size = 32;
    } else if (sha->mode == SC_SHA_MODE_224) {
        *out_size = 28;
    } else if (sha->mode == SC_SHA_MODE_1) {
        *out_size = 20;
    } else {
        return 0;
    }

    return SC_OK;
}

#endif
#endif
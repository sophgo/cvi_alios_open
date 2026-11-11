/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     rambus_rsa.c
 * @brief    CSI Source File for RSA Driver
 * @version  V2.0
 * @date     24. OCT 2020
 ******************************************************************************/
#include <drv/cvi_irq.h>
#include <stdio.h>
#include <string.h>
#include <drv/rsa.h>
#include <drv/common.h>
#include "sfxlpka.h"
#include <drv/rsa.h>
#include "rambus.h"

#define RSA_2048_BYTE_LEN 256
#define SHA256_DIGEST_BYTE_LEN 32
#define RSA_PKCS1_PADDING_SIZE 11

/**
  \brief       Initialize RSA Interface. 1. Initializes the resources needed for the RSA interface 2.registers event callback function
  \param[in]   rsa  rsa handle to operate.
  \param[in]   idx  device id
  \return      \ref csi_error_t
*/
csi_error_t csi_rsa_init(csi_rsa_t *rsa, uint32_t idx)
{

        CHECK_RET_WITH_RET(rsa, SC_PARAM_INV);

        return CSI_OK;
}

/**
  \brief       De-initialize RSA Interface. stops operation and releases the software resources used by the interface
  \param[in]   rsa  rsa handle to operate.
  \return      none
*/
void csi_rsa_uninit(csi_rsa_t *rsa)
{
        CHECK_RET(rsa);
}

/**
  \brief       attach the callback handler to RSA
  \param[in]   rsa  operate handle.
  \param[in]   cb    callback function
  \param[in]   arg   user can define it by himself as callback's param
  \return      error code
*/
csi_error_t csi_rsa_attach_callback(csi_rsa_t *rsa, csi_rsa_callback_t cb,
                                    void *arg)
{
        CHECK_RET_WITH_RET(rsa, SC_PARAM_INV);
        //     rsa->cb = cb;
        //     rsa->arg = arg;
        return CSI_UNSUPPORTED;
}

/**
  \brief       detach the callback handler
  \param[in]   rsa  operate handle.
*/
void csi_rsa_detach_callback(csi_rsa_t *rsa)
{
        CHECK_RET(rsa);
        rsa->cb = NULL;
}

/**
  \brief       generate rsa key pair.
  \param[in]   rsa       rsa handle to operate.
  \param[out]  context   Pointer to the rsa context
  \return      \ref csi_error_t
*/
csi_error_t csi_rsa_gen_key(csi_rsa_t *rsa, csi_rsa_context_t *context)
{
        return CSI_UNSUPPORTED;
}

static int rsaes_pkcs1_enc(unsigned char *to, int tlen,
                           const unsigned char *from, int flen)
{
        int            j;
        unsigned char *p;
        uint32_t       ret;

        if (flen > (tlen - RSA_PKCS1_PADDING_SIZE)) {
                return SC_PARAM_INV;
        }

        p = (unsigned char *)to;

        *(p++) = 0;
        *(p++) = 2; /* Public Key BT (Block Type) */

        /* pad out with non-zero random data */
        j = tlen - 3 - flen;

        ret = rb_get_random_byte(p, j);
        CHECK_RET_WITH_RET(ret == CSI_OK, ret);
        p += j;

        *(p++) = '\0';

        memcpy(p, from, (unsigned int)flen);
        return CSI_OK;
}

static int rsaes_pkcs1_dec(unsigned char *to, uint32_t *tlen,
                           const unsigned char *from, int flen)
{
        int      i;
        int      index = 1;
        int      len   = flen - 1;
        int      padc;
        uint32_t pc;
        uint32_t ret;

        //from[0] != 0)
        if (from[0] != 2) {
                TRACE_ERR(SC_CRYPT_FAIL);
                return SC_CRYPT_FAIL;
        }

        ret = CSI_ERROR;
        for (i = index; i < len; i++) {
                if (from[i] == 0) {
                        ret  = CSI_OK;
                        padc = i - index;
                        break;
                }
        }
        CHECK_RET_WITH_RET(ret == CSI_OK, SC_CRYPT_FAIL);
        CHECK_RET_WITH_RET(padc >= 8, SC_CRYPT_FAIL);

        pc = len - padc - 3 + index;
        CHECK_RET_WITH_RET(pc > 0, SC_CRYPT_FAIL);
        memcpy(to, from + 2 - index + padc + 1, pc);

        *tlen = pc;

        return CSI_OK;
}

int rsa_encrypt_async(csi_rsa_t *rsa, uint8_t *n, uint8_t *e, uint8_t *src,
                      uint32_t src_size, uint8_t *out, uint32_t padding,
                      uint32_t keybits_len)
{
        uint32_t ret = CSI_UNSUPPORTED;
        return ret;
}

/* 0: suc, other fail */
static int rsa_encrypt(csi_rsa_t *rsa, uint8_t *n, uint8_t *e, uint8_t *src,
                       uint32_t src_size, uint8_t *out, uint32_t type,
                       uint32_t keybits_len)
{
        uint32_t                ret;
        SfxlPKA_ModExp_Params_t pb;
        uint8_t                 em[RSA_2048_BYTE_LEN]   = {0};
        uint8_t                 em_t[RSA_2048_BYTE_LEN] = {0};

        ret = rsaes_pkcs1_enc(em, RSA_2048_BYTE_LEN, src, src_size);
        CHECK_RET_WITH_RET(ret == CSI_OK, CSI_ERROR);
        pb.Exponent.Word_p = (uint32_t *)e;

        pb.Exponent.WordCount = RSA_2048_BYTE_LEN / sizeof(uint32_t);
        pb.Modulus.Word_p     = (uint32_t *)n;
        pb.Modulus.WordCount  = RSA_2048_BYTE_LEN / sizeof(uint32_t);
        pb.Base.Word_p        = (uint32_t *)em;
        pb.Base.WordCount     = RSA_2048_BYTE_LEN / sizeof(uint32_t);
        /* ret */
        pb.Result.Word_p    = (uint32_t *)em_t;
        pb.Result.WordCount = RSA_2048_BYTE_LEN / sizeof(uint32_t);
        pb.Extra            = 1;
        Log_HexDump("base", 0, (uint8_t *)pb.Base.Word_p,
                    pb.Base.WordCount * sizeof(uint32_t));
        Log_HexDump("exp", 0, (uint8_t *)pb.Exponent.Word_p,
                    pb.Exponent.WordCount * sizeof(uint32_t));
        Log_HexDump("mod", 0, (uint8_t *)pb.Modulus.Word_p,
                    pb.Modulus.WordCount * sizeof(uint32_t));
        ret = SfxlPKA_ModExp(0, &pb);
        if (ret != SFXL_PKA_NO_ERROR) {
                TRACE_ERR(ret);
                return ret;
        }
        memcpy(out, em_t, RSA_2048_BYTE_LEN);
        Log_HexDump("em", 0, em, 256);
        Log_HexDump("em_t", 0, em_t, 256);

        return ret;
}

/**
  \brief       encrypt
  \param[in]   rsa       rsa handle to operate.
  \param[in]   context   Pointer to the rsa context
  \param[in]   src       Pointer to the source data.
  \param[in]   src_size  the source data len
  \param[out]  out       Pointer to the result buffer
  \return      \ref csi_error_t
*/

csi_error_t csi_rsa_encrypt(csi_rsa_t *rsa, csi_rsa_context_t *context,
                            void *src, uint32_t src_size, void *out)
{
        csi_error_t ret        = CSI_OK;
        uint32_t    bit_length = 2048;
        CHECK_RET_WITH_RET(rsa, SC_PARAM_INV);
        CHECK_RET_WITH_RET(context, SC_PARAM_INV);
        CHECK_RET_WITH_RET(context->n, SC_PARAM_INV);
        CHECK_RET_WITH_RET(context->e, SC_PARAM_INV);
        CHECK_RET_WITH_RET(context->padding_type == RSA_PADDING_MODE_PKCS1,
                           SC_PARAM_INV);
        CHECK_RET_WITH_RET(context->key_bits == RSA_KEY_BITS_2048,
                           SC_PARAM_INV);
        CHECK_RET_WITH_RET(src, SC_PARAM_INV);
        CHECK_RET_WITH_RET(out, SC_PARAM_INV);
        CHECK_RET_WITH_RET(src_size > 0, SC_PARAM_INV);

        rsa->state.busy = 1U;
        ret = rsa_encrypt(rsa, (uint8_t *)context->n, (uint8_t *)context->e,
                          (uint8_t *)src, (uint32_t)src_size, (uint8_t *)out,
                          (uint32_t)(context->padding_type), bit_length);
        rsa->state.busy = 0U;

        return ret;
}

int rsa_decrypt(csi_rsa_t *rsa, uint8_t *n, uint8_t *d, uint8_t *src,
                uint32_t src_size, uint8_t *out, uint32_t *out_size,
                uint32_t padding, uint32_t keybits_len)
{
        uint32_t                ret;
        SfxlPKA_ModExp_Params_t pb;
        uint8_t                 em_t[RSA_2048_BYTE_LEN] = {0};

        pb.Exponent.Word_p = (uint32_t *)d;

        pb.Exponent.WordCount = RSA_2048_BYTE_LEN / sizeof(uint32_t);
        pb.Modulus.Word_p     = (uint32_t *)n;
        pb.Modulus.WordCount  = RSA_2048_BYTE_LEN / sizeof(uint32_t);
        pb.Base.Word_p        = (uint32_t *)src;
        pb.Base.WordCount     = RSA_2048_BYTE_LEN / sizeof(uint32_t);
        /* ret */
        pb.Result.Word_p    = (uint32_t *)em_t;
        pb.Result.WordCount = RSA_2048_BYTE_LEN / sizeof(uint32_t);
        pb.Extra            = 1;
        Log_HexDump("base", 0, (uint8_t *)pb.Base.Word_p,
                    pb.Base.WordCount * sizeof(uint32_t));
        Log_HexDump("exp", 0, (uint8_t *)pb.Exponent.Word_p,
                    pb.Exponent.WordCount * sizeof(uint32_t));
        Log_HexDump("mod", 0, (uint8_t *)pb.Modulus.Word_p,
                    pb.Modulus.WordCount * sizeof(uint32_t));
        ret = SfxlPKA_ModExp(0, &pb);
        if (ret != SFXL_PKA_NO_ERROR) {
                TRACE_ERR(ret);
                return ret;
        }
        Log_HexDump("em_t", 0, em_t, 256);
        ret = rsaes_pkcs1_dec(out, out_size, em_t, RSA_2048_BYTE_LEN);
        Log_HexDump("out", 0, out, *out_size);
        CHECK_RET_WITH_RET(ret == CSI_OK, ret);

        return ret;
}

/**
  \brief       decrypt
  \param[in]   rsa       rsa handle to operate.
  \param[in]   context   Pointer to the rsa context
  \param[in]   src       Pointer to the source data.
  \param[in]   src_size  the source data len
  \param[out]  out       Pointer to the result buffer
  \param[out]  out_size  the result size
  \return      \ref csi_error_t
*/
csi_error_t csi_rsa_decrypt(csi_rsa_t *rsa, csi_rsa_context_t *context,
                            void *src, uint32_t src_size, void *out,
                            uint32_t *out_size)
{
        csi_error_t ret        = CSI_OK;
        uint32_t    bit_length = 2048;
        CHECK_RET_WITH_RET(rsa, SC_PARAM_INV);
        CHECK_RET_WITH_RET(context, SC_PARAM_INV);
        CHECK_RET_WITH_RET(context->n, SC_PARAM_INV);
        CHECK_RET_WITH_RET(context->d, SC_PARAM_INV);
        CHECK_RET_WITH_RET(context->padding_type == RSA_PADDING_MODE_PKCS1,
                           SC_PARAM_INV);
        CHECK_RET_WITH_RET(context->key_bits == RSA_KEY_BITS_2048,
                           SC_PARAM_INV);
        CHECK_RET_WITH_RET(src, SC_PARAM_INV);
        CHECK_RET_WITH_RET(out, SC_PARAM_INV);
        CHECK_RET_WITH_RET(out_size, SC_PARAM_INV);
        CHECK_RET_WITH_RET(src_size > 0, SC_PARAM_INV);

        rsa->state.busy = 1U;

        ret = rsa_decrypt(rsa, (uint8_t *)context->n, (uint8_t *)context->d,
                          (uint8_t *)src, (uint32_t)src_size, (uint8_t *)out,
                          (uint32_t *)out_size,
                          (uint32_t)(context->padding_type), bit_length);
        rsa->state.busy = 0U;

        return ret;
}

static int rsassa_pkcs1(unsigned char *to, int tlen, const unsigned char *from,
                        int flen)
{
        int            j;
        unsigned char *p;

        if (flen > (tlen - RSA_PKCS1_PADDING_SIZE)) {
                return SC_PARAM_INV;
        }

        p = (unsigned char *)to;

        *(p++) = 0;
        *(p++) = 1;
        j      = tlen - 3 - flen;
        memset(p, 0xff, j);
        p += j;
        *(p++) = '\0';
        memcpy(p, from, (unsigned int)flen);
        return 0;
}

/* 0: suc, other fail */
static int rsa_verify(csi_rsa_t *rsa, uint8_t *n, uint8_t *e, uint8_t *src,
                      uint32_t src_size, uint8_t *signature, uint32_t sig_size,
                      uint32_t type, uint32_t keybits_len, uint8_t *result)
{
        uint32_t                ret;
        SfxlPKA_ModExp_Params_t pb;
        uint8_t                 em[RSA_2048_BYTE_LEN]   = {0};
        uint8_t                 em_t[RSA_2048_BYTE_LEN] = {0};

        ret = rsassa_pkcs1(em, RSA_2048_BYTE_LEN, src, src_size);
        CHECK_RET_WITH_RET(ret == CSI_OK, ret);
        pb.Exponent.Word_p = (uint32_t *)e;

        pb.Exponent.WordCount = RSA_2048_BYTE_LEN / sizeof(uint32_t);
        pb.Modulus.Word_p     = (uint32_t *)n;
        pb.Modulus.WordCount  = RSA_2048_BYTE_LEN / sizeof(uint32_t);
        pb.Base.Word_p        = (uint32_t *)signature;
        pb.Base.WordCount     = RSA_2048_BYTE_LEN / sizeof(uint32_t);
        /* ret */
        pb.Result.Word_p    = (uint32_t *)em_t;
        pb.Result.WordCount = RSA_2048_BYTE_LEN / sizeof(uint32_t);
        pb.Extra            = 1;
        Log_HexDump("base", 0, (uint8_t *)pb.Base.Word_p,
                    pb.Base.WordCount * sizeof(uint32_t));
        Log_HexDump("exp", 0, (uint8_t *)pb.Exponent.Word_p,
                    pb.Exponent.WordCount * sizeof(uint32_t));
        Log_HexDump("mod", 0, (uint8_t *)pb.Modulus.Word_p,
                    pb.Modulus.WordCount * sizeof(uint32_t));
        ret = SfxlPKA_ModExp(0, &pb);
        if (ret != SFXL_PKA_NO_ERROR) {
                TRACE_ERR(ret);
                ret = SC_DRV_FAILED;
                return ret;
        }
        if (em_t[0] == 01) {
                ret = memcmp(em_t, em + 1, RSA_2048_BYTE_LEN - 1);
        } else {
                ret = memcmp(em_t, em, RSA_2048_BYTE_LEN);
        }
        Log_HexDump("em", 0, em, 256);
        Log_HexDump("em_t", 0, em_t, 256);

        return ret;
}

int rsa_sign(csi_rsa_t *rsa, uint8_t *n, uint8_t *d, uint8_t *src,
             uint32_t src_size, uint8_t *signature, uint32_t type,
             uint32_t keybits_len)
{
        uint32_t                ret;
        SfxlPKA_ModExp_Params_t pb;
        uint8_t                 m[RSA_2048_BYTE_LEN] = {0};
        ret = rsassa_pkcs1(m, RSA_2048_BYTE_LEN, src, src_size);
        CHECK_RET_WITH_RET(ret == CSI_OK, ret);

        pb.Exponent.Word_p    = (uint32_t *)d;
        pb.Exponent.WordCount = RSA_2048_BYTE_LEN / sizeof(uint32_t);
        pb.Modulus.Word_p     = (uint32_t *)n;
        pb.Modulus.WordCount  = RSA_2048_BYTE_LEN / sizeof(uint32_t);
        pb.Base.Word_p        = (uint32_t *)m;
        pb.Base.WordCount     = RSA_2048_BYTE_LEN / sizeof(uint32_t);
        /* ret */
        pb.Result.Word_p    = (uint32_t *)signature;
        pb.Result.WordCount = RSA_2048_BYTE_LEN / sizeof(uint32_t);
        pb.Extra            = 4;
        Log_HexDump("base", 0, (uint8_t *)pb.Base.Word_p,
                    pb.Base.WordCount * sizeof(uint32_t));
        Log_HexDump("exp", 0, (uint8_t *)pb.Exponent.Word_p,
                    pb.Exponent.WordCount * sizeof(uint32_t));
        Log_HexDump("mod", 0, (uint8_t *)pb.Modulus.Word_p,
                    pb.Modulus.WordCount * sizeof(uint32_t));
        ret = SfxlPKA_ModExp(0, &pb);
        if (ret != SFXL_PKA_NO_ERROR) {
                TRACE_ERR(ret);
                ret = SC_DRV_FAILED;
        }
        Log_HexDump("ret", 0, (uint8_t *)pb.Result.Word_p,
                    pb.Result.WordCount * sizeof(uint32_t));

        return ret;
}

/**
  \brief       rsa sign
  \param[in]   rsa       rsa handle to operate.
  \param[in]   context   Pointer to the rsa context
  \param[in]   src       Pointer to the source data.
  \param[in]   src_size  the source data len
  \param[out]  signature Pointer to the signature
  \param[in]   hash_type the source data hash type
  \return      \ref csi_error_t
*/
csi_error_t csi_rsa_sign(csi_rsa_t *rsa, csi_rsa_context_t *context, void *src,
                         uint32_t src_size, void *signature,
                         csi_rsa_hash_type_t hash_type)
{
        csi_error_t ret        = CSI_OK;
        uint32_t    bit_length = 2048;
        CHECK_RET_WITH_RET(rsa, SC_PARAM_INV);
        CHECK_RET_WITH_RET(context, SC_PARAM_INV);
        CHECK_RET_WITH_RET(context->n, SC_PARAM_INV);
        CHECK_RET_WITH_RET(context->d, SC_PARAM_INV);
        CHECK_RET_WITH_RET(src, SC_PARAM_INV);
        CHECK_RET_WITH_RET(signature, SC_PARAM_INV);
        CHECK_RET_WITH_RET(src_size > 0, SC_PARAM_INV);
        CHECK_RET_WITH_RET(hash_type == RSA_HASH_TYPE_SHA256, SC_PARAM_INV);
        CHECK_RET_WITH_RET(context->key_bits == RSA_KEY_BITS_2048,
                           SC_PARAM_INV);

        rsa->state.busy = 1U;

        ret = rsa_sign(rsa, (uint8_t *)context->n, (uint8_t *)context->d,
                       (uint8_t *)src, (uint32_t)src_size, (uint8_t *)signature,
                       (uint32_t)(hash_type), bit_length);
        rsa->state.busy = 0U;
        if (ret) {
                TRACE_ERR(ret);
                return ret;
        }

        return ret;
}

int rsa_verify_asy(csi_rsa_t *rsa, uint8_t *n, uint8_t *e, uint8_t *src,
                   uint32_t src_size, uint8_t *signature, uint32_t sig_size,
                   uint32_t type, uint32_t keybits_len)
{
        uint32_t ret = CSI_UNSUPPORTED;
        return ret;
}
/**
  \brief       rsa verify
  \param[in]   rsa       rsa handle to operate.
  \param[in]   context   Pointer to the rsa context
  \param[in]   src       Pointer to the source data.
  \param[in]   src_size  the source data len
  \param[in]   signature Pointer to the signature
  \param[in]   sig_size  the signature size
  \param[in]   hash_type the source data hash type
  \return      verify result
*/
bool csi_rsa_verify(csi_rsa_t *rsa, csi_rsa_context_t *context, void *src,
                    uint32_t src_size, void *signature, uint32_t sig_size,
                    csi_rsa_hash_type_t hash_type)
{
        // csi_rsa_callback_t  cb_func;
        bool     result;
        uint32_t ret;
        uint32_t bit_length = 2048;
        CHECK_RET_WITH_RET(rsa, false);
        CHECK_RET_WITH_RET(context, false);
        CHECK_RET_WITH_RET(context->n, false);
        CHECK_RET_WITH_RET(context->e, false);
        CHECK_RET_WITH_RET(context->padding_type == RSA_PADDING_MODE_PKCS1,
                           false);
        CHECK_RET_WITH_RET(src, false);
        CHECK_RET_WITH_RET(signature, false);
        CHECK_RET_WITH_RET(src_size > 0, false);
        CHECK_RET_WITH_RET(hash_type == RSA_HASH_TYPE_SHA256, false);
        CHECK_RET_WITH_RET(context->key_bits == RSA_KEY_BITS_2048, false);

        rsa->state.busy = 1U;
        ret = rsa_verify(
            rsa, (uint8_t *)context->n, (uint8_t *)context->e, (uint8_t *)src,
            (uint32_t)src_size, (uint8_t *)signature, sig_size,
            (uint32_t)(context->padding_type), bit_length, (uint8_t *)&result);
        rsa->state.busy = 0U;
        if (ret) {
                TRACE_ERR(ret);
                return false;
        }
        result = true;
        return result;
}

/**
  \brief       encrypt(async mode)
  \param[in]   rsa       rsa handle to operate.
  \param[in]   context   Pointer to the rsa context
  \param[in]   src       Pointer to the source data.
  \param[in]   src_size  the source data len
  \param[out]  out       Pointer to the result buffer
  \return      \ref csi_error_t
*/
csi_error_t csi_rsa_encrypt_async(csi_rsa_t *rsa, csi_rsa_context_t *context,
                                  void *src, uint32_t src_size, void *out)
{
        csi_error_t ret = CSI_UNSUPPORTED;
        return ret;
}

/**
  \brief       decrypt(async mode)
  \param[in]   rsa       rsa handle to operate.
  \param[in]   context   Pointer to the rsa context
  \param[in]   src       Pointer to the source data.
  \param[in]   src_size  the source data len
  \param[out]  out       Pointer to the result buffer
  \param[out]  out_size  the result size
  \return      \ref csi_error_t
*/
csi_error_t csi_rsa_decrypt_async(csi_rsa_t *rsa, csi_rsa_context_t *context,
                                  void *src, uint32_t src_size, void *out,
                                  uint32_t *out_size)
{
        csi_error_t ret = CSI_UNSUPPORTED;

        return ret;
}

/**
  \brief       rsa sign(async mode)
  \param[in]   rsa       rsa handle to operate.
  \param[in]   context   Pointer to the rsa context
  \param[in]   src       Pointer to the source data.
  \param[in]   src_size  the source data len
  \param[out]  signature Pointer to the signature
  \param[in]   hash_type the source data hash type
  \return      \ref csi_error_t
*/
csi_error_t csi_rsa_sign_async(csi_rsa_t *rsa, csi_rsa_context_t *context,
                               void *src, uint32_t src_size, void *signature,
                               csi_rsa_hash_type_t hash_type)
{
        csi_error_t ret = CSI_UNSUPPORTED;

        return ret;
}

/**
  \brief       rsa verify(async mode)
  \param[in]   rsa       rsa handle to operate.
  \param[in]   context   Pointer to the rsa context
  \param[in]   src       Pointer to the source data.
  \param[in]   src_size  the source data len
  \param[in]   signature Pointer to the signature
  \param[in]   sig_size  the signature size
  \param[in]   hash_type the source data hash type
  \return      verify result
*/
csi_error_t csi_rsa_verify_async(csi_rsa_t *rsa, csi_rsa_context_t *context,
                                 void *src, uint32_t src_size, void *signature,
                                 uint32_t            sig_size,
                                 csi_rsa_hash_type_t hash_type)
{

        return CSI_UNSUPPORTED;
}

/**
  \brief       Get RSA state.
  \param[in]   rsa      rsa handle to operate.
  \param[out]  state    rsa state \ref csi_rsa_state_t.
  \return      \ref csi_error_t
*/
csi_error_t csi_rsa_get_state(csi_rsa_t *rsa, csi_rsa_state_t *state)
{
        CHECK_RET_WITH_RET(rsa, SC_PARAM_INV);
        *state = rsa->state;
        return CSI_OK;
}

/**
  \brief       Get big prime data
  \param[in]   rsa          rsa handle to operate.
  \param[in]   p            Pointer to the prime
  \param[in]   bit_length   Pointer to the prime bit length
  \return      \ref csi_error_t
*/
csi_error_t csi_rsa_get_prime(csi_rsa_t *rsa, void *p, uint32_t bit_length)
{
        return CSI_UNSUPPORTED;
}

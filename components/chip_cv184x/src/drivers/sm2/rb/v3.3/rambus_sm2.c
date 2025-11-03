/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     rambus_sm2.c
 * @brief    CSI Source File for SM2 Driver
 * @version  V2.0
 * @date     10. Dec 2020
 ******************************************************************************/
#include <drv/cvi_irq.h>
#include <stdio.h>
#include <string.h>
#include <drv/sm2.h>
#include <drv/common.h>
#include "sfxlpka.h"
#include "rambus.h"
#include "rambus_eip_sm2.h"

/**
  \brief       Initialize SM2.
  \param[in]   idx  device id
  \return      \ref uint32_t
*/
csi_error_t csi_sm2_init(csi_sm2_t *sm2, uint32_t idx)
{
        CHECK_RET_WITH_RET(sm2, SC_PARAM_INV);

        csi_error_t ret = CSI_OK;
        if (0 == target_get(DEV_RAMBUS_150B_PKA_TAG, idx, &sm2->dev)) {
                ret = CSI_OK;
        } else {
                ret = CSI_ERROR;
        }

        return ret;
}

/**
  \brief       De-initialize SM2 Interface. stops operation and releases the software resources used by the interface
  \param[in]   sm2  sm2 handle to operate.
  \return      none
*/
void csi_sm2_uninit(csi_sm2_t *sm2)
{
        CHECK_RET(sm2);
}

/**
  \brief       sm2 get capability.
  \param[in]   sm2  Operate handle.
  \return      \ref uint32_t
*/
csi_error_t csi_sm2_config(csi_sm2_t *sm2, sm2_cipher_order_e co,
                           sm2_endian_mode_e endian)
{
        CHECK_RET_WITH_RET(sm2, SC_PARAM_INV);
        return CSI_OK;
}

/**
  \brief       Attach the callback handler to SM2
  \param[in]   sm2  Operate handle.
  \param[in]   cb    Callback function
  \param[in]   arg   User can define it by himself as callback's param
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_sm2_attach_callback(csi_sm2_t *sm2, csi_sm2_callback_t cb,
                                    void *arg)
{
        return CSI_UNSUPPORTED;
}

/**
  \brief       Detach the callback handler
  \param[in]   sm2  Operate handle.
*/
void csi_sm2_detach_callback(csi_sm2_t *sm2)
{
}

/**
  \brief       sm2 get capability.
*/
csi_error_t csi_sm2_get_capabilities(csi_sm2_t *sm2, sm2_capabilities_t *cap)
{
        cap->sm2_curve = 1;
        return CSI_OK;
}

csi_error_t csi_sm2_check_keypair(csi_sm2_t *sm2, uint8_t pubkey[65],
                                  uint8_t prikey[32])
{
        return CSI_UNSUPPORTED;
}

/**
  \brief       generate sm2 key.
  \param[in]   sm2       sm2 handle to operate.
  \param[out]  private   Pointer to the sm2 private key, alloc by caller.
  \param[out]  public   Pointer to the sm2 public key, alloc by caller.
  \return      \ref uint32_t
*/
csi_error_t csi_sm2_gen_key(csi_sm2_t *sm2, uint8_t pubkey[65],
                            uint8_t prikey[32])
{
        return CSI_UNSUPPORTED;
}

#ifdef CONFIG_RB_SM2_RK
static uint8_t rk[32] = {
    0x6C, 0xB2, 0x8D, 0x99, 0x38, 0x5C, 0x17, 0x5C, 0x94, 0xF9, 0x4E,
    0x93, 0x48, 0x17, 0x66, 0x3F, 0xC1, 0x76, 0xD9, 0x25, 0xDD, 0x72,
    0xB7, 0x27, 0x26, 0x0D, 0xBA, 0xAE, 0x1F, 0xB2, 0xF9, 0x6F,
};
#endif

/**
  \brief       sm2 sign
  \param[in]   sm2       sm2 handle to operate.
  \param[in]   d       Pointer to the digest.
  \param[out]  privkey Pointer to the private key
  \param[out]  s Pointer to the signature
  \return      \ref uint32_t
*/
csi_error_t csi_sm2_sign(csi_sm2_t *sm2, uint8_t d[32], uint8_t prikey[32],
                         uint8_t s[64])
{
        CHECK_RET_WITH_RET(sm2, SC_PARAM_INV);
        CHECK_RET_WITH_RET(prikey, SC_PARAM_INV);
        CHECK_RET_WITH_RET(d, SC_PARAM_INV);
        CHECK_RET_WITH_RET(s, SC_PARAM_INV);
        uint32_t ret;

        SfxlPKA_Vector_t priv; // input
        SfxlPKA_Vector_t vd;   // input
        SfxlPKA_Vector_t k;    // input
        SfxlPKA_Vector_t r;    // output
#ifndef CONFIG_RB_SM2_RK
        uint8_t rk[32];
        ret = rb_get_random_byte(rk, 32);
#endif
        CHECK_RET_WITH_RET(ret == CSI_OK, ret);
        priv.Word_p    = (uint32_t *)prikey;
        priv.WordCount = 32 / sizeof(uint32_t);
        vd.Word_p      = (uint32_t *)d;
        vd.WordCount   = 32 / sizeof(uint32_t);
        k.Word_p       = (uint32_t *)rk;
        k.WordCount    = 32 / sizeof(uint32_t);
        r.Word_p       = (uint32_t *)s;
        r.WordCount    = 64 / sizeof(uint32_t);
        Log_HexDump("priv", 0, (uint8_t *)priv.Word_p,
                    priv.WordCount * sizeof(uint32_t));
        Log_HexDump("d", 0, (uint8_t *)vd.Word_p,
                    vd.WordCount * sizeof(uint32_t));
        Log_HexDump("k", 0, (uint8_t *)k.Word_p,
                    k.WordCount * sizeof(uint32_t));

        ret = eip_sm2_sign(sm2->dev.reg_base, &priv, &vd, &k, &r);
        if (ret != EIP28_STATUS_OK) {
                TRACE_ERR(ret);
                ret = SC_DRV_FAILED;
                return SC_DRV_FAILED;
        }
        Log_HexDump("r", 0, (uint8_t *)r.Word_p,
                    r.WordCount * sizeof(uint32_t));
        if (ret != EIP28_STATUS_OK) {
                TRACE_ERR(SC_DRV_FAILED);
                return SC_DRV_FAILED;
        }
        return CSI_OK;
}

/**
  \brief       sm2 sign
  \param[in]   sm2       sm2 handle to operate.
  \param[in]   d       Pointer to the digest.
  \param[out]  privkey Pointer to the private key
  \param[out]  s Pointer to the signature
  \return      \ref uint32_t
*/
csi_error_t csi_sm2_sign_async(csi_sm2_t *sm2, uint8_t d[32],
                               uint8_t prikey[32], uint8_t s[64])
{
        return CSI_UNSUPPORTED;
}

/* TODO */
/**
  \brief       sm2 verify
  \param[in]   sm2       sm2 handle to operate.
  \param[in]   d       Pointer to the digest.
  \param[out]  privkey Pointer to the private key
  \param[out]  s Pointer to the signature
  \return      verify result
*/
bool csi_sm2_verify(csi_sm2_t *sm2, uint8_t d[32], uint8_t pubkey[65],
                    uint8_t s[64])
{
        uint32_t ret;
        uint8_t  pub[64];
        CHECK_RET_WITH_RET(sm2, false);
        CHECK_RET_WITH_RET(pubkey[0] == 0x04, false);
        CHECK_RET_WITH_RET(d, false);
        CHECK_RET_WITH_RET(s, false);

        SfxlPKA_Vector_t vpub;
        SfxlPKA_Vector_t vd; // e
        SfxlPKA_Vector_t vs;
        memcpy(pub, pubkey + 1, 64);
        vpub.Word_p    = (uint32_t *)pub;
        vpub.WordCount = (64) / sizeof(uint32_t);
        vd.Word_p      = (uint32_t *)d;
        vd.WordCount   = 32 / sizeof(uint32_t);

        vs.Word_p    = (uint32_t *)s;
        vs.WordCount = (64) / sizeof(uint32_t);
        Log_HexDump("pub", 0, (uint8_t *)vpub.Word_p,
                    vpub.WordCount * sizeof(uint32_t));
        Log_HexDump("d", 0, (uint8_t *)vd.Word_p,
                    vd.WordCount * sizeof(uint32_t));
        Log_HexDump("s", 0, (uint8_t *)vs.Word_p,
                    vs.WordCount * sizeof(uint32_t));

        ret = eip_sm2_verify(sm2->dev.reg_base, &vpub, &vd, &vs);
        if (ret != EIP28_STATUS_OK) {
                TRACE_ERR(ret);
                return false;
        }

        return true;
}

/**
  \brief       sm2 verify
  \param[in]   sm2       sm2 handle to operate.
  \param[in]   d       Pointer to the digest.
  \param[out]  privkey Pointer to the private key
  \param[out]  s Pointer to the signature
  \return      verify result
*/
bool csi_sm2_verify_async(csi_sm2_t *sm2, uint8_t d[32], uint8_t pubkey[65],
                          uint8_t s[64])
{
        return CSI_UNSUPPORTED;
}

/**
  \brief       sm2 encrypto
  \param[in]   sm2       sm2 handle to operate.
  \param[in]   Plain       Pointer to the plaintext.
  \param[in]  PlainByteLen plaintext len
  \param[in]  pubKey public key.
  \param[out]  Cipher Pointer to the chipher
  \param[out]  CipherByteLen Pointer to the chipher len.
  \return      uint32_t
*/
csi_error_t csi_sm2_encrypt(csi_sm2_t *sm2, uint8_t *Plain,
                            uint32_t PlainByteLen, uint8_t pubKey[65],
                            uint8_t *Cipher, uint32_t *CipherByteLen)
{
        return CSI_UNSUPPORTED;
}

/**
  \brief       sm2 encrypto
  \param[in]   sm2       sm2 handle to operate.
  \param[in]  Cipher Pointer to the chipher
  \param[in]  CipherByteLen chipher len.
  \param[in]  prikey private key.
  \param[out]   Plain       Pointer to the plaintext.
  \param[out]  PlainByteLen plaintext len
  \return      uint32_t
*/
csi_error_t csi_sm2_decrypt(csi_sm2_t *sm2, uint8_t *Cipher,
                            uint32_t CipherByteLen, uint8_t prikey[32],
                            uint8_t *Plain, uint32_t *PlainByteLen)
{
        return CSI_UNSUPPORTED;
}

/**
  \brief       sm2 key exchange
  \param[in]   sm2       sm2 handle to operate.
  \return      uint32_t
*/
csi_error_t csi_sm2_exchangekey(csi_sm2_t *sm2, sm2_exchange_role_e role,
                                uint8_t *dA, uint8_t *PB, uint8_t *rA,
                                uint8_t *RA, uint8_t *RB, uint8_t *ZA,
                                uint8_t *ZB, uint32_t kByteLen, uint8_t *KA,
                                uint8_t *S1, uint8_t *SA)
{
        return CSI_UNSUPPORTED;
}

/**
  \brief       sm2 key exchange get Z.
  \param[in]   sm2       sm2 handle to operate.
  \return      uint32_t
*/
csi_error_t csi_sm2_getZ(csi_sm2_t *sm2, uint8_t *ID, uint32_t byteLenofID,
                         uint8_t pubKey[65], uint8_t Z[32])
{
        return CSI_UNSUPPORTED;
}

/**
  \brief       sm2 key exchange get E
  \param[in]   sm2       sm2 handle to operate.
  \return      uint32_t
*/
csi_error_t csi_sm2_getE(csi_sm2_t *sm2, uint8_t *M, uint32_t byteLen,
                         uint8_t Z[32], uint8_t E[32])
{
        return CSI_UNSUPPORTED;
}

/**
  \brief       Get SM2 state.
  \param[in]   sm2      SM2 handle to operate.
  \param[out]  state    SM2 state \ref csi_sm2_state_t.
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_sm2_get_state(csi_sm2_t *sm2, csi_sm2_state_t *state)
{
        return CSI_UNSUPPORTED;
}

/**
  \brief       Enable sm2 power manage
  \param[in]   sm2  SM2 handle to operate.
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_sm2_enable_pm(csi_sm2_t *sm2)
{
        return CSI_UNSUPPORTED;
}

/**
  \brief       Disable sm2 power manage
  \param[in]   sm2  SM2 handle to operate.
*/
void csi_sm2_disable_pm(csi_sm2_t *sm2)
{
}
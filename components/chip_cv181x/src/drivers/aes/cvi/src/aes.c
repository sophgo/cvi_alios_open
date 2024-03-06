#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <csi_core.h>

#include "drv/common.h"
#include "csi_crypto.h"
#include "mmio.h"
#include "drv/aes.h"
#include "drv/tick.h"

#include <aos/kernel.h>

static char encrypt_key[32] = { 0 };
static int encrypt_key_len;
static char decrypt_key[32] = { 0 };
static int decrypt_key_len;
/**
  \brief       Initialize AES interface. Initializes the resources needed for the AES interface
  \param[in]   aes    Handle to operate
  \param[in]   idx    Device id
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_aes_init(csi_aes_t *aes, uint32_t idx)
{
    CSI_PARAM_CHK(aes, CSI_ERROR);

    return CSI_OK;
}

/**
  \brief       De-initialize AES interface. Stops operation and releases the software resources used by the interface
  \param[in]   aes    Dandle to operate
  \return      None
*/
void csi_aes_uninit(csi_aes_t *aes)
{

}

/**
  \brief       Set encrypt key
  \param[in]   aes        Handle to operate
  \param[in]   key        Pointer to the key buf
  \param[in]   key_len    Pointer to \ref csi_aes_key_bits_t
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_aes_set_encrypt_key(csi_aes_t *aes, void *key, csi_aes_key_bits_t key_len)
{
    CSI_PARAM_CHK(aes, CSI_ERROR);
    CSI_PARAM_CHK(key, CSI_ERROR);
      
    csi_error_t ret = CSI_OK;

    switch (key_len) {
        case AES_KEY_LEN_BITS_128:
            memcpy(encrypt_key, key, 16);
            encrypt_key_len = 16;
            break;

        case AES_KEY_LEN_BITS_192:
            memcpy(encrypt_key, key, 24);
            encrypt_key_len = 24;
            break;

        case AES_KEY_LEN_BITS_256:
            memcpy(encrypt_key, key, 32);
            encrypt_key_len = 32;
            break;

        default:
            ret = CSI_ERROR;
            break;
    } 

    return ret;
}

/**
  \brief       Set decrypt key
  \param[in]   aes        Handle to operate
  \param[in]   key        Pointer to the key buf
  \param[in]   key_len    Pointer to \ref csi_aes_key_bits_t
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_aes_set_decrypt_key(csi_aes_t *aes, void *key, csi_aes_key_bits_t key_len)
{
    CSI_PARAM_CHK(aes, CSI_ERROR);
    CSI_PARAM_CHK(key, CSI_ERROR);

    csi_error_t ret = CSI_OK;

    switch (key_len) {
        case AES_KEY_LEN_BITS_128:
            memcpy(decrypt_key, key, 16);
            decrypt_key_len = 16;
            break;

        case AES_KEY_LEN_BITS_192:
            memcpy(decrypt_key, key, 24);
            decrypt_key_len = 24;
            break;

        case AES_KEY_LEN_BITS_256:
            memcpy(decrypt_key, key, 32);
            decrypt_key_len = 32;
            break;

        default:
            ret = CSI_ERROR;
            break;
    } 

    return ret;
}

/**
  \brief       AES ecb encrypt
  \param[in]   aes     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_aes_ecb_encrypt(csi_aes_t *aes, void *in, void *out, uint32_t size)
{
    CSI_PARAM_CHK(aes, CSI_ERROR);
    CSI_PARAM_CHK(in, CSI_ERROR);
    CSI_PARAM_CHK(out, CSI_ERROR);
    if (size & 0xF || size == 0) return CSI_ERROR;

    __aligned(64) uint32_t dma_descriptor[32] = { 0 };
    uint32_t status;
    uint32_t padding_size = (size + 64 - 1) & (~0x3F);
    uint8_t *in_ex = (uint8_t *)aos_malloc(padding_size + 63);
    if (in_ex == NULL) {
      return CSI_ERROR;
    }

    uint8_t *out_ex = (uint8_t *)aos_malloc(padding_size + 63);
    if (out_ex == NULL) {
      aos_free(in_ex);
      return CSI_ERROR;
    }

    uint64_t src = ((uintptr_t)in_ex + 63) & (~0x3F);
    uint64_t dst = ((uintptr_t)out_ex + 63) & (~0x3F);

    memcpy((void *)src, in, size);

    // Prepare descriptor
    dma_descriptor[CRYPTODMA_CTRL] = DES_USE_DESCRIPTOR_KEY | DES_USE_DESCRIPTOR_IV | DES_USE_AES | 0xF;
    dma_descriptor[CRYPTODMA_CIPHER] = AES_KEY_MODE << 3 | 0x1;

    dma_descriptor[CRYPTODMA_SRC_ADDR_L] = (uint32_t)(src & 0xFFFFFFFF);
    dma_descriptor[CRYPTODMA_SRC_ADDR_H] = (uint32_t)(src >> 32);

    dma_descriptor[CRYPTODMA_DST_ADDR_L] = (uint32_t)(dst & 0xFFFFFFFF);
    dma_descriptor[CRYPTODMA_DST_ADDR_H] = (uint32_t)(dst >> 32);

    dma_descriptor[CRYPTODMA_DATA_AMOUNT_L] = size;
    dma_descriptor[CRYPTODMA_DATA_AMOUNT_H] = 0;

    memcpy(&dma_descriptor[CRYPTODMA_KEY], encrypt_key, 16);

    // Set cryptodma control
    mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_INT_MASK, 0x3);
    mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_L,
                    (uint32_t)((uintptr_t)dma_descriptor & 0xFFFFFFFF));
    mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_H,
                    (uint32_t)((uintptr_t)dma_descriptor >> 32));

    status = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL);

    csi_dcache_clean_range((uint64_t *)dma_descriptor, sizeof(dma_descriptor));
    csi_dcache_clean_range((uint64_t *)src, padding_size);

    // Clear interrupt
    mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT, 0x3);
    // Trigger cryptodma engine
    mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL,
                    DMA_WRITE_MAX_BURST << 24 | DMA_READ_MAX_BURST << 16 | DMA_DESCRIPTOR_MODE << 1 | DMA_ENABLE);

    uint32_t time_start = csi_tick_get_ms();
    do {
        status = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT);
        if ((csi_tick_get_ms() - time_start) >= 10) {
            printf("aes timeout\n");
            return CSI_ERROR;
        }
    } while (status == 0);

    csi_dcache_invalid_range((uint64_t *)dst, padding_size);

    memcpy(out, (void *)dst, size);
    aos_free(in_ex);
    aos_free(out_ex);
    return CSI_OK;
}

/**
  \brief       AES ecb decrypt
  \param[in]   aes     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_aes_ecb_decrypt(csi_aes_t *aes, void *in, void *out, uint32_t size)
{
    CSI_PARAM_CHK(aes, CSI_ERROR);
    CSI_PARAM_CHK(in, CSI_ERROR);
    CSI_PARAM_CHK(out, CSI_ERROR);
    if (size & 0xF || size == 0) return CSI_ERROR;

    __aligned(64) uint32_t dma_descriptor[32] = { 0 };
    uint32_t status;
    uint32_t padding_size = (size + 64 - 1) & (~0x3F);
    uint8_t *in_ex = (uint8_t *)aos_malloc(padding_size + 63);
    if (in_ex == NULL) {
      return CSI_ERROR;
    }

    uint8_t *out_ex = (uint8_t *)aos_malloc(padding_size + 63);
    if (out_ex == NULL) {
      aos_free(in_ex);
      return CSI_ERROR;
    }

    uint64_t src = ((uintptr_t)in_ex + 63) & (~0x3F);
    uint64_t dst = ((uintptr_t)out_ex + 63) & (~0x3F);

    memcpy((void *)src, in, size);

    // Prepare descriptor
    dma_descriptor[CRYPTODMA_CTRL] = DES_USE_DESCRIPTOR_KEY | DES_USE_DESCRIPTOR_IV | DES_USE_AES | 0xF;
    dma_descriptor[CRYPTODMA_CIPHER] = AES_KEY_MODE << 3 | 0x0;

    dma_descriptor[CRYPTODMA_SRC_ADDR_L] = (uint32_t)(src & 0xFFFFFFFF);
    dma_descriptor[CRYPTODMA_SRC_ADDR_H] = (uint32_t)(src >> 32);

    dma_descriptor[CRYPTODMA_DST_ADDR_L] = (uint32_t)(dst & 0xFFFFFFFF);
    dma_descriptor[CRYPTODMA_DST_ADDR_H] = (uint32_t)(dst >> 32);

    dma_descriptor[CRYPTODMA_DATA_AMOUNT_L] = size;
    dma_descriptor[CRYPTODMA_DATA_AMOUNT_H] = 0;

    memcpy(&dma_descriptor[CRYPTODMA_KEY], decrypt_key, 16);

    // Set cryptodma control
    mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_INT_MASK, 0x3);
    mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_L,
                    (uint32_t)((uintptr_t)dma_descriptor & 0xFFFFFFFF));
    mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_H,
                    (uint32_t)((uintptr_t)dma_descriptor >> 32));

    status = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL);

    csi_dcache_clean_range((uint64_t *)dma_descriptor, sizeof(dma_descriptor));
    csi_dcache_clean_range((uint64_t *)src, padding_size);

    // Clear interrupt
    mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT, 0x3);
    // Trigger cryptodma engine
    mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL,
                    DMA_WRITE_MAX_BURST << 24 | DMA_READ_MAX_BURST << 16 | DMA_DESCRIPTOR_MODE << 1 | DMA_ENABLE);

    uint32_t time_start = csi_tick_get_ms();
    do {
        status = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT);
        if ((csi_tick_get_ms() - time_start) >= 10) {
            printf("aes timeout\n");
            return CSI_ERROR;
        }
    } while (status == 0);

    csi_dcache_invalid_range((uint64_t *)dst, padding_size);
    memcpy(out, (void *)dst, size);
    aos_free(in_ex);
    aos_free(out_ex);
    return CSI_OK;
}

/**
  \brief       AES cbc encrypt
  \param[in]   aes     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \param[in]   iv      Init vector
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_aes_cbc_encrypt(csi_aes_t *aes, void *in, void *out, uint32_t size, void *iv)
{
    CSI_PARAM_CHK(aes, CSI_ERROR);
    CSI_PARAM_CHK(in, CSI_ERROR);
    CSI_PARAM_CHK(out, CSI_ERROR);
    CSI_PARAM_CHK(iv, CSI_ERROR);
    if (size & 0xF || size == 0) return CSI_ERROR;

    __aligned(64) uint32_t dma_descriptor[32] = { 0 };
    uint32_t status;
    uint32_t padding_size = (size + 64 - 1) & (~0x3F);
    uint8_t *in_ex = (uint8_t *)aos_malloc(padding_size + 63);
    if (in_ex == NULL) {
      return CSI_ERROR;
    }

    uint8_t *out_ex = (uint8_t *)aos_malloc(padding_size + 63);
    if (out_ex == NULL) {
      aos_free(in_ex);
      return CSI_ERROR;
    }

    uint64_t src = ((uintptr_t)in_ex + 63) & (~0x3F);
    uint64_t dst = ((uintptr_t)out_ex + 63) & (~0x3F);

    memcpy((void *)src, in, size);

    // Prepare descriptor
    dma_descriptor[CRYPTODMA_CTRL] = DES_USE_DESCRIPTOR_KEY | DES_USE_DESCRIPTOR_IV | DES_USE_AES | 0xF;
    dma_descriptor[CRYPTODMA_CIPHER] = AES_KEY_MODE << 3 | CBC_ENABLE << 1 | 0x1;

    dma_descriptor[CRYPTODMA_SRC_ADDR_L] = (uint32_t)(src & 0xFFFFFFFF);
    dma_descriptor[CRYPTODMA_SRC_ADDR_H] = (uint32_t)(src >> 32);

    dma_descriptor[CRYPTODMA_DST_ADDR_L] = (uint32_t)(dst & 0xFFFFFFFF);
    dma_descriptor[CRYPTODMA_DST_ADDR_H] = (uint32_t)(dst >> 32);

    dma_descriptor[CRYPTODMA_DATA_AMOUNT_L] = size;
    dma_descriptor[CRYPTODMA_DATA_AMOUNT_H] = 0;

    memcpy(&dma_descriptor[CRYPTODMA_KEY], encrypt_key, 16);
    memcpy(&dma_descriptor[CRYPTODMA_IV], iv, 16);

    // Set cryptodma control
    mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_INT_MASK, 0x3);
    mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_L,
                    (uint32_t)((uintptr_t)dma_descriptor & 0xFFFFFFFF));
    mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_H,
                    (uint32_t)((uintptr_t)dma_descriptor >> 32));

    status = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL);

    csi_dcache_clean_range((uint64_t *)dma_descriptor, sizeof(dma_descriptor));
    csi_dcache_clean_range((uint64_t *)src, padding_size);

    // Clear interrupt
    mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT, 0x3);
    // Trigger cryptodma engine
    mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL,
                    DMA_WRITE_MAX_BURST << 24 | DMA_READ_MAX_BURST << 16 | DMA_DESCRIPTOR_MODE << 1 | DMA_ENABLE);

    uint32_t time_start = csi_tick_get_ms();
    do {
        status = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT);
        if ((csi_tick_get_ms() - time_start) >= 10) {
            printf("aes timeout\n");
            return CSI_ERROR;
        }
    } while (status == 0);

    csi_dcache_invalid_range((uint64_t *)dst, padding_size);
    memcpy(out, (void *)dst, size);
    aos_free(in_ex);
    aos_free(out_ex);
    return CSI_OK;
}

/**
  \brief       AES cbc decrypt
  \param[in]   aes     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \param[in]   iv      Init vector
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_aes_cbc_decrypt(csi_aes_t *aes, void *in, void *out, uint32_t size, void *iv)
{
    CSI_PARAM_CHK(aes, CSI_ERROR);
    CSI_PARAM_CHK(in, CSI_ERROR);
    CSI_PARAM_CHK(out, CSI_ERROR);
    if (size == 0) return CSI_ERROR;

    __aligned(64) uint32_t dma_descriptor[32] = { 0 };
    uint32_t status;
    uint32_t padding_size = (size + 64 - 1) & (~0x3F);
    uint8_t *in_ex = (uint8_t *)aos_malloc(padding_size + 63);
    if (in_ex == NULL) {
      return CSI_ERROR;
    }

    uint8_t *out_ex = (uint8_t *)aos_malloc(padding_size + 63);
    if (out_ex == NULL) {
      aos_free(in_ex);
      return CSI_ERROR;
    }

    uint64_t src = ((uintptr_t)in_ex + 63) & (~0x3F);
    uint64_t dst = ((uintptr_t)out_ex + 63) & (~0x3F);

    memcpy((void *)src, in, size);

    // Prepare descriptor
    dma_descriptor[CRYPTODMA_CTRL] = DES_USE_DESCRIPTOR_KEY | DES_USE_DESCRIPTOR_IV | DES_USE_AES | 0xF;
    dma_descriptor[CRYPTODMA_CIPHER] = AES_KEY_MODE << 3 | CBC_ENABLE << 1 | 0x0;

    dma_descriptor[CRYPTODMA_SRC_ADDR_L] = (uint32_t)(src & 0xFFFFFFFF);
    dma_descriptor[CRYPTODMA_SRC_ADDR_H] = (uint32_t)(src >> 32);

    dma_descriptor[CRYPTODMA_DST_ADDR_L] = (uint32_t)(dst & 0xFFFFFFFF);
    dma_descriptor[CRYPTODMA_DST_ADDR_H] = (uint32_t)(dst >> 32);

    dma_descriptor[CRYPTODMA_DATA_AMOUNT_L] = size;
    dma_descriptor[CRYPTODMA_DATA_AMOUNT_H] = 0;

    memcpy(&dma_descriptor[CRYPTODMA_KEY], decrypt_key, 16);
    memcpy(&dma_descriptor[CRYPTODMA_IV], iv, 16);

    // Set cryptodma control
    mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_INT_MASK, 0x3);
    mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_L,
                    (uint32_t)((uintptr_t)dma_descriptor & 0xFFFFFFFF));
    mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_H,
                    (uint32_t)((uintptr_t)dma_descriptor >> 32));

    status = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL);

    csi_dcache_clean_range((uint64_t *)dma_descriptor, sizeof(dma_descriptor));
    csi_dcache_clean_range((uint64_t *)src, size);

    // Clear interrupt
    mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT, 0x3);
    // Trigger cryptodma engine
    mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL,
                    DMA_WRITE_MAX_BURST << 24 | DMA_READ_MAX_BURST << 16 | DMA_DESCRIPTOR_MODE << 1 | DMA_ENABLE);

    uint32_t time_start = csi_tick_get_ms();
    do {
        status = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT);
        if ((csi_tick_get_ms() - time_start) >= 10) {
            printf("aes timeout\n");
            return CSI_ERROR;
        }
    } while (status == 0);

    csi_dcache_invalid_range((uint64_t *)dst, size);
    memcpy(out, (void *)dst, size);
    aos_free(in_ex);
    aos_free(out_ex);
    return CSI_OK;
}

/**
  \brief       AES cfb1 encrypt
  \param[in]   aes     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \param[in]   iv      Init vector
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_aes_cfb1_encrypt(csi_aes_t *aes, void *in, void *out,  uint32_t size, void *iv)
{
    return CSI_UNSUPPORTED;
}

/**
  \brief       AES cfb1 decrypt
  \param[in]   aes     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \param[in]   iv      Init vector
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_aes_cfb1_decrypt(csi_aes_t *aes, void *in, void *out,  uint32_t size, void *iv)
{
    return CSI_UNSUPPORTED;
}

/**
  \brief       AES cfb8 encrypt
  \param[in]   aes     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \param[in]   iv      Init vector
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_aes_cfb8_encrypt(csi_aes_t *aes, void *in, void *out, uint32_t size, void *iv)
{
    return CSI_UNSUPPORTED;
}

/**
  \brief       AES cfb8 decrypt
  \param[in]   aes     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \param[in]   iv      Init vector
  \return      Error code \ref Csi_error_t
*/
csi_error_t csi_aes_cfb8_decrypt(csi_aes_t *aes, void *in, void *out, uint32_t size, void *iv)
{
    return CSI_UNSUPPORTED;
}

/**
  \brief       AES cfb128 decrypt
  \param[in]   aes     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \param[in]   iv      Init vector
  \param[out]  num     The number of the 128-bit block we have used
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_aes_cfb128_decrypt(csi_aes_t *aes, void *in, void *out, uint32_t size, void *iv, uint32_t *num)
{
    return CSI_UNSUPPORTED;
}

/**
  \brief       AES cfb128 encrypt
  \param[in]   aes     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \param[in]   iv      Init vector
  \param[out]  num     The number of the 128-bit block we have used
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_aes_cfb128_encrypt(csi_aes_t *aes, void *in, void *out, uint32_t size, void *iv, uint32_t *num)
{
    return CSI_UNSUPPORTED;
}

/**
  \brief       AES ofb encrypt
  \param[in]   aes     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \param[in]   iv      Init vector
  \param[out]  num     The number of the 128-bit block we have used
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_aes_ofb_encrypt(csi_aes_t *aes, void *in, void *out, uint32_t size, void *iv, uint32_t *num)
{
    return CSI_UNSUPPORTED;
}

/**
  \brief       AES ofb decrypt
  \param[in]   aes     Handle to operate
  \param[in]   in      Pointer to the source data
  \param[out]  out     Pointer to the result data
  \param[in]   size    The source data size
  \param[in]   iv      Init vector
  \param[out]  num     The number of the 128-bit block we have used
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_aes_ofb_decrypt(csi_aes_t *aes, void *in, void *out, uint32_t size, void *iv, uint32_t *num)
{
    return CSI_UNSUPPORTED;
}

/**
  \brief       AES ctr encrypt
  \param[in]   aes              Handle to operate
  \param[in]   in               Pointer to the source data
  \param[out]  out              Pointer to the result data
  \param[in]   size             The source data size
  \param[in]   iv               Init vector
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_aes_ctr_encrypt(csi_aes_t *aes, void *in,void *out, uint32_t size, void *iv)
{
    return CSI_UNSUPPORTED;        
}

/**
  \brief       AES ctr decrypt
  \param[in]   aes              Handle to operate
  \param[in]   in               Pointer to the source data
  \param[out]  out              Pointer to the result data
  \param[in]   size             The source data size
  \param[in]   iv               Init vecotr
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_aes_ctr_decrypt(csi_aes_t *aes, void *in, void *out, uint32_t size, void *iv)
{
    return CSI_UNSUPPORTED;
}

/**
  \brief       Enable AES power manage
  \param[in]   aes    Handle to operate
  \return      Error code \ref csi_error_t
*/
csi_error_t csi_aes_enable_pm(csi_aes_t *aes)
{
    return CSI_UNSUPPORTED;
}

/**
  \brief       Disable AES power manage
  \param[in]   aes    Handle to operate
  \return      None
*/
void csi_aes_disable_pm(csi_aes_t *aes)
{

}

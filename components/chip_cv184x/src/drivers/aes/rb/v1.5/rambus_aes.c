/*
 *	Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/*******************************************************
 * @file    rambus_aes.c
 * @brief   source file for dev_aes csi driver
 * @version V1.0
 * @date    2. July 2020
 * ******************************************************/

#include <drv/aes.h>
#include <drv/cvi_irq.h>
#include <basic_defs.h>
#include "soc.h"
#include "device_types.h"
#include "device_rw.h"
#include "rambus.h"
#include "drv/tick.h"

/**
  \brief       Initialize AES Interface. 
  \param[in]   aes    operate handle
  \param[in]   idx    device id: support 0.
  \return      error code \ref csi_error_t
*/
csi_error_t csi_aes_init(csi_aes_t *aes, uint32_t idx)
{
        CHECK_RET_WITH_RET(aes, SC_PARAM_INV);

        csi_error_t ret = CSI_OK;
        if (0 == target_get(DEV_RAMBUS_120SI_TAG, idx, &aes->dev)) {
                ret = CSI_OK;
        } else {
                ret = CSI_ERROR;
        }

        return ret;
}
/**
  \brief       De-initialize AES Interface.
  \param[in]   aes    aes handle to operate
  \return      None
*/
void csi_aes_uninit(csi_aes_t *aes)
{
        CHECK_RET(aes);
}

static int rb_aes_set_key(Device_Handle_t *dev_aes, uint8_t *key,
                          uint32_t len_byte)
{
        uint32_t temp[32 / 4] __attribute__((aligned(64))) = {0};
        uint32_t t;
        memcpy(temp, key, len_byte);

        //Log_HexDump("key", 0, key, len_byte);
        CHECK_64byte_ALIGNMENT(temp, SC_PARAM_INV);
        LOG_INFO("===key %s, %d, addr:%p, size:%d\n", __FUNCTION__, __LINE__,
                 temp, len_byte);
        LOG_INFO("===key %s, %d, addr:%p, size:%d\n", __FUNCTION__, __LINE__,
                 temp,
                 len_byte < CACHE_BLOCK_SIZE ? CACHE_BLOCK_SIZE : len_byte);
#ifdef CONFIG_RB_CACHE_ENABLE
        csi_dcache_clean_invalid_range(
            (uint64_t *)temp,
            len_byte < CACHE_BLOCK_SIZE ? CACHE_BLOCK_SIZE : len_byte);
#endif
        //preload key to ram_area_written7
        //-----------------------------------------------------------------------------------
        Device_Write32(dev_aes, EIP120_KEY_STORE_SIZE,
                       KEY_STORE_KEY_SIZE_256); //key size 256
        Device_Write32(dev_aes, EIP120_KEY_STORE_WRITE_AREA,
                       KEY_STORE_AREA_0); //ram_area_written0/1
        Device_Write32(dev_aes, EIP120_CTRL_ALG_SEL,
                       ALG_SEL_KEY_STORE); //key store
        Device_Write32(
            dev_aes, EIP120_AIC_ENABLE_CTRL,
            0x00000001); //enable bit of the result available interrupt
        Device_Write32(dev_aes, EIP120_AIC_ACK, 0x00000001);
        Device_Write32(dev_aes, EIP120_DMAC_IER, 0x00000033);
        Device_Write32(dev_aes, EIP120_DMAC_IMR, 0x00000033);
        Device_Write32(dev_aes, EIP120_DMAC_CH0_CTRL1, 0x00000000);
        Device_Write32(dev_aes, EIP120_DMAC_ICR, 0x00000003); //clear ch0 inte
        Device_Write32(dev_aes, EIP120_DMAC_CH0_SRCADDR,
                       (uint32_t)((uint64_t)temp & 0xFFFFFFFF));
        Device_Write32(dev_aes, EIP120_DMAC_CH0_SRCADDR_HI,
                       (uint32_t)(((uint64_t)temp >> 32) & 0xFFFFFFFF));
        Device_Write32(dev_aes, EIP120_DMAC_CH0_DMALENGTH, len_byte);
        Device_Write32(dev_aes, EIP120_DMAC_CH0_CTRL0, DMA_CH0_CTRL0);
        Device_Write32(dev_aes, EIP120_DMAC_CH0_CTRL1, 0x00001011);

        //wait IRQ_OUT 0x1
        /* TODO retry */
        while (Device_Read32(dev_aes, EIP120_AIC_ACK) != 0x00000001)
                ;
        LOG_INFO("wait ok %s, %d\n", __FUNCTION__, __LINE__);

        Device_Write32(dev_aes, EIP120_AIC_ACK, 0x00000001);

        Device_Write32(dev_aes, EIP120_CTRL_ALG_SEL,
                       0x00000000); //disable keystore func
        t = Device_Read32(dev_aes, EIP120_CTRL_ERROR_STAT);
        CHECK_RET_WITH_RET(t == 0x00000000, SC_DRV_FAILED);

        t = Device_Read32(dev_aes, EIP120_KEY_STORE_WRITTEN_AREA);
        CHECK_RET_WITH_RET(t == 0x00000003, SC_DRV_FAILED);

        return CSI_OK;
}

static int rb_aes_clr_key(Device_Handle_t *dev_aes)
{
        Device_Write32(dev_aes, EIP120_KEY_STORE_WRITTEN_AREA,
                       KEY_STORE_AREA_0 | KEY_STORE_AREA_1);

        return CSI_OK;
}

#ifdef CONFIG_RB_AES_DMA_MODE

static int rb_aes_config_cbc(Device_Handle_t *dev_aes)
{
        //-----------------------------------------------------------------------------------
        //load key from ram_area7 to dev_aes engine
        //-----------------------------------------------------------------------------------
        Device_Write32(dev_aes, EIP120_CTRL_ALG_SEL, ALG_SEL_AES); //AES
        Device_Write32(dev_aes, EIP120_AIC_ENABLE_CTRL, 0x00000001);
        Device_Write32(dev_aes, EIP120_AIC_ACK, 0x00000001);
        Device_Write32(dev_aes, EIP120_KEY_STORE_READ_AREA,
                       KEY_STORE_READ_AREA_0); //read key

        return CSI_OK;
}

static int rb_aes_config_dma(Device_Handle_t *dev_aes, uint8_t *in,
                             uint8_t *out, uint32_t len_byte)
{
        uint32_t t;
        LOG_INFO("===in %s, %d, addr:%p, size:%d\n", __FUNCTION__, __LINE__, in,
                 len_byte < CACHE_BLOCK_SIZE ? CACHE_BLOCK_SIZE : len_byte);
        LOG_INFO("===in %s, %d, addr:%p, size:%d\n", __FUNCTION__, __LINE__, in,
                 len_byte < CACHE_BLOCK_SIZE
                     ? CACHE_BLOCK_SIZE
                     : len_byte < CACHE_BLOCK_SIZE ? CACHE_BLOCK_SIZE
                                                   : len_byte);
#ifdef CONFIG_RB_CACHE_ENABLE
        csi_dcache_clean_invalid_range(
            (uint64_t *)in,
            len_byte < CACHE_BLOCK_SIZE ? CACHE_BLOCK_SIZE : len_byte);
        LOG_INFO("===out %s, %d, addr:%p, size:%d\n", __FUNCTION__, __LINE__,
                 out, len_byte);
        LOG_INFO("===out %s, %d, addr:%p, size:%d\n", __FUNCTION__, __LINE__,
                 out,
                 len_byte < CACHE_BLOCK_SIZE ? CACHE_BLOCK_SIZE : len_byte);
        csi_dcache_clean_invalid_range(
            (uint64_t *)out,
            len_byte < CACHE_BLOCK_SIZE ? CACHE_BLOCK_SIZE : len_byte);
#endif

        //ch1 data_out
        Device_Write32(dev_aes, EIP120_DMAC_IER, 0x00000033);
        Device_Write32(dev_aes, EIP120_DMAC_IMR, 0x00000033);
        Device_Write32(dev_aes, EIP120_DMAC_CH1_CTRL1, 0x00000000);
        Device_Write32(dev_aes, EIP120_DMAC_ICR, 0x00000030);
        Device_Write32(dev_aes, EIP120_DMAC_CH1_DESTADDR,
                       (uint32_t)((uint64_t)out & 0xFFFFFFFF));
        Device_Write32(dev_aes, EIP120_DMAC_CH1_DESTADDR_HI,
                       (uint32_t)(((uint64_t)out >> 32) & 0xFFFFFFFF));

        Device_Write32(dev_aes, EIP120_DMAC_CH1_DMALENGTH, len_byte);
        // Device_Write32(dev_aes, EIP120_DMAC_CH1_CTRL0, 0x04040401);
        Device_Write32(dev_aes, EIP120_DMAC_CH1_CTRL0, DMA_CH1_CTRL0);
        Device_Write32(dev_aes, EIP120_DMAC_CH1_CTRL1, 0x00000341);
        //ch0 message in
        Device_Write32(dev_aes, EIP120_DMAC_IER, 0x00000033);
        Device_Write32(dev_aes, EIP120_DMAC_IMR, 0x00000033);
        Device_Write32(dev_aes, EIP120_DMAC_CH0_CTRL1, 0x00000000);
        Device_Write32(dev_aes, EIP120_DMAC_ICR, 0x00000003);
        Device_Write32(dev_aes, EIP120_DMAC_CH0_SRCADDR,
                       (uint32_t)((uint64_t)in & 0xFFFFFFFF));
        Device_Write32(dev_aes, EIP120_DMAC_CH0_SRCADDR_HI,
                       (uint32_t)(((uint64_t)in >> 32) & 0xFFFFFFFF));

        RB_PERF_START_POINT();
        Device_Write32(dev_aes, EIP120_DMAC_CH0_DMALENGTH, len_byte);
        // Device_Write32(dev_aes, EIP120_DMAC_CH0_CTRL0, 0x04040104);
        Device_Write32(dev_aes, EIP120_DMAC_CH0_CTRL0, DMA_CH0_CTRL0);
        Device_Write32(dev_aes, EIP120_DMAC_CH0_CTRL1, 0x00001011);

        while (Device_Read32(dev_aes, EIP120_AIC_ACK) != 0x00000001)
                ;
        Device_Write32(dev_aes, EIP120_AIC_ACK, 0x00000001);
        Device_Write32(dev_aes, EIP120_CTRL_ALG_SEL, 0x00000000);
        t = Device_Read32(dev_aes, EIP120_CTRL_ERROR_STAT);
        CHECK_RET_WITH_RET(t == 0x00000000, SC_DRV_FAILED);

        RB_PERF_END_POINT();
        LOG_INFO("===out %s, %d, addr:%p, size:%d\n", __FUNCTION__, __LINE__,
                 out, len_byte);
        LOG_INFO("===out %s, %d, addr:%p, size:%d\n", __FUNCTION__, __LINE__,
                 out,
                 len_byte < CACHE_BLOCK_SIZE ? CACHE_BLOCK_SIZE : len_byte);

#ifdef CONFIG_RB_CACHE_ENABLE
        csi_dcache_invalid_range((uint64_t *)out, len_byte < CACHE_BLOCK_SIZE
                                                      ? CACHE_BLOCK_SIZE
                                                      : len_byte);
#endif

        return CSI_OK;
}
#else

static int rb_aes_config_cbc(Device_Handle_t *dev_aes)
{
        //-----------------------------------------------------------------------------------
        //load key from ram_area7 to dev_aes engine
        //-----------------------------------------------------------------------------------
        Device_Write32(dev_aes, EIP120_CTRL_ALG_SEL, 0x00000000); //AES
        Device_Write32(dev_aes, EIP120_AIC_ENABLE_CTRL, 0x00000001);
        Device_Write32(dev_aes, EIP120_AIC_ACK, 0x00000001);
        Device_Write32(dev_aes, EIP120_KEY_STORE_READ_AREA,
                       KEY_STORE_READ_AREA_0); //read key

        return CSI_OK;
}

static int rb_aes_slave_block(Device_Handle_t *dev_aes, uint8_t *in,
                              uint8_t *out)
{
        /* wait input ready */
        while ((Device_Read32(dev_aes, EIP120_CRYPTO_CTRL) & 0x00000002) !=
               0x00000002)
                ;
        Device_Write32(dev_aes, EIP120_CRYPTO_DATA_IN_0, *(uint32_t *)in);
        Device_Write32(dev_aes, EIP120_CRYPTO_DATA_IN_1, *(uint32_t *)(in + 4));
        Device_Write32(dev_aes, EIP120_CRYPTO_DATA_IN_2, *(uint32_t *)(in + 8));
        Device_Write32(dev_aes, EIP120_CRYPTO_DATA_IN_3,
                       *(uint32_t *)(in + 12));

        Device_Write32(dev_aes, EIP120_CRYPTO_CTRL, 0x00000002);

        /* wait out ready */
        while ((Device_Read32(dev_aes, EIP120_CRYPTO_CTRL) & 0x00000001) !=
               0x00000001)
                ;
        *(uint32_t *)out = Device_Read32(dev_aes, EIP120_CRYPTO_DATA_IN_0);
        *(uint32_t *)(out + 4) =
            Device_Read32(dev_aes, EIP120_CRYPTO_DATA_IN_1);
        *(uint32_t *)(out + 8) =
            Device_Read32(dev_aes, EIP120_CRYPTO_DATA_IN_2);
        *(uint32_t *)(out + 12) =
            Device_Read32(dev_aes, EIP120_CRYPTO_DATA_IN_3);
        Device_Write32(dev_aes, EIP120_CRYPTO_CTRL, 0x00000001);

        return CSI_OK;
}

static int rb_aes_config_slave(Device_Handle_t *dev_aes, uint8_t *in,
                               uint8_t *out, uint32_t len_byte)
{
        uint32_t t;
        uint32_t blocks;
        int      ret;
        LOG_INFO("===in %s, %d, addr:%p, size:%d\n", __FUNCTION__, __LINE__, in,
                 len_byte < CACHE_BLOCK_SIZE ? CACHE_BLOCK_SIZE : len_byte);
        LOG_INFO("===in %s, %d, addr:%p, size:%d\n", __FUNCTION__, __LINE__, in,
                 len_byte < CACHE_BLOCK_SIZE
                     ? CACHE_BLOCK_SIZE
                     : len_byte < CACHE_BLOCK_SIZE ? CACHE_BLOCK_SIZE
                                                   : len_byte);

        RB_PERF_START_POINT();
        blocks = len_byte / AES_BLOCK_SIZE_IN_BYTE;
        for (int i = 0; i < blocks; i++) {
                ret = rb_aes_slave_block(dev_aes,
                                         in + (i * AES_BLOCK_SIZE_IN_BYTE),
                                         out + (i * AES_BLOCK_SIZE_IN_BYTE));
                CHECK_RET_WITH_RET(ret == CSI_OK, SC_DRV_FAILED);
        }

        Device_Write32(dev_aes, EIP120_CTRL_ALG_SEL, 0x00000000);
        t = Device_Read32(dev_aes, EIP120_CTRL_ERROR_STAT);
        CHECK_RET_WITH_RET(t == 0x00000000, SC_DRV_FAILED);
        RB_PERF_END_POINT();
        LOG_INFO("===out %s, %d, addr:%p, size:%d\n", __FUNCTION__, __LINE__,
                 out, len_byte);

        return CSI_OK;
}
#endif

static int rb_aes_set_iv(Device_Handle_t *dev_aes, uint8_t *iv,
                         uint32_t len_byte)
{
        uint32_t temp[16 / 4] __attribute__((aligned(4))) = {0};
        memcpy(temp, iv, len_byte);
        Device_Write32(dev_aes, EIP120_CRYPTO_IV_IN_0, temp[0]);

        Device_Write32(dev_aes, EIP120_CRYPTO_IV_IN_1, temp[1]);
        Device_Write32(dev_aes, EIP120_CRYPTO_IV_IN_2, temp[2]);
        Device_Write32(dev_aes, EIP120_CRYPTO_IV_IN_3, temp[3]);

        return CSI_OK;
}

static int rb_aes_get_iv(Device_Handle_t *dev_aes, uint8_t *iv,
                         uint32_t len_byte)
{
        IDENTIFIER_NOT_USED(len_byte);
        *(uint32_t *)(iv)      = Device_Read32(dev_aes, EIP120_CRYPTO_IV_IN_0);
        *(uint32_t *)(iv + 4)  = Device_Read32(dev_aes, EIP120_CRYPTO_IV_IN_1);
        *(uint32_t *)(iv + 8)  = Device_Read32(dev_aes, EIP120_CRYPTO_IV_IN_2);
        *(uint32_t *)(iv + 12) = Device_Read32(dev_aes, EIP120_CRYPTO_IV_IN_3);

        return CSI_OK;
}

static int rb_aes_set_dir(Device_Handle_t *dev_aes, uint32_t enc,
                          uint32_t key_len_byte)
{
        /* NOTE cbc/256 cur */
        IDENTIFIER_NOT_USED(key_len_byte); //256
        if (enc) {
                Device_Write32(dev_aes, EIP120_CRYPTO_CTRL,
                               0x20000038 |
                                   AES_DIR_BIT); //enc key256 cbc  save_context
        } else {
                Device_Write32(dev_aes, EIP120_CRYPTO_CTRL, 0x20000038);
        }

        return CSI_OK;
}

static int rb_aes_set_length(Device_Handle_t *dev_aes, uint32_t len_byte)
{
        Device_Write32(dev_aes, EIP120_CRYPTO_C_LENGTH_0, len_byte);
        Device_Write32(dev_aes, EIP120_CRYPTO_C_LENGTH_1, 0x00000000);
        Device_Write32(dev_aes, EIP120_CRYPTO_AUTH_LENGTH, 0x00000000);

        return CSI_OK;
}

/**
  \brief       Set encrypt key
  \param[in]   dev_aes        dev_aes handle to operate
  \param[in]   key        Pointer to the key buf
  \param[in]   key_len    Pointer to \ref csi_aes_key_bits_t
  \return      error code \ref csi_error_t
*/
csi_error_t csi_aes_set_encrypt_key(csi_aes_t *aes, void *key,
                                    csi_aes_key_bits_t key_len)
{
        CHECK_RET_WITH_RET(aes, SC_PARAM_INV);
        CHECK_RET_WITH_RET(key, SC_PARAM_INV);
        CHECK_RET_WITH_RET(key_len == AES_KEY_LEN_BITS_256, SC_PARAM_INV);

        csi_error_t      ret     = CSI_OK;
        Device_Handle_t *dev_aes = (Device_Handle_t *)HANDLE_REG_BASE(aes);
        uint32_t         keynum  = 0U;

        switch (key_len) {
                case AES_KEY_LEN_BITS_256:
                        keynum = 32;
                        break;

                case AES_KEY_LEN_BITS_128:
                case AES_KEY_LEN_BITS_192:
                /* fall */
                default:
                        ret = SC_PARAM_INV;
                        break;
        }

        if (CSI_OK == ret) {
                ret = rb_aes_set_key(dev_aes, key, keynum);
        }

        return ret;
}

/**
  \brief       Set decrypt key
  \param[in]   aes        aes handle to operate
  \param[in]   key        Pointer to the key buf
  \param[in]   key_len    Pointer to \ref csi_aes_key_bits_t
  \return      error code \ref csi_error_t
*/
csi_error_t csi_aes_set_decrypt_key(csi_aes_t *aes, void *key,
                                    csi_aes_key_bits_t key_len)
{
        return csi_aes_set_encrypt_key(aes, key, key_len);
}

/**
  \brief       Aes ecb encrypt or decrypt
  \param[in]   aes     aes handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \return      error code \ref csi_error_t
*/
csi_error_t csi_aes_ecb_encrypt(csi_aes_t *aes, void *in, void *out,
                                uint32_t size)
{
        CHECK_RET_WITH_RET(aes, SC_PARAM_INV);
        csi_error_t ret = CSI_UNSUPPORTED;

        return ret;
}
/**
  \brief       Aes ecb decrypt
  \param[in]   aes     aes handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \return      error code \ref csi_error_t
*/
csi_error_t csi_aes_ecb_decrypt(csi_aes_t *aes, void *in, void *out,
                                uint32_t size)
{
        CHECK_RET_WITH_RET(aes, SC_PARAM_INV);
        csi_error_t ret = CSI_UNSUPPORTED;

        return ret;
}

static csi_error_t rb_aes_cbc_dma(Device_Handle_t *dev_aes, uint8_t *in,
                                  uint8_t *out, uint32_t size, uint8_t *iv,
                                  uint32_t enc)
{
        uint8_t     tiv[16] __attribute__((aligned(4)));
        uint8_t *   pin  = in;
        uint8_t *   pout = out;
        csi_error_t ret  = CSI_OK;
        int         left = size;
        uint32_t    len;
        memcpy(tiv, iv, 16);

        while (left > 0) {
                if (left >= EIP_DMA_MAX_LEN_IN_BYTE) {
                        len = EIP_DMA_MAX_LEN_IN_BYTE;
                } else {
                        len = left;
                }
                LOG_INFO("===%s, %d, len:%d\n", __FUNCTION__, __LINE__, len);
                ret = rb_aes_config_cbc(dev_aes);
                CHECK_RET_WITH_RET(ret == CSI_OK, ret);

                Log_HexDump("set_iv", 0, tiv, 16);
                ret = rb_aes_set_iv(dev_aes, tiv, 128 / 8);
                CHECK_RET_WITH_RET(ret == CSI_OK, ret);

                ret = rb_aes_set_dir(dev_aes, enc, 256 / 8);
                CHECK_RET_WITH_RET(ret == CSI_OK, ret);

                ret = rb_aes_set_length(dev_aes, len);
                CHECK_RET_WITH_RET(ret == CSI_OK, ret);

                ret = rb_aes_config_dma(dev_aes, pin, pout, len);
                CHECK_RET_WITH_RET(ret == CSI_OK, ret);

                ret = rb_aes_get_iv(dev_aes, tiv, 128 / 8);
                CHECK_RET_WITH_RET(ret == CSI_OK, ret);

                left -= len;
                pin += len;
                pout += len;
        }

        LOG_INFO("===%s, %d, %p, %p, %p, %d\n", __FUNCTION__, __LINE__, in, out,
                 iv, size);
        // Log_HexDump("out", 0, out, size);

        return ret;
}

/**
  \brief       Aes cbc encrypt or decrypt
  \param[in]   aes     aes handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \return      error code \ref csi_error_t
*/
csi_error_t csi_aes_cbc_encrypt(csi_aes_t *aes, void *in, void *out,
                                uint32_t size, void *iv)
{
        CHECK_RET_WITH_RET(aes, SC_PARAM_INV);
        CHECK_RET_WITH_RET(in, SC_PARAM_INV);
        CHECK_RET_WITH_RET(out, SC_PARAM_INV);
        CHECK_RET_WITH_RET(iv, SC_PARAM_INV);
        CHECK_16byte_ALIGNMENT(size, SC_PARAM_INV);

        csi_error_t     ret     = CSI_OK;
        Device_Handle_t dev_aes = (Device_Handle_t)HANDLE_REG_BASE(aes);

        // Log_HexDump("in", 0, in, size);
        // Log_HexDump("iv", 0, iv, 16);
#ifdef CONFIG_RB_AES_DMA_MODE
        CHECK_64byte_ALIGNMENT(in, SC_PARAM_INV);
        CHECK_64byte_ALIGNMENT(out, SC_PARAM_INV);

        ret = rb_aes_cbc_dma(dev_aes, in, out, size, iv, 1);
        CHECK_RET_WITH_RET(ret == CSI_OK, ret);
#else
        ret = rb_aes_config_cbc(dev_aes);
        CHECK_RET_WITH_RET(ret == CSI_OK, ret);

        ret = rb_aes_set_iv(dev_aes, (uint8_t *)iv, 128 / 8);
        CHECK_RET_WITH_RET(ret == CSI_OK, ret);

        ret = rb_aes_set_dir(dev_aes, 1, 256 / 8);
        CHECK_RET_WITH_RET(ret == CSI_OK, ret);

        ret = rb_aes_set_length(dev_aes, size);
        CHECK_RET_WITH_RET(ret == CSI_OK, ret);

        ret = rb_aes_config_slave(dev_aes, in, out, size);
        CHECK_RET_WITH_RET(ret == CSI_OK, ret);
#endif
        ret = rb_aes_clr_key(dev_aes);
        CHECK_RET_WITH_RET(ret == CSI_OK, ret);

        LOG_INFO("===%s, %d, %p, %p, %p, %d\n", __FUNCTION__, __LINE__, in, out,
                 iv, size);
        // Log_HexDump("out", 0, out, size);

        return ret;
}
/**
  \brief       Aes cbc decrypt
  \param[in]   aes     aes handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \return      error code \ref csi_error_t
*/
csi_error_t csi_aes_cbc_decrypt(csi_aes_t *aes, void *in, void *out,
                                uint32_t size, void *iv)
{
        CHECK_RET_WITH_RET(aes, SC_PARAM_INV);
        CHECK_RET_WITH_RET(in, SC_PARAM_INV);
        CHECK_RET_WITH_RET(out, SC_PARAM_INV);
        CHECK_RET_WITH_RET(iv, SC_PARAM_INV);
        CHECK_16byte_ALIGNMENT(size, SC_PARAM_INV);

        csi_error_t      ret     = CSI_OK;
        Device_Handle_t *dev_aes = (Device_Handle_t *)HANDLE_REG_BASE(aes);

        //Log_HexDump("in", 0, in, size);
        //Log_HexDump("iv", 0, iv, 16);
#ifdef CONFIG_RB_AES_DMA_MODE
        CHECK_64byte_ALIGNMENT(in, SC_PARAM_INV);
        CHECK_64byte_ALIGNMENT(out, SC_PARAM_INV);

        ret = rb_aes_cbc_dma(dev_aes, in, out, size, iv, 0);
        CHECK_RET_WITH_RET(ret == CSI_OK, ret);

#else
        ret = rb_aes_config_cbc(dev_aes);
        CHECK_RET_WITH_RET(ret == CSI_OK, ret);

        ret = rb_aes_set_iv(dev_aes, (uint8_t *)iv, 128 / 8);
        CHECK_RET_WITH_RET(ret == CSI_OK, ret);

        ret = rb_aes_set_dir(dev_aes, 0, 256 / 8);
        CHECK_RET_WITH_RET(ret == CSI_OK, ret);

        ret = rb_aes_set_length(dev_aes, size);
        CHECK_RET_WITH_RET(ret == CSI_OK, ret);

        ret = rb_aes_config_slave(dev_aes, in, out, size);
        CHECK_RET_WITH_RET(ret == CSI_OK, ret);
#endif
        ret = rb_aes_clr_key(dev_aes);
        CHECK_RET_WITH_RET(ret == CSI_OK, ret);

        //Log_HexDump("out", 0, out, size);

        return ret;
}
/**
  \brief       Aes cfb1 encrypt
  \param[in]   aes     aes handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \return      error code \ref csi_error_t
*/
csi_error_t csi_aes_cfb1_encrypt(csi_aes_t *aes, void *in, void *out,
                                 uint32_t size, void *iv)
{
        return CSI_UNSUPPORTED;
}
/**
  \brief       Aes cfb1 decrypt
  \param[in]   aes     aes handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \return      error code \ref csi_error_t
*/
csi_error_t csi_aes_cfb1_decrypt(csi_aes_t *aes, void *in, void *out,
                                 uint32_t size, void *iv)
{
        return CSI_UNSUPPORTED;
}
/**
  \brief       Aes cfb8 encrypt
  \param[in]   aes     aes handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \return      error code \ref csi_error_t
*/
csi_error_t csi_aes_cfb8_encrypt(csi_aes_t *aes, void *in, void *out,
                                 uint32_t size, void *iv)
{
        return CSI_UNSUPPORTED;
}
/**
  \brief       Aes cfb8 decrypt
  \param[in]   dev_aes     dev_aes handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \return      error code \ref csi_error_t
*/
csi_error_t csi_aes_cfb8_decrypt(csi_aes_t *aes, void *in, void *out,
                                 uint32_t size, void *iv)
{
        return CSI_UNSUPPORTED;
}
/**
  \brief       Aes cfb128 decrypt
  \param[in]   dev_aes     dev_aes handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \param[out]  num     the number of the 128-bit block we have used
  \return      error code \ref csi_error_t
*/
csi_error_t csi_aes_cfb128_decrypt(csi_aes_t *aes, void *in, void *out,
                                   uint32_t size, void *iv, uint32_t *num)
{
        return CSI_UNSUPPORTED;
}
/**
  \brief       Aes cfb128 encrypt
  \param[in]   dev_aes     dev_aes handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \param[out]  num     the number of the 128-bit block we have used
  \return      error code \ref csi_error_t
*/
csi_error_t csi_aes_cfb128_encrypt(csi_aes_t *aes, void *in, void *out,
                                   uint32_t size, void *iv, uint32_t *num)
{
        return CSI_UNSUPPORTED;
}
/**
  \brief       Aes ofb encrypt
  \param[in]   dev_aes     dev_aes handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \param[out]  num     the number of the 128-bit block we have used
  \return      error code \ref csi_error_t
*/
csi_error_t csi_aes_ofb_encrypt(csi_aes_t *aes, void *in, void *out,
                                uint32_t size, void *iv, uint32_t *num)
{
        return CSI_UNSUPPORTED;
}
/**
  \brief       Aes ofb decrypt
  \param[in]   dev_aes     dev_aes handle to operate
  \param[in]   in      Pointer to the Source data
  \param[out]  out     Pointer to the Result data
  \param[in]   size    the Source data size
  \param[in]   iv      init vector
  \param[out]  num     the number of the 128-bit block we have used
  \return      error code \ref csi_error_t
*/
csi_error_t csi_aes_ofb_decrypt(csi_aes_t *aes, void *in, void *out,
                                uint32_t size, void *iv, uint32_t *num)
{
        return CSI_UNSUPPORTED;
}
/**
  \brief       Aes ctr encrypt
  \param[in]   dev_aes              dev_aes handle to operate
  \param[in]   in               Pointer to the Source data
  \param[out]  out              Pointer to the Result data
  \param[in]   size             the Source data size
  \param[in]   nonce_counter    Pointer to the 128-bit nonce and counter
  \param[in]   stream_block     Pointer to the saved stream-block for resuming
  \param[in]   iv               init vector
  \param[out]  num              the number of the 128-bit block we have used
  \return      error code \ref csi_error_t
*/
csi_error_t csi_aes_ctr_encrypt(csi_aes_t *aes, void *in, void *out,
                                uint32_t size, uint8_t nonce_counter[16],
                                uint8_t stream_block[16], void *iv,
                                uint32_t *num)
{
        return CSI_UNSUPPORTED;
}
/**
  \brief       Aes ctr decrypt
  \param[in]   dev_aes              dev_aes handle to operate
  \param[in]   in               Pointer to the Source data
  \param[out]  out              Pointer to the Result data
  \param[in]   size             the Source data size
  \param[in]   nonce_counter    Pointer to the 128-bit nonce and counter
  \param[in]   stream_block     Pointer to the saved stream-block for resuming
  \param[in]   iv               init vecotr
  \param[out]  num              the number of the 128-bit block we have used
  \return      error code \ref csi_error_t
*/
csi_error_t csi_aes_ctr_decrypt(csi_aes_t *aes, void *in, void *out,
                                uint32_t size, uint8_t nonce_counter[16],
                                uint8_t stream_block[16], void *iv,
                                uint32_t *num)
{
        return CSI_UNSUPPORTED;
}

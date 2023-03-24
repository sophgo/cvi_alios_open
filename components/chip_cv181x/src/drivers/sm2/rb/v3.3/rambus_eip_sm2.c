/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/******************************************************************************
 * @file     rambus_eip_sm2.c
 * @brief    Source File for SM2 Driver
 * @version  V2.0
 * @date     10. Dec 2020
 ******************************************************************************/
#include <drv/cvi_irq.h>
#include <stdio.h>
#include <string.h>
#include <drv/sm2.h>
#include <drv/common.h>
#include "sfxlpka.h"
#include "c_sfxlpka.h"
#include "c_eip28.h"
#include "rambus.h"
#include "rambus_eip_sm2.h"
#include "eip28_internal.h"
#include "eip28_level0.h"
#include "adapter_eip28.h"

static const uint32_t g_param[70] = {
    0x08f1dfc3, 0x722edb8b, 0x5c45517d, 0x45728391, 0xbf6ff7de, 0xe8b92435,
    0x4c044f18, 0x8542d69e, 0x00000000, 0x00000000, 0x3937e498, 0xec65228b,
    0x6831d7e0, 0x2f3c848b, 0x73bbfeff, 0x2417842e, 0xfa32c3fd, 0x787968b4,
    0x00000000, 0x00000000, 0x27c5249a, 0x6e12d1da, 0xb16ba06e, 0xf61d59a5,
    0x484bfe48, 0x9cf84241, 0xb23b0c84, 0x63e4c6d3, 0x00000000, 0x00000000,
    0xc32e79b7, 0x5ae74ee7, 0x0485628d, 0x29772063, 0xbf6ff7dd, 0xe8b92435,
    0x4c044f18, 0x8542d69e, 0x00000000, 0x00000000, 0x7fedd43d, 0x4c4e6c14,
    0xadd50bdc, 0x32220b3b, 0xc3cc315e, 0x746434eb, 0x1b62eab6, 0x421debd6,
    0x00000000, 0x00000000, 0xe46e09a2, 0xa85841b9, 0xbfa36ea1, 0xe5d7fdfc,
    0x153b70c4, 0xd47349d2, 0xcbb42c07, 0x0680512b, 0x00000000, 0x00000000,
    0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000,
};

static EIP28_Status_t eip_sm2_config()
{

        EIP28_BigUInt_t BigUInt;
        EIP28_Status_t  ret   = -1;
        EIP28_IOArea_t *parea = &Adapter_EIP28_IOArea;

        BigUInt.StoreAsMSB = false;

        BigUInt.Bytes_p   = (uint8_t *)g_param;
        BigUInt.ByteCount = 70 * 4; // Bytes
        ret               = EIP28_Memory_PutBigUInt_CALLATOMIC(
            (EIP28_IOArea_t *)parea, EIP150_SM2_P_OFFSET, 70, &BigUInt, NULL);
        if (ret != EIP28_STATUS_OK) {
                TRACE_ERR(ret);
                return ret;
        }

        return EIP28_STATUS_OK;
}

static void eip_sm2_read_word(Device_Handle_t Device, unsigned int StartWord,
                              unsigned int BytesCount, uint8_t *Bytes_p)
{
        // we process the byte stream from start to tail
        // but write the words in reverse order
        uint32_t addr;
        uint8_t *p = Bytes_p + BytesCount - 4;

        LOG_INFO("===%s, %d, %x, : %d\n", __FUNCTION__, __LINE__, StartWord,
                 BytesCount);
        addr = StartWord;
        //StartWord += (BytesCount >> 2);
        while (BytesCount >= 4) {
                uint32_t data = EIP28_PKARAM_RD(Device, addr);

                *p++ = (uint8_t)(data >> 24);
                *p++ = (uint8_t)(data >> 16);
                *p++ = (uint8_t)(data >> 8);
                *p++ = (uint8_t)(data >> 0);
                addr += 1;
                p -= 8;
                BytesCount -= 4;
        }
}

EIP28_Status_t eip28_memory_getbiguint(EIP28_IOArea_t *const    IOArea_p,
                                       const EIP28_WordOffset_t StartWord,
                                       const unsigned int       NrOfWords,
                                       EIP28_BigUInt_t *const   BigUInt_p)
{
        EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

        EIP28_CHECK_POINTER(BigUInt_p);
        EIP28_CHECK_POINTER(BigUInt_p->Bytes_p);
        EIP28_CHECK_VECTOR_LIMIT(StartWord, NrOfWords);

        eip_sm2_read_word(Device, StartWord, NrOfWords, BigUInt_p->Bytes_p);

        IDENTIFIER_NOT_USED(TrueIOArea_p);

        EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}

static void eip_sm2_write_word(Device_Handle_t Device, unsigned int StartWord,
                               unsigned int BytesCount, uint8_t *Bytes_p)
{
        uint32_t addr;
        uint8_t *p = Bytes_p + BytesCount - 4;

        LOG_INFO("===%s, %d, %x, : %d\n", __FUNCTION__, __LINE__, StartWord,
                 BytesCount);
        addr = StartWord;
        while (BytesCount >= 4) {
                uint32_t data =
                    EIP28_PKARAM_MAKE_WORD_FROM_BYTESPTR_MSB_FIRST(p);
                EIP28_PKARAM_WR(Device, addr, data);
                addr += 1;
                p -= 4;
                BytesCount -= 4;
        }
}

static EIP28_Status_t eip_sm2_memory_putbiguint(
    EIP28_IOArea_t *const IOArea_p, const EIP28_WordOffset_t StartWord,
    const unsigned int           NrOfWordsToFill, // 0 if not used
    const EIP28_BigUInt_t *const BigUInt_p,
    EIP28_WordOffset_t *const    LastWordUsed_p) // NULL allowed
{
        EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

        EIP28_CHECK_POINTER(BigUInt_p);
        EIP28_CHECK_POINTER(BigUInt_p->Bytes_p);
        EIP28_CHECK_VECTOR_LIMIT(StartWord, 1 + (BigUInt_p->ByteCount - 1) / 4);

        if (NrOfWordsToFill > 0) {
                EIP28_CHECK_VECTOR_LIMIT(StartWord, NrOfWordsToFill);
                EIP28_CHECK_INT_ATMOST((unsigned int)1 +
                                           (BigUInt_p->ByteCount - 1) / 4,
                                       NrOfWordsToFill);
        }
        LOG_INFO("===%s, %d, %x, : %d\n", __FUNCTION__, __LINE__, StartWord,
                 BigUInt_p->ByteCount);
        eip_sm2_write_word(Device, StartWord, BigUInt_p->ByteCount,
                           BigUInt_p->Bytes_p);

        IDENTIFIER_NOT_USED(TrueIOArea_p);

        EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}

static EIP28_Status_t eip_sm2_sign_wirte(unsigned long     addr,
                                         SfxlPKA_Vector_t *d,
                                         SfxlPKA_Vector_t *e,
                                         SfxlPKA_Vector_t *k)
{
        EIP28_BigUInt_t BigUInt;
        EIP28_Status_t  ret   = -1;
        EIP28_IOArea_t *parea = &Adapter_EIP28_IOArea;

        BigUInt.StoreAsMSB = true;

        BigUInt.Bytes_p   = (uint8_t *)e->Word_p;
        BigUInt.ByteCount = e->WordCount * 4; // Bytes
        ret               = eip_sm2_memory_putbiguint((EIP28_IOArea_t *)parea,
                                        EIP150_SM2_E_OFFSET, e->WordCount,
                                        &BigUInt, NULL);
        if (ret != EIP28_STATUS_OK) {
                TRACE_ERR(ret);
                return ret;
        }
        TRACE();

        BigUInt.Bytes_p   = (uint8_t *)d->Word_p;
        BigUInt.ByteCount = d->WordCount * 4; // Bytes
        ret               = eip_sm2_memory_putbiguint((EIP28_IOArea_t *)parea,
                                        EIP150_SM2_D_OFFSET, d->WordCount,
                                        &BigUInt, NULL);
        if (ret != EIP28_STATUS_OK) {
                TRACE_ERR(ret);
                return ret;
        }
        TRACE();

        BigUInt.Bytes_p   = (uint8_t *)k->Word_p;
        BigUInt.ByteCount = k->WordCount * 4; // Bytes
        ret               = eip_sm2_memory_putbiguint((EIP28_IOArea_t *)parea,
                                        EIP150_SM2_K_OFFSET, k->WordCount,
                                        &BigUInt, NULL);
        if (ret != EIP28_STATUS_OK) {
                TRACE_ERR(ret);
                return ret;
        }

        return EIP28_STATUS_OK;
}

static EIP28_Status_t eip_sm2_sign_start(unsigned long addr)
{

        Device_Handle_t dev = (Device_Handle_t)addr;

        // write the offset of operand A, operand B, operand C and result
        EIP28_A_PTR_WR(dev, EIP150_SM2_D_OFFSET);
        EIP28_B_PTR_WR(dev, EIP150_SM2_P_OFFSET);
        EIP28_C_PTR_WR(dev, EIP150_SM2_E_OFFSET);
        EIP28_D_PTR_WR(dev, EIP150_SM2_K_OFFSET);

        // write the ALen, BLen
        EIP28_A_LEN_WR(dev, 32 / 4);
        EIP28_B_LEN_WR(dev, 32 / 4);

        RB_PERF_START_POINT();
        // Set the modexpvar bit & run bit in PKA_FUNCTION Register
        EIP28_FUNCTION_START_OPERATION(dev, EIP28_FUNCTION_SEQ_SM2_SIGN);

        return EIP28_STATUS_OK;
}

static EIP28_Status_t eip_sm2_sign_read(unsigned long addr, SfxlPKA_Vector_t *r)
{
        EIP28_BigUInt_t BigUInt;
        EIP28_Status_t  ret    = EIP28_ERROR;
        EIP28_IOArea_t *parea  = &Adapter_EIP28_IOArea;
        bool            IsDone = false;

        unsigned int LoopCount = SFXL_PKA_BUSYWAIT_COUNT;

        while (LoopCount > 0) {
                EIP28_CheckIfDone((EIP28_IOArea_t *)parea, &IsDone);
                if (IsDone) {
                        ret = EIP28_STATUS_OK;
                        break; // success
                } else {

                        SfxlPKALib_uSeconds_Wait(SFXL_PKA_DELAY_USEC);
                        LoopCount--;
                }
        } // while
        RB_PERF_END_POINT();
        ret = EIP28_Status_SeqCntrl_CALLATOMIC((EIP28_IOArea_t *)parea);
        // See if an error occurred
        if (ret > 1) {
                // Check if status is "InvalidTrigger"
                if (ret == 0x21) {
                        TRACE_ERR(ret); //PKA_ERROR_NOT_IMPLEMENTED
                        return EIP28_ERROR;
                }
                TRACE_ERR(ret);
                return EIP28_ERROR;
        }
        CHECK_RET_WITH_RET(ret == 1, ret);
        // Read the Actual result length
        BigUInt.StoreAsMSB = true;
        BigUInt.ByteCount  = 32 * 4;
        BigUInt.Bytes_p    = (uint8_t *)r->Word_p;
        ret                = eip28_memory_getbiguint((EIP28_IOArea_t *)parea,
                                      EIP150_SM2_R_OFFSET, 32, &BigUInt);
        if (ret != EIP28_STATUS_OK) {
                TRACE_ERR(ret);
                return EIP28_ERROR;
        }
        BigUInt.Bytes_p = (uint8_t *)r->Word_p + 32;
        ret             = eip28_memory_getbiguint((EIP28_IOArea_t *)parea,
                                      EIP150_SM2_S_OFFSET, 32, &BigUInt);
        if (ret != EIP28_STATUS_OK) {
                TRACE_ERR(ret);
                return EIP28_ERROR;
        }

        return ret;
}

EIP28_Status_t eip_sm2_sign(unsigned long addr, SfxlPKA_Vector_t *d,
                            SfxlPKA_Vector_t *e, SfxlPKA_Vector_t *k,
                            SfxlPKA_Vector_t *r)
{

        EIP28_Status_t ret = EIP28_STATUS_OK;

        TRACE();

        ret = eip_sm2_config();
        CHECK_RET_WITH_RET(ret == EIP28_STATUS_OK, ret);

        ret = eip_sm2_sign_wirte(addr, d, e, k);
        CHECK_RET_WITH_RET(ret == EIP28_STATUS_OK, ret);

        TRACE();
        ret = eip_sm2_sign_start(addr);
        CHECK_RET_WITH_RET(ret == EIP28_STATUS_OK, ret);

        TRACE();
        ret = eip_sm2_sign_read(addr, r);
        CHECK_RET_WITH_RET(ret == EIP28_STATUS_OK, ret);

        TRACE();
        return ret;
}

static EIP28_Status_t eip_sm2_verify_wirte(unsigned long     addr,
                                           SfxlPKA_Vector_t *pub,
                                           SfxlPKA_Vector_t *e,
                                           SfxlPKA_Vector_t *sig)
{
        EIP28_BigUInt_t BigUInt;
        EIP28_Status_t  ret   = -1;
        EIP28_IOArea_t *parea = &Adapter_EIP28_IOArea;
        uint32_t        z[9]  = {0};
        uint32_t        rz    = 1;
        //pub
        memset((uint8_t *)z, 0, 8);
        BigUInt.Bytes_p   = (uint8_t *)pub->Word_p;
        BigUInt.ByteCount = pub->WordCount * 4 / 2; // Bytes
        ret               = eip_sm2_memory_putbiguint((EIP28_IOArea_t *)parea,
                                        EIP150_SM2_V_PUB_OFFSET,
                                        pub->WordCount / 2, &BigUInt, NULL);
        if (ret != EIP28_STATUS_OK) {
                TRACE_ERR(ret);
                return ret;
        }
        BigUInt.Bytes_p   = (uint8_t *)z;
        BigUInt.ByteCount = 2 * 4; // Bytes
        ret               = eip_sm2_memory_putbiguint((EIP28_IOArea_t *)parea,
                                        EIP150_SM2_V_PUB_OFFSET + (32 / 4), 2,
                                        &BigUInt, NULL);
        if (ret != EIP28_STATUS_OK) {
                TRACE_ERR(ret);
                return ret;
        }
        BigUInt.Bytes_p   = (uint8_t *)pub->Word_p + 32;
        BigUInt.ByteCount = pub->WordCount * 4 / 2; // Bytes
        ret               = eip_sm2_memory_putbiguint((EIP28_IOArea_t *)parea,
                                        EIP150_SM2_V_PUB_OFFSET + (32 / 4 + 2),
                                        pub->WordCount / 2, &BigUInt, NULL);
        if (ret != EIP28_STATUS_OK) {
                TRACE_ERR(ret);
                return ret;
        }
        BigUInt.Bytes_p   = (uint8_t *)z;
        BigUInt.ByteCount = 2 * 4; // Bytes
        ret               = eip_sm2_memory_putbiguint((EIP28_IOArea_t *)parea,
                                        EIP150_SM2_V_PUB_OFFSET + (64 / 4 + 2),
                                        2, &BigUInt, NULL);
        if (ret != EIP28_STATUS_OK) {
                TRACE_ERR(ret);
                return ret;
        }
        BigUInt.StoreAsMSB = false;
        BigUInt.Bytes_p    = (uint8_t *)&rz;
        BigUInt.ByteCount  = 1 * 4; // Bytes
        ret                = EIP28_Memory_PutBigUInt_CALLATOMIC(
            (EIP28_IOArea_t *)parea, EIP150_SM2_V_PUB_OFFSET + (64 / 4 + 4), 1,
            &BigUInt, NULL);
        if (ret != EIP28_STATUS_OK) {
                TRACE_ERR(ret);
                return ret;
        }
        BigUInt.Bytes_p   = (uint8_t *)z;
        BigUInt.ByteCount = 9 * 4; // Bytes
        ret               = eip_sm2_memory_putbiguint((EIP28_IOArea_t *)parea,
                                        EIP150_SM2_V_PUB_OFFSET + (64 / 4 + 5),
                                        9, &BigUInt, NULL);
        if (ret != EIP28_STATUS_OK) {
                TRACE_ERR(ret);
                return ret;
        }
        TRACE();

        //e
        BigUInt.StoreAsMSB = true;
        BigUInt.Bytes_p    = (uint8_t *)e->Word_p;
        BigUInt.ByteCount  = e->WordCount * 4; // Bytes
        ret                = eip_sm2_memory_putbiguint((EIP28_IOArea_t *)parea,
                                        EIP150_SM2_V_E_OFFSET, e->WordCount,
                                        &BigUInt, NULL);
        if (ret != EIP28_STATUS_OK) {
                TRACE_ERR(ret);
                return ret;
        }
        BigUInt.Bytes_p   = (uint8_t *)z;
        BigUInt.ByteCount = 2 * 4; // Bytes
        ret               = eip_sm2_memory_putbiguint((EIP28_IOArea_t *)parea,
                                        EIP150_SM2_V_E_OFFSET + (32 / 4), 2,
                                        &BigUInt, NULL);
        if (ret != EIP28_STATUS_OK) {
                TRACE_ERR(ret);
                return ret;
        }
        TRACE();

        //sig
        BigUInt.Bytes_p   = (uint8_t *)sig->Word_p;
        BigUInt.ByteCount = sig->WordCount * 4 / 2; // Bytes
        ret               = eip_sm2_memory_putbiguint((EIP28_IOArea_t *)parea,
                                        EIP150_SM2_V_S_OFFSET,
                                        sig->WordCount / 2, &BigUInt, NULL);
        if (ret != EIP28_STATUS_OK) {
                TRACE_ERR(ret);
                return ret;
        }
        BigUInt.Bytes_p   = (uint8_t *)z;
        BigUInt.ByteCount = 2 * 4; // Bytes
        ret               = eip_sm2_memory_putbiguint((EIP28_IOArea_t *)parea,
                                        EIP150_SM2_V_S_OFFSET + (32 / 4), 2,
                                        &BigUInt, NULL);
        if (ret != EIP28_STATUS_OK) {
                TRACE_ERR(ret);
                return ret;
        }
        BigUInt.Bytes_p   = (uint8_t *)sig->Word_p + 32;
        BigUInt.ByteCount = sig->WordCount * 4 / 2; // Bytes
        ret               = eip_sm2_memory_putbiguint((EIP28_IOArea_t *)parea,
                                        EIP150_SM2_V_S_OFFSET + (32 / 4 + 2),
                                        sig->WordCount / 2, &BigUInt, NULL);
        if (ret != EIP28_STATUS_OK) {
                TRACE_ERR(ret);
                return ret;
        }
        BigUInt.Bytes_p   = (uint8_t *)z;
        BigUInt.ByteCount = 2 * 4; // Bytes
        ret               = eip_sm2_memory_putbiguint((EIP28_IOArea_t *)parea,
                                        EIP150_SM2_V_S_OFFSET + (64 / 4 + 2), 2,
                                        &BigUInt, NULL);
        if (ret != EIP28_STATUS_OK) {
                TRACE_ERR(ret);
                return ret;
        }

        return EIP28_STATUS_OK;
}

static EIP28_Status_t eip_sm2_verify_start(unsigned long addr)
{

        Device_Handle_t dev = (Device_Handle_t)addr;
        // write the offset of operand A, operand B, operand C and result
        EIP28_A_PTR_WR(dev, EIP150_SM2_V_PUB_OFFSET);
        EIP28_B_PTR_WR(dev, EIP150_SM2_V_P_OFFSET);
        EIP28_C_PTR_WR(dev, EIP150_SM2_V_E_OFFSET);
        EIP28_D_PTR_WR(dev, EIP150_SM2_V_S_OFFSET);

        // write the ALen, BLen
        EIP28_A_LEN_WR(dev, 32 / 4);
        EIP28_B_LEN_WR(dev, 32 / 4);

        // Set the modexpvar bit & run bit in PKA_FUNCTION Register
        RB_PERF_START_POINT();
        EIP28_FUNCTION_START_OPERATION(dev, EIP28_FUNCTION_SEQ_SM2_VERIFY);

        return EIP28_STATUS_OK;
}

static EIP28_Status_t eip_sm2_verify_read(unsigned long addr)
{
        EIP28_Status_t        ret       = EIP28_ERROR;
        EIP28_IOArea_t *      parea     = &Adapter_EIP28_IOArea;
        bool                  IsDone    = false;
        unsigned int          LoopCount = SFXL_PKA_BUSYWAIT_COUNT;
        EIP28_CompareResult_t res;

        while (LoopCount > 0) {
                EIP28_CheckIfDone((EIP28_IOArea_t *)parea, &IsDone);
                if (IsDone) {
                        ret = EIP28_STATUS_OK;
                        break; // success
                } else {

                        SfxlPKALib_uSeconds_Wait(SFXL_PKA_DELAY_USEC);
                        LoopCount--;
                }
        } // while
        RB_PERF_END_POINT();
        ret = EIP28_Status_SeqCntrl_CALLATOMIC((EIP28_IOArea_t *)parea);
        // See if an error occurred
        if (ret > 1) {
                // Check if status is "InvalidTrigger"
                if (ret == 0x21) {
                        TRACE_ERR(ret); //PKA_ERROR_NOT_IMPLEMENTED
                        return EIP28_ERROR;
                }
                TRACE_ERR(ret);
                return EIP28_ERROR;
        }
        CHECK_RET_WITH_RET(ret == 1, ret);

        EIP28_ReadResult_Compare(parea, &res);
        if (res == EIP28_COMPARERESULT_A_EQUALS_B) {
                ret = EIP28_STATUS_OK;
        } else {
                ret = EIP28_ERROR;
                TRACE_ERR(ret);
        }

        return ret;
}

EIP28_Status_t eip_sm2_verify(unsigned long addr, SfxlPKA_Vector_t *pub,
                              SfxlPKA_Vector_t *e, SfxlPKA_Vector_t *sig)
{

        EIP28_Status_t ret = EIP28_STATUS_OK;

        TRACE();

        ret = eip_sm2_config();
        CHECK_RET_WITH_RET(ret == EIP28_STATUS_OK, ret);

        ret = eip_sm2_verify_wirte(addr, pub, e, sig);
        CHECK_RET_WITH_RET(ret == EIP28_STATUS_OK, ret);

        TRACE();
        ret = eip_sm2_verify_start(addr);
        CHECK_RET_WITH_RET(ret == EIP28_STATUS_OK, ret);

        TRACE();
        ret = eip_sm2_verify_read(addr);
        CHECK_RET_WITH_RET(ret == EIP28_STATUS_OK, ret);

        TRACE();
        return ret;
}

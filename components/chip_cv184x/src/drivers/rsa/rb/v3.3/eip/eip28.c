/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */
/* eip28.c
 *
 * Driver Library for the Security-IP-EIP-28 Public Key Accelerator.
 */



/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

// Public EIP28 Driver Library API, all functions except
// EIP28_GetWorkAreaSize and EIP28_StartOp_ModExp_CupAmodB_CALLATOMIC
#include "eip28.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "c_eip28.h" // configuration options

#include "basic_defs.h" // uint32_t, bool, IDENTIFIER_NOT_USED

#include "device_types.h" // Device_Handle_t

#include "eip28_level0.h" // EIP28 register access

#include "eip28_internal.h" // Internal definitions.

/*----------------------------------------------------------------------------
 * EIP28_HwFwAreCompatible
 *
 * This routine checks if the Hardware is compatible with the Firmware.
 *
 * Return type: bool
 *      true    = Hardware is     compatible with the Firmware
 *      false   = Hardware is not compatible with the Firmware
 */
#ifndef EIP28_REMOVE_FIRMWARE_DOWNLOAD
static bool
EIP28Lib_HwFwAreCompatible(
    const uint32_t HwOptions,
    const uint32_t FwRequirements,
    unsigned int FwWordCount)
{
    // Program RAM or ROM
    switch (EIP28_PRG_RAM(HwOptions))
    {
    case EIP28_OPTION_PRG_ROM:
        // FW is in ROM
        return false;

    default:
        // no test here whether FW fits
        break;
    }

    // PKCP configurations
    switch (EIP28_PKCP_CONF(HwOptions))
    {
    case EIP28_OPTION_PKCP_16x16_MULTIPLIER:
        switch (EIP28_FW_PKCP_TYPE(FwRequirements))
        {
        case EIP28_FW_PKCP_TYPE_16x16:
            break;
        case EIP28_FW_PKCP_TYPE_DO_NOT_CARE:
            break;
        default:
            return false;
        } // switch
        break;

    case EIP28_OPTION_PKCP_32x32_MULTIPLIER:
        switch (EIP28_FW_PKCP_TYPE(FwRequirements))
        {
        case EIP28_FW_PKCP_TYPE_32x32:
            break;
        case EIP28_FW_PKCP_TYPE_DO_NOT_CARE:
            break;
        default:
            return false;
        } // switch
        break;

    default:
        return false;
    } // switch

    // LNME configurations
    switch (EIP28_LNME_CONF(HwOptions))
    {
    case EIP28_OPTION_LNME_NO:
        // no LNME available; FW should not required one
        if (EIP28_FW_NR_LNME(FwRequirements) == 0)
            break;
        return false;

    default:
        // HW must contain enough LNME's for this FW
        if (EIP28_LNME_CONF(HwOptions) >= EIP28_FW_NR_LNME(FwRequirements))
        {
            // HW should contain enough PE's
            if (EIP28_1_LNME_NR_PE(HwOptions) >=
                EIP28_FW_LONGEST_LNME_PE_CHAIN(FwRequirements))
                break;
        }
        return false;
    } // switch

    IDENTIFIER_NOT_USED(FwWordCount);

    return true;
}
#endif /* EIP28_REMOVE_FIRMWARE_DOWNLOAD */

/*----------------------------------------------------------------------------
 * EIP28_Firmware_Download_Util
 *
 * This routine downloads the firmware from given firmware reference
 * location of given firmware size to the PKA RAM of EIP28.
 * It also checks if the firmware is downloaded was OK by reading back
 * PKA RAM after download.
 *
 * Return type: bool
 *      true    = Firmware download successful
 *      false   = Firmware download failed
 */
#ifndef EIP28_REMOVE_FIRMWARE_DOWNLOAD
static bool
EIP28Lib_Firmware_Download_Util(
    Device_Handle_t Device,
    const uint32_t *const Firmware_p,
    const unsigned int FirmwareWordCount)
{
    uint32_t data;
    unsigned int word_cntr;
    const uint32_t *Fw_src_ptr = Firmware_p;

    if (NULL == Firmware_p)
        return false;

    // Hold sequencer in RESET state to allow R/W access of PKA PROGRAM RAM
    EIP28_SEQCTRL_RESET_SET(Device);

    for (word_cntr = 0; word_cntr < FirmwareWordCount; word_cntr++)
    {
        if (!(word_cntr & ~EIP28_FWRAM_PAGE_SIZE))
        {
            // Set correct page
            EIP28_SEQCTRL_WR(Device, EIP28_SEQCTRL_RESET | ((word_cntr & 0x1800) << 18));
        }
        EIP28_PKARAM_FW_WR(Device, word_cntr, *Fw_src_ptr);
        Fw_src_ptr++;
    }

    Fw_src_ptr = Firmware_p;
    for (word_cntr = 0; word_cntr < FirmwareWordCount; word_cntr++)
    {
        if (!(word_cntr & ~EIP28_FWRAM_PAGE_SIZE))
        {
            // Set correct page
            EIP28_SEQCTRL_WR(Device, EIP28_SEQCTRL_RESET | ((word_cntr & 0x1800) << 18));
        }
        data = EIP28_PKARAM_FW_RD(Device, word_cntr);
        if (data != *Fw_src_ptr++)
        {
            return false;
        }
    }

    // Release sequencer from RESET state to allow EIP28
    // to execute firmware.
    EIP28_SEQCTRL_RESET_CLR(Device);

    return true;
}
#endif /* EIP28_REMOVE_FIRMWARE_DOWNLOAD */

/*----------------------------------------------------------------------------
 * EIP28Lib_Detect
 *
 * Checks the presence of EIP28 hardware. Returns true when found.
 */
static bool
EIP28Lib_Detect(
    Device_Handle_t Device,
    bool fSkipFWCheck)
{
#ifndef EIP28_REMOVE_EIPNR_CHECK
    uint32_t sw_ver;
    // read and check the revision register
    if (!EIP28_REVISION_CHECK_EIP_NR(Device))
    {
        LOG_CRIT("err %s, %d\n", __FUNCTION__, __LINE__);
        return false;
    }

    if (!fSkipFWCheck)
    {
        // read and check the firmware revision register
        // if it is zero, the firmware has not been loaded & started yet
        if ((sw_ver = EIP28_Read32(Device, EIP28_OFFSET_SW_REV)) == 0)
        {
            LOG_CRIT("err %s, %d, 0x%x\n", __FUNCTION__, __LINE__, sw_ver);
            return false;
        }
        LOG_CRIT("%s, %d, 0x%x\n", __FUNCTION__, __LINE__, sw_ver);
    }
#endif

    return true;
}

/*----------------------------------------------------------------------------
 * EIP28Lib_Check
 *
 * Sanity checks
 */
static bool
EIP28Lib_SanityCheck(
    Device_Handle_t Device)
{
    // Stop the engine (in case it was running)
    EIP28_Write32(Device, EIP28_OFFSET_FUNCTION, 0);

    // Make sure PKA RAM is accessible
    EIP28_MakePkaRamAccessible(Device);

    // read-write test one of the registers
    {
        uint32_t Value;

        // Set all defined bits of the PKA_APTR register
        EIP28_Write32(Device, EIP28_OFFSET_A_PTR, (MASK_11_BITS - BIT_0));
        Value = EIP28_Read32(Device, EIP28_OFFSET_A_PTR);
        if ((Value & MASK_11_BITS) != (MASK_11_BITS - BIT_0)) {
            LOG_CRIT("err %s, %d, 0x%x\n", __FUNCTION__, __LINE__, Value);
            return false;
        }

        // Clear all defined bits of the PKA_APTR register
        EIP28_Write32(Device, EIP28_OFFSET_A_PTR, 0);
        Value = EIP28_Read32(Device, EIP28_OFFSET_A_PTR);
        if ((Value & MASK_11_BITS) != 0) {
            LOG_CRIT("err %s, %d, 0x%x\n", __FUNCTION__, __LINE__, Value);
            return false;
        }
    }

    // read-write test one of the PKA RAM memory locations
    {
        uint32_t Value;

        // Write a test values to the PKA RAM
        EIP28_PKARAM_WR(Device, 0, 0x55555555);
        Value = EIP28_PKARAM_RD(Device, 0);
        if (Value != 0x55555555) {
            LOG_CRIT("err %s, %d, 0x%x\n", __FUNCTION__, __LINE__, Value);
            return false;
        }

        // Write a test values to the PKA RAM
        EIP28_PKARAM_WR(Device, 0, 0xAAAAAAAA);
        Value = EIP28_PKARAM_RD(Device, 0);
        if (Value != 0xAAAAAAAA)
        if (Value != 0x55555555) {
            LOG_CRIT("err %s, %d, 0x%x\n", __FUNCTION__, __LINE__, Value);
            return false;
        }

        // Clean up
        EIP28_PKARAM_WR(Device, 0, 0);
    }

    return true;
}

/*----------------------------------------------------------------------------
 * EIP28Lib_PkaRam_ProbeSize
 *
 * This function returns the actual size (in words) of the EIP28 PKA-RAM,
 * expected to be 1KByte, 2KByte, 4KByte or 8KByte. If the PKA-RAM malfunctions,
 * a size of 0 is returned.

 */
#define EIP28LIB_TESTWORD 0x7E57DA7AU
// Minimum memory size and check RAM with steps of 1 KB
#define EIP28LIB_PKARAM_MIN_SIZE 0x400 / 4
// Maximum memory size is 8 KB
#define EIP28LIB_PKARAM_MAX_SIZE 0x2000 / 4
static unsigned int
EIP28Lib_PkaRam_ProbeSize(
    Device_Handle_t Device)
{
    uint32_t n;

    EIP28_PKARAM_WR(Device, 0, ~EIP28LIB_TESTWORD);

    for (n = EIP28LIB_PKARAM_MIN_SIZE; n < EIP28LIB_PKARAM_MAX_SIZE; n <<= 1)
    {
        if (EIP28_PKARAM_RD(Device, 0) != ~EIP28LIB_TESTWORD)
        {
            /* PKA-RAM appears broken. */
            n = 0;
            break;
        }

        EIP28_PKARAM_WR(Device, n, EIP28LIB_TESTWORD);

        if (EIP28_PKARAM_RD(Device, n) != EIP28LIB_TESTWORD)
        {
            /* PKA-RAM appears broken. */
            n = 0;
            break;
        }

        if (EIP28_PKARAM_RD(Device, 0) == EIP28LIB_TESTWORD)
        {
            /* Address 'n' maps on address 0 -> 'n' is actual size. */
            break;
        }
    } /* for */

    // LOG_CRIT("%s, %d, 0x%x\n", __FUNCTION__, __LINE__, n);
    return n;
}

/*----------------------------------------------------------------------------
 * EIP28_SetParams_Modulo
 *
 * Set parameters for modulo and ModuloOrCopy operations
 */
#if !defined(EIP28_REMOVE_MODULO) || \
    !defined(EIP28_REMOVE_MODULO_OR_COPY)
static EIP28_Status_t
EIP28Lib_SetParams_Modulo(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t A_wo,
    const EIP28_WordOffset_t B_wo,
    const unsigned int A_Len,
    const unsigned int B_Len,
    const EIP28_WordOffset_t Result_wo)
{
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

    // Result(C) Vector Length = B_Len + 1
    // check the offset A_wo, B_wo, Result_wo
    EIP28_CHECK_INT_ATMOST(A_wo + A_Len, TrueIOArea_p->PKARamNrOfWords);
    EIP28_CHECK_INT_ATMOST(B_wo + B_Len, TrueIOArea_p->PKARamNrOfWords);
    EIP28_CHECK_INT_ATMOST((Result_wo + B_Len + 1), TrueIOArea_p->PKARamNrOfWords);

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(A_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(B_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Result_wo);

    // Most significant 32 bit word of B operand cannot be zero.
    // Highest word of Modulus Vector shall not be zero
    CHECK_PKARAM_WORD_NOT_ZERO(B_wo, B_Len);

    // check for no overlap of vector C with  vector A
    EIP28_CHECK_NO_OVERLAP(A_wo, A_Len, Result_wo, (B_Len + 1));
    // check for no overlap of vector C with  vector B
    EIP28_CHECK_NO_OVERLAP(B_wo, B_Len, Result_wo, (B_Len + 1));

    // write the offset of operand A, operand B and result
    EIP28_A_PTR_WR(Device, A_wo);
    EIP28_B_PTR_WR(Device, B_wo);
    EIP28_C_PTR_WR(Device, Result_wo);
    TrueIOArea_p->LastOperation_Remainder_wo = Result_wo;
    TrueIOArea_p->LastOperationUsedSequencer = false;

    // write the ALen and BLen
    EIP28_A_LEN_WR(Device, A_Len);
    EIP28_B_LEN_WR(Device, B_Len);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif

void zeroize_pka_ram(
    Device_Handle_t Device)
{
        //Zeroize enable
        //0xffff200080, bit[0] ZEROIZE_RESET_N
        Device_Write32((void *)0xffff200080, 0, 1);
        
        while (((EIP28_SEQCTRL_RD(Device)) & 0x00040000) != 0x00040000)
                ;
        Device_Write32((void *)0xffff200080, 0, 0);
        //Wait for zeroize is done
        while (((EIP28_SEQCTRL_RD(Device)) & 0x00040000) != 0x00000000)
                ;
}

/*----------------------------------------------------------------------------
 * EIP28_Initialize
 */
EIP28_Status_t
EIP28_Initialize_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    Device_Handle_t Device,
    const uint32_t *const Firmware_p,
    unsigned int FirmwareWordCount)
{
    volatile EIP28_True_IOArea_t *const TrueIOArea_p = IOAREA(IOArea_p);
    bool fDownloadFW = false;

    TrueIOArea_p->Device = Device;
    TrueIOArea_p->LastOperation_Result_wo = 0;
    TrueIOArea_p->LastOperation_Remainder_wo = 0;
    TrueIOArea_p->LastOperationUsedSequencer = false;
    TrueIOArea_p->LastOperation_ResultLen = 0;

    // Init hardware
    EIP28_FUNCTION_WR(Device, 0); // disables all options

    fDownloadFW = (FirmwareWordCount && Firmware_p);

    // Detect presence of eip28 hardware
    if (!EIP28Lib_Detect(Device, /*SkipFWCheck:*/ fDownloadFW))
    {
        
        LOG_CRIT("err %s, %d, 0x%x\n", __FUNCTION__, __LINE__, 0);
        return EIP28_ERROR;
    }

    // Init hardware
    EIP28_FUNCTION_WR(Device, 0); // disables all options

    if (fDownloadFW)
    {
        // firmware provided
        // prior download, verify the firmware is compatible with the hardware
        uint32_t HwOptions;

        HwOptions = EIP28_Read32(Device, EIP28_OFFSET_OPTIONS);

        if (!EIP28Lib_HwFwAreCompatible(
                HwOptions,
                Firmware_p[FirmwareWordCount - 1], // expectations word
                FirmwareWordCount))
        {
            LOG_CRIT("err %s, %d, 0x%x\n", __FUNCTION__, __LINE__, 0);
            return EIP28_ERROR;
        }

        if (!EIP28Lib_Firmware_Download_Util(
                Device,
                Firmware_p,
                FirmwareWordCount))
        {
            LOG_CRIT("err %s, %d, 0x%x\n", __FUNCTION__, __LINE__, 0);
            return EIP28_ERROR;
        }
        // Set Memory size to FW size
        TrueIOArea_p->PKAProgNrOfWords = FirmwareWordCount;
    }

    // Determine actual size of PKA-RAM, and return an error
    // if the PKA-RAM doesn't appear to work at all.
    TrueIOArea_p->PKARamNrOfWords = EIP28Lib_PkaRam_ProbeSize(Device);
    if (TrueIOArea_p->PKARamNrOfWords == 0)
    {
        LOG_CRIT("err %s, %d, 0x%x\n", __FUNCTION__, __LINE__, 0);
        return EIP28_ERROR;
    }

    if (!EIP28Lib_SanityCheck(Device)) {
        LOG_CRIT("err %s, %d, 0x%x\n", __FUNCTION__, __LINE__, 0);
        return EIP28_ERROR;
    }

    zeroize_pka_ram(Device);
    

    return EIP28_STATUS_OK;
}

/*----------------------------------------------------------------------------
 * EIP28_GetRevisionInfo
 *
 * This function returns the revision information.
 */
#ifndef EIP28_REMOVE_EIPNR_CHECK
EIP28_Status_t
EIP28_GetRevisionInfo(
    EIP28_IOArea_t *const IOArea_p,
    EIP28_RevisionInfo_t *const Info_p)
{
    uint32_t V;
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

    if (Info_p == NULL)
        return EIP28_ERROR;

    V = EIP28_Read32(Device, EIP28_OFFSET_REVISION);
    Info_p->HW.Major = (V >> 24) & MASK_4_BITS;
    Info_p->HW.Minor = (V >> 20) & MASK_4_BITS;
    Info_p->HW.PatchLevel = (V >> 16) & MASK_4_BITS;

    V = EIP28_Read32(Device, EIP28_OFFSET_SW_REV);
    Info_p->FW.Major = (V >> 24) & MASK_4_BITS;
    Info_p->FW.Minor = (V >> 20) & MASK_4_BITS;
    Info_p->FW.PatchLevel = (V >> 16) & MASK_4_BITS;
    Info_p->FW.Capabilities = (V >> 28) & MASK_4_BITS;

    V = EIP28_Read32(Device, EIP28_OFFSET_OPTIONS);
    Info_p->Options = V;

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
    return EIP28_ERROR;
}
#endif // EIP28_REMOVE_EIPNR_CHECK

/*----------------------------------------------------------------------------
 * CheckReset_SeqCtrl
 *
 * Returns true If the reset bit in SeqCtrl equals 1,
 * Parity_Err and Hamming_Err are read from the SeqCtrl and set accordingly.
 *
 * Returns false if the reset bit in SeqCtrl is 0.
 */
bool EIP28_CheckReset_SeqCtrl(
    EIP28_IOArea_t *const IOArea_p,
    bool *const Parity_Err_p,
    bool *const Hamming_Err_p)
{
    volatile EIP28_True_IOArea_t *const TrueIOArea_p = IOAREA(IOArea_p);
    Device_Handle_t Device = TrueIOArea_p->Device;
    uint32_t Seq_CTRL = EIP28_SEQCTRL_RD(Device);

    bool rv = Seq_CTRL & EIP28_SEQCTRL_RESET;
    if (rv)
    {
        *Parity_Err_p = Seq_CTRL & EIP28_SEQCTRL_PARITY_ERR;
        *Hamming_Err_p = Seq_CTRL & EIP28_SEQCTRL_HAMMING_ERR;
    }

    return rv;
}

/*----------------------------------------------------------------------------
 * CheckIfDone
 *
 * Sets IsDone_p to true when the EIP28 is idle or false when it is running.
 */
void EIP28_CheckIfDone(
    EIP28_IOArea_t *const IOArea_p,
    bool *const IsDone_p)
{
    // cannot use EIP28_INSERTCODE_FUNCTION_ENTRY_CODE due to "void" return
    volatile EIP28_True_IOArea_t *const TrueIOArea_p = IOAREA(IOArea_p);
    Device_Handle_t Device = TrueIOArea_p->Device;

    // we cannot test if it is done
    // but we can test if it is still running
    if (EIP28_FUNCTION_ISRUNNING(Device))
        *IsDone_p = false;
    else
        *IsDone_p = true;
}

/*----------------------------------------------------------------------------
 * EIP28_SetSCAP_DummyCorr_CALLATOMIC
 */
void EIP28_SetSCAP_DummyCorr_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const bool Dummy_Corr)
{
    // cannot use EIP28_INSERTCODE_FUNCTION_ENTRY_CODE due to "void" return
    volatile EIP28_True_IOArea_t *const TrueIOArea_p = IOAREA(IOArea_p);
    Device_Handle_t Device = TrueIOArea_p->Device;

    EIP28_SCAP_CTRL_SET(Device, Dummy_Corr);
}

/*----------------------------------------------------------------------------
 * ReadResult_WordCount
 */
void EIP28_ReadResult_WordCount_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    unsigned int *const Result_NrOfWords_p)
{
    // cannot use EIP28_INSERTCODE_FUNCTION_ENTRY_CODE due to "void" return
    volatile EIP28_True_IOArea_t *const TrueIOArea_p = IOAREA(IOArea_p);
    Device_Handle_t Device = TrueIOArea_p->Device;

    if (NULL != Result_NrOfWords_p)
    {
        if (false == TrueIOArea_p->LastOperationUsedSequencer)
        {
            uint32_t M = EIP28_MSW_RD(Device);

            if ((M & EIP28_MSW_RESULTISZERO) != 0)
            {
                // must ignore the address part of the field
                M = 0;
            }
            else
            {
                M &= EIP28_MSW_ADDRESS_MASK;
                M = M - TrueIOArea_p->LastOperation_Result_wo + 1;
            }

            *Result_NrOfWords_p = M;
        }
        else
        {
            // MSW register is not updated for sequencer-based operations
            uint32_t W;
            uint32_t wo = TrueIOArea_p->LastOperation_Result_wo +
                          TrueIOArea_p->LastOperation_ResultLen;

            do
            {
                wo--;
                W = EIP28_PKARAM_RD(Device, wo);
            } while ((W == 0) && (wo > TrueIOArea_p->LastOperation_Result_wo));

            *Result_NrOfWords_p = 1 + (wo - TrueIOArea_p->LastOperation_Result_wo);
        }
    }
}

/*----------------------------------------------------------------------------
 * ReadResult_RemainderWordCount
 *
 * To be used for Divide and Modulo only.
 */
#ifndef EIP28_REMOVE_READRESULT_REMAINDER
void EIP28_ReadResult_RemainderWordCount_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    unsigned int *const Remainder_NrOfWords_p)
{
    volatile EIP28_True_IOArea_t *const TrueIOArea_p = IOAREA(IOArea_p);
    Device_Handle_t Device = TrueIOArea_p->Device;

    if (NULL != Remainder_NrOfWords_p)
    {
        if (false == TrueIOArea_p->LastOperationUsedSequencer)
        {
            uint32_t M = EIP28_DIVMSW_RD(Device);

            if (EIP28_FUNCTION_RD(Device) & EIP28_FUNCTION_COPY)
            {
                // if the preceding Modulo operation was actually done
                // via a Copy, use MSW instead of DIVMSW.
                M = EIP28_MSW_RD(Device);
            }

            if ((M & EIP28_DIVMSW_RESULTISZERO) != 0)
            {
                // must ignore the address part of the field
                M = 0;
            }
            else
            {
                M &= EIP28_DIVMSW_ADDRESS_MASK;
                M = M - TrueIOArea_p->LastOperation_Remainder_wo + 1;
            }

            *Remainder_NrOfWords_p = M;
        }
        else
        {
            // DIVMSW is not used for sequencer-based operations
            *Remainder_NrOfWords_p = 0;
        }
    }
}
#endif /* EIP28_REMOVE_READRESULT_REMAINDER */

/*----------------------------------------------------------------------------
 * ReadResult_ModInv_Status
 *
 * Use after StartOp_ModInv has completed to see if the inverse exists.
 */
#ifndef EIP28_REMOVE_MODINV
void EIP28_ReadResult_ModInv_Status(
    EIP28_IOArea_t *const IOArea_p,
    bool *const Result_IsUndefined_p)
{
    volatile EIP28_True_IOArea_t *const TrueIOArea_p = IOAREA(IOArea_p);
    Device_Handle_t Device = TrueIOArea_p->Device;

    // read the additional result code from the SHIFT register
    const uint32_t R = EIP28_SHIFT_RD(Device);

    // decode the result and set the out-parameters
    if (NULL != Result_IsUndefined_p)
    {
        if ((EIP28_SHIFT_RESULT_NO_INVERSE_EXISTS == R) ||
            (EIP28_SHIFT_RESULT_ERROR_MODULUS_EVEN == R))
        {
            *Result_IsUndefined_p = true;
        }
        else
        {
            *Result_IsUndefined_p = false;
        }
    }
}
#endif /* EIP28_REMOVE_MODINV */

/*----------------------------------------------------------------------------
 * ReadResult_ECCPoint_Status
 */
#ifndef EIP28_REMOVE_READRESULT_ECCSTATUS
void EIP28_ReadResult_ECCPoint_Status(
    EIP28_IOArea_t *const IOArea_p,
    bool *const Result_IsAtInfinity_p,
    bool *const Result_IsUndefined_p)
{
    volatile EIP28_True_IOArea_t *const TrueIOArea_p = IOAREA(IOArea_p);
    Device_Handle_t Device = TrueIOArea_p->Device;

    // read the additional result code from the SHIFT register
    const uint32_t R = EIP28_SHIFT_RD(Device);

    // decode the result and set the out-parameters
    if (NULL != Result_IsUndefined_p)
    {
        if (R == EIP28_SHIFT_RESULT_POINT_UNDEFINED)
            *Result_IsUndefined_p = true;
        else
            *Result_IsUndefined_p = false;
    }

    if (NULL != Result_IsAtInfinity_p)
    {
        if (R == EIP28_SHIFT_RESULT_POINT_AT_INFINITY)
            *Result_IsAtInfinity_p = true;
        else
            *Result_IsAtInfinity_p = false;
    }
}
#endif /* EIP28_REMOVE_READRESULT_ECCSTATUS */

/*----------------------------------------------------------------------------
 * Memory_GetWordCount
 */
#ifndef EIP28_REMOVE_BIGUINT_HELPER_FUNC
unsigned int
EIP28_Memory_GetWordCount(
    EIP28_IOArea_t *const IOArea_p)
{
    volatile EIP28_True_IOArea_t *const TrueIOArea_p = IOAREA(IOArea_p);

    return TrueIOArea_p->PKARamNrOfWords;
}
#endif /* EIP28_REMOVE_BIGUINT_HELPER_FUNC */

/*----------------------------------------------------------------------------
 * Memory_PutBigUInt
 */
#ifndef EIP28_REMOVE_BIGUINT_HELPER_FUNC
EIP28_Status_t
EIP28_Memory_PutBigUInt_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t StartWord,
    const unsigned int NrOfWordsToFill, // 0 if not used
    const EIP28_BigUInt_t *const BigUInt_p,
    EIP28_WordOffset_t *const LastWordUsed_p) // NULL allowed
{
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

    EIP28_CHECK_POINTER(BigUInt_p);
    EIP28_CHECK_POINTER(BigUInt_p->Bytes_p);
    EIP28_CHECK_VECTOR_LIMIT(StartWord, 1 + (BigUInt_p->ByteCount - 1) / 4);

    if (NrOfWordsToFill > 0)
    {
        EIP28_CHECK_VECTOR_LIMIT(StartWord, NrOfWordsToFill);
        EIP28_CHECK_INT_ATMOST(
            (unsigned int)1 + (BigUInt_p->ByteCount - 1) / 4,
            NrOfWordsToFill);
    }

    // function can handle length=0
    if (BigUInt_p->StoreAsMSB)
    {
        EIP28_PKARAM_WRITE_WORDS_FROM_BYTESPTR_MSB_FIRST(
            Device,
            StartWord,
            BigUInt_p->ByteCount,
            BigUInt_p->Bytes_p);
    }
    else
    {
        EIP28_PKARAM_WRITE_WORDS_FROM_BYTESPTR_LSB_FIRST(
            Device,
            StartWord,
            BigUInt_p->ByteCount,
            BigUInt_p->Bytes_p);
    }

    // calculate the last word used and pad with zero words, if required
    {
        // calculate offset following last word we must fill
        // note: adding -1 to wo_stop calculation can cause underflow
        uint32_t wo_stop = StartWord + NrOfWordsToFill;

        // calculate offset of last word we filled
        uint32_t wo = StartWord;

        if (BigUInt_p->ByteCount > 0)
            wo += ((BigUInt_p->ByteCount - 1) >> 2);

        // pad the bigint with zero words to fill the fixed-size
        // allocated space in PKA RAM
        while (wo + 1 < wo_stop)
        {
            wo++;
            EIP28_PKARAM_WR(Device, wo, 0);
        }

        if (NULL != LastWordUsed_p)
            *LastWordUsed_p = (EIP28_WordOffset_t)(wo);
    }

    IDENTIFIER_NOT_USED(TrueIOArea_p);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif /* EIP28_REMOVE_BIGUINT_HELPER_FUNC */

/*----------------------------------------------------------------------------
 * Memory_GetBigUInt
 */
#ifndef EIP28_REMOVE_BIGUINT_HELPER_FUNC
EIP28_Status_t
EIP28_Memory_GetBigUInt_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t StartWord,
    const unsigned int NrOfWords,
    EIP28_BigUInt_t *const BigUInt_p)
{
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

    EIP28_CHECK_POINTER(BigUInt_p);
    EIP28_CHECK_POINTER(BigUInt_p->Bytes_p);
    EIP28_CHECK_VECTOR_LIMIT(StartWord, NrOfWords);

    if (BigUInt_p->StoreAsMSB)
    {
        BigUInt_p->ByteCount = EIP28_PKARAM_READ_WORDS_TO_BYTESPTR_MSB_FIRST(
            Device,
            StartWord,
            NrOfWords,
            BigUInt_p->Bytes_p);
        // the above routine has already performed the trimming
    }
    else
    {
        EIP28_PKARAM_READ_WORDS_TO_BYTESPTR_LSB_FIRST(
            Device,
            StartWord,
            NrOfWords,
            BigUInt_p->Bytes_p);

        // trim the result: remove zero MSB's
        // at least 1 byte will be left
        {
            unsigned int n = NrOfWords * 4;
            while (n > 1 && BigUInt_p->Bytes_p[n - 1] == 0)
                n--;
            BigUInt_p->ByteCount = n;
        }
    }

    IDENTIFIER_NOT_USED(TrueIOArea_p);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif /* EIP28_REMOVE_BIGUINT_HELPER_FUNC */

/*----------------------------------------------------------------------------
 * Memory_GetBigUInt
 */
#ifndef EIP28_REMOVE_BIGUINT_ZP_HELPER_FUNC
EIP28_Status_t
EIP28_Memory_GetBigUInt_ZeroPad_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t StartWord,
    const unsigned int NrOfWords,
    EIP28_BigUInt_t *const BigUInt_p,
    const unsigned int WantedNumberLength_NrOfBytes)
{
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

    EIP28_CHECK_POINTER(BigUInt_p);
    EIP28_CHECK_POINTER(BigUInt_p->Bytes_p);
    EIP28_CHECK_VECTOR_LIMIT(StartWord, NrOfWords);

    if (BigUInt_p->StoreAsMSB)
    {
        BigUInt_p->ByteCount =
            EIP28_PKARAM_READ_WORDS_TO_BYTESPTR_MSB_FIRST_ZERO_PAD(
                Device,
                StartWord,
                NrOfWords,
                BigUInt_p->Bytes_p,
                WantedNumberLength_NrOfBytes);
    }
    else
    {
        EIP28_PKARAM_READ_WORDS_TO_BYTESPTR_LSB_FIRST(
            Device,
            StartWord,
            NrOfWords,
            BigUInt_p->Bytes_p);

        {
            unsigned int i = 0;

            if (WantedNumberLength_NrOfBytes > (NrOfWords * 4))
            {
                // padding to be done
                i = (NrOfWords * 4);
                while (i < WantedNumberLength_NrOfBytes)
                {
                    BigUInt_p->Bytes_p[i] = 0;
                    i++;
                }
            }

            // set the ByteCount without zero MSBs
            {
                unsigned int n = NrOfWords * 4;
                while (n > 1 && BigUInt_p->Bytes_p[n - 1] == 0)
                    n--;
                BigUInt_p->ByteCount = n;
            }
        }
    }

    IDENTIFIER_NOT_USED(TrueIOArea_p);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif /* EIP28_REMOVE_BIGUINT_ZP_HELPER_FUNC */

/*----------------------------------------------------------------------------
 * StartOp_Multiply_AmulB
 */
#ifndef EIP28_REMOVE_MULTIPLY
EIP28_Status_t
EIP28_StartOp_Multiply_AmulB_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t A_wo,
    const EIP28_WordOffset_t B_wo,
    const unsigned int A_Len,
    const unsigned int B_Len,
    const EIP28_WordOffset_t Result_wo)
{
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

    //EIP28_CHECK_INT_ATLEAST(A_Len, 0);                 // A_Len > 0
    EIP28_CHECK_INT_ATMOST(B_Len, EIP28_VECTOR_MAXLEN); // B_Len <= MAX_LEN

    // Result(C) Vector Length = A_Len + B_Len + 6

    // check the offset A_wo, B_wo, Result_wo
    EIP28_CHECK_INT_ATMOST(A_wo + A_Len, TrueIOArea_p->PKARamNrOfWords);
    EIP28_CHECK_INT_ATMOST(B_wo + B_Len, TrueIOArea_p->PKARamNrOfWords);
    EIP28_CHECK_INT_ATMOST((Result_wo + A_Len + B_Len + 6),
                           TrueIOArea_p->PKARamNrOfWords);

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(A_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(B_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Result_wo);

    // check for no overlap of vector C with  vector A
    EIP28_CHECK_NO_OVERLAP(A_wo, A_Len, Result_wo, (A_Len + B_Len + 6));
    // check for no overlap of vector C with  vector B
    EIP28_CHECK_NO_OVERLAP(B_wo, B_Len, Result_wo, (A_Len + B_Len + 6));

    // write the offset of operand A, operand B and result
    EIP28_A_PTR_WR(Device, A_wo);
    EIP28_B_PTR_WR(Device, B_wo);
    EIP28_C_PTR_WR(Device, Result_wo);
    TrueIOArea_p->LastOperation_Result_wo = Result_wo;
    TrueIOArea_p->LastOperationUsedSequencer = false;

    // write the ALen and BLen
    EIP28_A_LEN_WR(Device, A_Len);
    EIP28_B_LEN_WR(Device, B_Len);

    // Set the multiply bit & run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_MULTIPLY);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif /* EIP28_REMOVE_MULTIPLY */

/*----------------------------------------------------------------------------
 * StartOp_SubAdd_AplusCminB
 */
#ifndef EIP28_REMOVE_SUBADD
EIP28_Status_t
EIP28_StartOp_SubAdd_AplusCminB_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t A_wo,
    const EIP28_WordOffset_t B_wo,
    const EIP28_WordOffset_t C_wo,
    const unsigned int ABC_Len,
    const EIP28_WordOffset_t Result_wo)
{
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;
    // 0 < ABC_Len <= Max_Len
    EIP28_CHECK_INT_INRANGE(ABC_Len, 1, EIP28_VECTOR_MAXLEN);

    // check the offset A_wo, B_wo, C_wo, Result_wo
    EIP28_CHECK_INT_ATMOST(A_wo + ABC_Len, TrueIOArea_p->PKARamNrOfWords);
    EIP28_CHECK_INT_ATMOST(B_wo + ABC_Len, TrueIOArea_p->PKARamNrOfWords);
    EIP28_CHECK_INT_ATMOST(C_wo + ABC_Len, TrueIOArea_p->PKARamNrOfWords);
    EIP28_CHECK_INT_ATMOST((Result_wo + ABC_Len + 1),
                           TrueIOArea_p->PKARamNrOfWords);

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(A_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(B_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(C_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Result_wo);

    // Result(C) Vector Length = ABC_Len + 1

    // check for overlap of vector D with  vector A
    EIP28_CHECK_MEMORY_OVERLAP(A_wo, ABC_Len, Result_wo);
    // check for overlap of vector D with  vector B
    EIP28_CHECK_MEMORY_OVERLAP(B_wo, ABC_Len, Result_wo);
    // check for overlap of vector D with  vector C
    EIP28_CHECK_MEMORY_OVERLAP(C_wo, ABC_Len, Result_wo);

    // write the offset of operand A, operand B, operand C and result
    EIP28_A_PTR_WR(Device, A_wo);
    EIP28_B_PTR_WR(Device, B_wo);
    EIP28_C_PTR_WR(Device, C_wo);
    EIP28_D_PTR_WR(Device, Result_wo);
    TrueIOArea_p->LastOperation_Result_wo = Result_wo;
    TrueIOArea_p->LastOperationUsedSequencer = false;

    // write the ABCLen
    EIP28_A_LEN_WR(Device, ABC_Len);

    // Set the addsub bit and run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_ADDSUB);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif /* EIP28_REMOVE_SUBADD */

/*----------------------------------------------------------------------------
 * StartOp_Add_AplusB
 */
#ifndef EIP28_REMOVE_ADD
EIP28_Status_t
EIP28_StartOp_Add_AplusB_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t A_wo,
    const EIP28_WordOffset_t B_wo,
    const unsigned int A_Len,
    const unsigned int B_Len,
    const EIP28_WordOffset_t Result_wo)
{
#ifdef EIP28_STRICT_ARGS
    unsigned int Result_Len;
#endif
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

    //EIP28_CHECK_INT_ATLEAST(A_Len, 0);                  // A_Len > 0
    EIP28_CHECK_INT_ATMOST(B_Len, EIP28_VECTOR_MAXLEN); // B_Len <= MAX_LEN

#ifdef EIP28_STRICT_ARGS
    // Result(C) Vector Length = MAX(A_Len,B_Len) + 1
    Result_Len = MAX(A_Len, B_Len) + 1;
#endif
    // check the offset A_wo, B_wo, C_wo
    EIP28_CHECK_INT_ATMOST(A_wo + A_Len, TrueIOArea_p->PKARamNrOfWords);
    EIP28_CHECK_INT_ATMOST(B_wo + B_Len, TrueIOArea_p->PKARamNrOfWords);
    EIP28_CHECK_INT_ATMOST((Result_wo + Result_Len + 1),
                           TrueIOArea_p->PKARamNrOfWords);

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(A_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(B_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Result_wo);

    // Result(C) Vector Length = MAX(A_Len,B_Len) + 1

    // check for overlap of vector C with  vector A
    EIP28_CHECK_MEMORY_OVERLAP(A_wo, A_Len, Result_wo);
    // check for overlap of vector C with  vector B
    EIP28_CHECK_MEMORY_OVERLAP(B_wo, B_Len, Result_wo);

    // write the offset of operand A, operand B and result
    EIP28_A_PTR_WR(Device, A_wo);
    EIP28_B_PTR_WR(Device, B_wo);
    EIP28_C_PTR_WR(Device, Result_wo);
    TrueIOArea_p->LastOperation_Result_wo = Result_wo;
    TrueIOArea_p->LastOperationUsedSequencer = false;

    // write the ALen and BLen
    EIP28_A_LEN_WR(Device, A_Len);
    EIP28_B_LEN_WR(Device, B_Len);

    // Set the Add bit and run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_ADD);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif /* EIP28_REMOVE_ADD */

/*----------------------------------------------------------------------------
 * StartOp_Sub_AminB
 */
#ifndef EIP28_REMOVE_SUBTRACT
EIP28_Status_t
EIP28_StartOp_Sub_AminB_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t A_wo,
    const EIP28_WordOffset_t B_wo,
    const unsigned int A_Len,
    const unsigned int B_Len,
    const EIP28_WordOffset_t Result_wo)
{
#ifdef EIP28_STRICT_ARGS
    unsigned int Result_Len;
#endif
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

    //EIP28_CHECK_INT_ATLEAST(A_Len, 0);                  // A_Len > 0
    EIP28_CHECK_INT_ATMOST(B_Len, EIP28_VECTOR_MAXLEN); // B_Len <= MAX_LEN

#ifdef EIP28_STRICT_ARGS
    // Result(C) Vector Length = MAX(A_Len,B_Len)
    Result_Len = MAX(A_Len, B_Len);
#endif
    // check the offset A_wo, B_wo, Result_wo
    EIP28_CHECK_INT_ATMOST(A_wo + A_Len, TrueIOArea_p->PKARamNrOfWords);
    EIP28_CHECK_INT_ATMOST(B_wo + B_Len, TrueIOArea_p->PKARamNrOfWords);
    EIP28_CHECK_INT_ATMOST(Result_wo + Result_Len, TrueIOArea_p->PKARamNrOfWords);

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(A_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(B_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Result_wo);

    // check result is positive
    EIP28_CHECK_INT_ATMOST(B_Len, A_Len); // B_Len <= A_Len

    // check for overlap of vector C with  vector A
    EIP28_CHECK_MEMORY_OVERLAP(A_wo, A_Len, Result_wo);
    // check for overlap of vector C with  vector B
    EIP28_CHECK_MEMORY_OVERLAP(B_wo, B_Len, Result_wo);

    // write the offset of operand A, operand B and result
    EIP28_A_PTR_WR(Device, A_wo);
    EIP28_B_PTR_WR(Device, B_wo);
    EIP28_C_PTR_WR(Device, Result_wo);
    TrueIOArea_p->LastOperation_Result_wo = Result_wo;
    TrueIOArea_p->LastOperationUsedSequencer = false;

    // write the ALen and BLen
    EIP28_A_LEN_WR(Device, A_Len);
    EIP28_B_LEN_WR(Device, B_Len);

    // Set the subtract bit & run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_SUBTRACT);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif /* EIP28_REMOVE_SUBTRACT */

/*----------------------------------------------------------------------------
 * StartOp_ShiftRight_AshiftrightS
 */
#ifndef EIP28_REMOVE_SHIFTRIGHT
EIP28_Status_t
EIP28_StartOp_ShiftRight_AshiftrightS_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t A_wo,
    const unsigned int A_Len,
    const unsigned int S,
    const EIP28_WordOffset_t Result_wo)
{
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;
    EIP28_CHECK_INT_INRANGE(A_Len, 1, EIP28_VECTOR_MAXLEN); // 0<A_Len<=MAX_LEN
    EIP28_CHECK_INT_ATMOST(S, 31);                          // In the range 0-31

    //check the offset A_wo, Result_wo
    EIP28_CHECK_INT_ATMOST(A_wo + A_Len, TrueIOArea_p->PKARamNrOfWords);
    EIP28_CHECK_INT_ATMOST(Result_wo + A_Len, TrueIOArea_p->PKARamNrOfWords);

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(A_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Result_wo);

    // Result(C) Vector Length = A_Len

    // check for overlap of vector C with  vector A
    EIP28_CHECK_MEMORY_OVERLAP(A_wo, A_Len, Result_wo);

    // write the offset of operand A and result
    EIP28_A_PTR_WR(Device, A_wo);
    EIP28_C_PTR_WR(Device, Result_wo);
    TrueIOArea_p->LastOperation_Result_wo = Result_wo;
    TrueIOArea_p->LastOperationUsedSequencer = false;

    // write the ALen only
    EIP28_A_LEN_WR(Device, A_Len);

    // write the PKA bit right shift Value
    EIP28_SHIFT_WR(Device, S);

    // Set the right shift bit  & run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_RSHIFT);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif /* EIP28_REMOVE_SHIFTRIGHT */

/*----------------------------------------------------------------------------
 * StartOp_ShiftLeft_AshiftleftS
 */
#ifndef EIP28_REMOVE_SHIFTLEFT
EIP28_Status_t
EIP28_StartOp_ShiftLeft_AshiftleftS_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t A_wo,
    const unsigned int A_Len,
    const unsigned int S,
    const EIP28_WordOffset_t Result_wo)
{
    unsigned int Result_Len;
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;
    EIP28_CHECK_INT_INRANGE(A_Len, 1, EIP28_VECTOR_MAXLEN); // 0<A_Len<=MAX_LEN
    EIP28_CHECK_INT_ATMOST(S, 31);                          // In the range 0-31

    // Result(C) Vector Length = A_Len     (when shift value is zero)
    //                         = A_Len + 1 (when shift value is non-zero)
    Result_Len = A_Len;
    if (S > 0)
        Result_Len++;

    // check the offset A_wo, Result_wo
    EIP28_CHECK_INT_ATMOST(A_wo + A_Len, TrueIOArea_p->PKARamNrOfWords);
    EIP28_CHECK_INT_ATMOST(Result_wo + Result_Len, TrueIOArea_p->PKARamNrOfWords);

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(A_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Result_wo);

    // check for overlap of vector C with  vector A
    EIP28_CHECK_MEMORY_OVERLAP(A_wo, A_Len, Result_wo);

    // write the offset of operand A and result
    EIP28_A_PTR_WR(Device, A_wo);
    EIP28_C_PTR_WR(Device, Result_wo);
    TrueIOArea_p->LastOperation_Result_wo = Result_wo;
    TrueIOArea_p->LastOperationUsedSequencer = false;

    // write the ALen only
    EIP28_A_LEN_WR(Device, A_Len);

    // write the PKA bit left shift Value
    EIP28_SHIFT_WR(Device, S);

    // Set the lefts shift bit & run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_LSHIFT);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif /* EIP28_REMOVE_SHIFTLEFT */

/*----------------------------------------------------------------------------
 * StartOp_ModuloOrCopy_AmodB
 */
#ifndef EIP28_REMOVE_MODULO_OR_COPY
EIP28_Status_t
EIP28_StartOp_ModuloOrCopy_AmodB_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t A_wo,
    const EIP28_WordOffset_t B_wo,
    const unsigned int A_Len,
    const unsigned int B_Len,
    const EIP28_WordOffset_t Result_wo)
{
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

    // 1 < B_len <= Max_Len
    EIP28_CHECK_INT_INRANGE(B_Len, 2, EIP28_VECTOR_MAXLEN);
    // 0 < A_len <= Max_Len
    EIP28_CHECK_INT_INRANGE(A_Len, 1, EIP28_VECTOR_MAXLEN);

    if ((rv = EIP28Lib_SetParams_Modulo(IOArea_p,
                                        A_wo,
                                        B_wo,
                                        A_Len,
                                        B_Len,
                                        Result_wo)) != EIP28_STATUS_OK)
    {
#ifdef EIP28_STRICT_ARGS
        goto FUNC_RETURN;
#endif
    }

    if (A_Len < B_Len)
    {
        unsigned int wo;

        // Since A < B, no actual Modulo operation is needed since the
        // result is known to be A. Simply do a Copy and pad with zeroes
        // to make the result B_Len+1 words.
        EIP28_FUNCTION_START_OPERATION(
            Device,
            EIP28_FUNCTION_COPY);

        for (wo = Result_wo + A_Len; wo < Result_wo + B_Len + 1; wo++)
        {
            EIP28_PKARAM_WR(Device, wo, 0);
        }
    }
    else
    {
        // Set the modulus & run bit in PKA_FUNCTION Register
        EIP28_FUNCTION_START_OPERATION(
            Device,
            EIP28_FUNCTION_MODULO);
    }

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif /* EIP28_REMOVE_MODULO_OR_COPY */

/*----------------------------------------------------------------------------
 * StartOp_Modulo_AmodB
 */
#ifndef EIP28_REMOVE_MODULO
EIP28_Status_t
EIP28_StartOp_Modulo_AmodB_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t A_wo,
    const EIP28_WordOffset_t B_wo,
    const unsigned int A_Len,
    const unsigned int B_Len,
    const EIP28_WordOffset_t Result_wo)
{
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

    EIP28_CHECK_INT_INRANGE(B_Len, 2, A_Len); // 1 < B_len <= A_Len
    // B_Len <= A_Len <= Max_Len
    EIP28_CHECK_INT_INRANGE(A_Len, B_Len, EIP28_VECTOR_MAXLEN);

    if ((rv = EIP28Lib_SetParams_Modulo(IOArea_p,
                                        A_wo,
                                        B_wo,
                                        A_Len,
                                        B_Len,
                                        Result_wo)) != EIP28_STATUS_OK)
    {
#ifdef EIP28_STRICT_ARGS
        goto FUNC_RETURN;
#endif
    }

    // Set the modulus & run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_MODULO);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif /* EIP28_REMOVE_MODULO */

/*----------------------------------------------------------------------------
 * StartOp_Copy_Adup
 */
#ifndef EIP28_REMOVE_COPY
EIP28_Status_t
EIP28_StartOp_Copy_Adup_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t A_wo,
    const unsigned int A_Len,
    const EIP28_WordOffset_t Result_wo)
{
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;
    // 0 < A_Len <= Max_Len
    EIP28_CHECK_INT_INRANGE(A_Len, 1, EIP28_VECTOR_MAXLEN);

    // Result(C) Vector Length = A_Len
    // check the offset A_wo, Result_wo
    EIP28_CHECK_INT_ATMOST(A_wo + A_Len, TrueIOArea_p->PKARamNrOfWords);
    EIP28_CHECK_INT_ATMOST((Result_wo + A_Len), TrueIOArea_p->PKARamNrOfWords);

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(A_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Result_wo);

    // all forms of overlap are allowed
    // if C starts at A_wo+2 then C is filled with the 64bits found at A_wo

    // write the offset of operand A and result
    EIP28_A_PTR_WR(Device, A_wo);
    EIP28_C_PTR_WR(Device, Result_wo);
    TrueIOArea_p->LastOperation_Result_wo = Result_wo;
    TrueIOArea_p->LastOperationUsedSequencer = false;

    // write the ALen
    EIP28_A_LEN_WR(Device, A_Len);

    // Set the copy & run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(Device, EIP28_FUNCTION_COPY);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif /* EIP28_REMOVE_COPY */

/*----------------------------------------------------------------------------
 * StartOp_ModInv_invAmodB
 */
#ifndef EIP28_REMOVE_MODINV
EIP28_Status_t
EIP28_StartOp_ModInv_invAmodB_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t A_wo,
    const EIP28_WordOffset_t B_wo,
    const unsigned int A_Len,
    const unsigned int B_Len,
    const EIP28_WordOffset_t Result_wo)
{
#ifdef EIP28_STRICT_ARGS
    unsigned int Result_len;
#endif
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;
    // 1 <= A_len <= Max_Len
    EIP28_CHECK_INT_INRANGE(A_Len, 1, EIP28_VECTOR_MAXLEN);
    // 1 <= B_Len <= Max_Len
    EIP28_CHECK_INT_INRANGE(B_Len, 1, EIP28_VECTOR_MAXLEN);

#ifdef EIP28_STRICT_ARGS
    Result_len = EIP28_GetWorkAreaSize(MODINV, 0, A_Len, B_Len);

    // check the offset A_wo, B_wo, Result_wo
    // The last X bytes of PKA RAM will be used as general scratchpad, and
    // should not overlap with any of input vectors
    EIP28_CHECK_INT_ATMOST(
        A_wo + A_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(MODINV));
    EIP28_CHECK_INT_ATMOST(
        B_wo + B_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(MODINV));
    EIP28_CHECK_INT_ATMOST(
        Result_wo + Result_len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(MODINV));
#endif

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(A_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(B_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Result_wo);

    // Modulus B must be odd
    CHECK_PKARAM_WORD_IS_ODD(B_wo);

    // Highest word of Modulus Vector shall not be zero
    CHECK_PKARAM_WORD_NOT_ZERO(B_wo, B_Len);

    // Modulus B may not have value 1

    // Vector Result_wo(D) may not overlap with any of the vectors
    // check for no overlap of vector D with  vector A
    EIP28_CHECK_NO_OVERLAP(A_wo, A_Len, Result_wo, Result_len);
    // check for no overlap of vector D with  vector B
    EIP28_CHECK_NO_OVERLAP(B_wo, B_Len, Result_wo, Result_len);

    // write the offset of operand A, operand B and result
    EIP28_A_PTR_WR(Device, A_wo);
    EIP28_B_PTR_WR(Device, B_wo);
    EIP28_D_PTR_WR(Device, Result_wo);
    TrueIOArea_p->LastOperation_Result_wo = Result_wo;
    TrueIOArea_p->LastOperationUsedSequencer = true;
    TrueIOArea_p->LastOperation_ResultLen = B_Len;

    // write the ALen and BLen
    EIP28_A_LEN_WR(Device, A_Len);
    EIP28_B_LEN_WR(Device, B_Len);

    // Set the modInv sequencer ops bit and run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_SEQ_MODINV);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif /* EIP28_REMOVE_MODINV */

/*----------------------------------------------------------------------------
 * StartOp_ModExp_CRT
 */
#ifndef EIP28_REMOVE_MODEXPCRT
EIP28_Status_t
EIP28_StartOp_ModExp_CRT_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t ExpPQ_wo,
    const EIP28_WordOffset_t ModPQ_wo,
    const EIP28_WordOffset_t InvQ_wo,
    const EIP28_WordOffset_t InputResult_wo,
    const unsigned int NrOfOddPowers,
    const unsigned int A_Len,
    const unsigned int B_Len)
{
    int Result_len;
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;
    // 0 < A_Len <= Max_Len
    EIP28_CHECK_INT_INRANGE(A_Len, 1, EIP28_VECTOR_MAXLEN);
    // 1 < B_Len <= Max_Len
    EIP28_CHECK_INT_INRANGE(B_Len, 2, EIP28_VECTOR_MAXLEN);
#ifdef EIP28_MONTGOMERY_LADDER_OPTION
    // NrOfOddPowers <= 16 (0 means using MontgomeryLadder)
    EIP28_CHECK_INT_ATMOST(NrOfOddPowers, 16);
#else
    // 0 < NrOfOddPowers <= 16
    EIP28_CHECK_INT_INRANGE(NrOfOddPowers, 1, 16);
#endif
    Result_len = EIP28_GetWorkAreaSize(MODEXP_CRT, NrOfOddPowers, A_Len, B_Len);

    // cannot check the spacing between ExpP and ExpQ

    // check that the vectors are inside the available RAM
    // The last X bytes of PKA RAM will be used as general scratchpad, and
    // should not overlap with any of input vectors and D vector
    EIP28_CHECK_INT_ATMOST(
        ExpPQ_wo + 2 * A_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(MODEXP_CRT));
    // misses alignment spacer
    EIP28_CHECK_INT_ATMOST(
        ModPQ_wo + 2 * B_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(MODEXP_CRT));
    // misses buffer words
    EIP28_CHECK_INT_ATMOST(
        InvQ_wo + B_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(MODEXP_CRT));
    EIP28_CHECK_INT_ATMOST(
        InputResult_wo + (unsigned int)Result_len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(MODEXP_CRT));

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(ExpPQ_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(ModPQ_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(InvQ_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(InputResult_wo);

    // Modulus P & Q must be odd
    CHECK_PKARAM_WORD_IS_ODD(ModPQ_wo);

    // Must round to next aligned address for this test
    CHECK_PKARAM_WORD_IS_ODD(
        EIP28_Memory_IncAndAlignNext64(
            (EIP28_WordOffset_t)(ModPQ_wo + B_Len)));

    // 0 < ExpP < (Mod P - 1) & 0 < ExpQ < (Mod Q - 1)
    // Mod P > Mod Q > 2^32
    // Mod P and Mod Q must be co-prime
    // Input < (Mod P * Mod Q)
    // (Q inverse * Mod P) = 1 (Mod Q)
    // Mod P and Mod Q must be followed by an empty 32-bit buffer word

    // Vector D may not overlap with any of the vectors
    // check for no overlap of vector D with  vector A
    // misses alignment spacer
    EIP28_CHECK_NO_OVERLAP(ExpPQ_wo, 2 * A_Len, InputResult_wo, Result_len);
    // check for no overlap of vector D with  vector B
    // misses buffer words
    EIP28_CHECK_NO_OVERLAP(ModPQ_wo, 2 * B_Len, InputResult_wo, Result_len);
    // check for no overlap of vector D with  vector C
    EIP28_CHECK_NO_OVERLAP(InvQ_wo, B_Len, InputResult_wo, Result_len);

    // write the offset of operand A, operand B, operand C and result
    EIP28_A_PTR_WR(Device, ExpPQ_wo);
    EIP28_B_PTR_WR(Device, ModPQ_wo);
    EIP28_C_PTR_WR(Device, InvQ_wo);
    EIP28_D_PTR_WR(Device, InputResult_wo);
    TrueIOArea_p->LastOperation_Result_wo = InputResult_wo;
    TrueIOArea_p->LastOperationUsedSequencer = true;
    TrueIOArea_p->LastOperation_ResultLen = 2U * B_Len; // caller knows!

    // write the ALen, BLen
    EIP28_A_LEN_WR(Device, A_Len);
    EIP28_B_LEN_WR(Device, B_Len);

    // write the NrofOddPowers
    EIP28_SHIFT_WR(Device, NrOfOddPowers);

    // Set the modexpvar bit & run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_SEQ_MODEXPCRT);

    IDENTIFIER_NOT_USED(Result_len);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif /* EIP28_REMOVE_MODEXPCRT */

/*----------------------------------------------------------------------------
 * StartOp_Divide_AdivB
 */
#ifndef EIP28_REMOVE_DIVIDE
EIP28_Status_t
EIP28_StartOp_Divide_AdivB_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t A_wo,
    const EIP28_WordOffset_t B_wo,
    const unsigned int A_Len,
    const unsigned int B_Len,
    const EIP28_WordOffset_t Remainder_wo,
    const EIP28_WordOffset_t Quotient_wo)
{
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;
    // 1 < B_len <= A_Len
    EIP28_CHECK_INT_INRANGE(B_Len, 2, A_Len);
    // B_Len <= A_Len <= Max_Len
    EIP28_CHECK_INT_INRANGE(A_Len, B_Len, EIP28_VECTOR_MAXLEN);

    // check the offset A_wo, B_wo, Remainder_wo, Quotient_wo
    EIP28_CHECK_INT_ATMOST(A_wo + A_Len, TrueIOArea_p->PKARamNrOfWords);
    EIP28_CHECK_INT_ATMOST(B_wo + B_Len, TrueIOArea_p->PKARamNrOfWords);
    EIP28_CHECK_INT_ATMOST((Remainder_wo + B_Len + 1),
                           TrueIOArea_p->PKARamNrOfWords);
    EIP28_CHECK_INT_ATMOST((Quotient_wo + A_Len - B_Len + 1),
                           TrueIOArea_p->PKARamNrOfWords);

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(A_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(B_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Remainder_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Quotient_wo);

    // Highest word of Modulus Vector shall not be zero
    CHECK_PKARAM_WORD_NOT_ZERO(B_wo, B_Len);

    // Remainder(C) Vector Length = B_Len + 1
    // Quotient(D) Vector Length = A_Len - B_Len + 1

    // check for no overlap of vector C with  vector A
    EIP28_CHECK_NO_OVERLAP(A_wo, A_Len, Remainder_wo, (B_Len + 1));
    // check for no overlap of vector C with  vector B
    EIP28_CHECK_NO_OVERLAP(B_wo, B_Len, Remainder_wo, (B_Len + 1));
    // check for no overlap of vector C with  vector D
    EIP28_CHECK_NO_OVERLAP(Quotient_wo, (A_Len - B_Len + 1),
                           Remainder_wo, (B_Len + 1));

    // check for no overlap of vector D with  vector A
    EIP28_CHECK_NO_OVERLAP(A_wo, A_Len, Quotient_wo, (A_Len - B_Len + 1));
    // check for no overlap of vector D with  vector B
    EIP28_CHECK_NO_OVERLAP(B_wo, B_Len, Quotient_wo, (A_Len - B_Len + 1));
    // check for no overlap of vector D with  vector C
    EIP28_CHECK_NO_OVERLAP(Remainder_wo, (B_Len + 1), Quotient_wo,
                           (A_Len - B_Len + 1));

    // write the offset of operand A, operand B, Remainder_wo, Quotient_wo
    EIP28_A_PTR_WR(Device, A_wo);
    EIP28_B_PTR_WR(Device, B_wo);
    EIP28_C_PTR_WR(Device, Remainder_wo);
    EIP28_D_PTR_WR(Device, Quotient_wo);
    TrueIOArea_p->LastOperation_Result_wo = Quotient_wo;
    TrueIOArea_p->LastOperation_Remainder_wo = Remainder_wo;
    TrueIOArea_p->LastOperationUsedSequencer = false;

    // write the ALen and BLen
    EIP28_A_LEN_WR(Device, A_Len);
    EIP28_B_LEN_WR(Device, B_Len);

    // Set the divide bit & run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_DIVIDE);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE
}
#endif /* EIP28_REMOVE_DIVIDE */

/*----------------------------------------------------------------------------
 * StartOp_Compare_AcmpB
 */
#ifndef EIP28_REMOVE_COMPARE
EIP28_Status_t
EIP28_StartOp_Compare_AcmpB_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t A_wo,
    const EIP28_WordOffset_t B_wo,
    const unsigned int AB_Len)
{
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;
    // 0 < A_Len <= Max_Len
    EIP28_CHECK_INT_INRANGE(AB_Len, 1, EIP28_VECTOR_MAXLEN);

    // check the offset A_wo, B_wo
    EIP28_CHECK_INT_ATMOST((A_wo + AB_Len), TrueIOArea_p->PKARamNrOfWords);
    EIP28_CHECK_INT_ATMOST((B_wo + AB_Len), TrueIOArea_p->PKARamNrOfWords);

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(A_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(B_wo);

    // Compare does not write a result vector

    // write the offset of operand A, operand B, Result_wo
    EIP28_A_PTR_WR(Device, A_wo);
    EIP28_B_PTR_WR(Device, B_wo);

    // write the ABLen
    EIP28_A_LEN_WR(Device, AB_Len);

    // Set the compare bit & run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_COMPARE);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif /* EIP28_REMOVE_COMPARE */

/*----------------------------------------------------------------------------
 * ReadResult_Compare
 */
#ifndef EIP28_REMOVE_COMPARE
void EIP28_ReadResult_Compare(
    EIP28_IOArea_t *const IOArea_p,
    EIP28_CompareResult_t *const CompareResult_p)
{
    volatile EIP28_True_IOArea_t *const TrueIOArea_p = IOAREA(IOArea_p);
    Device_Handle_t Device = TrueIOArea_p->Device;

    if (NULL != CompareResult_p)
    {
        uint32_t R = EIP28_COMPARE_RD(Device);

        if (R == EIP28_COMPARE_A_EQUAL_B)
            *CompareResult_p = EIP28_COMPARERESULT_A_EQUALS_B;
        else
        {
            if (R == EIP28_COMPARE_A_LESSER_B)
                *CompareResult_p = EIP28_COMPARERESULT_A_LESSTHAN_B;
            else
            {
                //if (R == EIP28_COMPARE_A_GREATER_B)
                *CompareResult_p = EIP28_COMPARERESULT_A_GREATERTHAN_B;
            }
        }
    }
}
#endif /* EIP28_REMOVE_COMPARE */

/*----------------------------------------------------------------------------
 * StartOp_EccAdd_Affine_AplusC
 */
#ifndef EIP28_REMOVE_ECCADD
EIP28_Status_t
EIP28_StartOp_EccAdd_Affine_AplusC_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t Axy_wo,
    const EIP28_WordOffset_t pa_wo,
    const EIP28_WordOffset_t Cxy_wo,
    const unsigned int B_Len,
    const EIP28_WordOffset_t Resultxy_wo)
{
#ifdef EIP28_STRICT_ARGS
    int Result_len;
    unsigned int L;
#endif
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

    // maximum vector length is 768 bits
    EIP28_CHECK_INT_INRANGE(B_Len, 2, 768 / 32); // 1 < B_len <= 24

    // Result Vector size
#ifdef EIP28_STRICT_ARGS
    L = B_Len + 2 + (B_Len & 1);
    Result_len = EIP28_GetWorkAreaSize(ECC_ADD_AFFINE, 0, 0, B_Len);

    // check the that the vectors end inside the PKA RAM
    // note: last X bytes of PKARAM are used as scratchpad
    EIP28_CHECK_INT_ATMOST(
        Axy_wo + 2 * L,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECC_ADD_AFFINE));
    EIP28_CHECK_INT_ATMOST(
        pa_wo + 2 * L,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECC_ADD_AFFINE));
    EIP28_CHECK_INT_ATMOST(
        Cxy_wo + 2 * L,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECC_ADD_AFFINE));
    EIP28_CHECK_INT_ATMOST(
        Resultxy_wo + Result_len,
        (int)TrueIOArea_p->PKARamNrOfWords -
            EIP28_GetScratchpadSize(ECC_ADD_AFFINE));
#endif

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(Axy_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(pa_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Cxy_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Resultxy_wo);

    // Highest word of Modulus Vector shall not be zero
    CHECK_PKARAM_WORD_NOT_ZERO(pa_wo, B_Len);

    // vectors are allowed to overlap,
    // although this does normally not make sense

    // All components (input and result) must have 3(B_Len odd) or
    // 2(B_Len even) buffer words after their MSW

    // Modulus p must be a prime > 2^63
    // Effective modulus size(in bits) must be a multiple of 32,
    // with the exception of 521; a < p and b < p
    // Neither pntA nor pntC can be the "point at infinity"

    // write the offset of all the vector
    EIP28_A_PTR_WR(Device, Axy_wo);
    EIP28_B_PTR_WR(Device, pa_wo);
    EIP28_C_PTR_WR(Device, Cxy_wo);
    EIP28_D_PTR_WR(Device, Resultxy_wo);

    TrueIOArea_p->LastOperationUsedSequencer = true;
    TrueIOArea_p->LastOperation_ResultLen = VLEN2A(B_Len) + VLEN(B_Len); // caller knows!

    // write the ALen and BLen
    EIP28_A_LEN_WR(Device, B_Len); // Not Used but must be B_Len
    EIP28_B_LEN_WR(Device, B_Len);

    // Set the ECCAdd run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_SEQ_ECCADD_AFFINE);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif /* EIP28_REMOVE_ECCADD */

/*----------------------------------------------------------------------------
 * StartOp_EccAdd_Projective_AplusC
 */
#ifndef EIP28_REMOVE_ECCADD
EIP28_Status_t
EIP28_StartOp_EccAdd_Projective_AplusC_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t Axyz_wo,
    const EIP28_WordOffset_t pa_wo,
    const EIP28_WordOffset_t Cxyz_wo,
    const unsigned int B_Len,
    const EIP28_WordOffset_t Resultxyz_wo)
{
#ifdef EIP28_STRICT_ARGS
    int Result_len;
    unsigned int L, E;
#endif
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

    // maximum vector length is 768 bits
    EIP28_CHECK_INT_INRANGE(B_Len, 2, 768 / 32); // 1 < B_len <= 24

    // Result Vector size
#ifdef EIP28_STRICT_ARGS
    L = B_Len + 2 + (B_Len & 1);
    E = B_Len + 2;
    Result_len = EIP28_GetWorkAreaSize(ECC_ADD_PROJECTIVE, 0, 0, B_Len);

    // check the that the vectors end inside the PKA RAM
    // note: last X bytes of PKARAM are used as scratchpad
    EIP28_CHECK_INT_ATMOST(
        Axyz_wo + 2 * L + E,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECC_ADD_PROJECTIVE));
    EIP28_CHECK_INT_ATMOST(
        pa_wo + 2 * L + E,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECC_ADD_PROJECTIVE));
    EIP28_CHECK_INT_ATMOST(
        Cxyz_wo + 2 * L + E,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECC_ADD_PROJECTIVE));
    EIP28_CHECK_INT_ATMOST(
        Resultxyz_wo + 3 * L + Result_len,
        TrueIOArea_p->PKARamNrOfWords -
            EIP28_GetScratchpadSize(ECC_ADD_PROJECTIVE));
#endif

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(Axyz_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(pa_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Cxyz_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Resultxyz_wo);

    // Highest word of Modulus Vector shall not be zero
    CHECK_PKARAM_WORD_NOT_ZERO(pa_wo, B_Len);

    // vectors are allowed to overlap,
    // although this does normally not make sense

    // All components (input and result) must have 3(B_Len odd) or
    // 2(B_Len even) buffer words after their MSW

    // Modulus p must be a prime > 2^63
    // Effective modulus size(in bits) must be a multiple of 32,
    // with the exception of 521; a < p and b < p
    // Neither pntA nor pntC can be the "point at infinity"

    // write the offset of all the vector
    EIP28_A_PTR_WR(Device, Axyz_wo);
    EIP28_B_PTR_WR(Device, pa_wo);
    EIP28_C_PTR_WR(Device, Cxyz_wo);
    EIP28_D_PTR_WR(Device, Resultxyz_wo);

    TrueIOArea_p->LastOperationUsedSequencer = true;
    TrueIOArea_p->LastOperation_ResultLen = 2U * VLEN2A(B_Len) + VLEN(B_Len); // caller knows!

    // write the ALen and BLen
    EIP28_A_LEN_WR(Device, B_Len); // Not Used but must be B_Len
    EIP28_B_LEN_WR(Device, B_Len);

    // Set the ECCAdd run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_SEQ_ECCADD_PROJECTIVE);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif /* EIP28_REMOVE_ECCADD */

/*----------------------------------------------------------------------------
 * StartOp_EccMul_Affine_kmulC
 */
#ifndef EIP28_REMOVE_ECCMUL
EIP28_Status_t
EIP28_StartOp_EccMul_Affine_kmulC_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t k_wo,
    const EIP28_WordOffset_t pab_wo,
    const EIP28_WordOffset_t Cxy_wo,
    const unsigned int A_Len, // used for k only
    const unsigned int B_Len,
    const EIP28_WordOffset_t Resultxy_wo)
{
#ifdef EIP28_STRICT_ARGS
    int Result_len;
    unsigned int L, E;
#endif
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

    // maximum vector length is 768 bits
    EIP28_CHECK_INT_INRANGE(A_Len, 1, 768 / 32); // 0 < A_len <= 24
    EIP28_CHECK_INT_INRANGE(B_Len, 2, 768 / 32); // 1 < B_len <= 24

#ifdef EIP28_STRICT_ARGS
    L = B_Len + 2 + (B_Len & 1);
    E = B_Len + 2;
    // Result Vector size
    Result_len = EIP28_GetWorkAreaSize(ECC_MUL_AFFINE, 0, A_Len, B_Len);

    // check the offset of the Vector
    // note: last X bytes of PKARAM are used as scratchpad
    EIP28_CHECK_INT_ATMOST(
        k_wo + A_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECC_MUL_AFFINE));
    EIP28_CHECK_INT_ATMOST(
        pab_wo + 2 * L + E,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECC_MUL_AFFINE));
    EIP28_CHECK_INT_ATMOST(
        Cxy_wo + L + E,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECC_MUL_AFFINE));
    EIP28_CHECK_INT_ATMOST(
        Resultxy_wo + 2 * L + Result_len,
        TrueIOArea_p->PKARamNrOfWords -
            EIP28_GetScratchpadSize(ECC_MUL_AFFINE));
#endif

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(k_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(pab_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Cxy_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Resultxy_wo);

    // check for no overlap of Result with k
    EIP28_CHECK_NO_OVERLAP(k_wo, A_Len, Resultxy_wo, Result_len);
    // check for no overlap of Result with pab
    EIP28_CHECK_NO_OVERLAP(pab_wo, 3 * B_Len, Resultxy_wo, Result_len);
    // check for no overlap of Result with C
    EIP28_CHECK_NO_OVERLAP(Cxy_wo, 2 * B_Len, Resultxy_wo, Result_len);

    // Highest word of Modulus Vector shall not be zero
    CHECK_PKARAM_WORD_NOT_ZERO(pab_wo, B_Len);

    // All input components(Vector B, C) must have 3(ABC_Len) or 2(ABC_Len)
    // buffer words after their MSW as well as each result
    // components(Vector D)

    // Modulus p must be a prime > 2^63
    // Effective modulus size(in bits) must be a multiple of 32
    // a < p and b < p
    // Neither pntA nor pntC can be the "point at infinity"

    // write the offset of vector A, B & C
    EIP28_A_PTR_WR(Device, k_wo);
    EIP28_B_PTR_WR(Device, pab_wo);
    EIP28_C_PTR_WR(Device, Cxy_wo);
    EIP28_D_PTR_WR(Device, Resultxy_wo);

    TrueIOArea_p->LastOperationUsedSequencer = true;
    TrueIOArea_p->LastOperation_ResultLen = VLEN2A(B_Len) + VLEN(B_Len); // caller knows!

    // write the lengths
    EIP28_A_LEN_WR(Device, A_Len);
    EIP28_B_LEN_WR(Device, B_Len);

    // Set the ECCMUL run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_SEQ_ECCMUL_AFFINE);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif /* EIP28_REMOVE_ECCMUL */

/*----------------------------------------------------------------------------
 * StartOp_EccMul_Projective_kmulC
 */
#ifndef EIP28_REMOVE_ECCMUL
EIP28_Status_t
EIP28_StartOp_EccMul_Projective_kmulC_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t k_wo,
    const EIP28_WordOffset_t pab_wo,
    const EIP28_WordOffset_t Cxyr_wo,
    const unsigned int A_Len, // used for k only
    const unsigned int B_Len,
    const EIP28_WordOffset_t Resultxyz_wo)
{
#ifdef EIP28_STRICT_ARGS
    int Result_len;
    unsigned int L, E;
#endif
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

    // maximum vector length is 768 bits
    EIP28_CHECK_INT_INRANGE(A_Len, 1, 768 / 32); // 0 < A_len <= 24
    EIP28_CHECK_INT_INRANGE(B_Len, 2, 768 / 32); // 1 < B_len <= 24

#ifdef EIP28_STRICT_ARGS
    L = B_Len + 2 + (B_Len & 1);
    E = B_Len + 2;
    // Result Vector size
    Result_len = EIP28_GetWorkAreaSize(ECC_MUL_PROJECTIVE, 0, A_Len, B_Len);

    // check the offset of the Vector
    // note: last X bytes of PKARAM are used as scratchpad
    EIP28_CHECK_INT_ATMOST(
        k_wo + A_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECC_MUL_PROJECTIVE));
    EIP28_CHECK_INT_ATMOST(
        pab_wo + 2 * L + E,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECC_MUL_PROJECTIVE));
    EIP28_CHECK_INT_ATMOST(
        Cxyr_wo + 2 * L + E,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECC_MUL_PROJECTIVE));
    EIP28_CHECK_INT_ATMOST(
        Resultxyz_wo + 3 * L + Result_len,
        TrueIOArea_p->PKARamNrOfWords -
            EIP28_GetScratchpadSize(ECC_MUL_PROJECTIVE));
#endif

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(k_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(pab_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Cxyr_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Resultxyz_wo);

    // check for no overlap of Result with k
    EIP28_CHECK_NO_OVERLAP(k_wo, A_Len, Resultxyz_wo, Result_len);
    // check for no overlap of Result with pab
    EIP28_CHECK_NO_OVERLAP(pab_wo, 3 * B_Len, Resultxyz_wo, Result_len);
    // check for no overlap of Result with C
    EIP28_CHECK_NO_OVERLAP(Cxyr_wo, 2 * B_Len, Resultxyz_wo, Result_len);

    // Highest word of Modulus Vector shall not be zero
    CHECK_PKARAM_WORD_NOT_ZERO(pab_wo, B_Len);

    // All input components(Vector B, C) must have 3(ABC_Len) or 2(ABC_Len)
    // buffer words after their MSW as well as each result
    // components(Vector D)

    // Modulus p must be a prime > 2^63
    // Effective modulus size(in bits) must be a multiple of 32
    // a < p and b < p
    // Neither pntA nor pntC can be the "point at infinity"

    // write the offset of vector A, B & C
    EIP28_A_PTR_WR(Device, k_wo);
    EIP28_B_PTR_WR(Device, pab_wo);
    EIP28_C_PTR_WR(Device, Cxyr_wo);
    EIP28_D_PTR_WR(Device, Resultxyz_wo);

    TrueIOArea_p->LastOperationUsedSequencer = true;
    TrueIOArea_p->LastOperation_ResultLen = 2 * VLEN2A(B_Len) + VLEN(B_Len); // caller knows!

    // write the lengths
    EIP28_A_LEN_WR(Device, A_Len);
    EIP28_B_LEN_WR(Device, B_Len);

    // Set the ECCMUL run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_SEQ_ECCMUL_PROJECTIVE);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif /* EIP28_REMOVE_ECCMUL */

/*----------------------------------------------------------------------------
 * StartOp_EccMul_Montgomery_kmulC
 */
#ifndef EIP28_REMOVE_ECCMULMONT
EIP28_Status_t
EIP28_StartOp_EccMul_Montgomery_kmulC_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t k_wo,
    const EIP28_WordOffset_t pa_wo,
    const EIP28_WordOffset_t Cx_wo,
    const unsigned int A_Len, // used for k only
    const unsigned int B_Len,
    const EIP28_WordOffset_t Resultx_wo)
{
#ifdef EIP28_STRICT_ARGS
    int Result_len;
    unsigned int L, E;
#endif
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

    // maximum vector length is 768 bits
    EIP28_CHECK_INT_INRANGE(A_Len, 1, 768 / 32); // 0 < A_len <= 24
    EIP28_CHECK_INT_INRANGE(B_Len, 2, 768 / 32); // 1 < B_len <= 24

#ifdef EIP28_STRICT_ARGS
    L = B_Len + 2 + (B_Len & 1);
    E = B_Len + 2;
    // Result Vector size
    Result_len = EIP28_GetWorkAreaSize(ECC_MUL_MONT, 0, A_Len, B_Len);

    // check the offset of the Vector
    // note: last X bytes of PKARAM are used as scratchpad
    EIP28_CHECK_INT_ATMOST(
        k_wo + A_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECC_MUL_MONT));
    EIP28_CHECK_INT_ATMOST(
        pa_wo + L + B_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECC_MUL_MONT));
    EIP28_CHECK_INT_ATMOST(
        Cx_wo + E,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECC_MUL_MONT));
    EIP28_CHECK_INT_ATMOST(
        Resultx_wo + L + Result_len,
        TrueIOArea_p->PKARamNrOfWords -
            EIP28_GetScratchpadSize(ECC_MUL_MONT));
#endif

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(k_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(pa_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Cx_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Resultx_wo);

    // check for no overlap of Result with k
    EIP28_CHECK_NO_OVERLAP(k_wo, A_Len, Resultx_wo, Result_len);
    // check for no overlap of Result with pa
    EIP28_CHECK_NO_OVERLAP(pa_wo, 2 * B_Len, Resultx_wo, Result_len);
    // check for no overlap of Result with C.x
    EIP28_CHECK_NO_OVERLAP(Cx_wo, B_Len, Resultx_wo, Result_len);

    // Highest word of Modulus Vector shall not be zero
    CHECK_PKARAM_WORD_NOT_ZERO(pa_wo, B_Len);

    // All input components(Vector B, C) must have 3(ABC_Len) or 2(ABC_Len)
    // buffer words after their MSW as well as each result
    // components(Vector D)

    // Modulus p must be a prime > 2^63
    // Effective modulus size(in bits) must be a multiple of 32
    // a < p and b < p
    // Neither pntA nor pntC can be the "point at infinity"

    // write the offset of vector A, B & C
    EIP28_A_PTR_WR(Device, k_wo);
    EIP28_B_PTR_WR(Device, pa_wo);
    EIP28_C_PTR_WR(Device, Cx_wo);
    EIP28_D_PTR_WR(Device, Resultx_wo);

    TrueIOArea_p->LastOperationUsedSequencer = true;
    TrueIOArea_p->LastOperation_ResultLen = VLEN(B_Len); // caller knows!

    // write the lengths
    EIP28_A_LEN_WR(Device, A_Len);
    EIP28_B_LEN_WR(Device, B_Len);

    // Set the ECCMUL run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_SEQ_ECCMONTMUL);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif /* EIP28_REMOVE_ECCMULMONT */

/*----------------------------------------------------------------------------
 * StartOp_DSA_Sign_CALLATOMIC
 */
#ifndef EIP28_REMOVE_DSASIGN
EIP28_Status_t
EIP28_StartOp_DSA_Sign_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t alpha_wo,
    const EIP28_WordOffset_t pgn_wo,
    const EIP28_WordOffset_t h_wo,
    const EIP28_WordOffset_t inputResult_wo,
    const unsigned int A_Len,
    const unsigned int B_Len,
    const unsigned int S)
{
#ifdef EIP28_STRICT_ARGS
    int Result_len;
#endif
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

    // maximum vector length is 4160
    EIP28_CHECK_INT_INRANGE(A_Len, 3, 4160 / 32); // 2 < A_len <= 130
    EIP28_CHECK_INT_INRANGE(B_Len, 3, 4160 / 32); // 2 < B_len <= 130
#ifdef EIP28_MONTGOMERY_LADDER_OPTION
    EIP28_CHECK_INT_INRANGE(S, 0, 16); // 0 <= B_len <= 16
#else
    EIP28_CHECK_INT_INRANGE(S, 1, 16); // 0 <  B_len <= 16
#endif

#ifdef EIP28_STRICT_ARGS
    // Result Vector size
    Result_len = EIP28_GetWorkAreaSize(DSA_SIGN, 0, A_Len, B_Len);

    // check the offset of the Vector
    // note: last X bytes of PKARAM are used as scratchpad
    EIP28_CHECK_INT_ATMOST(
        alpha_wo + B_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(DSA_SIGN));
    EIP28_CHECK_INT_ATMOST(
        pgn_wo + 2 * A_Len + B_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(DSA_SIGN));
    EIP28_CHECK_INT_ATMOST(
        h_wo + B_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(DSA_SIGN));
    EIP28_CHECK_INT_ATMOST(
        inputResult_wo + Result_len,
        (int)TrueIOArea_p->PKARamNrOfWords -
            EIP28_GetScratchpadSize(DSA_SIGN));
#endif

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(alpha_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(pgn_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(h_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(inputResult_wo);

    // check for no overlap of Result with alpha
    EIP28_CHECK_NO_OVERLAP(alpha_wo, A_Len, inputResult_wo, Result_len);
    // check for no overlap of Result with pgn
    EIP28_CHECK_NO_OVERLAP(pgn_wo, 2 * A_Len + B_Len, inputResult_wo, Result_len);
    // check for no overlap of Result with h
    EIP28_CHECK_NO_OVERLAP(h_wo, B_Len, inputResult_wo, Result_len);

    // All input components(Vector B, C) must have 3(ABC_Len) or 2(ABC_Len)
    // buffer words after their MSW as well as each result
    // components(Vector D)

    // Modulus p must be a prime > 2^63
    // Effective modulus size(in bits) must be a multiple of 32
    // a < p and b < p
    // Neither pntA nor pntC can be the "point at infinity"

    // write the offset of vector A, B & C
    EIP28_A_PTR_WR(Device, alpha_wo);
    EIP28_B_PTR_WR(Device, pgn_wo);
    EIP28_C_PTR_WR(Device, h_wo);
    EIP28_D_PTR_WR(Device, inputResult_wo);

    TrueIOArea_p->LastOperationUsedSequencer = true;
    TrueIOArea_p->LastOperation_ResultLen = VLEN2A(B_Len) + VLEN(B_Len); // caller knows!

    // write the lengths
    EIP28_A_LEN_WR(Device, A_Len);
    EIP28_B_LEN_WR(Device, B_Len);
    EIP28_SHIFT_WR(Device, S);

    // Set the ECCMUL run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_SEQ_DSASIGN);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif // EIP28_REMOVE_DSASIGN

/*----------------------------------------------------------------------------
 * StartOp_DSA_Verify_CALLATOMIC
 */
#ifndef EIP28_REMOVE_DSAVERIFY
EIP28_Status_t
EIP28_StartOp_DSA_Verify_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t y_wo,
    const EIP28_WordOffset_t pgn_wo,
    const EIP28_WordOffset_t h_wo,
    const EIP28_WordOffset_t inputResult_wo,
    const unsigned int A_Len, // used for k only
    const unsigned int B_Len,
    const unsigned int S)
{
#ifdef EIP28_STRICT_ARGS
    int Result_len;
#endif
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

    // maximum vector length is 4160
    EIP28_CHECK_INT_INRANGE(A_Len, 3, 4160 / 32); // 2 < A_len <= 130
    EIP28_CHECK_INT_INRANGE(B_Len, 3, 4160 / 32); // 2 < B_len <= 130
#ifdef EIP28_MONTGOMERY_LADDER_OPTION
    EIP28_CHECK_INT_INRANGE(S, 0, 16); // 0 <= B_len <= 16
#else
    EIP28_CHECK_INT_INRANGE(S, 1, 16); // 0 <  B_len <= 16
#endif

#ifdef EIP28_STRICT_ARGS
    // Result Vector size
    Result_len = EIP28_GetWorkAreaSize(DSA_VERIFY, 0, A_Len, B_Len);

    // check the offset of the Vector
    // note: last X bytes of PKARAM are used as scratchpad
    EIP28_CHECK_INT_ATMOST(
        y_wo + A_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(DSA_VERIFY));
    EIP28_CHECK_INT_ATMOST(
        pgn_wo + 2 * A_Len + B_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(DSA_VERIFY));
    EIP28_CHECK_INT_ATMOST(
        h_wo + B_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(DSA_VERIFY));
    EIP28_CHECK_INT_ATMOST(
        inputResult_wo + Result_len,
        (int)TrueIOArea_p->PKARamNrOfWords -
            EIP28_GetScratchpadSize(DSA_VERIFY));
#endif

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(y_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(pgn_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(h_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(inputResult_wo);

    // check for no overlap of Result with y
    EIP28_CHECK_NO_OVERLAP(y_wo, A_Len, inputResult_wo, Result_len);
    // check for no overlap of Result with pgn
    EIP28_CHECK_NO_OVERLAP(pgn_wo, 2 * A_Len + B_Len, inputResult_wo, Result_len);
    // check for no overlap of Result with h
    EIP28_CHECK_NO_OVERLAP(h_wo, B_Len, inputResult_wo, Result_len);
    // check for no overlap of Result with rs

    // Highest word of sub-prime Vector shall not be zero
    CHECK_PKARAM_WORD_NOT_ZERO(pgn_wo, A_Len);

    // All input components(Vector B, C) must have 3(ABC_Len) or 2(ABC_Len)
    // buffer words after their MSW as well as each result
    // components(Vector D)

    // Modulus p must be a prime > 2^63
    // Effective modulus size(in bits) must be a multiple of 32
    // a < p and b < p
    // Neither pntA nor pntC can be the "point at infinity"

    // write the offset of vector A, B & C
    EIP28_A_PTR_WR(Device, y_wo);
    EIP28_B_PTR_WR(Device, pgn_wo);
    EIP28_C_PTR_WR(Device, h_wo);
    EIP28_D_PTR_WR(Device, inputResult_wo);

    TrueIOArea_p->LastOperationUsedSequencer = true;
    TrueIOArea_p->LastOperation_ResultLen = VLEN2A(B_Len) + VLEN(B_Len); // caller knows!

    // write the lengths
    EIP28_A_LEN_WR(Device, A_Len);
    EIP28_B_LEN_WR(Device, B_Len);
    EIP28_SHIFT_WR(Device, S);

    // Set the ECCMUL run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_SEQ_DSAVERIFY);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif // EIP28_REMOVE_DSAVERIFY

/*----------------------------------------------------------------------------
 * StartOp_ECCDSA_Sign_CALLATOMIC
 */
#ifndef EIP28_REMOVE_ECCDSASIGN
EIP28_Status_t
EIP28_StartOp_ECCDSA_Sign_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t k_wo,
    const EIP28_WordOffset_t pab_wo,
    const EIP28_WordOffset_t h_wo,
    const EIP28_WordOffset_t inputResult_wo,
    const unsigned int A_Len, // used for k only
    const unsigned int B_Len)
{
#ifdef EIP28_STRICT_ARGS
    int Result_len;
#endif
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

    // maximum vector length is 768 bits
    EIP28_CHECK_INT_INRANGE(A_Len, 1, 768 / 32); // 0 < A_len <= 24
    EIP28_CHECK_INT_INRANGE(B_Len, 2, 768 / 32); // 1 < B_len <= 24

#ifdef EIP28_STRICT_ARGS
    // Result Vector size
    Result_len = EIP28_GetWorkAreaSize(ECCDSA_SIGN, 0, A_Len, B_Len);

    // check the offset of the Vector
    // note: last X bytes of PKARAM are used as scratchpad
    EIP28_CHECK_INT_ATMOST(
        k_wo + B_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECCDSA_SIGN));
    EIP28_CHECK_INT_ATMOST(
        pab_wo + 7 * B_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECCDSA_SIGN));
    EIP28_CHECK_INT_ATMOST(
        h_wo + B_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECCDSA_SIGN));
    EIP28_CHECK_INT_ATMOST(
        inputResult_wo + Result_len,
        (int)TrueIOArea_p->PKARamNrOfWords -
            EIP28_GetScratchpadSize(ECCDSA_SIGN));
#endif

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(k_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(pab_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(h_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(inputResult_wo);

    // check for no overlap of Result with k
    EIP28_CHECK_NO_OVERLAP(k_wo, A_Len, inputResult_wo, Result_len);
    // check for no overlap of Result with pab
    EIP28_CHECK_NO_OVERLAP(pab_wo, 7 * B_Len, inputResult_wo, Result_len);
    // check for no overlap of Result with C
    EIP28_CHECK_NO_OVERLAP(h_wo, B_Len, inputResult_wo, Result_len);

    // Highest word of Modulus Vector shall not be zero
    CHECK_PKARAM_WORD_NOT_ZERO(pab_wo, B_Len);

    // All input components(Vector B, C) must have 3(ABC_Len) or 2(ABC_Len)
    // buffer words after their MSW as well as each result
    // components(Vector D)

    // Modulus p must be a prime > 2^63
    // Effective modulus size(in bits) must be a multiple of 32
    // a < p and b < p
    // Neither pntA nor pntC can be the "point at infinity"

    // write the offset of vector A, B & C
    EIP28_A_PTR_WR(Device, k_wo);
    EIP28_B_PTR_WR(Device, pab_wo);
    EIP28_C_PTR_WR(Device, h_wo);
    EIP28_D_PTR_WR(Device, inputResult_wo);

    TrueIOArea_p->LastOperationUsedSequencer = true;
    TrueIOArea_p->LastOperation_ResultLen = VLEN2A(B_Len) + VLEN(B_Len); // caller knows!

    // write the lengths
    EIP28_A_LEN_WR(Device, A_Len);
    EIP28_B_LEN_WR(Device, B_Len);

    // Set the ECCMUL run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_SEQ_ECCDSASIGN);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif // EIP28_REMOVE_ECCDSASIGN

/*----------------------------------------------------------------------------
 * StartOp_ECCDSA_Verify_CALLATOMIC
 */
#ifndef EIP28_REMOVE_ECCDSAVERIFY
EIP28_Status_t
EIP28_StartOp_ECCDSA_Verify_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t k_wo,
    const EIP28_WordOffset_t pab_wo,
    const EIP28_WordOffset_t Cxy_wo,
    const EIP28_WordOffset_t inputResult_wo,
    const unsigned int A_Len, // used for k only
    const unsigned int B_Len)
{
#ifdef EIP28_STRICT_ARGS
    int Result_len;
#endif
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

    // maximum vector length is 768 bits
    EIP28_CHECK_INT_INRANGE(A_Len, 1, 768 / 32); // 0 < A_len <= 24
    EIP28_CHECK_INT_INRANGE(B_Len, 2, 768 / 32); // 1 < B_len <= 24

#ifdef EIP28_STRICT_ARGS
    // Result Vector size
    Result_len = EIP28_GetWorkAreaSize(ECCDSA_VERIFY, 0, A_Len, B_Len);

    // check the offset of the Vector
    // note: last X bytes of PKARAM are used as scratchpad
    EIP28_CHECK_INT_ATMOST(
        k_wo + A_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECCDSA_VERIFY));
    EIP28_CHECK_INT_ATMOST(
        pab_wo + 3 * B_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECCDSA_VERIFY));
    EIP28_CHECK_INT_ATMOST(
        Cxy_wo + 2 * B_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECCDSA_VERIFY));
    EIP28_CHECK_INT_ATMOST(
        inputResult_wo + Result_len,
        (int)TrueIOArea_p->PKARamNrOfWords -
            EIP28_GetScratchpadSize(ECCDSA_VERIFY));
#endif

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(k_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(pab_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Cxy_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(inputResult_wo);

    // check for no overlap of Result with k
    EIP28_CHECK_NO_OVERLAP(k_wo, 3 * B_Len, inputResult_wo, Result_len);
    // check for no overlap of Result with pab
    EIP28_CHECK_NO_OVERLAP(pab_wo, 7 * B_Len, inputResult_wo, Result_len);
    // check for no overlap of Result with C
    EIP28_CHECK_NO_OVERLAP(Cxy_wo, B_Len, inputResult_wo, Result_len);

    // Highest word of Modulus Vector shall not be zero
    CHECK_PKARAM_WORD_NOT_ZERO(pab_wo, B_Len);

    // All input components(Vector B, C) must have 3(ABC_Len) or 2(ABC_Len)
    // buffer words after their MSW as well as each result
    // components(Vector D)

    // Modulus p must be a prime > 2^63
    // Effective modulus size(in bits) must be a multiple of 32
    // a < p and b < p
    // Neither pntA nor pntC can be the "point at infinity"

    // write the offset of vector A, B & C
    EIP28_A_PTR_WR(Device, k_wo);
    EIP28_B_PTR_WR(Device, pab_wo);
    EIP28_C_PTR_WR(Device, Cxy_wo);
    EIP28_D_PTR_WR(Device, inputResult_wo);

    TrueIOArea_p->LastOperationUsedSequencer = true;
    TrueIOArea_p->LastOperation_ResultLen = VLEN2A(B_Len) + VLEN(B_Len); // caller knows!

    // write the lengths
    EIP28_A_LEN_WR(Device, A_Len);
    EIP28_B_LEN_WR(Device, B_Len);

    // Set the ECCMUL run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_SEQ_ECCDSAVERIFY);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif // EIP28_REMOVE_ECCDSAVERIFY

#ifndef EIP28_REMOVE_SCAP
/*----------------------------------------------------------------------------
 * EIP28_StartOp_ModExpScap_CALLATOMIC
 */
EIP28_Status_t
EIP28_StartOp_ModExpScap_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t A_wo,
    const EIP28_WordOffset_t B_wo,
    const EIP28_WordOffset_t C_wo,
    const EIP28_WordOffset_t Result_wo,
    const unsigned int B_Len,
    const unsigned int Flags)
{
    int Result_len;
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;
    // 1 < B_Len <= Max_Len
    EIP28_CHECK_INT_INRANGE(B_Len, 2, EIP28_VECTOR_MAXLEN);
    // Check Flags
    EIP28_CHECK_INT_ATMOST(Flags, 3);

    Result_len = EIP28_GetWorkAreaSize(MODEXP_SCAP,
                                       0,
                                       0,
                                       B_Len);

    // check the offset A_wo, B_wo, C_wo
    // The last X bytes of PKA RAM will be used as general scratchpad, and
    // should not overlap with any of input vectors and result vector
    EIP28_CHECK_INT_ATMOST(
        A_wo + B_Len + 3,
        TrueIOArea_p->PKARamNrOfWords - EIP28_MODEXP_SCAP_SCRATCHPAD_WORD_COUNT);
    switch (Flags)
    {
    case 0: // Blind, UnBlind, no phiN
        EIP28_CHECK_INT_ATMOST(
            B_wo + (3 * B_Len) + 3,
            TrueIOArea_p->PKARamNrOfWords - EIP28_MODEXP_SCAP_SCRATCHPAD_WORD_COUNT);
        break;
    case 1: // No Blind, No UnBlind, no phiN
        EIP28_CHECK_INT_ATMOST(
            B_wo + B_Len + 1,
            TrueIOArea_p->PKARamNrOfWords - EIP28_MODEXP_SCAP_SCRATCHPAD_WORD_COUNT);
        break;
    case 2: // Blind + UnBlind + phiN
        EIP28_CHECK_INT_ATMOST(
            B_wo + (4 * B_Len) + 4,
            TrueIOArea_p->PKARamNrOfWords - EIP28_MODEXP_SCAP_SCRATCHPAD_WORD_COUNT);
        break;
    case 3: // No Blind, No UnBlind, phiN
        EIP28_CHECK_INT_ATMOST(
            B_wo + (2 * B_Len) + 2,
            TrueIOArea_p->PKARamNrOfWords - EIP28_MODEXP_SCAP_SCRATCHPAD_WORD_COUNT);
        break;
    }
    EIP28_CHECK_INT_ATMOST(
        C_wo + B_Len + 1,
        TrueIOArea_p->PKARamNrOfWords - EIP28_MODEXP_SCAP_SCRATCHPAD_WORD_COUNT);
    EIP28_CHECK_INT_ATMOST(
        Result_wo + Result_len,
        (int)TrueIOArea_p->PKARamNrOfWords - EIP28_MODEXP_SCAP_SCRATCHPAD_WORD_COUNT);

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(A_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(B_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(C_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Result_wo);

    // Modulus B must be odd
    CHECK_PKARAM_WORD_IS_ODD(B_wo);

    // Vector D may not overlap with any of the vectors
    // check for no overlap of result vector with vector A
    EIP28_CHECK_NO_OVERLAP(A_wo, B_Len, Result_wo, Result_len);
    // check for no overlap of result vector with vector B
    EIP28_CHECK_NO_OVERLAP(B_wo, B_Len, Result_wo, Result_len);

    // Vector D may not overlap vector C,
    // except that PKA_CPTR = PKA_DPTR is allowed
    if (C_wo != Result_wo)
    {
        EIP28_CHECK_NO_OVERLAP(C_wo, B_Len, Result_wo, Result_len);
    }

    // write the offset of operand A, operand B, operand C and result
    EIP28_A_PTR_WR(Device, A_wo);
    EIP28_B_PTR_WR(Device, B_wo);
    EIP28_C_PTR_WR(Device, C_wo);
    EIP28_D_PTR_WR(Device, Result_wo);
    TrueIOArea_p->LastOperation_Result_wo = Result_wo;
    TrueIOArea_p->LastOperationUsedSequencer = true;
    TrueIOArea_p->LastOperation_ResultLen = B_Len;

    // write the ALen and BLen
    EIP28_A_LEN_WR(Device, B_Len); // Not Used but must be B_Len
    EIP28_B_LEN_WR(Device, B_Len);

    // write the Flags
    EIP28_SHIFT_WR(Device, Flags);

    // Set the modexpvar bit & run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_SEQ_MODEXP_SCAP);

    IDENTIFIER_NOT_USED(Result_len);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}

/*----------------------------------------------------------------------------
 * StartOp_ModExpCrtScap
 */
EIP28_Status_t
EIP28_StartOp_ModExpCrtScap_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t A_wo,
    const EIP28_WordOffset_t B_wo,
    const EIP28_WordOffset_t C_wo,
    const EIP28_WordOffset_t InputResult_wo,
    const unsigned int A_Len,
    const unsigned int B_Len)
{
    int Result_len;
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;
    // 0 <= A_Len <= Max_Len
    EIP28_CHECK_INT_ATMOST(A_Len, EIP28_VECTOR_MAXLEN);
    // 1 < B_Len <= Max_Len
    EIP28_CHECK_INT_INRANGE(B_Len, 2, EIP28_VECTOR_MAXLEN);

    Result_len = EIP28_GetWorkAreaSize(MODEXP_CRT_SCAP, 0, A_Len, B_Len);

    // check that the vectors are inside the available RAM
    // The last X bytes of PKA RAM will be used as general scratchpad, and
    // should not overlap with any of input vectors and D vector
    EIP28_CHECK_INT_ATMOST(
        A_wo + (2 * B_Len) + A_Len + 6,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(MODEXP_CRT_SCAP));
    // misses alignment spacer
    EIP28_CHECK_INT_ATMOST(
        B_wo + (6 * B_Len) + 4,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(MODEXP_CRT_SCAP));
    // misses buffer words
    EIP28_CHECK_INT_ATMOST(
        C_wo + B_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(MODEXP_CRT_SCAP));
    EIP28_CHECK_INT_ATMOST(
        InputResult_wo + (unsigned int)Result_len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(MODEXP_CRT_SCAP));

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(A_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(B_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(C_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(InputResult_wo);

    // Modulus p must be odd
    CHECK_PKARAM_WORD_IS_ODD(B_wo);
    // Modulus q must be odd
    // Must round to next aligned address for this test
    CHECK_PKARAM_WORD_IS_ODD(
        EIP28_Memory_IncAndAlignNext64(
            (EIP28_WordOffset_t)(B_wo + B_Len + 1)));

    // 0 < ExpP < (Mod P - 1) & 0 < ExpQ < (Mod Q - 1)
    // Mod P > Mod Q > 2^32
    // Mod P and Mod Q must be co-prime
    // Input < (Mod P * Mod Q)
    // (Q inverse * Mod P) = 1 (Mod Q)
    // Mod P and Mod Q must be followed by an empty 32-bit buffer word

    // Vector D may not overlap with any of the vectors
    // check for no overlap of vector D with  vector A
    // misses alignment spacer
    EIP28_CHECK_NO_OVERLAP(A_wo, (2 * B_Len) + A_Len + 6, InputResult_wo, Result_len);
    // check for no overlap of vector D with  vector B
    // misses buffer words
    EIP28_CHECK_NO_OVERLAP(B_wo, (6 * B_Len) + 4, InputResult_wo, Result_len);
    // check for no overlap of vector D with  vector C
    EIP28_CHECK_NO_OVERLAP(C_wo, B_Len, InputResult_wo, Result_len);

    // write the offset of operand A, operand B, operand C and result
    EIP28_A_PTR_WR(Device, A_wo);
    EIP28_B_PTR_WR(Device, B_wo);
    EIP28_C_PTR_WR(Device, C_wo);
    EIP28_D_PTR_WR(Device, InputResult_wo);
    TrueIOArea_p->LastOperation_Result_wo = InputResult_wo;
    TrueIOArea_p->LastOperationUsedSequencer = true;
    TrueIOArea_p->LastOperation_ResultLen = 2U * B_Len; // caller knows!

    // write the ALen, BLen
    EIP28_A_LEN_WR(Device, A_Len);
    EIP28_B_LEN_WR(Device, B_Len);
    // shift must be 0
    EIP28_SHIFT_WR(Device, 0);

    // Set the modexpvar bit & run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_SEQ_MODEXPCRT_SCAP);

    IDENTIFIER_NOT_USED(Result_len);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
/*----------------------------------------------------------------------------
 * EIP28_StartOp_ECCMulProjScap
 */
EIP28_Status_t
EIP28_StartOp_ECCMulProjScap_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t k_wo,
    const EIP28_WordOffset_t pabn_wo,
    const EIP28_WordOffset_t P1xy_wo,
    const EIP28_WordOffset_t Resultxyz_wo,
    const unsigned int B_Len)
{
#ifdef EIP28_STRICT_ARGS
    int Result_len;
#endif
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

    // maximum vector length is 768 bits
    EIP28_CHECK_INT_INRANGE(B_Len, 2, 768 / 32); // 1 < B_len <= 24

#ifdef EIP28_STRICT_ARGS
    // Result Vector size
    Result_len = EIP28_GetWorkAreaSize(ECC_MUL_PROJ_SCAP, 0, 0, B_Len);

    // check the offset of the Vector
    // note: last X bytes of PKARAM are used as scratchpad
    EIP28_CHECK_INT_ATMOST(
        k_wo + B_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECC_MUL_PROJ_SCAP));
    EIP28_CHECK_INT_ATMOST(
        pabn_wo + 4 * B_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECC_MUL_PROJ_SCAP));
    EIP28_CHECK_INT_ATMOST(
        P1xy_wo + 3 * B_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECC_MUL_PROJ_SCAP));
    EIP28_CHECK_INT_ATMOST(
        Resultxyz_wo + Result_len,
        (int)TrueIOArea_p->PKARamNrOfWords -
            EIP28_GetScratchpadSize(ECC_MUL_PROJ_SCAP));
#endif

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(k_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(pabn_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(P1xy_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Resultxyz_wo);

    // check for no overlap of Result with k
    EIP28_CHECK_NO_OVERLAP(k_wo, B_Len, Resultxyz_wo, Result_len);
    // check for no overlap of Result with pab
    EIP28_CHECK_NO_OVERLAP(pabn_wo, 4 * B_Len, Resultxyz_wo, Result_len);
    // check for no overlap of Result with C
    EIP28_CHECK_NO_OVERLAP(P1xy_wo, 2 * B_Len, Resultxyz_wo, Result_len);

    // Highest word of Modulus Vector shall not be zero
    CHECK_PKARAM_WORD_NOT_ZERO(pabn_wo, B_Len);

    // All input components(Vector B, C) must have 3(ABC_Len) or 2(ABC_Len)
    // buffer words after their MSW as well as each result
    // components(Vector D)

    // Modulus p must be a prime > 2^63
    // Effective modulus size(in bits) must be a multiple of 32
    // a < p and b < p
    // Neither pntA nor pntC can be the "point at infinity"

    // write the offset of vector A, B & C
    EIP28_A_PTR_WR(Device, k_wo);
    EIP28_B_PTR_WR(Device, pabn_wo);
    EIP28_C_PTR_WR(Device, P1xy_wo);
    EIP28_D_PTR_WR(Device, Resultxyz_wo);

    TrueIOArea_p->LastOperationUsedSequencer = true;
    TrueIOArea_p->LastOperation_ResultLen = 3 * VLEN2A(B_Len);

    // write the lengths
    EIP28_A_LEN_WR(Device, B_Len); // Not Used but must be B_Len
    EIP28_B_LEN_WR(Device, B_Len);

    // Set the ECCMUL run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_SEQ_ECCMUL_SCAP);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}

/*----------------------------------------------------------------------------
 * EIP28_StartOp_ECCMulMontScap_CALLATOMIC
 */
EIP28_Status_t
EIP28_StartOp_ECCMulMontScap_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t k_wo,
    const EIP28_WordOffset_t pan_wo,
    const EIP28_WordOffset_t p1x_wo,
    const EIP28_WordOffset_t Resultx_wo,
    const unsigned int B_Len)
{
#ifdef EIP28_STRICT_ARGS
    int Result_len;
#endif
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

    // maximum vector length is 768 bits
    EIP28_CHECK_INT_INRANGE(B_Len, 2, 768 / 32); // 1 < B_len <= 24

#ifdef EIP28_STRICT_ARGS
    // Result Vector size
    Result_len = EIP28_GetWorkAreaSize(ECC_MUL_MONT_SCAP, 0, 0, B_Len);

    // check the offset of the Vector
    // note: last X bytes of PKARAM are used as scratchpad
    EIP28_CHECK_INT_ATMOST(
        k_wo + B_Len + 3,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECC_MUL_MONT_SCAP));
    EIP28_CHECK_INT_ATMOST(
        pan_wo + (3 * B_Len) + 6,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECC_MUL_MONT_SCAP));
    EIP28_CHECK_INT_ATMOST(
        p1x_wo + B_Len + 2,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECC_MUL_MONT_SCAP));
    EIP28_CHECK_INT_ATMOST(
        Resultx_wo + Result_len,
        (int)TrueIOArea_p->PKARamNrOfWords -
            EIP28_GetScratchpadSize(ECC_MUL_MONT_SCAP));
#endif

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(k_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(pan_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(p1x_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Resultx_wo);

    // check for no overlap of Result with k
    EIP28_CHECK_NO_OVERLAP(k_wo, B_Len + 3, Resultx_wo, Result_len);
    // check for no overlap of Result with pa
    EIP28_CHECK_NO_OVERLAP(pan_wo, (3 * B_Len) + 6, Resultx_wo, Result_len);
    // check for no overlap of Result with C.x
    EIP28_CHECK_NO_OVERLAP(p1x_wo, B_Len + 2, Resultx_wo, Result_len);

    // Highest word of Modulus Vector shall not be zero
    CHECK_PKARAM_WORD_NOT_ZERO(pan_wo, B_Len);

    // All input components(Vector B, C) must have 3(ABC_Len) or 2(ABC_Len)
    // buffer words after their MSW as well as each result
    // components(Vector D)

    // Modulus p must be a prime > 2^63
    // Effective modulus size(in bits) must be a multiple of 32
    // a < p and b < p
    // Neither pntA nor pntC can be the "point at infinity"

    // write the offset of vector A, B & C
    EIP28_A_PTR_WR(Device, k_wo);
    EIP28_B_PTR_WR(Device, pan_wo);
    EIP28_C_PTR_WR(Device, p1x_wo);
    EIP28_D_PTR_WR(Device, Resultx_wo);

    TrueIOArea_p->LastOperationUsedSequencer = true;
    TrueIOArea_p->LastOperation_ResultLen = B_Len; // caller knows!

    // write the ALen and BLen
    EIP28_A_LEN_WR(Device, B_Len); // Not Used but must be B_Len
    EIP28_B_LEN_WR(Device, B_Len);

    // Set the ECCMUL run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_SEQ_ECCMONTMUL_SCAP);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}

EIP28_Status_t
EIP28_StartOp_DSASignScap_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t alpha_wo,
    const EIP28_WordOffset_t pgn_wo,
    const EIP28_WordOffset_t h_wo,
    const EIP28_WordOffset_t inputResult_wo,
    const unsigned int A_Len,
    const unsigned int B_Len)
{
#ifdef EIP28_STRICT_ARGS
    int Result_len;
#endif
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

    // maximum vector length is 4160
    EIP28_CHECK_INT_INRANGE(A_Len, 3, 4160 / 32); // 2 < A_len <= 130
    EIP28_CHECK_INT_INRANGE(B_Len, 3, 4160 / 32); // 2 < B_len <= 130

#ifdef EIP28_STRICT_ARGS
    // Result Vector size
    Result_len = EIP28_GetWorkAreaSize(DSA_SIGN_SCAP, 0, A_Len, B_Len);

    // check the offset of the Vector
    // note: last X bytes of PKARAM are used as scratchpad
    EIP28_CHECK_INT_ATMOST(
        alpha_wo + B_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(DSA_SIGN_SCAP));
    EIP28_CHECK_INT_ATMOST(
        pgn_wo + (2 * A_Len) + B_Len + 6,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(DSA_SIGN_SCAP));
    EIP28_CHECK_INT_ATMOST(
        h_wo + B_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(DSA_SIGN_SCAP));
    EIP28_CHECK_INT_ATMOST(
        inputResult_wo + Result_len,
        (int)TrueIOArea_p->PKARamNrOfWords -
            EIP28_GetScratchpadSize(DSA_SIGN_SCAP));
#endif

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(alpha_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(pgn_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(h_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(inputResult_wo);

    // check for no overlap of Result with alpha
    EIP28_CHECK_NO_OVERLAP(alpha_wo, B_Len, inputResult_wo, Result_len);
    // check for no overlap of Result with pgn
    EIP28_CHECK_NO_OVERLAP(pgn_wo, (2 * A_Len) + B_Len + 6, inputResult_wo, Result_len);
    // check for no overlap of Result with h
    EIP28_CHECK_NO_OVERLAP(h_wo, B_Len, inputResult_wo, Result_len);

    // All input components(Vector B, C) must have 3(ABC_Len) or 2(ABC_Len)
    // buffer words after their MSW as well as each result
    // components(Vector D)

    // Modulus p must be a prime > 2^63
    // Effective modulus size(in bits) must be a multiple of 32
    // a < p and b < p
    // Neither pntA nor pntC can be the "point at infinity"

    // write the offset of vector A, B & C
    EIP28_A_PTR_WR(Device, alpha_wo);
    EIP28_B_PTR_WR(Device, pgn_wo);
    EIP28_C_PTR_WR(Device, h_wo);
    EIP28_D_PTR_WR(Device, inputResult_wo);

    TrueIOArea_p->LastOperationUsedSequencer = true;
    TrueIOArea_p->LastOperation_ResultLen = VLEN2A(B_Len) + VLEN(B_Len);

    // write the lengths
    EIP28_A_LEN_WR(Device, A_Len);
    EIP28_B_LEN_WR(Device, B_Len);

    // Set the ECCMUL run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_SEQ_DSASIGN_SCAP);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}

EIP28_Status_t
EIP28_StartOp_ECCDSASignScap_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t alpha_wo,
    const EIP28_WordOffset_t pabnxy_wo,
    const EIP28_WordOffset_t h_wo,
    const EIP28_WordOffset_t inputResult_wo,
    const unsigned int B_Len)
{
#ifdef EIP28_STRICT_ARGS
    int Result_len;
#endif
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

    // maximum vector length is 768 bits
    EIP28_CHECK_INT_INRANGE(B_Len, 2, 768 / 32); // 1 < B_len <= 24

#ifdef EIP28_STRICT_ARGS
    // Result Vector size
    Result_len = EIP28_GetWorkAreaSize(ECCDSA_SIGN_SCAP, 0, 0, B_Len);

    // check the offset of the Vector
    // note: last X bytes of PKARAM are used as scratchpad
    EIP28_CHECK_INT_ATMOST(
        alpha_wo + B_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECCDSA_SIGN_SCAP));
    EIP28_CHECK_INT_ATMOST(
        pabnxy_wo + (6 * B_Len) + 12,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECCDSA_SIGN_SCAP));
    EIP28_CHECK_INT_ATMOST(
        h_wo + B_Len,
        TrueIOArea_p->PKARamNrOfWords -
            (unsigned int)EIP28_GetScratchpadSize(ECCDSA_SIGN_SCAP));
    EIP28_CHECK_INT_ATMOST(
        inputResult_wo + Result_len,
        (int)TrueIOArea_p->PKARamNrOfWords -
            EIP28_GetScratchpadSize(ECCDSA_SIGN_SCAP));
#endif

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(alpha_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(pabnxy_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(h_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(inputResult_wo);

    // check for no overlap of Result with k
    EIP28_CHECK_NO_OVERLAP(alpha_wo, B_Len, inputResult_wo, Result_len);
    // check for no overlap of Result with pab
    EIP28_CHECK_NO_OVERLAP(pabnxy_wo, (6 * B_Len) + 12, inputResult_wo, Result_len);
    // check for no overlap of Result with C
    EIP28_CHECK_NO_OVERLAP(h_wo, B_Len, inputResult_wo, Result_len);

    // Highest word of Modulus Vector shall not be zero
    CHECK_PKARAM_WORD_NOT_ZERO(pabnxy_wo, B_Len);

    // All input components(Vector B, C) must have 3(ABC_Len) or 2(ABC_Len)
    // buffer words after their MSW as well as each result
    // components(Vector D)

    // Modulus p must be a prime > 2^63
    // Effective modulus size(in bits) must be a multiple of 32
    // a < p and b < p
    // Neither pntA nor pntC can be the "point at infinity"

    // write the offset of vector A, B & C
    EIP28_A_PTR_WR(Device, alpha_wo);
    EIP28_B_PTR_WR(Device, pabnxy_wo);
    EIP28_C_PTR_WR(Device, h_wo);
    EIP28_D_PTR_WR(Device, inputResult_wo);

    TrueIOArea_p->LastOperationUsedSequencer = true;
    TrueIOArea_p->LastOperation_ResultLen = VLEN2A(B_Len) + VLEN(B_Len);

    // write the ALen and BLen
    EIP28_A_LEN_WR(Device, B_Len); // Not Used but must be B_Len
    EIP28_B_LEN_WR(Device, B_Len);

    // Set the ECCMUL run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_SEQ_ECCDSASIGN_SCAP);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}

EIP28_Status_t
EIP28_StartOp_PrepBlindMsgScap_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p,
    const EIP28_WordOffset_t A_wo,
    const EIP28_WordOffset_t B_wo,
    const EIP28_WordOffset_t Result_wo,
    const unsigned int A_Len,
    const unsigned int B_Len)
{
#ifdef EIP28_STRICT_ARGS
    unsigned int Result_Len;
#endif
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;

    EIP28_CHECK_INT_ATMOST(A_Len, EIP28_VECTOR_MAXLEN);
    EIP28_CHECK_INT_ATMOST(B_Len, EIP28_VECTOR_MAXLEN);

#ifdef EIP28_STRICT_ARGS
    Result_Len = (2 * B_Len) + 2;
    // check the offset A_wo, B_wo, C_wo
    EIP28_CHECK_INT_ATMOST(A_wo + A_Len, TrueIOArea_p->PKARamNrOfWords);
    EIP28_CHECK_INT_ATMOST(B_wo + B_Len, TrueIOArea_p->PKARamNrOfWords);
    EIP28_CHECK_INT_ATMOST((Result_wo + Result_Len + 1),
                           TrueIOArea_p->PKARamNrOfWords);
#endif

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(A_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(B_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Result_wo);

    // Modulus N must be odd
    CHECK_PKARAM_WORD_IS_ODD(B_wo);

    // check for overlap of vector C with  vector A
    EIP28_CHECK_MEMORY_OVERLAP(A_wo, A_Len, Result_wo);
    // check for overlap of vector C with  vector B
    EIP28_CHECK_MEMORY_OVERLAP(B_wo, B_Len, Result_wo);

    // write the offset of operand A, operand B and result
    EIP28_A_PTR_WR(Device, A_wo);
    EIP28_B_PTR_WR(Device, B_wo);
    EIP28_D_PTR_WR(Device, Result_wo);
    TrueIOArea_p->LastOperation_Result_wo = Result_wo;
    TrueIOArea_p->LastOperationUsedSequencer = false;

    // write the ALen and BLen
    EIP28_A_LEN_WR(Device, A_Len);
    EIP28_B_LEN_WR(Device, B_Len);

    // Set the Add bit and run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
        Device,
        EIP28_FUNCTION_SEQ_PREP_BLIND_MSG);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif /* EIP28_REMOVE_SCAP */

/*----------------------------------------------------------------------------
 * EIP28_MakePkaRamAccessible
 */
void EIP28_MakePkaRamAccessible(
    const Device_Handle_t Device)
{
    EIP28_Write32(Device, EIP28_OFFSET_SEQCTRL, 0);
}

/*----------------------------------------------------------------------------
 * EIP28_CheckFsmError_SCAPCtrl_CALLATOMIC
 */
bool EIP28_CheckFsmError_SCAPCtrl_CALLATOMIC(
    EIP28_IOArea_t *const IOArea_p)
{
    volatile EIP28_True_IOArea_t *const TrueIOArea_p = IOAREA(IOArea_p);
    Device_Handle_t Device = TrueIOArea_p->Device;
    bool rv = (EIP28_SCAP_CTRL_RD(Device) & EIP28_SCAP_CTRL_FSM_ERROR);

    if (rv)
    {
        // Reset fsm_error bit
        EIP28_SCAP_CTRL_FSM_ERROR_RESET(Device);
    }

    return rv;
}

/*----------------------------------------------------------------------------
 * EIP28_GetScratchpadSize
 *
 * Output:
 *      ResultLen is in 32-bit words
 */
int EIP28_GetScratchpadSize(
    const EIP28_Operation_List_t op)
{
    unsigned int ResultLen;

    // Switch between the operation
    switch (op)
    {
    case MULTIPLY:
        ResultLen = 3;
        break;

    case ADD:
    case SUBTRACT:
    case ADDSUB:
    case RIGHTSHIFT:
    case LEFTSHIFT:
    case DIVIDE:
    case MODULO:
    case COMPARE:
    case COPY:
        ResultLen = 0;
        break;

    case MODEXP:
        ResultLen = EIP28_MODEXP_SCRATCHPAD_WORD_COUNT;
        break;

    case MODEXP_CRT_SCAP:
        ResultLen = EIP28_MODEXP_SCAP_SCRATCHPAD_WORD_COUNT;
        break;

    case MODEXP_SCAP:
        ResultLen = 9;
        break;

    case MODEXP_CRT:
        ResultLen = 11;
        break;

    case MODINV:
        ResultLen = 12;
        break;

    case PREP_BLIND_MSG:
        ResultLen = 15;
        break;

    case DSA_SIGN_SCAP:
        ResultLen = 17;
        break;

    case ECC_ADD_AFFINE:
    case ECC_ADD_PROJECTIVE:
    case ECC_MUL_AFFINE:
    case ECC_MUL_PROJECTIVE:
    case ECC_MUL_MONT:
        ResultLen = 19;
        break;

    case DSA_SIGN:
    case DSA_VERIFY:
    case ECCDSA_SIGN:
    case ECCDSA_VERIFY:
        ResultLen = 20;
        break;

    case ECC_MUL_PROJ_SCAP:
    case ECC_MUL_MONT_SCAP:
        ResultLen = 21;
        break;

    case ECCDSA_SIGN_SCAP:
        ResultLen = 25;
        break;

    default:
        ResultLen = -1; // error for now, but extend this when needed
    }

    return ResultLen;
}

/*----------------------------------------------------------------------------
 * EIP28_GetPkaRamSize
 */
uint32_t
EIP28_GetPkaRamSize(
    EIP28_IOArea_t *const IOArea_p)
{
    volatile EIP28_True_IOArea_t *const TrueIOArea_p = IOAREA(IOArea_p);

    return TrueIOArea_p->PKARamNrOfWords;
}

/* end of file eip28.c */

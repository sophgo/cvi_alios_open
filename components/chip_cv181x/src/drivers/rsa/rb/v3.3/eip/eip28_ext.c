/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */
/* eip28_ext.c
 *
 * Driver Library for the Security-IP-EIP-28 Public Key Accelerator.
 * Hardware-specific implementation for EIP28_PKCP.
 */



/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

// EIP28 Driver Library API, functions:
#include "eip28.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */
#include "c_eip28.h"            // configuration options

#include "eip28_level0.h"       // EIP28 register access

#include "eip28_internal.h"     // Internal definitions.


/*----------------------------------------------------------------------------
 * EIP28_GetWorkAreaSize
 *
 * Output:
 *      ResultLen is in 32-bit words
 */
int
EIP28_GetWorkAreaSize(
        const EIP28_Operation_List_t op,
        const unsigned int NrOfOddPowers,
        const unsigned int ALen,
        const unsigned int ModLen)
{
    unsigned int ResultLen = 0;

    // Switch between the operation
    switch (op)
    {
        case MULTIPLY:
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
            // Work space with PKCP is (#odd powers + 2) x (B_Len + 2) + 2
            ResultLen = (NrOfOddPowers + 2) * (ModLen + 2) + 2;
            break;

        case MODEXP_CRT:
            // Work space with PKCP is
            //   (#of odd powers + 2) x (BLen + 2) + 2
            ResultLen = (NrOfOddPowers + 2) * (ModLen + 2) + 2;
            break;

        case ECC_ADD_PROJECTIVE:
        case ECC_ADD_AFFINE:
        case ECC_MUL_PROJECTIVE:
        case ECC_MUL_AFFINE:
        case ECC_MUL_MONT:
            // Work space with PKCP is
            // 20*(BLen + 2 + BLen MOD 2)
            ResultLen = 20 * (ModLen + 2 + (ModLen & 1));
            break;

        case MODINV:
            ResultLen = 4 * (MAX(ALen, ModLen) + 2 +
                                    (MAX(ALen, ModLen) & 1)) + 2;
            break;

        case DSA_SIGN:
            // Work space with PKCP is
            //   (2 * (BLen + 2 + BLen MOD 2)) + (#of odd powers + 3) x (ALen + 2) + 2
            // For Montgomery Ladder mode, calculate with #of odd powers = 1
#ifdef EIP28_MONTGOMERY_LADDER_OPTION
            ResultLen = (2 * (ModLen + 2 + (ModLen & 1))) + (1 + 3) * (ALen + 2) + 2;
#else
            ResultLen = (2 * (ModLen + 2 + (ModLen & 1))) + (ModLen + 3) * (ALen + 2) + 2;
#endif
            break;
        case DSA_VERIFY:
            // Work space with PKCP is
            //   (2 * (BLen + 2 + BLen MOD 2)) + (ALen + 2 + ALen MOD 2) +
            //   ((#of odd powers + 3) * (ALen + 2) + 2)
            // For Montgomery Ladder mode, calculate with #of odd powers = 1
#ifdef EIP28_MONTGOMERY_LADDER_OPTION
            ResultLen = (2 * (ModLen + 2 + (ModLen & 1))) + (ALen + 2 + (ALen & 1)) +
                                 ((1 + 3) * (ALen + 2) + 2);
#else
            ResultLen = (2 * (ModLen + 2 + (ModLen & 1))) + (ALen + 2 + (ALen & 1)) +
                                ((ModLen + 3) * (ALen + 2) + 2);
#endif
            break;
        case ECCDSA_SIGN:
            // Work space with PKCP is
            //   21*(BLen + 2 + BLen MOD 2)
            ResultLen = 21 * (ModLen + 2 + (ModLen & 1));
            break;
        case ECCDSA_VERIFY:
            // Work space with PKCP is
            //   27*(BLen + 2 + BLen MOD 2)
            ResultLen = 27 * (ModLen + 2 + (ModLen & 1));
            break;


        case MODEXP_SCAP:
            // Work space with PKCP is
            //   4*(BLen + 2)
            ResultLen = 4 * (ModLen + 2);
            break;

        case MODEXP_CRT_SCAP:
            // Work space with PKCP is
            //   7*(2 * BLen + 2) + 1
            ResultLen = 7 * (2 * ModLen + 2) + 1;
            break;

        case ECC_MUL_PROJ_SCAP:
        case ECC_MUL_MONT_SCAP:
            // Work space with PKCP is
            //   23*(BLen + 2 + BLen MOD 2)
            ResultLen = 23 * (ModLen + 2 + (ModLen & 1));
            break;

        case DSA_SIGN_SCAP:
            // Work space with PKCP is
            //   (5 * (ALen + 2 + ALen MOD 2)) + (BLen + 2 + BLen MOD 2) + 2
            ResultLen = (5 * (ALen + 2 + (ALen & 1))) + (ModLen + 2 + (ModLen & 1)) + 2;
            break;

        case ECCDSA_SIGN_SCAP:
            // Work space with PKCP is
            //   24*(BLen + 2 + BLen MOD 2)
            ResultLen = 24 * (ModLen + 2 + (ModLen & 1));
            break;

        case PREP_BLIND_MSG:
            // Work space with PKCP is
            //   6*(BLen + 2) + 1 + ((BLen & 1) * -3)
            ResultLen = 6 * (ModLen + 2) + 1 + ((ModLen & 1) * -3);
            break;

        default:
            ResultLen = 0;
            break;

    } // switch

    return (int)ResultLen;
}


/*----------------------------------------------------------------------------
 * StartOp_ModExp
 */
#ifndef EIP28_REMOVE_MODEXP
EIP28_Status_t
EIP28_StartOp_ModExp_CupAmodB_CALLATOMIC(
        EIP28_IOArea_t * const IOArea_p,
        const EIP28_WordOffset_t A_wo,
        const EIP28_WordOffset_t B_wo,
        const EIP28_WordOffset_t C_wo,
        const unsigned int A_Len,
        const unsigned int BC_Len,
        const unsigned int NrOfOddPowers,
        const EIP28_WordOffset_t Result_wo)
{
    int Result_len;
    EIP28_INSERTCODE_FUNCTION_ENTRY_CODE;
    // 0 < A_Len <= Max_Len
    EIP28_CHECK_INT_INRANGE(A_Len,  1, EIP28_VECTOR_MAXLEN);
    // 1 < B_Len <= Max_Len
    EIP28_CHECK_INT_INRANGE(BC_Len, 2, EIP28_VECTOR_MAXLEN);
#ifdef EIP28_MONTGOMERY_LADDER_OPTION
    // NrOfOddPowers <= 16 (0 means using MontgomeryLadder)
    EIP28_CHECK_INT_ATMOST(NrOfOddPowers, 16);
#else
    // 0 < NrOfOddPowers <= 16
    EIP28_CHECK_INT_INRANGE(NrOfOddPowers, 1, 16);
#endif

    Result_len = EIP28_GetWorkAreaSize(MODEXP,
                                       NrOfOddPowers,
                                       A_Len,
                                       BC_Len);

    // check the offset A_wo, B_wo, C_wo
    // The last X bytes of PKA RAM will be used as general scratchpad, and
    // should not overlap with any of input vectors and result vector
    EIP28_CHECK_INT_ATMOST(
            A_wo + A_Len,
            TrueIOArea_p->PKARamNrOfWords - EIP28_MODEXP_SCRATCHPAD_WORD_COUNT);
    EIP28_CHECK_INT_ATMOST(
            B_wo + BC_Len,
            TrueIOArea_p->PKARamNrOfWords - EIP28_MODEXP_SCRATCHPAD_WORD_COUNT);
    EIP28_CHECK_INT_ATMOST(
            C_wo + BC_Len,
            TrueIOArea_p->PKARamNrOfWords - EIP28_MODEXP_SCRATCHPAD_WORD_COUNT);
    EIP28_CHECK_INT_ATMOST(
            Result_wo + Result_len,
            (int) TrueIOArea_p->PKARamNrOfWords - EIP28_MODEXP_SCRATCHPAD_WORD_COUNT);

    // check offset to be 64-bit aligned
    EIP28_CHECK_64BIT_ALIGNMENT(A_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(B_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(C_wo);
    EIP28_CHECK_64BIT_ALIGNMENT(Result_wo);

    // Modulus B must be odd
    CHECK_PKARAM_WORD_IS_ODD(B_wo);

    // Base C < Modulus B

    // Modulus B > 2^32
    // BC_Len represents number of 32-bit words
    //if Length is less than one word report error
    EIP28_CHECK_INT_ATLEAST(BC_Len, 1);

    // Vector D may not overlap with any of the vectors
    // check for no overlap of result vector with vector A
    EIP28_CHECK_NO_OVERLAP(A_wo, A_Len, Result_wo, Result_len);
    // check for no overlap of result vector with vector B
    EIP28_CHECK_NO_OVERLAP(B_wo, BC_Len, Result_wo, Result_len);

    // Vector D may not overlap vector C,
    // except that PKA_CPTR = PKA_DPTR is allowed
    if (C_wo != Result_wo)
    {
        EIP28_CHECK_NO_OVERLAP(C_wo, BC_Len, Result_wo, Result_len);
    }

    // write the offset of operand A, operand B, operand C and result
    EIP28_A_PTR_WR(Device, A_wo);
    EIP28_B_PTR_WR(Device, B_wo);
    EIP28_C_PTR_WR(Device, C_wo);
    EIP28_D_PTR_WR(Device, Result_wo);
    TrueIOArea_p->LastOperation_Result_wo = Result_wo;
    TrueIOArea_p->LastOperationUsedSequencer = true;
    TrueIOArea_p->LastOperation_ResultLen = BC_Len;

    // write the ALen, BLen
    // EIP28_A_LEN_WR(Device, A_Len + 2);
    // EIP28_B_LEN_WR(Device, BC_Len + 2);
    EIP28_A_LEN_WR(Device, A_Len);
    EIP28_B_LEN_WR(Device, BC_Len);

    // write the NrofOddPowers
    EIP28_SHIFT_WR(Device, NrOfOddPowers);

    // Set the modexpvar bit & run bit in PKA_FUNCTION Register
    EIP28_FUNCTION_START_OPERATION(
            Device,
            EIP28_FUNCTION_SEQ_MODEXP);

    IDENTIFIER_NOT_USED(Result_len);

    EIP28_INSERTCODE_FUNCTION_EXIT_CODE;
}
#endif /* EIP28_REMOVE_MODEXP */


/*----------------------------------------------------------------------------
 * EIP28_ReadError_SCAP_Fsm_CALLATOMIC
 */
bool
EIP28_CheckFsmError_LNME0Status_CALLATOMIC(
        EIP28_IOArea_t * const IOArea_p)
{
    IDENTIFIER_NOT_USED(IOArea_p);
    // Register not available for PKCP configurations
    return false;
}


/*----------------------------------------------------------------------------
 * EIP28_Status_SeqCntrl_CALLATOMIC
 */
uint32_t
EIP28_Status_SeqCntrl_CALLATOMIC(
        EIP28_IOArea_t * const IOArea_p)
{
    volatile EIP28_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);
    Device_Handle_t Device = TrueIOArea_p->Device;

    // See if Sequencer status = 0x21 which means rejected command
    return EIP28_SEQUENCER_STATUS(Device);
}

/* end of file eip28_ext.c */

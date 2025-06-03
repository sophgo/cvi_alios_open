/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */
/* adapter_pka_eip28.c
 *
 * Implementation of PKA APIs.
 *
 * Adapter for PKA (EIP28 HW accelerator).
 *
 */

/****************************************************************************
 *                    PKA Includes and Variable's Instances                 *
 ****************************************************************************/

#include "c_adapter.h"
#include "clib.h"
#include "adapter_eip28.h"
#include "pka_opcodes_eip28.h"
#include "api_pka.h"
#include "eip28.h"
#include "basic_defs.h"
#include "device_types.h"
#include "dmares_types.h"
#include "dmares_rw.h"

#ifdef ADAPTER_EIP28_INTERRUPT_ENABLE
#include "adapter_interrupts.h" // Adapter_Interrupt_*, IRQ_EIP28_READY
#endif                          // ADAPTER_EIP28_INTERRUPT_ENABLE

#include "adapter_dmabuf.h"

/****************************************************************************
 *                    PKA Internal Data Structures and Defines              *
 ****************************************************************************/

#define LogMN "PKA_EIP28:: "
#define LogErr LogMN "Error::  "
#define LogWarn LogMN "Warning::  "
#define LogInfo LogMN "Info::  "

// Till PKA Debugging
//#define PKA_EXTREME_DEBUG
#ifdef PKA_EXTREME_DEBUG
#define LOG_FUNC_ENTER LOG_INFO(LogMN "Enter %s()\n", __FUNCTION__)
#define LOG_FUNC_EXIT LOG_INFO(LogMN "Exit  %s()\n", __FUNCTION__)
#define LOG_WARN_STRICTARGSFAILED                                              \
        LOG_WARN(LogWarn "(line %d) STRICT_ARGS check failed\n", __LINE__);
#else
#define LOG_FUNC_ENTER
#define LOG_FUNC_EXIT
#define LOG_WARN_STRICTARGSFAILED
#endif // End of PKA_EXTREME_DEBUG

#ifdef ADAPTER_PKA_STRICT_ARGS
#define PKA_CHECK_POINTER(_p)                                                  \
        if (NULL == (_p)) {                                                    \
                LOG_WARN_STRICTARGSFAILED;                                     \
                return PKA_ERROR_INVALID_PARAMETER;                            \
        }
#define PKA_CHECK_INT_INRANGE(_i, _min, _max)                                  \
        if ((_i) < (_min) || (_i) > (_max)) {                                  \
                LOG_WARN_STRICTARGSFAILED;                                     \
                return PKA_ERROR_INVALID_PARAMETER;                            \
        }
#define PKA_CHECK_INT_ATLEAST(_i, _min)                                        \
        if ((_i) < (_min)) {                                                   \
                LOG_WARN_STRICTARGSFAILED;                                     \
                return PKA_ERROR_INVALID_PARAMETER;                            \
        }
#define PKA_CHECK_INT_ATMOST(_i, _max)                                         \
        if ((_i) > (_max)) {                                                   \
                LOG_WARN_STRICTARGSFAILED;                                     \
                return PKA_ERROR_INVALID_PARAMETER;                            \
        }
#else
#define PKA_CHECK_POINTER(_p)
#define PKA_CHECK_INT_INRANGE(_i, _min, _max)
#define PKA_CHECK_INT_ATLEAST(_i, _min)
#define PKA_CHECK_INT_ATMOST(_i, _max)
#endif

#define VLEN(_l) ((_l) + 0)
#define VLEN1(_l) ((_l) + 1)
#define VLEN2(_l) ((_l) + 2)
#define VLEN3(_l) ((_l) + 3)
#define VLENA(_l) ((_l) + 0 + ((_l + 0) & 1))
#define VLEN1A(_l) ((_l) + 1 + ((_l + 1) & 1))
#define VLEN2A(_l) ((_l) + 2 + ((_l + 2) & 1))
#define VLEN3A(_l) ((_l) + 3 + ((_l + 3) & 1))

/*----------------------------------------------------------------------------
 * PKA Adapter Internal Data Structures
 *
 */

// Interrupt Handling
typedef struct {
        PKA_Session_t              Session;
        PKA_ResultNotifyFunction_t ResultNotifyCB;
} Adapter_PKA_Notify_t;

// Change it to match underlying hardware limitations
typedef unsigned short PKA_WordOffset_t;

// Change it to match underlying hardware limitations
typedef unsigned short PKA_VectorWordLength_t;

// Data structure to encapsulate data, state etc. info of PKA adapter
typedef struct {
        EIP28_IOArea_t *EIP28_IOArea_p;

        bool IsInitialized;

        // User provided Items
        PKA_Session_t        Session;
        const PKA_Command_t *Command_p;
        void *               UserHandle;

        // PKA required Vectors and params
        uint8_t *              Vectors[PKA_MAX_BUFFERS];
        PKA_WordOffset_t       VectorWordOffsets[PKA_MAX_BUFFERS];
        PKA_VectorWordLength_t VectorSize[PKA_MAX_BUFFERS]; // Calculated by PKA
        PKA_VectorWordLength_t
                                VectorBufferSize[PKA_MAX_BUFFERS]; // Given by App
        PKA_VectorWordLength_t *VSize;

        PKA_VectorWordLength_t A_Len; // In words
        PKA_VectorWordLength_t B_Len; // In words
        PKA_VectorWordLength_t Extra;

        // Number of Input Vectors and Index of first Result
        uint16_t NrInputVectors;

        // Packet successfully submitted
        unsigned int CommandPutSuccess;

        // ID of PKA Device Instance being used

        // Sync. and Locking Items

        // Interrupt Items (Callback, ISR, BH, IRQ NR)
        Adapter_PKA_Notify_t PKANotify;

} Adapter_PKA_Context_t;

// Mandatory spaces between Vectors placed in PKA RAM
// For Operations -
//      ModExp_CRT
#define PKA_VECTOR_GAP_TYPE1 (((PKACtx_p->B_Len) & 1) + 1)

// Mandatory spaces between Vectors placed in PKA RAM
// For Operations -
//      ECCAdd, ECCMul, ECDSASignGen, ECDSASignVerify,
//      DSASignGen, DSASignVerify
#define PKA_VECTOR_GAP_TYPE2 (3 - ((PKACtx_p->B_Len) & 1))

/****************************************************************************
 *                       PKA Internal Variables' Instances                   *
 ****************************************************************************/

// Context Instance
static Adapter_PKA_Context_t PKACtx;

// Supported Capability Table for PKA
static char ADAPTER_PKA_CAPABILITIES[] =
    "Multiply, Divide, Mod, ModExp, "
    "ModExpCRT, ModInv, ECCAdd, ECCMultiply, "
    "Copy, LeftShift, RightShift, Add, Sub, "
#ifndef ADAPTER_PKA_REMOVE_ECCMULMONT
    "Compare, AddSub, ECCMulMont";
#else
    "Compare, AddSub";
#endif

#ifndef ADAPTER_PKA_REMOVE_ECCMULMONT
static unsigned int ADAPTER_PKA_CAP_NR = 18;
#else
static unsigned int ADAPTER_PKA_CAP_NR = 17;
#endif

// Supported Capability Table for Firmware 'E' config
static char ADAPTER_PKA_CAPABILITIES_E[] = ", DSASign, DSAVerify, "
                                           "ECCDSASign, ECCDSAVerify";

static unsigned int ADAPTER_PKA_CAP_E_NR = 4;

// Supported Capability Table for Firmware 'S' config
static char ADAPTER_PKA_CAPABILITIES_S[] = ", ModExp-scap, ModExpCRT-scap, "
                                           "ECCMultiply-scap, ECCMulMont-scap, "
                                           "prep-blind-msg";

static unsigned int ADAPTER_PKA_CAP_S_NR = 5;

// Supported Capability Table for Firmware 'ES' config
static char ADAPTER_PKA_CAPABILITIES_ES[] = ", DSASign-scap, ECCDSASign-scap";

static unsigned int ADAPTER_PKA_CAP_ES_NR = 2;

/****************************************************************************
 *                       PKA Internal Functions' Definition                 *
 ****************************************************************************/

/*----------------------------------------------------------------------------
 * Adapter_PKA_BigIntegerCompare
 *
 * 1. if A > B
 * -1, if A < B
 * 0, if A == B
 */
static inline int
Adapter_PKA_BigIntegerCompare(const EIP28_BigUInt_t *const BigUInt_A_p,
                              const EIP28_BigUInt_t *const BigUInt_B_p)
{
        int          i;
        unsigned int A_ByteLen, B_ByteLen;
        A_ByteLen = BigUInt_A_p->ByteCount;
        B_ByteLen = BigUInt_B_p->ByteCount;
        /* assume littleE */
        if (A_ByteLen > B_ByteLen)
                return 1;
        else if (A_ByteLen < B_ByteLen)
                return -1;

        i = A_ByteLen;
        if (BigUInt_A_p->StoreAsMSB == false) {
                while (i--)
                        if (BigUInt_A_p->Bytes_p[i] != BigUInt_B_p->Bytes_p[i])
                                break;

                if (i == -1)
                        return 0;
                else if (BigUInt_A_p->Bytes_p[i] > BigUInt_B_p->Bytes_p[i])
                        return 1;
                else
                        return -1;
        } else {
                i = 0;
                while (i < A_ByteLen) {
                        if (BigUInt_A_p->Bytes_p[i] != BigUInt_B_p->Bytes_p[i])
                                break;
                        i++;
                }

                if (i == A_ByteLen)
                        return 0;
                else if (BigUInt_A_p->Bytes_p[i] > BigUInt_B_p->Bytes_p[i])
                        return 1;
                else
                        return -1;
        }
}

/*----------------------------------------------------------------------------
 * Adapter_PKA_CheckIfDone
 *
 * Checks if the operation has completed its execution.
 *
 * Return Value
 *      0  Operation still executing
 *      1  Operation execution done
 */
static inline bool
Adapter_PKA_CheckIfDone(Adapter_PKA_Context_t *const PKACtx_p)
{
        bool IsDone = false;
        EIP28_CheckIfDone((PKACtx_p->EIP28_IOArea_p), &IsDone);
        return IsDone;
}

/*-----------------------------------------------------------------------------
 * Adapter_PKA_SetupVectors
 *
 * a. Get required parameters to PKACtx from external sources (alien structs)
 *      (this structure instance will remain in cache, but not sure
 *      for alien structure where are they lying.)
 * b. Convert the User Provided References to Device/Adapter readable format
 * c. Check Parameters to be valid and in range for each command
 * d. Check if vectors will not overflow device memory
 * e. Setup result vector index based on command
 */
static int Adapter_PKA_SetupVectors(unsigned int           VectorCount,
                                    Adapter_PKA_Context_t *PKACtx_p)
{
        unsigned int cnt;

        LOG_FUNC_ENTER;
        for (cnt = 0; cnt < VectorCount; cnt++) {
                PKACtx_p->Vectors[cnt] =
                    (uint8_t *)PKACtx_p->Command_p->Handles[cnt].p;
                PKACtx_p->VectorBufferSize[cnt] =
                    PKACtx_p->Command_p->Handles[cnt].size;
                PKACtx_p->VectorSize[cnt] =
                    PKACtx_p->Command_p->Handles[cnt].size;
                //PKACtx_p->VectorBufferSize[cnt] = PKACtx_p->VectorSize[cnt];
                // PKACtx_p->VectorBufferSize[cnt] = (PKA_VectorWordLength_t)((DMARec_p->Props.Size + 3) / 4);
        }
        PKACtx_p->A_Len = PKACtx_p->Command_p->A_Len;
        PKACtx_p->B_Len = PKACtx_p->Command_p->B_Len;
        PKACtx_p->Extra = PKACtx_p->Command_p->Extra;

        // Vector Size - App. provided or PKA Calculated
        PKACtx_p->VSize = PKACtx_p->VectorSize;

        LOG_FUNC_EXIT;
        return 0;
}

#define PKA_MODEXPCRT_MOD_LENGTH(_len) (_len) + (_len) + 2 - ((_len)&1)

#define PKA_MODEXPCRT_EXP_LENGTH(_len) (_len) + (_len) + ((_len)&1)

/*-----------------------------------------------------------------------------
 * Adapter_PKA_CalculateWordOffsetAndVectorLenght
 *
 * Fills all the WordOffsets of vectors for a requested operation
 * Checks for Vectors to be lying inside the PKA RAM
 */
static int Adapter_PKA_Validate_CalculateWordOffsetAndVectorLenght(
    Adapter_PKA_Context_t *PKACtx_p)
{
        PKA_VectorWordLength_t A_Len, B_Len;
        PKA_WordOffset_t       A_wo, B_wo, C_wo;
        PKA_WordOffset_t       Res1_wo; //, Res2_wo;
        int                    Opr = 0;
        PKA_VectorWordLength_t WorkAreaSize;
#ifdef ADAPTER_PKA_STRICT_ARGS
        int PkaRamSize = EIP28_GetPkaRamSize(PKACtx_p->EIP28_IOArea_p);
#endif
        IDENTIFIER_NOT_USED(WorkAreaSize);

#define V PKACtx_p->Vectors
#define VWo PKACtx_p->VectorWordOffsets
#define VBufSize PKACtx_p->VectorBufferSize // Provided by App
#define VASize PKACtx_p->VectorSize         // Calculated Size (A=actual)

        LOG_FUNC_ENTER;

        // Get Length of Vectors and other Parameters
        A_Len = PKACtx_p->Command_p->A_Len;
        B_Len = PKACtx_p->Command_p->B_Len;
        Opr   = PKACtx_p->Command_p->OpCode;

        switch (Opr) {
                case PKA_OPCODE_MODEXP: {
                        EIP28_BigUInt_t BigUInt_Mod, BigUInt_Base;
                        PKA_CHECK_INT_INRANGE(A_Len, 1,
                                              ADAPTER_PKA_VECTOR_MAX_WORDS);
                        PKA_CHECK_INT_INRANGE(B_Len, 1,
                                              ADAPTER_PKA_VECTOR_MAX_WORDS);

                        // Get Vectors from Command
                        if (Adapter_PKA_SetupVectors(4, PKACtx_p) != 0) {

                                LOG_CRIT("%s, %d, 0x%x\n", __FUNCTION__,
                                         __LINE__, 0);
                                return PKA_ERROR_INVALID_PARAMETER;
                        }

                        WorkAreaSize =
                            (PKA_VectorWordLength_t)EIP28_GetWorkAreaSize(
                                MODEXP, PKACtx_p->Extra, A_Len,
                                B_Len); // In words

                        // Size of each Vector
                        VASize[0] = A_Len; // Exp
                        VASize[1] = B_Len; // Mod
                        VASize[2] = B_Len; // Base
                        //VASize[3] = B_Len+1; // Result
                        VASize[3] = B_Len; // Result

                        // Word Offsets of each Vector
                        VWo[0] = A_wo = 0;                // Exponent
                        B_wo          = A_wo + VASize[0]; // Modulus
                        /* buf +2 */
                        VWo[1] = B_wo = EIP28_Memory_IncAndAlignNext64(B_wo);
                        C_wo          = B_wo + VASize[1]; // Base
                        VWo[2] = C_wo = EIP28_Memory_IncAndAlignNext64(C_wo);
#ifdef ADAPTER_PKA_OVERLAP_INPUT_RESULT
                        VWo[3] = Res1_wo = C_wo;
#else
                        Res1_wo = C_wo + VASize[1]; // Modulo Result
                        VWo[3]  = Res1_wo =
                            EIP28_Memory_IncAndAlignNext64(Res1_wo);
#endif
                        // Check if all vector lie inside PKA RAM
                        PKA_CHECK_INT_ATMOST(
                            Res1_wo + WorkAreaSize +
                                EIP28_GetScratchpadSize(MODEXP),
                            PkaRamSize);

                        // Odd Power Range (0 = Montgomery Ladder)
                        PKA_CHECK_INT_ATMOST(PKACtx_p->Extra, 16);

                        // Check if Application Provided Buffers are sufficient enough
                        PKA_CHECK_INT_ATLEAST(VBufSize[0], VASize[0]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[1], VASize[1]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[2], VASize[2]);
                        VBufSize[3] = VASize[3];
                        PKA_CHECK_INT_ATLEAST(VBufSize[3], VASize[3]);

                        // C < B
                        BigUInt_Mod.Bytes_p     = V[1];
                        BigUInt_Mod.ByteCount   = B_Len * 4;
                        BigUInt_Mod.StoreAsMSB  = true;
                        BigUInt_Base.Bytes_p    = V[2];
                        BigUInt_Base.ByteCount  = B_Len * 4;
                        BigUInt_Base.StoreAsMSB = true;
                        if (Adapter_PKA_BigIntegerCompare(&BigUInt_Base,
                                                          &BigUInt_Mod) >= 0) {
                                LOG_WARN(
                                    LogWarn
                                    "Base must be less than the Modulus\n");
                                return PKA_ERROR_INVALID_PARAMETER;
                        }

                        // Number of Input Vectors and Index of result
                        PKACtx_p->NrInputVectors = 3;

                        break;
                }
                case PKA_OPCODE_MODEXP_CRT: {
                        EIP28_BigUInt_t BigUInt_ModP, BigUInt_ModQ;
                        unsigned int    modq_offs;

                        PKA_VectorWordLength_t ExpBuf_WordLen, ModBuf_WordLen;

                        // A_Len = ExpP and ExpQ
                        // B_Len = ModP, ModQ, InvQ

                        PKA_CHECK_INT_INRANGE(A_Len, 1,
                                              ADAPTER_PKA_VECTOR_MAX_WORDS);
                        PKA_CHECK_INT_INRANGE(B_Len, 1,
                                              ADAPTER_PKA_VECTOR_MAX_WORDS);

                        // Get Vectors from Command
                        if (Adapter_PKA_SetupVectors(4, PKACtx_p) != 0)
                                return PKA_ERROR_INVALID_PARAMETER;

                        WorkAreaSize =
                            (PKA_VectorWordLength_t)EIP28_GetWorkAreaSize(
                                MODEXP_CRT, PKACtx_p->Extra, A_Len,
                                B_Len); // In words

                        // Size of each Vector
                        // Each vector have sub-vectors placed back to back
                        // Size of Exponent and Modulus Vectors
                        ExpBuf_WordLen = PKA_MODEXPCRT_EXP_LENGTH(A_Len);
                        ModBuf_WordLen = PKA_MODEXPCRT_MOD_LENGTH(B_Len);

                        VASize[0] = ExpBuf_WordLen; // ExpP&Q + Alignment
                        VASize[1] =
                            ModBuf_WordLen; // ModP&Q + Alignment + MandtoryBuf
                        VASize[2] = B_Len;  // InvQ
                        VASize[3] = 2 * B_Len; // Input Text

                        // Word Offsets of each Vector
                        VWo[0] = A_wo = 0;                // Exponent
                        B_wo          = A_wo + VASize[0]; // Modulus
                        VWo[1] = B_wo = EIP28_Memory_IncAndAlignNext64(B_wo);
                        C_wo          = B_wo + VASize[1]; // Inverse
                        VWo[2] = C_wo = EIP28_Memory_IncAndAlignNext64(C_wo);
                        Res1_wo       = C_wo + VASize[2]; // Input/Result
                        VWo[3]        = Res1_wo =
                            EIP28_Memory_IncAndAlignNext64(Res1_wo);

                        // Check if all vector lie inside PKA RAM
                        PKA_CHECK_INT_ATMOST(
                            VWo[3] + WorkAreaSize +
                                EIP28_GetScratchpadSize(MODEXP_CRT),
                            PkaRamSize);

                        // Odd Power Range
                        PKA_CHECK_INT_INRANGE(PKACtx_p->Extra, 1, 16);

                        // Check if Application Provided Buffers are sufficient enough
                        PKA_CHECK_INT_ATLEAST(VBufSize[0], VASize[0]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[1], VASize[1]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[2], VASize[2]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[3], VASize[3]);

                        // ModQ < ModP
                        BigUInt_ModP.Bytes_p   = V[1];
                        BigUInt_ModP.ByteCount = B_Len * 4;
                        modq_offs              = B_Len * 4 + 8;
                        if (B_Len & 1) {
                                modq_offs -=
                                    4; // ModQ starts at even word address.
                        }
                        BigUInt_ModQ.Bytes_p   = V[1] + modq_offs;
                        BigUInt_ModQ.ByteCount = B_Len * 4;
                        if (Adapter_PKA_BigIntegerCompare(&BigUInt_ModQ,
                                                          &BigUInt_ModP) >= 0) {
                                LOG_WARN(LogWarn
                                         "ModQ must be less than ModP\n");
                                return PKA_ERROR_INVALID_PARAMETER;
                        }

                        // Number of Input Vectors
                        PKACtx_p->NrInputVectors = 3;

                        break;
                }
                case PKA_OPCODE_ECC_MULTIPLY_MONTGOMERY: {
                        int ScratchPadSize;
                        PKA_CHECK_INT_INRANGE(A_Len, 1, 24);
                        PKA_CHECK_INT_INRANGE(B_Len, 2, 24);

                        if (Adapter_PKA_SetupVectors(4, PKACtx_p) != 0)
                                return PKA_ERROR_INVALID_PARAMETER;

                        WorkAreaSize =
                            (PKA_VectorWordLength_t)EIP28_GetWorkAreaSize(
                                ECC_MUL_MONT, 0, A_Len,
                                B_Len); // In words
                        ScratchPadSize = EIP28_GetScratchpadSize(ECC_MUL_MONT);

                        // Vector Length (e.g. A.x + A.y)
                        // 1 Mandatory Word already included in V_Len
                        VASize[0] = VLEN(A_Len);                  // k
                        VASize[1] = VLEN2A(B_Len) + VLEN2(B_Len); // p + a
                        VASize[2] = VLEN2(B_Len);                 // C.x
                        VASize[3] = VLEN2A(B_Len);                // Res.x

                        // Word Offsets of each Vector
                        VWo[0] = A_wo = 0;                // k scalar
                        B_wo          = A_wo + VASize[0]; // C.x
                        VWo[1] = B_wo = EIP28_Memory_IncAndAlignNext64(B_wo);
                        C_wo          = B_wo + VASize[1] + 1; // p, a
                        VWo[2] = C_wo = EIP28_Memory_IncAndAlignNext64(C_wo);
                        Res1_wo = C_wo + VASize[2] + 1; // ECC_mont_Mul Result
                        VWo[3]  = Res1_wo =
                            EIP28_Memory_IncAndAlignNext64(Res1_wo);

                        // Check if all vector lie inside PKA RAM
                        PKA_CHECK_INT_ATMOST(Res1_wo + VASize[3] +
                                                 WorkAreaSize + ScratchPadSize,
                                             PkaRamSize);
                        IDENTIFIER_NOT_USED(ScratchPadSize);

                        // Check if Application Provided Buffers are sufficient enough
                        PKA_CHECK_INT_ATLEAST(VBufSize[0], VASize[0]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[1], VASize[1]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[2], VASize[2]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[3], VASize[3]);

                        // Number of Input Vectors
                        PKACtx_p->NrInputVectors = 3;
                        break;
                }
                case PKA_OPCODE_MODINV: {
                        PKA_CHECK_INT_INRANGE(A_Len, 1,
                                              ADAPTER_PKA_VECTOR_MAX_WORDS);
                        PKA_CHECK_INT_INRANGE(B_Len, 1,
                                              ADAPTER_PKA_VECTOR_MAX_WORDS);

                        if (Adapter_PKA_SetupVectors(3, PKACtx_p) != 0)
                                return PKA_ERROR_INVALID_PARAMETER;

                        WorkAreaSize =
                            (PKA_VectorWordLength_t)EIP28_GetWorkAreaSize(
                                MODINV, PKACtx_p->Extra, A_Len, B_Len);

                        VASize[0] = A_Len; // Num for Inv
                        VASize[1] = B_Len; // Base
                        VASize[2] = B_Len; // Result

                        // Word Offsets of each Vector
                        VWo[0] = A_wo = 0;
                        B_wo          = A_wo + VASize[0];
                        VWo[1] = B_wo = EIP28_Memory_IncAndAlignNext64(B_wo);
                        Res1_wo       = B_wo + VASize[1];
                        VWo[2]        = Res1_wo =
                            EIP28_Memory_IncAndAlignNext64(Res1_wo);

                        // Check if all vector lie inside PKA RAM
                        PKA_CHECK_INT_ATMOST(
                            Res1_wo + VASize[2] + WorkAreaSize +
                                EIP28_GetScratchpadSize(MODINV),
                            PkaRamSize);

                        // Check if Application Provided Buffers are sufficient enough
                        PKA_CHECK_INT_ATLEAST(VBufSize[0], VASize[0]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[1], VASize[1]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[2], VASize[2]);

                        // Number of Input Vectors
                        PKACtx_p->NrInputVectors = 2;

                        break;
                }
                case PKA_OPCODE_ECC_ADD_AFFINE: {
                        int ScratchPadSize;
                        PKA_CHECK_INT_INRANGE(B_Len, 1, 24);

                        if (Adapter_PKA_SetupVectors(4, PKACtx_p) != 0)
                                return PKA_ERROR_INVALID_PARAMETER;

                        WorkAreaSize =
                            (PKA_VectorWordLength_t)EIP28_GetWorkAreaSize(
                                ECC_ADD_AFFINE, 0, A_Len,
                                B_Len); // In words
                        ScratchPadSize =
                            EIP28_GetScratchpadSize(ECC_ADD_AFFINE);
                        VASize[0] = VLEN2A(B_Len) + VLEN2(B_Len); // A.x + A.y
                        VASize[1] = 2 * VLEN2A(B_Len) + VLEN2(B_Len);
                        ;                                         // p + a + b
                        VASize[2] = VLEN2A(B_Len) + VLEN2(B_Len); // C.x + C.y
                        VASize[3] = 2 * VLEN2A(B_Len); // Res.x + Res.y

                        // Word Offsets of each Vector
                        VWo[0] = A_wo = 0; // A.x and A.y
                        B_wo          = A_wo + VASize[0] +
                               1; // p, a and b + Mandatory Buffer
                        VWo[1] = B_wo = EIP28_Memory_IncAndAlignNext64(B_wo);
                        C_wo          = B_wo + VASize[1] +
                               1; // C.x and C.y (and C.z) + Mandatory Buffer
                        VWo[2] = C_wo = EIP28_Memory_IncAndAlignNext64(C_wo);
                        Res1_wo       = C_wo + VASize[2] + 1; // ECC_Add Result
                        VWo[3]        = Res1_wo =
                            EIP28_Memory_IncAndAlignNext64(Res1_wo);

                        // Check if all vector lie inside PKA RAM
                        PKA_CHECK_INT_ATMOST(Res1_wo + VASize[3] +
                                                 WorkAreaSize + ScratchPadSize,
                                             PkaRamSize);
                        IDENTIFIER_NOT_USED(ScratchPadSize);

                        // Check if Application Provided Buffers are sufficient enough
                        PKA_CHECK_INT_ATLEAST(VBufSize[0], VASize[0]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[1], VASize[1]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[2], VASize[2]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[3], VASize[3]);

                        // Number of Input Vectors
                        PKACtx_p->NrInputVectors = 3;

                        break;
                }
                case PKA_OPCODE_ECC_ADD_PROJECTIVE: {
                        int ScratchPadSize;
                        PKA_CHECK_INT_INRANGE(B_Len, 1, 24);

                        if (Adapter_PKA_SetupVectors(4, PKACtx_p) != 0)
                                return PKA_ERROR_INVALID_PARAMETER;

                        WorkAreaSize =
                            (PKA_VectorWordLength_t)EIP28_GetWorkAreaSize(
                                ECC_ADD_PROJECTIVE, 0, A_Len,
                                B_Len); // In words
                        ScratchPadSize =
                            EIP28_GetScratchpadSize(ECC_ADD_PROJECTIVE);

                        // Vector Length (e.g. A.x + A.y)
                        VASize[0] =
                            2 * VLEN2A(B_Len) + VLEN2(B_Len); // A.x + A.y + A.z
                        VASize[1] = 2 * VLEN2A(B_Len) + VLEN2(B_Len);
                        ; // p + a + b
                        VASize[2] =
                            2 * VLEN2A(B_Len) + VLEN2(B_Len); // C.x + C.y
                        VASize[3] = 3 * VLEN2A(B_Len); // Res.x + Res.y + Res.z

                        // Word Offsets of each Vector
                        VWo[0] = A_wo = 0; // A.x and A.y
                        B_wo          = A_wo + VASize[0] +
                               1; // p, a and b + Mandatory Buffer
                        VWo[1] = B_wo = EIP28_Memory_IncAndAlignNext64(B_wo);
                        C_wo          = B_wo + VASize[1] +
                               1; // C.x and C.y (and C.z) + Mandatory Buffer
                        VWo[2] = C_wo = EIP28_Memory_IncAndAlignNext64(C_wo);
                        Res1_wo       = C_wo + VASize[2] + 1; // ECC_Add Result
                        VWo[3]        = Res1_wo =
                            EIP28_Memory_IncAndAlignNext64(Res1_wo);

                        // Check if all vector lie inside PKA RAM
                        PKA_CHECK_INT_ATMOST(Res1_wo + VASize[3] +
                                                 WorkAreaSize + ScratchPadSize,
                                             PkaRamSize);
                        IDENTIFIER_NOT_USED(ScratchPadSize);

                        // Check if Application Provided Buffers are sufficient enough
                        PKA_CHECK_INT_ATLEAST(VBufSize[0], VASize[0]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[1], VASize[1]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[2], VASize[2]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[3], VASize[3]);

                        // Number of Input Vectors
                        PKACtx_p->NrInputVectors = 3;

                        break;
                }
                case PKA_OPCODE_ECC_MULTIPLY_PROJECTIVE: {
                        int ScratchPadSize;
                        PKA_CHECK_INT_INRANGE(A_Len, 1, 24);
                        PKA_CHECK_INT_INRANGE(B_Len, 2, 24);

                        if (Adapter_PKA_SetupVectors(4, PKACtx_p) != 0)
                                return PKA_ERROR_INVALID_PARAMETER;

                        WorkAreaSize =
                            (PKA_VectorWordLength_t)EIP28_GetWorkAreaSize(
                                ECC_MUL_PROJECTIVE, 0, A_Len,
                                B_Len); // In words
                        ScratchPadSize =
                            EIP28_GetScratchpadSize(ECC_MUL_PROJECTIVE);

                        // Vector Length (e.g. A.x + A.y)
                        // 1 Mandatory Word already included in V_Len
                        VASize[0] = VLEN(A_Len); // Scalar k
                        VASize[1] =
                            2 * VLEN2A(B_Len) + VLEN2(B_Len); // p + a + b
                        VASize[2] =
                            2 * VLEN2A(B_Len) + VLEN2(B_Len); // C.x + C.y + Rz
                        VASize[3] = 3 * VLEN2A(B_Len); // Res.x + Res.y + Res.z

                        // Word Offsets of each Vector
                        VWo[0] = A_wo = 0;         // k scalar
                        B_wo   = A_wo + VASize[0]; // C.x and C.y (and Rz)
                        VWo[1] = B_wo = EIP28_Memory_IncAndAlignNext64(B_wo);
                        C_wo          = B_wo + VASize[1] + 1; // p, a and b
                        VWo[2] = C_wo = EIP28_Memory_IncAndAlignNext64(C_wo);
                        Res1_wo       = C_wo + VASize[2] + 1; // ECC_Mul Result
                        VWo[3]        = Res1_wo =
                            EIP28_Memory_IncAndAlignNext64(Res1_wo);

                        // Check if all vector lie inside PKA RAM
                        PKA_CHECK_INT_ATMOST(Res1_wo + VASize[3] +
                                                 WorkAreaSize + ScratchPadSize,
                                             PkaRamSize);
                        IDENTIFIER_NOT_USED(ScratchPadSize);

                        // Check if Application Provided Buffers are sufficient enough
                        PKA_CHECK_INT_ATLEAST(VBufSize[0], VASize[0]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[1], VASize[1]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[2], VASize[2]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[3], VASize[3]);

                        // Number of Input Vectors
                        PKACtx_p->NrInputVectors = 3;
                        break;
                }
                case PKA_OPCODE_ECC_MULTIPLY_AFFINE: {
                        int ScratchPadSize;
                        PKA_CHECK_INT_INRANGE(A_Len, 1, 24);
                        PKA_CHECK_INT_INRANGE(B_Len, 2, 24);

                        if (Adapter_PKA_SetupVectors(4, PKACtx_p) != 0)
                                return PKA_ERROR_INVALID_PARAMETER;

                        WorkAreaSize =
                            (PKA_VectorWordLength_t)EIP28_GetWorkAreaSize(
                                ECC_MUL_AFFINE, 0, A_Len,
                                B_Len); // In words
                        ScratchPadSize =
                            EIP28_GetScratchpadSize(ECC_MUL_AFFINE);

                        // Vector Length (e.g. A.x + A.y)
                        // 1 Mandatory Word already included in V_Len
                        VASize[0] = VLEN(A_Len); // Scalar k
                        VASize[1] =
                            2 * VLEN2A(B_Len) + VLEN2(B_Len); // p + a + b
                        VASize[2] =
                            2 * VLEN2A(B_Len) + VLEN2(B_Len); // C.x + C.y
                        VASize[3] = 2 * VLEN2A(B_Len);        // Res.x + Res.y

                        // Word Offsets of each Vector
                        VWo[0] = A_wo = 0;         // k scalar
                        B_wo   = A_wo + VASize[0]; // C.x and C.y (and Rz)
                        VWo[1] = B_wo = EIP28_Memory_IncAndAlignNext64(B_wo);
                        C_wo          = B_wo + VASize[1] + 1; // p, a and b
                        VWo[2] = C_wo = EIP28_Memory_IncAndAlignNext64(C_wo);
                        Res1_wo       = C_wo + VASize[2] + 1; // ECC_Mul Result
                        VWo[3]        = Res1_wo =
                            EIP28_Memory_IncAndAlignNext64(Res1_wo);

                        // Check if all vector lie inside PKA RAM
                        PKA_CHECK_INT_ATMOST(Res1_wo + VASize[3] +
                                                 WorkAreaSize + ScratchPadSize,
                                             PkaRamSize);
                        IDENTIFIER_NOT_USED(ScratchPadSize);

                        // Check if Application Provided Buffers are sufficient enough
                        PKA_CHECK_INT_ATLEAST(VBufSize[0], VASize[0]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[1], VASize[1]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[2], VASize[2]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[3], VASize[3]);

                        // Number of Input Vectors
                        PKACtx_p->NrInputVectors = 3;
                        break;
                }
                case PKA_OPCODE_DSA_SIGN: {
                        int ScratchPadSize;
                        PKA_CHECK_INT_INRANGE(A_Len, 3,
                                              ADAPTER_PKA_VECTOR_MAX_WORDS);
                        PKA_CHECK_INT_INRANGE(B_Len, 3,
                                              ADAPTER_PKA_VECTOR_MAX_WORDS);

                        if (Adapter_PKA_SetupVectors(4, PKACtx_p) != 0)
                                return PKA_ERROR_INVALID_PARAMETER;

                        WorkAreaSize =
                            (PKA_VectorWordLength_t)EIP28_GetWorkAreaSize(
                                DSA_SIGN, 0, A_Len,
                                B_Len); // In words
                        ScratchPadSize = EIP28_GetScratchpadSize(DSA_SIGN);

                        // Vector Length (e.g. A.x + A.y)
                        VASize[0] = VLEN(B_Len);
                        VASize[1] = 2 * VLEN2A(A_Len) + VLEN2(B_Len);
                        VASize[2] = VLEN(B_Len);
                        // Output vector D is largest, so use that.
                        VASize[3] = 2 * VLEN2A(B_Len);

                        // Word Offsets of each Vector
                        VWo[0] = A_wo = 0;
                        B_wo          = A_wo + VASize[0];
                        VWo[1] = B_wo = EIP28_Memory_IncAndAlignNext64(B_wo);
                        C_wo          = B_wo + VASize[1];
                        VWo[2] = C_wo = EIP28_Memory_IncAndAlignNext64(C_wo);
                        Res1_wo       = C_wo + VASize[2];
                        VWo[3]        = Res1_wo =
                            EIP28_Memory_IncAndAlignNext64(Res1_wo);

                        // Check if all vector lie inside PKA RAM
                        PKA_CHECK_INT_ATMOST(Res1_wo + VASize[3] +
                                                 WorkAreaSize + ScratchPadSize,
                                             PkaRamSize);
                        IDENTIFIER_NOT_USED(ScratchPadSize);

                        // Check if Application Provided Buffers are sufficient enough
                        PKA_CHECK_INT_ATLEAST(VBufSize[0], VASize[0]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[1], VASize[1]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[2], VASize[2]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[3], VASize[3]);

                        // Number of Input Vectors
                        PKACtx_p->NrInputVectors = 3;

                        break;
                }
                case PKA_OPCODE_DSA_VERIFY: {
                        int ScratchPadSize;
                        PKA_CHECK_INT_INRANGE(A_Len, 3,
                                              ADAPTER_PKA_VECTOR_MAX_WORDS);
                        PKA_CHECK_INT_INRANGE(B_Len, 3,
                                              ADAPTER_PKA_VECTOR_MAX_WORDS);

                        if (Adapter_PKA_SetupVectors(4, PKACtx_p) != 0)
                                return PKA_ERROR_INVALID_PARAMETER;

                        WorkAreaSize =
                            (PKA_VectorWordLength_t)EIP28_GetWorkAreaSize(
                                DSA_VERIFY, 0, A_Len,
                                B_Len); // In words
                        ScratchPadSize = EIP28_GetScratchpadSize(DSA_VERIFY);

                        // Vector Length (e.g. A.x + A.y)
                        VASize[0] = VLEN(A_Len);
                        VASize[1] = 2 * VLEN2A(A_Len) + VLEN2(B_Len);
                        VASize[2] = VLEN(B_Len);
                        VASize[3] = 2 * VLEN2A(B_Len);

                        // Word Offsets of each Vector
                        VWo[0] = A_wo = 0;
                        B_wo          = A_wo + VASize[0];
                        VWo[1] = B_wo = EIP28_Memory_IncAndAlignNext64(B_wo);
                        C_wo          = B_wo + VASize[1];
                        VWo[2] = C_wo = EIP28_Memory_IncAndAlignNext64(C_wo);
                        Res1_wo       = C_wo + VASize[2];
                        VWo[3]        = Res1_wo =
                            EIP28_Memory_IncAndAlignNext64(Res1_wo);

                        // Check if all vector lie inside PKA RAM
                        PKA_CHECK_INT_ATMOST(Res1_wo + VASize[3] +
                                                 WorkAreaSize + ScratchPadSize,
                                             PkaRamSize);
                        IDENTIFIER_NOT_USED(ScratchPadSize);

                        // Check if Application Provided Buffers are sufficient enough
                        PKA_CHECK_INT_ATLEAST(VBufSize[0], VASize[0]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[1], VASize[1]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[2], VASize[2]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[3], VASize[3]);

                        // Number of Input Vectors
                        PKACtx_p->NrInputVectors = 3;

                        break;
                }
                case PKA_OPCODE_ECDSA_SIGN: {
                        int ScratchPadSize;
                        PKA_CHECK_INT_INRANGE(A_Len, 3,
                                              ADAPTER_PKA_VECTOR_MAX_WORDS);
                        PKA_CHECK_INT_INRANGE(B_Len, 3,
                                              ADAPTER_PKA_VECTOR_MAX_WORDS);

                        if (Adapter_PKA_SetupVectors(4, PKACtx_p) != 0)
                                return PKA_ERROR_INVALID_PARAMETER;

                        WorkAreaSize =
                            (PKA_VectorWordLength_t)EIP28_GetWorkAreaSize(
                                ECCDSA_SIGN, 0, A_Len,
                                B_Len); // In words
                        ScratchPadSize = EIP28_GetScratchpadSize(ECCDSA_SIGN);

                        // Vector Length (e.g. A.x + A.y)
                        VASize[0] = VLEN(B_Len);
                        VASize[1] = 6 * VLEN2A(B_Len) + VLEN2(B_Len);
                        VASize[2] = VLEN(B_Len);
                        // Vector D output is largest, so use that one.
                        VASize[3] = 2 * VLEN2A(B_Len);

                        // Word Offsets of each Vector
                        VWo[0] = A_wo = 0;
                        B_wo          = A_wo + VASize[0];
                        VWo[1] = B_wo = EIP28_Memory_IncAndAlignNext64(B_wo);
                        C_wo          = B_wo + VASize[1];
                        VWo[2] = C_wo = EIP28_Memory_IncAndAlignNext64(C_wo);
                        Res1_wo       = C_wo + VASize[2];
                        VWo[3]        = Res1_wo =
                            EIP28_Memory_IncAndAlignNext64(Res1_wo);

                        // Check if all vector lie inside PKA RAM
                        PKA_CHECK_INT_ATMOST(Res1_wo + VASize[3] +
                                                 WorkAreaSize + ScratchPadSize,
                                             PkaRamSize);
                        IDENTIFIER_NOT_USED(ScratchPadSize);

                        // Check if Application Provided Buffers are sufficient enough
                        PKA_CHECK_INT_ATLEAST(VBufSize[0], VASize[0]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[1], VASize[1]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[2], VASize[2]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[3], VASize[3]);

                        // Number of Input Vectors
                        PKACtx_p->NrInputVectors = 3;

                        break;
                }
                case PKA_OPCODE_ECDSA_VERIFY: {
                        int ScratchPadSize;
                        PKA_CHECK_INT_INRANGE(A_Len, 3,
                                              ADAPTER_PKA_VECTOR_MAX_WORDS);
                        PKA_CHECK_INT_INRANGE(B_Len, 3,
                                              ADAPTER_PKA_VECTOR_MAX_WORDS);

                        if (Adapter_PKA_SetupVectors(4, PKACtx_p) != 0)
                                return PKA_ERROR_INVALID_PARAMETER;

                        WorkAreaSize =
                            (PKA_VectorWordLength_t)EIP28_GetWorkAreaSize(
                                ECCDSA_VERIFY, 0, A_Len,
                                B_Len); // In words
                        ScratchPadSize = EIP28_GetScratchpadSize(ECCDSA_VERIFY);

                        // Vector Length (e.g. A.x + A.y)
                        VASize[0] = 2 * VLEN2A(B_Len) + VLEN(B_Len);
                        VASize[1] = 6 * VLEN2A(B_Len) + VLEN2(B_Len);
                        VASize[2] = VLEN(B_Len);
                        VASize[3] = 2 * VLEN2A(B_Len);

                        // Word Offsets of each Vector
                        VWo[0] = A_wo = 0;
                        B_wo          = A_wo + VASize[0];
                        VWo[1] = B_wo = EIP28_Memory_IncAndAlignNext64(B_wo);
                        C_wo          = B_wo + VASize[1];
                        VWo[2] = C_wo = EIP28_Memory_IncAndAlignNext64(C_wo);
                        Res1_wo       = C_wo + VASize[2];
                        VWo[3]        = Res1_wo =
                            EIP28_Memory_IncAndAlignNext64(Res1_wo);

                        // Check if all vector lie inside PKA RAM
                        PKA_CHECK_INT_ATMOST(Res1_wo + VASize[3] +
                                                 WorkAreaSize + ScratchPadSize,
                                             PkaRamSize);
                        IDENTIFIER_NOT_USED(ScratchPadSize);

                        // Check if Application Provided Buffers are sufficient enough
                        PKA_CHECK_INT_ATLEAST(VBufSize[0], VASize[0]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[1], VASize[1]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[2], VASize[2]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[3], VASize[3]);

                        // Number of Input Vectors
                        PKACtx_p->NrInputVectors = 3;

                        break;
                }
#ifdef ADAPTER_PKA_SCAP_ENABLE
                case PKA_OPCODE_MODEXP_SCAP: {
                        PKA_CHECK_INT_INRANGE(B_Len, 1,
                                              ADAPTER_PKA_VECTOR_MAX_WORDS);

                        // Get Vectors from Command
                        if (Adapter_PKA_SetupVectors(4, PKACtx_p) != 0)
                                return PKA_ERROR_INVALID_PARAMETER;

                        WorkAreaSize =
                            (PKA_VectorWordLength_t)EIP28_GetWorkAreaSize(
                                MODEXP_SCAP, 0, A_Len,
                                B_Len); // In words

                        // Size of each Vector
                        VASize[0] = VLEN3(B_Len);
                        // Allocate max size independent of the shift register bits
                        VASize[1] = 3 * VLEN1A(B_Len) + VLEN1(B_Len);
                        VASize[2] = VLEN1(B_Len);
                        VASize[3] = VLEN1A(B_Len);

                        // Word Offsets of each Vector
                        VWo[0] = A_wo = 0;
                        B_wo          = A_wo + VASize[0];
                        VWo[1] = B_wo = EIP28_Memory_IncAndAlignNext64(B_wo);
                        C_wo          = B_wo + VASize[1];
                        VWo[2] = C_wo = EIP28_Memory_IncAndAlignNext64(C_wo);
#ifdef ADAPTER_PKA_OVERLAP_INPUT_RESULT
                        VWo[3] = Res1_wo = C_wo;
#else
                        Res1_wo = C_wo + VASize[1];
                        VWo[3]  = Res1_wo =
                            EIP28_Memory_IncAndAlignNext64(Res1_wo);
#endif
                        // Check if all vector lie inside PKA RAM
                        PKA_CHECK_INT_ATMOST(
                            Res1_wo + WorkAreaSize +
                                EIP28_GetScratchpadSize(MODEXP_SCAP),
                            PkaRamSize);

                        // Check bits
                        PKA_CHECK_INT_ATMOST(PKACtx_p->Extra, 3);

                        // Check if Application Provided Buffers are sufficient enough
                        PKA_CHECK_INT_ATLEAST(VBufSize[0], VASize[0]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[1], VASize[1]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[2], VASize[2]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[3], VASize[3]);

                        // Number of Input Vectors and Index of result
                        PKACtx_p->NrInputVectors = 3;

                        break;
                }
                case PKA_OPCODE_MODEXP_CRT_SCAP: {
                        PKA_CHECK_INT_ATMOST(A_Len,
                                             ADAPTER_PKA_VECTOR_MAX_WORDS);
                        PKA_CHECK_INT_INRANGE(B_Len, 1,
                                              ADAPTER_PKA_VECTOR_MAX_WORDS);

                        // Get Vectors from Command
                        if (Adapter_PKA_SetupVectors(4, PKACtx_p) != 0)
                                return PKA_ERROR_INVALID_PARAMETER;

                        WorkAreaSize =
                            (PKA_VectorWordLength_t)EIP28_GetWorkAreaSize(
                                MODEXP_CRT_SCAP, PKACtx_p->Extra, A_Len,
                                B_Len); // In words

                        // Size of each Vector
                        VASize[0] = 2 * VLEN3A(B_Len) + VLEN(A_Len);
                        VASize[1] = 2 * VLEN1A(B_Len) + VLEN1A(2 * B_Len) +
                                    VLEN1(2 * B_Len);
                        VASize[2] = VLEN(B_Len);
                        VASize[3] = VLEN1A(2 * B_Len);

                        // Word Offsets of each Vector
                        VWo[0] = A_wo = 0;                // Exponent
                        B_wo          = A_wo + VASize[0]; // Modulus
                        VWo[1] = B_wo = EIP28_Memory_IncAndAlignNext64(B_wo);
                        C_wo          = B_wo + VASize[1]; // Inverse
                        VWo[2] = C_wo = EIP28_Memory_IncAndAlignNext64(C_wo);
                        Res1_wo       = C_wo + VASize[2]; // Input/Result
                        VWo[3]        = Res1_wo =
                            EIP28_Memory_IncAndAlignNext64(Res1_wo);

                        // Check if all vector lie inside PKA RAM
                        PKA_CHECK_INT_ATMOST(
                            VWo[3] + WorkAreaSize +
                                EIP28_GetScratchpadSize(MODEXP_CRT_SCAP),
                            PkaRamSize);

                        // Check if Application Provided Buffers are sufficient enough
                        PKA_CHECK_INT_ATLEAST(VBufSize[0], VASize[0]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[1], VASize[1]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[2], VASize[2]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[3], VASize[3]);

                        // Number of Input Vectors
                        PKACtx_p->NrInputVectors = 3;

                        break;
                }
                case PKA_OPCODE_ECC_MULTIPLY_PROJECTIVE_SCAP: {
                        int ScratchPadSize;
                        PKA_CHECK_INT_INRANGE(B_Len, 2, 24);

                        if (Adapter_PKA_SetupVectors(4, PKACtx_p) != 0)
                                return PKA_ERROR_INVALID_PARAMETER;

                        WorkAreaSize =
                            (PKA_VectorWordLength_t)EIP28_GetWorkAreaSize(
                                ECC_MUL_PROJ_SCAP, 0, 0,
                                B_Len); // In words
                        ScratchPadSize =
                            EIP28_GetScratchpadSize(ECC_MUL_PROJ_SCAP);

                        // Vector Lengths
                        VASize[0] = VLEN3(B_Len);
                        VASize[1] = 3 * VLEN2A(B_Len) + VLEN2(B_Len);
                        VASize[2] = 2 * VLEN2A(B_Len);
                        VASize[3] = 3 * VLEN2A(B_Len);

                        // Word Offsets of each Vector
                        VWo[0] = A_wo = 0;
                        B_wo          = A_wo + VASize[0]; //
                        VWo[1] = B_wo = EIP28_Memory_IncAndAlignNext64(B_wo);
                        C_wo          = B_wo + VASize[1] + 1;
                        VWo[2] = C_wo = EIP28_Memory_IncAndAlignNext64(C_wo);
                        Res1_wo       = C_wo + VASize[2] + 1;
                        VWo[3]        = Res1_wo =
                            EIP28_Memory_IncAndAlignNext64(Res1_wo);

                        // Check if all vector lie inside PKA RAM
                        PKA_CHECK_INT_ATMOST(Res1_wo + VASize[3] +
                                                 WorkAreaSize + ScratchPadSize,
                                             PkaRamSize);
                        IDENTIFIER_NOT_USED(ScratchPadSize);

                        // Check if Application Provided Buffers are sufficient enough
                        PKA_CHECK_INT_ATLEAST(VBufSize[0], VASize[0]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[1], VASize[1]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[2], VASize[2]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[3], VASize[3]);

                        // Number of Input Vectors
                        PKACtx_p->NrInputVectors = 3;
                        break;
                }
                case PKA_OPCODE_ECC_MULTIPLY_MONTGOMERY_SCAP: {
                        int ScratchPadSize;
                        PKA_CHECK_INT_INRANGE(B_Len, 2, 24);

                        if (Adapter_PKA_SetupVectors(4, PKACtx_p) != 0)
                                return PKA_ERROR_INVALID_PARAMETER;

                        WorkAreaSize =
                            (PKA_VectorWordLength_t)EIP28_GetWorkAreaSize(
                                ECC_MUL_MONT_SCAP, 0, 0,
                                B_Len); // In words
                        ScratchPadSize =
                            EIP28_GetScratchpadSize(ECC_MUL_MONT_SCAP);

                        // Vector Length
                        VASize[0] = VLEN3(B_Len);
                        VASize[1] = 2 * VLEN2A(B_Len) + VLEN2(B_Len);
                        VASize[2] = VLEN2A(B_Len);
                        VASize[3] = VLEN2A(B_Len);

                        // Word Offsets of each Vector
                        VWo[0] = A_wo = 0;
                        B_wo          = A_wo + VASize[0];
                        VWo[1] = B_wo = EIP28_Memory_IncAndAlignNext64(B_wo);
                        C_wo          = B_wo + VASize[1] + 1;
                        VWo[2] = C_wo = EIP28_Memory_IncAndAlignNext64(C_wo);
                        Res1_wo       = C_wo + VASize[2] + 1;
                        VWo[3]        = Res1_wo =
                            EIP28_Memory_IncAndAlignNext64(Res1_wo);

                        // Check if all vector lie inside PKA RAM
                        PKA_CHECK_INT_ATMOST(Res1_wo + VASize[3] +
                                                 WorkAreaSize + ScratchPadSize,
                                             PkaRamSize);
                        IDENTIFIER_NOT_USED(ScratchPadSize);

                        // Check if Application Provided Buffers are sufficient enough
                        PKA_CHECK_INT_ATLEAST(VBufSize[0], VASize[0]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[1], VASize[1]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[2], VASize[2]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[3], VASize[3]);

                        // Number of Input Vectors
                        PKACtx_p->NrInputVectors = 3;
                        break;
                }
                case PKA_OPCODE_DSA_SIGN_SCAP: {
                        int ScratchPadSize;
                        PKA_CHECK_INT_INRANGE(A_Len, 3,
                                              ADAPTER_PKA_VECTOR_MAX_WORDS);
                        PKA_CHECK_INT_INRANGE(B_Len, 3,
                                              ADAPTER_PKA_VECTOR_MAX_WORDS);

                        if (Adapter_PKA_SetupVectors(4, PKACtx_p) != 0)
                                return PKA_ERROR_INVALID_PARAMETER;

                        WorkAreaSize =
                            (PKA_VectorWordLength_t)EIP28_GetWorkAreaSize(
                                DSA_SIGN_SCAP, 0, A_Len,
                                B_Len); // In words
                        ScratchPadSize = EIP28_GetScratchpadSize(DSA_SIGN_SCAP);

                        // Vector Length
                        VASize[0] = VLEN(B_Len);
                        VASize[1] = 2 * VLEN2A(A_Len) + VLEN2(B_Len);
                        VASize[2] = VLEN(B_Len);
                        // Output vector D is largest, so use that.
                        VASize[3] = 2 * VLEN2A(B_Len);

                        // Word Offsets of each Vector
                        VWo[0] = A_wo = 0;
                        B_wo          = A_wo + VASize[0];
                        VWo[1] = B_wo = EIP28_Memory_IncAndAlignNext64(B_wo);
                        C_wo          = B_wo + VASize[1];
                        VWo[2] = C_wo = EIP28_Memory_IncAndAlignNext64(C_wo);
                        Res1_wo       = C_wo + VASize[2];
                        VWo[3]        = Res1_wo =
                            EIP28_Memory_IncAndAlignNext64(Res1_wo);

                        // Check if all vector lie inside PKA RAM
                        PKA_CHECK_INT_ATMOST(Res1_wo + VASize[3] +
                                                 WorkAreaSize + ScratchPadSize,
                                             PkaRamSize);
                        IDENTIFIER_NOT_USED(ScratchPadSize);

                        // Check if Application Provided Buffers are sufficient enough
                        PKA_CHECK_INT_ATLEAST(VBufSize[0], VASize[0]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[1], VASize[1]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[2], VASize[2]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[3], VASize[3]);

                        // Number of Input Vectors
                        PKACtx_p->NrInputVectors = 3;
                        break;
                }
                case PKA_OPCODE_ECDSA_SIGN_SCAP: {
                        int ScratchPadSize;
                        PKA_CHECK_INT_INRANGE(B_Len, 3,
                                              ADAPTER_PKA_VECTOR_MAX_WORDS);

                        if (Adapter_PKA_SetupVectors(4, PKACtx_p) != 0)
                                return PKA_ERROR_INVALID_PARAMETER;

                        WorkAreaSize =
                            (PKA_VectorWordLength_t)EIP28_GetWorkAreaSize(
                                ECCDSA_SIGN_SCAP, 0, 0,
                                B_Len); // In words
                        ScratchPadSize =
                            EIP28_GetScratchpadSize(ECCDSA_SIGN_SCAP);

                        // Vector Length
                        VASize[0] = VLEN(B_Len);
                        VASize[1] = 6 * VLEN2A(B_Len);
                        VASize[2] = VLEN(B_Len);
                        // Vector D output is largest, so use that one.
                        VASize[3] = 2 * VLEN2A(B_Len);

                        // Word Offsets of each Vector
                        VWo[0] = A_wo = 0;
                        B_wo          = A_wo + VASize[0];
                        VWo[1] = B_wo = EIP28_Memory_IncAndAlignNext64(B_wo);
                        C_wo          = B_wo + VASize[1];
                        VWo[2] = C_wo = EIP28_Memory_IncAndAlignNext64(C_wo);
                        Res1_wo       = C_wo + VASize[2];
                        VWo[3]        = Res1_wo =
                            EIP28_Memory_IncAndAlignNext64(Res1_wo);

                        // Check if all vector lie inside PKA RAM
                        PKA_CHECK_INT_ATMOST(Res1_wo + VASize[3] +
                                                 WorkAreaSize + ScratchPadSize,
                                             PkaRamSize);
                        IDENTIFIER_NOT_USED(ScratchPadSize);

                        // Check if Application Provided Buffers are sufficient enough
                        PKA_CHECK_INT_ATLEAST(VBufSize[0], VASize[0]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[1], VASize[1]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[2], VASize[2]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[3], VASize[3]);

                        // Number of Input Vectors
                        PKACtx_p->NrInputVectors = 3;

                        break;
                }
                case PKA_OPCODE_PREP_BLIND_MSG: {
                        int ScratchPadSize;
                        PKA_CHECK_INT_INRANGE(A_Len, 1,
                                              ADAPTER_PKA_VECTOR_MAX_WORDS);
                        PKA_CHECK_INT_INRANGE(B_Len, 1,
                                              ADAPTER_PKA_VECTOR_MAX_WORDS);

                        if (Adapter_PKA_SetupVectors(3, PKACtx_p) != 0)
                                return PKA_ERROR_INVALID_PARAMETER;

                        WorkAreaSize =
                            (PKA_VectorWordLength_t)EIP28_GetWorkAreaSize(
                                PREP_BLIND_MSG, 0, A_Len,
                                B_Len); // In words
                        ScratchPadSize =
                            EIP28_GetScratchpadSize(PREP_BLIND_MSG);

                        // Vector Length
                        VASize[0] = VLEN(A_Len);
                        VASize[1] = VLEN(B_Len);
                        VASize[2] = 2 * VLEN1A(B_Len);

                        // Word Offsets of each Vector
                        VWo[0] = A_wo = 0;
                        B_wo          = A_wo + VASize[0];
                        VWo[1] = B_wo = EIP28_Memory_IncAndAlignNext64(B_wo);
                        Res1_wo       = B_wo + VASize[1];
                        VWo[2]        = Res1_wo =
                            EIP28_Memory_IncAndAlignNext64(Res1_wo);

                        // Check if all vector lie inside PKA RAM
                        PKA_CHECK_INT_ATMOST(Res1_wo + VASize[2] +
                                                 WorkAreaSize + ScratchPadSize,
                                             PkaRamSize);
                        IDENTIFIER_NOT_USED(ScratchPadSize);

                        // Check if Application Provided Buffers are sufficient enough
                        PKA_CHECK_INT_ATLEAST(VBufSize[0], VASize[0]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[1], VASize[1]);
                        PKA_CHECK_INT_ATLEAST(VBufSize[2], VASize[2]);

                        // Number of Input Vectors
                        PKACtx_p->NrInputVectors = 2;

                        break;
                }
#endif // ADAPTER_PKA_SCAP_ENABLE
                default: {
                        LOG_INFO(LogInfo "Operation Not supported\n");
                        break;
                }
        }

#undef VWo
#undef VBufSize

        IDENTIFIER_NOT_USED(WorkAreaSize)
        LOG_FUNC_EXIT;
        return 0;
}

/*-----------------------------------------------------------------------------
 * Adapter_PKA_WriteBigUIntToDevice
 *
 * a. Offsets, length (in words) of vectors, result vector index
 * and Device all are present in PKA Context
 *
 * Return Value:
 *      true - when operation completes successfully
 *      false - when write to device failed. Caller should treat it as
 *              PKA_ERROR_INVALID_PARAMETER
 */
static bool
Adapter_PKA_WriteBigUIntToDevice(Adapter_PKA_Context_t *const PKACtx_p)
{
        unsigned int    IndexLoop;
        unsigned int    NrInputVectors;
        EIP28_BigUInt_t BigUInt;
        EIP28_Status_t  EIP28_Status = -1;

        NrInputVectors = PKACtx_p->NrInputVectors;

        // Input and Result have same Index
        if (PKACtx_p->Command_p->OpCode == PKA_OPCODE_MODEXP_CRT ||
            PKACtx_p->Command_p->OpCode == PKA_OPCODE_DSA_SIGN ||
            PKACtx_p->Command_p->OpCode == PKA_OPCODE_DSA_VERIFY ||
            PKACtx_p->Command_p->OpCode == PKA_OPCODE_ECDSA_SIGN ||
            PKACtx_p->Command_p->OpCode == PKA_OPCODE_ECDSA_VERIFY) {
                NrInputVectors += 1;
        }
#ifdef ADAPTER_PKA_SCAP_ENABLE
        // Input and Result have same Index
        if (PKACtx_p->Command_p->OpCode == PKA_OPCODE_MODEXP_CRT_SCAP ||
            PKACtx_p->Command_p->OpCode == PKA_OPCODE_DSA_SIGN_SCAP ||
            PKACtx_p->Command_p->OpCode == PKA_OPCODE_ECDSA_SIGN_SCAP) {
                NrInputVectors += 1;
        }
#endif
        LOG_FUNC_ENTER;
        for (IndexLoop = 0; IndexLoop < NrInputVectors; IndexLoop++) {
                BigUInt.StoreAsMSB = true;
                BigUInt.Bytes_p    = PKACtx_p->Vectors[IndexLoop];
                BigUInt.ByteCount  = PKACtx_p->VSize[IndexLoop] * 4; // Bytes
                EIP28_Status       = EIP28_Memory_PutBigUInt_CALLATOMIC(
                    (PKACtx_p->EIP28_IOArea_p),
                    PKACtx_p->VectorWordOffsets[IndexLoop],
                    PKACtx_p->VSize[IndexLoop], &BigUInt, NULL);
                if (EIP28_Status != EIP28_STATUS_OK) {
                        LOG_CRIT(LogErr "WriteBigUIntToDevice[%d] Failed\n",
                                 IndexLoop);
                        return false;
                }
        }

        LOG_FUNC_EXIT;
        return true;
}

/*----------------------------------------------------------------------------
 * Adapter_PKA_OpcodeMul
 *
 */
#ifndef ADAPTER_PKA_REMOVE_MULTIPLY
static int Adapter_PKA_OpcodeMul(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        EIP28_Status = EIP28_StartOp_Multiply_AmulB_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VSize[0],
            PKACtx_p->VSize[1], PKACtx_p->VectorWordOffsets[ResIndex]);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "Multiply Start failed.\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
#endif

/*----------------------------------------------------------------------------
 * Adapter_PKA_OpcodeAdd
 *
 */
#ifndef ADAPTER_PKA_REMOVE_ADD
static int Adapter_PKA_OpcodeAdd(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        EIP28_Status = EIP28_StartOp_Add_AplusB_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VSize[0],
            PKACtx_p->VSize[1], PKACtx_p->VectorWordOffsets[ResIndex]);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "Add Start failed.\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
#endif

/*----------------------------------------------------------------------------
 * Adapter_PKA_OpcodeSub
 *
 */
#ifndef ADAPTER_PKA_REMOVE_SUBTRACT
static int Adapter_PKA_OpcodeSub(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        EIP28_Status = EIP28_StartOp_Sub_AminB_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VSize[0],
            PKACtx_p->VSize[1], PKACtx_p->VectorWordOffsets[ResIndex]);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "Subtract Start failed.\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
#endif

/*----------------------------------------------------------------------------
 * Adapter_PKA_OpcodeSubAdd
 *
 */
#ifndef ADAPTER_PKA_REMOVE_SUBADD
static int Adapter_PKA_OpcodeSubAdd(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        EIP28_Status = EIP28_StartOp_SubAdd_AplusCminB_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VectorWordOffsets[2],
            PKACtx_p->VSize[0], PKACtx_p->VectorWordOffsets[ResIndex]);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "Subtract Start failed.\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
#endif

/*----------------------------------------------------------------------------
 * Adapter_PKA_OpcodeRightShift
 *
 */
#ifndef ADAPTER_PKA_REMOVE_SHIFTRIGHT
static int Adapter_PKA_OpcodeRightShift(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        EIP28_Status = EIP28_StartOp_ShiftRight_AshiftrightS_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VSize[0], PKACtx_p->Extra,
            PKACtx_p->VectorWordOffsets[ResIndex]);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "RightShift Start failed.\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
#endif

/*----------------------------------------------------------------------------
 * Adapter_PKA_OpcodeLeftShift
 *
 */
#ifndef ADAPTER_PKA_REMOVE_SHIFTLEFT
static int Adapter_PKA_OpcodeLeftShift(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        EIP28_Status = EIP28_StartOp_ShiftLeft_AshiftleftS_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VSize[0], PKACtx_p->Extra,
            PKACtx_p->VectorWordOffsets[ResIndex]);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "LeftShift Start failed.\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
#endif

/*----------------------------------------------------------------------------
 * Adapter_PKA_OpcodeDivide
 *
 */
#ifndef ADAPTER_PKA_REMOVE_DIVIDE
static int Adapter_PKA_OpcodeDivide(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        EIP28_Status = EIP28_StartOp_Divide_AdivB_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VSize[0],
            PKACtx_p->VSize[1], PKACtx_p->VectorWordOffsets[ResIndex],
            PKACtx_p->VectorWordOffsets[ResIndex + 1]);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "Divide Start failed.\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
#endif

/*----------------------------------------------------------------------------
 * Adapter_PKA_OpcodeMod
 *
 */
#ifndef ADAPTER_PKA_REMOVE_MOD
static int Adapter_PKA_OpcodeMod(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        EIP28_Status = EIP28_StartOp_Modulo_AmodB_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VSize[0],
            PKACtx_p->VSize[1], PKACtx_p->VectorWordOffsets[ResIndex]);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "Modulo Start failed.\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
#endif

#ifndef ADAPTER_PKA_REMOVE_COMPARE
/*-----------------------------------------------------------------------------
 * Adapter_PKA_OpcodeCompare
 *
 */
static int Adapter_PKA_OpcodeCompare(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;

        LOG_FUNC_ENTER;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        EIP28_Status = EIP28_StartOp_Compare_AcmpB_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VSize[0]);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "Compare Start failed.\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
#endif

/*----------------------------------------------------------------------------
 * Adapter_PKA_OpcodeCopy
 *
 */
#ifndef ADAPTER_PKA_REMOVE_COPY
static int Adapter_PKA_OpcodeCopy(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        EIP28_Status = EIP28_StartOp_Copy_Adup_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VSize[0], PKACtx_p->VectorWordOffsets[ResIndex]);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "Copy Start failed.\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
#endif

/*----------------------------------------------------------------------------
 * Adapter_PKA_OpcodeModExp
 *
 */
#ifndef ADAPTER_PKA_REMOVE_MODEXP
static int Adapter_PKA_OpcodeModExp(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;
        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;
        EIP28_Status = EIP28_StartOp_ModExp_CupAmodB_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VectorWordOffsets[2],
            PKACtx_p->VSize[0], PKACtx_p->VSize[1], PKACtx_p->Extra,
            PKACtx_p->VectorWordOffsets[ResIndex]);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "ModExp Start failed.\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
#endif

/*----------------------------------------------------------------------------
 * Adapter_PKA_OpcodeModExpCRT
 *
 * This function expects that the reference provided by the User have
 * sub-vectors in same sequence and with proper mandatory buffers as
 * EIP-28 requires. (Coz this is documented in the Opcode Specification
 * - pka_opcodes_eip28.h file in PKA_API\incl\)
 *
 * As sub-vectors comes integrated so it must be broken for strict argument
 * checking.
 */
#ifndef ADAPTER_PKA_REMOVE_MODEXPCRT
static int Adapter_PKA_OpcodeModExpCRT(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        EIP28_Status = EIP28_StartOp_ModExp_CRT_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VectorWordOffsets[2],
            PKACtx_p->VectorWordOffsets[ResIndex], //Input/Output Wo
            PKACtx_p->Extra, PKACtx_p->A_Len, PKACtx_p->B_Len);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "ModExp-CRT Start failed.\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
#endif

/*----------------------------------------------------------------------------
 * Adapter_PKA_OpcodeModInv
 *
 */
#ifndef ADAPTER_PKA_REMOVE_MODINV
static int Adapter_PKA_OpcodeModInv(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        EIP28_Status = EIP28_StartOp_ModInv_invAmodB_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VSize[0],
            PKACtx_p->VSize[1], PKACtx_p->VectorWordOffsets[ResIndex]);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "ModInv Start failed.\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
#endif

/*----------------------------------------------------------------------------
 * Adapter_PKA_OpcodeECCAddAffine
 *
 */
#ifndef ADAPTER_PKA_REMOVE_ECCADD
static int Adapter_PKA_OpcodeECCAddAffine(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        //Start ECC-ADD operation
        EIP28_Status = EIP28_StartOp_EccAdd_Affine_AplusC_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VectorWordOffsets[2],
            PKACtx_p->B_Len, PKACtx_p->VectorWordOffsets[ResIndex]);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "ECC-Add_Affine Start failed.\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
#endif

/*----------------------------------------------------------------------------
 * Adapter_PKA_OpcodeECCAddProjective
 *
 */
#ifndef ADAPTER_PKA_REMOVE_ECCADD
static int
Adapter_PKA_OpcodeECCAddProjective(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        //Start ECC-ADD operation
        EIP28_Status = EIP28_StartOp_EccAdd_Projective_AplusC_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VectorWordOffsets[2],
            PKACtx_p->B_Len, PKACtx_p->VectorWordOffsets[ResIndex]);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "ECC-Add_Projective Start failed.\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
#endif

/*----------------------------------------------------------------------------
 * Adapter_PKA_OpcodeECCMulAffine
 *
 */
#ifndef ADAPTER_PKA_REMOVE_ECCMUL
static int Adapter_PKA_OpcodeECCMulAffine(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        //Start ECC-ADD operation
        EIP28_Status = EIP28_StartOp_EccMul_Affine_kmulC_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VectorWordOffsets[2],
            PKACtx_p->A_Len, PKACtx_p->B_Len,
            PKACtx_p->VectorWordOffsets[ResIndex]);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "ECC-Mul_Affine Start failed\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
#endif

/*----------------------------------------------------------------------------
 * Adapter_PKA_OpcodeECCMulProjective
 *
 */
#ifndef ADAPTER_PKA_REMOVE_ECCMUL
static int
Adapter_PKA_OpcodeECCMulProjective(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        //Start ECC-ADD operation
        EIP28_Status = EIP28_StartOp_EccMul_Projective_kmulC_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VectorWordOffsets[2],
            PKACtx_p->A_Len, PKACtx_p->B_Len,
            PKACtx_p->VectorWordOffsets[ResIndex]);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "ECC-Mul_Projective Start failed\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
#endif

/*----------------------------------------------------------------------------
 * Adapter_PKA_OpcodeECCMulMontgomery
 *
 */
#ifndef ADAPTER_PKA_REMOVE_ECCMULMONT
static int
Adapter_PKA_OpcodeECCMulMontgomery(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        //Start ECC-Mul_Montgomery operation
        EIP28_Status = EIP28_StartOp_EccMul_Montgomery_kmulC_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VectorWordOffsets[2],
            PKACtx_p->A_Len, PKACtx_p->B_Len,
            PKACtx_p->VectorWordOffsets[ResIndex]);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "ECC-Mul_Montogomery Start failed\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
#endif

/*----------------------------------------------------------------------------
 * Adapter_PKA_OpcodeDSASign
 *
 */
#ifndef ADAPTER_PKA_REMOVE_DSASIGN
static int Adapter_PKA_OpcodeDSASign(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        //Start DSA_Sign operation
        EIP28_Status = EIP28_StartOp_DSA_Sign_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VectorWordOffsets[2],
            PKACtx_p->VectorWordOffsets[ResIndex], //Input/Output Wo
            PKACtx_p->A_Len, PKACtx_p->B_Len, PKACtx_p->Extra);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "DSA Sign Start failed\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
#endif

/*----------------------------------------------------------------------------
 * Adapter_PKA_OpcodeDSAVerify
 *
 */
#ifndef ADAPTER_PKA_REMOVE_DSAVERIFY
static int Adapter_PKA_OpcodeDSAVerify(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        //Start DSA_Verify operation
        EIP28_Status = EIP28_StartOp_DSA_Verify_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VectorWordOffsets[2],
            PKACtx_p->VectorWordOffsets[ResIndex], PKACtx_p->A_Len,
            PKACtx_p->B_Len, PKACtx_p->Extra);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "DSA Verify Start failed\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
#endif

/*----------------------------------------------------------------------------
 * Adapter_PKA_OpcodeECCDSASign
 *
 */
#ifndef ADAPTER_PKA_REMOVE_ECCDSASIGN
static int Adapter_PKA_OpcodeECCDSASign(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        //Start ECC DSA_Sign operation
        EIP28_Status = EIP28_StartOp_ECCDSA_Sign_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VectorWordOffsets[2],
            PKACtx_p->VectorWordOffsets[ResIndex], PKACtx_p->A_Len,
            PKACtx_p->B_Len);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "ECC DSA Sign Start failed\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
#endif

/*----------------------------------------------------------------------------
 * Adapter_PKA_OpcodeECCDSAVerify
 *
 */
#ifndef ADAPTER_PKA_REMOVE_ECCDSAVERIFY
static int Adapter_PKA_OpcodeECCDSAVerify(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        //Start ECC-DSA_Verify operation
        EIP28_Status = EIP28_StartOp_ECCDSA_Verify_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VectorWordOffsets[2],
            PKACtx_p->VectorWordOffsets[ResIndex], PKACtx_p->A_Len,
            PKACtx_p->B_Len);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "ECC DSA Verify Start failed\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
#endif

#ifdef ADAPTER_PKA_SCAP_ENABLE
static int Adapter_PKA_OpcodeModExpScap(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        //Start ModExp Scap operation
        EIP28_Status = EIP28_StartOp_ModExpScap_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VectorWordOffsets[2],
            PKACtx_p->VectorWordOffsets[ResIndex], PKACtx_p->B_Len,
            PKACtx_p->Extra);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "ModExp SCAP Start failed\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}

static int
Adapter_PKA_OpcodeModExpCrtScap(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        //Start ModExpCrt Scap operation
        EIP28_Status = EIP28_StartOp_ModExpCrtScap_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VectorWordOffsets[2],
            PKACtx_p->VectorWordOffsets[ResIndex], PKACtx_p->A_Len,
            PKACtx_p->B_Len);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "ModExpCrt Scap Start failed\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
static int
Adapter_PKA_OpcodeECCMulProjectiveScap(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        //Start ECC MulProjective Scap operation
        EIP28_Status = EIP28_StartOp_ECCMulProjScap_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VectorWordOffsets[2],
            PKACtx_p->VectorWordOffsets[ResIndex], PKACtx_p->B_Len);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "ECC MulProjective Scap failed\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
static int
Adapter_PKA_OpcodeECCMulMontgomeryScap(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        //Start ECCMulMontgomeryScap operation
        EIP28_Status = EIP28_StartOp_ECCMulMontScap_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VectorWordOffsets[2],
            PKACtx_p->VectorWordOffsets[ResIndex], PKACtx_p->B_Len);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "ECCMulMontgomeryScap Start failed\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
static int Adapter_PKA_OpcodeDSASignScap(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        //Start DSASignScapoperation
        EIP28_Status = EIP28_StartOp_DSASignScap_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VectorWordOffsets[2],
            PKACtx_p->VectorWordOffsets[ResIndex], PKACtx_p->A_Len,
            PKACtx_p->B_Len);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "DSASignScap Start failed\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
static int
Adapter_PKA_OpcodeECCDSASignScap(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        //Start ECCDSASignScap operation
        EIP28_Status = EIP28_StartOp_ECCDSASignScap_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1], PKACtx_p->VectorWordOffsets[2],
            PKACtx_p->VectorWordOffsets[ResIndex], PKACtx_p->B_Len);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "ECCDSASignScap Start failed\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
static int
Adapter_PKA_OpcodePrepBlindMsgScap(Adapter_PKA_Context_t *const PKACtx_p)
{
        EIP28_Status_t EIP28_Status = -1;
        unsigned int   ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;

        if (!Adapter_PKA_WriteBigUIntToDevice(PKACtx_p))
                return PKA_ERROR_INVALID_PARAMETER;

        //Start PrepBlindMsgScap operation
        EIP28_Status = EIP28_StartOp_PrepBlindMsgScap_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[0],
            PKACtx_p->VectorWordOffsets[1],
            PKACtx_p->VectorWordOffsets[ResIndex], PKACtx_p->A_Len,
            PKACtx_p->B_Len);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "PrepBlindMsgScap Start failed\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        LOG_FUNC_EXIT;
        // Return
        return PKA_RESULT_DONE;
}
#endif

/*-----------------------------------------------------------------------------
 * Adapter_PKA_ReadResultInBuffer
 *
 * ReadResult Word Count and Copy result in PKA RAM to respective buffer
 *
 */
static bool Adapter_PKA_ReadResultInBuffer(Adapter_PKA_Context_t *PKACtx_p,
                                           unsigned int           ResIndex)
{
        EIP28_Status_t  EIP28_Status = -1;
        EIP28_BigUInt_t EIP28_BigUInt;
        uint32_t        Result_Len;

        LOG_FUNC_ENTER;
        Result_Len = PKACtx_p->VSize[ResIndex];
        EIP28_BigUInt.StoreAsMSB = true;
        EIP28_BigUInt.ByteCount  = Result_Len * 4;
        EIP28_BigUInt.Bytes_p    = PKACtx_p->Vectors[ResIndex];

        // Zero out the memory of Result Buffer
        memset(EIP28_BigUInt.Bytes_p, 0, Result_Len);

        EIP28_Status = EIP28_Memory_GetBigUInt_CALLATOMIC(
            (PKACtx_p->EIP28_IOArea_p), PKACtx_p->VectorWordOffsets[ResIndex],
            Result_Len, &EIP28_BigUInt);
        if (EIP28_Status != EIP28_STATUS_OK) {
                LOG_CRIT(LogErr "Result Read failed\n");
                return false;
        }

        LOG_FUNC_EXIT;
        return true;
}

#ifndef ADAPTER_PKA_REMOVE_COMPARE
/*-----------------------------------------------------------------------------
 * Adapter_PKA_ReadResultCompare
 *
 * Read the result after compare from PKA_Compare register
 *
 */
static uint8_t Adapter_PKA_ReadResultCompare(Adapter_PKA_Context_t *PKACtx_p)
{
        EIP28_CompareResult_t CmpRes = 0;

        LOG_FUNC_ENTER;

        EIP28_ReadResult_Compare((PKACtx_p->EIP28_IOArea_p), &CmpRes);

        LOG_FUNC_EXIT;

        return (uint8_t)CmpRes;
}
#endif

/*-----------------------------------------------------------------------------
 * Adapter_PKA_ResultGetforOperation
 *
 * Update the Result in the PKA_Result_t for actual outcome of the operation
 *
 */
static int Adapter_PKA_ResultGetforOperation(PKA_Result_t *         Result_p,
                                             Adapter_PKA_Context_t *PKACtx_p)
{
        PKA_Opcodes_EIP28_t Op;
#if !defined(ADAPTER_PKA_REMOVE_ECCADD) || !defined(ADAPTER_PKA_REMOVE_ECCMUL)
        bool Result_IsUndefined = false, Result_IsAtInfinity = false;
#endif
        unsigned int Result_Len, Quotient_Len, Remainder_Len;
        unsigned int ResIndex;

        LOG_FUNC_ENTER;
        ResIndex = PKACtx_p->NrInputVectors;
        Op       = PKACtx_p->Command_p->OpCode;

        switch (Op) {
                case PKA_OPCODE_MULTIPLY:
                case PKA_OPCODE_ADD:
                case PKA_OPCODE_SUBTRACT:
                case PKA_OPCODE_ADD_SUBTRACT:
                case PKA_OPCODE_RIGHT_SHIFT:
                case PKA_OPCODE_LEFT_SHIFT:
                case PKA_OPCODE_COPY:
                case PKA_OPCODE_MODEXP:
                case PKA_OPCODE_MODEXP_CRT:
#ifndef PKA_ADAPTER_REMOVE_SCAP
                case PKA_OPCODE_MODEXP_SCAP:
                case PKA_OPCODE_MODEXP_CRT_SCAP:
#endif
                {
                        // Read the Actual result length
                        EIP28_ReadResult_WordCount_CALLATOMIC(
                            (PKACtx_p->EIP28_IOArea_p), &Result_Len);

                        if (!Adapter_PKA_ReadResultInBuffer(PKACtx_p, ResIndex))
                                return PKA_ERROR_INVALID_PARAMETER;

                        // Read Extra result information if device provides
                        Result_p->Result  = PKA_RESULT_DONE;
                        Result_p->fIsZero = false;
                        Result_p->MSW     = (uint16_t)Result_Len - 1;
                        break;
                }
                case PKA_OPCODE_DIVIDE: {
                        // Read the Remainder Word Count
                        EIP28_ReadResult_RemainderWordCount_CALLATOMIC(
                            (PKACtx_p->EIP28_IOArea_p), &Remainder_Len);

                        if (!Adapter_PKA_ReadResultInBuffer(PKACtx_p, ResIndex))
                                return PKA_ERROR_INVALID_PARAMETER;

                        // Read the result and result length from PKA RAM
                        EIP28_ReadResult_WordCount_CALLATOMIC(
                            (PKACtx_p->EIP28_IOArea_p), &Quotient_Len);

                        if (!Adapter_PKA_ReadResultInBuffer(PKACtx_p,
                                                            ResIndex + 1))
                                return PKA_ERROR_INVALID_PARAMETER;

                        // Read Extra result information if device provides
                        Result_p->Result   = PKA_RESULT_DONE;
                        Result_p->fIsZero  = false;
                        Result_p->fIsZero2 = false;
                        Result_p->MSW      = (uint16_t)Quotient_Len - 1;
                        Result_p->MSW2     = (uint16_t)Remainder_Len - 1;

                        break;
                }
#ifndef ADAPTER_PKA_REMOVE_COMPARE
                case PKA_OPCODE_COMPARE: {
                        // Read result information
                        Result_p->Result = PKA_RESULT_DONE;
                        // Compare result
                        // For specific commands only, if not supported this value is 0 otherwise:
                        // If 'A' == 'B', compare result = 0b001
                        // Else if 'A' < 'B', compare result = 0b010
                        // Else (if 'A' > 'B'), compare result = 0b100
                        Result_p->CMPResult =
                            Adapter_PKA_ReadResultCompare(PKACtx_p);
                        break;
                }
#endif
#ifdef PKA_ADAPTER_REMOVE_MOD
                case PKA_OPCODE_MODULO: {
                        // Read the Remainder Word Count
                        EIP28_ReadResult_RemainderWordCount_CALLATOMIC(
                            (PKACtx_p->EIP28_IOArea_p), &Remainder_Len);

                        if (!Adapter_PKA_ReadResultInBuffer(PKACtx_p, ResIndex))
                                return PKA_ERROR_INVALID_PARAMETER;

                        // Read the result and result length from PKA RAM
                        EIP28_ReadResult_WordCount_CALLATOMIC(
                            (PKACtx_p->EIP28_IOArea_p), &Quotient_Len);

                        // if (!Adapter_PKA_ReadResultInBuffer(PKACtx_p, ResIndex + 1))
                        //    return PKA_ERROR_INVALID_PARAMETER;

                        // Read Extra result information if device provides
                        Result_p->Result   = PKA_RESULT_DONE;
                        Result_p->fIsZero  = false;
                        Result_p->fIsZero2 = false;
                        Result_p->MSW      = (uint16_t)Remainder_Len - 1;
                        Result_p->MSW2     = (uint16_t)Quotient_Len - 1;

                        break;
                }
#else
                case PKA_OPCODE_MODULO: {
                        // Read the Remainder Word Count
                        EIP28_ReadResult_RemainderWordCount_CALLATOMIC(
                            (PKACtx_p->EIP28_IOArea_p), &Result_Len);

                        if (!Adapter_PKA_ReadResultInBuffer(PKACtx_p, ResIndex))
                                return PKA_ERROR_INVALID_PARAMETER;

                        // Read Extra result information if device provides
                        Result_p->Result  = PKA_RESULT_DONE;
                        Result_p->fIsZero = false;
                        Result_p->MSW     = (uint16_t)Result_Len - 1;
                        break;
                }
#endif
#ifndef ADAPTER_PKA_REMOVE_MODINV
                case PKA_OPCODE_MODINV: {
                        // Read the result and result length from PKA RAM
                        EIP28_ReadResult_WordCount_CALLATOMIC(
                            (PKACtx_p->EIP28_IOArea_p), &Result_Len);

                        if (!Adapter_PKA_ReadResultInBuffer(PKACtx_p, ResIndex))
                                return PKA_ERROR_INVALID_PARAMETER;

                        EIP28_ReadResult_ModInv_Status(
                            (PKACtx_p->EIP28_IOArea_p), &Result_IsUndefined);
                        if (Result_IsUndefined) {
                                Result_p->Result  = PKA_RESULT_ERR_NO_INV;
                                Result_p->fIsZero = true;
                        } else {
                                Result_p->Result  = PKA_RESULT_DONE;
                                Result_p->fIsZero = false;
                                Result_p->MSW     = (uint16_t)Result_Len - 1;
                        }

                        break;
                }
#endif
#ifndef PKA_ADAPTER_REMOVE_SCAP
                case PKA_OPCODE_ECC_MULTIPLY_PROJECTIVE_SCAP:
                case PKA_OPCODE_ECC_MULTIPLY_MONTGOMERY_SCAP:
#endif // PKA_ADAPTER_REMOVE_SCAP

#if !defined(ADAPTER_PKA_REMOVE_ECCADD) || !defined(ADAPTER_PKA_REMOVE_ECCMUL)
                case PKA_OPCODE_ECC_ADD_AFFINE:
                case PKA_OPCODE_ECC_ADD_PROJECTIVE:
                case PKA_OPCODE_ECC_MULTIPLY_AFFINE:
                case PKA_OPCODE_ECC_MULTIPLY_PROJECTIVE:
                case PKA_OPCODE_ECC_MULTIPLY_MONTGOMERY: {

                        // Get the result length from PKA RAM
                        Result_Len = PKACtx_p->B_Len;

                        if (!Adapter_PKA_ReadResultInBuffer(PKACtx_p, ResIndex))
                                return PKA_ERROR_INVALID_PARAMETER;

                        // Get More result info specific to this operation
                        EIP28_ReadResult_ECCPoint_Status(
                            (PKACtx_p->EIP28_IOArea_p), &Result_IsAtInfinity,
                            &Result_IsUndefined);
                        Result_p->fIsZero = true;
                        if (Result_IsUndefined)
                                Result_p->Result = PKA_RESULT_ERR_NO_INV;
                        else if (Result_IsAtInfinity)
                                Result_p->Result = PKA_RESULT_ERR_AT_INFINITY;
                        else {
                                Result_p->Result  = PKA_RESULT_DONE;
                                Result_p->fIsZero = false;
                                Result_p->MSW     = (uint16_t)Result_Len - 1;
                        }
                        break;
                }
#endif
#ifndef PKA_ADAPTER_REMOVE_SCAP
                case PKA_OPCODE_DSA_SIGN_SCAP:
                case PKA_OPCODE_ECDSA_SIGN_SCAP:
#endif // PKA_ADAPTER_REMOVE_SCAP
#if !defined(ADAPTER_PKA_REMOVE_DSASIGN) ||                                    \
    !defined(ADAPTER_PKA_REMOVE_ECCDSASIGN)
                case PKA_OPCODE_DSA_SIGN:
                case PKA_OPCODE_DSA_VERIFY:
                case PKA_OPCODE_ECDSA_SIGN:
                case PKA_OPCODE_ECDSA_VERIFY: {
                        // Get the result length from PKA RAM
                        Result_Len = 2 * PKACtx_p->B_Len;

                        if (!Adapter_PKA_ReadResultInBuffer(PKACtx_p, ResIndex))
                                return PKA_ERROR_INVALID_PARAMETER;

                        // Get More result info specific to this operation
                        EIP28_ReadResult_ECCPoint_Status(
                            (PKACtx_p->EIP28_IOArea_p), &Result_IsAtInfinity,
                            &Result_IsUndefined);
                        Result_p->fIsZero = true;
                        if (Result_IsUndefined)
                                Result_p->Result = PKA_RESULT_ERR_NO_INV;
                        else if (Result_IsAtInfinity)
                                Result_p->Result = PKA_RESULT_ERR_AT_INFINITY;
                        else {
                                Result_p->Result  = PKA_RESULT_DONE;
                                Result_p->fIsZero = false;
                                Result_p->MSW     = (uint16_t)Result_Len - 1;
                        }
                        break;
                }
#endif
                case PKA_OPCODE_PREP_BLIND_MSG: {
                        // Get the result length from PKA RAM
                        Result_Len =
                            VLEN1A(PKACtx_p->B_Len) + VLEN(PKACtx_p->B_Len);

                        if (!Adapter_PKA_ReadResultInBuffer(PKACtx_p, ResIndex))
                                return PKA_ERROR_INVALID_PARAMETER;
                        Result_p->Result = PKA_RESULT_DONE;
                        Result_p->MSW    = (uint16_t)Result_Len - 1;
                        break;
                }
                default: {
                        LOG_INFO(LogInfo "Operation Not supported\n");
                }
        }

        LOG_FUNC_EXIT;
        return PKA_RESULT_DONE;
}

/****************************************************************************
 *                   PKA Interrupt Handling Functions' Definition           *
 ****************************************************************************/

#if defined(ADAPTER_EIP28_INTERRUPT_ENABLE)
/*---------------------------------------------------------------------------
 * Adapter_PKA_InterruptHandler
 *
 * This function is registered for EIP-28 interrupt handling
 */
static void Adapter_PKA_InterruptHandler(const int nIRQ)
{
        PKA_ResultNotifyFunction_t CB;
        LOG_FUNC_ENTER;
        IDENTIFIER_NOT_USED(nIRQ);

        if (PKACtx.PKANotify.ResultNotifyCB == NULL) {
                LOG_INFO(LogInfo "Callback not registered\n");
                return;
        }
        CB                              = PKACtx.PKANotify.ResultNotifyCB;
        PKACtx.PKANotify.ResultNotifyCB = NULL;
        CB(PKACtx.Session, 1);
        LOG_FUNC_EXIT;
}
#endif

/****************************************************************************
 *                   PKA Public Interface Functions' Definition             *
 ****************************************************************************/

/*----------------------------------------------------------------------------
 * PKA_Init
 *
 */
int PKA_Init(const unsigned int InstanceNumber)
{
        IDENTIFIER_NOT_USED(InstanceNumber)

        LOG_FUNC_ENTER;

        if (!Adapter_EIP28_Init()) {
                LOG_CRIT(LogErr "PKA_Init: Adapter_EIP28_Init failed\n");
                return PKA_ERROR_INTERNAL;
        }

#ifdef ADAPTER_PKA_SCAP_ENABLE
        EIP28_SetSCAP_DummyCorr_CALLATOMIC(&Adapter_EIP28_IOArea, true);
#endif

#ifdef ADAPTER_EIP28_INTERRUPT_ENABLE
        // enable the data-available interrupt
        Adapter_Interrupt_SetHandler(IRQ_EIP28_READY,
                                     Adapter_PKA_InterruptHandler);
#endif /* ADAPTER_EIP28_INTERRUPTS_ENABLE */

        LOG_FUNC_EXIT;
        return 0;
}

/*----------------------------------------------------------------------------
 * PKA_UnInit
 *
 */
int PKA_UnInit(const unsigned int InstanceNumber)
{
        IDENTIFIER_NOT_USED(InstanceNumber)
        LOG_FUNC_ENTER;

#ifdef ADAPTER_PKA_SCAP_ENABLE
        EIP28_SetSCAP_DummyCorr_CALLATOMIC(&Adapter_EIP28_IOArea, false);
#endif

        Adapter_EIP28_UnInit();

        LOG_FUNC_EXIT;
        return 0;
}

/*---------------------------------------------------------------------------
 * PKA_Open
 */
int PKA_Open(PKA_Session_t *const Session_p, const unsigned int InstanceNumber)
{
        IDENTIFIER_NOT_USED(InstanceNumber);
        LOG_FUNC_ENTER;

        memset(&PKACtx, 0, sizeof(Adapter_PKA_Context_t));

        // Assign Unique ID
        Session_p->p             = (void *)28; //(void *)current->pid;
        PKACtx.Session.p         = Session_p->p;
        PKACtx.IsInitialized     = Adapter_EIP28_IsUsable;
        PKACtx.CommandPutSuccess = 0;
        PKACtx.EIP28_IOArea_p    = &Adapter_EIP28_IOArea;
        LOG_INFO(LogInfo "Device Up\n");

#ifdef ADAPTER_EIP28_INTERRUPT_ENABLE
        Adapter_Interrupt_Enable(IRQ_EIP28_READY, 0);
#endif /* ADAPTER_EIP28_INTERRUPTS_ENABLE */

        LOG_FUNC_EXIT;
        return 0;
}

/*---------------------------------------------------------------------------
 * PKA_Close
 *
 */
int PKA_Close(PKA_Session_t const Session_p)
{
        IDENTIFIER_NOT_USED(Session_p.p);
        LOG_FUNC_ENTER;

#ifdef ADAPTER_EIP28_INTERRUPT_ENABLE
        // Disable hardware interrupt
        Adapter_Interrupt_Disable(IRQ_EIP28_READY, 0);
#endif

        LOG_FUNC_EXIT;
        return 0;
}

/*---------------------------------------------------------------------------
 * PKA_Capabilities_Get
 *
 */
int PKA_Capabilities_Get(PKA_Capabilities_t *const Capabilities_p)
{
        unsigned int         i;
        EIP28_RevisionInfo_t RevInfo;

        LOG_FUNC_ENTER;
        strncpy(Capabilities_p->szTextDescription, ADAPTER_PKA_CAPABILITIES,
                PKA_MAXLEN_TEXT);

        Capabilities_p->OpCodesCount   = ADAPTER_PKA_CAP_NR;
        Capabilities_p->Vector_MaxBits = ADAPTER_PKA_VECTOR_MAX_WORDS * 32;
        for (i = 1; i <= Capabilities_p->OpCodesCount; i++)
                Capabilities_p->OpCodes[i] = i;

        if (EIP28_STATUS_OK ==
            EIP28_GetRevisionInfo(&Adapter_EIP28_IOArea, &RevInfo)) {
                Capabilities_p->FirmwareCapabilities = RevInfo.FW.Capabilities;
                Capabilities_p->LNME_Config = (RevInfo.Options & 0x1C) >> 2;
                // Check to see if this is a 'E' config
                if ((RevInfo.FW.Capabilities == 5) ||
                    (RevInfo.FW.Capabilities == 9)) {
                        strncat(Capabilities_p->szTextDescription,
                                ADAPTER_PKA_CAPABILITIES_E,
                                PKA_MAXLEN_TEXT -
                                    strlen(Capabilities_p->szTextDescription));
                        Capabilities_p->OpCodesCount =
                            (ADAPTER_PKA_CAP_NR + ADAPTER_PKA_CAP_E_NR);
                        for (i = 0; i < ADAPTER_PKA_CAP_E_NR; i++)
                                Capabilities_p
                                    ->OpCodes[PKA_OPCODE_DSA_SIGN + i] =
                                    PKA_OPCODE_DSA_SIGN + i;
                }
                // Check to see if this is a 'S' config
                if ((RevInfo.FW.Capabilities == 8) ||
                    (RevInfo.FW.Capabilities == 9)) {
                        strncat(Capabilities_p->szTextDescription,
                                ADAPTER_PKA_CAPABILITIES_S,
                                PKA_MAXLEN_TEXT -
                                    strlen(Capabilities_p->szTextDescription));
                        // Report highest number
                        Capabilities_p->OpCodesCount =
                            (ADAPTER_PKA_CAP_NR + ADAPTER_PKA_CAP_E_NR +
                             ADAPTER_PKA_CAP_S_NR);
                        for (i = 0; i < ADAPTER_PKA_CAP_S_NR; i++) {
                                Capabilities_p
                                    ->OpCodes[PKA_OPCODE_MODEXP_SCAP + i] =
                                    PKA_OPCODE_MODEXP_SCAP + i;
                        }
                }
                // Check to see if this is a 'ES' config
                if (RevInfo.FW.Capabilities == 9) {
                        strncat(Capabilities_p->szTextDescription,
                                ADAPTER_PKA_CAPABILITIES_ES,
                                PKA_MAXLEN_TEXT -
                                    strlen(Capabilities_p->szTextDescription));
                        // Report highest number
                        Capabilities_p->OpCodesCount =
                            (ADAPTER_PKA_CAP_NR + ADAPTER_PKA_CAP_E_NR +
                             ADAPTER_PKA_CAP_S_NR + ADAPTER_PKA_CAP_ES_NR);
                        for (i = 0; i < ADAPTER_PKA_CAP_ES_NR; i++)
                                Capabilities_p
                                    ->OpCodes[PKA_OPCODE_DSA_SIGN_SCAP + i] =
                                    PKA_OPCODE_DSA_SIGN_SCAP + i;
                }
        } else {
                LOG_CRIT(LogErr "Can't get Revision Information!\n");
                return PKA_ERROR_INTERNAL;
        }

        LOG_FUNC_EXIT;
        return 0;
}

/*---------------------------------------------------------------------------
 * PKA_CommandPut
 */
int PKA_CommandPut(const PKA_Session_t        Session,
                   const PKA_Command_t *const Command_p)
{
        int                rv          = 0; // Return Value
        unsigned int       Opr         = 0; // Operation to be performed
        bool               Parity_Err  = false;
        bool               Hamming_Err = false;
        PKA_Capabilities_t Capabilities;

        LOG_FUNC_ENTER;

        // Init structure.
        memset(&Capabilities, 0, sizeof(PKA_Capabilities_t));

        // LOG_CRIT("%s, %d, 0x%x\n", __FUNCTION__, __LINE__, 0);
        if (true == EIP28_CheckReset_SeqCtrl(PKACtx.EIP28_IOArea_p, &Parity_Err,
                                             &Hamming_Err)) {
                LOG_CRIT("Reset bit was set in the Sequence Control register!\n"
                         "\tParity error? %s\n"
                         "\tHamming error? %s\n",
                         Parity_Err ? "yes" : "no", Hamming_Err ? "yes" : "no");

                return PKA_ERROR_RESET;
        }

        if (PKA_Capabilities_Get(&Capabilities)) {
                LOG_CRIT("Can't get capabilities info!\n");
                return PKA_ERROR_INTERNAL;
        }
        Opr = (unsigned int)Command_p->OpCode;

        // LOG_CRIT("%s, %d, 0x%x\n", __FUNCTION__, __LINE__, 0);
        // Check Opcode, Session, Command_p, callers validity (if lock is granted)
        if (!Capabilities.OpCodes[Opr]) {
                LOG_CRIT(LogInfo "Opcode not implemented\n");
                return PKA_ERROR_NOT_IMPLEMENTED;
        }
        // LOG_CRIT("%s, %d, 0x%x\n", __FUNCTION__, __LINE__, 0);
        if (Session.p != PKACtx.Session.p) {
                LOG_CRIT(LogInfo "Session Invalid \n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        // LOG_CRIT("%s, %d, 0x%x\n", __FUNCTION__, __LINE__, 0);
        PKACtx.Command_p = Command_p;

        rv = Adapter_PKA_Validate_CalculateWordOffsetAndVectorLenght(&PKACtx);
        if (rv < 0) {
                LOG_CRIT(LogErr "Vector Validation Failed \n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        switch (Opr) {
#ifndef ADAPTER_PKA_REMOVE_MODEXP
                case PKA_OPCODE_MODEXP:
                        rv = Adapter_PKA_OpcodeModExp(&PKACtx);
                        break;
#endif
                default:
                        rv = PKA_ERROR_INVALID_OPCODE;
                        break;
        }

        // Command put success
        if (rv == PKA_RESULT_DONE)
                PKACtx.CommandPutSuccess = 1;

        LOG_FUNC_EXIT;
        return rv;
}

/*---------------------------------------------------------------------------
 * PKA_CommandPutMany
 *
 * Not supported in this implementations.
 */
int PKA_CommandPutMany(const PKA_Session_t Session, const bool fInOrder,
                       const unsigned int   CommandCount,
                       const PKA_Command_t *Commands_p)
{
        IDENTIFIER_NOT_USED(Commands_p);
        IDENTIFIER_NOT_USED(CommandCount);
        IDENTIFIER_NOT_USED(fInOrder);
        IDENTIFIER_NOT_USED(Session.p);
        LOG_FUNC_ENTER;

        LOG_FUNC_EXIT;
        return PKA_ERROR_NOT_IMPLEMENTED;
}

/*---------------------------------------------------------------------------
 * PKA_ResultGet
 *
 * Return Value
 *     <0  Error (see list of PKA_ERROR_* codes)
 *      0  No result was available.
 *      1  Success. Result structure has been populated.
 */

int PKA_ResultGet(const PKA_Session_t Session, PKA_Result_t *const Result_p,
                  uint16_t ResultCode_Expected)
{
        bool Hamming_Err = false;
        bool Parity_Err  = false;

        IDENTIFIER_NOT_USED(Session.p);
        IDENTIFIER_NOT_USED(ResultCode_Expected);
        LOG_FUNC_ENTER;

        // Retrieve result only if Command was successfluly submitted
        if (PKACtx.CommandPutSuccess < 1) {
                LOG_INFO(LogInfo "No Command Submitted yet\n");
                return 0;
        }

        if (true == EIP28_CheckReset_SeqCtrl(PKACtx.EIP28_IOArea_p, &Parity_Err,
                                             &Hamming_Err)) {
                LOG_CRIT(LogErr
                         "Reset bit was set in the Sequence Control register!\n"
                         "\tParity error? %s\n"
                         "\tHamming error? %s\n",
                         Parity_Err ? "yes" : "no", Hamming_Err ? "yes" : "no");

                return PKA_ERROR_RESET;
        }

        // Check device if Operation done
        // If operation is done update Result with actual outcome of the operation
        if (true == Adapter_PKA_CheckIfDone(&PKACtx)) {
#ifdef ADAPTER_PKA_FSM_ERROR_CHECK_ENABLE
                bool Fsm_Error = EIP28_CheckFsmError_LNME0Status_CALLATOMIC(
                                     PKACtx.EIP28_IOArea_p) |
                                 EIP28_CheckFsmError_SCAPCtrl_CALLATOMIC(
                                     PKACtx.EIP28_IOArea_p);

                if (true == Fsm_Error) {
                        return PKA_ERROR_SCAP;
                }
#endif
                {
                        uint32_t Status = EIP28_Status_SeqCntrl_CALLATOMIC(
                            PKACtx.EIP28_IOArea_p);

                        // See if an error occurred
                        if (Status > 1) {
                                // Check if status is "InvalidTrigger"
                                if (Status == 0x21) {
                                        LOG_WARN(LogWarn "Sequencer operation "
                                                         "not implemented\n");
                                        return PKA_ERROR_NOT_IMPLEMENTED;
                                }
                                LOG_CRIT(LogErr
                                         "Sequencer has a error: 0x%02X\n",
                                         Status);
                                return PKA_ERROR_INTERNAL;
                        }
                }
                RB_PERF_END_POINT();

                // LOG_INFO(LogInfo "Execution Complete\n");
                if (Adapter_PKA_ResultGetforOperation(Result_p, &PKACtx))
                        return PKA_ERROR_INVALID_PARAMETER;
        } else {
                Result_p->Result = PKA_RESULT_PENDING;
                return 0; // PKA Result NOT available
        }

        Result_p->UserHandle = PKACtx.UserHandle;

        LOG_FUNC_EXIT;
        return 1; // PKA Result Available
}

/*---------------------------------------------------------------------------
 * PKA_ResultNotify_Request
 *
 */
int PKA_ResultNotify_Request(const PKA_Session_t        Session,
                             PKA_ResultNotifyFunction_t CBFunc)
{
        LOG_FUNC_ENTER;

        if (CBFunc == NULL || !PKACtx.IsInitialized) {
                LOG_INFO(LogInfo
                         "NULL Callback function or PKA not initialized\n");
                return PKA_ERROR_INVALID_PARAMETER;
        }

        PKACtx.PKANotify.Session        = Session;
        PKACtx.PKANotify.ResultNotifyCB = CBFunc;

        LOG_FUNC_EXIT;
        return 0;
}

/* end of adapter_pka_eip28.c */

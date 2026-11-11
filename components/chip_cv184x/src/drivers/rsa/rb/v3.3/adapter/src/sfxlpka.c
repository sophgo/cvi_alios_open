/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */
/* sfxlpka.c
 *
 * SLAD PKA API and DMABuf API integration module for Security-IP PKA API.
 *
 */



/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

// Security-IP PKA API
#include "sfxlpka.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration file
#include "c_sfxlpka.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"

// Driver Framework C Run-time Library API
#include "clib.h"

// Driver Framework Device API
#include "device_swap.h"

// Driver Entry/Exit point API
// #include "api_driver28_init.h"

// SLAD PKA API
#include "api_pka.h"
#include "pka_opcodes_eip28.h"


#include "api_dmabuf.h"
#include "eip28.h"
#include "stdint.h"
#include "adapter_eip28.h"

#include <unistd.h>     // sleep(), usleep()

#ifdef SFXL_PKA_NOTIFICATIONS_ENABLE
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#endif
#include "api_driver28_init.h"
#include <drv/common.h>

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

// Maximum 4 buffers are used per one PKA operation, see pka_opcodes.h,
// may not be greater than PKA_MAX_BUFFERS in api_pka.h
#define SFXL_PKA_DMABUF_MAX_HANDLES             4

// During the execution of a complex operation, some values may need to "grow"
// somewhat. A typical example is an operation that is implemented using
// Montgomery numbers. In such an operation, it is convenient to internally
// work with numbers that are one word longer than the modulus. In that case,
// it is convenient to also extend the modulus with an extra zero word
// so that its length matches that of the internal numbers.
// To allow input (and output) numbers to grow, the presence of an anonymous
// but mandatory buffer word is required in some vectors.
#define SFXL_PKA_MANDATORY_WORD_COUNT           1

// PKA_MAX_BUFFERS is defined in api_pka.h
#if SFXL_PKA_DMABUF_MAX_HANDLES > PKA_MAX_BUFFERS
#error "SFXL_PKA_DMABUF_MAX_HANDLES may not be greater than PKA_MAX_BUFFERS"
#endif

// DMAbuf API handles used for the ModExpCrt operations
#define SFXL_PKA_DMABUF_FIXED_HANDLES           3

// Maximum supported PKA vector size is 4096 bits (128 32-bit words),
// max 2 vectors per buffer plus alignment word and mandatory buffer word
#define SFXL_PKA_DMABUF_MAX_WORD_COUNT          (2 * 128 + 2)

// DMABuf handle and host address for ModExpCRT operations
typedef struct
{
    //   one for ExpP and ExpQ vectors,
    //   one for ModP and ModQ vectors and
    //   one for input / result vectors
    DMABuf_Handle_t DMABuf_Handle;

    // Host addresses of the DMA buffers
    DMABuf_HostAddress_t DMABuf_HostAddr;

} SfxlPKA_ModExpCrt_DMABuf_t;

// Security-IP PKA device data structure
typedef struct
{
    // Maximum configured number of simultaneous PKA sessions
    PKA_Session_t   Sessions [SFXL_PKA_SESSIONS_MAX_COUNT];

    // Device capabilities
    PKA_Capabilities_t  Capabilities;

    DMABuf_Properties_t RequestedProperties [SFXL_PKA_DMABUF_MAX_HANDLES];

    // DMABuf Handles for ModExpCRT operations,
    //   one for ExpP and ExpQ vectors,
    //   one for ModP and ModQ vectors and
    //   one for input / result vectors
    SfxlPKA_ModExpCrt_DMABuf_t ModExpCrt_DMABuf [SFXL_PKA_DMABUF_FIXED_HANDLES];

#ifdef SFXL_PKA_NOTIFICATIONS_ENABLE
    pthread_mutex_t Mutex;
    pthread_cond_t Cond;
#endif

    volatile bool fNotified;

    // true when the CPU endianness differs from that of the PKA device
    bool fEndianMismatch;

    // Data structure to use to submit a command to the PKA device
    PKA_Command_t PKA_Command;

    // Data structure to use to receive a result from the PKA device
    PKA_Result_t PKA_Result;

} SfxlPKA_Device_t;


/*----------------------------------------------------------------------------
 * Local variables
 */

// Maximum number of PKA devices
static SfxlPKA_Device_t PKA_Devices [SFXL_PKA_DEVICES_MAX_COUNT];


// #ifdef SFXL_PKA_NOTIFICATIONS_ENABLE
// /*---------------------------------------------------------------------------
//    SfxlPKALib_Flag_Wait
//  */
// static inline void
// SfxlPKALib_Flag_Wait(
//         const unsigned int TimeoutMsec,
//         pthread_mutex_t * const Mutex_p,
//         pthread_cond_t * const Cond_p)
// {
//     struct timespec ts;
//     unsigned int secs,nsecs;

//     ZEROINIT(ts);

//     nsecs = (TimeoutMsec % 1000) * 1000000;
//     secs = TimeoutMsec / 1000;

//     clock_gettime(CLOCK_REALTIME, &ts);

//     ts.tv_nsec += nsecs;
//     ts.tv_sec += secs;

//     if (ts.tv_nsec >= 1000000000)
//     {
//         ts.tv_nsec -= 1000000000;
//         ts.tv_sec += 1;
//     }

//     pthread_mutex_lock(Mutex_p);

//     pthread_cond_timedwait(Cond_p, Mutex_p, &ts);

//     pthread_mutex_unlock(Mutex_p);
// }


// /*---------------------------------------------------------------------------
//    SfxlPKALib_Wakeup
//  */
// static inline void
// SfxlPKALib_Wakeup(
//         pthread_cond_t * const Cond_p)
// {
//     pthread_cond_signal(Cond_p);
// }


// static void
// SfxlPKALib_ResultNotifyFunction(
//         const PKA_Session_t Session,
//         const unsigned int ResultCount)
// {
//     // Only one device is supported
//     SfxlPKA_Device_t * Device_p = &PKA_Devices[0];

//     IDENTIFIER_NOT_USED(ResultCount);

//     if (Device_p->Sessions[0].p != Session.p)
//     {
//         LOG_CRIT("SfxlPka: %s, notification failure, PKA session mismatch\n",
//                  __func__);
//         return;
//     }

//     LOG_INFO("SfxPka: %s, notification received\n", __func__);

//     Device_p->fNotified = true;

//     SfxlPKALib_Wakeup(&Device_p->Cond);
// }
// #endif // SFXL_PKA_NOTIFICATIONS_ENABLE


/*---------------------------------------------------------------------------
   SfxlPKALib_uSeconds_Wait
 */
void
SfxlPKALib_uSeconds_Wait(const unsigned int uSeconds)
{
    //sleep((uSeconds)/1000000);      // Wait the whole seconds
    udelay((uSeconds)%1000000);     // Wait what's left from it
}


/*---------------------------------------------------------------------------
   SfxlPKALib_Registered_DMABuf_Release
 */
static void
SfxlPKALib_Registered_DMABuf_Release(
        const unsigned int DeviceID)
{
    unsigned int i;
    SfxlPKA_Device_t * Device_p = &PKA_Devices[DeviceID];

    for (i = 0; i < SFXL_PKA_DMABUF_MAX_HANDLES; i++)
    {
        if (!DMABuf_Handle_IsSame(&Device_p->PKA_Command.Handles[i],
                                  &DMABuf_NULLHandle))
        {
            DMABuf_Release(Device_p->PKA_Command.Handles[i]);
            Device_p->PKA_Command.Handles[i] = DMABuf_NULLHandle;
        }
    }

    return;
}


/*---------------------------------------------------------------------------
   SfxlPKALib_Error_Translate
 */
inline static SfxlPKA_Error_t
SfxlPKALib_Error_Translate(
        const DMABuf_Status_t DMABuf_Rc)
{
    // Translate DMAbuf API errors to Sfxl PKA API errors
    if (DMABuf_Rc == DMABUF_ERROR_OUT_OF_MEMORY)
        return SFXL_PKA_OUT_OF_SYSMEMORY_ERROR;
    else if (DMABuf_Rc == DMABUF_ERROR_BAD_ARGUMENT)
        return SFXL_PKA_INPUT_ERROR;
    else
        return SFXL_PKA_INTERNAL_ERROR;
}


/*---------------------------------------------------------------------------
   SfxlPKALib_Result_Get
 */
static SfxlPKA_Error_t
SfxlPKALib_Result_Get(
        const PKA_Session_t Session,
        PKA_Result_t * const PKA_Res_p,
        const char * const szFuncName)
{
    int PKA_Rc;

#ifdef SFXL_PKA_NOTIFICATIONS_ENABLE
    // Try to get the processed result
    PKA_Rc = PKA_ResultGet(Session,
                           PKA_Res_p,
                           0); // expected result, not used
    if (PKA_Rc == 0)
    {
        LOG_CRIT("SfxlPka: %s, no result available\n",
                 szFuncName);
        PKA_Rc = -99;
    }
#else
    unsigned int LoopCount = SFXL_PKA_BUSYWAIT_COUNT;

    while (LoopCount > 0)
    {
        // Try to get the processed result
        PKA_Rc = PKA_ResultGet(Session,
                               PKA_Res_p,
                               0); // expected result, not used
        if (PKA_Rc > 0)
            break; // success
        else if (PKA_Rc == 0)
        {
            SfxlPKALib_uSeconds_Wait(SFXL_PKA_DELAY_USEC);
            LoopCount--;
        }
        else
            break; // device error
    } // while
#endif

    // Process the result
    if(PKA_Rc == 0)
    {
        LOG_CRIT("SfxlPka: %s, timeout waiting for result\n", szFuncName);
        return SFXL_PKA_TIMEOUT_ERROR;
    }
    else if(PKA_Rc < 0)
    {
        LOG_CRIT("SfxlPka: %s, "
                 "PKA result get function/device error %d/%d,\n",
                 szFuncName,
                 PKA_Rc,
                 PKA_Res_p->Result);
        return SFXL_PKA_DEVICE_ERROR;
    }
    else
    {
        // LOG_INFO("SfxlPka: %s, result received successfully\n",
        //          szFuncName);
    }

    return SFXL_PKA_NO_ERROR;

}


/*---------------------------------------------------------------------------
   SfxlPKALib_Op_Exec
 */
static SfxlPKA_Error_t
SfxlPKALib_Op_Exec(
        SfxlPKA_Device_t * const Device_p,
        const char * const szFuncName)
{
    int PKA_Rc;
    SfxlPKA_Error_t SfxlPKA_Rc;

    PKA_Session_t Session       = Device_p->Sessions[0];
    PKA_Command_t * PKA_Cmd_p   = &Device_p->PKA_Command;
    PKA_Result_t * PKA_Res_p    = &Device_p->PKA_Result;

#ifdef SFXL_PKA_NOTIFICATIONS_ENABLE
    Device_p->fNotified = false;

    PKA_Rc = PKA_ResultNotify_Request(Session, SfxlPKALib_ResultNotifyFunction);
    if (PKA_Rc != 0)
    {
        LOG_CRIT("SfxlPKA: %s, notification request error %d\n",
                 szFuncName,
                 PKA_Rc);
        return SFXL_PKA_DEVICE_ERROR;
    }
#endif

    // LOG_CRIT("%s, %d, 0x%x\n", __FUNCTION__, __LINE__, 0);
    // Submit packet to device
    PKA_Rc = PKA_CommandPut(Session, PKA_Cmd_p);
    if (PKA_Rc != 0)
    {
        LOG_CRIT("SfxlPKA: %s, command put error %d\n",
                 szFuncName,
                 PKA_Rc);
        return SFXL_PKA_DEVICE_ERROR;
    }

    // LOG_INFO("SfxPka: %s, command started at the device\n", szFuncName);

#ifdef SFXL_PKA_NOTIFICATIONS_ENABLE
    SfxlPKALib_Flag_Wait((SFXL_PKA_BUSYWAIT_COUNT *
                          SFXL_PKA_DELAY_USEC) / 1000,
                          &Device_p->Mutex,
                          &Device_p->Cond);

    if(Device_p->fNotified == false)
    {
        LOG_CRIT("SfxlPka: %s, timeout waiting for result notification\n",
                 szFuncName);

        // Try to get result anyway
        SfxlPKA_Rc = SfxlPKALib_Result_Get(Session,
                                           PKA_Res_p,
                                           szFuncName);
        if (SfxlPKA_Rc != SFXL_PKA_NO_ERROR)
            LOG_CRIT("SfxlPka: %s, "
                     "failed to get result for timed out notification\n",
                     szFuncName);

        return SFXL_PKA_TIMEOUT_ERROR;
    }
#endif

    SfxlPKA_Rc = SfxlPKALib_Result_Get(Session,
                                       PKA_Res_p,
                                       szFuncName);

    return SfxlPKA_Rc;
}


/*---------------------------------------------------------------------------
   SfxlPKALib_Endian_Swap

   Note: SLAD PKA API implementation expects the vector 32-bit data words
         in the DMABuf buffers filled with Least Significant Words (LSW) first
         and within words Least Significant Bytes (LSB) first.
 */
inline static void
SfxlPKALib_Endian_Swap (
        uint32_t * Word32_p,
        const unsigned int Word32Count)
{
    unsigned int i;
    LOG_CRIT("%s, %d\n", __FUNCTION__, __LINE__);

    // Swap the bytes in 32-bit words
    for (i = 0; i < Word32Count; i++)
        Word32_p[i] = Device_SwapEndian32(Word32_p[i]);
}


/*---------------------------------------------------------------------------
   SfxlPKA_Device_Init
 */
SfxlPKA_Error_t
SfxlPKA_Device_Init ()
{
    int PKA_Rc;
    DMABuf_Status_t DMABuf_Rc;
    SfxlPKA_Error_t SfxlPKA_Rc = SFXL_PKA_DEVICE_ERROR;
    SfxlPKA_Device_t * Device_p;

    uint32_t DeviceID =  0;
#ifdef SFXL_PKA_STRICT_ARGS_CHECK
    if (DeviceID > SFXL_PKA_DEVICES_MAX_COUNT)
        return SFXL_PKA_INPUT_ERROR;
#endif

    // Get device capabilities
    Device_p = &PKA_Devices[DeviceID];

    // // Detect the CPU endianness, EIP-28 PKA device is Little Endian
    // {
    //     uint32_t Word32 = 0x12345678;
    //     uint8_t * Byte_p = (uint8_t*)&Word32;

    //     // LSB first?
    //     if (*Byte_p == 0x78)
    //         Device_p->fEndianMismatch = false;
    //     else if (*Byte_p == 0x12)
    //         Device_p->fEndianMismatch = true;
    //     else
    //         return SFXL_PKA_INTERNAL_ERROR; // Unknown endian format

    //     LOG_CRIT("SfxlPKA: %s, CPU is %s Endian, %lu-byte integer\n",
    //              __func__,
    //              Device_p->fEndianMismatch ? "Big" : "Little",
    //              (unsigned long)sizeof(unsigned long));

    //     //Device_p->fEndianMismatch = false;
    // }
    /* bigendian input, MUST false, otherwise alg check failed. */
    Device_p->fEndianMismatch = false;

    // PKA device driver entry point
    PKA_Rc = Driver28_Init();
    if (PKA_Rc != 0)
    {
        LOG_CRIT("SfxlPKA: %s, driver initialization failed, error %d\n",
                 __func__,
                 PKA_Rc);
        return SFXL_PKA_DEVICE_ERROR;
    }

    // Initialize PKA device
    PKA_Rc = PKA_Init(DeviceID);
    if (PKA_Rc != 0)
    {
        LOG_CRIT("SfxlPKA: %s, device initialization failed, error %d\n",
                 __func__,
                 PKA_Rc);
        goto pkainit_error;
    }

    PKA_Rc = PKA_Capabilities_Get(&Device_p->Capabilities);
    if (PKA_Rc != 0)
    {
        LOG_CRIT("SfxlPKA: %s, get capabilities failed, error %d\n",
                 __func__,
                 PKA_Rc);
        goto pkaopen_error;
    }

    LOG_INFO("SfxlPKA: %s, device %d capabilities:\n",
                         __func__,
                         DeviceID);
    LOG_INFO("\tSupported operations: %s\n"
                         "\tMaximum vector size (bits): %d\n",
                         Device_p->Capabilities.szTextDescription,
                         Device_p->Capabilities.Vector_MaxBits);

    // Open PKA session

    // Only one session per PKA device is supported at the moment
    PKA_Rc = PKA_Open(&Device_p->Sessions[0], DeviceID);
    if (PKA_Rc != 0)
    {
        LOG_CRIT("SfxlPKA: %s, open session failed, error %d\n",
                 __func__,
                 PKA_Rc);
        goto pkaopen_error;
    }

    ZEROINIT(Device_p->RequestedProperties);

// #ifdef SFXL_PKA_NOTIFICATIONS_ENABLE
//     if (pthread_mutex_init(&Device_p->Mutex, NULL) != 0)
//     {
//         LOG_CRIT("SfxlPKA: %s, mutex init failed\n", __func__);
//         SfxlPKA_Rc = SFXL_PKA_INTERNAL_ERROR;
//         goto pkamutex_error;
//     }

//     if (pthread_cond_init(&Device_p->Cond, NULL) != 0)
//     {
//         LOG_CRIT("SfxlPKA: %s, condition init failed\n", __func__);
//         SfxlPKA_Rc = SFXL_PKA_INTERNAL_ERROR;
//         goto pkacond_error;
//     }
// #endif

    // Set all DMABuf handles to NULL
    {
        unsigned int i;
        DMABuf_Properties_t DMABufProperties;

        ZEROINIT(DMABufProperties);

        DMABufProperties.Alignment  = SFXL_PKA_DMABUF_ALIGNMENT_BYTE_COUNT;
        DMABufProperties.Bank       = SFXL_PKA_DMABUF_BANK;
        DMABufProperties.fCached    = true;
        DMABufProperties.Size       = SFXL_PKA_DMABUF_MAX_WORD_COUNT *
                                                  sizeof(SfxlPKA_Word_t);

        // Prepare DMA buffer properties for all required DMA buffer handles
        for (i = 0; i < SFXL_PKA_DMABUF_MAX_HANDLES; i++)
        {
            Device_p->PKA_Command.Handles [i] = DMABuf_NULLHandle;
            Device_p->RequestedProperties[i].Alignment  =
                                          SFXL_PKA_DMABUF_ALIGNMENT_BYTE_COUNT;
            Device_p->RequestedProperties[i].Bank       = SFXL_PKA_DMABUF_BANK;
            Device_p->RequestedProperties[i].fCached    = true;
        }

        // Pre-allocate all fixed DMA buffers for ModExpCrt operations
        for (i = 0; i < SFXL_PKA_DMABUF_FIXED_HANDLES; i++)
        {
            DMABuf_Rc = DMABuf_Alloc(
                             DMABufProperties,
                             &Device_p->ModExpCrt_DMABuf[i].DMABuf_HostAddr,
                             &Device_p->ModExpCrt_DMABuf[i].DMABuf_Handle);
            if (DMABuf_Rc != DMABUF_STATUS_OK)
                goto dmabuf_error;
        }
    }

    return SFXL_PKA_NO_ERROR; // success

    // Something went wrong, free all resources claimed so far

dmabuf_error:
    LOG_CRIT("SfxlPKA: %s, DMA buffer allocation failed, error %d\n",
             __func__,
             DMABuf_Rc);

    // Release registered DMA buffers
    SfxlPKALib_Registered_DMABuf_Release(DeviceID);

    SfxlPKA_Rc = SFXL_PKA_OUT_OF_SYSMEMORY_ERROR;

// #ifdef SFXL_PKA_NOTIFICATIONS_ENABLE
//     pthread_cond_destroy(&Device_p->Cond);

// pkacond_error:
//     pthread_mutex_destroy(&Device_p->Mutex);

// pkamutex_error:
// #endif
    PKA_Close(Device_p->Sessions[0]);

pkaopen_error:
    PKA_UnInit(DeviceID);

pkainit_error:
    Driver28_Exit();

    return SfxlPKA_Rc;
}


/*---------------------------------------------------------------------------
   SfxlPKA_Device_Uninit
 */
SfxlPKA_Error_t
SfxlPKA_Device_Uninit ()
{
    int PKA_Rc;
    unsigned int i;
    SfxlPKA_Device_t * Device_p;
    uint32_t DeviceID =  0;

#ifdef SFXL_PKA_STRICT_ARGS_CHECK
    if (DeviceID > SFXL_PKA_DEVICES_MAX_COUNT)
        return SFXL_PKA_INPUT_ERROR;
#endif

    // Close PKA session
    Device_p = &PKA_Devices[DeviceID];

// #ifdef SFXL_PKA_NOTIFICATIONS_ENABLE
//     pthread_cond_destroy(&Device_p->Cond);
//     pthread_mutex_destroy(&Device_p->Mutex);
// #endif

    PKA_Rc = PKA_Close(Device_p->Sessions[0]);
    if (PKA_Rc != 0)
    {
        LOG_CRIT("SfxlPKA: %s, close session failed, error %d\n",
                 __func__,
                 PKA_Rc);
        return SFXL_PKA_DEVICE_ERROR;
    }

    // Un-initialize the device
    PKA_Rc = PKA_UnInit(DeviceID);
    if (PKA_Rc != 0)
    {
        LOG_CRIT("SfxlPKA: %s, get capabilities failed, error %d\n",
                 __func__,
                 PKA_Rc);
        return SFXL_PKA_DEVICE_ERROR;
    }

    // Release registered DMA buffers
    SfxlPKALib_Registered_DMABuf_Release(DeviceID);

    // Release all pre-allocated DMA buffers for ModExpCrt operations
    for (i = 0; i < SFXL_PKA_DMABUF_FIXED_HANDLES; i++)
    {
        if (!DMABuf_Handle_IsSame(&Device_p->ModExpCrt_DMABuf[i].DMABuf_Handle,
                                  &DMABuf_NULLHandle))
        {
            DMABuf_Release(Device_p->ModExpCrt_DMABuf[i].DMABuf_Handle);
            Device_p->ModExpCrt_DMABuf[i].DMABuf_Handle = DMABuf_NULLHandle;
        }
    }

    // Driver exit point
    Driver28_Exit();

    return SFXL_PKA_NO_ERROR;
}

/*---------------------------------------------------------------------------
   SfxlPKA_ModExp
 */
SfxlPKA_Error_t
SfxlPKA_ModExp(
        const unsigned int DeviceID,
        SfxlPKA_ModExp_Params_t * const Params_p)
{
    
    SfxlPKA_Error_t SfxlPKA_Rc = SFXL_PKA_INTERNAL_ERROR;
    SfxlPKA_Device_t * Device_p;
    PKA_Command_t * PKA_Cmd_p;
    PKA_Result_t * PKA_Res_p;
    DMABuf_Status_t DMABuf_Rc;
    DMABuf_Properties_t * DMABuf_ReqProps_p;
    DMABuf_Handle_t * DMABuf_Handles_p;

    // LOG_INFO("\nSfxlPKA: %s invoked\n", __func__);

#ifdef SFXL_PKA_STRICT_ARGS_CHECK
    if (DeviceID > SFXL_PKA_DEVICES_MAX_COUNT)
        return SFXL_PKA_INPUT_ERROR;

    // Check if result buffer is large enough.
    // See pka_opcodes.h, PKA_OPCODE_MODEXP opcode
    if (Params_p->Result.WordCount < Params_p->Modulus.WordCount + 1)
        return SFXL_PKA_INPUT_ERROR;
#endif

    Device_p = &PKA_Devices[DeviceID];

    DMABuf_ReqProps_p = Device_p->RequestedProperties;
    DMABuf_Handles_p  = Device_p->PKA_Command.Handles;

    // Set Encrypted to false
    ZEROINIT(Device_p->PKA_Command);

    PKA_Cmd_p = &Device_p->PKA_Command;

    PKA_Cmd_p->OpCode = PKA_OPCODE_MODEXP;
    // PKA_Cmd_p->Extra = SFXL_PKA_ODD_POWERS;
    PKA_Cmd_p->Extra = Params_p->Extra;

    // Register DMA buffers
    {
        // Vector A: input, exponent
        PKA_Cmd_p->A_Len = Params_p->Exponent.WordCount;
        DMABuf_ReqProps_p[0].Size = PKA_Cmd_p->A_Len * sizeof(SfxlPKA_Word_t);

        // Ensure the endianness differences are eliminated.
        if (Device_p->fEndianMismatch)
            SfxlPKALib_Endian_Swap(Params_p->Exponent.Word_p,
                                   Params_p->Exponent.WordCount);

        DMABuf_Rc = DMABuf_Register(DMABuf_ReqProps_p[0],
                                    Params_p->Exponent.Word_p,
                                    NULL,
                                    'N',
                                    &DMABuf_Handles_p[0]);
        if (DMABuf_Rc != DMABUF_STATUS_OK)
        {
            // Restore endianness
            if (Device_p->fEndianMismatch)
                SfxlPKALib_Endian_Swap(Params_p->Exponent.Word_p,
                                       Params_p->Exponent.WordCount);
            LOG_CRIT("%s, %d, 0x%x\n", __FUNCTION__, __LINE__, 0);
            goto dmabuf_error;
        }

        // Vector B: input, modulus
        PKA_Cmd_p->B_Len = Params_p->Modulus.WordCount;
        // LOG_CRIT("%s, %d, 0x%x\n", __FUNCTION__, __LINE__, PKA_Cmd_p->B_Len);
        DMABuf_ReqProps_p[1].Size = PKA_Cmd_p->B_Len * sizeof(SfxlPKA_Word_t);

        // Ensure the endianness differences are eliminated.
        if (Device_p->fEndianMismatch)
            SfxlPKALib_Endian_Swap(Params_p->Modulus.Word_p,
                                   Params_p->Modulus.WordCount);

        DMABuf_Rc = DMABuf_Register(DMABuf_ReqProps_p[1],
                                    Params_p->Modulus.Word_p,
                                    NULL,
                                    'N',
                                    &DMABuf_Handles_p[1]);
        if (DMABuf_Rc != DMABUF_STATUS_OK)
        {
            // Restore endianness
            if (Device_p->fEndianMismatch)
            {
                SfxlPKALib_Endian_Swap(Params_p->Exponent.Word_p,
                                       Params_p->Exponent.WordCount);
                SfxlPKALib_Endian_Swap(Params_p->Modulus.Word_p,
                                       Params_p->Modulus.WordCount);
            }
            
            LOG_CRIT("%s, %d, 0x%x\n", __FUNCTION__, __LINE__, 0);
            goto dmabuf_error;
        }

        // Vector C: input, base
        DMABuf_ReqProps_p[2].Size = Params_p->Base.WordCount *
                                                sizeof(SfxlPKA_Word_t);

        // Ensure the endianness differences are eliminated.
        if (Device_p->fEndianMismatch)
            SfxlPKALib_Endian_Swap(Params_p->Base.Word_p,
                                   Params_p->Base.WordCount);

        DMABuf_Rc = DMABuf_Register(DMABuf_ReqProps_p[2],
                                    Params_p->Base.Word_p,
                                    NULL,
                                    'N',
                                    &DMABuf_Handles_p[2]);
        if (DMABuf_Rc != DMABUF_STATUS_OK)
        {
            // Restore endianness
            if (Device_p->fEndianMismatch)
            {
                SfxlPKALib_Endian_Swap(Params_p->Exponent.Word_p,
                                       Params_p->Exponent.WordCount);
                SfxlPKALib_Endian_Swap(Params_p->Modulus.Word_p,
                                       Params_p->Modulus.WordCount);
                SfxlPKALib_Endian_Swap(Params_p->Base.Word_p,
                                       Params_p->Base.WordCount);
            }
            LOG_CRIT("%s, %d, 0x%x\n", __FUNCTION__, __LINE__, 0);
            goto dmabuf_error;
        }

        // Vector D: output, result
        DMABuf_ReqProps_p[3].Size = (PKA_Cmd_p->B_Len + 1) *
                                                      sizeof(SfxlPKA_Word_t);

        DMABuf_Rc = DMABuf_Register(DMABuf_ReqProps_p[3],
                                    Params_p->Result.Word_p,
                                    NULL,
                                    'N',
                                    &DMABuf_Handles_p[3]);
        if (DMABuf_Rc != DMABUF_STATUS_OK)
        {
            // Restore endianness
            if (Device_p->fEndianMismatch)
            {
                SfxlPKALib_Endian_Swap(Params_p->Exponent.Word_p,
                                       Params_p->Exponent.WordCount);
                SfxlPKALib_Endian_Swap(Params_p->Modulus.Word_p,
                                       Params_p->Modulus.WordCount);
                SfxlPKALib_Endian_Swap(Params_p->Base.Word_p,
                                       Params_p->Base.WordCount);
            }
            LOG_CRIT("%s, %d, 0x%x\n", __FUNCTION__, __LINE__, 0);
            goto dmabuf_error;
        }
    } // DMA buffers registered

    ZEROINIT(Device_p->PKA_Result);

    PKA_Res_p = &Device_p->PKA_Result;

    // Perform PKA operation
    SfxlPKA_Rc = SfxlPKALib_Op_Exec(Device_p, __func__);
    if (SfxlPKA_Rc != SFXL_PKA_NO_ERROR)
    {
        // Restore endianness
        if (Device_p->fEndianMismatch)
        {
            SfxlPKALib_Endian_Swap(Params_p->Exponent.Word_p,
                                   Params_p->Exponent.WordCount);
            SfxlPKALib_Endian_Swap(Params_p->Modulus.Word_p,
                                   Params_p->Modulus.WordCount);
            SfxlPKALib_Endian_Swap(Params_p->Base.Word_p,
                                   Params_p->Base.WordCount);
        }
        goto error;
    }

    if (PKA_Res_p->MSW > Params_p->Result.WordCount)
    {
        LOG_CRIT("\nSfxlPKA: %s error, result too large\n", __func__);
        SfxlPKA_Rc = SFXL_PKA_DEVICE_ERROR;
        // Restore endianness
        if (Device_p->fEndianMismatch)
        {
            SfxlPKALib_Endian_Swap(Params_p->Exponent.Word_p,
                                   Params_p->Exponent.WordCount);
            SfxlPKALib_Endian_Swap(Params_p->Modulus.Word_p,
                                   Params_p->Modulus.WordCount);
            SfxlPKALib_Endian_Swap(Params_p->Base.Word_p,
                                   Params_p->Base.WordCount);
        }
        goto error;
    }

    Params_p->Result.WordCount = PKA_Res_p->MSW + 1;

    // Ensure the endianness differences are eliminated.
    if (Device_p->fEndianMismatch)
        SfxlPKALib_Endian_Swap(Params_p->Result.Word_p,
                               Params_p->Result.WordCount);

    // Restore endianness
    if (Device_p->fEndianMismatch)
    {
        SfxlPKALib_Endian_Swap(Params_p->Exponent.Word_p,
                               Params_p->Exponent.WordCount);
        SfxlPKALib_Endian_Swap(Params_p->Modulus.Word_p,
                               Params_p->Modulus.WordCount);
        SfxlPKALib_Endian_Swap(Params_p->Base.Word_p,
                               Params_p->Base.WordCount);
    }

    // Release registered DMA buffers
    SfxlPKALib_Registered_DMABuf_Release(DeviceID);

    return SFXL_PKA_NO_ERROR;

dmabuf_error:
    // DMA-safe buffer allocation failed
    LOG_CRIT("SfxlPKA: %s, DMA buffer registration error %d\n",
            __func__,
            DMABuf_Rc);

    // Translate DMAbuf API errors to Sfxl PKA API errors
    SfxlPKA_Rc = SfxlPKALib_Error_Translate(DMABuf_Rc);

error:
    // Release registered DMA buffers
    SfxlPKALib_Registered_DMABuf_Release(DeviceID);

    return SfxlPKA_Rc;
}


/*---------------------------------------------------------------------------
   SfxlPKA_ModExpCrt
 */
SfxlPKA_Error_t
SfxlPKA_ModExpCrt(
        const unsigned int DeviceID,
        SfxlPKA_ModExpCrt_Params_t * const Params_p)
{
    SfxlPKA_Error_t SfxlPKA_Rc = SFXL_PKA_INTERNAL_ERROR;
    SfxlPKA_Device_t * Device_p;
    PKA_Command_t * PKA_Cmd_p;
    PKA_Result_t * PKA_Res_p;
    DMABuf_Status_t DMABuf_Rc;
    DMABuf_Properties_t * DMABuf_ReqProps_p;
    DMABuf_Handle_t * DMABuf_Handles_p;
    DMABuf_HostAddress_t HostAddr;

    // LOG_INFO("\nSfxlPKA: %s invoked\n", __func__);

#ifdef SFXL_PKA_STRICT_ARGS_CHECK
    if (DeviceID > SFXL_PKA_DEVICES_MAX_COUNT)
        return SFXL_PKA_INPUT_ERROR;

    // Check if result buffer is large enough.
    // See pka_opcodes.h, PKA_OPCODE_MODEXPCRT opcode
    if (Params_p->Result.WordCount < 2 * Params_p->ExponentP.WordCount ||
        Params_p->Result.WordCount < 2 * Params_p->ExponentQ.WordCount)
        return SFXL_PKA_INPUT_ERROR;
#endif

    Device_p = &PKA_Devices[DeviceID];

    DMABuf_ReqProps_p = Device_p->RequestedProperties;
    DMABuf_Handles_p  = Device_p->PKA_Command.Handles;

    // Set Encrypted to false
    ZEROINIT(Device_p->PKA_Command);

    PKA_Cmd_p = &Device_p->PKA_Command;

    PKA_Cmd_p->OpCode = PKA_OPCODE_MODEXP_CRT;
    PKA_Cmd_p->Extra = SFXL_PKA_ODD_POWERS;

    // Register/assign DMA buffers
    {
        unsigned int AdjustedSize;

        PKA_Cmd_p->B_Len = Params_p->ModulusP.WordCount;

        // Vector C (input): Inverse Q
        DMABuf_ReqProps_p[2].Size = Params_p->InverseQ.WordCount *
                                                    sizeof(SfxlPKA_Word_t);

        // Ensure the endianness differences are eliminated.
        if (Device_p->fEndianMismatch)
            SfxlPKALib_Endian_Swap(Params_p->InverseQ.Word_p,
                                   Params_p->InverseQ.WordCount);

        DMABuf_Rc = DMABuf_Register(DMABuf_ReqProps_p[2],
                                    Params_p->InverseQ.Word_p,
                                    NULL,
                                    'N',
                                    &DMABuf_Handles_p[2]);
        if (DMABuf_Rc != DMABUF_STATUS_OK)
            goto dmabuf_error;

        // Vector A (input): Exponent P zero-padded to next multiple of 64-bit
        //                   and Exponent Q

        // Adjust size for alignment
        // ExponentP.WordCount is in SfxlPKA_Word_t words,
        // ExpQ must follow it aligned at 64-bit offset
        if (Params_p->ExponentP.WordCount & 1)
            AdjustedSize = Params_p->ExponentP.WordCount + 1;
        else
            AdjustedSize = Params_p->ExponentP.WordCount;

        PKA_Cmd_p->A_Len = Params_p->ExponentP.WordCount;

        // Copy ExpP
        DMABuf_Handles_p[0] = Device_p->ModExpCrt_DMABuf[0].DMABuf_Handle;
        HostAddr.p = Device_p->ModExpCrt_DMABuf[0].DMABuf_HostAddr.p;
        memcpy(HostAddr.p,
               Params_p->ExponentP.Word_p,
               Params_p->ExponentP.WordCount * sizeof(SfxlPKA_Word_t));

        // Ensure the endianness differences are eliminated.
        if (Device_p->fEndianMismatch)
            SfxlPKALib_Endian_Swap(HostAddr.p, Params_p->ExponentP.WordCount);

        // If needed zero-pad ExpP to next multiple of 64-bit
        if (Params_p->ExponentP.WordCount & 1)
        {
            SfxlPKA_Word_t * p = (SfxlPKA_Word_t *)HostAddr.p;

            p += Params_p->ExponentP.WordCount;

            memset(p, 0, sizeof(SfxlPKA_Word_t));
        }

        // Append ExpQ
        {
            SfxlPKA_Word_t * p = (SfxlPKA_Word_t *)HostAddr.p;

            p += AdjustedSize;

            memcpy(p,
                   Params_p->ExponentQ.Word_p,
                   Params_p->ExponentQ.WordCount * sizeof(SfxlPKA_Word_t));

            // Ensure the endianness differences are eliminated.
            if (Device_p->fEndianMismatch)
                SfxlPKALib_Endian_Swap((uint32_t*)p, Params_p->ExponentQ.WordCount);
        }

        // Vector B (input): Modulus P zero-padded to next multiple of 64-bit
        //                   and Modulus Q

        // Adjust size for mandatory buffer word and alignment
        // ModulusP.WordCount is in SfxlPKA_Word_t words,
        // one mandatory buffer word and ModQ must follow ModP at
        // the offset aligned at 64-bit
        if ((Params_p->ModulusP.WordCount + SFXL_PKA_MANDATORY_WORD_COUNT) & 1)
            AdjustedSize = Params_p->ModulusP.WordCount +
                           SFXL_PKA_MANDATORY_WORD_COUNT +
                           1; // add alignment word
        else
            AdjustedSize = Params_p->ModulusP.WordCount +
                                    SFXL_PKA_MANDATORY_WORD_COUNT;

        // Account for size alignment and mandatory buffer word too

        // Copy ModP
        DMABuf_Handles_p[1] = Device_p->ModExpCrt_DMABuf[1].DMABuf_Handle;
        HostAddr.p = Device_p->ModExpCrt_DMABuf[1].DMABuf_HostAddr.p;
        memcpy(HostAddr.p,
               Params_p->ModulusP.Word_p,
               Params_p->ModulusP.WordCount * sizeof(SfxlPKA_Word_t));

        // Ensure the endianness differences are eliminated.
        if (Device_p->fEndianMismatch)
            SfxlPKALib_Endian_Swap(HostAddr.p, Params_p->ModulusP.WordCount);

        // Copy mandatory buffer word
        memset((uint8_t *)HostAddr.p +
                    (Params_p->ModulusP.WordCount * sizeof(SfxlPKA_Word_t)),
               0,
               SFXL_PKA_MANDATORY_WORD_COUNT * sizeof(SfxlPKA_Word_t));

        // If needed zero-pad ModP to next multiple of 64-bit
        if ((Params_p->ModulusP.WordCount + SFXL_PKA_MANDATORY_WORD_COUNT) & 1)
        {
            SfxlPKA_Word_t * p = (SfxlPKA_Word_t *)HostAddr.p;

            p += (Params_p->ModulusP.WordCount +
                            SFXL_PKA_MANDATORY_WORD_COUNT);

            memset(p, 0, sizeof(SfxlPKA_Word_t));
        }

        // Append ModQ
        {
            SfxlPKA_Word_t * p = (SfxlPKA_Word_t *)HostAddr.p;

            p += AdjustedSize;

            memcpy(p,
                   Params_p->ModulusQ.Word_p,
                   Params_p->ModulusQ.WordCount * sizeof(SfxlPKA_Word_t));

            // Ensure the endianness differences are eliminated.
            if (Device_p->fEndianMismatch)
                SfxlPKALib_Endian_Swap((uint32_t*)p,
                                       Params_p->ModulusQ.WordCount);
        }

        // Vector D input: input for ModExp;
        //          output: result.

        // Copy Input data
        DMABuf_Handles_p[3] = Device_p->ModExpCrt_DMABuf[2].DMABuf_Handle;
        HostAddr.p = Device_p->ModExpCrt_DMABuf[2].DMABuf_HostAddr.p;
        memcpy(HostAddr.p,
               Params_p->Base.Word_p,
               Params_p->Base.WordCount * sizeof(SfxlPKA_Word_t));

        // Ensure the endianness differences are eliminated.
        if (Device_p->fEndianMismatch)
            SfxlPKALib_Endian_Swap(HostAddr.p, Params_p->Base.WordCount);
    } // DMA buffers registered/allocated

    ZEROINIT(Device_p->PKA_Result);

    PKA_Res_p = &Device_p->PKA_Result;

    // Perform PKA operation
    SfxlPKA_Rc = SfxlPKALib_Op_Exec(Device_p, __func__);

    // Clear device PKA command handles
    DMABuf_Handles_p[0] = DMABuf_NULLHandle;
    DMABuf_Handles_p[1] = DMABuf_NULLHandle;
    DMABuf_Handles_p[3] = DMABuf_NULLHandle;

    if (SfxlPKA_Rc != SFXL_PKA_NO_ERROR)
        goto error;

    // Copy Result data
    if (PKA_Res_p->MSW > Params_p->Result.WordCount)
    {
        LOG_CRIT("\nSfxlPKA: %s error, result too large\n", __func__);
        SfxlPKA_Rc = SFXL_PKA_DEVICE_ERROR;
        goto error;
    }

    Params_p->Result.WordCount = PKA_Res_p->MSW + 1;

    memcpy(Params_p->Result.Word_p,
           HostAddr.p,
           Params_p->Result.WordCount * sizeof(SfxlPKA_Word_t));

    // Restore endianness
    if (Device_p->fEndianMismatch)
    {
        SfxlPKALib_Endian_Swap(Params_p->Result.Word_p,
                               Params_p->Result.WordCount);
        SfxlPKALib_Endian_Swap(Params_p->InverseQ.Word_p,
                               Params_p->InverseQ.WordCount);
    }

    // Release registered DMA buffers
    SfxlPKALib_Registered_DMABuf_Release(DeviceID);

    return SFXL_PKA_NO_ERROR;

dmabuf_error:
    // DMA-safe buffer allocation failed
    LOG_CRIT("SfxlPKA: %s, DMA buffer registration/allocation error %d\n",
            __func__,
            DMABuf_Rc);

    // Translate DMAbuf API errors to Sfxl PKA API errors
    SfxlPKA_Rc = SfxlPKALib_Error_Translate(DMABuf_Rc);

error:
    // Release registered DMA buffers
    SfxlPKALib_Registered_DMABuf_Release(DeviceID);

    // Restore endianness
    if (Device_p->fEndianMismatch)
        SfxlPKALib_Endian_Swap(Params_p->InverseQ.Word_p,
                               Params_p->InverseQ.WordCount);

    return SfxlPKA_Rc;
}


/* end of file sfxlpka.c */


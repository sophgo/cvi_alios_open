/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */


/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include "dmares_buf.h"
#include "dmares_rw.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

#include "device_swap.h"        // Device_SwapEndian32

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint32_t, NULL, inline, bool,
                                // IDENTIFIER_NOT_USED

// Driver Framework C Run-Time Library Abstraction API
#include "clib.h"               // ZEROINIT()

#ifdef HWPAL_DMARESOURCE_BANKS_ENABLE
// List API
#include "list.h"
#endif // HWPAL_DMARESOURCE_BANKS_ENABLE





/*----------------------------------------------------------------------------
 * DMAResource_Init
 */
bool
DMAResource_Init(void)
{

    return true;

}


/*----------------------------------------------------------------------------
 * DMAResource_UnInit
 *
 * This function can be used to uninitialize the DMAResource administration.
 * The caller must make sure that handles will not be used after this function
 * returns.
 * If memory was allocated by DMAResource_Init, this function will free it.
 */
void
DMAResource_UnInit(void)
{
}


/*----------------------------------------------------------------------------
 * DMAResource_Alloc
 */
int
DMAResource_Alloc(
        const DMAResource_Properties_t RequestedProperties,
        // DMAResource_AddrPair_t * const AddrPair_p,
        DMAResource_Handle_t * const Handle_p)
{
    return 0;

}


/*----------------------------------------------------------------------------
 * DMAResource_Release
 */
int
DMAResource_Release(
        const DMAResource_Handle_t Handle)
{
    return 0;
    
}


/*----------------------------------------------------------------------------
 * DMAResource_Read32
 */
uint32_t
DMAResource_Read32(
        const DMAResource_Handle_t Handle,
        const unsigned int WordOffset)
{
    return *((uint32_t*)(uintptr_t)Handle + WordOffset);
}


/*----------------------------------------------------------------------------
 * DMAResource_Write32
 */
void
DMAResource_Write32(
        const DMAResource_Handle_t Handle,
        const unsigned int WordOffset,
        const uint32_t Value)
{
    *((uint32_t*)(uintptr_t)Handle + WordOffset) = Value;

}

/*----------------------------------------------------------------------------
 * DMAResource_CheckAndRegister
 */
int
DMAResource_CheckAndRegister(
        const DMAResource_Properties_t RequestedProperties,
        // const DMAResource_AddrPair_t AddrPair,
        const char AllocatorRef,
        DMAResource_Handle_t * const Handle_p)
{
    return 0;
    
}

/*----------------------------------------------------------------------------
 * DMAResource_IsValidHandle
 *
 * This function tells whether a handle is valid.
 *
 * Handle
 *     A valid handle that was once returned by DMAResource_CreateRecord or
 *     one of the DMA Buffer Management functions (Alloc/Register/Attach).
 *
 * Return Value
 *     true   The handle is valid
 *     false  The handle is NOT valid
 */
bool
DMAResource_IsValidHandle(
        const DMAResource_Handle_t Handle)
{
    if (Handle) {
        return true;
    }
    return false;
}
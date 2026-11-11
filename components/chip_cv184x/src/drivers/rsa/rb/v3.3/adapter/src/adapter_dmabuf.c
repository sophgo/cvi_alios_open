/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */
/* adapter_dmabuf.c
 *
 * Implementation of the DMA Buffer Allocation API.
 */



/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

// DMABuf API
#include "api_dmabuf.h"

// Adapter DMABuf internal API
#include "adapter_dmabuf.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework DMAResource API
#include "dmares_types.h"
#include "dmares_buf.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"

// Driver Framework C Run-Time Library API
#include "clib.h" // memcmp

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

/*----------------------------------------------------------------------------
 * Local variables
 */

/*----------------------------------------------------------------------------
 * DMABuf_NULLHandle
 *
 */
const DMABuf_Handle_t DMABuf_NULLHandle = {NULL};

// Initial DMA buffer alignment setting
static int Adapter_DMABuf_Alignment = ADAPTER_DMABUF_ALIGNMENT_INVALID;

/*----------------------------------------------------------------------------
 * Adapter_DMAResource_Alignment_Set
 */
void Adapter_DMAResource_Alignment_Set(
    const int Alignment)
{
    Adapter_DMABuf_Alignment = Alignment;

    return;
}

/*----------------------------------------------------------------------------
 * Adapter_DMAResource_Alignment_Get
 */
int Adapter_DMAResource_Alignment_Get(void)
{
    return Adapter_DMABuf_Alignment;
}

/*----------------------------------------------------------------------------
 * DMABuf_Handle_IsSame
 */
bool DMABuf_Handle_IsSame(
    const DMABuf_Handle_t *const Handle1_p,
    const DMABuf_Handle_t *const Handle2_p)
{
    if (memcmp(Handle1_p, Handle2_p, sizeof(DMABuf_Handle_t)) == 0)
    {
        return true;
    }

    return false;
}

/*----------------------------------------------------------------------------
 * DMABuf_Alloc
 */
DMABuf_Status_t
DMABuf_Alloc(
    const DMABuf_Properties_t RequestedProperties,
    DMABuf_HostAddress_t *const Buffer_p,
    DMABuf_Handle_t *const Handle_p)
{
    if (Handle_p == NULL ||
        Buffer_p == NULL)
    {
        return DMABUF_ERROR_BAD_ARGUMENT;
    }

    // initialize the output parameters
    Handle_p->p = Buffer_p->p;
    return DMABUF_STATUS_OK;
}
/*----------------------------------------------------------------------------
 * DMABuf_Register
 */
DMABuf_Status_t
DMABuf_Register(
    const DMABuf_Properties_t RequestedProperties,
    void *Buffer_p,
    void *Alternative_p,
    const char AllocatorRef,
    DMABuf_Handle_t *const Handle_p)
{
    if (Handle_p == NULL ||
        Buffer_p == NULL)
    {
        return DMABUF_ERROR_BAD_ARGUMENT;
    }

    // initialize the output parameter
    Handle_p->p = Buffer_p;
    Handle_p->size = RequestedProperties.Size;

    return DMABUF_STATUS_OK;
}

/*----------------------------------------------------------------------------
 * DMABuf_Release
 */
DMABuf_Status_t
DMABuf_Release(
    DMABuf_Handle_t Handle)
{
    return DMABUF_STATUS_OK;
}

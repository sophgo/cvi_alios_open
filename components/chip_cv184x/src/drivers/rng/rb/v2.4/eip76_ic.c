/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/* eip76_ic.c
 *
 * Module implements the EIP-76 Driver Library Interrupt Control API
 */


/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

// EIP-76 Driver Library Read Random Number API
#include "eip76_ic.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint32_t

// EIP-76 Driver Library Types API
#include "eip76_types.h"        // EIP76_* types

// EIP-76 Driver Library Internal interfaces
#include "eip76_level0.h"       // Level 0 macros
#include "eip76_internal.h"     // Internal macros

/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * EIP76_INT_EnableSources
 *
 */
EIP76_Status_t
EIP76_INT_EnableSources(
        EIP76_IOArea_t * const IOArea_p,
        const EIP76_INT_SourceBitmap_t IntSources)
{
    Device_Handle_t Device;
    uint32_t RegVal;
    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP76_CHECK_POINTER(IOArea_p);

    EIP76_CHECK_INT_INRANGE(IntSources, 1, EIP76_EVENTS_MASK);

    Device = TrueIOArea_p->Device;

    RegVal = EIP76_CONTROL_RD(Device);

    RegVal |= IntSources;

    EIP76_CONTROL_WR(Device, RegVal);

    return EIP76_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP76_INT_DisableSources
 *
 */
EIP76_Status_t
EIP76_INT_DisableSources(
        EIP76_IOArea_t * const IOArea_p,
        const EIP76_INT_SourceBitmap_t IntSources)
{
    Device_Handle_t Device;
    uint32_t RegVal;
    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP76_CHECK_POINTER(IOArea_p);

    EIP76_CHECK_INT_INRANGE(IntSources, 1, EIP76_EVENTS_MASK);

    Device = TrueIOArea_p->Device;

    RegVal = EIP76_CONTROL_RD(Device);

    RegVal &= (~IntSources);

    EIP76_CONTROL_WR(Device, RegVal);

    return EIP76_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP76_INT_IsActive
 *
 */
EIP76_Status_t
EIP76_INT_IsActive(
        EIP76_IOArea_t * const IOArea_p,
        EIP76_INT_SourceBitmap_t * const PendingIntSources_p)
{
    Device_Handle_t Device;
    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP76_CHECK_POINTER(IOArea_p);

    EIP76_CHECK_POINTER(PendingIntSources_p);

    Device = TrueIOArea_p->Device;

    *PendingIntSources_p = EIP76_STATUS_RD(Device) & EIP76_EVENTS_MASK;

    return EIP76_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP76_INT_Acknowledge
 *
 */
EIP76_Status_t
EIP76_INT_Acknowledge(
        EIP76_IOArea_t * const IOArea_p,
        const EIP76_INT_SourceBitmap_t IntSources)
{
    Device_Handle_t Device;
    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP76_CHECK_POINTER(IOArea_p);

    EIP76_CHECK_INT_INRANGE(IntSources, 1, EIP76_EVENTS_MASK);

    Device = TrueIOArea_p->Device;

    EIP76_INTACK_WR(Device, IntSources);

    return EIP76_NO_ERROR;
}


/* end of file eip76_ic.c */



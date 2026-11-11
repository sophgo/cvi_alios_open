/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/* eip76_nopp.c
 *
 * Module implements the stub for the Post Processor interface
 */

#ifdef NOPP_DEBUG

/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

// EIP-76 External Post Processor Interface
#include "eip76_pp.h"

// EIP-76 Internal Post Processor Interface
#include "eip76_internal_pp.h"  // EIP76_Internal_PostProcessor_*


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint32_t

// Driver Framework Device API
#include "device_types.h"       // Device_Handle_t

// EIP-76 Driver Library Types API
#include "eip76_types.h"        // EIP76_* types

// EIP-76 Driver Library Internal interfaces
#include "eip76_level0.h"       // Level 0 macros


/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * EIP76_Internal_PostProcessor_PS_AI_Write
 *
 */
void
EIP76_Internal_PostProcessor_PS_AI_Write(
        const Device_Handle_t Device,
        const uint32_t * PS_AI_Data_p,
        const unsigned int PS_AI_WordCount)
{
    IDENTIFIER_NOT_USED(Device);
    IDENTIFIER_NOT_USED(PS_AI_Data_p);
    IDENTIFIER_NOT_USED((bool)PS_AI_WordCount);

    return;
}


/*----------------------------------------------------------------------------
 * EIP76_PostProcessor_BlockCount_Get
 *
 */
EIP76_Status_t
EIP76_PostProcessor_BlockCount_Get(
        EIP76_IOArea_t * const IOArea_p,
        uint32_t * const BlockCount_p)
{
    IDENTIFIER_NOT_USED(IOArea_p);
    IDENTIFIER_NOT_USED(BlockCount_p);

    return EIP76_UNSUPPORTED_FEATURE_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP76_PostProcessor_IsBusy
 *
 */
EIP76_Status_t
EIP76_PostProcessor_IsBusy(
        EIP76_IOArea_t * const IOArea_p,
        EIP76_EventStatus_t * const Events_p)
{
    IDENTIFIER_NOT_USED(IOArea_p);
    IDENTIFIER_NOT_USED(Events_p);

    return EIP76_UNSUPPORTED_FEATURE_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP76_PostProcessor_Reseed_Start
 *
 */
EIP76_Status_t
EIP76_PostProcessor_Reseed_Start(
        EIP76_IOArea_t * const IOArea_p,
        EIP76_EventStatus_t * const Events_p)
{
    IDENTIFIER_NOT_USED(IOArea_p);
    IDENTIFIER_NOT_USED(Events_p);

    return EIP76_UNSUPPORTED_FEATURE_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP76_PostProcessor_Reseed_Write
 *
 */
EIP76_Status_t
EIP76_PostProcessor_Reseed_Write(
        EIP76_IOArea_t * const IOArea_p,
        const uint32_t * PS_AI_Data_p,
        const unsigned int PS_AI_WordCount,
        EIP76_EventStatus_t * const Events_p)
{
    IDENTIFIER_NOT_USED(IOArea_p);
    IDENTIFIER_NOT_USED(PS_AI_Data_p);
    IDENTIFIER_NOT_USED((bool)PS_AI_WordCount);
    IDENTIFIER_NOT_USED(Events_p);

    return EIP76_UNSUPPORTED_FEATURE_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP76_PostProcessor_NIST_Write
 *
 */
EIP76_Status_t
EIP76_PostProcessor_NIST_Write(
        EIP76_IOArea_t * const IOArea_p,
        const uint32_t * PS_AI_Data_p,
        const unsigned int PS_AI_WordCount,
        const unsigned int VectorType,
        EIP76_EventStatus_t * const Events_p)
{
    IDENTIFIER_NOT_USED(IOArea_p);
    IDENTIFIER_NOT_USED(PS_AI_Data_p);
    IDENTIFIER_NOT_USED((bool)PS_AI_WordCount);
    IDENTIFIER_NOT_USED((bool)VectorType);
    IDENTIFIER_NOT_USED(Events_p);

    return EIP76_UNSUPPORTED_FEATURE_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP76_PostProcessor_PS_AI_Write
 *
 */
EIP76_Status_t
EIP76_PostProcessor_PS_AI_Write(
        EIP76_IOArea_t * const IOArea_p,
        const uint32_t * PS_AI_Data_p,
        const unsigned int PS_AI_WordCount,
        EIP76_EventStatus_t * const Events_p)
{
    IDENTIFIER_NOT_USED(IOArea_p);
    IDENTIFIER_NOT_USED(PS_AI_Data_p);
    IDENTIFIER_NOT_USED((bool)PS_AI_WordCount);
    IDENTIFIER_NOT_USED(Events_p);

    return EIP76_UNSUPPORTED_FEATURE_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP76_PostProcessor_Key_Write
 *
 */
EIP76_Status_t
EIP76_PostProcessor_Key_Write(
        EIP76_IOArea_t * const IOArea_p,
        const uint32_t * Key_Data_p)
{
    IDENTIFIER_NOT_USED(IOArea_p);
    IDENTIFIER_NOT_USED(Key_Data_p);

    return EIP76_UNSUPPORTED_FEATURE_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP76_PostProcessor_V_Write
 *
 */
EIP76_Status_t
EIP76_PostProcessor_V_Write(
        EIP76_IOArea_t * const IOArea_p,
        const uint32_t * V_Data_p)
{
    IDENTIFIER_NOT_USED(IOArea_p);
    IDENTIFIER_NOT_USED(V_Data_p);

    return EIP76_UNSUPPORTED_FEATURE_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP76_PostProcessor_Input_Write
 *
 */
EIP76_Status_t
EIP76_PostProcessor_Input_Write(
        EIP76_IOArea_t * const IOArea_p,
        const uint32_t * Input_Data_p,
        EIP76_EventStatus_t * const Events_p)
{
    IDENTIFIER_NOT_USED(IOArea_p);
    IDENTIFIER_NOT_USED(Input_Data_p);
    IDENTIFIER_NOT_USED(Events_p);

    return EIP76_UNSUPPORTED_FEATURE_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP76_PostProcessor_Result_Read
 *
 * This function can also be used for the SP 800-90 Post Processor to read
 * 1) result of the AES-256 Core known-answer test
 * 2) result of the NIST known-answer test on the complete Post Processor
 */
EIP76_Status_t
EIP76_PostProcessor_Result_Read(
        EIP76_IOArea_t * const IOArea_p,
        uint32_t * Output_Data_p,
        EIP76_EventStatus_t * const Events_p)
{
    IDENTIFIER_NOT_USED(IOArea_p);
    IDENTIFIER_NOT_USED(Output_Data_p);
    IDENTIFIER_NOT_USED(Events_p);

    return EIP76_UNSUPPORTED_FEATURE_ERROR;
}


/* end of file eip76_nopp.c */
#endif

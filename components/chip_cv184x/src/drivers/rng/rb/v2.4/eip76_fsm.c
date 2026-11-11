/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/* eip76_fsm.c
 *
 * Module implements the EIP-76 Driver Library State Machine Internal interface
 */


/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#ifdef TRNG_FSM_ENABLE
#include "eip76_fsm.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// EIP-76 Driver Library Types API
#include "eip76_types.h"        // EIP76_* types

#include "c_eip76.h"
/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * EIP76_State_Set
 *
 */
EIP76_Status_t
EIP76_State_Set(
        volatile EIP76_State_t * const CurrentState,
        const EIP76_State_t NewState)
{
    switch (*CurrentState)
    {
    case EIP76_STATE_RESET:
        switch (NewState)
        {
        case EIP76_STATE_RANDOM_GENERATING:
        case EIP76_STATE_RESET_BCDF:
            *CurrentState = NewState;
            break;
        default:
            break;
        }
        break;

    case EIP76_STATE_RESET_BCDF:
        switch (NewState)
        {
        case EIP76_STATE_RANDOM_GENERATING:
            *CurrentState = NewState;
            break;
        default:
            break;
        }
        break;

    case EIP76_STATE_RANDOM_GENERATING:
        switch (NewState)
        {
        case EIP76_STATE_RESET:
        case EIP76_STATE_RANDOM_READY:
        case EIP76_STATE_RANDOM_READING:
        case EIP76_STATE_SP80090_RESEED_START:
        case EIP76_STATE_KAT_START:
        case EIP76_STATE_RANDOM_GENERATING:
        case EIP76_STATE_SP80090_RESEED_WRITING:
            *CurrentState = NewState;
            break;
        default:
            break;
        }
        break;

    case EIP76_STATE_RANDOM_READY:
        switch (NewState)
        {
        case EIP76_STATE_RESET:
        case EIP76_STATE_RANDOM_READING:
        case EIP76_STATE_SP80090_RESEED_START:
        case EIP76_STATE_KAT_START:
        case EIP76_STATE_RANDOM_READY:
            *CurrentState = NewState;
            break;
        default:
            break;
        }
        break;

    case EIP76_STATE_RANDOM_READING:
        switch (NewState)
        {
        case EIP76_STATE_RANDOM_GENERATING:
        case EIP76_STATE_RANDOM_READING:
            *CurrentState = NewState;
            break;
        default:
            break;
        }
        break;

#if (EIP76_POST_PROCESSOR_TYPE == EIP76_POST_PROCESSOR_SP800_90) ||  \
    (EIP76_POST_PROCESSOR_TYPE == EIP76_POST_PROCESSOR_BC_DF)
    case EIP76_STATE_SP80090_RESEED_START:
        switch (NewState)
        {
        case EIP76_STATE_RESET:
        case EIP76_STATE_SP80090_RESEED_READY:
        case EIP76_STATE_SP80090_RESEED_START:
        case EIP76_STATE_RANDOM_GENERATING:
            *CurrentState = NewState;
            break;
        default:
            break;
        }
        break;

    case EIP76_STATE_SP80090_RESEED_READY:
        switch (NewState)
        {
        case EIP76_STATE_RESET:
        case EIP76_STATE_RANDOM_GENERATING:
        case EIP76_STATE_SP80090_RESEED_READY:
        case EIP76_STATE_SP80090_RESEED_WRITING:
            *CurrentState = NewState;
            break;
        default:
            break;
        }
        break;

    case EIP76_STATE_SP80090_RESEED_WRITING:
        switch (NewState)
        {
        case EIP76_STATE_RESET:
        case EIP76_STATE_RANDOM_GENERATING:
            *CurrentState = NewState;
            break;
        default:
            break;
        }
        break;
#endif

    case EIP76_STATE_KAT_START:
        switch (NewState)
        {
        case EIP76_STATE_RESET:
        case EIP76_STATE_RANDOM_GENERATING:
        case EIP76_STATE_KAT_READY:
        case EIP76_STATE_KAT_START:
            *CurrentState = NewState;
            break;
        default:
            break;
        }
        break;

    case EIP76_STATE_KAT_READY:
        switch (NewState)
        {
        case EIP76_STATE_RESET:
        case EIP76_STATE_RANDOM_GENERATING:
        case EIP76_STATE_KAT_PRM_PROCESSING:
        case EIP76_STATE_KAT_SP80090_PROCESSING:
        case EIP76_STATE_KAT_CF_PROCESSING:
        case EIP76_STATE_KAT_REP_PROCESSING:
        case EIP76_STATE_KAT_ADAP_PROCESSING:
        case EIP76_STATE_KAT_SP80090_BCDF_RESEEDED:
        case EIP76_STATE_KAT_READY:
            *CurrentState = NewState;
            break;
        default:
            break;
        }
        break;

    case EIP76_STATE_KAT_PRM_PROCESSING:
        switch (NewState)
        {
        case EIP76_STATE_RESET:
        case EIP76_STATE_RANDOM_GENERATING:
        case EIP76_STATE_KAT_READY:
        case EIP76_STATE_KAT_PRM_M_FAILED:
        case EIP76_STATE_KAT_PRM_DONE:
        case EIP76_STATE_KAT_PRM_PROCESSING:
            *CurrentState = NewState;
            break;
        default:
            break;
        }
        break;


    case EIP76_STATE_KAT_CF_PROCESSING:
        switch (NewState)
        {
        case EIP76_STATE_RESET:
        case EIP76_STATE_KAT_READY:
        case EIP76_STATE_KAT_CF_PROCESSING:
        case EIP76_STATE_KAT_CF_DONE:
            *CurrentState = NewState;
            break;
        default:
            break;
        }
        break;

    case EIP76_STATE_KAT_REP_PROCESSING:

        switch (NewState)
        {
        case EIP76_STATE_RESET:
        case EIP76_STATE_KAT_READY:
        case EIP76_STATE_KAT_REP_PROCESSING:
            *CurrentState = NewState;
            break;
        default:
            break;
        }
        break;

    case EIP76_STATE_KAT_ADAP_PROCESSING:

        switch (NewState)
        {
        case EIP76_STATE_RESET:
        case EIP76_STATE_KAT_READY:
        case EIP76_STATE_KAT_ADAP_PROCESSING:
            *CurrentState = NewState;
            break;
        default:
            break;
        }
        break;

    case EIP76_STATE_KAT_CF_DONE:
        switch (NewState)
        {
        case EIP76_STATE_RESET:
        case EIP76_STATE_KAT_CF_PROCESSING:
        case EIP76_STATE_RANDOM_GENERATING:
            *CurrentState = NewState;
            break;
        default:
            break;
        }
        break;

    case EIP76_STATE_KAT_PRM_M_FAILED:
        switch (NewState)
        {
        case EIP76_STATE_RESET:
        case EIP76_STATE_RANDOM_GENERATING:
        case EIP76_STATE_KAT_PRM_M_FAILED:
            *CurrentState = NewState;
            break;
        default:
            break;
        }
        break;

    case EIP76_STATE_KAT_PRM_DONE:
        switch (NewState)
        {
        case EIP76_STATE_RESET:
        case EIP76_STATE_RANDOM_GENERATING:
        case EIP76_STATE_KAT_PRM_DONE:
            *CurrentState = NewState;
            break;
        default:
            break;
        }
        break;

#if (EIP76_POST_PROCESSOR_TYPE == EIP76_POST_PROCESSOR_SP800_90) ||  \
    (EIP76_POST_PROCESSOR_TYPE == EIP76_POST_PROCESSOR_BC_DF)
    case EIP76_STATE_KAT_SP80090_PROCESSING:
        switch (NewState)
        {
        case EIP76_STATE_RESET:
        case EIP76_STATE_RANDOM_GENERATING:
        case EIP76_STATE_KAT_READY:
            *CurrentState = NewState;
            break;
        default:
            break;
        }
        break;
#endif

#if (EIP76_POST_PROCESSOR_TYPE == EIP76_POST_PROCESSOR_BC_DF)
    case EIP76_STATE_KAT_SP80090_BCDF_RESEEDED:
        switch (NewState)
        {
        case EIP76_STATE_RESET:
        case EIP76_STATE_KAT_SP80090_BCDF_NOISE:
        case EIP76_STATE_SP80090_RESEED_WRITING:
            *CurrentState = NewState;
            break;
        default:
            break;
        }
        break;

    case EIP76_STATE_KAT_SP80090_BCDF_NOISE:
        switch (NewState)
        {
        case EIP76_STATE_RESET:
        case EIP76_STATE_KAT_SP80090_BCDF_READY:
        case EIP76_STATE_KAT_SP80090_BCDF_RESEEDED:
        case EIP76_STATE_RANDOM_GENERATING:
            *CurrentState = NewState;
            break;
        default:
            break;
        }
        break;

    case EIP76_STATE_KAT_SP80090_BCDF_READY:
        switch (NewState)
        {
        case EIP76_STATE_RESET:
        case EIP76_STATE_KAT_SP80090_PROCESSING:
        case EIP76_STATE_KAT_SP80090_BCDF_PROCESSING:
        case EIP76_STATE_RANDOM_GENERATING:
            *CurrentState = NewState;
            break;
        default:
            break;
        }
        break;

    case EIP76_STATE_KAT_SP80090_BCDF_PROCESSING:
        switch (NewState)
        {
        case EIP76_STATE_RESET:
        case EIP76_STATE_KAT_START:
        case EIP76_STATE_RANDOM_GENERATING:
            *CurrentState = NewState;
            break;
        default:
            break;
        }
        break;
#endif

    default:
        break;
    }

    if (*CurrentState != NewState)
    {
        return EIP76_ILLEGAL_IN_STATE;
    }
    return EIP76_NO_ERROR;
}
#endif /*TRNG_FSM_ENABLE*/
/* end of file eip76_fsm.c */

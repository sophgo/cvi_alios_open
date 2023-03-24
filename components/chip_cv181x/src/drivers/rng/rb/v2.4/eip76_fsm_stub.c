/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/* eip76_fsm_stub.c
 *
 * Stub module for the EIP-76 Driver Library State Machine Internal interface
 */


/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include "eip76_fsm.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Driver Framework Basic Definitions API
//#include "basic_defs.h"         // uint32_t

// EIP-76 Driver Library Types API
#include "eip76_types.h"        // EIP76_* types

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
    IDENTIFIER_NOT_USED(CurrentState);
    IDENTIFIER_NOT_USED(NewState);

    return EIP76_NO_ERROR;
}


/* end of file eip76_fsm_stub.c */

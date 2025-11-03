/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/* eip76_random.c
 *
 * Module implements the EIP-76 Driver Library Read Random Number API
 */


/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

// EIP-76 Driver Library Read Random Number API
#include "eip76_read.h"

/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip76.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint32_t

// Driver Framework Device API
#include "device_types.h"       // Device_Handle_t

// EIP-76 Driver Library Types API
#include "eip76_types.h"        // EIP76_* types

// EIP-76 Driver Library Internal interfaces
#include "eip76_level0.h"       // Level 0 macros
#include "eip76_fsm.h"          // State machine
#include "eip76_internal.h"     // Internal macros

/*----------------------------------------------------------------------------
 * Definitions and macros
 */


/*----------------------------------------------------------------------------
 * EIP76_Request_Random_Data
 *
 */
EIP76_Status_t
EIP76_Request_Random_Data(
        EIP76_IOArea_t * const IOArea_p,
        unsigned int NumberOfBytes)
{
    Device_Handle_t Device;
    uint32_t ControlValue, StatusValue, NumberOfBlks, WriteValue;

    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP76_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    ControlValue = EIP76_CONTROL_RD(Device);

    // First check if number of data_bloacks is zero, else return
    if ((ControlValue >> 20) & MASK_12_BITS)
        return EIP76_BUSY_RETRY_LATER;

    StatusValue =  EIP76_STATUS_RD(Device);

    // Count available data blocks
    NumberOfBlks = (StatusValue & MASK_1_BIT) +
                   ((StatusValue >> 16) & MASK_8_BITS);
    if (NumberOfBlks * 16 < NumberOfBytes)
    {
        NumberOfBlks = (NumberOfBytes + 15) / 16; // Round up

        // Reduce blocks by the number of blocks already available
        NumberOfBlks -= ((StatusValue & MASK_1_BIT) +
                        ((StatusValue >> 16) & MASK_8_BITS));

        EIP76_CHECK_INT_ATMOST(NumberOfBlks, EIP76_REQUEST_DATA_MAX_BLK_COUNT);

        // Only data_blocks field is updated in register.
        WriteValue = EIP76_REQUEST_DATA |
                      ((NumberOfBlks & MASK_12_BITS) << 20);
        EIP76_CONTROL_WR(Device, WriteValue);

#ifdef EIP76_BLOCKS_THRESHOLD_OPTION
        // Do not write Threshold when in test mode
        if (!(ControlValue & EIP76_ENABLE_TEST_MODE))
        {
            WriteValue = EIP76_BUFFER_SIZE_RD(Device);

            if (WriteValue > 0)
            {
                NumberOfBlks +=  ((StatusValue & MASK_1_BIT) +
                                 ((StatusValue >> 16) & MASK_8_BITS));
                if ( NumberOfBlks > WriteValue)
                {
                    NumberOfBlks = WriteValue;
                }
                // Set threshold value one less then the needed number of blocks
                WriteValue = NumberOfBlks - 1;

                // Set the number of blocks in threshold register
                EIP76_BLOCKS_THRESHOLD_WR(Device, WriteValue);
            }
        }
#endif // EIP76_BLOCKS_THRESHOLD_OPTION
    }

    return EIP76_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP76Lib_PostProcessor_Random_Read
 *
 */
static EIP76_Status_t
EIP76Lib_Random_Read(
        const Device_Handle_t Device,
        EIP76_Random128_t * Random128_p)
{
#if EIP76_READ_TIMEOUT > 0
    EIP76_INTACK_WR(Device, 0);
    Random128_p->word[0] = EIP76_OUTPUT_0_RD(Device);
    Random128_p->word[1] = EIP76_OUTPUT_1_RD(Device);
    Random128_p->word[2] = EIP76_OUTPUT_2_RD(Device);
    Random128_p->word[3] = EIP76_OUTPUT_3_RD(Device);
#else
    Random128_p->word[0] = EIP76_OUTPUT_0_RD(Device);
    Random128_p->word[1] = EIP76_OUTPUT_1_RD(Device);
    Random128_p->word[2] = EIP76_OUTPUT_2_RD(Device);
    Random128_p->word[3] = EIP76_OUTPUT_3_RD(Device);

	LOG_INFO("8 %s, %x\n", __FUNCTION__, Random128_p->word[0]);
	LOG_INFO("8 %s, %x\n", __FUNCTION__, Random128_p->word[1]);
	LOG_INFO("8 %s, %x\n", __FUNCTION__, Random128_p->word[2]);
	LOG_INFO("8 %s, %x\n", __FUNCTION__, Random128_p->word[3]);
#endif // EIP76_READ_TIMEOUT

    return EIP76_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP76_Random_IsBusy
 *
 */
EIP76_Status_t
EIP76_Random_IsBusy(
        EIP76_IOArea_t * const IOArea_p,
        EIP76_EventStatus_t * const Events_p)
{
    Device_Handle_t Device;
    uint32_t StatusRegVal;
    EIP76_Status_t rv = EIP76_NO_ERROR;
    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP76_CHECK_POINTER(Events_p);

    // No events detected yet
    *Events_p = 0;

    EIP76_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    StatusRegVal = EIP76_STATUS_RD(Device);

    // Store event status with the available random data 128-bit block count
    *Events_p = StatusRegVal;

    if ( EIP76_STATUS_IS_READY(StatusRegVal))
    {
	    LOG_INFO("6 %s, %d\n", __FUNCTION__, __LINE__);
        // Transit to a new state
        rv = EIP76_State_Set((volatile EIP76_State_t*)&TrueIOArea_p->State,
                             EIP76_STATE_RANDOM_READY);
    }
    else
    {
        // Keep the state, no random data ready yet
        rv = EIP76_BUSY_RETRY_LATER;
    }
    return rv;
}


/*----------------------------------------------------------------------------
 * EIP76_Random_Get
 *
 */
EIP76_Status_t
EIP76_Random_Get(
        EIP76_IOArea_t * const IOArea_p,
        EIP76_Random128_t * Data_p,
        const unsigned int NumberCount,
        unsigned int * const GeneratedNumberCount_p,
        EIP76_EventStatus_t * const Events_p)
{
    uint32_t StatusRegVal;
    unsigned int i;
    Device_Handle_t Device;
    EIP76_Status_t rv = EIP76_NO_ERROR;
    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP76_CHECK_POINTER(Events_p);

    // No events detected yet
    *Events_p = 0;

    EIP76_CHECK_POINTER(GeneratedNumberCount_p);

    *GeneratedNumberCount_p = 0;

    EIP76_CHECK_POINTER(IOArea_p);

    EIP76_CHECK_POINTER(Data_p);

    EIP76_CHECK_INT_ATLEAST(NumberCount, 1);

    Device = TrueIOArea_p->Device;

    for (i = 0; i < NumberCount; i++)
    {
        StatusRegVal = EIP76_STATUS_RD(Device);

        // Store event status with the available random data 128-bit block count
        *Events_p = StatusRegVal;

        if (!EIP76_STATUS_IS_READY(StatusRegVal) )
        {
            if (i == 0)
            {
                // Keep the state, no random data ready yet
                return EIP76_ILLEGAL_IN_STATE;  // May never occur
            }
            else
            {
                // Transit to a new state
                rv = EIP76_State_Set((volatile EIP76_State_t* const)&TrueIOArea_p->State,
                                      EIP76_STATE_RANDOM_GENERATING);

                if (rv != EIP76_NO_ERROR)
                {
                    return EIP76_ILLEGAL_IN_STATE; // May never occur
                }
                else
                {
                    *GeneratedNumberCount_p = i;
                    return EIP76_BUSY_RETRY_LATER;
                }
            }
        }
        else
        {
            // Random data is ready, transit to a new state
            rv = EIP76_State_Set((volatile EIP76_State_t* const)&TrueIOArea_p->State,
                                 EIP76_STATE_RANDOM_READING);

            if (rv != EIP76_NO_ERROR)
            {
                return EIP76_ILLEGAL_IN_STATE;
            }
        }

        // In the EIP76_STATE_RANDOM_READING state now, random data is ready

#ifdef EIP76_BLOCKS_THRESHOLD_OPTION
        // Set Theshold back to 0 to get all data
        EIP76_BLOCKS_THRESHOLD_WR(Device, 0);
#endif // EIP76_BLOCKS_THRESHOLD_OPTION

        // Read the random number
        rv = EIP76Lib_Random_Read(Device, Data_p++);

        if (rv != EIP76_NO_ERROR)
        {
            // Failed to read a random number, transit to a new state
            rv = EIP76_State_Set((volatile EIP76_State_t*)&TrueIOArea_p->State,
                                  EIP76_STATE_RANDOM_GENERATING);

            if (rv != EIP76_NO_ERROR)
            {
                return EIP76_ILLEGAL_IN_STATE;    // May never occur
            }
            else
            {
                return EIP76_RANDOM_READ_ERROR;
            }
        }

        // Acknowledge the read number
	    LOG_INFO("8 %s, %d\n", __FUNCTION__, __LINE__);
        EIP76_INTACK_WR(Device, EIP76_STATUS_READY);
    } // for

    *GeneratedNumberCount_p = i;

    // Transit to a new state
    rv = EIP76_State_Set((volatile EIP76_State_t*)&TrueIOArea_p->State,
                          EIP76_STATE_RANDOM_GENERATING);

    return rv;
}


/*----------------------------------------------------------------------------
 * EIP76_Alarm_Handle
 *
 */
EIP76_Status_t
EIP76_Alarm_Handle(
        EIP76_IOArea_t * const IOArea_p,
        EIP76_EventStatus_t * const Events_p,
        uint32_t * FROAlarmMask_p)
{
    uint32_t RegVal;
    Device_Handle_t Device;
    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP76_CHECK_POINTER(IOArea_p);

    EIP76_CHECK_POINTER(Events_p);

    EIP76_CHECK_POINTER(FROAlarmMask_p);

    Device = TrueIOArea_p->Device;

    // No events detected yet
    *Events_p = 0;

    RegVal = EIP76_STATUS_RD(Device);

    // Store detected events
    *Events_p = (RegVal & EIP76_EVENTS_MASK);

#ifndef EIP76_AUTO_DETUNE_OPTION
    // Determine what FRO's are stopped
    *FROAlarmMask_p = EIP76_ALARMSTOP_RD(Device);
    RegVal = ((*FROAlarmMask_p) & ((uint32_t)EIP76_FRO_ENABLED_MASK));
    if( RegVal != 0 )
    {
        // Perform FRO de-tune operation for all the stopped FRO's,
        // this must be done before the FRO's are enabled.
        // This operations disables the FRO's
        EIP76_FRO_DETUNE(Device, RegVal);

        // Clear alarm mask
        EIP76_ALARMMASK_WR(Device, 0);

        // Clear alarm stop mask
        EIP76_ALARMSTOP_WR(Device, 0);

        // Enable the configured FRO's
        EIP76_FROENABLE_WR(Device, EIP76_FRO_ENABLED_MASK);
    }
#endif  // EIP76_AUTO_DETUNE_OPTION

    // Acknowledge the detected alarm events,
    // Data ready status bit is not an alarm event
    EIP76_INTACK_WR(Device, ((*Events_p) & (~EIP76_STATUS_READY)));

    return EIP76_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP76_FatalError_Handle
 *
 */
EIP76_Status_t
EIP76_FatalError_Handle(
        EIP76_IOArea_t * const IOArea_p)
{
    uint32_t RegVal;
    Device_Handle_t Device;
    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP76_CHECK_POINTER(IOArea_p);

    // EIP76_CHECK_POINTER(Events_p);

    Device = TrueIOArea_p->Device;

    RegVal = EIP76_STATUS_RD(Device);
	LOG_INFO("fatal %s, %d\n", __FUNCTION__, __LINE__);

    // Check if fatal events are present
    if ( (RegVal & (EIP76_STUCK_OUT_EVENT | EIP76_NOISE_FAIL_EVENT)) != 0 )
    {
        // Acknowledge all the fatal error events before disabling TRNG
        EIP76_INTACK_WR(Device,
                        (EIP76_STUCK_OUT_EVENT |
                         EIP76_NOISE_FAIL_EVENT |
                         EIP76_SHUTDOWN_OFLO_EVENT) );

        // Acknowledge all the fatal error events before disabling TRNG
                EIP76_INTACK_WR(Device,
                                (EIP76_STUCK_OUT_EVENT |
                                 EIP76_SHUTDOWN_OFLO_EVENT) );

        // Disable TRNG
        EIP76_CONTROL_WR(Device, 0);

        // Transit to a new state
        return EIP76_State_Set((volatile EIP76_State_t*)&TrueIOArea_p->State,
                               EIP76_STATE_RESET);
    }

// #if (EIP76_SHUTDOWN_FATAL == 1)
//     // Check if fatal events are present
//     if ( (RegVal & EIP76_SHUTDOWN_OFLO_EVENT ) != 0 )
//     {
//         // Acknowledge all the fatal error events before disabling TRNG
//         EIP76_INTACK_WR(Device,
//                         (EIP76_STUCK_OUT_EVENT |
//                          EIP76_NOISE_FAIL_EVENT |
//                          EIP76_SHUTDOWN_OFLO_EVENT) );

//         // Disable TRNG
//         EIP76_CONTROL_WR(Device, 0);

//         // Transit to a new state
//         return EIP76_State_Set((EIP76_State_t*)&TrueIOArea_p->State,
//                                EIP76_STATE_RESET);
//     }
// #endif

    return EIP76_NO_ERROR;
}


/* end of file eip76_random.c */

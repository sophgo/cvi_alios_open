/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */

/* eip76_test.c
 *
 * Module implements the EIP76 Driver Library Known-answer test (KAT) API
 */


/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

// EIP-76 Driver Library Known-answer test (KAT) API
#include "eip76_test.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default configuration
#include "c_eip76.h"

// Driver Framework Basic Definitions API
#include "basic_defs.h"         // uint32_t

// EIP-76 Driver Library Types API
#include "eip76_types.h"        // EIP76_* types

// EIP-76 Driver Library Internal interfaces
#include "eip76_level0.h"       // Level 0 macros
#include "eip76_internal.h"     // Internal macros
#include "eip76_fsm.h"          // State machine

/*----------------------------------------------------------------------------
 * Definitions and macros
 */

#define START_SHA_KAT_TEST               ((uint32_t)0x00008030)
#define TEST_SAMPLE_CYCLES               0x10000


/*----------------------------------------------------------------------------
 * EIP76Lib_Test_Is_Ready
 *
 */
static bool
EIP76Lib_Test_Is_Ready(
        const Device_Handle_t Device)
{
#ifdef EIP76_CONT_POKER_TEST_READY_CDS
    // Check if continuous Poker test is enabled,
    if ((EIP76_TEST_RD(Device) & EIP76_TEST_CONT_POKER) != 0 )
    {
        volatile uint32_t RunCntRegVal1, RunCntRegVal2;

        // Continuous Poker test is enabled,
        // Reading twice the same value from the TRNG_RUN_CNT:run_test_count
        // field means test is ready
        RunCntRegVal1 = EIP76_RUNCNT_RD(Device);
        RunCntRegVal2 = EIP76_RUNCNT_RD(Device);
        if( RunCntRegVal1 == RunCntRegVal2 )
            return true;
        else
            return false;
    }
    else
#endif // EIP76_CONT_POKER_TEST_READY_CDS
    {
        uint32_t Mask = EIP76_STATUS_TEST_READY;
        uint32_t StatusRegVal = EIP76_STATUS_RD(Device);

#if (EIP76_POST_PROCESSOR_TYPE == EIP76_POST_PROCESSOR_BC_DF)
        if ((EIP76_TEST_RD(Device) & EIP76_TEST_SP_800_90) != 0 )
        {
            // SP80090 BCDF test
            Mask |= EIP76_STATUS_RESEED_AI;
        }
#endif // EIP76_POST_PROCESSOR_BC_DF

        if((StatusRegVal & Mask) == Mask)
            return true;
        else
            return false;
    }
}


uint32_t
Device_SwapEndian(
        const uint32_t Value)
{
#ifdef DEVICE_SWAP_SAFE
    return (((Value & 0x000000FFU) << 24) |
            ((Value & 0x0000FF00U) <<  8) |
            ((Value & 0x00FF0000U) >>  8) |
            ((Value & 0xFF000000U) >> 24));
#else
    // reduces typically unneeded AND operations
    return ((Value << 24) |
            ((Value & 0x0000FF00U) <<  8) |
            ((Value & 0x00FF0000U) >>  8) |
            (Value >> 24));
#endif
}


/*----------------------------------------------------------------------------
 * EIP76_Test_Start
 *
 */
EIP76_Status_t
EIP76_Test_Start(
        EIP76_IOArea_t * const IOArea_p,
        const EIP76_Test_Type_t TestType,
        EIP76_EventStatus_t * const Events_p)
{
    Device_Handle_t Device;
    uint32_t RegVal;
    uint32_t ControlWord = 0;
    uint8_t SHAVersion;
    EIP76_Status_t rv;
    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP76_CHECK_POINTER(IOArea_p);

    EIP76_CHECK_POINTER(Events_p);

    Device = TrueIOArea_p->Device;

    RegVal = EIP76_STATUS_RD(Device);

    SHAVersion = EIP76_SHA_VERSION_RD(Device);

    // Store event status
    *Events_p = (RegVal & EIP76_EVENTS_MASK);

    // Save TRNG_CONTROL register (internal TRNG HW state)
    RegVal = EIP76_CONTROL_RD(Device);

    // Check if Post Processor is enabled
    if (TestType != EIP76_PRM_KAT_TYPE &&
        TestType != EIP76_SHA_KAT_TYPE &&
        TestType != EIP76_SHA_RANDOM &&
        TestType != EIP76_ADPRO_TYPE_64 &&
        TestType != EIP76_ADPRO_TYPE_4k &&
        TestType != EIP76_ADPRO_TYPE_512 &&
        TestType != EIP76_PRM_CONTINUOUS_KAT_TYPE &&
        TestType != EIP76_SP80090_AES_CORE_KAT_TYPE &&
        TestType != EIP76_REPCOUNT_TYPE &&
        (RegVal & EIP76_CONTROL_POSTPROCESSOR_ENABLE) == 0)
        // Post Processor KAT is requested while PP is disabled
        return EIP76_ILLEGAL_IN_STATE;

    TrueIOArea_p->SavedControl = RegVal;

    // Transit to a new state
    rv = EIP76_State_Set((volatile EIP76_State_t* const ) &TrueIOArea_p->State,
                         EIP76_STATE_KAT_START);
    if (rv != EIP76_NO_ERROR)
        return EIP76_ILLEGAL_IN_STATE; // May never occur

    // set Entropy generation configuration to default
    EIP76_CONFIG_WR(Device,
                    EIP76_NOISE_BLOCKS,
                    EIP76_SAMPLE_DIVIDER,
                    EIP76_READ_TIMEOUT,
                    EIP76_SAMPLE_CYCLES,
                    EIP76_SAMPLE_SCALE);

    // Enter Test Mode and disable fatal error interrupts
    RegVal |= EIP76_CONTROL_ENABLE_TEST_MODE;
    RegVal &= (~(EIP76_STUCK_OUT_EVENT | EIP76_NOISE_FAIL_EVENT));
#if (EIP76_SHUTDOWN_FATAL == 1)
    RegVal &= (~EIP76_SHUTDOWN_OFLO_EVENT);
#endif
    EIP76_CONTROL_WR(Device, RegVal);

    if (TestType == EIP76_PRM_KAT_TYPE ||
        TestType == EIP76_PRM_CONTINUOUS_KAT_TYPE)
    {
        // Read ALARMCNT register
        RegVal = EIP76_ALARMCNT_RD(Device);

        // Reset counters for test
        EIP76_COUNT_WR(Device, RESET_COUNTERS);

        // Clear any test related bits in the INTACK register
        EIP76_INTACK_WR(Device, EIP76_INTACK_CLEAR_PRM);

        // PRM test is requested, set stall_run_poker in TRNG_ALARMCNT so
        // that in case test fails its result can be inspected and is not
        // overwritten
        RegVal |= EIP76_ALARMCNT_STALL_RUN_POKER;

        EIP76_ALARMCNT_WR(Device, RegVal);

    }

    // Start KAT
    if (TestType == EIP76_PRM_KAT_TYPE)
    {
        // PRM test
        EIP76_TEST_WR(Device, EIP76_TEST_RUN_POKER);
    }
    else if (TestType == EIP76_PRM_CONTINUOUS_KAT_TYPE)
    {
        // PRM Continuous test
        EIP76_TEST_WR(Device, EIP76_TEST_RUN_POKER | EIP76_TEST_CONT_POKER);
    }
    else if (TestType == EIP76_SHA_KAT_TYPE)
    {
        TrueIOArea_p->ConfigStatus = EIP76_CONFIG_RD(Device);

        // SHA test, restart hash core. This is needed to write the noise_blocks
        // field
        EIP76_TEST_WR(Device, EIP76_TEST_SHA);

        if (SHAVersion == 2) {
            // 50 512 bit blocks for SHA2 KAT test
            ControlWord |= EIP76_SHA2_NOISE_BLOCKS;
        } else {
            // 44 512 bit blocks for SHA1 KAT test
            ControlWord |= EIP76_SHA1_NOISE_BLOCKS;
        }

        // Configure the noise_block field for test
        EIP76_CONFIG_NOISEBLK_WR(Device, ControlWord);

        // Disable hash core
        EIP76_TEST_WR(Device, 0);

        // Enable known noise test mode, conditioner test and set cont_poker
        // so that the PR module doesn't stall the test.
        EIP76_TEST_WR(Device, START_SHA_KAT_TEST);

        ControlWord |= TEST_SAMPLE_CYCLES;

        // Configure the noise_block field for test
        EIP76_CONFIG_NOISEBLK_WR(Device, ControlWord);
    }
    else if (TestType == EIP76_SHA_RANDOM)
    {
        // Ack any related events
        EIP76_INTACK_WR(Device, 0x1);

        TrueIOArea_p->ConfigStatus = EIP76_CONFIG_RD(Device);

        // SHA test, restart hash core. This is needed to write the noise_blocks
        // field
        EIP76_TEST_WR(Device, EIP76_TEST_SHA);

        ControlWord |= EIP76_SHA_RANDOM_NOISE_BLOCKS;

        // Configure the noise_block field for test
        EIP76_CONFIG_NOISEBLK_WR(Device, ControlWord);

        // Disable hash core
        EIP76_TEST_WR(Device, 0);

        // Enable known noise test mode, conditioner test and set cont_poker
        // so that the PR module doesn't stall the test.
        EIP76_TEST_WR(Device, START_SHA_KAT_TEST);

        ControlWord |= TEST_SAMPLE_CYCLES;

        // Configure the noise_block field for test
        EIP76_CONFIG_NOISEBLK_WR(Device, ControlWord);
    }
    else if (TestType == EIP76_REPCOUNT_TYPE)
    {
        TrueIOArea_p->ConfigStatus = EIP76_CONFIG_RD(Device);

        // Set cont_poker, test_known_noise and test_spb
        EIP76_TEST_WR(Device, EIP76_TEST_REPCOUNTORADAPTIVE);

        ControlWord |= TEST_SAMPLE_CYCLES;

        // Configure the noise_block field for test
        EIP76_CONFIG_NOISEBLK_WR(Device, ControlWord);

        // Reset the counters through INTACK register
        EIP76_INTACK_WR(Device, EIP76_REPCNT_FAIL_EVENT | EIP76_APROP_FAIL_EVENT);

        // Set cut-off value for repetition test
        // This is the maximum value of the repetition counter 63.
        EIP76_SPB_WR(Device, EIP76_CUTOFF_REP);
    }
    else if ((TestType == EIP76_ADPRO_TYPE_64)  ||
             (TestType == EIP76_ADPRO_TYPE_512) ||
             (TestType == EIP76_ADPRO_TYPE_4k))
    {
        TrueIOArea_p->ConfigStatus = EIP76_CONFIG_RD(Device);

        // Set cont_poker, test_known_noise and test_spb
        EIP76_TEST_WR(Device, EIP76_TEST_REPCOUNTORADAPTIVE);

        ControlWord |= TEST_SAMPLE_CYCLES;

        // Configure the noise_block field for test
        EIP76_CONFIG_NOISEBLK_WR(Device, ControlWord);

        // Set show counters and show values to reset the counters
        EIP76_SPB_WR(Device, EIP76_SHOW_COUNTERS_AND_VALUES);

        if (TestType == EIP76_ADPRO_TYPE_64)
        {
            // Set cutoff value for adaptive proportion 64 test
            // this is the maximum value of the adaptive proportion 64 counter 63.
            EIP76_SPB_WR(Device, EIP76_CUTOFF_APROP_64 << 8);
        }
        else if (TestType == EIP76_ADPRO_TYPE_512)
        {
            // Set cutoff value for adaptive proportion 512 test
            // this is the maximum value of the adaptive proportion 512 counter 511.
            EIP76_SPB_WR(Device, EIP76_CUTOFF_APROP_512 << 16);
        }
        else
        {
            // Set cutoff value for adaptive proportion 4k test
            // this is the maximum value of the adaptive proportion 4096 counter 4095.
            EIP76_SPB_WR(Device, EIP76_CUTOFF_APROP_4k << 16);
        }

        // Reset TRNG_COUNT to keep the conditioner quit
        EIP76_COUNT_WR(Device, RESET_COUNTERS);
    }
    else if (TestType == EIP76_SP80090_AES_CORE_KAT_TYPE)
    {
        // Post Processor Test
        EIP76_TEST_WR(Device, EIP76_TEST_POST_PROC);
    }
#if (EIP76_POST_PROCESSOR_TYPE == EIP76_POST_PROCESSOR_BC_DF)
    else if (TestType == EIP76_SP80090_BC_DF_KAT_TYPE)
    {
        // Post Processor Test
        EIP76_TEST_WR(Device, EIP76_TEST_SP_800_90 | EIP76_TEST_KNOWN_NOISE);

        // Configure the noise_block field for this test
        EIP76_CONFIG_WR(Device,
                        1, // SP80090 BCDF KAT noise blocks
                        EIP76_SAMPLE_DIVIDER,
                        EIP76_READ_TIMEOUT,
                        3, // SP80090 BCDF KAT sample cycles
                        EIP76_SAMPLE_SCALE);
    }
#endif
    else
    {
        // Post Processor Test
        EIP76_TEST_WR(Device, EIP76_TEST_SP_800_90);
    }

    // Check if test is ready
    {
        uint32_t Mask = EIP76_STATUS_TEST_READY;

        if (TestType == EIP76_SP80090_BC_DF_KAT_TYPE)
            Mask |= EIP76_STATUS_RESEED_AI;

        RegVal = EIP76_STATUS_RD(Device);

        if ((RegVal & Mask) != Mask)
        {
            // Test is not ready, remain in EIP76_STATE_KAT_START state
            return EIP76_BUSY_RETRY_LATER;
        }
    }

    // Test is ready, transit to a new state
    return EIP76_State_Set((volatile EIP76_State_t*) &TrueIOArea_p->State,
                           EIP76_STATE_KAT_READY);
}


/*----------------------------------------------------------------------------
 * EIP76_Test_Abort
 *
 */
EIP76_Status_t
EIP76_Test_Abort(
        EIP76_IOArea_t * const IOArea_p,
        EIP76_EventStatus_t * const Events_p)
{
    Device_Handle_t Device;
    uint32_t RegVal;
    EIP76_Status_t rv;
    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP76_CHECK_POINTER(IOArea_p);

    EIP76_CHECK_POINTER(Events_p);

    // No events detected yet
    *Events_p = 0;

    Device = TrueIOArea_p->Device;

    TrueIOArea_p->Flag  = false;
    TrueIOArea_p->Index = 0;

    // Check if in test mode
    RegVal = EIP76_CONTROL_RD(Device);
    if( (RegVal & EIP76_CONTROL_ENABLE_TEST_MODE) == 0 )
        return EIP76_ILLEGAL_IN_STATE;

    // Transit to a new state
    rv = EIP76_State_Set((volatile EIP76_State_t*)&TrueIOArea_p->State,
                         EIP76_STATE_RANDOM_GENERATING);

    if(rv != EIP76_NO_ERROR)
        return EIP76_ILLEGAL_IN_STATE; // May never occur

    RegVal = EIP76_STATUS_RD(Device);

    // Store event status
    *Events_p = (RegVal & EIP76_EVENTS_MASK);

    // Set to 0 number of written 64-bit words as test input
    TrueIOArea_p->PRM_WordCount = 0;

    // Read Test Mode
    RegVal = EIP76_TEST_RD(Device);

    // Reset Cutoff values as they are changed for the test
    if  (RegVal == EIP76_TEST_REPCOUNTORADAPTIVE)
    {
        if (EIP76_APROP512_RD(Device))
        {
            EIP76_SPB_WR(Device, EIP76_CUTOFF_512_DEFAULT);
        }
        else {
            EIP76_SPB_WR(Device, EIP76_CUTOFF_DEFAULT);
        }
    }

    // Clear all existing tests that could have been started
    RegVal &= (~(EIP76_TEST_CONT_POKER |
                 EIP76_TEST_RUN_POKER  |
                 EIP76_TEST_POST_PROC  |
                 EIP76_TEST_SP_800_90  |
                 EIP76_TEST_REPCOUNTORADAPTIVE));
    EIP76_TEST_WR(Device, RegVal);

    // Clear stall_run_poker and fro_test_mux in TRNG_ALARMCNT
    RegVal = EIP76_ALARMCNT_RD(Device);
    EIP76_ALARMCNT_WR(Device, RegVal & (~(EIP76_ALARMCNT_STALL_RUN_POKER)));

#ifdef EIP76_HW_PRM_ENABLED // Check if PRM tests are enabled
    // Acknowledge test events
    EIP76_INTACK_WR(Device,
                    (EIP76_RUN_FAIL_EVENT | EIP76_LONG_RUN_FAIL_EVENT |
                    EIP76_POKER_FAIL_EVENT | EIP76_MONOBIT_FAIL_EVENT |
                    EIP76_REPCNT_FAIL_EVENT | EIP76_APROP_FAIL_EVENT |
                    EIP76_STUCK_NRBG_EVENT));
#else
    // Acknowledge test events
      EIP76_INTACK_WR(Device,
                      (EIP76_REPCNT_FAIL_EVENT | EIP76_APROP_FAIL_EVENT |
                      EIP76_STUCK_NRBG_EVENT));
#endif

    // Restore TRNG_CONTROL register (internal TRNG HW state) stored
    // when the test was started
    EIP76_CONTROL_WR(Device, TrueIOArea_p->SavedControl);

    return EIP76_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP76_Test_Status_Get
 *
 */
EIP76_Status_t
EIP76_Test_Status_Get(
        EIP76_IOArea_t * const IOArea_p,
        EIP76_EventStatus_t * const Events_p)
{
    Device_Handle_t Device;
    uint32_t StatusRegVal;
    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP76_CHECK_POINTER(IOArea_p);

    EIP76_CHECK_POINTER(Events_p);

    // No events detected yet
    *Events_p = 0;

    Device = TrueIOArea_p->Device;

    StatusRegVal = EIP76_STATUS_RD(Device);

    // Store event status
    *Events_p = (StatusRegVal & EIP76_EVENTS_MASK);

    if( TrueIOArea_p->PRM_WordCount < 625 )
    {
        if ( EIP76Lib_Test_Is_Ready(Device) )
        {
            return EIP76_State_Set((volatile EIP76_State_t*)&TrueIOArea_p->State,
                                   EIP76_STATE_KAT_READY);
        }
        else
        {
            // test is not ready, stay in current state
            return EIP76_BUSY_RETRY_LATER;
        }
    }
    else
    {
        uint32_t RunCntRegVal;

        RunCntRegVal = EIP76_RUNCNT_RD(Device) & EIP76_RUN_CNT_TEST_COUNT_MASK;

        if( RunCntRegVal < 20001 )
            // Monobit test is not finished yet, stay in current state
            return EIP76_BUSY_RETRY_LATER;

        // Check if Monobit test failed
        if( (StatusRegVal & EIP76_MONOBIT_FAIL_EVENT) != 0 )
        {
            EIP76_Status_t rv = EIP76_NO_ERROR;

            // Monobit test is failed and thus finished,
            // transit to a new state
            rv = EIP76_State_Set((volatile EIP76_State_t*)&TrueIOArea_p->State,
                                 EIP76_STATE_KAT_PRM_M_FAILED);

            if(rv != EIP76_NO_ERROR)
                return EIP76_ILLEGAL_IN_STATE; // May never occur

            return EIP76_MONOBIT_FAIL;
        }

        if( RunCntRegVal < EIP76_TEST_FINISHED_RUN_COUNT )
            // Poker test is not finished yet, stay in current state
            return EIP76_BUSY_RETRY_LATER;

        // PRM test is finished, transit to a new state
        return EIP76_State_Set((volatile EIP76_State_t* const)&TrueIOArea_p->State,
                               EIP76_STATE_KAT_PRM_DONE);
    }
}


/*----------------------------------------------------------------------------
 * EIP76_Test_Block_Write
 *
 */
EIP76_Status_t
EIP76_Test_CF_Block_Write(
        EIP76_IOArea_t * const IOArea_p,
        const uint32_t InputWord_First_p,
        const uint32_t InputWord_Second_p,
        EIP76_EventStatus_t * const Events_p)
{
    Device_Handle_t Device;
    uint32_t StatusRegVal, ControlRegVal;
    EIP76_Status_t rv = EIP76_NO_ERROR;
    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP76_CHECK_POINTER(IOArea_p);

    EIP76_CHECK_POINTER(Events_p);

    // No events detected yet
    *Events_p = 0;

    Device = TrueIOArea_p->Device;

    // Check if in test mode
    ControlRegVal = EIP76_CONTROL_RD(Device);
    if( (ControlRegVal & EIP76_CONTROL_ENABLE_TEST_MODE) == 0 )
        return EIP76_ILLEGAL_IN_STATE;

    // Transit to a new state
    rv = EIP76_State_Set((volatile EIP76_State_t*)&TrueIOArea_p->State,
                         EIP76_STATE_KAT_CF_PROCESSING);

    if(rv != EIP76_NO_ERROR)
        return EIP76_ILLEGAL_IN_STATE; // May never occur

    StatusRegVal = EIP76_STATUS_RD(Device);

    // Store event status
    *Events_p = (StatusRegVal & EIP76_EVENTS_MASK);

    // Write 64-bit input word for the test
    EIP76_MAINSHIFTREG_L_WR(Device, InputWord_First_p);
    EIP76_MAINSHIFTREG_H_WR(Device, InputWord_Second_p);


    // Check if test is ready
    if ( !EIP76Lib_Test_Is_Ready(Device) )
        // Test is not ready,
        // remain in EIP76_STATE_KAT_START state
        return EIP76_BUSY_RETRY_LATER;


    return EIP76_NO_ERROR;
}

/*----------------------------------------------------------------------------
 * EIP76_Test_State_Block_Write
 *
 */
EIP76_Status_t
EIP76_Test_State_Block_Write(
        EIP76_IOArea_t * const IOArea_p,
        const uint32_t InputWord_First_p,
        const uint32_t InputWord_Second_p,
        const EIP76_State_t NewState,
        EIP76_EventStatus_t * const Events_p)
{
    Device_Handle_t Device;
       uint32_t StatusRegVal, ControlRegVal;
       EIP76_Status_t rv = EIP76_NO_ERROR;
       volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

       EIP76_CHECK_POINTER(IOArea_p);

       EIP76_CHECK_POINTER(Events_p);

       // No events detected yet
       *Events_p = 0;

       Device = TrueIOArea_p->Device;

       // Check if in test mode
       ControlRegVal = EIP76_CONTROL_RD(Device);
       if( (ControlRegVal & EIP76_CONTROL_ENABLE_TEST_MODE) == 0 )
           return EIP76_ILLEGAL_IN_STATE;

       // Transit to a new state
       rv = EIP76_State_Set((volatile EIP76_State_t*)&TrueIOArea_p->State,
                           NewState);

       if(rv != EIP76_NO_ERROR)
           return EIP76_ILLEGAL_IN_STATE; // May never occur

       StatusRegVal = EIP76_STATUS_RD(Device);

       // Store event status
       *Events_p = (StatusRegVal & EIP76_EVENTS_MASK);

       // Write 64-bit input word for the test
       EIP76_MAINSHIFTREG_L_WR(Device, InputWord_First_p);
       EIP76_MAINSHIFTREG_H_WR(Device, InputWord_Second_p);

       // Check if test is ready
       if ( !EIP76Lib_Test_Is_Ready(Device) )
           // Test is not ready,
           // remain in EIP76_STATE_KAT_START state
           return EIP76_BUSY_RETRY_LATER;


       return EIP76_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP76_Test_PRM_Write64
 *
 */
EIP76_Status_t
EIP76_Test_PRM_Write64(
        EIP76_IOArea_t * const IOArea_p,
        const uint32_t InputWord_First_p,
        const uint32_t InputWord_Second_p,
        EIP76_EventStatus_t * const Events_p)
{
    Device_Handle_t Device;
    uint32_t StatusRegVal, TestRegVal, ControlRegVal;
    EIP76_Status_t rv = EIP76_NO_ERROR;
    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP76_CHECK_POINTER(IOArea_p);

    EIP76_CHECK_POINTER(Events_p);

    // No events detected yet
    *Events_p = 0;

    Device = TrueIOArea_p->Device;

    // Check if in test mode
    ControlRegVal = EIP76_CONTROL_RD(Device);
    if( (ControlRegVal & EIP76_CONTROL_ENABLE_TEST_MODE) == 0 )
        return EIP76_ILLEGAL_IN_STATE;

    // Transit to a new state
    rv = EIP76_State_Set((volatile EIP76_State_t*)&TrueIOArea_p->State,
                         EIP76_STATE_KAT_PRM_PROCESSING);

    if(rv != EIP76_NO_ERROR)
        return EIP76_ILLEGAL_IN_STATE; // May never occur

    StatusRegVal = EIP76_STATUS_RD(Device);

    // Store event status
    *Events_p = (StatusRegVal & EIP76_EVENTS_MASK);

    TestRegVal = EIP76_TEST_RD(Device);

    // Increase 64-bit word count only for normal Poker test
    if( (TestRegVal & EIP76_TEST_RUN_POKER) != 0 &&
        (TestRegVal & EIP76_TEST_CONT_POKER) == 0)
        TrueIOArea_p->PRM_WordCount = TrueIOArea_p->PRM_WordCount + 2;

    // Write 64-bit input word for the test
    EIP76_MAINSHIFTREG_L_WR(Device, InputWord_First_p);
    EIP76_MAINSHIFTREG_H_WR(Device, InputWord_Second_p);


    // Check if test is ready
    if ( !EIP76Lib_Test_Is_Ready(Device) )
        // Test is not ready,
        // remain in EIP76_STATE_KAT_START state
        return EIP76_BUSY_RETRY_LATER;

    if( TrueIOArea_p->PRM_WordCount < 625 )
    {
        // Test is ready, transit to a new state
        rv = EIP76_State_Set((volatile EIP76_State_t*)&TrueIOArea_p->State,
                             EIP76_STATE_KAT_READY);

        if(rv != EIP76_NO_ERROR)
            return EIP76_ILLEGAL_IN_STATE; // May never occur
    }

    return EIP76_NO_ERROR;
}


#ifndef EIP76_MONOBIT_DISABLE
/*----------------------------------------------------------------------------
 * EIP76_Test_M_Result_Read
 *
 */
EIP76_Status_t
EIP76_Test_M_Result_Read(
        EIP76_IOArea_t * const IOArea_p,
        uint32_t * const MonobitCount_p,
        EIP76_EventStatus_t * const Events_p)
{
    Device_Handle_t Device;
    uint32_t RegVal;
    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP76_CHECK_POINTER(IOArea_p);

    EIP76_CHECK_POINTER(Events_p);

    EIP76_CHECK_POINTER(MonobitCount_p);

    // No events detected yet
    *Events_p = 0;

    Device = TrueIOArea_p->Device;

    // Check if in test mode
    RegVal = EIP76_CONTROL_RD(Device);
    if( (RegVal & EIP76_CONTROL_ENABLE_TEST_MODE) == 0 )
        return EIP76_ILLEGAL_IN_STATE;

    RegVal = EIP76_STATUS_RD(Device);

    // Store event status
    *Events_p = (RegVal & EIP76_EVENTS_MASK);

    *MonobitCount_p = EIP76_MONOBITCNT_RD(Device) & EIP76_MONOBITCNT_MASK;

    // Set to 0 number of written 32-bit words as test input
    TrueIOArea_p->PRM_WordCount = 0;

    // Leave Test Mode
    RegVal = EIP76_TEST_RD(Device);
    // Clear all existing tests that could have been started
    RegVal &= (~(EIP76_TEST_CONT_POKER |
                 EIP76_TEST_RUN_POKER  |
                 EIP76_TEST_POST_PROC  |
                 EIP76_TEST_SP_800_90   ));
    EIP76_TEST_WR(Device, RegVal);

    // Clear stall_run_poker and fro_test_mux in TRNG_ALARMCNT
    RegVal = EIP76_ALARMCNT_RD(Device);
    EIP76_ALARMCNT_WR(Device, RegVal & (~(EIP76_ALARMCNT_STALL_RUN_POKER)));

    // Acknowledge test events
    EIP76_INTACK_WR(Device,
                     (EIP76_RUN_FAIL_EVENT | EIP76_LONG_RUN_FAIL_EVENT |
                     EIP76_POKER_FAIL_EVENT | EIP76_MONOBIT_FAIL_EVENT));

    // Restore TRNG_CONTROL register (internal TRNG HW state) stored
    // when the test was started
    EIP76_CONTROL_WR(Device, TrueIOArea_p->SavedControl);

    // PRM test is finished, transit to a new state
    return EIP76_State_Set((volatile EIP76_State_t*)&TrueIOArea_p->State,
            EIP76_STATE_RANDOM_GENERATING);
}
#endif // not EIP76_MONOBIT_DISABLE


/*----------------------------------------------------------------------------
 * EIP76_Test_PR_Result_Read
 *
 */
EIP76_Status_t
EIP76_Test_PR_Result_Read(
        EIP76_IOArea_t * const IOArea_p,
        EIP76_RunKAT_Result_t * const RunKATResult_p,
        EIP76_PokerKAT_Result_t * const PokerKATResult_p,
        EIP76_EventStatus_t * const Events_p)
{
    Device_Handle_t Device;
    uint32_t RegVal;
    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP76_CHECK_POINTER(IOArea_p);

    EIP76_CHECK_POINTER(Events_p);

    EIP76_CHECK_POINTER(RunKATResult_p);

    EIP76_CHECK_POINTER(PokerKATResult_p);

    // No events detected yet
    *Events_p = 0;

    Device = TrueIOArea_p->Device;

    // Check if in test mode
    RegVal = EIP76_CONTROL_RD(Device);
    if( (RegVal & EIP76_CONTROL_ENABLE_TEST_MODE) == 0 )
        return EIP76_ILLEGAL_IN_STATE;

    RegVal = EIP76_STATUS_RD(Device);

    // Store event status
    *Events_p = (RegVal & EIP76_EVENTS_MASK);

    // Read Run test results
    EIP76_RUN_TEST_READ(Device, RunKATResult_p);

    // Read Poker test results
    EIP76_POKER_TEST_READ(Device, PokerKATResult_p);

    // Set to 0 number of written 32-bit words as test input
    TrueIOArea_p->PRM_WordCount = 0;

    // Leave Test Mode
    RegVal = EIP76_TEST_RD(Device);
    // Clear all existing tests that could have been started
    RegVal &= (~(EIP76_TEST_CONT_POKER |
                 EIP76_TEST_RUN_POKER  |
                 EIP76_TEST_POST_PROC  |
                 EIP76_TEST_SP_800_90   ));
    EIP76_TEST_WR(Device, RegVal);

    // Clear stall_run_poker and fro_test_mux in TRNG_ALARMCNT
    RegVal = EIP76_ALARMCNT_RD(Device);
    EIP76_ALARMCNT_WR(Device, RegVal & (~(EIP76_ALARMCNT_STALL_RUN_POKER)));

    // Acknowledge test events
    EIP76_INTACK_WR(Device,
                     (EIP76_RUN_FAIL_EVENT | EIP76_LONG_RUN_FAIL_EVENT |
                     EIP76_POKER_FAIL_EVENT | EIP76_MONOBIT_FAIL_EVENT));

    // Restore TRNG_CONTROL register (internal TRNG HW state) stored
    // when the test was started
    EIP76_CONTROL_WR(Device, TrueIOArea_p->SavedControl);

    // PRM test is finished, transit to a new state
    return EIP76_State_Set((volatile EIP76_State_t*)&TrueIOArea_p->State,
                          EIP76_STATE_RANDOM_GENERATING);
}


/*----------------------------------------------------------------------------
 * EIP76_Test_CF_Input_Write
 *
 */
EIP76_Status_t
EIP76_Test_CF_Input_Write(
        EIP76_IOArea_t * const IOArea_p,
        EIP76_EventStatus_t * const Events_p,
        uint32_t *SHA_Input_Data,
        uint8_t const SHAVersion)
{
       Device_Handle_t Device;
       uint32_t RegVal;

       volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

       EIP76_CHECK_POINTER(IOArea_p);

       EIP76_CHECK_POINTER(Events_p);

       // No events detected yet
       *Events_p = 0;

       Device = TrueIOArea_p->Device;

       // Check if in test mode
       RegVal = EIP76_CONTROL_RD(Device);
       if( (RegVal & EIP76_CONTROL_ENABLE_TEST_MODE) == 0 )
           return EIP76_ILLEGAL_IN_STATE;

       RegVal = EIP76_STATUS_RD(Device);

       // Store event status
       *Events_p = (RegVal & EIP76_EVENTS_MASK);

       if(SHAVersion == 1){
           // Write the CF output registers
           EIP76_INPUT_CF_0_WR(Device, SHA_Input_Data[0]);
           EIP76_INPUT_CF_1_WR(Device, SHA_Input_Data[1]);
           EIP76_INPUT_CF_2_WR(Device, SHA_Input_Data[2]);
           EIP76_INPUT_CF_3_WR(Device, SHA_Input_Data[3]);
           EIP76_INPUT_CF_4_WR(Device, SHA_Input_Data[4]);
       }
       else
       {
           // Write the CF output registers
           EIP76_INPUT_CF_0_WR(Device, SHA_Input_Data[0]);
           EIP76_INPUT_CF_1_WR(Device, SHA_Input_Data[1]);
           EIP76_INPUT_CF_2_WR(Device, SHA_Input_Data[2]);
           EIP76_INPUT_CF_3_WR(Device, SHA_Input_Data[3]);
           EIP76_INPUT_CF_4_WR(Device, SHA_Input_Data[4]);
           EIP76_INPUT_CF_5_WR(Device, SHA_Input_Data[5]);
           EIP76_INPUT_CF_6_WR(Device, SHA_Input_Data[6]);
           EIP76_INPUT_CF_7_WR(Device, SHA_Input_Data[7]);

       }

       return EIP76_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP76_Test_CF_Read_Result
 *
 */
EIP76_Status_t EIP76_Test_CF_Read_Result(EIP76_IOArea_t * const IOArea_p,
        EIP76_EventStatus_t * const Events_p, uint32_t const * SHA_MD,
        uint8_t SHAVersion)
{
    Device_Handle_t Device;
    uint32_t Digest[8];
    uint32_t RegVal;
    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP76_CHECK_POINTER(IOArea_p);

    EIP76_CHECK_POINTER(Events_p);

    // No events detected yet
    *Events_p = 0;

    Device = TrueIOArea_p->Device;

    // Check if in test mode
    RegVal = EIP76_CONTROL_RD(Device);
    if ((RegVal & EIP76_CONTROL_ENABLE_TEST_MODE) == 0)
        return EIP76_ILLEGAL_IN_STATE;

    RegVal = EIP76_STATUS_RD(Device);

    // Store event status
    *Events_p = (RegVal & EIP76_EVENTS_MASK);

    if (SHAVersion == 1) {
        // Read result from CF output registers, read 4 bits (the actual data)
        Digest[0] = Device_SwapEndian(EIP76_INPUT_CF_0_RD(Device));
        Digest[1] = Device_SwapEndian(EIP76_INPUT_CF_1_RD(Device));
        Digest[2] = Device_SwapEndian(EIP76_INPUT_CF_2_RD(Device));
        Digest[3] = Device_SwapEndian(EIP76_INPUT_CF_3_RD(Device));
        Digest[4] = Device_SwapEndian(EIP76_INPUT_CF_4_RD(Device));
        Digest[5] = 0;
        Digest[6] = 0;
        Digest[7] = 0;

        // Clear ready bit when done reading result
        EIP76_INTACK_WR(Device, CLEAR_READY_BIT);
        EIP76_INTACK_WR(Device, CLEAR_TEST_READY_BIT);

        // Compare with digest
        if ((Digest[0] == SHA_MD[0]) && (Digest[1] == SHA_MD[1]) && (Digest[2]
                == SHA_MD[2]) && (Digest[3] == SHA_MD[3]) && (Digest[4]
                == SHA_MD[4])) {

            // Write back the config register with pre test values
            EIP76_CONFIG_NOISEBLK_WR(Device, TrueIOArea_p->ConfigStatus);

            // Ack any related events
            EIP76_INTACK_WR(Device, CLEAR_INTACK);

            // Clear test register
            EIP76_TEST_WR(Device, 0);

            // Restore TRNG_CONTROL register (internal TRNG HW state) stored
            // when the test was started
            EIP76_CONTROL_WR(Device, TrueIOArea_p->SavedControl);

            // Last test done go to reset state
            EIP76_State_Set((volatile EIP76_State_t*) &TrueIOArea_p->State,
                    EIP76_STATE_RANDOM_GENERATING);

            return EIP76_NO_ERROR;
        }
        else
        {

            return EIP76_SHA_FAIL;
        };
    }
    else
    {

        // Read result from CF output registers
        Digest[0] = Device_SwapEndian(EIP76_INPUT_CF_0_RD(Device));
        Digest[1] = Device_SwapEndian(EIP76_INPUT_CF_1_RD(Device));
        Digest[2] = Device_SwapEndian(EIP76_INPUT_CF_2_RD(Device));
        Digest[3] = Device_SwapEndian(EIP76_INPUT_CF_3_RD(Device));
        Digest[4] = Device_SwapEndian(EIP76_INPUT_CF_4_RD(Device));
        Digest[5] = Device_SwapEndian(EIP76_INPUT_CF_5_RD(Device));
        Digest[6] = Device_SwapEndian(EIP76_INPUT_CF_6_RD(Device));
        Digest[7] = Device_SwapEndian(EIP76_INPUT_CF_7_RD(Device));

        // Clear ready bit when done reading result
        EIP76_INTACK_WR(Device, CLEAR_READY_BIT);
        EIP76_INTACK_WR(Device, CLEAR_TEST_READY_BIT);

        // Write back the config register with pre test values
        EIP76_CONFIG_NOISEBLK_WR(Device, TrueIOArea_p->ConfigStatus);

        // Compare with digest
        if ((Digest[0] == SHA_MD[0]) && (Digest[1] == SHA_MD[1]) && (Digest[2]
                == SHA_MD[2]) && (Digest[3] == SHA_MD[3]) && (Digest[4]
                == SHA_MD[4]) && (Digest[5] == SHA_MD[5]) && (Digest[6]
                == SHA_MD[6]) && (Digest[7] == SHA_MD[7])) {

            // Ack any related events
            EIP76_INTACK_WR(Device, CLEAR_INTACK);

            //Clear test register
            EIP76_TEST_WR(Device, 0);

            // Restore TRNG_CONTROL register (internal TRNG HW state) stored
            // when the test was started
            EIP76_CONTROL_WR(Device, TrueIOArea_p->SavedControl);

            // Last test done go to reset state
            EIP76_State_Set((volatile EIP76_State_t*) &TrueIOArea_p->State,
                    EIP76_STATE_RANDOM_GENERATING);

            return EIP76_NO_ERROR;
        }
        else
        {

            return EIP76_SHA_FAIL;
        };
    }
}

/*----------------------------------------------------------------------------
 * EIP76_Test_Rep_Count_Fail_Check
 *
 */
EIP76_Status_t EIP76_Test_Rep_Count_Fail_Check(EIP76_IOArea_t * const IOArea_p) {

    Device_Handle_t Device;
    uint32_t RegVal;
    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP76_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    // Read status
    RegVal = EIP76_STATUS_RD(Device);

    // Write back the config register with pre test values
    EIP76_CONFIG_NOISEBLK_WR(Device, TrueIOArea_p->ConfigStatus);

    // Ack any related events that may have occurred
    EIP76_INTACK_WR(Device, CLEAR_INTACK);

    // Reset Cutoff values as they are changed for the test
    if (EIP76_APROP512_RD(Device))
    {
        EIP76_SPB_WR(Device, EIP76_CUTOFF_512_DEFAULT);
    }
    else {
        EIP76_SPB_WR(Device, EIP76_CUTOFF_DEFAULT);
    }

    // Clear test register
    EIP76_TEST_WR(Device, 0);

    // Restore TRNG_CONTROL register (internal TRNG HW state) stored
    // when the test was started
    EIP76_CONTROL_WR(Device, TrueIOArea_p->SavedControl);

    // Last test done go to reset state
    EIP76_State_Set((volatile EIP76_State_t*) &TrueIOArea_p->State,
            EIP76_STATE_RANDOM_GENERATING);

    if (RegVal & BIT_13)
    {
        return EIP76_REPCNT_FAIL;
    }

    return EIP76_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP76_Test_CF_Status_Get
 *
 */
EIP76_Status_t
EIP76_Test_CF_Status_Get(EIP76_IOArea_t * const IOArea_p,
        EIP76_EventStatus_t * const Events_p)
{
    Device_Handle_t Device;
    uint32_t StatusRegVal;
    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP76_CHECK_POINTER(IOArea_p);

    EIP76_CHECK_POINTER(Events_p);

    // No events detected yet
    *Events_p = 0;

    Device = TrueIOArea_p->Device;

    StatusRegVal = EIP76_STATUS_RD(Device);

    // Store event status
    *Events_p = (StatusRegVal & EIP76_EVENTS_MASK);

    if ((StatusRegVal & EIP76_STATUS_TEST_READY) &&
        (StatusRegVal & EIP76_STATUS_READY)) {
        // Test and ready are ready, transit to a new state
        return EIP76_State_Set((volatile EIP76_State_t*) &TrueIOArea_p->State,
                EIP76_STATE_KAT_CF_DONE);
    }
    else {
        return EIP76_BUSY_RETRY_LATER;
    }

}


/*----------------------------------------------------------------------------
 * EIP76_Test_CF_NonLastChain
 *
 */
EIP76_Status_t
EIP76_Test_CF_NonLastChain(EIP76_IOArea_t * const IOArea_p)
{
    Device_Handle_t Device;
    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP76_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    EIP76_INTACK_WR(Device, CLEAR_READY_BIT);
    EIP76_INTACK_WR(Device, CLEAR_TEST_READY_BIT);

    // Test is ready, transit to a new state
    return EIP76_State_Set((volatile EIP76_State_t*) &TrueIOArea_p->State,
            EIP76_STATE_KAT_CF_PROCESSING);
}


/*----------------------------------------------------------------------------
 * EIP76_Test_Rep_Sample_Counter_Read
 *
 */
bool
EIP76_Test_Rep_Sample_Counter_Read(
        EIP76_IOArea_t * const IOArea_p,
        uint32_t ExpRepSample, uint32_t ExpRepCounter)
{
    uint32_t retVal;
    Device_Handle_t Device;

    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP76_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    // Read counters
    retVal = EIP76_SPB_RD(Device);

    // Check if show_counters bit has been set
    if (((retVal >> 28)&0x3) != 0x1)
    {
        // Set show_counters bit
        EIP76_SPB_WR(Device, EIP76_SHOW_COUNTERS);
        // Read counters
        retVal = EIP76_SPB_RD(Device);
    }

    // Check current counter value with expected value
    if ((retVal & 0x3f) != (ExpRepCounter & 0x3f))
        return false;

    // Set show values bit
    EIP76_SPB_WR(Device, EIP76_SHOW_VALUES);

    // Read values
    retVal = EIP76_SPB_RD(Device);

    // Check if there is no match in one off the words
    if (((retVal & 0xff) != (ExpRepSample & 0xff)))
        return false;

    return true;
}

/*----------------------------------------------------------------------------
 * EIP76_Test_Adap_Sample_Counter_Read
 *
 */
bool
EIP76_Test_Adap_Sample_Counter_Read(
        EIP76_IOArea_t * const IOArea_p,
        uint32_t ExpAdapSample,
        uint32_t ExpAdapCounter,
        const EIP76_Test_Type_t TestType)
{
    uint32_t retVal;
    Device_Handle_t Device;

    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP76_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    // Read counters
    retVal = EIP76_SPB_RD(Device);

    // Check if show_counters bit has been set
    if(((retVal >> 28)&0x3) != 0x1)
    {
        // Set show_counters bit
        EIP76_SPB_WR(Device, EIP76_SHOW_COUNTERS);
        // Read counters
        retVal = EIP76_SPB_RD(Device);
    }

    if (TestType == EIP76_ADPRO_TYPE_64)
    {
        // Check current counter value with expected value
        if ((retVal >> 8 & 0x3f) != (ExpAdapCounter & 0x3f))
        {
            return false;
        }
    }
    else if (TestType == EIP76_ADPRO_TYPE_512)
    {
        // Check current counter value with expected value
        if ((retVal >> 16 & 0x1ff) != (ExpAdapCounter & 0x1ff))
        {
            return false;
        }
    }
    else
    {
        // Check current counter value with expected value
        if ((retVal >> 16 & 0xfff) != (ExpAdapCounter & 0xfff))
            return false;
    }

    // Set show_values bit
    EIP76_SPB_WR(Device, EIP76_SHOW_VALUES);

    // Read values
    retVal = EIP76_SPB_RD(Device);

    if (TestType == EIP76_ADPRO_TYPE_64)
    {
        // Check if there is no match in one off the words
        if (((retVal >> 8 & 0xff) != (ExpAdapSample & 0xff)))
            return false;
    }
    else if (TestType == EIP76_ADPRO_TYPE_512)
    {
        // Check if there is no match in one off the words
        if (((retVal >> 16 & 0xff) != (ExpAdapSample & 0xff)))
            return false;
    }
    else
    {
        // Check if there is no match in one off the words
        if (((retVal >> 16 & 0xff) != (ExpAdapSample & 0xff)))
            return false;
    }

    return true;
}


/*----------------------------------------------------------------------------
 * EIP76_Test_Adap_Fail_Check
 *
 */
EIP76_Status_t
EIP76_Test_Adap_Fail_Check(
        EIP76_IOArea_t * const IOArea_p)
{
    Device_Handle_t Device;
    uint32_t RegVal;
    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    EIP76_CHECK_POINTER(IOArea_p);

    Device = TrueIOArea_p->Device;

    // Read status
    RegVal = EIP76_STATUS_RD(Device);

    // Write back the config register with pre test values
    EIP76_CONFIG_NOISEBLK_WR(Device, TrueIOArea_p->ConfigStatus);

    // Ack any related events that may have occurred
    EIP76_INTACK_WR(Device, CLEAR_INTACK);

    // Reset Cutoff values as they are changed for the test
    if (EIP76_APROP512_RD(Device))
    {
        EIP76_SPB_WR(Device, EIP76_CUTOFF_512_DEFAULT);
    }
    else {
        EIP76_SPB_WR(Device, EIP76_CUTOFF_DEFAULT);
    }

    // Clear test register
    EIP76_TEST_WR(Device, 0);

    // Restore TRNG_CONTROL register (internal TRNG HW state) stored
    // when the test was started
    EIP76_CONTROL_WR(Device, TrueIOArea_p->SavedControl);

    // Last test done go to reset state
    EIP76_State_Set((volatile EIP76_State_t*) &TrueIOArea_p->State,
            EIP76_STATE_RANDOM_GENERATING);

    // Check if error bit is set
    if (RegVal & BIT_14)
    {
        return EIP76_ADPRO_FAIL;
    }

    return EIP76_NO_ERROR;
}


/*----------------------------------------------------------------------------
 * EIP76_Test_Is_Ready
 *
 */
EIP76_Status_t
EIP76_Test_Is_Ready(
        EIP76_IOArea_t * const IOArea_p,
        EIP76_EventStatus_t * const Events_p,
        uint32_t * const data_buf_p)
{
    Device_Handle_t Device;
    uint32_t StatusRegVal;
    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);


    EIP76_CHECK_POINTER(IOArea_p);

    EIP76_CHECK_POINTER(Events_p);

    Device = TrueIOArea_p->Device;

    StatusRegVal = EIP76_STATUS_RD(Device);

    // Store event status
    *Events_p = (StatusRegVal & EIP76_EVENTS_MASK);

    if (EIP76_STATUS_IS_READY(StatusRegVal))
    {

        data_buf_p[0] = EIP76_OUTPUT_0_RD(Device);
        data_buf_p[1] = EIP76_OUTPUT_1_RD(Device);
        data_buf_p[2] = EIP76_OUTPUT_2_RD(Device);
        data_buf_p[3] = EIP76_OUTPUT_3_RD(Device);

        // Clear ready bit when done reading result
        EIP76_INTACK_WR(Device, CLEAR_READY_BIT);
        return EIP76_NO_ERROR;
    }
    else
    {
        // status is not ready, stay in current state
        return EIP76_BUSY_RETRY_LATER;
    }
}

/*----------------------------------------------------------------------------
 * EIP76_Test_Set_Reseed
 *
 */
void
EIP76_Test_Set_Reseed(
        EIP76_IOArea_t * const IOArea_p,
        EIP76_EventStatus_t * const Events_p)
{
    Device_Handle_t Device;

    volatile EIP76_True_IOArea_t * const TrueIOArea_p = IOAREA(IOArea_p);

    // EIP76_CHECK_POINTER(IOArea_p);

    // EIP76_CHECK_POINTER(Events_p);

    Device = TrueIOArea_p->Device;

    // No event found
    *Events_p = 0;

    EIP76_CONTROL_WR(Device, EIP76_CONTROL_ENABLE_RESEED);
}


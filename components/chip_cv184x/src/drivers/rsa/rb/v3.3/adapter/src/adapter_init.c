/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */




/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include "adapter_init.h"


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default Adapter configuration
#include "c_adapter.h"

#if  defined(ADAPTER_EIP28_INTERRUPT_ENABLE)
#include "adapter_interrupts.h" // Adapter_Interrupts_Init,
                                // Adapter_Interrupts_UnInit
#endif


#include "adapter_eip28.h"

// Driver Framework Device API
#include "device_mgmt.h"    // Device_Initialize, Device_UnInitialize
#include "device_rw.h"      // Device_Read32, Device_Write32

// Driver Framework DMAResource API

// Driver Framework Basic Definitions API
#include "basic_defs.h"     // bool, true, false


/*----------------------------------------------------------------------------
 * Local variables
 */

static bool Adapter_IsInitialized = false;


/*----------------------------------------------------------------------------
 * Adapter_Init
 *
 * Return Value
 *     true   Success
 *     false  Failure (fatal!)
 */
bool
Adapter_Init(void)
{
    // int nIRQ = -1;

    if (Adapter_IsInitialized != false)
    {
        LOG_WARN("Adapter_Init: Already initialized\n");
        return true;
    }

    // // trigger first-time initialization of the adapter
    // if (Device_Initialize(&nIRQ) < 0)
    //     return false;

    // if (!DMAResource_Init())
    // {
    //     // Device_UnInitialize();
    //     return false;
    // }


#if defined(ADAPTER_EIP28_INTERRUPT_ENABLE)
    if (!Adapter_Interrupts_Init(nIRQ))
    {
        LOG_CRIT("Adapter_Init: Adapter_Interrupts_Init failed\n");

        DMAResource_UnInit();

        Device_UnInitialize();

        return false;
    }
#endif

    Adapter_IsInitialized = true;

    return true;    // success
}


/*----------------------------------------------------------------------------
 * Adapter_UnInit
 */
void
Adapter_UnInit(void)
{
#if defined(ADAPTER_EIP28_INTERRUPT_ENABLE)
    int IRQ = 0;
#endif

    if (!Adapter_IsInitialized)
    {
        LOG_WARN("Adapter_UnInit: Adapter is not initialized\n");
        return;
    }

    Adapter_IsInitialized = false;

    Adapter_EIP28_UnInit();

#if defined(ADAPTER_EIP28_INTERRUPT_ENABLE)
    Adapter_Interrupts_UnInit(IRQ);
#endif

    // DMAResource_UnInit();

    // Device_UnInitialize();
}


/*----------------------------------------------------------------------------
 * Adapter_Report_Build_Params
 */
void
Adapter_Report_Build_Params(void)
{
    // This function is dependent on config file cs_adapter.h.
    // Please update this when Config file for Adapter is changed.
    LOG_INFO("Adapter build configuration:\n");

#define REPORT_SET(_X) \
    LOG_INFO("\t" #_X "\n")

#define REPORT_STR(_X) \
    LOG_INFO("\t" #_X ": %s\n", _X)

#define REPORT_INT(_X) \
    LOG_INFO("\t" #_X ": %d\n", _X)

#define REPORT_HEX32(_X) \
    LOG_INFO("\t" #_X ": 0x%08X\n", _X)

#define REPORT_EQ(_X, _Y) \
    LOG_INFO("\t" #_X " == " #_Y "\n")

#define REPORT_EXPL(_X, _Y) \
    LOG_INFO("\t" #_X _Y "\n")

    REPORT_INT(ADAPTER_MAX_DMARESOURCE_HANDLES);
#ifdef ADAPTER_REMOVE_BOUNCEBUFFERS
    REPORT_EXPL(ADAPTER_REMOVE_BOUNCEBUFFERS, " is SET => Bounce DISABLED");
#else
    REPORT_EXPL(ADAPTER_REMOVE_BOUNCEBUFFERS, " is NOT set => Bounce ENABLED");
#endif
    // PKA / EIP28
    LOG_INFO("PKA / EIP28:\n");
#ifdef ADAPTER_PKA_STRICT_ARGS
    REPORT_SET(ADAPTER_PKA_STRICT_ARGS);
#endif
#ifdef ADAPTER_REMOVE_EIP28_FIRMWARE
    REPORT_SET(ADAPTER_REMOVE_EIP28_FIRMWARE);
#endif
    REPORT_INT(ADAPTER_PKA_VECTOR_MAX_WORDS);
    //REPORT_INT(ADAPTER_EIP28_PKA_RAM_NR_OF_WORDS);
#ifdef ADAPTER_EIP28_INTERRUPT_ENABLE
    REPORT_SET(ADAPTER_EIP28_INTERRUPT_ENABLE);
#endif


    // Log
    LOG_INFO("Logging:\n");
#if (LOG_SEVERITY_MAX == LOG_SEVERITY_INFO)
    REPORT_EQ(LOG_SEVERITY_MAX, LOG_SEVERITY_INFO);
#elif (LOG_SEVERITY_MAX == LOG_SEVERITY_WARNING)
    REPORT_EQ(LOG_SEVERITY_MAX, LOG_SEVERITY_WARNING);
#elif (LOG_SEVERITY_MAX == LOG_SEVERITY_CRITICAL)
    REPORT_EQ(LOG_SEVERITY_MAX, LOG_SEVERITY_CRITICAL);
#else
    REPORT_EXPL(LOG_SEVERITY_MAX, " - Unknown (not info/warn/crit)");
#endif

    // Adapter other
    LOG_INFO("Other:\n");
    REPORT_STR(ADAPTER_DRIVER_NAME);
    REPORT_STR(ADAPTER_LICENSE);
    REPORT_HEX32(ADAPTER_INTERRUPTS_TRACEFILTER);
}


/* end of file adapter_init.c */

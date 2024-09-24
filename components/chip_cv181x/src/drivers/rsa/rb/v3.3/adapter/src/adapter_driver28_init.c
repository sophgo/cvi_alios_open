/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */
/* adapter_driver28_init.c
 *
 * Adapter top level module, Security-IP-28 driver's entry point.
 */



/*----------------------------------------------------------------------------
 * This module implements (provides) the following interface(s):
 */

#include "api_driver28_init.h"    // Driver Init API


/*----------------------------------------------------------------------------
 * This module uses (requires) the following interface(s):
 */

// Default Adapter configuration
#include "c_adapter.h"      // ADAPTER_DRIVER_NAME

// Adapter Initialization API
#include "adapter_init.h"   // Adapter_*

// Logging API
#include "rambus_log.h"            // LOG_INFO


/*----------------------------------------------------------------------------
 * Driver28_Init
 */
int
Driver28_Init(void)
{
    LOG_INFO("%s driver: initializing\n", ADAPTER_DRIVER_NAME);

    Adapter_Report_Build_Params();

    if (!Adapter_Init())
    {
        return -1;
    }

    return 0;   // success
}


/*----------------------------------------------------------------------------
 * Driver28_Exit
 */
void
Driver28_Exit(void)
{
    LOG_INFO("%s driver: exit\n", ADAPTER_DRIVER_NAME);

    Adapter_UnInit();
}

//#include "adapter_driver28_init_ext.h"

/* end of file adapter_driver28_init.c */

/*
 * Copyright (C) 2017-2020 Alibaba Group Holding Limited
 */
/* adapter_eip28.c
 *
 * Adapter module responsible for the EIP28.
 */



#include "c_adapter.h"
#include "basic_defs.h"
#include "clib.h" // memset
#include "adapter_eip28.h"
#include "device_types.h"
#include "device_mgmt.h"
#include "device_rw.h"
#include "adapter_firmware.h"
#include <drv/rsa.h>
#include <drv/common.h>
#include <rambus.h>

#ifdef ADAPTER_EIP28_INTERRUPT_ENABLE
#include "adapter_interrupts.h" // Adapter_Interrupt*
#endif                          // ADAPTER_EIP28_INTERRUPT_ENABLE

#include "eip28.h"

#include "stdbool.h"
// #ifndef ADAPTER_EIP28_REMOVE_FIRMWARE_DOWNLOAD
// #include "adapter_firmware.h"
// #endif

EIP28_IOArea_t Adapter_EIP28_IOArea;
bool Adapter_EIP28_IsUsable = false;

/*----------------------------------------------------------------------------
 * Adapter_EIP28_Init
 */
bool Adapter_EIP28_Init()
{
    EIP28_Status_t res;
    Adapter_Firmware_t FW_Handle;
    uint32_t * Image_p;
    unsigned int WordCount;
    EIP28_RevisionInfo_t Rev_Info;

    if (Adapter_EIP28_IsUsable)
        return true;

    // Device_EIP28 = Device_Find("EIP28");
    // if (Device_EIP28 == NULL)
    // {
    //     LOG_CRIT("Adapter_EIP28_Init: Failed to locate device EIP28\n" );
    //     return false;
    // }
    struct csi_dev rsa_dev;
    if (0 != target_get(DEV_RAMBUS_150B_PKA_TAG, 0, &rsa_dev))
    {
        TRACE_ERR(SC_DRV_FAILED);
        return false;
    }
    Device_Handle_t Device_EIP28 = (Device_Handle_t)rsa_dev.reg_base;

    FW_Handle = Adapter_Firmware_Acquire(ADAPTER_EIP28_FIRMWARE_NAME,
                                            &Image_p,
                                            &WordCount);

    if (FW_Handle == NULL)
    {
        LOG_CRIT("Adapter_EIP28_Init: Adapter_Firmware_Acquire() could not "
                        "load FW file %s!\n", ADAPTER_EIP28_FIRMWARE_NAME);
        return false;
    }

    LOG_INFO("Loading EIP28 firmware with size: %d\n", WordCount);

    res = EIP28_Initialize_CALLATOMIC(
                    &Adapter_EIP28_IOArea,
                    Device_EIP28,
                    Image_p,
                    WordCount);

    Adapter_Firmware_Release(FW_Handle);
    if (res != EIP28_STATUS_OK)
    {

        LOG_CRIT("err %s, %d, 0x%x\n", __FUNCTION__, __LINE__, res);
        return false;
    }

    EIP28_GetRevisionInfo(&Adapter_EIP28_IOArea, &Rev_Info);

    LOG_INFO("Successfully initialized EIP28 v%d.%d with FW v%d.%d\n",
             Rev_Info.HW.Major, Rev_Info.HW.Minor,
             Rev_Info.FW.Major, Rev_Info.FW.Minor);

    WordCount = EIP28_Memory_GetWordCount(&Adapter_EIP28_IOArea);

    LOG_INFO("PKA RAM 32-bit word count = %d\n", WordCount);

    Adapter_EIP28_IsUsable = true;

    return true;
}

/*----------------------------------------------------------------------------
 * Adapter_EIP28_UnInit
 */
void Adapter_EIP28_UnInit(void)
{
#ifdef ADAPTER_EIP28_INTERRUPT_ENABLE
    // mask interrupts
    Adapter_Interrupt_Disable(IRQ_EIP28_READY, 0);
#endif /* ADAPTER_EIP28_INTERRUPTS_ENABLE */

    Adapter_EIP28_IsUsable = false;
}

/* end of file adapter_eip28.c */

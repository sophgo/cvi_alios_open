/*
 * Copyright (c) 2019, Cvitek. All rights reserved.
 *
 */
#ifndef __CV_EFUSE_H
#define __CV_EFUSE_H

#include <platform.h>

#define SEC_EFUSE_BASE (SEC_SUBSYS_BASE + 0x000C0000)
#define SEC_EFUSE_SHADOW_REG (SEC_EFUSE_BASE + 0x100)

#define EFUSE_SCS_CONFIG_REG (SEC_EFUSE_SHADOW_REG + 0xA0)
#define BIT_SCS_ENABLE 0
#define BIT_TEE_SCS_ENABLE 2
#define BIT_BOOT_LOADER_ENCRYPTION 6

#endif /*__CV_EFUSE_H*/

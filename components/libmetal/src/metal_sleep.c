/*
 * Copyright (c) 2024, Cvitek Co. Ltd. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * AliOS adaptation: sleep using aos_msleep.
 */

#include <metal/sleep.h>
#include <aos/kernel.h>

int metal_sleep_usec_impl(unsigned int usec)
{
    if (usec < 1000) {
        /* For sub-millisecond delays, use aos_msleep(1) as minimum */
        aos_msleep(1);
    } else {
        aos_msleep(usec / 1000);
    }
    return 0;
}

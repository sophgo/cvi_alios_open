/*
 * Copyright (C) 2017 C-SKY Microsystems Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions an
 * limitations under the License.
 */

#include <autotest.h>
#include <test_config.h>
#include <test_log.h>
#include <test_common.h>
#include <string.h>
#include <aos/kernel.h>

#if defined(CONFIG_KERNEL) || defined(CONFIG_STRESS)
#include <csi_kernel.h>
#endif

#ifndef CONFIG_INIT_TASK_STACK_SIZE
#define INIT_TASK_STACK_SIZE 8192
#else
#define INIT_TASK_STACK_SIZE CONFIG_INIT_TASK_STACK_SIZE
#endif

volatile char _mc_name[MODULES_LENGTH] = {'\0'}; // define module and testcase name
volatile int _test_result = 0;
static aos_task_t app_task_handle;
extern void test_init(void);
extern void stress_test_init(void);

int test_entry(void)
{


#if defined(CONFIG_KERNEL) || defined(CONFIG_STRESS)
    k_status_t status;
    k_task_handle_t t_handle = NULL;

    status = csi_kernel_init();

    if (status != 0) {
        while (1)
            ;
    }

#if defined(CONFIG_STRESS)
    status = csi_kernel_task_new((k_task_entry_t)stress_test_init, NULL, NULL, KPRIO_HIGH7, 0, NULL, 2048, &t_handle);
#else
    status = csi_kernel_task_new((k_task_entry_t)test_init, NULL, NULL, KPRIO_LOW3, 0, NULL, 2048, &t_handle);
#endif

    if (status != 0) {
        while (1)
            ;
    }


    status = csi_kernel_start();

    if (status != 0) {
        while (1)
            ;
    }

#endif

#if defined(CONFIG_DRIVER) || defined(CONFIG_CORE)
    //	extern void mm_heap_initialize();
    //    mm_heap_initialize();
    test_init();
#endif

    return 0;
}

static void application_task_entry(void *arg)
{
    test_entry();

    aos_task_exit(0);
}

int __driver_test_service__(void)
{
    /* kernel init */
    aos_init();
#ifdef CONFIG_OS_TRACE
    trace_init_data();
#endif

    /* init task */
    aos_task_new_ext(&app_task_handle, "app_task", application_task_entry,
                     NULL, INIT_TASK_STACK_SIZE, AOS_DEFAULT_APP_PRI);

    /* kernel start */
    aos_start();

    return 0;
}

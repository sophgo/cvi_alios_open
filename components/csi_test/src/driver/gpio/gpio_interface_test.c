/*
 * Copyright (C) 2020 C-SKY Microsystems Co., Ltd. All rights reserved.
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

#include <gpio_test.h>

static void gpio_callback(csi_gpio_t *hd, uint32_t pins, void *arg)
{
    ;
}

int test_gpio_interface(void *args)
{
    csi_error_t ret_status;
    csi_gpio_t  gpio_hd;

    TEST_CASE_READY();

    /* Call the csi_gpio_init function for interface testing */

    ret_status = csi_gpio_init(NULL, 1);
    TEST_CASE_ASSERT(ret_status == -1, "parameter: gpio is NULL, csi_gpio_init expects -1 returned, actual %d", ret_status);

    ret_status = csi_gpio_init(&gpio_hd, 0xFFFFFFFF);
    TEST_CASE_ASSERT(ret_status == -1,
                     "parameter: port_idx(0xFFFFFFFF) is out of range, csi_gpio_init expects -1 returned, actual %d", ret_status);

    /* Call the csi_gpio_uninit function for interface testing */

    csi_gpio_uninit(NULL);

    /* Call the csi_gpio_attach_callback function for interface testing */

    ret_status = csi_gpio_attach_callback(NULL, gpio_callback, NULL);
    TEST_CASE_ASSERT(ret_status == -1, "parameter: gpio is NULL, csi_gpio_attach_callback expects -1 returned, actual %d",
                     ret_status);

    ret_status = csi_gpio_attach_callback(&gpio_hd, NULL, NULL);
    TEST_CASE_ASSERT(ret_status == 0, "parameter: callback is NULL, csi_gpio_attach_callback expects 0 returned, actual %d",
                     ret_status);

    /* Call the csi_gpio_debounce function for interface testing */

    ret_status = csi_gpio_debounce(NULL, 1, 1);
    TEST_CASE_ASSERT(ret_status == -1, "parameter: gpio is NULL, csi_gpio_debounce expects -1 returned, actual %d",
                     ret_status);

    /* Call the csi_gpio_dir function for interface testing */

    ret_status = csi_gpio_dir(NULL, 1, 1);
    TEST_CASE_ASSERT(ret_status == -1, "parameter: gpio is NULL, csi_gpio_dir expects -1 returned, actual %d", ret_status);

    /* Call the csi_gpio_irq_enable function for interface testing */

    ret_status = csi_gpio_irq_enable(NULL, 1, 1);
    TEST_CASE_ASSERT(ret_status == -1, "parameter: gpio is NULL, csi_gpio_irq_enable expects -1 returned, actual %d",
                     ret_status);

    /* Call the csi_gpio_irq_mode function for interface testing */

    ret_status = csi_gpio_irq_mode(NULL, 1, 1);
    TEST_CASE_ASSERT(ret_status == -1, "parameter: gpio is NULL, csi_gpio_irq_mode expects -1 returned, actual %d",
                     ret_status);


    /* Call the csi_gpio_mode function for interface testing */

    ret_status = csi_gpio_mode(NULL, 1, 1);
    TEST_CASE_ASSERT(ret_status == -1, "parameter: gpio is NULL, csi_gpio_mode expects -1 returned, actual %d", ret_status);


    /* Call the csi_gpio_read function for interface testing */

    ret_status = csi_gpio_read(NULL, 1);
    TEST_CASE_ASSERT(ret_status == -1, "parameter: gpio is NULL, csi_gpio_read expects -1 returned, actual %d", ret_status);

#ifdef CONFIG_PM
    /* Call the csi_gpio_enable_pm function for interface testing */

    csi_gpio_disable_pm(NULL);

    ret_status = csi_gpio_enable_pm(NULL);
    TEST_CASE_ASSERT(ret_status == -1, "parameter: gpio is NULL, csi_gpio_enable_pm expects -1 returned, actual %d",
                     ret_status);

    ret_status = dw_gpio_pm_action(NULL, 0);
    TEST_CASE_ASSERT(ret_status == -1, "parameter: dev is NULL, dw_gpio_pm_action expects -1 returned, actual %d",
                     ret_status);


#endif


    csi_gpio_uninit(NULL);

    return 0;
}

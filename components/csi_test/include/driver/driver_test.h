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

#ifndef __DRIVER_TEST__
#define __DRIVER_TEST__



extern int test_pinmux_main(char *args);
extern int test_uart_main(char *args);
extern int test_gpio_main(char *args);
extern int test_pwm_main(char *args);
extern int test_dma_main(char *args);
extern int test_wdt_main(char *args);
extern int test_codec_main(char *args);
extern int test_spi_main(char *args);
extern int test_spiflash_main(char *args);
extern int test_timer_main(char *args);
extern int test_i2s_main(char *args);
extern int test_iic_main(char *args);
extern int test_adc_main(char *args);
extern int test_rtc_main(char *args);
extern int test_mbox_main(char *args);
extern int test_efuse_main(char *args);
extern int test_intnest_main(char *args);
extern int test_pm_main(char *args);
extern int test_pmu_main(char *args);
extern int test_baudcalc_main(char *args);
extern int test_tipc_main(char *args);
extern int test_sasc_main(char *args);
extern int test_rsa_main(char *args);
extern int test_aes_main(char *args);
extern int test_sha_main(char *args);
extern int test_trng_main(char *args);
extern int test_spinand_main(char *args);
extern int test_mmc_main(char *args);
extern int test_eflash_main(char *args);
extern int test_jupiter_pmu_main(char *args);
extern int test_irq_main(void *args);
extern int test_gpio_pin_main(char *args);
extern int test_pin_main(char *args);

#endif
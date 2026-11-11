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

#ifndef __IRQ_TEST__
#define __IRQ_TEST__

#include <stdint.h>
#include <soc.h>
#include <string.h>
#include <stdlib.h>
#include <autotest.h>
#include <test_log.h>
#include <test_config.h>
#include <test_common.h>
#include <csi_core.h>


int test_irq_handle(void *args);
int test_irq_stack(void *args);
int test_irq_tail(void *args);
int test_irq_vector_speedup(void *args);
int test_irq_vic(void *args);


void irqTail_Handler_3(void);
void irqTail_Handler_2(void);
void irqTail_Handler_1(void);
void vectorSpeedUp_Handler_1(void);
void vectorSpeedUp_Handler_2(void);

int timer_start(uint8_t idx);
uint32_t timer_stop(void);

extern uint32_t save_mepc;

#define IRQ_STACK_LENGTH (1024 * 10) //中断栈大小

#endif

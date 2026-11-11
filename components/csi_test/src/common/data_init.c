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

#include <stdint.h>
#include <stdlib.h>
void transfer_data(char *addr, uint32_t size)
{
    uint32_t i;

    for (i = 0; i < size; i++) {
        *(addr + i) = i % 32;
    }
}

void dataset(uint8_t *addr, uint32_t size, uint8_t pattern)
{
    uint32_t i;

    for (i = 0; i < size; i++) {
        *(addr + i) = pattern;
    }
}

void generate_rand_array(uint8_t *rand_data, uint8_t max_rand, uint32_t length)
{
    uint32_t i;

    for (i = 0; i < length; i++) {
        rand_data[i] = rand() % (max_rand + 1);
    }
}

void generate_rand_array2(char *rand_data, uint8_t max_rand, uint32_t length)
{
    uint32_t i;

    for (i = 0; i < length; i++) {
        rand_data[i] = rand() % (max_rand + 1);
    }
}
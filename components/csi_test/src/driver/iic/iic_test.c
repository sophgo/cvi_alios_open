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

#include <iic_test.h>

const test_info_t iic_test_funcs_map[] = {
    {"IIC_INTERFACE", test_iic_interface, 0},
    {"IIC_MASTER_ASYNC_SEND", test_iic_masterAsyncSend, 5},
    {"IIC_MASTER_SYNC_SEND", test_iic_masterSyncSend, 6},
    {"IIC_MASTER_DMA_SEND", test_iic_masterDmaSend, 5},
    {"IIC_SLAVE_ASYNC_SEND", test_iic_slaveAsyncSend, 5},
    {"IIC_SLAVE_SYNC_SEND", test_iic_slaveSyncSend, 6},
    {"IIC_SLAVE_DMA_SEND", test_iic_slaveDmaSend, 5},
    {"IIC_MASTER_ASYNC_RECEIVE", test_iic_masterAsyncReceive, 5},
    {"IIC_MASTER_SYNC_RECEIVE", test_iic_masterSyncReceive, 6},
    {"IIC_MASTER_DMA_RECEIVE", test_iic_masterDmaReceive, 5},
    {"IIC_SLAVE_ASYNC_RECEIVE", test_iic_slaveAsyncReceive, 5},
    {"IIC_SLAVE_SYNC_RECEIVE", test_iic_slaveSyncReceive, 6},
    {"IIC_SLAVE_DMA_RECEIVE", test_iic_slaveDmaReceive, 5},
    {"IIC_MEMORY_TRANSFER", test_iic_memoryTransfer, 8},
    {"IIC_GET_STATE_MASTER_SEND", test_iic_getStateMasterSend, 5},
    {"IIC_GET_STATE_SLAVE_RECEIVE", test_iic_getStateSlaveReceive, 5}
};

int test_iic_main(char *args)
{
    int ret;
    ret = testcase_jump(args, (void *)iic_test_funcs_map);

    if (ret != 0) {
        TEST_CASE_TIPS("IIC don't supported this command");
    }

    return ret;
}


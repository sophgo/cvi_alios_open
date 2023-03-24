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

#include "i2s_test.h"
test_i2s_args_t i2s_args;

const test_info_t i2s_test_funcs_map[] = {
    {"I2S_INTERFACE", test_i2s_interface, 0},
    {"I2S_SYNC_SEND_S", test_i2s_sync_send, 8},
	{"I2S_SYNC_SEND_M", test_i2s_sync_send, 8},
    {"I2S_SYNC_RECEIVE_S", test_i2s_sync_receive, 8},
	{"I2S_SYNC_RECEIVE_M", test_i2s_sync_receive, 8},
    {"I2S_ASYNC_SEND_S", test_i2s_sync_send, 8},
	{"I2S_ASYNC_SEND_M", test_i2s_sync_send, 8},
    {"I2S_ASYNC_RECEIVE_S", test_i2s_async_receive, 8},
	{"I2S_ASYNC_RECEIVE_M", test_i2s_async_receive, 8},
    {"I2S_DMA_SEND_S", test_i2s_dma_send, 8},
	{"I2S_DMA_SEND_M", test_i2s_dma_send, 8},
    {"I2S_DMA_RECEIVE_S", test_i2s_dma_receive, 8},
	{"I2S_DMA_RECEIVE_M", test_i2s_dma_receive, 8}
};

int test_i2s_main(char *args){

    int mc_len = 0;
    mc_len = strlen((void *)_mc_name) ;

//    //rate,width,protocol,polarity,sound_ch,period,sclk_freq,mclk_freq
//    //48000,16,0,0,0,1024,32,256
//    //argc = sscanf(args, "%d,%d,%d,%d,%d,%d,%d,%d", &i2s_args.rate, &i2s_args.width, &i2s_args.protocol, &i2s_args.polarity,&i2s_args.sound_ch,&i2s_args.period,&i2s_args.sclk_freq, &i2s_args.mclk_freq);
//    sscanf(args, "%d,%d,%d,%d,%d,%d,%d,%d", (int *)&i2s_args.idx, (int *)&i2s_args.rate, (int *)&i2s_args.width, (int *)&i2s_args.protocol, (int *)&i2s_args.polarity, &i2s_args.period, (int *)&i2s_args.sclk_freq, (int *)&i2s_args.mclk_freq);
//    i2s_args.idx = 1;
//    TEST_CASE_TIPS("idx: %d, rate: %d,width:%d, protocol: %d, polarity: %d, period: %d, sclk_freq: %d, mclk_freq: %d",i2s_args.idx, i2s_args.rate, i2s_args.width, i2s_args.protocol, i2s_args.polarity, i2s_args.period, i2s_args.sclk_freq, i2s_args.mclk_freq);
//    //if (argc == 0 && strcmp(_mc_name,"I2S_INTERFACE")!=0 ){
//    //    TEST_CASE_ASSERT_QUIT(1==0, "must pass arguments for command %s", _mc_name);
//    // }

    if (!strcmp((void *)(_mc_name + mc_len - 2), "_S")){
        i2s_args.mode = I2S_MODE_SLAVE;
    }else{
        i2s_args.mode = I2S_MODE_MASTER;
    }

//    if(!strcmp((void *)_mc_name,"I2S_INTERFACE")){
//        test_i2s_interface();
//        return 0;
//    }
//    
//    if(!strcmp((void *)_mc_name,"I2S_STATUS")){
//        test_i2s_sync_status(args);
//        return 0;
//    }
//
//    //AT+I2S_SYNC_SEND_S, AT+I2S_SYNC_SEND_M
//    if(strstr((void *)_mc_name,"I2S_SYNC_SEND") !=NULL){
//        test_i2s_sync_send(&i2s_args);
//        return 0;
//    }
//
//    if(strstr((void *)_mc_name,"I2S_SYNC_RECEIVE") !=NULL){
//        test_i2s_sync_receive(&i2s_args);
//        return 0;
//    }
//    
//    if(strstr((void *)_mc_name,"I2S_ASYNC_SEND")!=NULL){
//        test_i2s_async_send(&i2s_args);
//        return 0;
//    }
//
//    if(strstr((void *)_mc_name,"I2S_ASYNC_RECEIVE")!=NULL){
//        test_i2s_async_receive(&i2s_args);
//        return 0;
//    }
//
//    if(strstr((void *)_mc_name,"I2S_DMA_SEND")!=NULL){
//        test_i2s_dma_send(&i2s_args);
//        return 0;
//    }
//
//    if(strstr((void *)_mc_name,"I2S_DMA_RECEIVE")!=NULL){
//        test_i2s_dma_receive(&i2s_args);
//        return 0;
//    }
// 
//    return -1;
	
	int ret;
    ret = testcase_jump(args, (void *)i2s_test_funcs_map);

    if (ret != 0) {
        TEST_CASE_TIPS("I2S don't supported this command");
    }

    return ret;
	
}
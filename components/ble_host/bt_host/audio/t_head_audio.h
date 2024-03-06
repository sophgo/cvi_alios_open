/*
 * Copyright (c) 2018-2022 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/*
* Sample rate MUST and ONLY be 16k Hz
*
* This functions does initialization of audio process modules
*
* Input
*      - nsmode        : 0 ~ 5  mild -> aggressive
*      - aecmode       : 0 ~ 4  0 is no NLP(Non-linear processing) used
*                        1 ~ 4, NLP suppression, mild -> aggressive
*      - cnflag        : Add comfort noise or not
*                        0 for no comfort noise, 1 for comfort noise
*      - vadmode       : VAD Threshold level, 0 ~ 4  low -> top
*                        When calculation result of one frame is bigger than VAD Threshold, VAD output is 1
*                        When calculation result of one frame is not bigger than VAD Threshold, VAD output is 0
*
* Output:
*      - return value  : Initialize OK return 0; Initialize FAIL return < 0
*/
short T_Head_audio_init(short nsmode, short aecmode, short cnflag, short vadmode);

/*
* Sample rate MUST and ONLY be 16k Hz
* The input and output signals should always be 10ms per frame.
*
* This functions does audio process
*
* Input
*      - data_in       : Pointer to input data per frame
*      - data_far      : Pointer to far end(reference) data per frame
*
* Output:
*      - data_out      : Pointer to output data per frame, output channel num is 1~3 base on different lib
*/
short T_Head_audio_process(short *data_in, short *data_far, short *data_out);

/*
* This function releases the memory of audio process modules
* MUST and ONLY used when closing the system
*/
void T_Head_audio_free(void);

#ifdef __cplusplus
}
#endif


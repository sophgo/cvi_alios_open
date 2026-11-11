## CODEC

### 概要

该文档主要描述CODEC模块的CSI2.0自动化测试AT命令。 文档中所涉及的这些命令是用来测试CODEC的输入和输出以及状态读取等操作是否符合预期。

### 测试原理

测试该项目，只需要一块测试板通过串口工具和PC相连。测试时PC发送AT指令启动测试，打印OK说明测试成功

### 测试流程

- 测试前应当准备好测试板
- 发送CODEC测试命令进行测试 （测试命令功能请参照下文）

### CODEC功能AT命令列表概览

以下是CODEC模块对应的测试命令列表及其功能简要介绍。

| AT命令列表                    | 功能                                                         |
| ----------------------------- | ------------------------------------------------------------ |
| AT+CODEC_INTERFACE            | CODEC接口异常测试。                                          |
| AT+CODEC_SYNC_OUTPUT          | 测试CODEC模块同步输出。                                      |
| AT+CODEC_SYNC_INPUT           | 测试CODEC模块同步输入。                                      |
| AT+CODEC_ASYNC_OUTPUT         | 测试CODEC模块异步输出。                                      |
| AT+CODEC_ASYNC_INPUT          | 测试CODEC模块异步输入。                                      |
| AT+CODEC_OUTPUT_CHANNEL_STATE | 测试CODEC模块写状态。用DMA同步方式输出一段数据，在过程中测试写状态是否正确 |
| AT+CODEC_INPUT_CHANNEL_STATE  | 测试CODEC模块读状态。用DMA同步方式输入一段数据，在过程中测试读状态是否正确 |
| AT+CODEC_OUTPUT_BUFFER        | 测试CODEC模块输出缓存的剩余数量。分别在同步模式和异步模式输出一段数据。看缓存的剩余数量和预期是否一致 |
| AT+CODEC_OUTPUT_MUTE          | 测试CODEC模块输出通道的静音。在DMA异步输出时启动静音功能。通过返回值确定输出是否正确 |
| AT+CODEC_INPUT_MUTE           | 测试CODEC模块输入通道的静音。在DMA异步输入时启动静音功能。通过返回值确定输入是否正确 |
| AT+CODEC_PAUSE_RESUME         | 测试CODEC模块输出通道暂停和恢复功能。用DMA同步方式输出一段数据。输出时启用暂停和恢复功能，通过读取输出的剩余缓存来确定暂停和恢复功能的有效性 |

### 参数列表

以下是CODEC模块对应的AT测试命令共用参数的简要介绍。（buffer_size参数仅供参考，如遇系统异常退出，请按实际项目参数设置相应的buffer_size）

| 参数        | 释义                           | 取值范围                                                     |
| --------------- | ------------------------------ | ------------------------------------------------------------ |
| idx             | 表示设备号，0表示codec0        | 0、1、...，具体可测编号需参考soc.h                                     |
| ch_idx          | 表示通道号，比如0表示0通道 | 0、1、...，具体可测通道号需参考soc.h                                    |
| sample_rate     | 表示采样率，Hz                 | 8000、11025、12000<br/>16000、22050、24000<br/>32000、44100、48000<br/>96000、192000、256000 |
| bit_width       | 表示采样的宽度，bit            | 16、24、32                                                   |
| sound_track_num | 表示声道数                     | 1、2、...                                                    |
| data_size       | 表示需要的数据长度             | 0x0 - 0xFFFFFFFF                                             |
| buffer_size     | 表示缓冲区的数据长度           | 0x0 - 0xFFFFFFFF                                             |
| digital_gain    | 表示通道的数码增益             | 0x0 - 0xFFFFFFFF                                             |
| analog_gain     | 表示通道的模拟增益             | 0x0 - 0xFFFFFFFF                                             |
| mix_gain        | 表示通道的混频增益             | 0x0 - 0xFFFFFFFF                                             |
| period          | 表示发送/接收多少数据上报周期  | 0x0 - 0xFFFFFFFF                                             |
| mode            | 表示输入/输出模式              | 0：单端模式<br/>1：差分模式                                  |

### 命令详解

#### AT+CODEC_INTERFACE

| 命令格式 | AT+CODEC_INTERFACE                                           |
| -------- | ------------------------------------------------------------ |
| 命令功能 | CODEC接口异常测试。该命令测试CODEC所CSI接口的异常调用，测试时将会传入异常和非法参数，验证CSI接口是否返回相应的错误码。 |
| 示例     | 步骤1:发送测试命令到测试板：<br>AT+CODEC_INTERFACE<br>测试完成后，板子对应串口会打印OK；否则测试失败 |

#### AT+CODEC_SYNC_OUTPUT

| 命令格式 | AT+CODEC_SYNC_OUTPUT=idx,ch_idx,sample_rate,bit_width,sound_track_num,<br/>data_size,buffer_size,digital_gain,analog_gain,mix_gain,period,mode |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CODEC模块同步输出。用同步模式输出一段数据，通过返回值和回调确定输出是否成功。 |
| 示例     | 步骤1:发送测试命令到测试板：<br>AT+CODEC_SYNC_OUTPUT=0,0,48000,16,1,2048,52800,0,46,0,512,1<br>这表示外设为codec0,通道序号为0，采样波特率为48000，16位采样宽度，声道数为1，需要发送的数据大小为2K，缓冲区大小为1K,数字增益为0 db,模拟增益为46db,混频增益为0，发送上报周期为512，设置模式为差分<br>测试完成后， 板子对应串口会打印OK；否则测试失败 |

#### AT+CODEC_SYNC_INPUT

| 命令格式 | AT+CODEC_SYNC_INPUT=idx,ch_idx,sample_rate,bit_width,sound_track_num,<br/>data_size,buffer_size,digital_gain,analog_gain,mix_gain,period,mode |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CODEC模块同步输入。用同步模式输入一段数据，通过返回值和回调确定输入是否成功。 |
| 示例     | 步骤1:发送测试命令到测试板：<br>AT+CODEC_SYNC_INPUT=0,0,48000,16,1,2048,52800,0,46,0,512,1<br>这表示外设为codec0,通道序号为0，采样波特率为48000，16位采样宽度，声道数为1，需要发送的数据大小为2K，缓冲区大小为1K,数字增益为0db,模拟增益为46db,混频增益为0，发送上报周期为512，设置模式为差分<br>测试完成后， 板子对应串口会打印OK；否则测试失败 |

#### AT+CODEC_ASYNC_OUTPUT

| 命令格式 | AT+CODEC_ASYNC_OUTPUT=idx,ch_idx,sample_rate,bit_width,sound_track_num, data_size,buffer_size,digital_gain,analog_gain,mix_gain,period,mode |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CODEC模块同步输出。用异步模式输出一段数据，通过返回值和回调确定输出是否成功。 |
| 示例     | 步骤1:发送测试命令到测试板： AT+CODEC_ASYNC_OUTPUT=0,0,48000,16,1,2048,52800,0,46,0,512,1<br/>这表示外设为codec0,通道序号为0，采样波特率为48000，16位采样宽度，声道数为1，需要发送的数据大小为2K，缓冲区大小为1K,数字增益为0 db,模拟增益为46db,混频增益为0，发送上报周期为512，设置模式为差分 测试完成后， 板子对应串口会打印OK；否则测试失败 |

#### AT+CODEC_ASYNC_INPUT

| 命令格式 | AT+CODEC_ASYNC_INPUT=idx,ch_idx,sample_rate,bit_width,sound_track_num,<br/>data_size,buffer_size,digital_gain,analog_gain,mix_gain,period,mode |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CODEC模块异步输入。用异步模式输入一段数据，通过返回值和回调确定输入是否成功。 |
| 示例     | 步骤1:发送测试命令到测试板：<br>AT+CODEC_ASYNC_INPUT=0,0,48000,16,1,2048,52800,0,46,0,512,1<br>这表示外设为codec0,通道序号为0，采样波特率为48000，16位采样宽度，声道数为1，需要接收的数据大小为2K，缓冲区大小为1K,数字增益为0 db,模拟增益为46db,混频增益为0，接收上报周期为512，设置模式为差分<br>测试完成后， 板子对应串口会打印OK；否则测试失败 |

#### AT+CODEC_OUTPUT_CHANNEL_STATE

| 命令格式 | AT+CODEC_OUTPUT_CHANNEL_STATE=idx,ch_idx                     |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CODEC模块写状态。用同步模式输出一段数据，在过程中测试写状态是否正确。 |
| 示例     | 步骤1:发送测试命令到测试板：<br>AT+CODEC_OUTPUT_CHANNEL_STATE=0,0<br>这表示外设为codec0,通道序号为0<br>测试完成后， 板子对应串口会打印OK；否则测试失败 |

#### AT+CODEC_INPUT_CHANNEL_STATE

| 命令格式 | AT+CODEC_INPUT_CHANNEL_STATE=idx,ch_idx                      |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CODEC模块读状态。用同步模式输入一段数据，在过程中测试读状态是否正确。 |
| 示例     | 步骤1:发送测试命令到测试板：<br>AT+CODEC_INPUT_CHANNEL_STATE=0,0<br>这表示外设为codec0,通道序号为0<br>测试完成后， 板子对应串口会打印OK；否则测试失败 |

#### AT+CODEC_OUTPUT_BUFFER

| 命令格式 | AT+CODEC_OUTPUT_BUFFER=idx,ch_idx,buffer_size                |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CODEC模块输出缓存的剩余数量。分别在同步模式和异步模式输出一段数据，看缓存的剩余数量和预期是否一致。 |
| 示例     | 步骤1:发送测试命令到测试板：<br>AT+CODEC_OUTPUT_BUFFER=0,0,52800<br>这表示外设为codec0,通道序号为0，缓冲区大小为1024<br>测试完成后， 板子对应串口会打印OK；否则测试失败 |

#### AT+CODEC_OUTPUT_MUTE

| 命令格式 | AT+CODEC_OUTPUT_MUTE=idx,ch_idx,sample_rate,bit_width,sound_track_num,<br/>data_size,buffer_size,digital_gain,analog_gain,mix_gain,period,mode |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CODEC模块输出通道的静音。在异步模式输出时启动静音功能，通过返回值确定输出是否正确。 |
| 示例     | 步骤1:发送测试命令到测试板：<br>AT+CODEC_OUTPUT_MUTE=0,0,48000,16,1,2048,52800,0,46,0,512,1<br>这表示外设为codec0,通道序号为0，采样波特率为48000，16位采样宽度，声道数为1，需要发送的数据大小为2K，缓冲区大小为1K,数字增益为0 db,模拟增益为46db,混频增益为0，发送上报周期为512，设置模式为差分<br/>测试完成后， 板子对应串口会打印OK；否则测试失败 |

#### AT+CODEC_INPUT_MUTE

| 命令格式 | AT+CODEC_INPUT_MUTE=idx,ch_idx,sample_rate,bit_width,sound_track_num,<br/>data_size,buffer_size,digital_gain,analog_gain,mix_gain,period,mode |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CODEC模块输入通道的静音。在异步模式输入时启动静音功能，通过返回值确定输入是否正确 |
| 示例     | 步骤1:发送测试命令到测试板：<br>AT+CODEC_INPUT_MUTE=0,0,48000,16,1,2048,52800,0,46,0,512,1<br>这表示外设为codec0,通道序号为0，采样波特率为48000，16位采样宽度，声道数为1，需要接收的数据大小为2K，缓冲区大小为1K,数字增益为0 db,模拟增益为46db,混频增益为0，接收上报周期为512，设置模式为差分<br/>测试完成后， 板子对应串口会打印OK；否则测试失败 |

#### AT+CODEC_PAUSE_RESUME

| 命令格式 | AT+CODEC_PAUSE_RESUME=idx,ch_idx,sample_rate,bit_width,sound_track_num,<br/>data_size,buffer_size,mode |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CODEC模块输出通道暂停和恢复功能。用同步模式输出一段数据，输出时启用暂停和恢复功能，通过读取输出的剩余缓存来确定暂停和恢复功能的有效性。 |
| 示例     | 步骤1:发送测试命令到测试板：<br>AT+CODEC_PAUSE_RESUME=0,0,48000,16,1,2048,52800,1<br>这表示外设为codec0,通道序号为0，采样波特率为48000，16位采样宽度，声道数为1，需要发送的数据大小为2K，缓冲区大小为1K,设置模式为差分<br/>测试完成后， 板子对应串口会打印OK；否则测试失败 |

#### AT+CODEC_SYNC_OUTPUT_4K

| 命令格式 | AT+CODEC_SYNC_OUTPUT_4K
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CODEC模块同步输出功能。用同步模式按4K的长度循环往codec里写入准备好的数据，通过返回值和喇叭是否播放音乐来判断是否成功 |
| 示例     | 步骤1:发送测试命令到测试板：<br>AT+CODEC_SYNC_OUTPUT_4K<br>>测试完成后， 板子对应串口会打印OK；否则测试失败 |

#### AT+CODEC_SYNC_INPUT_5S

| 命令格式 | AT+CODEC_SYNC_INPUT_5S
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CODEC模块同步输入功能。用同步模式输入5秒钟的数据，即按设定长度持续读取codec的输入数据，保存到创建的malloc缓冲中，通过返回值来判断是否成功 |
| 示例     | 步骤1:发送测试命令到测试板：<br>AT+CODEC_SYNC_INPUT_5S<br>>测试完成后， 板子对应串口会打印OK；否则测试失败 |



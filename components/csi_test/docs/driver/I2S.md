

## I2S

### 概要

该文档主要描述I2S模块的CSI2.0自动化测试AT命令。 文档中所涉及的这些命令是用来测试I2S模块的接口工作是否正常、数据传输是否正确、接口返回值是否符合预期。

### 测试原理

测试时，应使被测试板与另外一块具备I2S功能的板子连接，并发送AT+PINMUX_CONFIG到被测试板把需要测试的引脚功能复用成I2S的引脚功能后，然后发送I2S测试命令进行相应的测试；测试程序会按照按照测试命令的配置进行相应的数据收发测试， 若接收方接收到的数据与预期一致，则相应的测试成功完成，否则测试失败。

### 测试流程

- 测试前应当准备好测试板（以下简称A板）、测试辅助板（以下简称B板）
- 选定需要测试的I2S针脚，并把A、B两块板子的I2S针脚相连
- 发送PINMUX_CONFIG命令到A板、B板， 把以上选定的引脚复用成I2S功能
- 发送I2S测试命令进行测试 （测试命令功能请参照下文）

### I2S功能AT命令列表概览

以下是I2S模块对应的测试命令列表及其功能简要介绍。

| AT命令列表    | 功能                            |
| ------------- | ------------------------------- |
| AT+I2S_INTERFACE | 接口测试 |
| AT+I2S_SYNC_SEND_M | 测试I2S作为master使用同步模式发送数据 |
| AT+I2S_SYNC_SEND_S | 测试I2S作为slave使用同步模式发送数据 |
| AT+I2S_SYNC_RECEIVE_M | 测试I2S作为master使用同步模式接收数据 |
| AT+I2S_SYNC_RECEIVE_S | 测试I2S作为slave使用同步模式接收数据 |
| AT+I2S_ASYNC_SEND_M | 测试I2S作为master使用异步模式发送数据 |
| AT+I2S_ASYNC_SEND_S | 测试I2S作为slave使用异步模式发送数据 |
| AT+I2S_ASYNC_RECEIVE_M | 测试I2S作为master使用异步模式接收数据 |
| AT+I2S_ASYNC_RECEIVE_S | 测试I2S作为slave使用异步模式接收数据 |
| AT+I2S_DMA_SEND_M | 测试I2S作为master使用DMA模式发送数据 |
| AT+I2S_DMA_SEND_S | 测试I2S作为slave使用DMA模式发送数据 |
| AT+I2S_DMA_RECEIVE_M | 测试I2S作为master使用DMA模式接收数据 |
| AT+I2S_DMA_RECEIVE_S | 测试I2S作为slave使用DMA模式接收数据 |


### 参数列表
以下是I2S模块对应的AT测试命令共用参数的简要介绍。

| 参数名字      | 释义              | 取值范围  |
| ------------- | ---------------- | --------- |
| idx           | 表示设备号,比如0表示i2s0 | 0、1、...，具体可测编号需参考soc.h |
| rate      | 表示配置的I2S的采样速率，Hz | 8000、11025、12000<br/>16000、22050、24000<br/>32000、44100、48000<br/>96000、192000、256000 |
| width     | 表示配置的I2S的采样宽度，bit | 16、24、32 |
| protocol     | 表示I2S配置的协议类型  | 0:I2S_PROTOCOL_I2S,I2S传输协议为I2S<br/>1:I2S_PROTOCOL_MSB_JUSTIFIED,I2S传输协议为MSB_JUSTIFIED<br/>2:I2S_PROTOCOL_LSB_JUSTIFIED,I2S传输协议为LSB_JUSTIFIED<br/>3:I2S_PROTOCOL_PCM,I2S传输协议为PCM |
| polarity      | 表示WS引脚电平极性对应的声道设置 | 0:I2S_LEFT_POLARITY_LOW，低电平对应左声道<br/>1:I2S_LEFT_POLARITY_HIGH，高电平对应左声道 |
| sclk_freq      | 表示配置I2S的sclk频率是采样频率的多少倍  | 16:I2S_SCLK_16FS，16倍<br/>32:I2S_SCLK_32FS，32倍<br/>48:I2S_SCLK_48FS，48倍<br/>64:I2S_SCLK_64FS，64倍 |
| mclk_freq | 表示配置I2S的mclk频率是采样频率的多少倍    | 256:I2S_MCLK_256FS，256倍<br/>384:I2S_MCLK_384FS，384倍 |

### 命令详解

#### AT+I2S_INTERFACE

| 命令格式 | AT+I2S_INTERFACE                                             |
| -------- | ------------------------------------------------------------ |
| 命令功能 | I2S接口异常测试。该命令测试I2S所CSI接口的异常调用，测试时将会传入异常和非法参数，验证CSI接口是否返回相应的错误码。 |
| 示例     | 发送命令AT+I2S_INTERFACE到A板，测试完成后，A板对应串口会打印OK；否则测试失败 |

#### AT+I2S_SYNC_SEND_M
| 命令格式 | AT+I2S_SYNC_SEND_M=idx,rate,width,protocol,polarity,sclk_freq,mclk_freq,size |
|------------------------------------------|-----------------------------|
| 命令功能 | 测试I2S作为master使用同步模式发送数据 |
| 配对命令 | AT+I2S_SYNC_RECEIVE_S;AT+I2S_ASYNC_RECEIVE_S;AT+I2S_DMA_RECEIVE_S |
| 示例 | 1.向B版发送命令:AT+I2S_ASYNC_RECEIVE_S=0,8000,16,0,0,16,256,2048<br>2.向A板发送命令:AT+I2S_SYNC_SEND_M=0,8000,16,0,0,16,256,2048<br>若测试通过，则连接A板和B板的串口最终会打印OK |

#### AT+I2S_SYNC_RECEIVE_S

| 命令格式 | AT+I2S_SYNC_RECEIVE_S=idx,rate,width,protocol,polarity,sclk_freq,mclk_freq,size |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试I2S作为slave使用同步模式发送数据                         |
| 配对命令 | AT+I2S_SYNC_SEND_M;AT+I2S_ASYNC_SEND_M;AT+I2S_DMA_SEND_M     |
| 示例     | 1.向A版发送命令:AT+I2S_SYNC_RECEIVE_S=0,8000,16,0,0,16,256,2048<br>2.向B板发送命令:AT+I2S_SYNC_SEND_M=0,8000,16,0,0,16,256,2048<br>若测试通过，则连接A板和B板的串口最终会打印OK |

#### AT+I2S_SYNC_SEND_S

| 命令格式 | AT+I2S_SYNC_SEND_S=idx,rate,width,protocol,polarity,sclk_freq,mclk_freq,size |
|------------------------------------------|-----------------------------|
| 命令功能 | 测试I2S作为slave使用同步模式发送数据 |
| 配对命令 | AT+I2S_SYNC_RECEIVE_M;AT+I2S_ASYNC_RECEIVE_M;AT+I2S_DMA_RECEIVE_M |
| 示例 | 1.向A版发送命令:AT+I2S_SYNC_SEND_S=0,8000,16,0,0,16,256,2048<br>2.向B板发送命令:AT+I2S_SYNC_RECEIVE_M=0,8000,16,0,0,16,256,2048<br>若测试通过，则连接A板和B板的串口最终会打印OK |

#### AT+I2S_SYNC_RECEIVE_M
| 命令格式 | AT+I2S_SYNC_RECEIVE_M=idx,rate,width,protocol,polarity,sclk_freq,mclk_freq,size |
|------------------------------------------|-----------------------------|
| 命令功能 | 测试I2S作为master使用同步模式发送数据 |
| 配对命令 | AT+I2S_SYNC_SEND_S;AT+I2S_ASYNC_SEND_S;AT+I2S_DMA_SEND_S |
| 示例 | 1.向B版发送命令:AT+I2S_ASYNC_SEND_S=0,8000,16,0,0,16,256,2048<br>2.向A板发送命令:AT+I2S_SYNC_RECEIVE_M=0,8000,16,0,0,16,256,2048<br>若测试通过，则连接A板和B板的串口最终会打印OK |

#### AT+I2S_ASYNC_SEND_M
| 命令格式 | AT+I2S_SYNC_SEND_M=idx,rate,width,protocol,polarity,sclk_freq,mclk_freq,size |
|------------------------------------------|-----------------------------|
| 命令功能 | 测试I2S作为master使用同步模式发送数据 |
| 配对命令 | AT+I2S_SYNC_RECEIVE_S;AT+I2S_ASYNC_RECEIVE_S;AT+I2S_DMA_RECEIVE_S |
| 示例 | 1.向B版发送命令:AT+I2S_ASYNC_RECEIVE_S=0,8000,16,0,0,16,256,2048<br>2.向A板发送命令:AT+I2S_SYNC_SEND_M=0,8000,16,0,0,16,256,2048<br>若测试通过，则连接A板和B板的串口最终会打印OK |

#### AT+I2S_ASYNC_RECEIVE_S

| 命令格式 | AT+I2S_ASYNC_RECEIVE_S=idx,rate,width,protocol,polarity,sclk_freq,mclk_freq,size |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试I2S作为slave使用同步模式发送数据                         |
| 配对命令 | AT+I2S_SYNC_SEND_M;AT+I2S_ASYNC_SEND_M;AT+I2S_DMA_SEND_M     |
| 示例     | 1.向A版发送命令:AT+I2S_ASYNC_RECEIVE_S=0,8000,16,0,0,16,256,2048<br>2.向B板发送命令:AT+I2S_SYNC_SEND_M=0,8000,16,0,0,16,256,2048<br>若测试通过，则连接A板和B板的串口最终会打印OK |

#### AT+I2S_ASYNC_SEND_S

| 命令格式 | AT+I2S_ASYNC_SEND_S=idx,rate,width,protocol,polarity,sclk_freq,mclk_freq,size |
|------------------------------------------|-----------------------------|
| 命令功能 | 测试I2S作为slave使用同步模式发送数据 |
| 配对命令 | AT+I2S_SYNC_RECEIVE_M;AT+I2S_ASYNC_RECEIVE_M;AT+I2S_DMA_RECEIVE_M |
| 示例 | 1.向A版发送命令:AT+I2S_ASYNC_SEND_S=0,8000,16,0,0,16,256,2048<br>2.向B板发送命令:AT+I2S_SYNC_RECEIVE_M=0,8000,16,0,0,16,256,2048<br>若测试通过，则连接A板和B板的串口最终会打印OK |

#### AT+I2S_ASYNC_RECEIVE_M
| 命令格式 | AT+I2S_ASYNC_RECEIVE_M=idx,rate,width,protocol,polarity,sclk_freq,mclk_freq,size |
|------------------------------------------|-----------------------------|
| 命令功能 | 测试I2S作为master使用同步模式发送数据 |
| 配对命令 | AT+I2S_SYNC_SEND_S;AT+I2S_ASYNC_SEND_S;AT+I2S_DMA_SEND_S |
| 示例 | 1.向B版发送命令:AT+I2S_ASYNC_SEND_S=0,8000,16,0,0,16,256,2048<br>2.向A板发送命令:AT+I2S_ASYNC_RECEIVE_M=0,8000,16,0,0,16,256,2048<br>若测试通过，则连接A板和B板的串口最终会打印OK |

#### AT+I2S_DMA_SEND_M
| 命令格式 | AT+I2S_DMA_SEND_M=idx,rate,width,protocol,polarity,sclk_freq,mclk_freq,size |
|------------------------------------------|-----------------------------|
| 命令功能 | 测试I2S作为master使用同步模式发送数据 |
| 配对命令 | AT+I2S_SYNC_RECEIVE_S;AT+I2S_ASYNC_RECEIVE_S;AT+I2S_DMA_RECEIVE_S |
| 示例 | 1.向B版发送命令:AT+I2S_ASYNC_RECEIVE_S=0,8000,16,0,0,16,256,2048<br>2.向A板发送命令:AT+I2S_SYNC_DMA_M=0,8000,16,0,0,16,256,2048<br>若测试通过，则连接A板和B板的串口最终会打印OK |

#### AT+I2S_DMA_RECEIVE_S

| 命令格式 | AT+I2S_DMA_RECEIVE_S=idx,rate,width,protocol,polarity,sclk_freq,mclk_freq,size |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试I2S作为slave使用同步模式发送数据                         |
| 配对命令 | AT+I2S_SYNC_SEND_M;AT+I2S_ASYNC_SEND_M;AT+I2S_DMA_SEND_M     |
| 示例     | 1.向A版发送命令:AT+I2S_DMA_RECEIVE_S=0,8000,16,0,0,16,256,2048<br>2.向B板发送命令:AT+I2S_SYNC_SEND_M=0,8000,16,0,0,16,256,2048<br>若测试通过，则连接A板和B板的串口最终会打印OK |

#### AT+I2S_DMA_SEND_S

| 命令格式 | AT+I2S_DMA_SEND_S=idx,rate,width,protocol,polarity,sclk_freq,mclk_freq,size |
|------------------------------------------|-----------------------------|
| 命令功能 | 测试I2S作为slave使用同步模式发送数据 |
| 配对命令 | AT+I2S_SYNC_RECEIVE_M;AT+I2S_ASYNC_RECEIVE_M;AT+I2S_DMA_RECEIVE_M |
| 示例 | 1.向A版发送命令:AT+I2S_DMA_SEND_S=0,8000,16,0,0,16,256,2048<br>2.向B板发送命令:AT+I2S_SYNC_RECEIVE_M=0,8000,16,0,0,16,256,2048<br>若测试通过，则连接A板和B板的串口最终会打印OK |

#### AT+I2S_DMA_RECEIVE_M
| 命令格式 | AT+I2S_DMA_RECEIVE_M=idx,rate,width,protocol,polarity,sclk_freq,mclk_freq,size |
|------------------------------------------|-----------------------------|
| 命令功能 | 测试I2S作为master使用同步模式发送数据 |
| 配对命令 | AT+I2S_SYNC_SEND_S;AT+I2S_ASYNC_SEND_S;AT+I2S_DMA_SEND_S |
| 示例 | 1.向B版发送命令:AT+I2S_ASYNC_SEND_S=0,8000,16,0,0,16,256,2048<br>2.向A板发送命令:AT+I2S_DMA_RECEIVE_M=0,8000,16,0,0,16,256,2048<br>若测试通过，则连接A板和B板的串口最终会打印OK |

#### 
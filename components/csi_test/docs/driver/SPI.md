

## SPI

### 概要

该文档主要描述SPI模块的CSI2.0自动化测试AT命令。 文档中所涉及的这些命令是用来测试SPI模块的接口工作是否正常、数据传输是否正确、接口返回值是否符合预期。

### 测试原理

测试时，应使被测试板与另外一块具备SPI功能的板子连接，并发送AT+PINMUX_CONFIG到被测试板把需要测试的引脚功能复用成SPI的引脚功能后，然后发送SPI测试命令进行相应的测试；测试程序会按照按照测试命令的配置进行相应的数据收发测试， 若接收方接收到的数据与预期一致，则相应的测试成功完成，否则测试失败。

### 测试流程

- 测试前应当准备好测试板（以下简称A板）、测试辅助板（以下简称B板）
- 选定需要测试的SPI针脚，并把A、B两块板子的SPI针脚相连
- 发送PINMUX_CONFIG命令到A板、B板， 把以上选定的引脚复用成SPI功能
- 发送SPI测试命令进行测试 （测试命令功能请参照下文）

### SPI功能AT命令列表概览

以下是SPI模块对应的测试命令列表及其功能简要介绍。

| AT命令列表                 | 功能                    |
| -------------------------- | ----------------------- |
| AT+SPI_INTERFACE           | SPI接口异常测试         |
| AT+SPI_SYNC_SEND_M         | master同步模式发送测试  |
| AT+SPI_SYNC_SEND_S         | slave同步模式发送 测试  |
| AT+SPI_SYNC_RECEIVE_M      | master同步模式接收      |
| AT+SPI_SYNC_RECEIVE_S      | slave同步模式接收       |
| AT+SPI_SYNC_SENDRECEIVE_M  | master同步模式发送接收  |
| AT+SPI_SYNC_SENDRECEIVE_S  | slave同步模式发送接收   |
| AT+SPI_ASYNC_SEND_M        | master异步模式发送 测试 |
| AT+SPI_ASYNC_SEND_S        | slave异步模式发送 测试  |
| AT+SPI_ASYNC_RECEIVE_M     | master异步模式接收      |
| AT+SPI_ASYNC_RECEIVE_S     | slave异步模式接收       |
| AT+SPI_ASYNC_SENDRECEIVE_M | master异步模式发送接收  |
| AT+SPI_ASYNC_SENDRECEIVE_S | slave异步模式发送接收   |
| AT+SPI_DMA_SEND_M          | master DMA模式下发送    |
| AT+SPI_DMA_SEND_S          | slave DMA模式下发送     |
| AT+SPI_DMA_RECEIVE_M       | master DMA模式接收      |
| AT+SPI_DMA_RECEIVE_S       | slave DMA模式接收       |
| AT+SPI_DMA_SENDRECEIVE_M   | master DMA模式发送接收  |
| AT+SPI_DMA_SENDRECEIVE_S   | slave DMA模式发送接收   |

注：1）以上命令若带_M, 则表示主从模式为master。若带S则表示主从模式为slave。 依照SPI通信协议原理，应当首先发送slave模式的命令到开发板， 然后发送master命令到另外一块开发板。

### 参数列表

以下是SPI模块对应的AT测试命令共用参数的简要介绍。

| 参数名字  | 释义                     | 取值范围                                                     |
| --------- | ------------------------ | ------------------------------------------------------------ |
| idx       | 表示设备号,比如0表示spi0 | 0、1、...，具体可测编号需参考soc.h                           |
| frame_len | 表示数据宽度,bit         | 4:SPI_FRAME_LEN_4，4bit<br/>5:SPI_FRAME_LEN_5，5bit<br/>6:SPI_FRAME_LEN_6，6bit<br/>7:SPI_FRAME_LEN_7，7bit<br/>8:SPI_FRAME_LEN_8，8bit<br/>9:SPI_FRAME_LEN_9，9bit<br/>10:SPI_FRAME_LEN_10，10bit<br/>11:SPI_FRAME_LEN_11，11bit<br/>12:SPI_FRAME_LEN_12，12bit<br/>13:SPI_FRAME_LEN_13，13bit<br/>14:SPI_FRAME_LEN_14，14bit<br/>15:SPI_FRAME_LEN_15，15bit<br/>16:SPI_FRAME_LEN_16，16bit |
| cp_format | 表示时钟相位极性         | 0:SPI_FORMAT_CPOL0_CPHA0<br/>1:SPI_FORMAT_CPOL0_CPHA1<br/>2:SPI_FORMAT_CPOL1_CPHA0<br/>3:SPI_FORMAT_CPOL1_CPHA1 |
| baud      | 表示通信的波特率，Hz     | 整数，例如：100000、400000、2500000、3400000....             |

### 命令详解

#### AT+SPI_INTERFACE

| 命令格式 | AT+SPI_INTERFACE                                             |
| -------- | ------------------------------------------------------------ |
| 命令功能 | SPI接口异常测试。该命令测试SPI所CSI接口的异常调用，测试时将会传入异常和非法参数，验证CSI接口是否返回相应的错误码。 |
| 示例     | 发送命令AT+SPI_INTERFACE到A板，测试完成后，A板对应串口会打印OK；否则测试失败 |

#### AT+SPI_SYNC_SEND_M

| 命令格式 | AT+SPI_SYNC_SEND_M=idx,frame_len,cp_format,baud |
|------------------------------------------|--------------------------------------------------------------------------------------------------|
| 命令功能 | master同步模式下发送数据 |
|可搭配的辅助测试命令|AT+SPI_SYNC_RECEIVE_S;AT+SPI_ASYNC_RECEIVE_S;AT+SPI_DMA_RECEIVE_S|
| 示例 | 1.向B板发送命令:AT+SPI_SYNC_RECEIVE_S=0,8,0,2500000<br/>2.向A板发送命令:AT+SPI_SYNC_SEND_M=0,8,0,2500000<br/>若测试通过，则连接A板和B板的串口最终会打印OK |
| 备注 | 发送该命令到被测试板时，应预先发送slave接收数据的命令到测试辅助板 |

#### AT+SPI_SYNC_RECEIVE_S

| 命令格式             | AT+SPI_SYNC_RECEIVE_S=idx,frame_len,cp_format,baud       |
| -------------------- | -------------------------------------------------------- |
| 命令功能             | slave同步模式下接收数据                                  |
| 可搭配的辅助测试命令 | AT+SPI_SYNC_SEND_M;AT+SPI_ASYNC_SEND_M;AT+SPI_DMA_SEND_M |
| 示例                 | 参考AT+SPI_SYNC_SEND_M示例                               |

#### AT+SPI_SYNC_SEND_S

| 命令格式 | AT+SPI_SYNC_SEND_S=idx,frame_len,cp_format,baud |
| -------- | ------------------------------------------------------------ |
| 命令功能 | slave同步模式下发送数据 |
|可搭配的辅助测试命令|AT+SPI_SYNC_RECEIVE_M;AT+SPI_ASYNC_RECEIVE_M;AT+SPI_DMA_RECEIVE_M|
| 示例     | 1.向A板发送命令:AT+SPI_SYNC_SEND_S=0,8,0,2500000<br/>2.向B板发送命令:AT+SPI_SYNC_RECEIVE_M=0,8,0,2500000<br/>若测试通过，则连接A板和B板的串口最终会打印OK |
| 备注 | 应预先发送该命令到被测试板，再发送接收数据的命令到测试辅助板 |

#### AT+SPI_SYNC_RECEIVE_M

| 命令格式             | AT+SPI_SYNC_RECEIVE_M=idx,frame_len,cp_format,baud       |
| -------------------- | -------------------------------------------------------- |
| 命令功能             | master同步模式下接收数据                                 |
| 可搭配的辅助测试命令 | AT+SPI_SYNC_SEND_S;AT+SPI_ASYNC_SEND_S;AT+SPI_DMA_SEND_S |
| 示例                 | 参考AT+SPI_SYNC_SEND_S示例                               |

#### AT+SPI_SYNC_SENDRECEIVE_M

| 命令格式             | AT+SPI_SYNC_SENDRECEIVE_M=idx,frame_len,cp_format,baud       |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | master同步模式下发送接收数据                                 |
| 可搭配的辅助测试命令 | AT+SPI_SYNC_SENDRECEIVE_S;AT+SPI_ASYNC_SENDRECEIVE_S;AT+SPI_DMA_SENDRECEIVE_S |
| 示例                 | 1.向B板发送命令:AT+SPI_SYNC_SENDRECEIVE_S=0,8,0,2500000<br/>2.向A板发送命令:AT+SPI_SYNC_SENDRECEIVE_M=0,8,0,2500000<br/>若测试通过，则连接A板和B板的串口最终会打印OK |
| 备注                 | 发送该命令到被测试板时，应预先发送slave接收数据的命令到测试辅助板 |

#### AT+SPI_SYNC_SENDRECEIVE_S

| 命令格式             | AT+SPI_SYNC_SENDRECEIVE_S=idx,frame_len,cp_format,baud       |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | slave同步模式下发送接收数据                                  |
| 可搭配的辅助测试命令 | AT+SPI_SYNC_SENDRECEIVE_M;AT+SPI_ASYNC_SENDRECEIVE_M;AT+SPI_DMA_SENDRECEIVE_M |
| 示例                 | 参考AT+SPI_SYNC_SENDRECEIVE_M示例                            |

#### AT+SPI_ASYNC_SEND_M

| 命令格式             | AT+SPI_ASYNC_SEND_M=idx,frame_len,cp_format,baud             |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | master异步模式下发送数据                                     |
| 可搭配的辅助测试命令 | AT+SPI_SYNC_RECEIVE_S;AT+SPI_ASYNC_RECEIVE_S;AT+SPI_DMA_RECEIVE_S |
| 示例                 | 1.向B板发送命令:AT+SPI_ASYNC_RECEIVE_S=0,8,0,2500000<br/>2.向A板发送命令:AT+SPI_ASYNC_SEND_M=0,8,0,2500000<br/>若测试通过，则连接A板和B板的串口最终会打印OK |
| 备注                 | 发送该命令到被测试板时，应预先发送slave接收数据的命令到测试辅助板 |

#### AT+SPI_ASYNC_RECEIVE_S

| 命令格式             | AT+SPI_ASYNC_RECEIVE_S=idx,frame_len,cp_format,baud      |
| -------------------- | -------------------------------------------------------- |
| 命令功能             | slave异步模式下接收数据                                  |
| 可搭配的辅助测试命令 | AT+SPI_SYNC_SEND_M;AT+SPI_ASYNC_SEND_M;AT+SPI_DMA_SEND_M |
| 示例                 | 参考AT+SPI_ASYNC_SEND_M示例                              |

#### AT+SPI_ASYNC_SEND_S

| 命令格式             | AT+SPI_ASYNC_SEND_S=idx,frame_len,cp_format,baud             |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | slave异步模式下发送数据                                      |
| 可搭配的辅助测试命令 | AT+SPI_SYNC_RECEIVE_M;AT+SPI_ASYNC_RECEIVE_M;AT+SPI_DMA_RECEIVE_M |
| 示例                 | 1.向A板发送命令:AT+SPI_ASYNC_SEND_S=0,8,0,2500000<br/>2.向B板发送命令:AT+SPI_ASYNC_RECEIVE_M=0,8,0,2500000<br/>若测试通过，则连接A板和B板的串口最终会打印OK |
| 备注                 | 应预先发送该命令到被测试板，再发送接收数据的命令到测试辅助板 |

#### AT+SPI_ASYNC_RECEIVE_M

| 命令格式             | AT+SPI_AYNC_RECEIVE_M=idx,frame_len,cp_format,baud       |
| -------------------- | -------------------------------------------------------- |
| 命令功能             | master异步模式下接收数据                                 |
| 可搭配的辅助测试命令 | AT+SPI_SYNC_SEND_S;AT+SPI_ASYNC_SEND_S;AT+SPI_DMA_SEND_S |
| 示例                 | 参考AT+SPI_ASYNC_SEND_S示例                              |

#### AT+SPI_ASYNC_SENDRECEIVE_M

| 命令格式             | AT+SPI_ASYNC_SENDRECEIVE_M=idx,frame_len,cp_format,baud      |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | master异步模式下发送接收数据                                 |
| 可搭配的辅助测试命令 | AT+SPI_SYNC_SENDRECEIVE_S;AT+SPI_ASYNC_SENDRECEIVE_S;AT+SPI_DMA_SENDRECEIVE_S |
| 示例                 | 1.向B板发送命令:AT+SPI_ASYNC_SENDRECEIVE_S=0,8,0,2500000<br/>2.向A板发送命令:AT+SPI_ASYNC_SENDRECEIVE_M=0,8,0,2500000<br/>若测试通过，则连接A板和B板的串口最终会打印OK |
| 备注                 | 发送该命令到被测试板时，应预先发送slave接收数据的命令到测试辅助板 |

#### AT+SPI_ASYNC_SENDRECEIVE_S

| 命令格式             | AT+SPI_ASYNC_SENDRECEIVE_S=idx,frame_len,cp_format,baud      |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | slave异步模式下发送接收数据                                  |
| 可搭配的辅助测试命令 | AT+SPI_SYNC_SENDRECEIVE_M;AT+SPI_ASYNC_SENDRECEIVE_M;AT+SPI_DMA_SENDRECEIVE_M |
| 示例                 | 参考AT+SPI_ASYNC_SENDRECEIVE_M示例                           |

#### AT+SPI_DMA_SEND_M

| 命令格式             | AT+SPI_DMA_SEND_M=idx,frame_len,cp_format,baud               |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | master DMA模式下发送数据                                     |
| 可搭配的辅助测试命令 | AT+SPI_SYNC_RECEIVE_S;AT+SPI_ASYNC_RECEIVE_S;AT+SPI_DMA_RECEIVE_S |
| 示例                 | 1.向B板发送命令:AT+SPI_DMA_RECEIVE_S=0,8,0,2500000<br/>2.向A板发送命令:AT+SPI_DMA_SEND_M=0,8,0,2500000<br/>若测试通过，则连接A板和B板的串口最终会打印OK |
| 备注                 | 发送该命令到被测试板时，应预先发送slave接收数据的命令到测试辅助板 |

#### AT+SPI_DMA_RECEIVE_S

| 命令格式             | AT+SPI_DMA_RECEIVE_S=idx,frame_len,cp_format,baud        |
| -------------------- | -------------------------------------------------------- |
| 命令功能             | slave DMA模式下接收数据                                  |
| 可搭配的辅助测试命令 | AT+SPI_SYNC_SEND_M;AT+SPI_ASYNC_SEND_M;AT+SPI_DMA_SEND_M |
| 示例                 | 参考AT+SPI_DMA_SEND_M示例                                |

#### AT+SPI_DMA_SEND_S

| 命令格式             | AT+SPI_DMA_SEND_S=idx,frame_len,cp_format,baud               |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | slave DMA模式下发送数据                                      |
| 可搭配的辅助测试命令 | AT+SPI_SYNC_RECEIVE_M;AT+SPI_ASYNC_RECEIVE_M;AT+SPI_DMA_RECEIVE_M |
| 示例                 | 1.向A板发送命令:AT+SPI_DMA_SEND_S=0,8,0,2500000<br/>2.向B板发送命令:AT+SPI_DMA_RECEIVE_M=0,8,0,2500000<br/>若测试通过，则连接A板和B板的串口最终会打印OK |
| 备注                 | 应预先发送该命令到被测试板，再发送接收数据的命令到测试辅助板 |

#### AT+SPI_DMA_RECEIVE_M

| 命令格式             | AT+SPI_DMA_RECEIVE_M=idx,frame_len,cp_format,baud        |
| -------------------- | -------------------------------------------------------- |
| 命令功能             | master DMA模式下接收数据                                 |
| 可搭配的辅助测试命令 | AT+SPI_SYNC_SEND_S;AT+SPI_ASYNC_SEND_S;AT+SPI_DMA_SEND_S |
| 示例                 | 参考AT+SPI_DMA_SEND_S示例                                |

#### AT+SPI_DMA_SENDRECEIVE_M

| 命令格式             | AT+SPI_DMA_SENDRECEIVE_M=idx,frame_len,cp_format,baud        |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | master DMA模式下发送接收数据                                 |
| 可搭配的辅助测试命令 | AT+SPI_SYNC_SENDRECEIVE_S;AT+SPI_ASYNC_SENDRECEIVE_S;AT+SPI_DMA_SENDRECEIVE_S |
| 示例                 | 1.向B板发送命令:AT+SPI_DMA_SENDRECEIVE_S=0,8,0,2500000<br/>2.向A板发送命令:AT+SPI_DMA_SENDRECEIVE_M=0,8,0,2500000<br/>若测试通过，则连接A板和B板的串口最终会打印OK |
| 备注                 | 发送该命令到被测试板时，应预先发送slave接收数据的命令到测试辅助板 |

#### AT+SPI_DMA_SENDRECEIVE_S

| 命令格式             | AT+SPI_DMA_SENDRECEIVE_S=idx,frame_len,cp_format,baud        |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | slave DMA模式下发送接收数据                                  |
| 可搭配的辅助测试命令 | AT+SPI_SYNC_SENDRECEIVE_M;AT+SPI_ASYNC_SENDRECEIVE_M;AT+SPI_DMA_SENDRECEIVE_M |
| 示例                 | 参考AT+SPI_DMA_SENDRECEIVE_M示例                             |
## UART

### 概要

该文档主要描述UART模块的CSI2.0自动化测试AT命令。 文档中所涉及的这些命令是用来测试UART模块的接口工作是否正常、数据传输是否正确、接口返回值是否符合预期。

### 测试原理

测试时，应使被测试板与另外一块具备UART功能的板子连接，并发送AT+PINMUX_CONFIG到被测试板把需要测试的引脚功能复用成UART的引脚功能后，然后发送UART测试命令进行相应的测试；测试程序会按照按照测试命令的配置进行相应的数据收发测试， 若接收方接收到的数据与预期一致，则相应的测试成功完成，否则测试失败。

### 测试流程

- 测试前应当准备好测试板（以下简称A板）、测试辅助板（以下简称B板）
- 选定需要测试的UART针脚，并把A、B两块板子的UART针脚相连(A板TX接B板RX,A板RX接B板TX)
- 发送PINMUX_CONFIG命令到A板、B板， 把以上选定的引脚复用成UART功能
- 发送UART测试命令进行测试 （测试命令功能请参照下文）

### UART功能AT命令列表概览

以下是UART模块对应的测试命令列表及其功能简要介绍。

| AT命令列表                     | 功能                                        | 备注   |
| ------------------------------ | ------------------------------------------- | ------ |
| AT+UART_INTERFACE              | UART接口异常测试                            |        |
| AT+UART_SYNC_SEND              | 测试UART模块的同步发送功能                  |        |
| AT+UART_SYNC_RECEIVE           | 测试UART模块的同步接收功能                  |        |
| AT+UART_ASYNC_SEND             | 测试UART模块的异步发送功能                  |        |
| AT+UART_ASYNC_RECEIVE          | 测试UART模块的异步接收功能                  |        |
| AT+UART_DMA_SEND               | 测试UART模块的DMA发送功能                   |        |
| AT+UART_DMA_RECEIVE            | 测试UART模块的DMA接收功能                   |        |
| AT+UART_PUT_CHAR               | 测试UART模块的字符发送功能                  |        |
| AT+UART_GET_CHAR               | 测试UART模块的字符接收功能                  |        |
| AT+UART_ASYNC_SENDRECEIVE      | 测试UART模块的异步发送接收功能              |        |
| AT+UART_DMA_SENDRECEIVE        | 测试UART模块的DMA发送接收功能               |        |
| AT+UART_ASYNC_SEND_FIFO        | 测试UART模块的异步异常发送功能（FIFIO清空） |        |
| AT+UART_ASYNC_RECEIVE_FIFO     | 测试UART模块的异步异常接收功能（FIFIO溢出） |        |
| AT+UART_ASYNC_SEND_FLOWCTRL    | 测试UART模块的流控发送清除功能              |        |
| AT+UART_ASYNC_RECEIVE_FLOWCTRL | 测试UART模块的流控发送请求功能              |        |
| AT+UART_ASYNC_SEND_ABNORMAL    | 测试UART模块的异步异常发送功能              | 可不测 |
| AT+UART_ASYNC_RECEIVE_ABNORMAL | 测试UART模块的异步异常接收功能              | 可不测 |
| AT+UART_DMA_SEND_ABNORMAL      | 测试UART模块的DMA异常发送功能               | 可不测 |
| AT+UART_DMA_RECEIVE_ABNORMAL   | 测试UART模块的DMA异常接收功能               | 可不测 |
| AT+UART_ASYNC_SEND_ALL         | 测试UART模块的所有异步发送功能              | 可不测 |
| AT+UART_ASYNC_RECEIVE_ALL      | 测试UART模块的所有异步接收功能              | 可不测 |
| AT+UART_DMA_SEND_ALL           | 测试UART模块的所有DMA发送功能               | 可不测 |
| AT+UART_DMA_RECEIVE_ALL        | 测试UART模块的所有DMA接收功能               | 可不测 |

### 参数列表

以下是UART模块对应的AT测试命令共用参数的简要介绍。

| 参数名字        | 释义                       | 取值范围                                                     |
| --------------- | -------------------------- | ------------------------------------------------------------ |
| idx             | 表示设备号,比如0表示uart0  | 0、1、...，具体可测编号需参考soc.h                           |
| baudrate        | 表示波特率                 | 例如110、300、600、1200、2400、4800、<br/>9600、14400、19200、38400、56000、57600、<br/>115200、128000、230400、256000、460800、<br/>500000、512000、600000、750000、921600、<br/>1000000、1500000、2000000 |
| data_bits       | 表示数据位                 | 0:UART_DATA_BITS_5，5位数据位宽<br/>1:UART_DATA_BITS_6，6位数据位宽<br/>2:UART_DATA_BITS_7，7位数据位宽<br/>3:UART_DATA_BITS_8，8位数据位宽<br/>4:UART_DATA_BITS_9，9位数据位宽 |
| parity          | 表示奇偶校验位             | 0:UART_PARITY_NONE，无校验<br/>1:UART_PARITY_EVEN，偶校验<br/>2:UART_PARITY_ODD，奇校验 |
| stop_bits       | 表示停止位                 | 0:UART_STOP_BITS_1，1停止位<br/>1:UART_STOP_BITS_2，2停止位<br/>2:UART_STOP_BITS_1_5，1.5停止位 |
| size            | 表示传输长度               | 整数                                                         |
| flowctrl        | 表示流控模式               | 0:UART_FLOWCTRL_NONE，无流控<br/>1:UART_FLOWCTRL_RTS，发送请求<br/>2:UART_FLOWCTRL_CTS，发送允许<br/>3:UART_FLOWCTRL_RTS_CTS，发送请求与发送允许功能同时打开 |
| callback_config | 表示在发送过程中更改的配置 | 0：配置波特率<br/>1：配置数据格数<br/>2：配置流控<br/>3：去初始化<br/>4：取消DMA链接通道 |
| idx_all         | 表示配置的所有UART设备号   | 如初始化uart1、uart2，传入参数0110<br/>初始化uart1、uart2、uart3，传入参数1110 |

### 命令详解

#### AT+UART_INTERFACE

| 命令格式 | AT+UART_INTERFACE                                            |
| -------- | ------------------------------------------------------------ |
| 命令功能 | UART接口异常测试。该命令测试UART所CSI接口的异常调用，测试时将会传入异常和非法参数，验证CSI接口是否返回相应的错误码。 |
| 示例     | 发送命令AT+UART_INTERFACE到A板，测试完成后，A板对应串口会打印OK；否则测试失败 |

#### AT+UART_SYNC_SEND

| 命令格式             | AT+UART_SYNC_SEND=idx,baudrate,data_bits,parity,stop_bits,size |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块的同步发送功能                                   |
| 可搭配的辅助测试命令 | AT+UART_SYNC_RECEIVE;AT+UART_ASYNC_RECEIVE;AT+UART_DMA_RECEIVE |
| 示例                 | 1.向B板发送命令:AT+UART_SYNC_RECEIVE=0,9600,3,0,0,10<br/>B板对应串口打印READY后<br/>2.向A板发送命令:AT+UART_SYNC_SEND=0,9600,3,0,0,10<br/>若测试通过，则连接A板和B板的串口最终会打印OK，否则测试失败 |


#### AT+UART_SYNC_RECEIVE

| 命令格式             | AT+UART_SYNC_RECEIVE=idx,baudrate,data_bits,parity,stop_bits,size |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块的同步接收功能                                   |
| 可搭配的辅助测试命令 | AT+UART_SYNC_SEND;AT+UART_ASYNC_SEND;AT+UART_DMA_SEND        |
| 示例                 | 参考AT+UART_SYNC_SEND示例                                    |

#### AT+UART_ASYNC_SEND
| 命令格式             | AT+UART_ASYNC_SEND=idx,baudrate,data_bits,parity,stop_bits,size |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块的异步发送功能                                   |
| 可搭配的辅助测试命令 | AT+UART_SYNC_RECEIVE;AT+UART_ASYNC_RECEIVE;AT+UART_DMA_RECEIVE |
| 示例                 | 1.向B板发送命令:AT+UART_ASYNC_RECEIVE=0,9600,3,0,0,10<br/>B板对应串口打印READY后<br/>2.向A板发送命令:AT+UART_ASYNC_SEND=0,9600,3,0,0,10<br/>若测试通过，则连接A板和B板的串口最终会打印OK，否则测试失败 |

#### AT+UART_ASYNC_RECEIVE
| 命令格式             | AT+UART_ASYNC_RECEIVE=idx,baudrate,data_bits,parity,stop_bits,size |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块的异步接收功能                                   |
| 可搭配的辅助测试命令 | AT+UART_SYNC_SEND;AT+UART_ASYNC_SEND;AT+UART_DMA_SEND        |
| 示例                 | 参考AT+UART_ASYNC_SEND示例                                   |
#### AT+UART_DMA_SEND
| 命令格式             | AT+UART_DMA_SEND=idx,baudrate,data_bits,parity,stop_bits,size |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块的DMA发送功能                                    |
| 可搭配的辅助测试命令 | AT+UART_SYNC_RECEIVE;AT+UART_ASYNC_RECEIVE;AT+UART_DMA_RECEIVE |
| 示例                 | 1.向B板发送命令:AT+UART_DMA_RECEIVE=0,9600,3,0,0,10<br/>B板对应串口打印READY后<br/>2.向A板发送命令:AT+UART_DMA_SEND=0,9600,3,0,0,10<br/>若测试通过，则连接A板和B板的串口最终会打印OK，否则测试失败 |
#### AT+UART_DMA_RECEIVE
| 命令格式             | AT+UART_DMA_RECEIVE=idx,baudrate,data_bits,parity,stop_bits,size |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块的DMA接收功能                                    |
| 可搭配的辅助测试命令 | AT+UART_SYNC_SEND;AT+UART_ASYNC_SEND;AT+UART_DMA_SEND        |
| 示例                 | 参考AT+UART_DMA_SEND示例                                     |

#### AT+UART_PUT_CHAR
| 命令格式             | AT+UART_PUT_CHAR=idx,baudrate,data_bits,parity,stop_bits     |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块的字符发送功能                                   |
| 可搭配的辅助测试命令 | AT+UART_GET_CHAR                                             |
| 示例                 | 1.向B板发送命令:AT+UART_GET_CHAR=0,9600,3,0,0<br/>B板对应串口打印READY后<br/>2.向A板发送命令:AT+UART_PUT_CHAR=0,9600,3,0,0<br/>若测试通过，则连接A板和B板的串口最终会打印OK，否则测试失败 |
#### AT+UART_GET_CHAR
| 命令格式             | AT+UART_GET_CHAR=idx,baudrate,data_bits,parity,stop_bits |
| -------------------- | -------------------------------------------------------- |
| 命令功能             | 测试UART模块的字符接收功能                               |
| 可搭配的辅助测试命令 | AT+UART_PUT_CHAR                                         |
| 示例                 | 参考AT+UART_PUT_CHAR示例                                 |
#### AT+UART_ASYNC_SENDRECEIVE

| 命令格式             | AT+UART_ASYNC_SENDRECEIVE=idx,baudrate,data_bits,parity,stop_bits,size |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块的异步发送接收功能                               |
| 可搭配的辅助测试命令 | AT+UART_ASYNC_SENDRECEIVE;AT+UART_DMA_SENDRECEIVE            |
| 示例                 | 1.向B板发送命令:AT+UART_ASYNC_SENDRECEIVE=0,9600,3,0,0,10<br/>B板对应串口打印READY后<br/>2.向A板发送命令:AT+UART_ASYNC_SENDRECEIVE=0,9600,3,0,0,10<br/>若测试通过，则连接A板和B板的串口最终会打印OK，否则测试失败 |

#### AT+UART_DMA_SENDRECEIVE

| 命令格式             | AT+UART_DMA_SENDRECEIVE=idx,baudrate,data_bits,parity,stop_bits,size |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块的异步发送功能                                   |
| 可搭配的辅助测试命令 | AT+UART_ASYNC_SENDRECEIVE;AT+UART_DMA_SENDRECEIVE            |
| 示例                 | 1.向B板发送命令:AT+UART_DMA_SENDRECEIVE=0,9600,3,0,0,10<br/>B板对应串口打印READY后<br/>2.向A板发送命令:AT+UART_DMA_SENDRECEIVE=0,9600,3,0,0,10<br/>若测试通过，则连接A板和B板的串口最终会打印OK，否则测试失败 |

#### AT+UART_ASYNC_SEND_FIFO

| 命令格式             | AT+UART_ASYNC_SEND_FIFO=idx,baudrate,data_bits,parity,stop_bits,size |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块的异步异常发送功能（FIFIO清空）                  |
| 可搭配的辅助测试命令 | AT+UART_ASYNC_RECEIVE_FIFO                                   |
| 示例                 | 1.向B板发送命令:AT+UART_ASYNC_RECEIVE_FIFO=0,9600,3,0,0,10000<br/>B板对应串口打印READY后<br/>2.向A板发送命令:AT+UART_ASYNC_SEND_FIFO=0,9600,3,0,0,1000<br/>若测试通过，则连接A板和B板的串口最终会打印OK，否则测试失败 |

#### AT+UART_ASYNC_RECEIVE_FIFO

| 命令格式             | AT+UART_ASYNC_RECEIVE_FIFO=idx,baudrate,data_bits,parity,stop_bits,size |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块的异步异常接收功能（FIFIO溢出）                  |
| 可搭配的辅助测试命令 | AT+UART_ASYNC_SEND_FIFO                                      |
| 示例                 | 参考AT+UART_ASYNC_SEND_FIFO示例                              |

#### AT+UART_ASYNC_SEND_FLOWCTRL

| 命令格式             | AT+UART_ASYNC_SEND_FLOWCTRL=idx,baudrate,data_bits,parity,stop_bits,size,flowctrl |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块的流控发送清除功能                               |
| 可搭配的辅助测试命令 | AT+UART_ASYNC_RECEIVE_FLOWCTRL                               |
| 示例                 | 1.向B板发送命令:AT+UART_ASYNC_RECEIVE_FLOWCTRL=0,9600,3,0,0,100,3<br/>B板对应串口打印READY后<br/>2.向A板发送命令:AT+UART_ASYNC_SEND_FLOWCTRL=0,9600,3,0,0,100,3<br/>若测试通过，则连接A板和B板的串口最终会打印OK，否则测试失败 |

#### AT+UART_ASYNC_RECEIVE_FLOWCTRL

| 命令格式             | AT+UART_ASYNC_RECEIVE_FLOWCTRL=idx,baudrate,data_bits,parity,stop_bits,size,flowctrl |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块的流控发送请求功能                               |
| 可搭配的辅助测试命令 | AT+UART_ASYNC_SEND_FLOWCTRL                                  |
| 示例                 | 参考AT+UART_ASYNC_SEND_FLOWCTRL示例                          |

#### AT+UART_ASYNC_SEND_ABNORMAL

| 命令格式             | AT+UART_ASYNC_SEND_ABNORMAL=idx,baudrate,data_bits,parity,stop_bits,<br/>size,callback_config |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块的异常异步发送功能                               |
| 可搭配的辅助测试命令 | AT+UART_ASYNC_RECEIVE_ABNORMAL;AT+UART_DMA_RECEIVE_ABNORMAL;<br/>AT+UART_SYNC_RECEIVE;AT+UART_ASYNC_RECEIVE;AT+UART_DMA_RECEIVE |
| 示例                 | 1.向B板发送命令:AT+UART_ASYNC_RECEIVE=0,115200,2,1,1,10000<br/>B板对应串口打印READY后<br/>2.向A板发送命令:AT+UART_ASYNC_SEND_ABNORMAL=0,115200,2,1,1,10000,0<br/>若测试通过，则连接A板和B板的串口最终会打印OK，否则测试失败<br/>3.向A板发送命令:AT+UART_ASYNC_SEND_ABNORMAL=0,115200,2,1,1,10000,3<br/>程序会跑飞；否则测试失败<br/>4.向A板发送命令:AT+UART_ASYNC_SEND_ABNORMAL=0,115200,2,1,1,10000,4<br/>程序会跑飞；否则测试失败 |

#### AT+UART_ASYNC_RECEIVE_ABNORMAL

| 命令格式             | AT+UART_ASYNC_RECEIVE_ABNORMAL=idx,baudrate,data_bits,parity,stop_bits,<br/>size,callback_config |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块的异常异步接收功能                               |
| 可搭配的辅助测试命令 | AT+UART_ASYNC_SEND_ABNORMAL;AT+UART_DMA_SEND_ABNORMAL<br>AT+UART_SYNC_SEND;AT+UART_ASYNC_SEND;AT+UART_DMA_SEND |
| 示例                 | 1.向B板发送命令:AT+UART_ASYNC_RECEIVE=0,115200,2,1,1,10000<br/>B板对应串口打印READY后<br/>2.向A板发送命令:AT+UART_ASYNC_SEND_ABNORMAL=0,115200,2,1,1,10000,0<br/>若测试通过，则连接A板和B板的串口最终会打印OK，否则测试失败<br/>3.向A板发送命令:AT+UART_ASYNC_RECEIVE_ABNORMAL=0,115200,2,1,1,10000,3<br/>程序会跑飞；否则测试失败<br/>4.向A板发送命令:AT+UART_ASYNC_RECEIVE_ABNORMAL=0,115200,2,1,1,10000,4<br/>程序会跑飞；否则测试失败 |

#### AT+UART_DMA_SEND_ABNORMAL

| 命令格式             | AT+UART_DMA_SEND_ABNORMAL=idx,baudrate,data_bits,parity,stop_bits,<br/>size,callback_config |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块的异常异步发送功能                               |
| 可搭配的辅助测试命令 | AT+UART_ASYNC_RECEIVE_ABNORMAL;AT+UART_DMA_RECEIVE_ABNORMAL<br>AT+UART_SYNC_RECEIVE;AT+UART_ASYNC_RECEIVE;AT+UART_DMA_RECEIVE |
| 示例                 | 1.向B板发送命令:AT+UART_DMA_RECEIVE=0,115200,2,1,1,10000<br/>B板对应串口打印READY后<br/>2.向A板发送命令:AT+UART_DMA_SEND_ABNORMAL=0,115200,2,1,1,10000,0<br/>若测试通过，则连接A板和B板的串口最终会打印OK，否则测试失败<br/>3.向A板发送命令:AT+UART_DMA_SEND_ABNORMAL=0,115200,2,1,1,10000,3<br/>程序会跑飞；否则测试失败<br/>4.向A板发送命令:AT+UART_DMA_SEND_ABNORMAL=0,115200,2,1,1,10000,4<br/>程序会跑飞；否则测试失败 |

#### AT+UART_DMA_RECEIVE_ABNORMAL

| 命令格式             | AT+UART_DMA_RECEIVE_ABNORMAL=idx,baudrate,data_bits,parity,stop_bits,<br/>size,callback_config |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块的异常异步接收功能                               |
| 可搭配的辅助测试命令 | AT+UART_ASYNC_SEND_ABNORMAL;AT+UART_DMA_SEND_ABNORMAL<br>AT+UART_SYNC_SEND;AT+UART_ASYNC_SEND;AT+UART_DMA_SEND |
| 示例                 | 1.向B板发送命令:AT+UART_DMA_RECEIVE=0,115200,2,1,1,10000<br/>B板对应串口打印READY后<br/>2.向A板发送命令:AT+UART_DMA_SEND_ABNORMAL=0,115200,2,1,1,10000,0<br/>若测试通过，则连接A板和B板的串口最终会打印OK，否则测试失败<br/>3.向A板发送命令:AT+UART_DMA_RECEIVE_ABNORMAL=0,115200,2,1,1,10000,3<br/>程序会跑飞；否则测试失败<br/>4.向A板发送命令:AT+UART_DMA_RECEIVE_ABNORMAL=0,115200,2,1,1,10000,4<br/>程序会跑飞；否则测试失败 |

#### AT+UART_ASYNC_SEND_ALL

| 命令格式             | AT+UART_ASYNC_SEND_ALL=idx_all,baudrate,data_bits,parity,stop_bits,size |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块的所有异步发送功能                               |
| 可搭配的辅助测试命令 | AT+UART_ASYNC_RECEIVE_ALL                                    |
| 示例                 | 1.向B板发送命令:AT+UART_ASYNC_RECEIVE_ALL=0,9600,3,0,0,10<br/>B板对应串口打印READY后<br/>2.向A板发送命令:AT+UART_ASYNC_SEND_ALL=0,9600,3,0,0,10<br/>若测试通过，则连接A板和B板的串口最终会打印OK，否则测试失败 |

#### AT+UART_ASYNC_RECEIVE_ALL

| 命令格式             | AT+UART_ASYNC_RECEIVE_ALL=idx_all,baudrate,data_bits,parity,stop_bits,size |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块的所有异步发送功能                               |
| 可搭配的辅助测试命令 | AT+UART_ASYNC_SEND_ALL                                       |
| 示例                 | 参考AT+UART_ASYNC_SEND_ALL示例                               |

#### AT+UART_DMA_SEND_ALL

| 命令格式             | AT+UART_DMA_SEND_ALL=idx_all,baudrate,data_bits,parity,stop_bits,size |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块的所有DMA发送功能                                |
| 可搭配的辅助测试命令 | AT+UART_DMA_RECEIVE_ALL                                      |
| 示例                 | 1.向B板发送命令:AT+UART_DMA_RECEIVE_ALL=0,9600,3,0,0,10<br/>B板对应串口打印READY后<br/>2.向A板发送命令:AT+UART_DMA_SEND_ALL=0,9600,3,0,0,10<br/>若测试通过，则连接A板和B板的串口最终会打印OK，否则测试失败 |

#### AT+UART_DMA_RECEIVE_ALL

| 命令格式             | AT+UART_DMA_RECEIVE_ALL=idx_all,baudrate,data_bits,parity,stop_bits,size |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块的所有DMA发送功能                                |
| 可搭配的辅助测试命令 | AT+UART_DMA_SEND_ALL                                         |
| 示例                 | 参考AT+UART_DMA_SEND_ALL示例                                 |
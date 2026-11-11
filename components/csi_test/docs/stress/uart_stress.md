## STRESS_UART

### 概要

该文档主要描述UART模块的CSI2.0压测AT命令。 文档中所涉及的这些命令是用来测试UART模块的接口工作是否正常、数据传输是否正确、接口返回值是否符合预期。



### 测试原理

测试时，应使被测试板与另外一块具备UART功能的板子连接，并发送AT+PINMUX_CONFIG到被测试板把需要测试的引脚功能复用成UART的引脚功能后，然后发送UART测试命令进行相应的测试；测试程序会按照按照测试命令的配置进行相应的数据收发测试， 若接收方接收到的数据与预期一致，则相应的测试成功完成，否则测试失败。



### 测试流程

- 测试前应当准备好测试板（以下简称A板）、测试辅助板（以下简称B板）
- 选定需要测试的UART针脚，并把A、B两块板子的UART针脚相连(A板TX接B板RX,A板RX接B板TX)
- 发送PINMUX_CONFIG命令到A板、B板， 把以上选定的引脚复用成UART功能
- 发送UART测试命令进行测试 （测试命令功能请参照下文）



### AT命令列表

以下是UART模块对应的测试命令列表及其功能简要介绍。

| AT命令列表                    | 功能                                             |
| ----------------------------- | ------------------------------------------------ |
| AT+STRESS_UART_SEND           | 测试UART模块的连续发送功能                       |
| AT+STRESS_UART_RECEIVE        | 测试UART模块的连续接收功能                       |
| AT+STRESS_UART_RANDOM_SEND    | 测试UART模块数据传输前随机配置工作模式（先发送） |
| AT+STRESS_UART_RANDOM_RECEIVE | 测试UART模块数据传输前随机配置工作模式（先接收） |

### AT+STRESS_UART_SEND

| 命令格式             | AT+UART_SYNC_SEND                                            |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块的连续发送功能                                   |
| 可搭配的辅助测试命令 | AT+STRESS_UART_RECEIVE                                       |
| 示例                 | 发送命令AT+STRESS_UART_RECEIVE到B板,  B板对应串口打印READY后，发送命令 AT+STRESS_UART_SEND到A板。测试完成后， A板、B板对应串口都会打印OK；否则测试失败 |

### AT+STRESS_UART_RECEIVE

| 命令格式             | AT+UART_SYNC_RECEIVE                                         |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块的连续发送功能                                   |
| 可搭配的辅助测试命令 | AT+STRESS_UART_SEND                                          |
| 示例                 | 发送命令AT+STRESS_UART_RECEIVE到A板,  B板对应串口打印READY后，发送命令 AT+STRESS_UART_SEND到B板。测试完成后， A板、B板对应串口都会打印OK；否则测试失败 |

### AT+STRESS_UART_RANDOM_SEND

| 命令格式             | AT+STRESS_UART_RANDOM_SEND                                   |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块数据传输前随机配置工作模式（先发送）             |
| 可搭配的辅助测试命令 | AT+STRESS_UART_RANDOM_RECEIVE                                |
| 示例                 | 发送命令AT+STRESS_UART_RECEIVE到B板,  B板对应串口打印READY后，发送命令 AT+STRESS_UART_SEND到A板。测试完成后， A板、B板对应串口都会打印OK；否则测试失败 |

### AT+STRESS_UART_RANDOM_RECEIVE

| 命令格式             | AT+STRESS_UART_RANDOM_RECEIVE                                |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试UART模块数据传输前随机配置工作模式（先接收）             |
| 可搭配的辅助测试命令 | AT+STRESS_UART_RANDOM_SEND                                   |
| 示例                 | 发送命令AT+STRESS_UART_RECEIVE到A板,  B板对应串口打印READY后，发送命令 AT+STRESS_UART_SEND到B板。测试完成后， A板、B板对应串口都会打印OK；否则测试失败 |
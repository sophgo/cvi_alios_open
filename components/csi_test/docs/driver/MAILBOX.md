## MAILBOX

### 概要

该文档主要描述MAILBOX模块的CSI2.0自动化测试AT命令。 文档中所涉及的这些命令是用来测试MAILBOX模块的接口工作是否正常、数据传输是否正确、接口返回值是否符合预期。

### 设备说明
MBOX（mailbox）在多核SOC 系统中，核间通信是多核协同工作的保证。MAILBOX是以中断触发方式提供核间通信的IP。支持发送CPU 向接收CPU 传递除中断事件外的信息量，以信息寄存器组的方式体现。

### 测试原理

测试时，给测试板发送MAILBOX测试命令进行相应的测试；测试程序会按照按照测试命令进行相应的测试， 若结果与预期一致，则相应的测试成功完成，否则测试失败。



### 测试流程

- 测试前应当准备好一块测试板两个核心（以下简称A板和B板）
- 发送MAILBOX测试命令进行测试 （测试命令功能请参照下文）



### AT命令列表 

以下是MAILBOX模块对应的测试命令列表及其功能简要介绍。

| AT命令列表   | 功能                  |
| ------------ | --------------------- |
| AT+MBOX_INTERFACE  | 对MAILBOX模块进行接口测试，进行接口测试时，传入非法值和异常值，验证CSI接口返回值是否符合预期 |
| AT+MBOX_SEND | 对MAILBOX模块进行发送功能测试                                                                 |
| AT+MBOX_RECEIVE | 对MAILBOX模块进行接收功能测试                                                              |
| AT+MBOX_SEND_PERFORMANCE | 对MAILBOX模块进行发送性能测试 |
| AT+MBOX_RECEIVE_PERFORMANCE | 辅助对MAILBOX模块进行发送性能测试 |


### 参数列表

以下是MAILBOX模块对应的AT测试命令共用参数的简要介绍。

| 参数名字   | 释义                          |
| ---------- | ----------------------------- |
| mailbox_id        | 表示mailbox的设备号               |
| channel_id        | 表示传输通道 |
| trans_size        | 表示传输长度                      |
| number_of_loop | 传输次数 |

### MBOX_SEND
| 命令格式 | AT+MBOX_SEND = mailbox_id,channel_id,trans_size |
|--------- |-----------------------------------------------------------------------------------------------------------------------------------------------------------|
| 命令功能 | 测试MAILBOX模块的发送功能|
| 可搭配的辅助测试命令 | AT+MBOX_RECEIVE                                                           |
| 命令参数 | mailbox_id：设备号<br/>channel_id:传输通道<br/>trans_size：数据传输长度<br/> |
| 示例     | 步骤1:向A板发送接收命令<br/>  AT+MBOX_RECEIVE=0,1,16<br/>步骤2：向B板发送发送命令<br/> AT+MBOX_SEND=0,0,16<br/>测试完成后A板和B板对应串口会打印OK；否则测试失败 |
| 备注     ||

### MBOX_RECEIVE
| 命令格式 | AT+MBOX_RECEIVE = mailbox_id,channel_id,trans_size |
|--------- |-----------------------------------------------------------------------------------------------------------------------------------------------------------|
| 命令功能 | 测试MAILBOX模块的接收功能|
| 可搭配的辅助测试命令 | AT+MBOX_SEND                                                  |
| 命令参数 | mailbox_id：设备号<br/>channel_id:传输通道<br/>trans_size：数据传输长度<br/> |
| 示例     | 步骤1:向A板发送接收命令<br/>  AT+MBOX_RECEIVE=0,1,16<br/>步骤2：向B板发送发送命令<br/> AT+MBOX_SEND=0,0,16<br/>测试完成后A板和B板对应串口会打印OK；否则测试失败 |
| 备注     ||



### MBOX_SEND_PERFORMANCE

| 命令格式             | AT+MBOX_SEND_PERFORMANCE= mailbox_id,channel_id,trans_size,number_of_loop |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试MAILBOX模块的发送功能                                    |
| 可搭配的辅助测试命令 | AT+MBOX_RECEIVE_PERFORMANCE                                  |
| 命令参数             | mailbox_id：设备号<br/>channel_id:传输通道<br/>trans_size：数据传输长度<br/>number_of_loop：传输次数 |
| 示例                 | 步骤1:向A板发送接收命令<br/>  AT+MBOX_RECEIVE_PERFORMANCE=0,1,16,10000<br/>步骤2：向B板发送发送命令<br/> AT+MBOX_SEND_PERFORMANCE=0,0,16,10000<br/>测试完成后A板和B板对应串口会打印OK；否则测试失败 |
| 备注                 |                                                              |

### MBOX_RECEIVE_PERFORMANCE

| 命令格式             | AT+MBOX_RECEIVE_PERFORMANCE = mailbox_id,channel_id,trans_size,number_of_loop |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试MAILBOX模块的接收功能                                    |
| 可搭配的辅助测试命令 | AT+MBOX_SEND_PERFORMANCE                                     |
| 命令参数             | mailbox_id：设备号<br/>channel_id:传输通道<br/>trans_size：数据传输长度<br/>number_of_loop：传输次数 |
| 示例                 | 步骤1:向A板发送接收命令<br/>  AT+MBOX_RECEIVE_PERFORMANCE=0,1,16,10000<br/>步骤2：向B板发送发送命令<br/> AT+MBOX_SEND_PERFORMANCE=0,0,16,10000<br/>测试完成后A板和B板对应串口会打印OK；否则测试失败 |
| 备注                 |                                                              |

### AT+MBOX_INTERFACE

| 命令格式 | AT+MBOX_INTERFACE                                            |
| -------- | ------------------------------------------------------------ |
| 命令功能 | MAILBOX模块接口测试。该命令将会测试MAILBOX模块下所有的CSI 接口， 测试CSI接口时将会传入异常值、非法值，验证CSI接口返回值是否符合期望 |
| 命令参数 | 无                                                           |
| 示例     | 发送命令AT+MBOX_INTERFACE到A板，测试完成后， 测试板对应串口会打印OK；否则测试失败 |



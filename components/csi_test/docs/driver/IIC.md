## IIC
### 概要
该文档主要描述CSI2.0自动化测试框架下IIC模块的AT测试命令。 文档中所涉及的命令是用来测试IIC模块的接口工作是否正常、接口返回值是否符合预期和该模块的特性功能是否正常。
### 测试原理
PC主机用USB转串口线连接测试板（以下简称A板）和辅助测试板（以下简称B板），作为PC同A板或B板数据通信，命令收发的通道；同时PC会使用USB线连接A板和B板的HAD调试器，作为程序下载的通道；A板上被测试的IIC通过杜邦线连接到B板的IIC。通过PC下发AT指令到A板测试程序，发送AT指令到B板运行辅助测试程序，当A板和B板都有测试成功的信息返回PC后，PC判定本次测试执行通过。


### 测试流程
- 测试前应当连接好A板和B板之间的IIC功能引脚
- 发送引脚复用的配置命令（PINMUX_CONFIG）到A板和B板，配置好需要测试的IIC引脚
- 发送IIC的AT测试命令进行功能测试

### IIC功能AT命令列表概览
以下是IIC模块对应的AT测试命令列表及其功能简要介绍。

| AT命令列表                       | 功能简介                   |
| -------------------------------- | -------------------------- |
| AT+IIC_INTERFACE | IIC接口异常测试 |
| AT+IIC_MASTER_ASYNC_SEND         | 测试IIC作为master使用异步模式发送     |
| AT+IIC_MASTER_SYNC_SEND         | 测试IIC作为master使用同步模式发送     |
| AT+IIC_MASTER_DMA_SEND         | 测试IIC作为master使用DMA模式发送     |
| AT+IIC_SLAVE_ASYNC_SEND         | 测试IIC作为slave使用异步模式发送     |
| AT+IIC_SLAVE_SYNC_SEND         | 测试IIC作为slave使用同步模式发送     |
| AT+IIC_SLAVE_DMA_SEND         | 测试IIC作为slave使用DMA模式发送     |
| AT+IIC_MASTER_ASYNC_RECEIVE         | 测试IIC作为master使用异步模式接收     |
| AT+IIC_MASTER_SYNC_RECEIVE         | 测试IIC作为master使用同步模式接收     |
| AT+IIC_MASTER_DMA_RECEIVE         | 测试IIC作为master使用DMA模式接收     |
| AT+IIC_SLAVE_ASYNC_RECEIVE         | 测试IIC作为slave使用异步模式接收     |
| AT+IIC_SLAVE_SYNC_RECEIVE         | 测试IIC作为slave使用同步模式接收     |
| AT+IIC_SLAVE_DMA_RECEIVE         | 测试IIC作为slave使用DMA模式接收     |
| AT+IIC_MEMORY_TRANSFER         | 测试IIC外接memory    |
| AT+IIC_GET_STATE_MASTER_SEND | 测试IIC作为master发送阶段的状态获取     |
| AT+IIC_GET_STATE_SLAVE_RECEIVE         | 测试IIC作为slave接收阶段的状态获取     |


### 参数列表
以下是IIC模块对应的AT测试命令共用参数的简要介绍。

| 参数名字      | 释义           | 取值范围   |
| ------------- | -------------- | --------- |
| idx           | 表示设备号,比如0表示iic0 | 0、1、...，具体可测编号需参考soc.h |
| addr_mode | 表示IIC配置的地址模式 | 0:IIC_ADDRESS_7BIT,7位地址模式<br/>1:IIC_ADDRESS_10BIT,10位地址模式 |
| speed | 表示IIC配置的传输模式 | 0:IIC_BUS_SPEED_STANDARD,<=100kHz<br/>1:IIC_BUS_SPEED_FAST,<=400kHz<br/>2:IIC_BUS_SPEED_FAST_PLUS,<=1MHz<br/>3:IIC_BUS_SPEED_HIGH,<=3.4MHz |
| trans_size | 表示IIC发起一次传输指定的传输size | 0-0xFFFFFFFF |
| slave_addr | 表示IIC通信时指定的slave设备地址 | 0-0xFFFFFFFF, 地址须考虑支持的地址模式 |
| own_addr | 表示IIC配置控制器的地址 | 0-0xFFFFFFFF, 地址须考虑支持的地址模式 |
| timeout | 表示IIC使用同步模式传输时，设置传输的bit之间的超时时间 | 0-0xFFFFFFFF |
| mem_addr | 表示IIC指定需要操作的外接器件存储地址 | 0-0xFFFFFFFF |
| mem_addr_size | 表示IIC指定需要操作的外接器件存储地址大小 | 0-0xFFFFFFFF |

### 命令详解

#### AT+IIC_INTERFACE

| 命令格式 | AT+IIC_INTERFACE                                             |
| -------- | ------------------------------------------------------------ |
| 命令功能 | IIC接口异常测试。该命令测试IIC所CSI接口的异常调用，测试时将会传入异常和非法参数，验证CSI接口是否返回相应的错误码。 |
| 示例     | 发送命令AT+IIC_INTERFACE到A板，测试完成后，A板对应串口会打印OK；否则测试失败 |

#### AT+IIC_MASTER_ASYNC_SEND
| 命令格式 | AT+IIC_MASTER_ASYNC_SEND=idx,addr_mode,speed,trans_size,slave_addr |
|----------------------| --------------|
| 命令功能 | 测试IIC作为master使用异步模式发送 |
| 可搭配的辅助测试命令 | AT+IIC_SLAVE_ASYNC_RECEIVE;AT+IIC_SLAVE_SYNC_RECEIVE;AT+IIC_SLAVE_DMA_RECEIVE |
| 示例 | 1.向B板发送命令:AT+IIC_SLAVE_ASYNC_RECEIVE=0,0,0,0,1024<br>2.向A板发送命令:AT+IIC_MASTER_ASYNC_SEND=0,0,0,1024,0<br/>若测试通过，则连接A板和B板的串口最终会打印OK |
| 备注 | 从机命令要先于主机命令发出 |

#### AT+IIC_SLAVE_ASYNC_RECEIVE

| 命令格式             | AT+IIC_SLAVE_ASYNC_RECEIVE=idx,addr_mode,speed,own_addr,trans_size |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试IIC作为slave使用异步模式接收                             |
| 可搭配的辅助测试命令 | AT+IIC_MASTER_ASYNC_SEND;AT+IIC_MASTER_SYNC_SEND;AT+IIC_MASTER_DMA_SEND |
| 示例                 | 1.向A板发送命令:AT+IIC_SLAVE_ASYNC_RECEIVE=0,0,0,0,1024<br>2.向B板发送命令:AT+IIC_MASTER_ASYNC_SEND=0,0,0,1024,0<br/>若测试通过，则连接A板和B板的串口最终会打印OK |
| 备注                 | 从机命令要先于主机命令发出                                   |

#### AT+IIC_MASTER_SYNC_SEND

| 命令格式 | AT+IIC_MASTER_SYNC_SEND=idx,addr_mode,speed,trans_size,slave_addr,timeout |
|----------------------| --------------|
| 命令功能 | 测试IIC作为master使用同步模式发送 |
| 可搭配的辅助测试命令 | AT+IIC_SLAVE_ASYNC_RECEIVE;AT+IIC_SLAVE_SYNC_RECEIVE;AT+IIC_SLAVE_DMA_RECEIVE |
| 示例 | 1.向B板发送命令:AT+IIC_SLAVE_SYNC_RECEIVE=0,0,0,0,1024,1000<br>2.向A板发送命令:AT+IIC_MASTER_SYNC_SEND=0,0,0,1024,0,1000<br/>若测试通过，则连接A板和B板的串口最终会打印OK |
| 备注 | 从机命令要先于主机命令发出 |

#### AT+IIC_SLAVE_SYNC_RECEIVE

| 命令格式             | AT+IIC_SLAVE_SYNC_RECEIVE=idx,addr_mode,speed,own_addr,trans_size |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试IIC作为slave使用同步模式接收                             |
| 可搭配的辅助测试命令 | AT+IIC_MASTER_ASYNC_SEND;AT+IIC_MASTER_SYNC_SEND;AT+IIC_MASTER_DMA_SEND |
| 示例                 | 1.向A板发送命令:AT+IIC_SLAVE_SYNC_RECEIVE=0,0,0,0,1024,1000<br>2.向B板发送命令:AT+IIC_MASTER_SYNC_SEND=0,0,0,1024,0,1000<br>若测试通过，则连接A板和B板的串口最终会打印OK |
| 备注                 | 从机命令要先于主机命令发出                                   |

#### AT+IIC_MASTER_DMA_SEND

| 命令格式 | AT+IIC_MASTER_DMA_SEND=idx,addr_mode,speed,trans_size,slave_addr |
|----------------------| --------------|
| 命令功能 | 测试IIC作为master使用DMA模式发送 |
| 可搭配的辅助测试命令 | AT+IIC_SLAVE_ASYNC_RECEIVE;AT+IIC_SLAVE_SYNC_RECEIVE;AT+IIC_SLAVE_DMA_RECEIVE |
| 示例 | 1.向B板发送命令:AT+IIC_SLAVE_DMA_RECEIVE=0,0,0,1024<br>2.向A板发送命令:AT+IIC_MASTER_DMA_SEND=0,0,0,1024,0<br>若测试通过，则连接A板和B板的串口最终会打印OK |
| 备注 | 从机命令要先于主机命令发出 |

#### AT+IIC_SLAVE_DMA_RECEIVE

| 命令格式             | AT+IIC_SLAVE_DMA_RECEIVE=idx,addr_mode,speed,own_addr,trans_size |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试IIC作为slave使用DMA模式接收                              |
| 可搭配的辅助测试命令 | AT+IIC_MASTER_ASYNC_SEND;AT+IIC_MASTER_SYNC_SEND;AT+IIC_MASTER_DMA_SEND |
| 示例                 | 1.向A板发送命令:AT+IIC_SLAVE_DMA_RECEIVE=0,0,0,0,1024<br>2.向B板发送命令:AT+IIC_MASTER_DMA_SEND=0,0,0,1024,0<br>若测试通过，则连接A板和B板的串口最终会打印OK |
| 备注                 | 从机命令要先于主机命令发出                                   |

#### AT+IIC_SLAVE_ASYNC_SEND

| 命令格式 | AT+IIC_SLAVE_ASYNC_SEND=idx,addr_mode,speed,own_addr,trans_size |
|----------------------| --------------|
| 命令功能 | 测试IIC作为slave使用异步模式发送 |
| 可搭配的辅助测试命令 | AT+IIC_MASTER_ASYNC_RECEIVE;AT+IIC_MASTER_SYNC_RECEIVE;AT+IIC_MASTER_DMA_RECEIVE |
| 示例 | 1.向A板发送命令:AT+IIC_SLAVE_ASYNC_SEND=0,0,0,0,1024<br>2.向B板发送命令:AT+IIC_MASTER_ASYNC_RECEIVE=0,0,0,1024,0<br>若测试通过，则连接A板和B板的串口最终会打印OK |
| 备注 | 从机命令要先于主机命令发出 |

#### AT+IIC_MASTER_ASYNC_RECEIVE

| 命令格式             | AT+IIC_MASTER_ASYNC_RECEIVE=idx,addr_mode,speed,trans_size,slave_addr |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试IIC作为master使用异步模式接收                            |
| 可搭配的辅助测试命令 | AT+IIC_SLAVE_ASYNC_SEND;AT+IIC_SLAVE_SYNC_SEND;AT+IIC_SLAVE_DMA_SEND |
| 示例                 | 1.向B板发送命令:AT+IIC_SLAVE_ASYNC_SEND=0,0,0,0,1024<br>2.向A板发送命令:AT+IIC_MASTER_ASYNC_RECEIVE=0,0,0,1024,0,1000<br>若测试通过，则连接A板和B板的串口最终会打印OK |
| 备注                 | 从机命令要先于主机命令发出                                   |

#### AT+IIC_SLAVE_SYNC_SEND

| 命令格式 | AT+IIC_SLAVE_SYNC_SEND=idx,addr_mode,speed,own_addr,trans_size,timeout |
|----------------------| --------------|
| 命令功能 | 测试IIC作为slave使用异步模式发送 |
| 可搭配的辅助测试命令 | AT+IIC_MASTER_ASYNC_RECEIVE;AT+IIC_MASTER_SYNC_RECEIVE;AT+IIC_MASTER_DMA_RECEIVE |
| 示例 | 1.向A板发送命令:AT+IIC_SLAVE_SYNC_SEND=0,0,0,0,1024,1000<br>2.向B板发送命令:AT+IIC_MASTER_SYNC_RECEIVE=0,0,0,1024,0,1000<br>若测试通过，则连接A板和B板的串口最终会打印OK |
| 备注 | 从机命令要先于主机命令发出 |

#### AT+IIC_MASTER_SYNC_RECEIVE

| 命令格式             | AT+IIC_MASTER_SYNC_RECEIVE=idx,addr_mode,speed,trans_size,slave_addr,timeout |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 测试IIC作为master使用同步模式接收                            |
| 可搭配的辅助测试命令 | AT+IIC_SLAVE_ASYNC_SEND;AT+IIC_SLAVE_SYNC_SEND;AT+IIC_SLAVE_DMA_SEND |
| 示例                 | 1.向B板发送命令:AT+IIC_SLAVE_SYNC_SEND=0,0,0,0,1024,1000<br>2.向A板发送命令:AT+IIC_MASTER_SYNC_RECEIVE=0,0,0,1024,0,1000<br>若测试通过，则连接A板和B板的串口最终会打印OK |
| 备注                 | 从机命令要先于主机命令发出                                   |

#### AT+IIC_SLAVE_DMA_SEND

| 命令格式 | AT+IIC_SLAVE_DMA_SEND=idx,addr_mode,speed,own_addr,trans_size |
|----------------------| --------------|
| 命令功能 | 测试IIC作为slave使用DMA模式发送 |
| 可搭配的辅助测试命令 | AT+IIC_MASTER_ASYNC_RECEIVE;AT+IIC_MASTER_SYNC_RECEIVE;AT+IIC_MASTER_DMA_RECEIVE |
| 示例 | 1.向A板发送命令:AT+IIC_SLAVE_DMA_SEND=0,0,0,0,1024<br>2.向B板发送命令:AT+IIC_MASTER_DMA_RECEIVE=0,0,0,1024,0<br>若测试通过，则连接A板和B板的串口最终会打印OK |
| 备注 | 从机命令要先于主机命令发出 |

#### AT+IIC_MASTER_DMA_RECEIVE
| 命令格式 | AT+IIC_MASTER_DMA_RECEIVE=idx,addr_mode,speed,trans_size,slave_addr |
|----------------------| --------------|
| 命令功能 | 测试IIC作为master使用DMA模式接收 |
| 可搭配的辅助测试命令 | AT+IIC_SLAVE_ASYNC_SEND;AT+IIC_SLAVE_SYNC_SEND;AT+IIC_SLAVE_DMA_SEND |
| 示例 | 1.向B板发送命令:AT+IIC_SLAVE_DMA_SEND=0,0,0,0,1024<br>2.向A板发送命令:AT+IIC_MASTER_DMA_RECEIVE=0,0,0,1024,0<br>若测试通过，则连接A板和B板的串口最终会打印OK |
| 备注 | 从机命令要先于主机命令发出 |


#### AT+IIC_MEMORY_TRANSFER
| 命令格式 | AT+IIC_MEMORY_TRANSFER=idx,addr_mode,speed,trans_size,slave_addr,timeout,<br/>mem_addr,mem_addr_size |
|----------------------| --------------|
| 命令功能 | 测试IIC外接memory |
| 示例 | 1.向A板发送命令:AT+IIC_MEMORY_TRANSFER=0,0,0,1024,0,1000,0,1024 |
| 备注 | 测试时需要先外接一个IIC接口的memory芯片 |


#### AT+IIC_GET_STATE_MASTER_SEND
| 命令格式 | AT+IIC_GET_STATE_MASTER_SEND=idx,addr_mode,speed,trans_size,slave_addr |
|----------------------| --------------|
| 命令功能 | 测试IIC作为master发送阶段的状态获取 |
| 可搭配的辅助测试命令 | AT+IIC_GET_STATE_SLAVE_RECEIVE |
| 示例 | 1.向B板发送命令:AT+IIC_GET_STATE_SLAVE_RECEIVE=0,0,0,0,1024<br>2.向A板发送命令:AT+IIC_GET_STATE_MASTER_SEND=0,0,0,1024,0<br>若测试通过，则连接A板和B板的串口最终会打印OK |
| 备注 | 从机命令要先于主机命令发出 |

#### AT+IIC_GET_STATE_SLAVE_RECEIVE
| 命令格式 | AT+IIC_GET_STATE_SLAVE_RECEIVE=idx,addr_mode,speed,own_addr,trans_size |
|----------------------| --------------|
| 命令功能 | 测试IIC作为slave接收阶段的状态获取 |
| 可搭配的辅助测试命令 | AT+IIC_GET_STATE_MASTER_SEND |
| 示例 | 1.向A板发送命令:AT+IIC_GET_STATE_SLAVE_RECEIVE=0,0,0,0,1024<br>2.向B板发送命令:AT+IIC_GET_STATE_MASTER_SEND=0,0,0,1024,0<br>若测试通过，则连接A板和B板的串口最终会打印OK |
| 备注 | 从机命令要先于主机命令发出 |
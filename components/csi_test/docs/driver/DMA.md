## DMA

### 概要

该文档主要描述DMA模块的CSI2.0自动化测试AT命令。 文档中所涉及的这些命令是用来测试DMA模块的接口工作是否正常、数据传输是否正确、接口返回值是否符合预期。

### 测试原理

测试时，应给被测试板发送DMA测试命令进行相应的测试；测试程序会按照按照测试命令的配置进行相应的测试， 若结果与预期一致，则相应的测试成功完成，否则测试失败。

### 测试流程

- 测试前应当准备好测试板（以下简称A板）
- 发送DMA测试命令进行测试 （测试命令功能请参照下文）

### DMA功能AT命令列表概览

以下是DMA模块对应的测试命令列表及其功能简要介绍。

| AT命令列表                | 功能                                    |
| ------------------------- | --------------------------------------- |
| AT+DMA_INTERFACE          | DMA接口异常测试。                       |
| AT+DMA_MEMORY_TO_MEMORY   | 测试内存到内存的数据传输通过DMA搬运     |
| AT+DMA_CHANNEL_ALLOC_FREE | 测试DMA模块的通道申请和释放             |
| AT+DMA_HALF_DONE_EVENT    | 测试DMA模块的数据传输完成一半的事件触发 |

### 参数列表

以下是DMA模块对应的AT测试命令共用参数的简要介绍。

| 参数            | 释义                              | 取值范围                                                     |
| --------------- | --------------------------------- | ------------------------------------------------------------ |
| dma_idx         | 表示设备号，0表示dma0             | 0、1、...，具体可测编号需参考soc.h                           |
| channel_idx     | 表示通道号，比如0表示0通道        | 0、1、...，具体可测通道号需参考soc.h                         |
| data_width      | 表示DMA源数据/目的数据宽度        | 0：DMA_DATA_WIDTH_8_BITS<br/>1：DMA_DATA_WIDTH_16_BITS<br/>2：DMA_DATA_WIDTH_32_BITS<br/>3：DMA_DATA_WIDTH_64_BITS<br/>4：DMA_DATA_WIDTH_128_BITS<br/>5：DMA_DATA_WIDTH_512_BITS |
| src_addr_inc    | 表示DMA源地址增长方式             | 0：DMA_ADDR_INC，地址递增<br/>1：DMA_ADDR_DEC，地址递减<br/>2：DMA_ADDR_CONSTANT，地址固定 |
| dest_addr_inc   | 表示DMA目的地址增长方式           | 0：DMA_ADDR_INC，地址递增<br/>1：DMA_ADDR_DEC，地址递减<br/>2：DMA_ADDR_CONSTANT，地址固定 |
| group_length    | 表示数据传输时一组数据长度，bytes | 0x0 - 0xFFFFFFFF                                             |
| transfer_length | 表示传输的数据长度，bytes         | 0x0 - 0xFFFFFFFF                                             |

### 命令详解

#### AT+DMA_MEMORY_TO_MEMORY

| 命令格式 | AT+DMA_MEMORY_TO_MEMORY=dma_idx,channel_idx,data_width,src_addr_inc,<br/>dest_addr_inc,group_length,transfer_length |
|------------------------------------------|--------------------------------------------------------------------------------------------------|
| 命令功能 | 测试内存到内存的数据传输通过DMA搬运。 |
| 示例 | 发送命令AT+DMA_MEMORY_TO_MEMORY=0,0,2,0,1,100,4096到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |

#### AT+DMA_CHANNEL_ALLOC_FREE

| 命令格式 | AT+DMA_CHANNEL_ALLOC_FREE=dma_idx,channel_idx                |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试DMA模块的通道申请和释放。                                |
| 示例     | 发送命令AT+DMA_CHANNEL_ALLOC_FREE=0,0到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |

#### AT+DMA_HALF_DONE_EVENT

| 命令格式 | AT+DMA_HALF_DONE_EVENT=dma_idx,channel_idx,transfer_length   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试DMA模块的数据传输完成一半的事件触发                      |
| 示例     | 发送命令AT+DMA_HALF_DONE_EVENT=0,0,2048到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |

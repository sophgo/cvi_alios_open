## SPIFLASH

### 概要

该文档主要描述SPIFLASH模块的CSI2.0自动化测试AT命令。 文档中所涉及的这些命令是用来测试SPIFLASH模块的接口工作是否正常、数据传输是否正确、接口返回值是否符合预期。

### 测试原理

测试时，应给被测试板发送SPIFLASH测试命令进行相应的测试；测试程序会按照按照测试命令的配置进行相应的SPIFLASH测试， 若与预期一致，则相应的测试成功完成，否则测试失败。

### 测试流程

- 测试前应当准备好测试板（以下简称A板）
- 发送SPIFLASH测试命令进行测试 （测试命令功能请参照下文）

### SPIFLASH功能AT命令列表概览 

以下是SPIFLASH模块对应的测试命令列表及其功能简要介绍。

| AT命令列表               | 功能                           |
| ------------------------ | ------------------------------ |
| AT+SPIFLASH_INTERFACE | SPIFLASH接口异常测试 |
| AT+SPIFLASH_PROGRAM_READ | 测试基于SPI控制器的SPIFLASH模块的读写功能 |
| AT+SPIFLASH_LOCK         | 测试基于SPI控制器的SPIFLASH模块的写保护功能 |
| AT+SPIFLASH_QSPI_PROGRAMREAD | 测试基于QSPI控制器的SPIFLASH模块的读写功能 |
| AT+SPIFLASH_WRITE_READ | 测试QSPI与FLASH的读写，CS脚用GPIO控制 |

### 参数列表

以下是SPIFLASH模块对应的AT测试命令共用参数的简要介绍。

| 参数名字      | 释义                                                         | 取值范围                                                     |
| ------------- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| gpio_idx      | 表示GPIO设备号，gpio设备号通常情况下都为0                    | 0、1、...，具体可测编号需参考soc.h                           |
| gpio_pin_mask | 表示片选引脚，比如0表示pin0                                  | 0、1、...，具体可测编号需参考soc.h                           |
| idx           | 表示设备号,比如0表示spi0/qspi0                               | 0、1、...，具体可测编号需参考soc.h                           |
| offset        | 表示读写spi/qspi的起始地址<br>注意：**1. offset必须为扇区大小的整数倍**<br>**2.在ram中运行可以为0，但在flash中运行必须大于该程序所用到的最大地址** | 0x0 - 0xFFFFFFFF                                             |
| size          | 表示读写spi/qspi的大小<br>注意：**1. size必须为扇区大小的整数倍** | 0x0 - 0xFFFFFFFF                                             |
| line_mode     | 表示传输总线模式                                             | 1:SPIFLASH_DATA_1_LINE，单线模式<br/>2:SPIFLASH_DATA_2_LINES，双线模式<br/>4:SPIFLASH_DATA_4_LINES，四线模式 |
| frequence     | 表示传输频率，Hz                                             | 整数，例如：20000000、40000000、80000000....                 |

### 命令详解

#### AT+SPIFLASH_INTERFACE

| 命令格式 | AT+SPIFLASH_INTERFACE                                        |
| -------- | ------------------------------------------------------------ |
| 命令功能 | SPIFLASH接口异常测试。该命令测试SPIFLASH所CSI接口的异常调用，测试时将会传入异常和非法参数，验证CSI接口是否返回相应的错误码。 |
| 示例     | 发送命令AT+SPIFLASH_INTERFACE到A板，测试完成后，A板对应串口会打印OK；否则测试失败 |

#### AT+SPIFLASH_PROGRAM_READ

| 命令格式 | AT+SPIFLASH_PROGRAM_READ=gpio_idx,gpio_pin_mask,idx,offset,size |
|------------------------------------------|--------------------------------------------------------------------------------------------------|
| 命令功能 | 测试基于SPI控制器的SPIFLASH模块的读写功能<br>擦除spiflash内容，spiflash写入数据，spiflash读取数据，比对写入和读取的内容是否相同 |
| 示例 | 发送命令AT+SPIFLASH_PROGRAM_READ=0,1,0,0,256到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |

#### AT+SPIFLASH_LOCK

| 命令格式 | AT+SPIFLASH_LOCK=gpio_idx,gpio_pin_mask,idx,offset,size      |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试基于SPI控制器的SPIFLASH模块的写保护功能<br>擦除spiflash内容，打开spi写保护功能，spiflash写入数据，spiflash读取数据，<br/>比对写入和读取的内容，判断写入spiflash是否成功，<br/>关闭spi写保护功能，spiflash写入数据，spiflash读取数据，<br/>比对写入和读取的内容，判断写入spiflash是否成功 |
| 示例     | 发送命令AT+SPIFLASH_LOCK=0,1,0,0,256到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |

#### AT+SPIFLASH_QSPI_PROGRAMREAD

| 命令格式 | AT+SPIFLASH_QSPI_PROGRAMREAD=idx,offset,size,line_mode,frequence |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试基于QSPI控制器的SPIFLASH模块的读写功能<br/>擦除spiflash内容，spiflash写入数据，spiflash读取数据，比对写入和读取的内容是否相同 |
| 示例     | 发送命令AT+SPIFLASH_QSPI_PROGRAMREAD=0,0,256,1,25000000到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |

#### AT+SPIFLASH_WRITE_READ

| 命令格式 | AT+SPIFLASH_WRITE_READ=gpio_idx,gpio_pin_mask,idx,offset,size,frequence,line_mode |
|------------------------------------------|-----------------------------------|
| 命令功能 | 测试QSPI与FLASH模块的读取功能<br>擦除spiflash内容，spiflash写入数据，spiflash读取数据，比对写入和读取的内容是否相同 |
| 示例 | 发送命令AT+SPIFLASH_WRITE_READ=0,1,0,0,256,1,25000000到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |
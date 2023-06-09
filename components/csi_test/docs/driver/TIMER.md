## TIMER

### 概要

该文档主要描述TIMER模块的CSI2.0自动化测试AT命令。 文档中所涉及的这些命令是用来测试TIMER模块的接口工作是否正常、数据传输是否正确、接口返回值是否符合预期。

### 测试原理

测试时，应给被测试板发送TIMER测试命令进行相应的测试；测试程序会按照按照测试命令的配置进行相应的TIMER测试， 若与预期一致，则相应的测试成功完成，否则测试失败。

命令参数中的超时时间建议在毫秒级，并且不超过TIMER单个最大计时周期（以CH2601为例，TIMER为32为，时钟约49MHz，最长计时约为87s）。

### 测试流程

- 测试前应当准备好测试板（以下简称A板）
- 发送TIMER测试命令进行测试 （测试命令功能请参照下文）

### TIMER功能AT命令列表概览

以下是TIMER模块对应的测试命令列表及其功能简要介绍。

| AT命令列表                 | 功能                            |
| -------------------------- | ------------------------------- |
| AT+TIMER_INTERFACE         | TIMER接口异常测试               |
| AT+TIMER_SETTING           | 测试TIMER模块的打开关闭的功能   |
| AT+TIMER_PRECISION         | 测试TIMER模块的计时功能         |
| AT+TIMER_GETLOADVALUE      | 测试TIMER模块的获取负载值的功能 |
| AT+TIMER_CALLBACK          | 测试TIMER模块的回调功能         |
| AT+TIMER_STATE             | 测试TIMER模块的状态查询功能     |
| AT+TIMER_GETREMAININGVALUE | 测试TIMER模块获取当前值的功能   |

### 参数列表

以下是TIMER模块对应的AT测试命令共用参数的简要介绍。

| 参数名字 | 释义                       | 取值范围                           |
| -------- | -------------------------- | ---------------------------------- |
| idx      | 表示设备号,比如0表示timer0 | 0、1、...，具体可测编号需参考soc.h |
| timeout  | 表示延时的时间(单位us)     |                                    |

### 命令详解

#### AT+TIMER_INTERFACE

| 命令格式 | AT+TIMER_INTERFACE                                           |
| -------- | ------------------------------------------------------------ |
| 命令功能 | RTC接口异常测试。该命令测试RTC所CSI接口的异常调用，测试时将会传入异常和非法参数，验证CSI接口是否返回相应的错误码。 |
| 示例     | 发送命令AT+TIMER_INTERFACE到A板，测试完成后，A板对应串口会打印OK；否则测试失败 |

#### AT+TIMER_PRECISION

| 命令格式 | AT+TIMER_PRECISION=idx,timeout                               |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试TIMER模块的计时功能<br/>打开定时器，延时后，查看定时器计时是否准确 |
| 示例     | 发送命令AT+TIMER_PRECISION=0,1000到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |

#### AT+TIMER_STATE

| 命令格式 | AT+TIMER_STATE=idx,timeout                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试TIMER模块的状态查询功能测试<br/>设置TIMER超时时间，打开TIMER，查看TIMER是否打开，关闭TIMER，查看TIMER是否关闭 |
| 示例     | 发送命令AT+TIMER_STATE=0,1000到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |

#### AT+TIMER_SETTING

| 命令格式 | AT+TIMER_SETTING=idx,timeout                                 |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试TIMER模块的打开关闭的功能<br/>设置TIMER超时时间，打开TIMER，关闭TIMER |
| 示例     | 发送命令AT+TIMER_SETTING=0,1000到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |

#### AT+TIMER_GETLOADVALUE

| 命令格式 | AT+TIMER_GETLOADVALUE=idx,timeout                            |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试TIMER模块的获取负载值的功能<br/>打开TIMER，获取负载值，查看获取发负载值是否正确 |
| 示例     | 发送命令AT+TIMER_GETLOADVALUE=0,1000到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |

#### AT+TIMER_GETREMAININGVALUE

| 命令格式 | AT+TIMER_GETREMAININGVALUE=idx,timeout                       |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试TIMER模块的获取当前定时器值的功能<br/>打开TIMER，在定时器中断前，获取定时器当前值，对比数值是否正确 |
| 示例     | 发送命令AT+TIMER_GETREMAININGVALUE=0,1000到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |

#### AT+TIMER_CALLBACK

| 命令格式 | AT+TIMER_CALLBACK=idx,timeout                                |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试TIMER模块的回调功能<br/>TIMER初始化，进入回调，打开TIMER，离开回调，关闭TIMER |
| 示例     | 发送命令AT+TIMER_CALLBACK=0,1000到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |

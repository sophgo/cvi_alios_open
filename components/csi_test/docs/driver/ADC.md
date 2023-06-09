## ADC

### 概要

该文档主要描述ADC模块的CSI2.0自动化测试AT命令。 文档中所涉及的这些命令是用来测试ADC模块的接口工作是否正常、数据传输是否正确、接口返回值是否符合预期。

### 测试原理

测试时，应给被测试板发送ADC测试命令进行相应的测试；测试程序会按照按照测试命令的配置进行相应的测试， 若结果与预期一致，则相应的测试成功完成，否则测试失败。

### 测试流程

- 测试前应当准备好测试板（以下简称A板），A板的ADC测试引脚接高或者接地
- 发送引脚复用的配置命令（PINMUX_CONFIG）到A板，配置好需要测试的ADC
- 发送ADC测试命令进行测试 （测试命令功能请参照下文）

### ADC功能AT命令列表概览

以下是ADC模块对应的测试命令列表及其功能简要介绍。

| AT命令列表       | 功能                                                |
| ---------------- | --------------------------------------------------- |
| AT+ADC_INTERFACE | ADC接口异常测试                                     |
| AT+ADC_SYNC      | 测试ADC模块的同步模式                               |
| AT+ADC_ASYNC     | 测试ADC模块的异步模式                               |
| AT+ADC_DMA       | 测试ADC模块的DMA模式                                |
| AT+ADC_STATUS    | 测试ADC模块的启停的状态，以及分频的信息的设置与读取 |

### 参数列表

以下是ADC模块对应的AT测试命令共用参数的简要介绍。

| 参数          | 释义                           | 取值范围                                   |
| ------------- | ------------------------------ | ------------------------------------------ |
| idx           | 表示设备号,比如0表示adc0       | 0、1、...，具体可测编号需参考soc.h         |
| s_time        | 表示采样时钟周期个数           | 1、2、...                                  |
| continue_mode | 表示开启/关闭ADC连续采样模式   | 0：禁用连续采样模式<br>1：启用连续采样模式 |
| freq_div      | 表示ADC分频系数                | 1、2、...、16、...、64、...、256、...      |
| exp_max       | 表示预期采样结果最大值         |                                            |
| exp_min       | 表示预期采样结果最小值         |                                            |
| channel       | 表示通道号,比如0表示adc的0通道 | 0、1、...，具体可测编号需参考soc.h         |

### 命令详解

#### AT+ADC_INTERFACE

| 命令格式 | AT+ADC_INTERFACE                                             |
| -------- | ------------------------------------------------------------ |
| 命令功能 | ADC接口异常测试。该命令测试ADC所CSI接口的异常调用，测试时将会传入异常和非法参数，验证CSI接口是否返回相应的错误码。 |
| 示例     | 发送命令AT+ADC_INTERFACE 到A板，测试完成后，A板对应串口会打印OK；否则测试失败 |

#### AT+ADC_SYNC

| 命令格式 | AT+ADC_SYNC=idx,s_time,continue_mode,freq_div,exp_max,exp_min,channel |
|------------------------------------------|--------------------------------------------------------------------------------------------------|
| 命令功能 | 测试ADC模块的同步模式 |
| 示例 | 发送命令AT+ADC_SYNC=0,1,0,2,275,270,1到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |

#### AT+ADC_ASYNC

| 命令格式 | AT+ADC_ASYNC=idx,s_time,continue_mode,freq_div,exp_max,exp_min,channel |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试ADC模块的异步模式                                        |
| 示例     | 发送命令AT+ADC_ASYNC=1,0,2,275,270,1到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |

#### AT+ADC_DMA

| 命令格式 | AT+ADC_DMA=idx,s_time,continue_mode,freq_div,exp_max,exp_min,channel |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试ADC模块的DMA模式                                         |
| 示例     | 发送命令AT+ADC_DMA=1,0,2,275,270,1到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |

#### AT+ADC_STATUS

| 命令格式 | AT+ADC_STATUS=idx,channel                                    |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试ADC模块的启停的状态，以及分频的信息的设置与读取          |
| 示例     | 发送命令AT+ADC_STATUS=0,1到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |


## PWM

### 概要

该文档主要描述PWM模块的CSI2.0自动化测试AT命令。 文档中所涉及的这些命令是用来测试PWM模块的接口工作是否正常、输入输出是否正确、接口返回值是否符合预期。

### 测试原理

测试时，应使被测试板与另外一块具备GPIO能的板子连接，并发送AT+PINMUX_CONFIG到被测试板把需要测试的引脚功能复用成PWM的引脚功能后，然后发送PWM测试命令进行相应的测试；测试程序会按照按照测试命令的配置进行相应的输入输出测试， 输入输出结果与预期一致，则相应的测试成功完成，否则测试失败。

### 测试流程

- 测试前应当连接好A板和B板之间的PWM和GPIO功能引脚
- 发送引脚复用的配置命令（PINMUX_CONFIG）到A板和B板，配置好需要测试的PWM和GPIO引脚
- 发送PWM的AT测试命令进行功能测试

### PWM功能AT命令列表概览

以下是PWM模块对应的测试命令列表及其功能简要介绍。

| AT命令列表           | 功能                |
| -------------------- | ------------------- |
| AT+PWM_INTERFACE     | PWM接口异常测试     |
| AT+PWM_OUTPUT        | PWM模块输出测试     |
| AT+PWM_OUTPUT_CHECK  | 辅助PWM模块输出测试 |
| AT+PWM_CAPTURE       | PWM模块捕获测试     |
| AT+PWM_CAPTURE_INPUT | 辅助PWM模块捕获测试 |

### 参数列表

以下是PWM模块对应的AT测试命令共用参数的简要介绍。

| 参数名字         | 释义                           | 取值范围                                                     |
| ---------------- | ------------------------------ | ------------------------------------------------------------ |
| idx              | 表示设备号,比如0表示pwm0       | 0、1、...，具体可测编号需参考soc.h                           |
| channel          | 表示通道号,比如0表示pwm的0通道 | 0、1、...，具体可测编号需参考soc.h                           |
| period           | 表示信号周期时间（单位us）     |                                                              |
| pulse_width      | 表示有效电平时间（单位us）     |                                                              |
| output_polarity  | 表示有效电平极性               | 0:PWM_POLARITY_HIGH，高电平<br/>1:PWM_POLARITY_LOW，低电平   |
| delay_ms         | 表示延时的时间(单位ms)         |                                                              |
| pin              | 表示单个引脚,比如0表示pin0     | 0、1、...，具体可测编号需参考soc.h                           |
| capture_polarity | 表示捕获方式                   | 0：PWM_CAPTURE_POLARITY_POSEDGE，上升沿捕获类型<br/>1：PWM_CAPTURE_POLARITY_NEGEDGE，下降沿捕获类型<br/>2：PWM_CAPTURE_POLARITY_BOTHEDGE，边沿捕获类型 |
| capture_count    | 表示捕获次数                   | 0、1、...                                                    |

### 命令详解

#### AT+PIN_INTERFACE

| 命令格式 | AT+PIN_INTERFACE                                             |
| -------- | ------------------------------------------------------------ |
| 命令功能 | PIN接口异常测试。该命令测试PIN所CSI接口的异常调用，测试时将会传入异常和非法参数，验证CSI接口是否返回相应的错误码。 |
| 示例     | 发送命令AT+PIN_INTERFACE到A板，测试完成后，A板对应串口会打印OK；否则测试失败 |

#### AT+PWM_OUTPUT

| 命令格式             | AT+PWM_OUTPUT=idx,channel,period,pulse_width,output_polarity,delay_ms |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | PWM模块输出测试                                              |
| 可搭配的辅助测试命令 | AT+PWM_OUTPUT_CHECK=idx,pin,period,pulse_width,output_polarity |
| 示例                 | 把A板的PA0和B板的PA1这两个引脚使用杜邦线连接, 然后按照下列步骤进行测试<br/>步骤1:配置A板的PA0引脚作为PWM的功能,向A板发送下列命令<br/>   AT+PINMUX_CONFIG=0,16<br/>  板子对应串口会打印OK；否则引脚配置失败<br/>步骤2:配置B板的PA1引脚作为GPIO的功能,向B板发送下列命令<br/>   AT+PINMUX_CONFIG=1,4<br/>   板子对应串口会打印OK；否则引脚配置失败<br/>步骤3:向A板发送测试PWM模块输出的命令<br/>   AT+PWM_OUTPUT=0,0,10,5,0,5000<br/>   测试完成后， 板子对应串口会打印OK；否则测试失败<br/>步骤4:向B板发送辅助测试PWM模块输出的命令<br/>   AT+PWM_OUTPUT_CHECK=0,1,10,5,0<br/>   测试完成后， 板子对应串口会打印OK；否则测试失败 |

### AT+PWM_OUTPUT_CHECK

| 命令格式             | AT+PWM_OUTPUT=idx,pin,period,pulse_width,output_polarity     |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 辅助PWM模块输出测试                                          |
| 可搭配的辅助测试命令 | AT+PWM_OUTPUT=idx,channel,period,pulse_width,output_polarity,delay_ms |
| 示例                 | 参考AT+PWM_OUTPUT示例                                        |

### AT+PWM_CAPTURE

| 命令格式             | AT+PWM_CAPTURE=idx,channel,capture_polarity,capture_count,delay_ms |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | PWM模块捕获测试测试                                          |
| 可搭配的辅助测试命令 | AT+PWM_CAPTURE_INPUT=idx,pin,capture_polarity,capture_count,delay_ms |
| 示例                 | 把A板的PA0和B板的PA1这两个引脚使用杜邦线连接, 然后按照下列步骤进行测试<br/>步骤1:配置A板的PA0引脚作为PWM的功能,向A板发送下列命令<br/>   AT+PINMUX_CONFIG=0,16<br/>  板子对应串口会打印OK；否则引脚配置失败<br/>步骤2:配置B板的PA1引脚作为GPIO的功能,向B板发送下列命令<br/>   AT+PINMUX_CONFIG=1,4<br/>   板子对应串口会打印OK；否则引脚配置失败<br/>步骤3:向B板发送辅助PWM模块捕获的命令<br/>   AT+PWM_CAPTURE_INPUT=0,1,0,1,1000<br/>   测试完成后， 板子对应串口会打印OK；否则测试失败<br/>步骤4:向A板发送辅助测试PWM模块捕获的命令<br/>   AT+PWM_CAPTURE=0,0,0,1,1000<br/>   测试完成后， 板子对应串口会打印OK；否则测试失败 |

### AT+PWM_CAPTURE_INPUT

| 命令格式             | AT+PWM_CAPTURE_INPUT=idx,pin,capture_polarity,capture_count,delay_ms |
| -------------------- | ------------------------------------------------------------ |
| 命令功能             | 辅助PWM模块捕获测试                                          |
| 可搭配的辅助测试命令 | AT+PWM_CAPTURE=idx,channel,capture_polarity,capture_count,delay_ms |
| 示例                 | 参考AT+PWM_CAPTURE示例                                       |

### 

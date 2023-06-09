## PIN

### 概要

该文档主要描述PIN模块的CSI2.0自动化测试AT命令。 文档中所涉及的这些命令是用来测试PIN模块的接口工作是否正常、数据传输是否正确、接口返回值是否符合预期。



### 测试原理

测试时，应给被测试板发送PIN测试命令进行相应的测试；测试程序会按照按照测试命令的配置进行相应的测试， 若结果与预期一致，则相应的测试成功完成，否则测试失败。

### 测试流程

- 测试前应当准备好测试板（以下简称A板）
- 发送PIN测试命令进行测试 （测试命令功能请参照下文）

### PIN功能AT命令列表概览

以下是PIN模块对应的测试命令列表及其功能简要介绍。

| AT命令列表    | 功能                                                         |
| ------------- | ------------------------------------------------------------ |
| AT+PINMUX_CONFIG | 测试硬件PIN脚的复用功能 |

### 参数列表

以下是PIN模块对应的AT测试命令共用参数的简要介绍。

| 参数名字 | 释义                 | 取值范围                           |
| -------- | -------------------- | ---------------------------------- |
| pin_name | 表示管脚号           | 0、1、...，具体可测编号需参考soc.h |
| func     | 表示管脚复用功能编号 | 0、1、...，具体可测编号需参考soc.h |

### AT+PINMUX_CONFIG

| 命令格式 | AT+PINMUX_CONFIG=pin_name,func |
|------------------------------------------|--------------------------------------------------------------------------------------------------|
| 命令功能 | 测试硬件PIN脚的复用功能 |
| 示例 | AT+PINMUX_CONFIG=0,0 <br/>配置管脚PA0，复用PA0_xxx_xxx功能 |

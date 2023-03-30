## TICK

### 概要

该文档主要描述TICK模块的CSI2.0自动化测试AT命令。 文档中所涉及的这些命令是用来测试TICK模块的接口工作是否正常、数据传输是否正确、接口返回值是否符合预期。



### 测试原理

测试时，应给被测试板发送TICK测试命令进行相应的测试；测试程序会按照按照测试命令的配置进行相应的测试， 若结果与预期一致，则相应的测试成功完成，否则测试失败。



### 测试流程

- 测试前应当准备好测试板（以下简称A板）
- 发送TICK测试命令进行测试 （测试命令功能请参照下文）



### AT命令列表 

以下是TICK模块对应的测试命令列表及其功能简要介绍。

| AT命令列表     | 功能                                                       |
| -------------- | ---------------------------------------------------------- |
| AT+TICK_HANDLE | 测试TICK时间模块中的系统时钟滴答数，以及转换的毫秒、微秒值 |



### AT+TICK_HANDLE

| 命令格式 | AT+TICK_HANDLE |
|------------------------------------------|--------------------------------------------------------------------------------------------------|
| 命令功能 | 测试TICK时间模块中的系统时钟滴答数，以及转换的毫秒、微秒值 |
| 命令参数 | 无 |
| 示例 | 发送命令AT+TICK_HANDLE 到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |

### 
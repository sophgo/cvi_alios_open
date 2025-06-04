## TRNG

### 概要

该文档主要描述TRNG模块的CSI2.0自动化测试AT命令。 文档中所涉及的这些命令是用来测试TRNG模块的接口工作是否正常、接口返回值是否符合预期。



### 测试原理

测试时，应给被测试板发送TRNG测试命令进行相应的测试；测试程序会按照按照测试命令的配置进行相应的生成随机数测试，若生成的随机数的结果与预期一致，则相应的测试成功完成，否则测试失败。



### 测试流程

- 测试前应当准备好测试板（以下简称A板）
- 发送TRNG测试命令进行测试 （测试命令功能请参照下文）



### RSA测试命令列表

| AT命令                      | 测试参数            | 功能                   |
| -------------------------- | ------------------- | ---------------------- |
| AT+TRNG_MULTI              | num_words,loop_nums | 测试生成多个随机数     |
| AT+TRNG_SINGLE             | loop_nums           | 测试生成单个随机数     |
| AT+TRNG_MULTI_PERFORMANCE  | num_words,loop_nums | 测试生成多个随机数性能 |
| AT+TRNG_SINGLE_PERFORMANCE | loop_nums           | 测试生成单个随机数性能 |
| AT+TRNG_SINGLE_STABILITY   | loop_nums           | 测试生成单个随机数可靠性 |
| AT+TRNG_MULTI_STABILITY    | num_words,loop_nums | 测试生成多个随机数可靠性 |



### AT+TRNG_MULTI

| 命令格式 | AT+TRNG_MULTI |
|------------------------------------------|--------------------------------------------------------------------------------------------------|
| 命令功能 | TRNG生成多个随机数模块接口测试 |
| 命令参数 | num_words：每次生成随机数的数量<br/>loop_nums：生成随机数的次数 |
| 示例 | 发送命令AT+TRNG_MULTI到A板,  A板对应串口会打印生成的随机数，测试完成后， A板对应串口会打印OK；否则测试失败 |



### AT+TRNG_SINGLE

| 命令格式 | AT+TRNG_SINGLE                                               |
| -------- | ------------------------------------------------------------ |
| 命令功能 | TRNG生成单个随机数模块接口测试                               |
| 命令参数 | loop_nums：生成随机数的次数                                  |
| 示例     | 发送命令AT+TRNG_SINGLE到A板,  A板对应串口会打印生成的随机数，测试完成后， A板对应串口会打印OK；否则测试失败 |



### AT+TRNG_SINGLE_PERFORMANCE

| 命令格式 | AT+TRNG_SINGLE_PERFORMANCE                                                |
| -------- | ------------------------------------------------------------ |
| 命令功能 | TRNG生成多个随机数模块性能测试                               |
| 命令参数 | loop_nums：生成随机数的次数 |
| 示例     | 发送命令AT+TRNG_SINGLE_PERFORMANCE到A板,  A板对应串口会打印性能值（times/ms），测试完成后， A板对应串口会打印OK；否则测试失败 |



### AT+TRNG_MULTI_PERFORMANCE

| 命令格式 | AT+TRNG_MULTI                                                |
| -------- | ------------------------------------------------------------ |
| 命令功能 | TRNG生成多个随机数模块性能测试                               |
| 命令参数 | num_words：每次生成随机数的数量<br/>loop_nums：生成随机数的次数 |
| 示例     | 发送命令AT+TRNG_MULTI_PERFORMANCE到A板,  A板对应串口会打印性能值（times/ms），测试完成后， A板对应串口会打印OK；否则测试失败 |



### AT+TRNG_SINGLE_STABILITY

| 命令格式 | AT+TRNG_SINGLE_STABILITY                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | TRNG生成单个随机数的稳定性测试                               |
| 命令参数 | loop_nums：生成随机数的次数                                  |
| 示例     | 发送命令AT+TRNG_SINGLE_STABILITY,  A板对应串口会打印性能值（times/ms）以及随机数，测试完成后， A板对应串口会打印OK；否则测试失败 |

### AT+TRNG_MULTI_STABILITY

| 命令格式 | AT+TRNG_MULTI_STABILITY                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | TRNG生成多个随机数模块接口测试                               |
| 命令参数 | num_words：每次生成随机数的数量<br/>loop_nums：生成随机数的次数                                  |
| 示例     | 发送命令AT+TRNG_MULTI_STABILITY,  A板对应串口会打印性能值（times/ms）以及随机数值，测试完成后， A板对应串口会打印OK；否则测试失败 |
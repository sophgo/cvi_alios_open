## EFLASH

### 概要
该文档主要描述CSI2.0自动化测试框架下EFLASH模块的AT测试命令。 文档中所涉及的命令是用来测试EFLASH模块的接口工作是否正常、接口返回值是否符合预期和该模块的特性功能是否正常。

### 测试原理
PC主机用USB转串口线连接测试板（以下简称A板），作为PC同A板数据通信，命令收发的通道；同时PC会使用USB线连接A板的HAD调试器，作为程序下载的通道；通过PC下发AT指令到A板测试程序，当A板有测试成功的信息返回PC后，PC判定本次测试执行通过。


### 测试流程
- 发送EFLASH的AT测试命令进行功能测试

### 命令列表
以下是EFLASH模块对应的AT测试命令列表及其功能简要介绍。
| AT命令列表                       | 功能简介                   |
| -------------------------------- | -------------------------- |
| AT+EFLASH_INTERFACE | API接口的异常参数判断 |
| AT+EFLASH_GET_INFO | 获取eflash存储设备额信息 |
| AT+EFLASH_REPEAT_INIT | 重复init和uninit eflash |
| AT+EFLASH_WRITE_READ | 编程eflash之后进行读取 |
| AT+EFLASH_WRITE_ERASE_READ | 编程、擦除eflash之后进行读取 |
| AT+EFLASH_ERASE_READ | 擦除eflash之后进行读取 |
| AT+EFLASH_CHIP_ERASE | eflash整片擦除 |

### 参数列表
以下是EFLASH模块对应的AT测试命令共用参数的简要介绍
| 参数名字 | 释义 | 取值范围 |
| -------- | ---- | ------- |
| eflash_idx | 选择eflash控制器的idx号 | 0x0 ~ 0xFF,参考对应SOC支持的eflash控制器个数 |
| offset | 指定测试eflash的起始地址 | 0x0 ~ 0xFFFFFFFF,参考eflash的大小 |
| size | 指定测试eflash的空间大小 | 0x0 ~ 0xFFFFFFFF,参考eflash的大小 |
| repeat_times | 指定测试重复执行多少次 | 0x0 ~ 0xFFFFFFFF |
| flash_size | eflash存储空间的大小 | 0x0 ~ 0xFFFFFFFF,参考eflash的大小 |
| sector_size | eflash的sector大小 | 0x0 ~ 0xFFFFFFFF,参考eflash的sector大小 |
| erased_value | eflash被擦后的默认值 | 参考eflash的特性 |


### 功能测试命令详解
#### AT+EFLASH_INTERFACE
|     |     |
| --- | --- |
| 命令格式 | AT+EFLASH_INTERFACE= |
| 命令功能 | API接口的异常参数判断 |
| 可搭配的辅助测试命令 | None |
| 示例 | 步骤1：PC向A板发送AT+EFLASH_INTERFACE= |
| 备注 | 该测试命令不需要跟参数 |

#### AT+EFLASH_GET_INFO
|     |     |
| --- | --- |
| 命令格式 | AT+EFLASH_GET_INFO=eflash_idx,flash_size,sector_size,erased_value,repeat_times |
| 命令功能 | 获取eflash存储设备额信息 |
| 可搭配的辅助测试命令 | None |
| 示例 | 步骤1：PC向A板发送AT+EFLASH_GET_INFO=0,128,32,0xFF,1 |
| 备注 | 把器件的真实信息通过给定flash_size,sector_size,erased_value;获取eflash的存储设备信息同给定的值作比较 |


#### AT+EFLASH_REPEAT_INIT
|     |     |
| --- | --- |
| 命令格式 | AT+EFLASH_REPEAT_INIT=eflash_idx,repeat_times |
| 命令功能 | 重复init和uninit eflash |
| 可搭配的辅助测试命令 | None |
| 示例 | 步骤1：PC向A板发送AT+EFLASH_REPEAT_INIT=0,10 |
| 备注 | 重复执行eflash的初始化和反初始化 |


#### AT+EFLASH_WRITE_READ
|     |     |
| --- | --- |
| 命令格式 | AT+EFLASH_WRITE_READ=eflash_idx,offset,size,repeat_times |
| 命令功能 | 重复init和uninit eflash |
| 可搭配的辅助测试命令 | None |
| 示例 | 步骤1：PC向A板发送AT+EFLASH_WRITE_READ=0,0,100,10 |
| 备注 | 先对指定地址的eflash空间进行编程，再进行读取，最后比较写入的值和读取出来的值是否一致 |


#### AT+EFLASH_WRITE_ERASE_READ
|     |     |
| --- | --- |
| 命令格式 | AT+EFLASH_WRITE_ERASE_READ=eflash_idx,offset,size,repeat_times |
| 命令功能 | 重复init和uninit eflash |
| 可搭配的辅助测试命令 | None |
| 示例 | 步骤1：PC向A板发送AT+EFLASH_WRITE_ERASE_READ=0,0,100,10 |
| 备注 | 先对指定地址的eflash空间进行编程，再进行擦除，最后读取，最后比较读取出来的值是否是擦除后的默认值 |


#### AT+EFLASH_ERASE_READ
|     |     |
| --- | --- |
| 命令格式 | AT+EFLASH_ERASE_READ=eflash_idx,offset,size,repeat_times |
| 命令功能 | 重复init和uninit eflash |
| 可搭配的辅助测试命令 | None |
| 示例 | 步骤1：PC向A板发送AT+EFLASH_ERASE_READ=0,0,100,10 |
| 备注 | 先对指定地址的eflash空间进行擦除，再进行读取，最后比较读取出来的值是否是擦除后的默认值 |


#### AT+EFLASH_CHIP_ERASE
|     |     |
| --- | --- |
| 命令格式 | AT+EFLASH_CHIP_ERASE=eflash_idx,repeat_times |
| 命令功能 | 重复init和uninit eflash |
| 可搭配的辅助测试命令 | None |
| 示例 | 步骤1：PC向A板发送AT+EFLASH_CHIP_ERASE=0,10 |
| 备注 | 对eflash进行片除，再进行读取，最后比较读取出来的值是否是擦除后的默认值 |


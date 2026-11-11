## SHA

### 概要

该文档主要描述SHA模块的CSI2.0自动化测试AT命令。 文档中所涉及的这些命令是用来测试SHA模块的接口工作是否正常、接口返回值是否符合预期。


### 测试准备

- 本测试为单板测试
- 不需要外接跳线
- 测试时，应给被测试板发送AES测试命令进行相应的测试；测试程序会按照按照测试命令的配置进行相应的加密解密测试， 若加密解密的结果与预期一致，则相应的测试成功完成，否则测试失败。


### 测试流程

- 测试前应当准备好测试板（以下简称A板）
- 发送SHA测试命令进行测试 （测试命令功能请参照下文）

### SHA测试命令列表

AT命令 | 测试参数 | 功能 |
|:---|:---|:---|
| AT+SHA_SYNC | idx,sha_mode | SHA同步加密功能测试(单次update) |
| AT+SHA_SYNC_MULTI | idx,sha_mode | SHA同步加密功能测试(多次update) |
| AT+SHA_SYNC_PERFORMANCE    | idx,sha_mode,test_loops | SHA同步加密性能测试 |
| AT+SHA_ASYNC | idx,sha_mode | SHA异步加密功能测试(单次update) |
| AT+SHA_ASYNC_MULTI | idx,sha_mode | SHA异步加密功能测试(多次update) |
| AT+SHA_ASYNC_PERFORMANCE    | idx,sha_mode,test_loops | SHA异步加密性能测试 |


### AT+SHA_SYNC

| 命令格式 | AT+SHA_SYNC=idx,sha_mode |
|:---|:---|
| 命令功能 | SHA同步加密功能测试(单次update)|
| 命令参数 | idx: 设备号<br>sha_mode: sha模式选择 (1->sha1, 2->sha256, 3->sha224, 4->sha512, 5->sha384, 6->sha512_256, 7->sha_512_224) |
| 示例 | 发送命令 AT+SHA_SYNC=0,1 到A板, 设置A板的SHA模式为sha1模式，A板对应串口会打印加密后的密文，并且会打印OK |
| 输出结果 | 测试完成后， A板对应串口会打印OK；否则测试失败 |


### AT+SHA_SYNC_MULTI

| 命令格式 | AT+SHA_SYNC_MULTI=idx,sha_mode |
|:---|:---|
| 命令功能 | SHA同步加密功能测试(多次update)|
| 命令参数 | idx: 设备号<br>sha_mode: sha模式选择 (1->sha1, 2->sha256, 3->sha224, 4->sha512, 5->sha384, 6->sha512_256, 7->sha_512_224) |
| 示例 | 发送命令 AT+SHA_SYNC_MULTI=0,1 到A板, 设置A板的SHA模式为sha1模式，A板对应串口会打印加密后的密文，并且会打印OK |
| 输出结果 | 测试完成后， A板对应串口会打印OK；否则测试失败 |


### AT+SHA_SYNC_PERF

| 命令格式 | AT+SHA_SYNC_PERF=idx,sha_mode,test_loops |
|:---|:---|
| 命令功能 | SHA同步加密性能测试|
| 命令参数 | idx: 设备号<br>sha_mode: sha模式选择 (1->sha1, 2->sha256, 3->sha224, 4->sha512, 5->sha384, 6->sha512_256, 7->sha_512_224)<br>test_loops: 循环次数 |
| 示例 | 发送命令 AT+SHA_SYNC_PERF=0,1 到A板, 设置A板的SHA模式为sha1模式，A板对应串口会打印加密后的密文，并且会打印OK |
| 输出结果 | 测试完成后， A板对应串口会打印OK；否则测试失败 |

### AT+SHA_ASYNC

| 命令格式 | AT+SHA_ASYNC=idx,sha_mode |
|:---|:---|
| 命令功能 | SHA异步加密功能测试(单次update)|
| 命令参数 | idx: 设备号<br>sha_mode: sha模式选择 (1->sha1, 2->sha256, 3->sha224, 4->sha512, 5->sha384, 6->sha512_256, 7->sha_512_224) |
| 示例 | 发送命令 AT+SHA_ASYNC=0,1 到A板, 设置A板的SHA模式为sha1模式，A板对应串口会打印加密后的密文，并且会打印OK |
| 输出结果 | 测试完成后， A板对应串口会打印OK；否则测试失败 |


### AT+SHA_ASYNC_MULTI

| 命令格式 | AT+SHA_ASYNC_MULTI=idx,sha_mode |
|:---|:---|
| 命令功能 | SHA异步加密功能测试(多次update)|
| 命令参数 | idx: 设备号<br>sha_mode: sha模式选择 (1->sha1, 2->sha256, 3->sha224, 4->sha512, 5->sha384, 6->sha512_256, 7->sha_512_224) |
| 示例 | 发送命令 AT+SHA_ASYNC_MULTI=0,1 到A板, 设置A板的SHA模式为sha1模式，A板对应串口会打印加密后的密文，并且会打印OK |
| 输出结果 | 测试完成后， A板对应串口会打印OK；否则测试失败 |


### AT+SHA_ASYNC_PERF

| 命令格式 | AT+SHA_ASYNC_PERF=idx,sha_mode,test_loops |
|:---|:---|
| 命令功能 | SHA异步加密性能测试|
| 命令参数 | idx: 设备号<br>sha_mode: sha模式选择 (1->sha1, 2->sha256, 3->sha224, 4->sha512, 5->sha384, 6->sha512_256, 7->sha_512_224)<br>test_loops: 循环次数 |
| 示例 | 发送命令 AT+SHA_ASYNC_PERF=0,1 到A板, 设置A板的SHA模式为sha1模式，A板对应串口会打印加密后的密文，并且会打印OK |
| 输出结果 | 测试完成后， A板对应串口会打印OK；否则测试失败 |
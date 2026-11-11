## AES

### 概要

该文档主要描述AES模块的CSI2.0自动化测试AT命令。 文档中所涉及的这些命令是用来测试AES模块的接口工作是否正常、接口返回值是否符合预期。

### 测试准备

- 本测试为单板测试
- 不需要外接跳线
- 测试时，应给被测试板发送AES测试命令进行相应的测试；测试程序会按照按照测试命令的配置进行相应的加密解密测试， 若加密解密的结果与预期一致，则相应的测试成功完成，否则测试失败。



### 测试流程

- 测试前应当准备好测试板（以下简称A板）
- 发送AES测试命令进行测试 （测试命令功能请参照下文）



### AT+AES_ECB_ENCRYPT

| 命令格式 | AT+AES_ECB_ENCRYPT=idx,key_len |
|:---|:---|
| 命令功能 | AES加解密模块ecb模式接口测试<br>对明文进行AES算法的ecb模式加密，对应串口会打印加密后的数据 |
| 命令参数 | idx: 控制器号<br>key_len: 加解密key的长度 (16->128bit，24->192bit，32->256bit) |
| 示例 | 发送命令AT+AES_ECB_ENCRYPT=0,16到A板,  A板对应串口会打印加密后的数据 |
| 输出结果 | 测试完成后， A板对应串口会打印OK；否则测试失败 |


### AT+AES_ECB_DECRYPT

| 命令格式 | AT+AES_ECB_DECRYPT=idx,key_len |
|:---|:---|
| 命令功能 | AES加解密模块ecb模式接口测试<br>对密文进行AES算法的ecb模式解密，对应串口会打印解密后的数据 |
| 命令参数 | idx: 控制器号<br>key_len: 加解密key的长度 (16->128bit，24->192bit，32->256bit) |
| 示例 | 发送命令AT+AES_ECB_DECRYPT=0,16到A板,  A板对应串口会打印加密后的数据 |
| 输出结果 | 测试完成后， A板对应串口会打印OK；否则测试失败 |


### AT+AES_CBC_ENCRYPT

| 命令格式 | AT+AES_ECB_ENCRYPT=idx,key_len |
|:---|:---|
| 命令功能 | AES加解密模块cbc模式接口测试<br>对明文进行AES算法的cbc模式加密，对应串口会打印加密后的数据 |
| 命令参数 | idx: 控制器号<br>key_len: 加解密key的长度 (16->128bit，24->192bit，32->256bit) |
| 示例 | 发送命令AT+AES_CBC_ENCRYPT=0,16到A板,  A板对应串口会打印加密后的数据 |
| 输出结果 | 测试完成后， A板对应串口会打印OK；否则测试失败 |


### AT+AES_CBC_DECRYPT

| 命令格式 | AT+AES_CBC_DECRYPT=idx,key_len |
|:---|:---|
| 命令功能 | AES加解密模块cbc模式接口测试<br>对明文进行AES算法的cbc模式解密，对应串口会打印解密后的数据 |
| 命令参数 | idx: 控制器号<br>key_len: 加解密key的长度 (16->128bit，24->192bit，32->256bit) |
| 示例 | 发送命令AT+AES_CBC_DECRYPT=0,16到A板,  A板对应串口会打印加密后的数据 |
| 输出结果 | 测试完成后， A板对应串口会打印OK；否则测试失败 |


### AT+AES_CFB128_ENCRYPT

| 命令格式 | AT+AES_CFB1_ENCRYPT=idx,key_len,num |
|:---|:---|
| 命令功能 | AES加解密模块cfb1模式接口测试<br>对明文进行AES算法的cfb128模式加密，对应串口会打印加密后的数据 |
| 命令参数 | idx: 控制器号<br>key_len: 加解密key的长度 (16->128bit，24->192bit，32->256bit)<br/>num：加密的次数 |
| 示例 | 发送命令AT+AES_CFB1_ENCRYPT=0,16到A板,  A板对应串口会打印加密后的数据 |
| 输出结果 | 测试完成后， A板对应串口会打印OK；否则测试失败 |


### AT+AES_CFB128_DECRYPT

| 命令格式 | AT+AES_CFB128_DECRYPT=idx,key_len,num |
|:---|:---|
| 命令功能 | AES加解密模块cfb128模式接口测试<br>对明文进行AES算法的cfb128模式解密，对应串口会打印解密后的数据 |
| 命令参数 | idx: 控制器号<br>key_len: 加解密key的长度 (16->128bit，24->192bit，32->256bit)<br/>num：加密的次数 |
| 示例 | 发送命令AT+AES_CFB128_DECRYPT=0,16到A板,  A板对应串口会打印解密后的数据 |
| 输出结果 | 测试完成后， A板对应串口会打印OK；否则测试失败 |


### AT+AES_ECB_ENCRYPT_PERF

| 命令格式 | AT+AES_ECB_ENCRYPT_PERF=idx,key_len,loops        |
|:---|:---|
| 命令功能 | AES加解密模块ecb模式接口测试<br>对明文进行AES算法的ecb模式加密，对应串口会打印加密后的数据 |
| 命令参数 | idx：控制器号<br>key_len: 加解密key的长度 (16->128bit，24->192bit，32->256bit)<br/>loop：循环的次数 单位(千次) |
| 示例 | 发送命令AT+AES_ECB_ENCRYPT=0,16,500到A板,  A板对应串口会打印加密后的数据 |
| 输出结果 | 测试完成后， A板对应串口会打印OK；否则测试失败 |


### AT+AES_ECB_DECRYPT_PERF

| 命令格式 | AT+AES_ECB_DECRYPT_PERF=idx,key_len,loops                    |
|:---|:---|
| 命令功能 | AES加解密模块ecb模式接口测试<br>对密文进行AES算法的ecb模式解密，对应串口会打印解密后的数据 |
| 命令参数 | idx：控制器号<br>key_len: 加解密key的长度 (16->128bit，24->192bit，32->256bit)<br/>loop：循环的次数 单位(千次) |
| 示例 | 发送命令AT+AES_ECB_DECRYPT=0,16,500到A板,  A板对应串口会打印加密后的数据 |
| 输出结果 | 测试完成后， A板对应串口会打印OK；否则测试失败 |


### AT+AES_CBC_ENCRYPT_PERF

| 命令格式 | AT+AES_ECB_ENCRYPT_PERF=idx,key_len,loops        |
|:---|:---|
| 命令功能 | AES加解密模块cbc模式接口测试<br>对明文进行AES算法的cbc模式加密，对应串口会打印加密后的数据 |
| 命令参数 | idx：控制器号<br>key_len: 加解密key的长度 (16->128bit，24->192bit，32->256bit)<br/>loop：循环的次数 单位(千次) |
| 示例 | 发送命令AT+AES_CBC_ENCRYPT_PERFORMANCE=0,16,500到A板,  A板对应串口会打印加密后的数据 |
| 输出结果 | 测试完成后， A板对应串口会打印OK；否则测试失败 |


### AT+AES_CBC_DECRYPT_PERF

| 命令格式 | AT+AES_CBC_DECRYPT_PERF=idx,key_len,loops                    |
|:---|:---|
| 命令功能 | AES加解密模块cbc模式接口测试<br>对明文进行AES算法的cbc模式解密，对应串口会打印解密后的数据 |
| 命令参数 | idx：控制器号<br>key_len: 加解密key的长度 (16->128bit，24->192bit，32->256bit)<br/>loop：循环的次数 单位(千次) |
| 示例 | 发送命令AT+AES_CBC_DECRYPT=0,16,500到A板,  A板对应串口会打印加密后的数据 |
| 输出结果 | 测试完成后， A板对应串口会打印OK；否则测试失败 |

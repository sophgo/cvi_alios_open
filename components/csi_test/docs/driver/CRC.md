## AES

### 概要

该文档主要描述CRC模块的CSI2.0自动化测试AT命令。 文档中所涉及的这些命令是用来测试CRC模块的接口工作是否正常、接口返回值是否符合预期。



### 测试原理

测试时，应给被测试板发送CRC测试命令进行相应的测试；测试程序会按照按照测试命令的配置进行相应的测试， 若结果与预期(CRC校验工具产生结果)一致，则相应的测试成功完成，否则测试失败。



### 测试流程

- 测试前应当准备好测试板（以下简称A板）
- 发送CRC测试命令进行测试 （测试命令功能请参照下文）

### 命令列表
以下是CRC模块对应的AT测试命令列表及其功能简要介绍。

| AT命令列表                        | 功能简介                   |
| -------------------------------- | -------------------------- |
| AT+CRC7_BE_FUNC                      | 测试CRC7_BE模式计算                |
| AT+CRC8_FUNC                      | 测试CRC8模式计算                |
| AT+CRC8_MAXIM_FUNC                      | 测试CRC8_MAXIM模式计算               |
| AT+CRC16_FUNC                      | 测试CRC16模式计算                |
| AT+CRC16_CCITT_FUNC                      | 测试CRC16_CCITT模式计算                |
| AT+CRC16_DNP_FUNC                      | 测试CRC16_DNP模式计算                |
| AT+CRC16_ITU_FUNC                      | 测试CRC16_ITU模式计算                |
| AT+CRC32_LE_FUNC                      | 测试CRC32_LE模式计算                |
| AT+CRC32_BE_FUNC                      | 测试CRC32_BE模式计算                |

| AT+CRC7_BE_PERFORMANCE                      | 测试CRC7_BE模式计算性能                |
| AT+CRC8_PERFORMANCE                      | 测试CRC8模式计算性能                |
| AT+CRC8_MAXIM_PERFORMANCE                      | 测试CRC8_MAXIM模式计算性能               |
| AT+CRC16_PERFORMANCE                      | 测试CRC16模式计算性能                |
| AT+CRC16_CCITT_PERFORMANCE                      | 测试CRC16_CCITT模式计算性能                |
| AT+CRC16_DNP_PERFORMANCE                      | 测试CRC16_DNP模式计算性能                |
| AT+CRC16_ITU_PERFORMANCE                      | 测试CRC16_ITU模式计算性能                |
| AT+CRC32_LE_PERFORMANCE                      | 测试CRC32_LE模式计算性能                |
| AT+CRC32_BE_PERFORMANCE                      | 测试CRC32_BE模式计算性能                |

| AT+CRC7_BE_STABILITY                      | 测试CRC7_BE模式计算可靠性                |
| AT+CRC8_STABILITY                      | 测试CRC8模式计算可靠性                |
| AT+CRC8_MAXIM_STABILITY                      | 测试CRC8_MAXIM模式计算可靠性               |
| AT+CRC16_STABILITY                      | 测试CRC16模式计算可靠性                |
| AT+CRC16_CCITT_STABILITY                      | 测试CRC16_CCITT模式计算可靠性                |
| AT+CRC16_DNP_STABILITY                      | 测试CRC16_DNP模式计算可靠性                |
| AT+CRC16_ITU_STABILITY                      | 测试CRC16_ITU模式计算可靠性                |
| AT+CRC32_LE_STABILITY                      | 测试CRC32_LE模式计算可靠性                |
| AT+CRC32_BE_STABILITY                      | 测试CRC32_BE模式计算可靠性                |

### AT+CRC7_BE_FUNC

| 命令格式 | AT+CRC7_BE_FUNC=crc_init_value,crc_data,data_size                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC7_BE功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC_FUNC=0,12,1到A板,  A板会打印计算前的数据和进行csi_crc7_be计算后的结果，并且A板对应串口会打印OK，将计算后的结果与CRC验证软件计算结果相比较，如果结果相同即为测试成功，否则测试失败 |

### AT+CRC8_FUNC

| 命令格式 | AT+CRC8_FUNC=crc_init_value,crc_data,data_size                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC8功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC8_FUNC=0,12,1到A板,  A板会打印计算前的数据和进行csi_crc8计算后的结果，并且A板对应串口会打印OK，将计算后的结果与CRC验证软件计算结果相比较，如果结果相同即为测试成功，否则测试失败 |


### AT+CRC8_MAXIM_FUNC

| 命令格式 | AT+CRC8_MAXIM_FUNC=crc_init_value,crc_data,data_size                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC8_MAXIM功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC8_MAXIM_FUNC=0,12,1到A板,  A板会打印计算前的数据和进行csi_crc8_maxim计算后的结果，并且A板对应串口会打印OK，将计算后的结果与CRC验证软件计算结果相比较，如果结果相同即为测试成功，否则测试失败 |


### AT+CRC16_FUNC

| 命令格式 | AT+CRC16_FUNC=crc_init_value,crc_data,data_size                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC16功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC16_FUNC=0,12,1到A板,  A板会打印计算前的数据和进行csi_crc16计算后的结果16，并且A板对应串口会打印OK，将计算后的结果与CRC验证软件计算结果相比较，如果结果相同即为测试成功，否则测试失败 |


### AT+CRC16_CCITT_FUNC

| 命令格式 | AT+CRC16_CCITT_FUNC=crc_init_value,crc_data,data_size                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC16_CCITT_FUNC=0,12,1到A板,  A板会打印计算前的数据和进行csi_crc16_ccitt计算后的结果，并且A板对应串口会打印OK，将计算后的结果与CRC验证软件计算结果相比较，如果结果相同即为测试成功，否则测试失败 |


### AT+CRC16_DNP_FUNC

| 命令格式 | AT+CRC16_DNP_FUNC=crc_init_value,crc_data,data_size                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC16_DNP功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC16_DNP_FUNC=0,12,1到A板,  A板会打印计算前的数据和进行csi_crc16_dnp计算后的结果16，并且A板对应串口会打印OK，将计算后的结果与CRC验证软件计算结果相比较，如果结果相同即为测试成功，否则测试失败 |


### AT+CRC16_ITU_FUNC

| 命令格式 | AT+CRC16_ITU_FUNC=crc_init_value,crc_data,data_size                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC16_ITU功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC16_ITU_FUNC=0,12,1到A板,  A板会打印计算前的数据和进行csi_crc16_itu计算后的结果16，并且A板对应串口会打印OK，将计算后的结果与CRC验证软件计算结果相比较，如果结果相同即为测试成功，否则测试失败 |


### AT+CRC32_LE_FUNC

| 命令格式 | AT+CRC32_LE_FUNC=crc_init_value,crc_data,data_size                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC32_LE_FUNC=0,12,1到A板,  A板会打印计算前的数据和进行csi_crc32_le计算后的结果，并且A板对应串口会打印OK，将计算后的结果与CRC验证软件计算结果相比较，如果结果相同即为测试成功，否则测试失败 |


### AT+CRC32_BE_FUNC

| 命令格式 | AT+CRC32_BE_FUNC=crc_init_value,crc_data,data_size                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC32_BE功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC32_BE_FUNC=0,12,1到A板,  A板会打印计算前的数据和进行csi_crc32_be计算后的结果，并且A板对应串口会打印OK，将计算后的结果与CRC验证软件计算结果相比较，如果结果相同即为测试成功，否则测试失败 |


### AT+CRC7_BE_PERFORMANCE

| 命令格式 | AT+CRC7_BE_PERFORMANCE=crc_init_value,crc_rand_data,data_size,test_loops                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC7_BE功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC_PERFORMANCE=0,1,1,20到A板,  A板会打印进行csi_crc7_be计算后的性能值（单位：ms/times），并且A板对应串口会打印OK，即为测试成功，否则测试失败 |

### AT+CRC8_PERFORMANCE

| 命令格式 | AT+CRC8_PERFORMANCE=crc_init_value,crc_rand_data,data_size,test_loops                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC8功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC8_PERFORMANCE=0,1,1,20到A板,  A板会打印进行csi_crc8计算后的性能值（单位：ms/times），并且A板对应串口会打印OK，即为测试成功，否则测试失败 |


### AT+CRC8_MAXIM_PERFORMANCE

| 命令格式 | AT+CRC8_MAXIM_PERFORMANCE=crc_init_value,crc_rand_data,data_size,test_loops                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC8_MAXIM功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC8_MAXIM_PERFORMANCE=0,1,1,20到A板,  A板会打印进行csi_crc8_maxim计算后的性能值（单位：ms/times），并且A板对应串口会打印OK，即为测试成功，否则测试失败 |


### AT+CRC16_PERFORMANCE

| 命令格式 | AT+CRC16_PERFORMANCE=crc_init_value,crc_rand_data,data_size,test_loops                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC16功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC16_PERFORMANCE=0,1,1,20到A板,  A板会打印进行csi_crc16计算后的性能值（单位：ms/times）16，并且A板对应串口会打印OK，即为测试成功，否则测试失败 |


### AT+CRC16_CCITT_PERFORMANCE

| 命令格式 | AT+CRC16_CCITT_PERFORMANCE=crc_init_value,crc_rand_data,data_size,test_loops                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC16_CCITT_PERFORMANCE=0,1,1,20到A板,  A板会打印进行csi_crc16_ccitt计算后的性能值（单位：ms/times），并且A板对应串口会打印OK，即为测试成功，否则测试失败 |


### AT+CRC16_DNP_PERFORMANCE

| 命令格式 | AT+CRC16_DNP_PERFORMANCE=crc_init_value,crc_rand_data,data_size,test_loops                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC16_DNP功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC16_DNP_PERFORMANCE=0,1,1,20到A板,  A板会打印进行csi_crc16_dnp计算后的性能值（单位：ms/times）16，并且A板对应串口会打印OK，即为测试成功，否则测试失败 |


### AT+CRC16_ITU_PERFORMANCE

| 命令格式 | AT+CRC16_ITU_PERFORMANCE=crc_init_value,crc_rand_data,data_size,test_loops                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC16_ITU功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC16_ITU_PERFORMANCE=0,1,1,20到A板,  A板会打印进行csi_crc16_itu计算后的性能值（单位：ms/times）16，并且A板对应串口会打印OK，即为测试成功，否则测试失败 |


### AT+CRC32_LE_PERFORMANCE

| 命令格式 | AT+CRC32_LE_PERFORMANCE=crc_init_value,crc_rand_data,data_size,test_loops                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC32_LE_PERFORMANCE=0,1,1,20到A板,  A板会打印进行csi_crc32_le计算后的性能值（单位：ms/times），并且A板对应串口会打印OK，即为测试成功，否则测试失败 |


### AT+CRC32_BE_PERFORMANCE

| 命令格式 | AT+CRC32_BE_PERFORMANCE=crc_init_value,crc_rand_data,data_size,test_loops                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC32_BE功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC32_BE_PERFORMANCE=0,1,1,20到A板,  A板会打印进行csi_crc32_be计算后的性能值（单位：ms/times），并且A板对应串口会打印OK，即为测试成功，否则测试失败 |


### AT+CRC7_BE_STABILITY

| 命令格式 | AT+CRC7_BE_STABILITY=crc_init_value,crc_rand_data,data_size,test_loops                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC7_BE功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC_STABILITY=0,1,1,20到A板,  A板会打印进行csi_crc7_be计算后的结果，将计算结果与CRC软件验证结果进行比较，并且A板对应串口会打印OK，如果结果相同，即为测试成功，否则测试失败 |

### AT+CRC8_STABILITY

| 命令格式 | AT+CRC8_STABILITY=crc_init_value,crc_rand_data,data_size,test_loops                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC8功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC8_STABILITY=0,1,1,20到A板,  A板会打印进行csi_crc8计算后的结果，将计算结果与CRC软件验证结果进行比较，并且A板对应串口会打印OK，如果结果相同，即为测试成功，否则测试失败 |


### AT+CRC8_MAXIM_STABILITY

| 命令格式 | AT+CRC8_MAXIM_STABILITY=crc_init_value,crc_rand_data,data_size,test_loops                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC8_MAXIM功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC8_MAXIM_STABILITY=0,1,1,20到A板,  A板会打印进行csi_crc8_maxim计算后的结果，将计算结果与CRC软件验证结果进行比较，并且A板对应串口会打印OK，如果结果相同，即为测试成功，否则测试失败 |


### AT+CRC16_STABILITY

| 命令格式 | AT+CRC16_STABILITY=crc_init_value,crc_rand_data,data_size,test_loops                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC16功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC16_STABILITY=0,1,1,20到A板,  A板会打印进行csi_crc16计算后的结果，将计算结果与CRC软件验证结果进行比较16，并且A板对应串口会打印OK，如果结果相同，即为测试成功，否则测试失败 |


### AT+CRC16_CCITT_STABILITY

| 命令格式 | AT+CRC16_CCITT_STABILITY=crc_init_value,crc_rand_data,data_size,test_loops                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC16_CCITT_STABILITY=0,1,1,20到A板,  A板会打印进行csi_crc16_ccitt计算后的结果，将计算结果与CRC软件验证结果进行比较，并且A板对应串口会打印OK，如果结果相同，即为测试成功，否则测试失败 |


### AT+CRC16_DNP_STABILITY

| 命令格式 | AT+CRC16_DNP_STABILITY=crc_init_value,crc_rand_data,data_size,test_loops                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC16_DNP功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC16_DNP_STABILITY=0,1,1,20到A板,  A板会打印进行csi_crc16_dnp计算后的结果，将计算结果与CRC软件验证结果进行比较16，并且A板对应串口会打印OK，如果结果相同，即为测试成功，否则测试失败 |


### AT+CRC16_ITU_STABILITY

| 命令格式 | AT+CRC16_ITU_STABILITY=crc_init_value,crc_rand_data,data_size,test_loops                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC16_ITU功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC16_ITU_STABILITY=0,1,1,20到A板,  A板会打印进行csi_crc16_itu计算后的结果，将计算结果与CRC软件验证结果进行比较16，并且A板对应串口会打印OK，如果结果相同，即为测试成功，否则测试失败 |


### AT+CRC32_LE_STABILITY

| 命令格式 | AT+CRC32_LE_STABILITY=crc_init_value,crc_rand_data,data_size,test_loops                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC32_LE_STABILITY=0,1,1,20到A板,  A板会打印进行csi_crc32_le计算后的结果，将计算结果与CRC软件验证结果进行比较，并且A板对应串口会打印OK，如果结果相同，即为测试成功，否则测试失败 |


### AT+CRC32_BE_STABILITY

| 命令格式 | AT+CRC32_BE_STABILITY=crc_init_value,crc_rand_data,data_size,test_loops                                   |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试CRC32_BE功能                              |
| 命令参数 | crc_init_value：CRC初始值<br/>crc_data:需要计算的数据<br/>data_size：计算数据长度 |
| 示例     | 发送命令AT+CRC32_BE_STABILITY=0,1,1,20到A板,  A板会打印进行csi_crc32_be计算后的结果，将计算结果与CRC软件验证结果进行比较，并且A板对应串口会打印OK，如果结果相同，即为测试成功，否则测试失败 |





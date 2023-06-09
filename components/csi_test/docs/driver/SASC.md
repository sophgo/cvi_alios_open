## SASC
### 概要
该文档主要描述CSI2.0自动化测试框架下SASC模块的AT测试命令。 文档中所涉及的命令是用来测试SASC模块的接口工作是否正常、接口返回值是否符合预期和该模块的特性功能是否正常。
### 测试原理
PC主机用USB转串口线连接测试板（以下简称A板），作为PC同A板进行数据通信和命令收发的通道；同时PC会使用USB线连接A板的HAD调试器，作为程序下载的通道；通过PC下发AT指令到A板测试程序，当A板有测试成功的信息返回PC后，PC判定本次测试执行通过。

### 测试流程
- 发送SASC的AT测试命令进行功能测试

### 命令列表
以下是SASC模块对应的AT测试命令列表及其功能简要介绍。

| AT命令列表                       | 功能简介                   |
| -------------------------------- | -------------------------- |
| AT+SASC_RAM_CONFIG | 测试CPU处于安全态和非安全态时，对SASC控制RAM区域的配置|
| AT+SASC_RAM_SS  | 测试CPU处于安全态和非安全态时对RAM区配置为安全属性的访问|
| AT+SASC_RAM_US | 测试CPU处于安全态和非安全态时对RAM区配置为非安全属性的访问 |
| AT+SASC_RAM | 测试当SASC的8个区域配置为不同的权限，CPU处于安全态和非安全态时对RAM的访问|
| AT+SASC_FLASH_CONFIG|测试CPU处于安全态和非安全态时，对SASC控制FLASH区域的配置|
| AT+SASC_FLASH_SS|测试CPU处于安全态和非安全态时对FLASH区配置为安全属性的访问|
|AT+SASC_FLASH_US|测试CPU处于安全态和非安全态时对FLASH区配置为非安全属性的访问|
|AT+SASC_FLASH|测试当SASC的8个区域配置为不同的权限，CPU处于安全态和非安全态时对FLASH的访问|


### 参数列表
以下是SASC模块对应的AT测试命令共用参数的简要介绍。

| 参数名字      | 释义                                              | 取值范围                                                     |
| ------------- | ------------------------------------------------- | ------------------------------------------------------------ |
| region_idx           | 表示使用那个region | 根据SASC实现的可控制的region而定                      |
| base_addr | 配置region的基地址 | 0x0 - 0xFFFFFFFF（需参考可控制的存储在总线上的划分） |
| ram_size | 指定控制的RAM的大小 | 5：4 Bytes<br>6:  8  Bytes<br>7:  16  Bytes<br>8:  32  Bytes<br/>9:  64  Bytes<br/>10:  128  Bytes<br/>11:  256  Bytes<br/>12:  512  Bytes<br/>13:  1K  Bytes<br/>14:  2K  Bytes<br/>15:  4K  Bytes<br/>16:  8K  Bytes<br/>17:  16K  Bytes<br/>18:  32K  Bytes<br/>19:  64K  Bytes<br/>20:  128K  Bytes<br/>Tips: 实际可使用的值需要参考硬件提供的RAM大小 |
| flash_size | 指定控制的FLASH的大小 | 0：1 sector<br>1：2 sectors<br/>2：4 sectors<br/>3：8 sectors<br/>4：16 sectors<br/>5：32 sectors<br/>6：64 sectors<br/>7：128 sectors<br/>8：256 sectors<br/>9：512 sectors<br/>10：1024 sectors<br/>11：2048 sectors<br/>Tips: 实际可使用的值需要参考硬件提供的FLASH大小 |


### 命令详解
#### AT+SASC_RAM_CONFIG
| 命令格式 | AT+SASC_RAM_CONFIG=region_idx,base_addr,ram_size |
|----------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| 命令功能 | 测试当CPU处于安全特权态时有权限配置指定大小的RAM段由SASC控制；当CPU处于非安全特权态时没有权限配置 |
| 搭配的辅助测试命令 | 无 |
| 示例 | 步骤1:向A板发送下列命令<br>    AT+SASC_RAM_CONFIG=0,0xFFE0000,5 |
| 备注 | RAM段由region_idx,base_addr,ram_size这三个参数的组合结果来指定，同时base_addr指定的基地址应该与指定的ram_size大小保证地址对齐 |

#### AT+SASC_RAM_SS
| 命令格式 | AT+SASC_RAM_SS=region_idx,base_addr,ram_size |
|----------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| 命令功能 | 测试当CPU处于安全特权态时有权限访问安全属性的RAM区，当CPU处于非安全特权态时没有权限访问 |
| 可搭配的辅助测试命令 | 无 |
| 示例 | 步骤1:向A板发送下列命令<br/>    AT+SASC_RAM_SS=0,0xFFE0000,6 |
| 备注 | RAM段由region_idx,base_addr,ram_size这三个参数的组合结果来指定，同时base_addr指定的基地址应该与指定的ram_size大小保证地址对齐 |

#### AT+SASC_RAM_US
| 命令格式 | AT+SASC_RAM_US=region_idx,base_addr,ram_size |
|----------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| 命令功能 | 测试当CPU处于安全特权态时有权限访问非安全属性的RAM区，当CPU处于非安全特权态时也有权限访问 |
| 可搭配的辅助测试命令 | 无 |
| 示例 | 步骤1:向A板发送下列命令<br/>    AT+SASC_RAM_US=0,0xFFE0000,6 |
| 备注 | RAM段由region_idx,base_addr,ram_size这三个参数的组合结果来指定，同时base_addr指定的基地址应该与指定的ram_size大小保证地址对齐 |

####  AT+SASC_RAM
| 命令格式 | AT+SASC_RAM=ram_size |
|----------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| 命令功能 | 测试当CPU处于安全特权态时有权限访问配置不同权限的8个RAM区，当CPU处于非安全特权态时根据设定的权限访问 |
| 可搭配的辅助测试命令 | 无 |
| 示例 | 步骤1:向A板发送下列命令<br/>    AT+SASC_RAM=6 |
| 备注 | ram_size指定测试程序配置的每个region的大小 |

####  AT+SASC_FLASH_CONFIG
| 命令格式 | AT+SASC_FLASH_CONFIG=region_idx,base_addr,flash_size |
|----------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| 命令功能 | 测试当CPU处于安全特权态时有权限配置指定大小的FLASH段由SASC控制；当CPU处于非安全特权态时没有权限配置 |
| 可搭配的辅助测试命令 | 无 |
| 示例 | 步骤1:向A板发送下列命令<br/>    AT+SASC_FLASH_CONFIG=0,0x8000000,0 |
| 备注 | FLASH段由region_idx,base_addr,flash_size这三个参数的组合结果来指定，同时base_addr指定的基地址应该与指定的flash_size大小保证地址对齐 |

####  AT+SASC_FLASH_SS
| 命令格式 | AT+SASC_FLASH_SS=region_idx,base_addr,flash_size |
|----------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| 命令功能 | 测试当CPU处于安全特权态时有权限访问安全属性的FLASH区，当CPU处于非安全特权态时没有权限访问 |
| 可搭配的辅助测试命令 | 无 |
| 示例 | 步骤1:向A板发送下列命令<br/>    AT+SASC_FLASH_SS=0,0x8000000,1 |
| 备注 | FLASH段由region_idx,base_addr,flash_size这三个参数的组合结果来指定，同时base_addr指定的基地址应该与指定的flash_size大小保证地址对齐 |

#### AT+SASC_FLASH_US
| 命令格式 | AT+SASC_FLASH_US=region_idx,base_addr,flash_size |
|----------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| 命令功能 | 测试当CPU处于安全特权态时有权限访问非安全属性的FLASH区，当CPU处于非安全特权态时也有权限访问 |
| 可搭配的辅助测试命令 | 无 |
| 示例 | 步骤1:向A板发送下列命令<br/>    AT+SASC_RAM_US=0,0x8000000,3 |
| 备注 | FLASH段由region_idx,base_addr,flash_size这三个参数的组合结果来指定，同时base_addr指定的基地址应该与指定的flash_size大小保证地址对齐 |

#### AT+SASC_FLASH
| 命令格式 | AT+SASC_FLASH=flash_size |
|----------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| 命令功能 | 测试当CPU处于安全特权态时有权限访问配置不同权限的8个FLASH区，当CPU处于非安全特权态时根据设定的权限访问 |
| 可搭配的辅助测试命令 | 无 |
| 示例 | 步骤1:向A板发送下列命令<br/>    AT+SASC_FLASH=0 |
| 备注 | flash_size指定测试程序配置的每个region的大小 |
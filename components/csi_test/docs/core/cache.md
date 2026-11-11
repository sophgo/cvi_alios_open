## CACHE
### 概要
cache存放的是对cache的测试,测试dcache时,需要堆的大小有10KB以上, cache的case建议手动执行,不然很有可能影响其它用例的执行。

### 测试流程
测试时，应给被测试板发送CACHE测试命令进行相应的测试；测试程序会按照按照测试命令的配置进行相应的测试， 若结果与预期一致，则相应的测试成功完成，否则测试失败。



### AT命令列表
| AT命令列表    | 功能                                                |
| ------------- | --------------------------------------------------- |
| AT+CORE_DCACHE_ON_OFF   | 测试dcache重复开关 |
| AT+CORE_ICACHE_ON_OFF   | 测试icache重复开关 |
| AT+CORE_DCACHE_INVALID   | 测试dcache的invalid功能 |
| AT+CORE_ICACHE_TICKS   | 测试icache打开或关闭时执行相同程序所使用的时间 |
| AT+CORE_DCACHE_TICKS   | 测试dcache打开或关闭时执行相同程序所使用的时间 |


### CK架构的cache测试命令详解
#### 参数列表
以下是CK架构的cache模块对应的AT测试命令共用参数的简要介绍。

| 参数名字     | 释义          | 取值范围      |
| ------------ | ------------ | ------------- |
| cache_index | 设置cache 控制块的idx | 0x2-0xFF, 0和1一般都会被SDK包的软件默认使用，具体可用的最大标号块需参考CACHE手册 |
| size_index | 设置cache 控制的大小 | 0-20, 需要根据内存的地址空间选择大小 |
| repeat_times | 设置测试用例重复运行的次数 | 0-0xFFFFFFFF |
| timer_idx | 设置使用的timer idx | 0x0-0xFF, 具体可使用的timer需参看SOC手册 |


| cache_index | 大小 |
| --- | --- |
| 0 | 4KB |
| 1 | 8KB |
| 2 | 16KB |
| 3 | 32KB |
| 4 | 64KB |
| 5 | 128KB |
| 6 | 256KB |
| 7 | 512KB |
| 8 | 1MB |
| 9 | 2MB |
| 10 | 4MB |
| 11 | 8MB |
| 12 | 16MB |
| 13 | 32MB |
| 14 | 64MB |
| 15 | 128MB |
| 16 | 256MB |
| 17 | 512MB |
| 18 | 1GB |
| 19 | 2GB |
| 20 | 4GB |

#### AT+CORE_DCACHE_ON_OFF
| 命令格式 | AT+CORE_DCACHE_ON_OFF=cache_index,size_index,repeat_times      |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试dcache重复开关                                        |
| 示例     | 发送命令AT+CORE_DCACHE_ON_OFF=2,2,1000到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |

#### AT+CORE_ICACHE_ON_OFF
| 命令格式 | AT+CORE_ICACHE_ON_OFF=cache_index,size_index,repeat_times      |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试icache重复开关                                        |
| 示例     | 发送命令AT+CORE_ICACHE_ON_OFF=2,2,1000到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |

#### AT+CORE_DCACHE_INVALID
| 命令格式 | AT+CORE_DCACHE_INVALID=cache_index,size_index      |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试dcache的invalid功能                                        |
| 示例     | 发送命令AT+CORE_DCACHE_INVALID=2,2到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |

#### AT+CORE_ICACHE_TICKS
| 命令格式 | AT+CORE_ICACHE_TICKS=cache_index,size_index,timer_idx      |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试icache打开或关闭时执行相同程序所使用的时间 |
| 示例     | 发送命令AT+CORE_ICACHE_TICKS=2,2,1到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |

#### AT+CORE_DCACHE_TICKS
| 命令格式 | AT+CORE_DCACHE_TICKS=cache_index,size_index,timer_idx      |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试dcache打开或关闭时执行相同程序所使用的时间  |
| 示例     | 发送命令AT+CORE_DCACHE_TICKS=2,2,1到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |



### RV架构的cache测试命令详解
#### 参数列表
以下是RV架构的cache模块对应的AT测试命令共用参数的简要介绍。

| 参数名字     | 释义          | 取值范围      |
| ------------ | ------------ | ------------- |
 repeat_times | 设置测试用例重复运行的次数 | 0-0xFFFFFFFF |
| timer_idx | 设置使用的timer idx | 0x0-0xFF, 具体可使用的timer需参看SOC手册 |


#### AT+CORE_DCACHE_ON_OFF
| 命令格式 | AT+CORE_DCACHE_ON_OFF=repeat_times      |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试dcache重复开关                                        |
| 示例     | 发送命令AT+CORE_DCACHE_ON_OFF=1000到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |

#### AT+CORE_ICACHE_ON_OFF
| 命令格式 | AT+CORE_ICACHE_ON_OFF=repeat_times      |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试icache重复开关                                        |
| 示例     | 发送命令AT+CORE_ICACHE_ON_OFF=1000到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |

#### AT+CORE_DCACHE_INVALID
| 命令格式 | AT+CORE_DCACHE_INVALID=      |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试dcache的invalid功能                                        |
| 示例     | 发送命令AT+CORE_DCACHE_INVALID到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |

#### AT+CORE_ICACHE_TICKS
| 命令格式 | AT+CORE_ICACHE_TICKS=timer_idx      |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试icache打开或关闭时执行相同程序所使用的时间 |
| 示例     | 发送命令AT+CORE_ICACHE_TICKS=1到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |

#### AT+CORE_DCACHE_TICKS
| 命令格式 | AT+CORE_DCACHE_TICKS=timer_idx      |
| -------- | ------------------------------------------------------------ |
| 命令功能 | 测试dcache打开或关闭时执行相同程序所使用的时间  |
| 示例     | 发送命令AT+CORE_DCACHE_TICKS=1到A板，测试完成后， A板对应串口会打印OK；否则测试失败 |
## JUPITER PMU模块

### 概要

该文档主要描述JUPITER PMU模块不同功能测试case的操作方法。 

### 测试原理

测试时，给测试板发送相关测试命令进行相应的测试；测试程序会按照按照测试命令进行相应的测试，并结合打印及现象，若结果与预期一致，则相应的测试成功完成，否则测试失败。



### 测试流程

- 测试前应当准备好一块JUPITER的测试板，供上5V的电源
- 将JUPITER的串口连接到电脑上（备注：1.测试时，大核串口连接的引脚为PA9和PA10，小核串口连接的引脚为PA0和PA1；2.串口线的绿线接PA1和PA10，白线接PA0和PA9,如果无法打印，请确认代码里的默认串口是否一致）
- 将JUPITER的DebugServer线连接到电脑上（备注：1.CKLink Lite的排线需要与JUPITER的插孔反插）
- 开始测试



### AT命令列表 

以下是JUPITER模块对应的测试命令列表及其功能简要介绍。

| AT命令列表                                    | 功能描述                                                                                |
| ---------------------------------------------| --------------------------------------------------------------------------------------- |
| AT+JUPITER_IO_DT_CHECK                       | IO引脚时钟观测功能测试                                                                    |
| AT+JUPITER_IO_LATCH_GPIO_INPUT_READ_CHECK         | PMU配置IO latch,进入低功耗模式前配置GPIO为输入，测试io latch锁定或者释放，是否可以正常读外接电平值                                        |
| AT+JUPITER_IO_LATCH_GPIO_INPUT_WRITE_CHECK         | PMU配置IO latch,进入低功耗模式前配置GPIO为输入，测试io latch锁定或者释放，是否可以输出高低电平                                        |
| AT+JUPITER_IO_LATCH_QSPI_READ_PROGRAM_CHECK  | PMU配置IO latch,测试QSPI方向锁定功能                                                      |
| AT+JUPITER_MEXSTATUS_CONTROL                 | 测试多核SOC设置mexstatus寄存器复位某个cpu对其它cpu无影响                                    |
| AT+JUPITER_RESET_MODE_TEST_BKSRAM_DATA       | 测试不同复位方式下，BK_SRAM区域数据保存情况                                                 |
| AT+JUPITER_RETENTION_SWITCH_TEST             | 测试低功耗模式下，MAIN_PWR_DCDC、MAIN_PWR_LDO、RF_PWR_DCDC、RF_PWR_LDO电源供电切换功能      |
| AT+JUPITER_SET_SRAM_DELAY                    | 测试JUPITER sram delay设置功能                                                            |
| AT+JUPITER_RUN_COREMARK                      | 使JUPITER执行跑分功能                                                                     |



### JUPITER_IO_DT_CHECK
| 命令格式  | AT+JUPITER_IO_DT_CHECK = pin |
|--------- |-----------------------------------------------------------------------------------------------------------------------------------------------------------|
| 命令功能  | IO引脚时钟观测功能测试|
| 可搭配的辅助测试命令 | 无                                                           |
| 命令参数  | pin:测试引脚<br/> |
| 示例     | 步骤1:将逻辑分析仪连接到JUPITER开发板上的测试引脚PA10上<br/>步骤2：打印BOOTIND后发送AT+IO引脚时钟观测功能测试=10<br/>步骤3：用逻辑分析仪抓取信号<br/> 步骤4：能抓取到该IO引脚上有稳定的时钟输出，即为测试成功，否则测试失败 |
| 备注     |参考Hobbit4_Jupiter Userguide文档的IOCTL章节，选择测试具有时钟观测功能的引脚


### JUPITER_IO_LATCH_GPIO_INPUT_READ_CHECK
| 命令格式 | AT+JUPITER_IO_LATCH_GPIO_INPUT_READ_CHECK = wakeup_pin,wakeup_pin_func,gpio_idx,pin,gpio_mode,bc_io_latch,lc_io_latch |
|--------- |-----------------------------------------------------------------------------------------------------------------------------------------------------------|READ_
| 命令功能 | PMU配置IO latch,进入低功耗模式前配置GPIO为输入,测试在IO latch不释放情况下无法改变GPIO的方向，释放IO latch后可以改变GPIO的方向|
| 可搭配的辅助测试命令 | 无                                                           |
| 命令参数 | wakeup_pin：低功耗唤醒引脚<br/>wakeup_pin_func：引脚唤醒功能<br/>gpio_idx：gpio设备号<br/>pin：GPIO测试引脚<br/>gpio_mode:GPIO模式<br/>bc_io_latch:开启大核的io latch功能<br/>lc_io_latch：开启小核的io latch功能<br/> |
| 示例     | 步骤1:将AT命令里设置的低功耗的唤醒引脚和大小核的测试引脚引出，将唤醒引脚接在低电平上<br/>步骤2：分别加载elf文件到大核和小核上，向大核发送AT+JUPITER_IO_LATCH_GPIO_INPUT_READ_CHECK=4,4,0,5,0,1,0，再先向小核发送AT+PINMUX_CONFIG=0,16,3,然后向小核发送AT+JUPITER_IO_LATCH_GPIO_INPUT_CHECK=4,4,0,5,0,1,0，JUPITER即进入低功耗<br/>步骤3：检查JUPITER是否进入低功耗模式（此时DebugServer自动断开，操作者手动连接一下，如果不能连接即为进入低功耗模式）<br/> 步骤4：将唤醒引脚由低电平接到高电平上，串口有打印出现即为唤醒成功<br/>步骤5：串口在打印“testing low”提示时,需要将大核和小核的测试引脚PA5和PA15接到低电平上，如果GPIO读取电平成功，紧接着会打印“testing high”提示，操作者需要将引脚PA5和PA15接到高电平上，紧接着又会打印“testing low”提示，操作者需要将引脚PA5和PA15接到低电平上，然后又会打印“testing high”提示，操作者需要将引脚PA5和PA15接到高电平上<br/>步骤5：测试完成时，大核和小核的窗口都打印OK即为测试成功，否则测试失败 |


### JUPITER_IO_LATCH_GPIO_INPUT_WRITE_CHECK
| 命令格式 | AT+JUPITER_IO_LATCH_GPIO_INPUT_WRITE_CHECK = wakeup_pin,wakeup_pin_func,gpio_idx,pin,gpio_mode |
|--------- |-----------------------------------------------------------------------------------------------------------------------------------------------------------|
| 命令功能 | PMU配置IO latch,进入低功耗模式前配置GPIO为输入,测试在IO latch不释放情况下无法改变GPIO的方向，释放IO latch后可以改变GPIO的方向|
| 可搭配的辅助测试命令 | 无                                                           |
| 命令参数 | wakeup_pin：低功耗唤醒引脚<br/>wakeup_pin_func：引脚唤醒功能<br/>gpio_idx：gpio设备号<br/>pin：GPIO测试引脚<br/>gpio_mode<br/>GPIO模式<br/> |
| 示例     | 步骤1:将AT命令里设置的低功耗的唤醒引脚和大小核的测试引脚引出，将唤醒引脚接在低电平上，将大小核的测试引脚接到万用表，在测试过程中用万用表量取电压<br/>步骤2：分别加载elf文件到大核和小核上，向大核发送AT+JUPITER_IO_LATCH_GPIO_INPUT_WRITE_CHECK=4,4,0,5,0,1,1，再先向小核发送AT+PINMUX_CONFIG=0,16,3,然后向小核发送AT+JUPITER_IO_LATCH_GPIO_INPUT_WRITE_CHECK=4,4,0,5,0,1,1，JUPITER即进入低功耗<br/>步骤3：检查JUPITER是否进入低功耗模式（此时DebugServer自动断开，操作者手动连接一下，如果不能连接即为进入低功耗模式）<br/> 步骤4：将唤醒引脚由低电平接到高电平上，此时暂时不会有打印，需要立即用万用表测量PA5和PA15的电平值<br/>步骤5：在打印sscom出现打印之前，万用表上的数值始终为低，不能有一次出现高即为IO latch未释放下GPIO方向锁定功能测试成功，否则失败，<br/>步骤5：当sscom出现“Program wakes up”提示后，紧接着会打印“testing high”或者“testing low”字样，此时万用表的显示需要对应上打印提示，当打印“testing high”时，万用表显示为高电平，打印“testing low”时，万用表显示为低，即为IO latch释放后，在IO latch未释放情况下配置的GPIO方向发挥作用功能测试成功，否则测试失败<br/>测试完成时，大核和小核的窗口都打印OK即为测试成功，否则测试失败 |
| 备注   |此条case为手动测试，不能纯粹根据最后打印的OK判断该case测试成功


### JUPITER_MEXSTATUS_CONTROL
| 命令格式  | AT+JUPITER_MEXSTATUS_CONTROL = setting |
|--------- |-----------------------------------------------------------------------------------------------------------------------------------------------------------|
| 命令功能  | 测试多核SOC设置mexstatus寄存器复位某个cpu对其它cpu无影响|
| 可搭配的辅助测试命令 | 无                                                           |
| 命令参数  | setting:是否设置cpu核复位 |
| 示例     | 步骤1:分别加载elf文件到大核和小核上<br/>步骤2：向大核发送AT+JUPITER_MEXSTATUS_CONTROL=1,向小核发送AT+JUPITER_MEXSTATUS_CONTROL=0<br/>步骤3：分别观测大核和小核现象，如果大核复位，小核打印OK即为测试成功，否则测试失败<br/> |
| 备注     |分别设置大核复位，小核不复位和小核复位，大核不复位情况


### JUPITER_RESET_MODE_TEST_BKSRAM_DATA
| 命令格式  | AT+JUPITER_RESET_MODE_TEST_BKSRAM_DATA = reset_mode,bksram_address,bksram_size,test_size,random_num |
|--------- |-----------------------------------------------------------------------------------------------------------------------------------------------------------|
| 命令功能  | IO引脚时钟观测功能测试|
| 可搭配的辅助测试命令 | 无                                                           |
| 命令参数  | reset_mode:复位模式(1为POR复位，2为PAD复位，3为WDT0复位，4为WDT1复位，5为CPU核复位)<br/>bksram_address：操作的BK_SRAM地址<br/>bksram_size：操作的BK_SRAM区域大小<br/>test_size：测试的内存大小<br/>random_num：随机测试内存区域个数<br/> |
| 示例     | 步骤1：分别加载elf文件到大核上，大核发送AT+JUPITER_RESET_MODE_TEST_BKSRAM_DATA=3,0x20018000,64,64,1<br/>步骤2：等待指定方式复位<br/>步骤3：不能使程序跑起来，重新连接DebugServer（.gdbinit文件什么都不要添加）,使用x /10wx 0x20018000查看操作地址的值，如果和写值相同即为测试成功  |
| 备注     |JUPITER的BK_SRAM区域地址范围为：0x2001_8000~0x2001_FFFF，大小为32kB


### JUPITER_RETENTION_SWITCH_TEST
| 命令格式  | AT+JUPITER_RETENTION_SWITCH_TEST = wakeup_pin,wakeup_pin_func,main_setting,rf_setting,dcdc_or_ldo,value |
|--------- |-----------------------------------------------------------------------------------------------------------------------------------------------------------|
| 命令功能  | IO引脚时钟观测功能测试|
| 可搭配的辅助测试命令 | 无                                                           |
| 命令参数  | pin:测试引脚<br/> |
| 示例     | 步骤1:将AT命令里设置的低功耗的唤醒引脚引出，将唤醒引脚接在低电平上<br/>步骤2：分别加载elf文件到大核和小核上，大核和小核分别发送AT+JUPITER_RETENTION_SWITCH_TEST=4,4,1,0,1,0x9<br/>步骤2：检查JUPITER是否进入低功耗模式（此时DebugServer自动断开，操作者手动连接一下，如果不能连接即为进入低功耗模式）<br/>步骤3：将唤醒引脚由低电平接到高电平上，串口有打印出现即为唤醒成功<br/>步骤4：测试完成时，大核和小核的窗口都打印OK即为测试成功，否则测试失败<br/> |
| 备注     |


### JUPITER_SET_SRAM_DELAY
| 命令格式  | AT+JUPITER_SET_SRAM_DELAY = setting_cycle |
|--------- |-----------------------------------------------------------------------------------------------------------------------------------------------------------|
| 命令功能  | 测试JUPITER sram delay设置功能|
| 可搭配的辅助测试命令 | AT+JUPITER_RUN_COREMARK = setting_cpu_freq,setting_cache                                                           |
| 命令参数  | setting_cycle:设置sram_delay个数<br/> |
| 示例     | 步骤1:小核发送AT+JUPITER_SET_SRAM_DELAY=0，设置成功以后再操作大核<br/>步骤2：大核发送AT+JUPITER_RUN_COREMARK=30000000,0<br/>步骤3：记录配置的 RAM delay值和对应coremark的结果<br/> 步骤4：遍历icache/dcache开启或关闭情况下，设置cpu为有效最大频率，记录在不同sram_delay值下coremark的结果 |
| 备注     |(1)关cache条件下，100M~200M的CPU频率,cycle delay支持1，2，3 cycle_delay；200M~300M的CPU频率,cycle delay支持2，3cycle_delay；300M~400M的CPU频率,cycle delay支持0，1，2cycle_delay(2)开cache条件下，300M的CPU频率只支持0，2cycle_delay(3)jupiter de CPU最小频率跑到18M


### JUPITER_RUN_COREMARK
| 命令格式  | AT+JUPITER_RUN_COREMARK = setting_cpu_freq,setting_cache |
|--------- |-----------------------------------------------------------------------------------------------------------------------------------------------------------|
| 命令功能  | 使JUPITER执行跑分功能|
| 可搭配的辅助测试命令 | 无                                                           |
| 命令参数  | setting_cpu_freq:设置CPU测试频率<br/>setting_cache:开启或者关闭cache<br/> |
| 示例     | 步骤1:小核发送AT+JUPITER_SET_SRAM_DELAY=0，设置成功以后再操作大核<br/>步骤2：大核发送AT+JUPITER_RUN_COREMARK=30000000,0<br/>步骤3：记录配置的 RAM delay值和对应coremark的结果<br/> 步骤4：遍历icache/dcache开启或关闭情况下，设置cpu为有效最大频率，记录在不同sram_delay值下coremark的结果 |
| 备注     |(1)关cache条件下，100M~200M的CPU频率,cycle delay支持1，2，3 cycle_delay；200M~300M的CPU频率,cycle delay支持2，3cycle_delay；300M~400M的CPU频率,cycle delay支持0，1，2cycle_delay(2)开cache条件下，300M的CPU频率只支持0，2cycle_delay(3)jupiter de CPU最小频率跑到18M





## cache stress user guide
### 简介
cache压测用例集成了coremark、dhrystone整形运算，performance浮点运算，iic读写eeprom和DMA搬运内存数据。

### 压测环境搭建
1. 获取被测板子相应的带有测试用例的SDK包
2. 被测板子外接IIC接口的eeprom
3. 接上被测板子的使用的打印串口
4. 被测板子与debug ICE盒子连接，ICE盒子与PC连接，PC端运行debugsever并连接到被测板子的CPU

### 压测用例构建
1. 进入csi-test/test_tools目录下,执行下列编译命令,编译成功后会在上级目录下生成STRESS_CACHE.elf:
    >python3 ./build.py STRESS_CACHE ../ rhino

### 压测用例加载
- flash版本用例
1. 确认PC上安装好了yoctools工具
2. 在csi-test目录下配置好 .gdbinit 文件
3. 在csi-test目录下download测试镜像到板子的flash
    >make flashall

- ram版本用例
1. 在csi-test目录下配置好 .gdbinit 文件
2. 在csi-test目录下load测试镜像到板子的ram
    >xxx_gdb ./STRESS_CACHE.elf
    >注:xxx_gdb为t-head工具链中的gdb



### 压测用例执行
1. 配置cache压测用例使用的IIC和运行的次数:
    >AT+CACHE_STRESS_CONFIG=iic_idx,iic_data_pin,iic_data_func,iic_clock_pin,iic_clock_func,running_times

2. 运行压测:
    >AT+STRESS_CACHE=

### 压测运行展示
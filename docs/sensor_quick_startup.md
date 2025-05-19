# Sensor 快启配置

Sensor 初始化时机通过`CONFIG_SENSOR_QUICK_STARTUP`来决定，置 1 代表在 boot 阶段进行初始化以节省进入 yoc 后等待 sensor 稳定的时间，置 0 代表正常流程

下面以 sc2356 为例进行修改演示

> [!Note]
> 该功能需要 boot 和 yoc 配合使用，两边的`CONFIG_SENSOR_QUICK_STARTUP`配置需要保持同步，否则会导致 sensor 初始化异常

## Boot 配置

- 复制 xxx_sensor_ctl.c 到 boot 目录

  - 在 sdk 中找到对应的 sensor 目录 `components/cvi_sensor/sms_sc2356`，并复制其中的 sc2356_sensor_ctl.c 到 `solutions/cv181x_boot/board/src/sensor/` 目录下
  - xxx_sensor_ctl.c 包含 sensor 的 i2c 初始化序列

- 修改 boot 目录下的 xxx_sensor_ctl.c

  - 对于`solutions/cv181x_boot/board/src/sensor/`目录下的`sc2356_sensor_ctl.c`，只需保留 i2c read/write、sensor probe/init 等函数确保初始化 setting 能正常设下去，删除其他非必要函数，可以对比修改前后的已有文件快速找出差异

- 修改 boot 目录下的 packge.yaml

  - 路径：`solutions/cv181x_boot/packge.yaml`
  - 在该 yaml 文件中添加`CONFIG_SENSOR_XXXX`的配置，并配置成 1
  - 配置`CONFIG_SENSOR_QUICK_STARTUP`为 1
    ```yaml
    CONFIG_SENSOR_QUICK_STARTUP: 1
    # SENSOR
    CONFIG_SENSOR_SMS_SC2356: 1
    ```

- 修改 boot 目录下的 sensor.c

  - 路径：`solutions/cv181x_boot/board/src/sensor/sensor.c`
  - 添加对应的`CONFIG_SENSOR_XXXX`配置，确保能执行到指定的 sensor，注意要确定 sensor 的 i2c 地址和 bus 号
    ```C
    #if CONFIG_SENSOR_SMS_SC2356
    extern int sc2356_probe(uint8_t i2c_addr, uint8_t i2c_dev);
    extern void sc2356_init(uint8_t ViPipe);
    extern void sc2356_exit(uint8_t ViPipe);
    #endif
    int sensor_init(void)
    {
    	......
    #if CONFIG_SENSOR_SMS_SC2356
    	ret = sc2356_probe(0x36, 1);
    	sc2356_init(0);
    	sc2356_exit(0);
    #endif
       ......
    }
    ```

- 修改 boot 目录下的 pinmux.c
  - 路径：`solutions/cv181x_boot/board/src/sensor/pinmux.c`
  - 根据原理图和 pinoutlist 配置 sensor 的引脚复用和状态，可以参考`solutions/smart_doorbell/customization/xxxx/custom_platform.c`里面的 SensorPinmux 函数来配置

## Yoc 配置

- Yaml 配置

  - 路径：`solutions/smart_doorbell/customization/xxxx/package.xxxx.yaml`，将`CONFIG_SENSOR_QUICK_STARTUP`置 1
    ```yaml
    CONFIG_SENSOR_QUICK_STARTUP: 1
    ```

- 修改 cvi_sensor 目录下的 xxx_sensor_ctl.c
  - 路径：`components/cvi_sensor/sms_sc2356/sc2356_sensor_ctl.c`
  - 在 sc2356_sensor_ctl.c 中的 probe 和 init 函数开头加上 return，避免 sensor 重复初始化
    ```C
    int sc2356_probe(VI_PIPE ViPipe)
    {
    #if CONFIG_SENSOR_QUICK_STARTUP
        return 0;
    #endif
    ......
    }
    static void sc2356_linear_1600x1200p30_init(VI_PIPE ViPipe)
    {
    #if CONFIG_SENSOR_QUICK_STARTUP
    	return;
    #endif
    ......
    }
    ```

## 重新生成固件进行烧录升级

因为上述内容涉及到对 boot 的修改，所以要重新编译 boot 并替换固件中的 boot.bin

编译 boot 指令：`make cv181x_boot`

boot 生成路径：`solutions/cv181x_boot/boot.bin`

> [!WARNING]
> 默认固件 boot 文件会从`boards/xxx/bootimgs`进行拷贝，需手动替换 sensor 快启的`boot.bin`文件，并修改文件名称为`boot`

## sensor 快启配置前后对比

以 cv1810c+sc2356 为例，通过输出捕获第一帧图像的时间戳打印进行测试，在 yaml 文件中开启`CONFIG_APP_DUMP_FRAME`之后，上电会打印 dump 到第一帧图像的时间戳

修改路径：`solutions/smart_doorbell/customization/xxxx/package.xxxx.yaml`：

```C
# 第一帧图像捕获测试
CONFIG_APP_DUMP_FRAME: 1
```

### 测试结果

- 未配置 sensor 快启之前：

```shell
dump frame 0 done. ##cur_ms:126
```

- 配置 sensor 快启之后：

```shell
dump frame 0 done. ##cur_ms:86
```

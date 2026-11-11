# CSI 2.0 对接测试文档

## 概述

CSI 2.0 的对接需要CSI 2.0 测试的支持。测试套件存在于csi_test目录中。对接是否完成，需要通过CSI对接测试来进行验证。对于CSI对接工程师，使用此套测试工具可以有效的帮助验证对接质量。

## 测试套件对接需求
1. CSI对接驱动模块中，许多会使用到pin脚，例如UART，请确保使用pin脚的模块都支持pinmux引脚复用，测试套件中所有的测试都会调用 csi_pin_set_mux() 接口进行测试，如果模块不支持引脚复用测试将会失败。
2. 在 `compoment/chip_<your-chip>` 的`soc.h`中,需要提供规范化的引脚复用定义。请在此文件中`pin_name_t`枚举类型定义每个引脚的名称，请在`pin_func_t`枚举类型中定义`pin_name`与`pin_func`的对应关系，格式为 “pinname-device(idx)-func” 例如 “GPIO01_UART0_TX”，具体可参考[文件](https://gitee.com/yocop/chip_ch2601/blob/master/sys/soc.h)
3. 在 `board/<your-board>` 中提供 `csi_config.h`，这个头文件主要对可选的CSI接口定义进行标注，下面给出一个`csi_config.h` 的示例：

```h
#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_


/* 
 * defination for the UART can be used by test AT command Console
 * define uart pins and it pin functions accordingly
 * undefine TEST_UART_PINMUX_INIT if this UART not support pinmux
 */
#define CONSOLE_UART_IDX        0
#define TEST_UART_PINMUX_INIT

#ifdef TEST_UART_PINMUX_INIT
#define CONSOLE_TXD             PIN0
#define CONSOLE_RXD             PIN1
#define CONSOLE_TXD_FUNC        PIN0_UART0_TX
#define CONSOLE_RXD_FUNC        PIN1_UART0_RX
#endif

/* 
 *  CSI Interface Configuration File
 *  Undefine optional unimplemented CSI functions
 *  for the optional functions that being implemented, 
 *  chnage the corrspoding line to define
*/

/* CSI GPIO and optional implemented functions */
#undef CSI_GPIO_DEBONCE

/* CSI GPIO_PIN optional implemented functions */
#undef CSI_GPIO_PIN_DEBONCE

/* CSI PIN_MUX optional implemented functions */
#undef CSI_PIN_SPEED
#undef CSI_PIN_WEAKUP
#undef CSI_PIN_DRIVE
#undef CSI_PIN_GET_DEVIDX
#undef CSI_PIN_GET_CHANNEL
#undef CSI_PIN_GET_PINNAME

/* CSI ADC optional implemented functions */
#undef CSI_ADC_CHANNEL_SAMPLING_TIME
#undef CSI_ADC_GET_STATE
#undef CSI_ADC_LINK

/* CSI PWM optional implemented functions */
#undef CSI_PWM_CAPTURE
#undef CSI_PWM_CALLBACK

/* CSI TIMER optional implemented functions */
#undef CSI_TIMER_IS_RUNNING

/* CSI RTC optional implemented functions */
#undef CSI_RTC_IS_RUNNING

/* CSI WDT optional implemented functions */
#define CSI_WDT_STOP
#define CSI_WDT_GET_REMAINING_TIME
#define CSI_WDT_IS_RUNNING
#define CSI_WDT_CALLBACK

/* CSI UART optional implemented functions */
#undef CSI_UART_FLOWCTRL
#undef CSI_UART_LINK_DMA
#undef CSI_UART_GET_STATE

/* CSI SPIFLASH optional implemented functions */
#define CSI_SPIFLASH_SPI
#undef CSI_SPIFLASH_QSPI
#undef CSI_SPIFLASH_LOCK
#undef CSI_SPIFLASH_READ_REG

/* CSI CODEC optional implemented functions */
#undef CSI_CODEC_FLOWCTRL
#undef CSI_CODEC_LINK_DMA
#undef CSI_CODEC_GET_STATE

#endif
```

4. 确保开发版可通过Yoc的烧写工具“product”对开发版进行烧写

## 测试套件编译

建议新建solution工程，专门供CSI TEST对接测试使用，此工程亦可方便以后在云上实验室进行1520验证  
1. 在`solution`目录中以`normboot`工程为模版，创建新的工程，例如`<chip_name>_csi_test`, 
2. 在新建的工程`solution/<chip_name>_csi_test`的源代码中，修改`app/app_main.c`, 在其中的#include `csi_test.h`, 在main 保留最基本的 `boarb_yoc_init()`,然后直接调用测试程序入口 `test_entry()`, 例如：

```
#include "app_main.h"
#include "csi_test.h"

int main(int argc, char *argv[])
{
    board_yoc_init();
    test_entry();
}
```

3. 在`package.yaml` 修改项目名为`<chip_name>_csi_test`, 在depends 段落中添加 `- csi_test: <branch>`, 其中 <branch> 为要对接的版本号
4. 添加 csi_test 代码仓库到 components 目录， 并checkout 到相应的 branch
5. 通过运行make 来编译此工程， 编译好的工程即为 csi test 套件

## 套件烧写

#### 使用gdb烧写(推荐)
1. 启动 DebugServerConsole，链接测试版
2. 配置 `ch2601_csi_test/gdbinit` 文件，使其中的IP地址和端口为上一步中DebugServerConsole的地址和端口
3. 使用 make flash 或 make flashall 进行烧写

## 测试套件运行

烧写成功后，连接上上`csi_config.h`中指定的UART接口，重启开发版如果看到输出`CSI TEST AT Command Console`，输入 `AT` UART 中输出 `OK` 即为测试套件默认配置启动成功。

默认配置中并不含有测试模块，测试模块需要通过修改`csi_test`目录下`package.yaml`文件来实现。 在`def_config:` 代码块中加入相应的模块,例如 `CONFIG_GPIO: 1`, 保存并重新编译烧写后，即可开始相应模块的测试。具体测试命令请参见相应测试模块文档。

- [ADC.md](docs/driver/ADC.md)
- [AES.md](docs/driver/AES.md)
- [AES_new.md](docs/driver/AES_new.md)
- [BAUD_CALC.md](docs/driver/BAUD_CALC.md)
- [CODEC.md](docs/driver/CODEC.md)
- [CRC.md](docs/driver/CRC.md)
- [DMA.md](docs/driver/DMA.md)
- [EFLASH.md](docs/driver/EFLASH.md)
- [GPIO.md](docs/driver/GPIO.md)
- [GPIO_PIN.md](docs/driver/GPIO_PIN.md)
- [I2S.md](docs/driver/I2S.md)
- [IIC.md](docs/driver/IIC.md)
- [JUPITER.md](docs/driver/JUPITER.md)
- [MAILBOX.md](docs/driver/MAILBOX.md)
- [MMC.md](docs/driver/MMC.md)
- [PIN.md](docs/driver/PIN.md)
- [PINMUX.md](docs/driver/PINMUX.md)
- [PWM.md](docs/driver/PWM.md)
- [QSPI.md](docs/driver/QSPI.md)
- [README.md](docs/driver/README.md)
- [RSA.md](docs/driver/RSA.md)
- [RTC.md](docs/driver/RTC.md)
- [SASC.md](docs/driver/SASC.md)
- [SHA.md](docs/driver/SHA.md)
- [SPI.md](docs/driver/SPI.md)
- [SPIFLASH.md](docs/driver/SPIFLASH.md)
- [SPINAND.md](docs/driver/SPINAND.md)
- [TICK.md](docs/driver/TICK.md)
- [TIMER.md](docs/driver/TIMER.md)
- [TIPC.md](docs/driver/TIPC.md)
- [TRNG.md](docs/driver/TRNG.md)
- [UART.md](docs/driver/UART.md)
- [WDT.md](docs/driver/WDT.md)
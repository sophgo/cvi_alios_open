# GPIO 测试工具 (AliOS)

## 概述

`testgpio` 是运行在 AliOS 上的 CLI GPIO 测试工具，提供输出、输入、翻转、中断和环回测试功能，用于验证目标板上的 GPIO 功能。

## 前置条件

- 板端需运行已启用 CLI 的 AliOS 系统。
- **环回测试**需要用杜邦线将两个 GPIO 引脚物理连接。
- 测试前需确保目标引脚已复用为 GPIO 功能（例如通过 `cvi_pinmux` 配置）。

## 命令说明

### 1. 输出测试

将 GPIO 引脚设置为输出高电平或低电平。

```
testgpio o <bank> <pin> <value>
```

| 参数  | 说明                          |
|-------|-------------------------------|
| bank  | GPIO 组号 (0, 1, 2 ...)       |
| pin   | 组内引脚号 (0~31)             |
| value | 0 = 低电平, 1 = 高电平       |

**示例：**
```
testgpio o 0 28 1       # bank0 pin28 输出高电平
testgpio o 0 28 0       # bank0 pin28 输出低电平
```

### 2. 输入测试

读取 GPIO 引脚的当前电平。

```
testgpio i <bank> <pin>
```

**示例：**
```
testgpio i 0 29         # 读取 bank0 pin29 电平
```

**预期输出：**
```
[PASS] GPIO bank0 pin29 input value: 1
```

### 3. 翻转测试

反复翻转 GPIO 引脚的输出电平。

```
testgpio toggle <bank> <pin> <delay_ms> <count>
```

| 参数     | 说明                        |
|----------|-----------------------------|
| delay_ms | 每次翻转的间隔时间（毫秒）  |
| count    | 翻转次数                    |

**示例：**
```
testgpio toggle 0 28 100 10     # bank0 pin28 每 100ms 翻转一次，共 10 次
```

**预期输出：**
```
Start toggle: bank0 pin28, interval 100 ms, count 10
  toggle[1]: level=1
  toggle[2]: level=0
  ...
[PASS] GPIO bank0 pin28 toggle done
```

### 4. 中断测试

等待指定边沿的 GPIO 中断触发。

```
testgpio irq <bank> <pin> <mode> [timeout_s]
```

| 参数      | 说明                                   |
|-----------|----------------------------------------|
| mode      | `rising`（上升沿）或 `falling`（下降沿）|
| timeout_s | 等待超时时间，单位秒（默认 5）         |

**示例：**
```
testgpio irq 0 29 rising 10    # 等待 bank0 pin29 上升沿中断，超时 10 秒
testgpio irq 0 29 falling      # 等待下降沿中断，默认超时 5 秒
```

**预期输出（成功）：**
```
Waiting for rising edge interrupt (bank0 pin29), timeout 10 s...
[PASS] rising edge interrupt triggered (pin_mask=0x20000000, 120 ms)
```

**预期输出（超时）：**
```
[FAIL] rising edge interrupt timeout (not triggered within 10 s)
```

### 5. 环回测试

综合测试，需要用导线**物理连接**两个 GPIO 引脚。自动依次执行 5 项子测试：

1. 输出高电平 → 读到高电平
2. 输出低电平 → 读到低电平
3. Toggle 环回验证
4. 上升沿中断环回
5. 下降沿中断环回

```
testgpio loopback <out_bank> <out_pin> <in_bank> <in_pin>
```

| 参数     | 说明                |
|----------|---------------------|
| out_bank | 输出端 GPIO 组号    |
| out_pin  | 输出端引脚号        |
| in_bank  | 输入端 GPIO 组号    |
| in_pin   | 输入端引脚号        |

**示例：**
```
testgpio loopback 0 28 0 29    # 输出端: bank0 pin28，输入端: bank0 pin29
```

**预期输出（全部通过）：**
```
--- Loopback Test: OUT(bank0 pin28) -> IN(bank0 pin29) ---
[PASS] Output HIGH -> Read HIGH
[PASS] Output LOW -> Read LOW
[PASS] Toggle loopback: 0 -> 1
[PASS] Rising edge interrupt triggered
[PASS] Falling edge interrupt triggered

--- Loopback Test Summary ---
Total: 5, Passed: 5, Failed: 0
[RESULT] Loopback Test PASS
```

### 6. 帮助

```
testgpio help
testgpio -h
testgpio            # 无参数时也会打印帮助信息
```

## 引脚复用自动配置（`set_gpio_pinmux`）

程序内置了 `set_gpio_pinmux()` 函数，在每个测试子命令（`o` / `i` / `toggle` / `irq` / `loopback`）执行前会**自动调用**，将目标引脚复用切换为 GPIO 模式，无需手动执行 `cvi_pinmux` 命令。

### 工作原理

`set_gpio_pinmux()` 根据传入的 `bank` + `pin` 在内部 switch 表中查找，匹配后调用平台宏 `PINMUX_CONFIG(PAD_NAME, GPIO_FUNC)` 完成寄存器配置。未匹配的引脚会静默跳过（不影响测试流程）。

### 当前已配置引脚

| Bank | Pin | PINMUX_CONFIG 调用                         | 等效 CLI 命令                        |
|------|-----|--------------------------------------------|--------------------------------------|
| 0    | 28  | `PINMUX_CONFIG(IIC0_SCL, XGPIOA_28)`      | `cvi_pinmux -w IIC0_SCL/XGPIOA_28`  |
| 0    | 29  | `PINMUX_CONFIG(IIC0_SDA, XGPIOA_29)`      | `cvi_pinmux -w IIC0_SDA/XGPIOA_29`  |

### 扩展方法

如需测试其他 GPIO 引脚，在 `set_gpio_pinmux()` 对应 bank 的 switch 中添加一行即可：

```c
static void set_gpio_pinmux(unsigned int bank, unsigned int pin)
{
    if (bank == 0) {
        switch (pin) {
        case 28: PINMUX_CONFIG(IIC0_SCL, XGPIOA_28); break;
        case 29: PINMUX_CONFIG(IIC0_SDA, XGPIOA_29); break;
        // 新增示例：
        case 18: PINMUX_CONFIG(JTAG_CPU_TCK, XGPIOA_18); break;
        case 19: PINMUX_CONFIG(JTAG_CPU_TMS, XGPIOA_19); break;
        default: return;
        }
    } else if (bank == 1) {
        switch (pin) {
        // case 0: PINMUX_CONFIG(PWM0_BUCK, XGPIOB_0); break;
        default: return;
        }
    }
    ...
}
```

> PAD 名称和 GPIO 功能名称定义在 `pinctrl-mars.h`（通过 `mars_reg_fmux_gpio.h` 展开），可通过 `cvi_pinmux -l` 查看所有可用引脚及其功能列表。

## GPIO Bank / Pin 映射关系

本工具中 GPIO 按 `bank`（组）+ `pin`（引脚号）方式组织：

| Bank | 引脚范围 | 对应 GPIO       |
|------|----------|-----------------|
| 0    | 0 ~ 31   | GPIOA 0 ~ 31   |
| 1    | 0 ~ 31   | GPIOB 0 ~ 31   |
| 2    | 0 ~ 31   | GPIOC 0 ~ 31   |
| ...  | ...      | ...             |

例如 `XGPIOA_28` 对应 `bank=0, pin=28`。

## 常见问题排查

| 现象                     | 可能原因                       | 解决方法                                |
|--------------------------|--------------------------------|-----------------------------------------|
| `csi_gpio_init failed`  | bank 编号无效                  | 检查板端 GPIO 组数                      |
| 环回读取值不匹配        | 未物理连接                     | 用杜邦线连接两个引脚                    |
| 中断超时                 | 引脚未复用为 GPIO / 无触发信号 | 检查 pinmux 配置和接线                  |
| 中断测试系统 panic       | 驱动 IRQ 链表未初始化          | 确认已应用 `csi_gpio_irq_register` 修复 |

## 源文件

- `test_gpio.c` — 测试程序源码
- `csi_gpio.c` — GPIO CSI 驱动实现（位于 `chip_cv184x/src/drivers/gpio/cvi/src/`）

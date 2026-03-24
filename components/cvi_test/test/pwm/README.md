# PWM 测试指南

## 1. 硬件概述

CV184X SOC 内置 3 组 PWM IP，共 16 路 PWM 输出通道：

| PWM IP | Bank 编号 | 通道数 | 全局 PWM 编号 |
|--------|-----------|--------|---------------|
| pwmchip0  | 0 | 6 (ch0~ch5) | pwm0  ~ pwm5  |
| pwmchip6  | 1 | 6 (ch0~ch5) | pwm6  ~ pwm11 |
| pwmchip12 | 2 | 4 (ch0~ch3) | pwm12 ~ pwm15 |

### AliOS CSI 映射关系

全局 PWM 编号 N 与 bank/channel 转换规则：

- **N < 12**：`bank = N / 6`，`channel = N % 6`
- **N >= 12**：`bank = 2`，`channel = N - 12`

**示例**：PWM7 → bank 1, channel 1；PWM13 → bank 2, channel 1

### 硬件引脚要求

| PWM 通道 | 引脚 (Pad)  | Pinmux 配置      | 寄存器          |
|----------|------------|-------------------|-----------------|
| PWM0     | PWM0_BUCK  | PWM_0             | 0x030010EC = 0  |
| PWM7     | SD1_D0     | PWM_7             | —               |

---

## 2. 测试工具

本目录包含两个测试工具：

| 工具 | CLI 命令 | 用途 |
|------|----------|------|
| `test_pwm.c`      | `testpwm`       | 手动 PWM 输出/捕获测试 |
| `pwm_auto_test.c` | `pwm_auto_test` | PWM 自动化功能测试（12 项测试用例） |

---

## 3. 手动测试工具 (`testpwm`)

### 3.1 命令格式

```
testpwm <bank> <channel> <mode> [参数...]
```

### 3.2 PWM 输出模式 (Output)

```
testpwm <bank> <channel> o <period_ns> <duty_ns> <polarity>
```

| 参数 | 说明 |
|------|------|
| `bank`      | PWM bank 编号（0~2） |
| `channel`   | 通道编号（bank 0/1: 0~5, bank 2: 0~3） |
| `o`         | 输出模式 |
| `period_ns` | PWM 周期，单位纳秒 |
| `duty_ns`   | 占空比高电平时间，单位纳秒 |
| `polarity`  | 极性，0 = LOW（正常），1 = HIGH（反转） |

**示例**：

```bash
# Bank 0, Channel 0, 周期 1ms (1KHz), 占空比 50%, 正常极性
testpwm 0 0 o 1000000 500000 0

# Bank 1, Channel 1 (PWM7), 周期 500us (2KHz), 占空比 25%, 反转极性
testpwm 1 1 o 500000 125000 1

# Bank 2, Channel 0 (PWM12), 周期 20us (50KHz), 占空比 50%
testpwm 2 0 o 20000 10000 0
```

### 3.3 PWM 捕获模式 (Capture / Input)

```
testpwm <bank> <channel> i
```

| 参数 | 说明 |
|------|------|
| `bank`    | PWM bank 编号（0~2） |
| `channel` | 通道编号 |
| `i`       | 捕获模式 |

捕获模式会在 5 秒内持续读取输入频率，每秒打印一次测量值。PWM 时钟基准为 250 MHz。

**示例**：

```bash
# 捕获 Bank 0, Channel 0 上的 PWM 输入信号频率
testpwm 0 0 i
```

---

## 4. 自动化测试工具 (`pwm_auto_test`)

### 4.1 命令格式

```
pwm_auto_test <bank_idx>              # 运行全部 12 项测试
pwm_auto_test <bank_idx> <tc_number>  # 运行指定测试用例 (1~12)
pwm_auto_test <bank_idx> -h           # 显示帮助信息
```

| 参数 | 说明 |
|------|------|
| `bank_idx`  | PWM bank 编号（0 = pwmchip0, 1 = pwmchip6, 2 = pwmchip12） |
| `tc_number` | 可选，测试用例编号（1~12），不指定则运行全部 |

### 4.2 测试用例列表

| 编号 | 名称 | 测试内容 |
|------|------|----------|
| TC01 | Init / Uninit | 验证 `csi_pwm_init` 和 `csi_pwm_uninit` 基本初始化/反初始化流程 |
| TC02 | Repeated init / uninit | 连续 10 次初始化/反初始化，验证无资源泄漏 |
| TC03 | Output config (period/duty) | 测试多种周期和占空比组合配置（1KHz 25%/50%/75%/100%、2KHz、10KHz、50KHz） |
| TC04 | Output start / stop | 验证输出启动/停止功能，包括停止后重新启动 |
| TC05 | Polarity (HIGH/LOW) | 验证正常极性 (`PWM_POLARITY_LOW`) 和反转极性 (`PWM_POLARITY_HIGH`) 配置 |
| TC06 | Multi-channel output | 同时配置并启动当前 bank 的所有通道，验证多通道并行输出 |
| TC07 | Boundary values | 边界值测试：最小周期 (100ns)、零占空比、100% 占空比、大周期 (1s) |
| TC08 | Invalid parameter handling | 非法参数测试：duty > period、无效极性值、无效 bank 编号 |
| TC09 | Capture config | 捕获模式 API 测试：POSEDGE（支持）、NEGEDGE/BOTHEDGE（应返回不支持） |
| TC10 | PWM0 functional test | PWM0 完整功能测试：配置 pinmux → 设置 1KHz/50% → 输出 2 秒 → 停止 |
| TC11 | PWM7 functional test | PWM7 完整功能测试：配置 pinmux → 设置 1KHz/100% → 输出 2 秒 → 停止 |
| TC12 | Stress test (start/stop) | 压力测试：连续 100 次 start/stop 循环，验证稳定性 |

### 4.3 使用示例

```bash
# 在 Bank 0 上运行全部测试
pwm_auto_test 0

# 在 Bank 1 上运行全部测试
pwm_auto_test 1

# 仅运行 TC01 (Init/Uninit) — Bank 0
pwm_auto_test 0 1

# 仅运行 TC03 (Output config) — Bank 0
pwm_auto_test 0 3

# 运行 TC10 (PWM0 功能测试，bank_idx 参数被忽略，固定使用 bank 0)
pwm_auto_test 0 10

# 运行 TC11 (PWM7 功能测试，bank_idx 参数被忽略，固定使用 bank 1)
pwm_auto_test 0 11

# 运行 TC12 (压力测试) — Bank 2
pwm_auto_test 2 12
```

> **注意**：TC10 和 TC11 分别固定使用 PWM0 (bank 0, ch 0) 和 PWM7 (bank 1, ch 1)，
> 命令行中的 `bank_idx` 参数对这两个用例无影响。

### 4.4 测试输出说明

测试运行时，每个断言会输出 `[PASS]` 或 `[FAIL]`，最终输出汇总报告：

```
============================================================
  PWM Auto Test Summary
============================================================
  Total : 30
  PASS  : 30
  FAIL  : 0
  Result: ALL PASSED
============================================================
```

- **Total**：总断言数
- **PASS**：通过数
- **FAIL**：失败数
- **Result**：`ALL PASSED`（全部通过）或 `SOME FAILED`（存在失败）

### 4.5 测试覆盖的 CSI API

| API | 覆盖的测试用例 |
|-----|----------------|
| `csi_pwm_init`           | TC01, TC02, TC03~TC12 |
| `csi_pwm_uninit`         | TC01, TC02, TC03~TC12 |
| `csi_pwm_out_config`     | TC03, TC04, TC05, TC06, TC07, TC08, TC10, TC11, TC12 |
| `csi_pwm_out_start`      | TC04, TC05, TC06, TC10, TC11, TC12 |
| `csi_pwm_out_stop`       | TC04, TC05, TC06, TC10, TC11, TC12 |
| `csi_pwm_capture_config` | TC09 |
| `csi_pwm_capture_start`  | TC09 |
| `csi_pwm_capture_stop`   | TC09 |

---

## 5. 常用 PWM 频率与参数参考

| 频率 | period_ns | 50% duty_ns | 说明 |
|------|-----------|-------------|------|
| 1 KHz   | 1000000     | 500000    | 常用测试频率 |
| 2 KHz   | 500000      | 250000    | — |
| 10 KHz  | 100000      | 50000     | — |
| 50 KHz  | 20000       | 10000     | — |
| 100 KHz | 10000       | 5000      | — |
| 1 MHz   | 1000        | 500       | — |
| 10 MHz  | 100         | 50        | 接近硬件最小周期 |

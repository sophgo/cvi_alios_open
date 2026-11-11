# SPI 测试指南

## 1. 硬件概述

CV184X SOC 内置多路 SPI 控制器，支持 Master / Slave 模式，支持 Poll、IRQ、DMA 三种传输方式。

### 硬件接线要求

数据通路测试（Loopback 回环测试）需要将 **MOSI** 与 **MISO** 短接：

| SPI 控制器 | MOSI 引脚 (SDO) | MISO 引脚 (SDI) | Pinmux 配置 |
|-----------|-----------------|-----------------|-------------|
| SPI0 | PAD_MIPI_TXM1 | PAD_MIPI_TXP1 | SPI0_SDO / SPI0_SDI |

> **注意**：使用 SPI0 以外的控制器时，需自行确认引脚配置并在代码中补充 pinmux 设置。

### 接线示意

```
SPI Master
  MOSI (SDO) ──────┐
                    │  短接（Loopback）
  MISO (SDI) ──────┘
  SCK  ────────────── (正常连接或悬空)
  CS   ────────────── (正常连接或悬空)
```

---

## 2. 测试工具

本目录包含两个测试工具：

| 工具 | CLI 命令 | 用途 |
|------|----------|------|
| `spi_auto_test.c`     | `spi_auto_test`     | SPI 自动化功能测试（16 项测试用例） |
| `spi_loopback_test.c` | `spi_loopback_test` | SPI 回环快速验证（依次执行 Poll / IRQ / DMA 模式） |

---

## 3. 回环快速验证工具 (`spi_loopback_test`)

### 3.1 命令格式

```
spi_loopback_test
```

无需参数，固定使用 **SPI0**，数据长度 **128 字节**。

### 3.2 测试流程

该工具会自动依次执行以下三种传输模式的回环测试：

| 序号 | 传输模式 | 说明 |
|------|----------|------|
| 1 | Poll mode | 轮询方式发送/接收 128 字节，对比数据 |
| 2 | IRQ mode  | 中断方式异步发送/接收 128 字节，对比数据 |
| 3 | DMA mode  | DMA 方式发送/接收 128 字节，对比数据 |

### 3.3 SPI 配置参数

| 参数 | 值 |
|------|-----|
| 控制器 | SPI0 |
| 模式 | Master |
| 时钟极性/相位 | CPOL=0, CPHA=0 |
| 帧长度 | 8-bit |
| 波特率 | 1 MHz |
| 数据长度 | 128 字节 |

### 3.4 输出说明

- 测试前打印发送缓冲区的 hex dump
- 每种模式完成后打印接收缓冲区的 hex dump
- 对比结果输出 `success` 或 `failed`

**示例输出**：

```
--------------------- show src buffer data ------------------------
00000000 - 00 01 02 03 04 05 06 07 ...

######################## start poll mode test ##############################
00000000 - 00 01 02 03 04 05 06 07 ...
######################## poll mode test success! ##############################

######################## start irq mode test ##############################
...
######################## irq mode test success! ##############################

######################## start dma mode test ##############################
...
######################## dma mode test success! ##############################
```

---

## 4. 自动化测试工具 (`spi_auto_test`)

### 4.1 命令格式

```
spi_auto_test <spi_idx>              # 运行全部 16 项测试
spi_auto_test <spi_idx> <tc_number>  # 运行指定测试用例 (1~16)
spi_auto_test <spi_idx> -h           # 显示帮助信息
```

| 参数 | 说明 |
|------|------|
| `spi_idx`   | SPI 控制器编号（0 = SPI0, 1 = SPI1, ...） |
| `tc_number` | 可选，测试用例编号（1~16），不指定则运行全部 |

### 4.2 测试用例列表

| 编号 | 名称 | 测试内容 | 需要回环接线 |
|------|------|----------|:----------:|
| TC01 | Init / Uninit | 验证 `csi_spi_init` 和 `csi_spi_uninit` 基本初始化/反初始化流程 | 否 |
| TC02 | Repeated init / uninit | 连续 10 次初始化/反初始化，验证无资源泄漏 | 否 |
| TC03 | Mode config (Master/Slave) | 验证 Master 和 Slave 模式设置 | 否 |
| TC04 | CPOL/CPHA config | 验证 4 种时钟极性/相位组合配置 (CPOL0_CPHA0 / CPOL0_CPHA1 / CPOL1_CPHA0 / CPOL1_CPHA1) | 否 |
| TC05 | Frame length config | 验证帧长度配置（8-bit、16-bit） | 否 |
| TC06 | Baud rate config | 验证多种波特率配置（100KHz ~ 25MHz） | 否 |
| TC07 | Callback attach / detach | 验证回调函数的注册与注销，包括注销后重新注册 | 否 |
| TC08 | Poll-mode loopback | Poll 模式 128 字节回环发送/接收，校验数据一致性 | **是** |
| TC09 | IRQ-mode loopback | IRQ 中断模式 128 字节回环异步发送/接收，校验数据一致性 | **是** |
| TC10 | DMA-mode loopback | DMA 模式 128 字节回环发送/接收，校验数据一致性 | **是** |
| TC11 | Various data lengths | Poll 模式下不同数据长度回环测试（1 / 4 / 16 / 64 / 128 / 256 / 512 / 1024 字节） | **是** |
| TC12 | All CPOL/CPHA loopback | 以 4 种 CPOL/CPHA 组合分别进行 64 字节 Poll 回环测试 | **是** |
| TC13 | Various baud rate loopback | 以不同波特率（100KHz ~ 25MHz）分别进行 16 字节 Poll 回环测试 | **是** |
| TC14 | Stress test | 压力测试：Poll 模式 128 字节回环连续 100 次循环，验证稳定性 | **是** |
| TC15 | Select slave | 验证 `csi_spi_select_slave` 接口调用不崩溃 | 否 |
| TC16 | Send-only (poll) | Poll 模式仅发送 64 字节，验证返回值正确（不校验接收数据） | 否 |

### 4.3 使用示例

```bash
# 在 SPI0 上运行全部测试
spi_auto_test 0

# 在 SPI1 上运行全部测试
spi_auto_test 1

# 仅运行 TC01 (Init/Uninit) — SPI0
spi_auto_test 0 1

# 仅运行 TC08 (Poll-mode loopback) — SPI0
spi_auto_test 0 8

# 仅运行 TC09 (IRQ-mode loopback) — SPI0
spi_auto_test 0 9

# 仅运行 TC10 (DMA-mode loopback) — SPI0
spi_auto_test 0 10

# 仅运行 TC14 (压力测试) — SPI0
spi_auto_test 0 14

# 显示帮助信息
spi_auto_test 0 -h
```

### 4.4 测试输出说明

测试运行时，每个断言会输出 `[PASS]` 或 `[FAIL]`，最终输出汇总报告：

```
============================================================
  SPI Auto Test Summary
============================================================
  Total : 42
  PASS  : 42
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
| `csi_spi_init`               | TC01, TC02, TC03~TC16 |
| `csi_spi_uninit`             | TC01, TC02, TC03~TC16 |
| `csi_spi_mode`               | TC03, TC08~TC14, TC15, TC16 |
| `csi_spi_cp_format`          | TC04, TC08~TC14 |
| `csi_spi_frame_len`          | TC05, TC08~TC14 |
| `csi_spi_baud`               | TC06, TC08~TC14, TC16 |
| `csi_spi_attach_callback`    | TC07, TC09, TC10 |
| `csi_spi_detach_callback`    | TC07, TC09, TC10 |
| `csi_spi_send_receive`       | TC08, TC11, TC12, TC13, TC14 |
| `csi_spi_send_receive_async` | TC09 |
| `csi_spi_link_dma`           | TC10 |
| `csi_spi_send_receive_dma`   | TC10 |
| `csi_spi_select_slave`       | TC15 |
| `csi_spi_send`               | TC16 |

---

## 5. 常用 SPI 波特率参考

| 波特率 | 说明 |
|--------|------|
| 100 KHz  | 低速，调试用 |
| 500 KHz  | — |
| 1 MHz    | 默认测试频率 |
| 2 MHz    | — |
| 5 MHz    | — |
| 10 MHz   | — |
| 25 MHz   | 高速，接近平台上限 |

---

## 6. 注意事项

1. **回环接线**：TC08~TC14 数据通路测试需要将 MOSI 和 MISO 短接，否则接收数据为全 0 或随机值，测试会失败。
2. **高波特率限制**：TC13 在高波特率（≥25MHz）下将数据长度限制为 16 字节（FIFO 大小），因为 Poll 模式下 CPU 取数速度可能跟不上硬件，导致 RX FIFO 溢出。
3. **Pinmux 配置**：目前仅 SPI0 的 pinmux 已在代码中自动配置（PAD_MIPI_TXM1 → SPI0_SDO, PAD_MIPI_TXP1 → SPI0_SDI），使用其他 SPI 控制器需手动配置引脚。
4. **DMA 模式**：TC10 使用 `csi_spi_link_dma` 传入 NULL 参数，由驱动内部分配 DMA 通道。

# ADC 测试模块使用指南

## 概述

本模块提供了 ADC（Analog-to-Digital Converter）驱动的测试功能，支持轮询模式和中断模式两种工作方式，可用于验证 ADC 硬件功能和驱动实现。

## 功能特性

- 支持多个 ADC 控制器（ADC0-ADC4）
- 支持多通道采集
- 支持两种工作模式：
  - **轮询模式**：阻塞式读取，适合简单测试
  - **中断模式**：异步读取，支持回调机制
- 自动 ADC 校准（Trim）
- 超时保护机制（中断模式）

## 命令格式

```bash
testadc <chip_id> <ch_id> [mode]
```

### 参数说明

| 参数 | 说明 | 取值范围 | 备注 |
|------|------|----------|------|
| `chip_id` | ADC 控制器 ID | 0-4 | 对应不同的 ADC 控制器 |
| `ch_id` | ADC 通道 ID | 0-N | 具体范围取决于芯片 |
| `mode` | 工作模式（可选） | 0 或 1 | 0=轮询模式（默认），1=中断模式 |

## ADC 通道映射

根据代码注释，以下是 ADC 通道映射关系：

| 命令示例 | 控制器 ID | 通道 ID | 实际 ADC 通道 | 所属域 |
|----------|---------|---------|---------------|--------|
| `testadc 0 1` | 0 | 1 | ADC 通道 1 | AP 域 |
| `testadc 0 2` | 0 | 2 | ADC 通道 2 | AP 域 |
| `testadc 0 3` | 0 | 3 | ADC 通道 3 | AP 域 |
| `testadc 1 1` | 1 | 1 | ADC 通道 4 | AP 域 |
| `testadc 1 2` | 1 | 2 | ADC 通道 5 | AP 域 |
| `testadc 1 3` | 1 | 3 | ADC 通道 6 | AP 域 |
| `testadc 2 1` | 2 | 1 | ADC 通道 7 | AP 域 |
| `testadc 2 2` | 2 | 2 | ADC 通道 8 | AP 域 |
| `testadc 2 3` | 2 | 3 | ADC 通道 9 | AP 域 |
| `testadc 3 1` | 3 | 1 | ADC 通道 10 | RTC 域 |
| `testadc 3 2` | 3 | 2 | ADC 通道 11 | RTC 域 |
| `testadc 3 3` | 3 | 3 | ADC 通道 12 | RTC 域 |
| `testadc 4 1` | 4 | 1 | ADC 通道 13 | RTC 域 |
| `testadc 4 2` | 4 | 2 | ADC 通道 14 | RTC 域 |
| `testadc 4 3` | 4 | 3 | ADC 通道 15 | RTC 域 |

## 使用示例

### 1. 轮询模式测试

```bash
# 测试 ADC0 的通道 1（默认轮询模式）
testadc 0 1

# 显式指定轮询模式
testadc 0 1 0
```

**预期输出：**
```
test adc: chip=0, ch=1, mode=polling
adc get value: 1234
test adc success.
```

### 2. 中断模式测试

```bash
# 测试 RTC 域的 ADC0，通道 1，使用中断模式
testadc 3 1 1
```

**预期输出：**
```
test adc: chip=3, ch=1, mode=interrupt
ADC started async, waiting for IRQ...
adc get value (async): 1234
test adc success.
```

### 3. 批量测试多个通道

```bash
# 依次测试不同的 ADC 通道
testadc 0 1
testadc 1 1
testadc 2 1
testadc 3 1
testadc 4 1
```

## 工作模式详解

### 轮询模式（Polling Mode）

- **流程**：
  1. 初始化 ADC
  2. 使能通道
  3. ADC 校准（Trim）
  4. 启动 ADC（`csi_adc_start`）
  5. 同步读取数据（`csi_adc_read`）
  6. 停止 ADC
  7. 反初始化

- **适用场景**：简单测试、低频采样
- **优点**：实现简单，易于调试
- **缺点**：CPU 占用高，不适合高频采样

### 中断模式（Interrupt Mode）

- **流程**：
  1. 初始化 ADC
  2. 使能通道
  3. ADC 校准（Trim）
  4. 配置接收缓冲区（`csi_adc_set_buffer`）
  5. 注册回调函数（`csi_adc_attach_callback`）
  6. 启动异步采集（`csi_adc_start_async`）
  7. 等待中断回调触发
  8. 停止异步采集（`csi_adc_stop_async`）
  9. 注销回调
  10. 反初始化

- **适用场景**：连续采样、实时应用
- **优点**：CPU 占用低，效率高
- **缺点**：需要正确配置中断和回调
- **超时时间**：2000ms（代码中可配置）

## 错误处理

代码中包含完整的错误处理机制：

| 错误位置 | 错误提示 | 可能原因 |
|----------|----------|----------|
| `csi_adc_init` | `csi_adc_init failed` | 控制器 ID 错误、硬件未就绪 |
| `csi_adc_channel_enable` | `csi_adc_channel_enable failed` | 通道 ID 错误、寄存器访问失败 |
| `csi_adc_start` | `csi_adc_start failed` | ADC 启动失败 |
| `csi_adc_set_buffer` | `csi_adc_set_buffer failed` | 缓冲区配置失败 |
| `csi_adc_attach_callback` | `csi_adc_attach_callback failed` | 回调注册失败 |
| `csi_adc_start_async` | `csi_adc_start_async failed` | 异步启动失败 |
| 中断超时 | `adc timeout! IRQ did not trigger.` | 中断未触发、硬件问题、IRQ 配置错误 |

## 注意事项

1. **中断模式挂起问题**：
   - 184x芯片只有 RTC 域的 ADC0 支持中断模式
   - 如果程序挂在 `csi_adc_channel_enable`，通常是寄存器访问或时钟问题，与 IRQ 无关
   - 如果中断模式超时，检查中断配置和硬件连接

2. **通道映射**：
   - 不同芯片的通道映射不同，请参考硬件文档

3. **校准操作**：
   - `csi_adc_trim()` 用于 ADC 校准，提高精度

4. **缓冲区对齐**：
   - 中断模式的接收缓冲区 `g_recv_buf` 可能需要考虑 cache 对齐
   - 当前代码中缓冲区大小为 1（单次采样）

5. **命令参数**：
   - 如果参数不足，会显示用法提示
   - 第三个参数 `mode` 是可选的，默认为轮询模式

## 代码结构

```c
test_adc()
├── 参数解析
├── ADC 初始化（csi_adc_init）
├── 通道使能（csi_adc_channel_enable）
├── ADC 校准（csi_adc_trim）
├── 轮询模式
│   ├── 启动 ADC（csi_adc_start）
│   ├── 读取数据（csi_adc_read）
│   └── 停止 ADC（csi_adc_stop）
└── 中断模式
    ├── 配置缓冲区（csi_adc_set_buffer）
    ├── 注册回调（csi_adc_attach_callback）
    ├── 启动异步（csi_adc_start_async）
    ├── 等待中断
    ├── 停止异步（csi_adc_stop_async）
    └── 注销回调（csi_adc_detach_callback）
```

## 相关 API

- `csi_adc_init()` - 初始化 ADC
- `csi_adc_uninit()` - 反初始化 ADC
- `csi_adc_channel_enable()` - 使能/禁用通道
- `csi_adc_trim()` - ADC 校准
- `csi_adc_start()` - 启动 ADC（轮询模式）
- `csi_adc_stop()` - 停止 ADC（轮询模式）
- `csi_adc_read()` - 读取 ADC 值（轮询模式）
- `csi_adc_set_buffer()` - 配置接收缓冲区（中断模式）
- `csi_adc_attach_callback()` - 注册回调函数（中断模式）
- `csi_adc_detach_callback()` - 注销回调函数（中断模式）
- `csi_adc_start_async()` - 启动异步采集（中断模式）
- `csi_adc_stop_async()` - 停止异步采集（中断模式）

## 许可证

请参考项目根目录的 LICENSE 文件。

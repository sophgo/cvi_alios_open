# UART 测试套件使用说明

本文档介绍了用于测试 UART（通用异步收发器）功能的命令行工具集。这些工具涵盖了基本的轮询收发、同步回环、DMA 传输以及中断接收等多种测试场景。

## 1. 硬件准备

-   **回环测试**: 对于 `testuart_sync_loop`、`testuart_async_loop`、`testuart_dma` 和 `testuart_async_dma` 测试，您需要将指定 UART 端口的 **TX** 引脚和 **RX** 引脚短接。
-   **与 PC 通信**: 对于 `testuart`、`testuart_fifo_readable`、`testuart_async_tx` 和 `testuart_async_rx` 测试，您需要使用 USB 转串口模块，将 PC 与开发板指定的 UART 端口连接起来。

## 2. 测试命令详解

### 2.1 `testuart` - 基本收发测试

此命令用于与 PC 串口助手进行基本的、阻塞式的收发测试。

#### 功能

-   `send`: 从开发板向 PC 发送指定的字符串。
-   `recv`: 接收来自 PC 的数据，并逐字符打印到开发板的控制台。

#### 用法

```shell
# 发送数据
testuart send <idx> <baudrate> <data>

# 接收数据
testuart recv <idx> <baudrate>
```

-   `<idx>`: UART 端口号 (例如: 2)。
-   `<baudrate>`: 波特率 (例如: 115200)。
-   `<data>`: 要发送的字符串。如果包含空格，请用引号括起来。

#### 示例

```shell
# 向 UART2 发送 "hello world"
testuart send 2 115200 "hello world"

# 在 UART2 上监听数据，直到手动停止
testuart recv 2 115200
```

---

### 2.2 `testuart_sync_loop` - 同步回环测试

此命令在指定的 UART 端口上执行一个同步（阻塞式）回环自检。它会发送一个预定义的字符串，并逐块接收，最后校验收到的数据是否完全一致。

#### 功能

-   验证 UART 的基本发送和接收功能是否正常。
-   测试在不同波特率下的数据完整性。

#### 用法

```shell
testuart_sync_loop <idx> <baudrate>
```

#### 示例

```shell
# 在 UART2 上以 115200 波特率进行同步回环测试
testuart_sync_loop 2 115200
```

#### 预期输出

如果测试成功，将打印 `Test PASSED: Data received correctly.`。

---

### 2.3 `testuart_async_loop` - 异步回环测试

此命令使用异步（中断驱动）模式执行回环测试。它会通过注册回调函数，使用 `csi_uart_send_async()` 和 `csi_uart_receive_async()` API 进行非阻塞的数据收发。

#### 功能

-   验证 UART 的异步发送和接收功能是否正常。
-   测试中断驱动模式下的数据传输和回调机制。
-   适用于需要非阻塞 UART 通信的应用场景。

#### 用法

```shell
testuart_async_loop <idx> <baudrate>
```

#### 示例

```shell
# 在 UART2 上以 115200 波特率进行异步回环测试
testuart_async_loop 2 115200

# 在 UART2 上以 576000 波特率进行异步回环测试
testuart_async_loop 2 576000
```

#### 预期输出

如果测试成功，将打印:
```
Async TX complete
Async RX complete
Test PASSED: Async loopback completed successfully!
  Data integrity verified: 50 bytes
```

#### 与同步回环的区别

-   **同步回环** (`testuart_sync_loop`): 使用阻塞式 API，CPU 会等待每次发送/接收完成。
-   **异步回环** (`testuart_async_loop`): 使用中断驱动，发送/接收操作在后台进行，通过回调通知完成。

---

### 2.4 `testuart_dma` - DMA 回环测试

此命令使用 DMA（直接内存访问）模式执行异步回环测试。它会同时启动接收和发送，整个过程由 DMA 控制器完成，并通过中断回调来通知 CPU 传输结果。

#### 功能

-   验证 UART 的 DMA 模式是否正常工作。
-   测试高吞吐量下的数据传输性能和 CPU 占用情况。

#### 用法

```shell
testuart_dma <idx> <baudrate>
```

**注意**: 根据代码，此测试目前仅为 **UART2** 硬编码了 DMA 通道 (TX: channel 1, RX: channel 0)。

#### 示例

```shell
# 在 UART2 上以 1500000 波特率进行 DMA 回环测试
testuart_dma 2 1500000
```

#### 预期输出

如果测试成功，将打印 `Test PASSED: DMA transfer completed successfully!`。

---

### 2.5 `testuart_fifo_readable` - 中断接收测试

此命令用于测试 UART 的 `UART_EVENT_RECEIVE_FIFO_READABLE` 中断。它会注册一个回调函数，当 UART 的接收 FIFO 中有数据时，中断被触发，回调函数负责读取数据。

#### 功能

-   验证 UART 接收中断和回调机制是否正常。
-   演示如何以事件驱动的方式处理串口数据。

#### 用法

```shell
testuart_fifo_readable <idx> <baudrate>
```

#### 如何测试

1.  在开发板上运行命令: `testuart_fifo_readable 2 115200`。
2.  在 PC 上打开串口调试工具，连接到开发板的 UART2。
3.  在 PC 的串口工具中输入任意字符（例如 "hello"）并发送。
4.  开发板的控制台应打印出 `Received 5 bytes: hello`。
5.  在 PC 的串口工具中输入 `exit` 并发送，测试程序将自动退出。

#### 示例

```shell
# 在 UART2 上启动中断接收测试
testuart_fifo_readable 2 115200
```

---

### 2.6 `testuart_async_dma` - 异步 DMA 回环测试 (Cache 安全)

此命令执行异步 DMA 回环测试，特别针对 Cache 一致性进行了处理，使用静态对齐缓冲区并显式进行 Cache 维护操作。

#### 功能

-   验证 UART 的 DMA 模式在 Cache 开启环境下的稳定性。
-   测试不同数据长度的 DMA 传输。

#### 用法

```shell
testuart_async_dma <idx> <baudrate> [size]
```

-   `size`: (可选) 测试数据长度，默认为 64 字节，最大支持 256 字节。

#### 示例

```shell
# 在 UART2 上以 115200 波特率测试 64 字节
testuart_async_dma 2 115200

# 在 UART2 上以 1500000 波特率测试 128 字节
testuart_async_dma 2 1500000 128
```

#### 预期输出

如果测试成功，将打印 `Test PASSED: Async DMA loopback completed successfully!`。

---

### 2.7 `testuart_async_tx` - 异步 DMA 发送测试

此命令用于测试 UART 的异步 DMA 发送功能。它会使用 DMA 方式向指定的 UART 端口发送自定义字符串，无需 CPU 持续干预。

#### 功能

-   验证 UART 的异步 DMA 发送功能是否正常。
-   测试高效的数据发送性能，减少 CPU 占用。
-   支持自定义发送内容。

#### 用法

```shell
testuart_async_tx <idx> <baudrate> <string>
```

-   `<idx>`: UART 端口号 (例如: UART2)。
-   `<baudrate>`: 波特率 (例如: 115200, 921600)。
-   `<string>`: 要发送的字符串内容。如果包含空格，请用引号括起来。

**注意**: 目前代码仅配置了 **UART2** 的 DMA 发送功能。最大发送长度为 256 字节。

#### 示例

```shell
# 向 UART2 发送 "hello dma"
testuart_async_tx 2 115200 "hello dma"

# 向 UART2 发送较长的字符串
testuart_async_tx 2 921600 "This is an async DMA transmission test"
```

#### 如何测试

1.  在 PC 上打开串口调试工具，连接到开发板的 UART2。
2.  在开发板上运行命令: `testuart_async_tx 2 115200 "hello dma"`。
3.  PC 串口工具应接收并显示 "hello dma"。

#### 预期输出

如果测试成功，将打印:
```
[TX Test] Sending 9 bytes...
[Async DMA] TX complete
[TX Test] Success.
```

---

### 2.8 `testuart_async_rx` - 异步 DMA 接收测试

此命令用于测试 UART 的异步 DMA 接收功能。它会使用 DMA 方式接收来自 PC 或其他设备的数据，并以十六进制格式显示接收到的内容。

#### 功能

-   验证 UART 的异步 DMA 接收功能是否正常。
-   测试高效的数据接收性能，减少 CPU 占用。
-   支持自定义接收数据长度。
-   包含 UART 硬件预热机制，确保接收稳定性。

#### 用法

```shell
testuart_async_rx <idx> <baudrate> [size]
```

-   `<idx>`: UART 端口号 (例如: UART2)。
-   `<baudrate>`: 波特率 (例如: 115200, 921600)。
-   `[size]`: (可选) 期望接收的字节数，默认为 16 字节，最大 256 字节。

**注意**: 目前代码仅配置了 **UART2** 的 DMA 接收功能。最大接收长度为 256 字节。

#### 示例

```shell
# 接收 5 个字节
testuart_async_rx 2 115200 5

# 接收默认的 16 个字节
testuart_async_rx 2 115200

# 接收 32 个字节
testuart_async_rx 2 921600 32
```

#### 如何测试

1.  在开发板上运行命令: `testuart_async_rx 2 115200 5`。
2.  程序会打印 `[RX Test] Waiting for 5 bytes from UART2...`。
3.  在 PC 串口工具中发送 5 个字节的数据（例如 "hello"）。
4.  开发板将以十六进制格式打印接收到的数据。

#### 预期输出

如果测试成功，将打印:
```
[RX Init] Warming up UART hardware...
[RX Test] Waiting for 5 bytes from UART2...
[Async DMA] RX complete
[RX Test] Received hex: 68 65 6C 6C 6F
```

#### 硬件预热机制

此命令在启动 DMA 接收前会先发送一个 0xFF 字节来"预热" UART 硬件，确保 UART 状态机进入活跃状态并稳定波特率时钟，提高接收的可靠性。

---

## 3. 配置其他 UART 端口的 DMA 功能

目前代码默认仅配置了 **UART2** 的 DMA 功能。如果需要在其他 UART 端口（如 UART0、UART1、UART3、UART4 等）上使用 DMA 功能，需要修改以下文件。

### 3.1 需要修改的文件

1. **DMA 通道映射配置文件**  
   路径: `components/chip_cv181x/src/drivers/dma/cvi/src/sysdma_remap.c`

2. **设备 DMA 通道列表配置文件**  
   路径: `components/chip_cv181x/src/sys/devices.c`

### 3.2 修改步骤

以下以添加 **UART4** 的 DMA 支持为例，说明具体的修改步骤。

#### 步骤 1: 修改 DMA 通道映射表

编辑 `sysdma_remap.c` 文件，在 `remap_table[]` 数组中添加或修改 UART 的 DMA 通道映射。

```c
// 文件: components/chip_cv181x/src/drivers/dma/cvi/src/sysdma_remap.c

static struct dma_remap_item remap_table[] = {
    // 如果要使用 UART4，将 UART2 的映射改为 UART4
    {CVI_UART4_TX, 1},      // DMA 通道 1 映射到 UART4 TX
    {CVI_UART4_RX, 0},      // DMA 通道 0 映射到 UART4 RX

    // 或者保留 UART2 并添加 UART4（需要使用不同的 DMA 通道）
    // {CVI_UART2_TX, 1},
    // {CVI_UART2_RX, 0},
    // {CVI_UART4_TX, 5},   // 使用不同的通道号
    // {CVI_UART4_RX, 4},

    {CVI_I2S3_TX, 2},
    {CVI_I2S0_RX, 3},
    // ... 其他设备的映射
};
```

**注意事项:**
- 每个 DMA 通道只能映射到一个设备
- 通道号范围通常为 0-15（取决于硬件）
- TX 和 RX 需要使用不同的通道

#### 步骤 2: 添加 UART DMA 通道列表

编辑 `devices.c` 文件，为新的 UART 端口添加 DMA 通道描述列表。

```c
// 文件: components/chip_cv181x/src/sys/devices.c

// 1. 定义 UART4 的 DMA 通道列表（在已有的 uart2_dma_ch_list 后面添加）
const csi_dma_ch_desc_t uart4_dma_ch_list[] = {
    {0, 0}, {0, 1}, {0, 2},  {0, 3},  {0, 4},  {0, 5},  {0, 6},  {0, 7},
    {0, 8}, {0, 9}, {0, 10}, {0, 11}, {0, 12}, {0, 13}, {0, 14}, {0, 15},
    {0xff, 0xff}  // 结束标记
};

// 2. 在 dma_spt_list 数组中注册 UART4 的 DMA 支持
const csi_dma_ch_spt_list_t dma_spt_list[] = {
    {DEV_DW_UART_TAG, 2, uart2_dma_ch_list},  // UART2
    {DEV_DW_UART_TAG, 4, uart4_dma_ch_list},  // UART4 (新添加)
    {DEV_DW_IIC_TAG,  0, iic0_dma_ch_list},
    // ... 其他设备
};
```

**字段说明:**
- 第一个参数: 设备类型标签 (`DEV_DW_UART_TAG` 表示 UART 设备)
- 第二个参数: UART 端口号 (0, 1, 2, 3, 4...)
- 第三个参数: 该 UART 可用的 DMA 通道列表

#### 步骤 3: 修改测试代码（可选）

如果要在测试代码中支持新的 UART 端口，需要修改 `test_uart.c` 中的 DMA 通道判断逻辑。

```c
// 文件: components/cvi_test/test/uart/test_uart.c

// 在 test_uart_async_dma_tx_func 等函数中:
if (uart_idx == 2) {
    // UART2 使用通道 0 和 1
} else if (uart_idx == 4) {
    // UART4 使用通道 0 和 1（或其他你配置的通道）
    printf("UART4 DMA is supported.\n");
} else {
    printf("Only UART2 and UART4 are supported for DMA.\n");
    return;
}
```

### 3.3 配置示例总结

| UART 端口 | TX DMA 通道 | RX DMA 通道 | 配置文件 |
|----------|------------|------------|---------|
| UART2    | 1          | 0          | sysdma_remap.c, devices.c |
| UART4    | 1          | 0          | sysdma_remap.c, devices.c |

**注意:** 如果同时启用多个 UART 的 DMA，每个 UART 必须使用不同的 DMA 通道，否则会产生冲突。

### 3.4 验证配置

配置完成后，可以使用以下命令验证新 UART 端口的 DMA 功能:

```shell
# 测试 UART4 的异步 DMA 发送
testuart_async_tx 4 115200 "test uart4"

# 测试 UART4 的异步 DMA 接收
testuart_async_rx 4 115200 10

# 测试 UART4 的 DMA 回环
testuart_async_dma 4 115200 64
```

### 3.5 常见问题

**Q: 为什么我的 UART 无法使用 DMA？**  
A: 请检查:
1. 是否在 `sysdma_remap.c` 中正确配置了 DMA 通道映射
2. 是否在 `devices.c` 中添加了对应的通道列表
3. DMA 通道是否与其他设备冲突
4. 硬件引脚是否正确配置

**Q: 可以同时使用多个 UART 的 DMA 功能吗？**  
A: 可以，但需要为每个 UART 分配不同的 DMA 通道，并确保硬件有足够的 DMA 通道可用。


# UART 测试套件使用说明

本文档介绍了用于测试 UART（通用异步收发器）功能的命令行工具集。这些工具涵盖了基本的轮询收发、同步回环、DMA 传输以及中断接收等多种测试场景。

## 1. 硬件准备

-   **回环测试**: 对于 `testuart_sync_loop` 和 `testuart_dma` 测试，您需要将指定 UART 端口的 **TX** 引脚和 **RX** 引脚短接。
-   **与 PC 通信**: 对于 `testuart` 和 `testuart_fifo_readable` 测试，您需要使用 USB 转串口模块，将 PC 与开发板指定的 UART 端口连接起来。

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

### 2.3 `testuart_dma` - DMA 回环测试

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

### 2.4 `testuart_fifo_readable` - 中断接收测试

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

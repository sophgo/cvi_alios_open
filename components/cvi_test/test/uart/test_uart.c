#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <aos/cli.h>
#include <aos/kernel.h>
#include <drv/uart.h>
#include <drv/pin.h>
#include <drv/dma.h>


/**
 * @brief 打印用法说明
 */
static void print_usage(void)
{
    printf("Usage:\n");
    printf("  testuart send <idx> <baudrate> <data>\n");
    printf("  testuart recv <idx> <baudrate>\n\n");
    printf("Description:\n");
    printf("  'send': Sends a string to the specified UART port.\n");
    printf("  'recv': Listens for incoming data on the UART port and prints it.\n\n");
    printf("Arguments:\n");
    printf("  idx      : UART port index (e.g., 0, 1, 2...).\n");
    printf("  baudrate : The baud rate to use (e.g., 115200, 576000, 921600).\n");
    printf("  data     : The string data to send. If it contains spaces, enclose it in quotes.\n\n");
    printf("Example:\n");
    printf("  testuart send 2 115200 \"hello world\"\n");
    printf("  testuart recv 2 115200\n");
}

/**
 * @brief UART测试主函数，用于与上位机进行收发测试
 */
void test_uart_func(int32_t argc, char **argv)
{
    if (argc < 4) {
        print_usage();
        return;
    }

    char *command = argv[1];
    int32_t uart_idx = atoi(argv[2]);
    uint32_t baudrate = atoi(argv[3]);

    csi_uart_t uart_handle;

    // 初始化并配置UART
    if (csi_uart_init(&uart_handle, uart_idx) != CSI_OK) {
        printf("Error: csi_uart_init failed for UART%d.\n", uart_idx);
        return;
    }

    if (csi_uart_baud(&uart_handle, baudrate) != CSI_OK) {
        printf("Error: csi_uart_baud failed to set %u.\n", baudrate);
        csi_uart_uninit(&uart_handle);
        return;
    }

    csi_uart_format(&uart_handle, UART_DATA_BITS_8, UART_PARITY_NONE, UART_STOP_BITS_1);

    // 根据命令执行不同操作
    if (strcmp(command, "send") == 0) {
        if (argc < 5) {
            printf("Error: Missing data for send command.\n");
            print_usage();
            goto uninit_uart;
        }
        const char *data_to_send = argv[4];
        uint32_t data_len = strlen(data_to_send);

        printf("Sending '%s' (%u bytes) to UART%d at %u baud...\n", data_to_send, data_len, uart_idx, baudrate);

        // 使用阻塞发送
        int32_t sent_len = csi_uart_send(&uart_handle, data_to_send, data_len, 5000); // 5秒超时

        if (sent_len == data_len) {
            printf("Send successful.\n");
        } else {
            printf("Send failed or timed out. Sent %d of %u bytes.\n", sent_len, data_len);
        }

    } else if (strcmp(command, "recv") == 0) {
        uint8_t rx_char;
        printf("Start listening on UART%d at %u baud.\n", uart_idx, baudrate);

        // 循环接收并打印字符
        while (1) {
            // 使用阻塞接收，等待一个字符
            int32_t ret = csi_uart_receive(&uart_handle, &rx_char, 1, AOS_WAIT_FOREVER);
            if (ret == 1) {
                // 成功接收到一个字符，直接输出到CLI
                // 换行符 \n 来强制刷新printf的缓冲区
                printf("%c\n", rx_char);
            } else {
                // 发生错误或超时
                printf("\ncsi_uart_receive error, ret = %d\n", ret);
                break;
            }
        }

    } else {
        printf("Error: Unknown command '%s'.\n", command);
        print_usage();
    }

uninit_uart:
    csi_uart_uninit(&uart_handle);
}

ALIOS_CLI_CMD_REGISTER(test_uart_func, testuart, uart send/receive test with PC);

/**
 * @brief 打印回环测试的用法说明
 */
static void print_sync_loop_usage(void)
{
    printf("Usage:\n");
    printf("  testuart_sync_loop <idx> <baudrate>\n\n");
    printf("Description:\n");
    printf("  Performs a robust, synchronous UART self-test. TX and RX pins must be connected.\n\n");
    printf("Arguments:\n");
    printf("  idx      : UART port index (e.g., 0, 1, 2...).\n");
    printf("  baudrate : The baud rate to use (e.g., 115200, 576000, 921600).\n\n");
    printf("Example:\n");
    printf("  testuart_sync_loop 2 115200\n");
}

/**
 * @brief UART同步回环测试主函数
 */
void test_uart_sync_loop_func(int32_t argc, char **argv)
{
    if (argc < 3) {
        print_sync_loop_usage();
        return;
    }

    int32_t uart_idx = atoi(argv[1]);
    uint32_t baudrate = atoi(argv[2]);

    printf("Starting SYNC UART loopback test on UART%d, Baudrate: %u\n", uart_idx, baudrate);
    printf("Please ensure TX and RX pins of UART%d are connected.\n", uart_idx);

    csi_uart_t uart_handle;
    const char *test_string = "Hello Cvitek UART Loopback Test! 1234567890";
    uint32_t data_len = strlen(test_string);
    uint8_t rx_buffer[128] = {0};

    // 1. 初始化UART
    if (csi_uart_init(&uart_handle, uart_idx) != CSI_OK) {
        printf("Error: csi_uart_init failed for UART%d.\n", uart_idx);
        return;
    }

    // 2. 配置波特率和数据格式
    csi_uart_baud(&uart_handle, baudrate);
    csi_uart_format(&uart_handle, UART_DATA_BITS_8, UART_PARITY_NONE, UART_STOP_BITS_1);

    // 3. 分块发送和接收，防止FIFO溢出
    printf("Sending and receiving '%s' (%u bytes) in chunks...\n", test_string, data_len);

    uint32_t sent_total = 0;
    uint32_t recv_total = 0;
    const uint32_t chunk_size = 8; // 每次发送8字节，避免溢出
    uint64_t start_time = aos_now_ms();
    const uint32_t total_timeout_ms = 2000;

    while ((sent_total < data_len || recv_total < data_len) &&
           (aos_now_ms() - start_time) < total_timeout_ms) {

        // 发送一个小块
        if (sent_total < data_len) {
            uint32_t to_send = data_len - sent_total;
            if (to_send > chunk_size) {
                to_send = chunk_size;
            }

            int32_t sent_now = csi_uart_send(&uart_handle,
                                            test_string + sent_total,
                                            to_send, 200);
            if (sent_now > 0) {
                sent_total += sent_now;
                printf("Sent %u bytes (total: %u/%u)\n", sent_now, sent_total, data_len);

                // 给硬件一点时间处理这个小块的回环
                // 计算传输时间：字节数 * 10位 / 波特率 * 1000ms + 裕量
                uint32_t tx_time_ms = (sent_now * 10 * 1000) / baudrate + 2;
                aos_msleep(tx_time_ms);
            }
        }

        // 及时读取已经回环的数据
        while (recv_total < sent_total) {
            int32_t to_recv = sent_total - recv_total;
            int32_t recv_now = csi_uart_receive(&uart_handle,
                                              rx_buffer + recv_total,
                                              to_recv, 100);
            if (recv_now > 0) {
                recv_total += recv_now;
                printf("Received %u bytes (total: %u/%u)\n", recv_now, recv_total, data_len);
            } else {
                // 本次没有数据可读，跳出内层循环
                break;
            }
        }
    }

    // 4. 检查结果
    if (recv_total != data_len) {
        printf("Test FAILED: Expected %u, got %u bytes.\n", data_len, recv_total);
        if (recv_total > 0) {
            rx_buffer[recv_total] = '\0';
            printf("Received data: '%s'\n", rx_buffer);
        }
        goto cleanup;
    }

    // 5. 校验数据
    if (memcmp(test_string, rx_buffer, data_len) != 0) {
        rx_buffer[data_len] = '\0';
        printf("Test FAILED: Data mismatch!\n");
        printf("  Expected: '%s'\n", test_string);
        printf("  Received: '%s'\n", rx_buffer);
    } else {
        printf("Test PASSED: Data received correctly.\n");
    }

cleanup:
    csi_uart_uninit(&uart_handle);
}
ALIOS_CLI_CMD_REGISTER(test_uart_sync_loop_func, testuart_sync_loop, uart sync loopback test);

/******************************************************************************
 * 以下是 UART 异步回环测试功能
 ******************************************************************************/

// 异步模式下的回调标志
static volatile uint8_t uart_async_tx_done = 0;
static volatile uint8_t uart_async_rx_done = 0;

/**
 * @brief UART 异步事件回调函数
 */
static void uart_async_event_callback(csi_uart_t *uart_handle, csi_uart_event_t event, void *arg)
{
    switch (event) {
        case UART_EVENT_SEND_COMPLETE:
            uart_async_tx_done = 1;
            printf("Async TX complete\n");
            break;
        case UART_EVENT_RECEIVE_COMPLETE:
            uart_async_rx_done = 1;
            printf("Async RX complete\n");
            break;
        case UART_EVENT_ERROR_OVERFLOW:
        case UART_EVENT_ERROR_PARITY:
        case UART_EVENT_ERROR_FRAMING:
            printf("UART Error event: %d\n", event);
            break;
        default:
            printf("Unknown UART event: %d\n", event);
            break;
    }
}

/**
 * @brief 打印异步回环测试的用法说明
 */
static void print_async_loop_usage(void)
{
    printf("Usage:\n");
    printf("  testuart_async_loop <idx> <baudrate>\n\n");
    printf("Description:\n");
    printf("  Performs an asynchronous UART loopback test. TX and RX pins must be connected.\n\n");
    printf("Arguments:\n");
    printf("  idx      : UART port index (e.g., 0, 1, 2...).\n");
    printf("  baudrate : The baud rate to use (e.g., 115200, 576000, 921600).\n\n");
    printf("Example:\n");
    printf("  testuart_async_loop 2 115200\n");
}

/**
 * @brief UART异步回环测试主函数
 */
void test_uart_async_loop_func(int32_t argc, char **argv)
{
    if (argc < 3) {
        print_async_loop_usage();
        return;
    }

    int32_t uart_idx = atoi(argv[1]);
    uint32_t baudrate = atoi(argv[2]);

    printf("Starting ASYNC UART loopback test on UART%d, Baudrate: %u\n", uart_idx, baudrate);
    printf("Please ensure TX and RX pins of UART%d are connected.\n", uart_idx);

    csi_uart_t uart_handle;
    const char *test_string = "Async UART Loopback Test: Hello World! 1234567890";
    uint32_t data_len = strlen(test_string);
    uint8_t rx_buffer[128] = {0};
    int32_t ret;

    // 1. 初始化UART
    if (csi_uart_init(&uart_handle, uart_idx) != CSI_OK) {
        printf("Error: csi_uart_init failed for UART%d.\n", uart_idx);
        return;
    }

    // 2. 配置波特率和数据格式
    ret = csi_uart_baud(&uart_handle, baudrate);
    if (ret != CSI_OK) {
        printf("Error: csi_uart_baud failed to set %u.\n", baudrate);
        goto cleanup;
    }
    csi_uart_format(&uart_handle, UART_DATA_BITS_8, UART_PARITY_NONE, UART_STOP_BITS_1);

    // 3. 注册异步回调函数
    ret = csi_uart_attach_callback(&uart_handle, uart_async_event_callback, NULL);
    if (ret != CSI_OK) {
        printf("Error: csi_uart_attach_callback failed.\n");
        goto cleanup;
    }

    // 4. 重置完成标志
    uart_async_tx_done = 0;
    uart_async_rx_done = 0;

    printf("Starting async send/receive of '%s' (%u bytes)...\n", test_string, data_len);

    // 5. 启动异步接收
    ret = csi_uart_receive_async(&uart_handle, rx_buffer, data_len);
    if (ret != CSI_OK) {
        printf("Error: csi_uart_receive_async failed, ret = %d\n", ret);
        goto cleanup;
    }

    // 增加延时，确保接收端完全准备好
    aos_msleep(10);

    // 6. 启动异步发送
    ret = csi_uart_send_async(&uart_handle, test_string, data_len);
    if (ret != CSI_OK) {
        printf("Error: csi_uart_send_async failed, ret = %d\n", ret);
        goto cleanup;
    }

    // 7. 等待传输完成
    uint32_t timeout_ms = 3000;
    uint64_t start_time = aos_now_ms();

    while ((uart_async_tx_done == 0 || uart_async_rx_done == 0) &&
           (aos_now_ms() - start_time) < timeout_ms) {
        aos_msleep(20);
    }

    // 8. 检查结果
    if (aos_now_ms() - start_time >= timeout_ms) {
        printf("Test FAILED: Async transfer timeout!\n");
        printf("  - TX done flag: %d\n", uart_async_tx_done);
        printf("  - RX done flag: %d\n", uart_async_rx_done);
        goto cleanup;
    }

    // 9. 验证数据完整性
    if (memcmp(test_string, rx_buffer, data_len) != 0) {
        rx_buffer[data_len] = '\0';
        printf("Test FAILED: Data mismatch!\n");
        printf("  Expected: '%s'\n", test_string);
        printf("  Received: '%s'\n", rx_buffer);
    } else {
        printf("Test PASSED: Async loopback completed successfully!\n");
        printf("  Data integrity verified: %u bytes\n", data_len);
    }

cleanup:
    csi_uart_detach_callback(&uart_handle);
    csi_uart_uninit(&uart_handle);
}

ALIOS_CLI_CMD_REGISTER(test_uart_async_loop_func, testuart_async_loop, uart async loopback test);

/******************************************************************************
 * 以下是 UART DMA 测试功能
 ******************************************************************************/

#define TEST_UART_DMA_BUFFER_SIZE 128

// DMA 模式下的回调标志
static volatile uint8_t uart_dma_tx_done = 0;
static volatile uint8_t uart_dma_rx_done = 0;

/**
 * @brief UART DMA事件回调函数
 */
static void uart_dma_event_callback(csi_uart_t *uart_handle, csi_uart_event_t event, void *arg)
{
    printf("UART DMA event callback, handle: %p, event: %d, arg: %p\n", uart_handle, event, arg);
    switch (event) {
        case UART_EVENT_SEND_COMPLETE:
            uart_dma_tx_done = 1;
            printf("DMA TX complete\n");
            break;
        case UART_EVENT_RECEIVE_COMPLETE:
            uart_dma_rx_done = 1;
            printf("DMA RX complete\n");
            break;
        default:
            printf("Unknown UART event: %d\n", event);
            break;
    }
}

/**
 * @brief 打印DMA测试的用法说明
 */
static void print_dma_usage(void)
{
    printf("Usage:\n");
    printf("  testuart_dma <idx> <baudrate>\n\n");
    printf("Description:\n");
    printf("  Performs a UART DMA loopback test. TX and RX pins must be connected.\n\n");
    printf("Arguments:\n");
    printf("  idx      : UART port index (e.g., 0, 1, 2...).\n");
    printf("  baudrate : The baud rate to use (e.g., 115200, 576000, 921600).\n\n");
    printf("Example:\n");
    printf("  testuart_dma 2 115200\n");
    printf("  testuart_dma 2 1500000\n");
}

/**
 * @brief UART DMA回环测试主函数
 */
void test_uart_dma_func(int32_t argc, char **argv)
{
    if (argc < 3) {
        print_dma_usage();
        return;
    }

    int32_t uart_idx = atoi(argv[1]);
    uint32_t baudrate = atoi(argv[2]);
    int32_t ret;

    printf("Starting UART DMA loopback test on UART%d, Baudrate: %u\n", uart_idx, baudrate);
    printf("Please ensure TX and RX pins of UART%d are connected.\n", uart_idx);

    // 根据 UART 索引确定对应的 DMA 通道
    int tx_dma_ch = -1, rx_dma_ch = -1;
    if (uart_idx == 2) {
        // 根据 sysdma_remap.c 中的配置, devices.c中的uart_hs_num数组也随sysdma_remap.c配置
        tx_dma_ch = 1;  // CVI_UART2_TX 映射到通道 1
        rx_dma_ch = 0;  // CVI_UART2_RX 映射到通道 0
    } else {
        printf("Error: UART%d DMA mapping not defined. Only UART2 is supported.\n", uart_idx);
        return;
    }

    csi_uart_t uart_handle;
    csi_dma_ch_t tx_dma, rx_dma;
    const char *test_string = "UART DMA Test: Hello World! 1234567890";
    uint32_t data_len = strlen(test_string);
    uint8_t tx_buffer[TEST_UART_DMA_BUFFER_SIZE];
    uint8_t rx_buffer[TEST_UART_DMA_BUFFER_SIZE];

    if (data_len >= TEST_UART_DMA_BUFFER_SIZE) {
        printf("Error: Test string too long for buffer\n");
        return;
    }

    // 准备测试数据
    memcpy(tx_buffer, test_string, data_len);
    memset(rx_buffer, 0, TEST_UART_DMA_BUFFER_SIZE);

    // 1. 初始化UART
    ret = csi_uart_init(&uart_handle, uart_idx);
    if (ret != CSI_OK) {
        printf("Error: csi_uart_init failed for UART%d.\n", uart_idx);
        return;
    }

    // 2. 配置波特率和数据格式
    ret = csi_uart_baud(&uart_handle, baudrate);
    if (ret != CSI_OK) {
        printf("Error: csi_uart_baud failed to set %u.\n", baudrate);
        goto cleanup_uart;
    }
    ret = csi_uart_format(&uart_handle, UART_DATA_BITS_8, UART_PARITY_NONE, UART_STOP_BITS_1);


    // 3. 初始化指定的DMA通道
    // 指定具体的 DMA 控制器和通道
    ret = csi_dma_ch_alloc(&tx_dma, tx_dma_ch, 0);  // 0 是 DMA 控制器索引
    if (ret != CSI_OK) {
        printf("Error: Failed to allocate TX DMA channel %d.\n", tx_dma_ch);
        goto cleanup_uart;
    }

    ret = csi_dma_ch_alloc(&rx_dma, rx_dma_ch, 0);  // 0 是 DMA 控制器索引
    if (ret != CSI_OK) {
        printf("Error: Failed to allocate RX DMA channel %d.\n", rx_dma_ch);
    }

    // 4. 注册DMA回调函数
    ret = csi_uart_attach_callback(&uart_handle, uart_dma_event_callback, NULL);
    if (ret != CSI_OK) {
        printf("Error: csi_uart_attach_callback failed.\n");
    }

    // 5. 链接DMA通道到UART
    ret = csi_uart_link_dma(&uart_handle, &tx_dma, &rx_dma);
    if (ret != CSI_OK) {
        printf("Error: csi_uart_link_dma failed.\n");
        printf("Note: DMA channel mapping may be incorrect.\n");
        goto cleanup_uart;
    }

    // 6. 重置完成标志
    uart_dma_tx_done = 0;
    uart_dma_rx_done = 0;

    // 7. 启动DMA异步传输
    ret = csi_uart_receive_async(&uart_handle, rx_buffer, data_len);
    if (ret != CSI_OK) {
        printf("Error: csi_uart_receive_async failed.\n");
        goto cleanup_uart;
    }

    // 增加延时，确保接收端完全准备好
    aos_msleep(10);

    ret = csi_uart_send_async(&uart_handle, tx_buffer, data_len);
    if (ret != CSI_OK) {
        printf("Error: csi_uart_send_async failed.\n");
        goto cleanup_uart;
    }

    // 8. 等待DMA传输完成
    uint32_t timeout_ms = 3000;
    uint64_t start_time = aos_now_ms();
    uint64_t last_dump_time = 0;

    while ((uart_dma_tx_done == 0 || uart_dma_rx_done == 0) &&
           (aos_now_ms() - start_time) < timeout_ms) {

        // 在TX完成但RX未完成时，开始轮询硬件寄存器状态
        if (uart_dma_tx_done == 1 && uart_dma_rx_done == 0) {
            if (aos_now_ms() - last_dump_time > 1000) { // 每1000ms打印一次
                printf("  - Polling RX status (TX is complete):\n");
                csi_uart_dump_registers(&uart_handle);
                last_dump_time = aos_now_ms();
            }
        }

        aos_msleep(20);
    }

    // 9. 检查传输结果
    if (aos_now_ms() - start_time >= timeout_ms) {
        printf("\nTest FAILED: DMA transfer timeout!\n");
        printf("  - TX done flag: %d\n", uart_dma_tx_done);
        printf("  - RX done flag: %d\n", uart_dma_rx_done);
        printf("  - UART handle state (if available):\n");
        printf("    - uart_handle.dev.reg_base: 0x%lx\n", uart_handle.dev.reg_base);
        printf("    - uart_handle.dev.irq_num: %u\n", uart_handle.dev.irq_num);
        printf("    - uart_handle.tx_dma: %p, uart_handle.rx_dma: %p\n", uart_handle.tx_dma, uart_handle.rx_dma);
        goto cleanup_uart;
    }

    // 10. 验证数据完整性
    if (memcmp(tx_buffer, rx_buffer, data_len) != 0) {
        printf("Test FAILED: Data mismatch!\n");
        rx_buffer[data_len] = '\0';
        printf("  Expected: '%s'\n", test_string);
        printf("  Received: '%s'\n", rx_buffer);
    } else {
        printf("Test PASSED: DMA transfer completed successfully!\n");
        printf("  Data integrity verified: %u bytes\n", data_len);
    }

cleanup_uart:
    printf("Cleaning up: Uninitializing UART...\n");
    csi_uart_uninit(&uart_handle);
}

// 注册UART DMA测试命令
ALIOS_CLI_CMD_REGISTER(test_uart_dma_func, testuart_dma, uart DMA loopback test);

/******************************************************************************
 * 以下是 UART 异步 DMA 回环测试功能
 ******************************************************************************/

#define TEST_UART_ASYNC_DMA_BUFFER_SIZE 256

// 异步 DMA 模式下的回调标志
static volatile uint8_t uart_async_dma_tx_done = 0;
static volatile uint8_t uart_async_dma_rx_done = 0;
static volatile uint32_t uart_async_dma_rx_count = 0;

// 使用静态全局 buffer 并对齐，避免 stack 上的 cache 一致性问题
static uint8_t tx_async_dma_buffer[TEST_UART_ASYNC_DMA_BUFFER_SIZE] __attribute__((aligned(64)));
static uint8_t rx_async_dma_buffer[TEST_UART_ASYNC_DMA_BUFFER_SIZE] __attribute__((aligned(64)));


/**
 * @brief UART 异步 DMA 事件回调函数
 */
static void uart_async_dma_event_callback(csi_uart_t *uart_handle, csi_uart_event_t event, void *arg)
{
    switch (event) {
        case UART_EVENT_SEND_COMPLETE:
            uart_async_dma_tx_done = 1;
            printf("[Async DMA] TX complete\n");
            break;
        case UART_EVENT_RECEIVE_COMPLETE:
            uart_async_dma_rx_done = 1;
            printf("[Async DMA] RX complete\n");
            break;
        case UART_EVENT_ERROR_OVERFLOW:
            printf("[Async DMA] Error: RX overflow\n");
            break;
        case UART_EVENT_ERROR_PARITY:
            printf("[Async DMA] Error: Parity error\n");
            break;
        case UART_EVENT_ERROR_FRAMING:
            printf("[Async DMA] Error: Framing error\n");
            break;
        default:
            printf("[Async DMA] Unknown event: %d\n", event);
            break;
    }
}

/**
 * @brief 打印异步 DMA 测试的用法说明
 */
static void print_async_dma_usage(void)
{
    printf("Usage:\n");
    printf("  testuart_async_dma <idx> <baudrate> [size]\n\n");
    printf("Description:\n");
    printf("  Performs an asynchronous UART DMA loopback test. TX and RX pins must be connected.\n\n");
    printf("Arguments:\n");
    printf("  idx      : UART port index (e.g., 0, 1, 2...).\n");
    printf("  baudrate : The baud rate to use (e.g., 115200, 921600, 1500000).\n");
    printf("  size     : (Optional) Data size to test (default: 64, max: 256).\n\n");
    printf("Example:\n");
    printf("  testuart_async_dma 2 115200\n");
    printf("  testuart_async_dma 2 1500000 128\n");
}

/**
 * @brief UART 异步 DMA 回环测试主函数
 */
void test_uart_async_dma_func(int32_t argc, char **argv)
{
    if (argc < 3) {
        print_async_dma_usage();
        return;
    }

    int32_t uart_idx = atoi(argv[1]);
    uint32_t baudrate = atoi(argv[2]);
    uint32_t test_size = 64; // 默认测试 64 字节
    int32_t ret;

    if (argc >= 4) {
        test_size = atoi(argv[3]);
        if (test_size > TEST_UART_ASYNC_DMA_BUFFER_SIZE) {
            printf("Warning: Size too large, using max %d bytes.\n", TEST_UART_ASYNC_DMA_BUFFER_SIZE);
            test_size = TEST_UART_ASYNC_DMA_BUFFER_SIZE;
        }
    }

    printf("Starting ASYNC DMA UART loopback test on UART%d\n", uart_idx);
    printf("  Baudrate: %u, Test size: %u bytes\n", baudrate, test_size);
    printf("Please ensure TX and RX pins of UART%d are connected.\n", uart_idx);

    // 目前仅配置 UART2 的 DMA 功能
    if (uart_idx != 2) {
        printf("Error: UART%d DMA mapping not defined. Only UART2 is supported.\n", uart_idx);
        return;
    }

    csi_uart_t uart_handle;
    csi_dma_ch_t tx_dma, rx_dma;
    // 使用静态对齐的 buffer
    uint8_t *tx_buffer = tx_async_dma_buffer;
    uint8_t *rx_buffer = rx_async_dma_buffer;

    // 准备测试数据 - 生成递增的测试模式
    for (uint32_t i = 0; i < test_size; i++) {
        tx_buffer[i] = (uint8_t)(i & 0xFF);
    }
    // 显式刷 cache，确保数据写入 RAM
    csi_dcache_clean_range((uintptr_t *)tx_buffer, TEST_UART_ASYNC_DMA_BUFFER_SIZE);

    memset(rx_buffer, 0, TEST_UART_ASYNC_DMA_BUFFER_SIZE);
    // 显式 invalidate cache，确保后续读取从 RAM 拉取
    csi_dcache_invalid_range((uintptr_t *)rx_buffer, TEST_UART_ASYNC_DMA_BUFFER_SIZE);

    // 1. 初始化 UART
    ret = csi_uart_init(&uart_handle, uart_idx);
    if (ret != CSI_OK) {
        printf("Error: csi_uart_init failed for UART%d.\n", uart_idx);
        return;
    }

    // 2. 配置波特率和数据格式
    ret = csi_uart_baud(&uart_handle, baudrate);
    if (ret != CSI_OK) {
        printf("Error: csi_uart_baud failed to set %u.\n", baudrate);
        goto cleanup_uart;
    }
    csi_uart_format(&uart_handle, UART_DATA_BITS_8, UART_PARITY_NONE, UART_STOP_BITS_1);

    // 3. 注册异步 DMA 回调函数
    ret = csi_uart_attach_callback(&uart_handle, uart_async_dma_event_callback, NULL);
    if (ret != CSI_OK) {
        printf("Error: csi_uart_attach_callback failed.\n");
        goto cleanup_uart;
    }

    // 清空 RX FIFO 中可能存在的残留数据 (在 link DMA 之前执行)
    {
        uint8_t dummy[32];
        int32_t flushed = csi_uart_receive(&uart_handle, dummy, sizeof(dummy), 0);
        if (flushed > 0) {
            printf("Flushed %d bytes from RX FIFO before test\n", flushed);
        }
    }

    // 4. 链接 DMA 通道到 UART
    // 注意: csi_uart_link_dma 内部会调用 csi_dma_ch_alloc，不需要预先分配
    ret = csi_uart_link_dma(&uart_handle, &tx_dma, &rx_dma);
    if (ret != CSI_OK) {
        printf("Error: csi_uart_link_dma failed.\n");
        goto cleanup_callback;
    }

    // 5. 重置完成标志
    uart_async_dma_tx_done = 0;
    uart_async_dma_rx_done = 0;

    printf("Starting async DMA transfer...\n");

    // 7. 先启动异步 DMA 接收
    ret = csi_uart_receive_async(&uart_handle, rx_buffer, test_size);
    if (ret != CSI_OK) {
        printf("Error: csi_uart_receive_async failed, ret = %d\n", ret);
        goto cleanup_dma;
    }

    // 等待接收端 DMA 完全准备就绪
    aos_msleep(10);

    // 8. 启动异步 DMA 发送
    ret = csi_uart_send_async(&uart_handle, tx_buffer, test_size);
    if (ret != CSI_OK) {
        printf("Error: csi_uart_send_async failed, ret = %d\n", ret);
        goto cleanup_dma;
    }

    // 9. 等待传输完成
    uint32_t timeout_ms = 5000; // 5秒超时
    uint64_t start_time = aos_now_ms();

    while ((uart_async_dma_tx_done == 0 || uart_async_dma_rx_done == 0) &&
           (aos_now_ms() - start_time) < timeout_ms) {
        aos_msleep(10);
    }

    // 10. 检查传输结果
    uint64_t elapsed_time = aos_now_ms() - start_time;

    if (uart_async_dma_tx_done == 0 || uart_async_dma_rx_done == 0) {
        printf("\nTest FAILED: Async DMA transfer timeout after %lu ms!\n", (unsigned long)elapsed_time);
        printf("  - TX done flag: %d\n", uart_async_dma_tx_done);
        printf("  - RX done flag: %d\n", uart_async_dma_rx_done);
        csi_uart_dump_registers(&uart_handle);
        goto cleanup_dma;
    }

    printf("Transfer completed in %lu ms\n", (unsigned long)elapsed_time);

    // 再次 invalidate cache，确保读取到 DMA 写入的最新数据
    csi_dcache_invalid_range((uintptr_t *)rx_buffer, TEST_UART_ASYNC_DMA_BUFFER_SIZE);

    // 10. 验证数据完整性
    uint32_t mismatch_count = 0;
    uint32_t first_mismatch_idx = 0;

    for (uint32_t i = 0; i < test_size; i++) {
        if (tx_buffer[i] != rx_buffer[i]) {
            if (mismatch_count == 0) {
                first_mismatch_idx = i;
            }
            mismatch_count++;
        }
    }

    if (mismatch_count > 0) {
        printf("Test FAILED: Data mismatch!\n");
        printf("  - Total mismatches: %u of %u bytes\n", mismatch_count, test_size);
        printf("  - First mismatch at index %u: expected 0x%02X, got 0x%02X\n",
               first_mismatch_idx, tx_buffer[first_mismatch_idx], rx_buffer[first_mismatch_idx]);

        // 打印前16字节用于调试
        printf("  - TX (first 16): ");
        for (uint32_t i = 0; i < 16 && i < test_size; i++) {
            printf("%02X ", tx_buffer[i]);
        }
        printf("\n  - RX (first 16): ");
        for (uint32_t i = 0; i < 16 && i < test_size; i++) {
            printf("%02X ", rx_buffer[i]);
        }
        printf("\n");
    } else {
        printf("Test PASSED: Async DMA loopback completed successfully!\n");
        printf("  - Data integrity verified: %u bytes\n", test_size);
        if (elapsed_time > 0) {
            printf("  - Effective throughput: %.2f KB/s\n",
                   (float)(test_size * 1000) / (float)(elapsed_time * 1024));
        }
    }

cleanup_dma:
    // 解除 DMA 链接并释放 DMA 通道
    // 传入 NULL 会触发 csi_uart_link_dma 内部释放已分配的 DMA 通道
    csi_uart_link_dma(&uart_handle, NULL, NULL);

cleanup_callback:
    csi_uart_detach_callback(&uart_handle);

cleanup_uart:
    printf("Cleaning up: Uninitializing UART...\n");
    csi_uart_uninit(&uart_handle);
}

// 注册 UART 异步 DMA 测试命令
ALIOS_CLI_CMD_REGISTER(test_uart_async_dma_func, testuart_async_dma, uart async DMA loopback test);

/**
 * @brief UART 异步 DMA 发送测试主函数
 */
void test_uart_async_dma_tx_func(int32_t argc, char **argv)
{
    if (argc < 3) {
        printf("Usage:\n");
        printf("  testuart_async_tx <idx> <baudrate> <string>\n");
        printf("  testuart_async_tx 2 115200 \"hello dma\"\n");
        return;
    }

    int32_t uart_idx = atoi(argv[1]);
    uint32_t baudrate = atoi(argv[2]);
    char *data = argv[3];
    uint32_t test_size = strlen(data);

    if (test_size > TEST_UART_ASYNC_DMA_BUFFER_SIZE)
        test_size = TEST_UART_ASYNC_DMA_BUFFER_SIZE;

    if (uart_idx != 2) {
        printf("Default: Only UART2 is supported for DMA.\n");
        return;
    }

    csi_uart_t uart_handle;
    csi_dma_ch_t tx_dma;
    int32_t ret;

    // 1. 准备数据并刷 Cache
    memcpy(tx_async_dma_buffer, data, test_size);
    csi_dcache_clean_range((uintptr_t *)tx_async_dma_buffer, TEST_UART_ASYNC_DMA_BUFFER_SIZE);

    // 2. 初始化 UART & 注册回调
    csi_uart_init(&uart_handle, uart_idx);
    csi_uart_baud(&uart_handle, baudrate);
    csi_uart_format(&uart_handle, UART_DATA_BITS_8, UART_PARITY_NONE, UART_STOP_BITS_1);
    csi_uart_attach_callback(&uart_handle, uart_async_dma_event_callback, NULL);

    // 3. 链接 TX DMA
    ret = csi_uart_link_dma(&uart_handle, &tx_dma, NULL);
    if (ret != CSI_OK) {
        printf("Link TX DMA failed\n");
        goto cleanup;
    }

    uart_async_dma_tx_done = 0;
    printf("[TX Test] Sending %u bytes...\n", test_size);

    // 4. 异步发送
    csi_uart_send_async(&uart_handle, tx_async_dma_buffer, test_size);

    // 5. 等待完成
    uint32_t timeout = 2000;
    while (uart_async_dma_tx_done == 0 && timeout--) aos_msleep(1);

    if (uart_async_dma_tx_done)
        printf("[TX Test] Success.\n");
    else
        printf("[TX Test] Timeout!\n");

cleanup:
    csi_uart_link_dma(&uart_handle, NULL, NULL);
    csi_uart_uninit(&uart_handle);
}
ALIOS_CLI_CMD_REGISTER(test_uart_async_dma_tx_func, testuart_async_tx, uart async DMA send test);

/**
 * @brief UART 异步 DMA 接收测试主函数 (最终修复版)
 */
void test_uart_async_dma_rx_func(int32_t argc, char **argv)
{
    if (argc < 3) {
        printf("Usage:\n");
        printf("  testuart_async_rx <idx> <baudrate> [size]\n");
        printf("  testuart_async_rx 2 115200 5\n");
        return;
    }

    int32_t uart_idx = atoi(argv[1]);
    uint32_t baudrate = atoi(argv[2]);
    uint32_t test_size = (argc >= 4) ? atoi(argv[3]) : 16;

    if (test_size > TEST_UART_ASYNC_DMA_BUFFER_SIZE)
        test_size = TEST_UART_ASYNC_DMA_BUFFER_SIZE;

    if (uart_idx != 2) {
        printf("Only UART2 is supported for DMA.\n");
        return;
    }

    csi_uart_t uart_handle;
    csi_dma_ch_t rx_dma;
    int32_t ret;

    // 1. 初始化 Buffer 和 Cache
    memset(rx_async_dma_buffer, 0, TEST_UART_ASYNC_DMA_BUFFER_SIZE);
    csi_dcache_invalid_range((uintptr_t *)rx_async_dma_buffer, TEST_UART_ASYNC_DMA_BUFFER_SIZE);

    // 2. 初始化 UART
    csi_uart_init(&uart_handle, uart_idx);
    csi_uart_baud(&uart_handle, baudrate);
    csi_uart_format(&uart_handle, UART_DATA_BITS_8, UART_PARITY_NONE, UART_STOP_BITS_1);

    // 发送一个 0xFF (全1)，这会将 TX 保持在高电平，模拟 Idle 状态，
    // 强制 UART 状态机进入活跃状态并稳定波特率时钟。
    uint8_t dummy_tx = 0xFF;
    printf("[RX Init] Warming up UART hardware...\n");
    csi_uart_send(&uart_handle, &dummy_tx, 1, 100);

    // 延时一小会儿，确保发送完成且硬件状态稳定
    aos_msleep(20);

    // 3. 注册回调
    csi_uart_attach_callback(&uart_handle, uart_async_dma_event_callback, NULL);

    // 4. 清洗 FIFO
    uint8_t dummy_buf[32];
    while (csi_uart_receive(&uart_handle, dummy_buf, sizeof(dummy_buf), 0) > 0);

    // 5. 链接 RX DMA
    // 注意：即使我们刚才用了同步发送，这里链接 DMA 后，之前的普通模式会自动切换
    if (csi_uart_link_dma(&uart_handle, NULL, &rx_dma) != CSI_OK) {
        printf("Link RX DMA failed\n");
        goto cleanup;
    }

    uart_async_dma_rx_done = 0;
    printf("[RX Test] Waiting for %u bytes from UART%d...\n", test_size, uart_idx);

    // 6. 异步接收启动
    ret = csi_uart_receive_async(&uart_handle, rx_async_dma_buffer, test_size);
    if (ret != CSI_OK) {
        printf("Error: Receive async failed ret=%d\n", ret);
        goto cleanup;
    }

    // 7. 等待接收完成
    uint32_t wait_count = 2000; // 20秒超时，给你足够时间在 PC 端点发送
    while (uart_async_dma_rx_done == 0 && wait_count--) {
        aos_msleep(10);
    }

    if (uart_async_dma_rx_done) {
        // 再次 invalidate cache，确保 CPU 读到 DMA 写入的数据
        csi_dcache_invalid_range((uintptr_t *)rx_async_dma_buffer, TEST_UART_ASYNC_DMA_BUFFER_SIZE);

        printf("[RX Test] Received hex: ");
        for (uint32_t i = 0; i < test_size; i++)
            printf("%02X ", rx_async_dma_buffer[i]);
        printf("\n");
    } else {
        printf("[RX Test] Timeout! Is the baudrate correct? (%d)\n", baudrate);
    }

cleanup:
    csi_uart_link_dma(&uart_handle, NULL, NULL);
    csi_uart_uninit(&uart_handle);
}
ALIOS_CLI_CMD_REGISTER(test_uart_async_dma_rx_func, testuart_async_rx, uart async DMA receive test);

/******************************************************************************
 * 以下是新增的 UART 接收中断异步处理测试功能
 ******************************************************************************/

#define FIFO_TEST_BUFFER_SIZE 64
static uint8_t fifo_rx_buffer[FIFO_TEST_BUFFER_SIZE];
static volatile uint32_t fifo_rx_len = 0;
static csi_uart_t *g_fifo_uart_handle = NULL; // 全局句柄，用于在回调中操作

/**
 * @brief 处理 FIFO_READABLE 事件的回调函数
 */
static void fifo_readable_callback(csi_uart_t *uart, csi_uart_event_t event, void *arg)
{
    if (event == UART_EVENT_RECEIVE_FIFO_READABLE) {
        // 从FIFO中读取所有可用数据，直到读空为止
        int32_t ret = csi_uart_receive(uart, fifo_rx_buffer + fifo_rx_len, FIFO_TEST_BUFFER_SIZE - fifo_rx_len, 0);
        if (ret > 0) {
            fifo_rx_len += ret;
        }
    }
}

/**
 * @brief 测试 FIFO_READABLE 中断模式的主函数
 */
void test_uart_fifo_readable_func(int32_t argc, char **argv)
{
    if (argc < 3) {
        printf("Usage: testuart_fifo_readable <idx> <baudrate>\n");
        return;
    }

    int32_t uart_idx = atoi(argv[1]);
    uint32_t baudrate = atoi(argv[2]);

    printf("Starting UART FIFO_READABLE interrupt test on UART%d, Baudrate: %u\n", uart_idx, baudrate);
    printf("Please send some data from PC to UART%d. Type 'exit' to stop.\n", uart_idx);

    csi_uart_t uart_handle;
    g_fifo_uart_handle = &uart_handle; // 赋值给全局句柄

    // 1. 初始化并配置UART
    if (csi_uart_init(&uart_handle, uart_idx) != CSI_OK) {
        printf("Error: csi_uart_init failed\n");
        return;
    }
    csi_uart_baud(&uart_handle, baudrate);
    csi_uart_format(&uart_handle, UART_DATA_BITS_8, UART_PARITY_NONE, UART_STOP_BITS_1);

    // 2. 注册回调
    if (csi_uart_attach_callback(&uart_handle, fifo_readable_callback, NULL) != CSI_OK) {
        printf("Error: Failed to attach callback\n");
        goto cleanup;
    }

    // 3. 清空缓冲区并开始监听
    fifo_rx_len = 0;
    memset(fifo_rx_buffer, 0, FIFO_TEST_BUFFER_SIZE);
    printf("Listening for data...\n");

    // 4. 循环检查接收到的数据
    while (1) {
        if (fifo_rx_len > 0) {
            // 打印接收到的数据
            printf("Received %u bytes: ", fifo_rx_len);
            for (uint32_t i = 0; i < fifo_rx_len; i++) {
                printf("%c", fifo_rx_buffer[i]);
            }
            printf("\n");

            // 检查是否收到退出命令
            if (strstr((const char *)fifo_rx_buffer, "exit") != NULL) {
                printf("'exit' command received. Exiting test.\n");
                break;
            }

            // 重置缓冲区
            fifo_rx_len = 0;
            memset(fifo_rx_buffer, 0, FIFO_TEST_BUFFER_SIZE);
        }
        aos_msleep(500); // 每500ms检查一次
    }

cleanup:
    csi_uart_detach_callback(&uart_handle);
    csi_uart_uninit(&uart_handle);
    g_fifo_uart_handle = NULL;
}
ALIOS_CLI_CMD_REGISTER(test_uart_fifo_readable_func, testuart_fifo_readable, test UART_EVENT_RECEIVE_FIFO_READABLE);
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
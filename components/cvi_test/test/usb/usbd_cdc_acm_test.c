/*
 * USB CDC ACM (virtual serial port) test application.
 *
 * "ACM" = Abstract Control Model, the CDC subclass that emulates a serial port.
 * On Linux the device appears as /dev/ttyACM0; on Windows as a COMx port.
 *
 * CLI commands
 * ─────────────────────────────────────────────────────────────────────────────
 *  cdcuart_init                    open usb_serial0, start background RX thread
 *  cdcuart_uninit                  stop RX thread, close device, print stats
 *  cdcuart_send        [str]       send a plain text string once
 *  cdcuart_send_burst  [n] [s] [t] send n packets of s bytes with timeout t ms;
 *                                  each packet has a 2-byte sequence header so
 *                                  the host can detect drops/reordering
 *  cdcuart_send_pattern [n]        send n × 256-byte packets with 0x00..0xFF
 *                                  pattern; host can verify byte integrity
 *  cdcuart_loopback    [secs]      echo every received byte back to the host
 *                                  for <secs> seconds
 * ─────────────────────────────────────────────────────────────────────────────
 *
 * Typical test sequence
 *   1. Connect USB cable; host should enumerate a virtual serial port.
 *   2. cdcuart_init                    →   open the USB CDC UART device and start the background RX
 *                                          thread
 *   3. cdcuart_send "hello"            →   send a single text string to the host over CDC UART
 *   4. cdcuart_send_pattern 10         →   send N fixed 256-byte packets with an incrementing byte
 *                                          pattern
 *   5. cdcuart_send_burst 100 512 10   →   send N packets of S bytes back-to-back to stress-test
 *                                          the CDC UART with a timeout of T ms
 *   6. cdcuart_loopback 10             →   enable RX-to-TX loopback mode for a fixed duration of 10
 *                                          seconds
 *   7. cdcuart_uninit                  →   stop the RX thread, close the CDC UART device, and print
 *                                          statistics
 */

#include <aos/aos.h>
#include <aos/cli.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include "devices/impl/uart_impl.h"

/* ── tunables ──────────────────────────────────────────────────────────────── */
#define CDC_UART_DEV_NAME  "usb_serial0"
#define RX_BUF_SIZE        512  /* bytes read per recv call                   */
#define DEFAULT_TIMEOUT_MS 100  /* per-packet TX timeout (ms)                 */
#define BURST_DEFAULT_N    50   /* default packet count for burst test        */
#define BURST_DEFAULT_SZ   256  /* default packet size for burst test (bytes) */
#define BURST_MAX_SZ       4096 /* maximum allowed burst packet size          */
#define PATTERN_PKT_SIZE   256  /* fixed packet size for pattern test         */
#define LOOPBACK_DEFAULT_S 10   /* default loopback duration (seconds)        */

/* ── context ───────────────────────────────────────────────────────────────── */
typedef struct {
    rvm_dev_t* dev;
    pthread_t rx_tid;
    volatile int run_rx;        /* 1 = RX thread running                      */
    volatile int loopback_mode; /* 1 = echo received bytes back to host       */
    uint64_t rx_total;          /* cumulative bytes received                  */
    uint64_t tx_total;          /* cumulative bytes sent successfully         */
    uint32_t tx_errors;         /* send failures / timeouts                   */
} cdc_test_ctx_t;

static cdc_test_ctx_t g_ctx;

/**
 * @brief  Check whether the CDC UART device is open.
 * @note   Prints an error hint if the device handle is NULL.
 * @retval 1  device is open and ready
 * @retval 0  device is not open
 */
static int is_open(void)
{
    if (!g_ctx.dev) {
        printf("[cdcuart] device not open – run cdcuart_init first\n");
        return 0;
    }
    return 1;
}

/**
 * @brief  Send data to the host over USB CDC UART.
 * @note   Updates g_ctx.tx_total on success and g_ctx.tx_errors on failure.
 *         Prints distinct messages for ETIMEDOUT vs other errors so EP
 *         recovery activity is easy to identify in the log.
 * @param  data:       pointer to the data buffer to send
 * @param  size:       number of bytes to send
 * @param  timeout_ms: per-send timeout in milliseconds
 * @retval  0  success
 * @retval -1  send failed (timeout or driver error)
 */
static int cdc_send(const void* data, uint32_t size, uint32_t timeout_ms)
{
    int ret = rvm_hal_uart_send(g_ctx.dev, data, size, timeout_ms);
    if (ret < 0) {
        g_ctx.tx_errors++;
        if (ret == -ETIMEDOUT) {
            printf("[cdcuart] send TIMEOUT (%u bytes, timeout=%ums) errors=%u\n", size, timeout_ms,
                   g_ctx.tx_errors);
        } else {
            printf("[cdcuart] send ERROR ret=%d tx_errors=%u\n", ret, g_ctx.tx_errors);
        }
        return -1;
    }
    g_ctx.tx_total += size;
    return 0;
}

/**
 * @brief  Background RX thread – drains all data arriving from the host.
 * @note   Prints up to the first 32 received bytes in hex for readability.
 *         When g_ctx.loopback_mode is set, echoes every received byte back
 *         to the host immediately via cdc_send().
 *         The thread exits when g_ctx.run_rx is cleared by cmd_cdcuart_uninit().
 * @param  arg: unused (required by pthread_create signature)
 * @retval NULL always
 */
static void* rx_thread(void* arg)
{
    (void)arg;
    uint8_t buf[RX_BUF_SIZE];

    prctl(PR_SET_NAME, "cdcuart_rx");
    printf("[cdcuart] RX thread started\n");

    while (g_ctx.run_rx) {
        int n = rvm_hal_uart_recv(g_ctx.dev, buf, sizeof(buf), 300);
        if (n <= 0) {
            continue;
        }

        g_ctx.rx_total += n;
        printf("[cdcuart] recv %d bytes (total=%llu):", n, (unsigned long long)g_ctx.rx_total);
        for (int i = 0; i < n && i < 32; i++) {
            printf(" %02x", buf[i]);
        }
        if (n > 32) {
            printf(" ...");
        }
        printf("\n");

        if (g_ctx.loopback_mode) {
            cdc_send(buf, n, DEFAULT_TIMEOUT_MS);
        }
    }

    printf("[cdcuart] RX thread stopped (rx_total=%llu)\n", (unsigned long long)g_ctx.rx_total);
    return NULL;
}

/**
 * @brief  Open the USB CDC UART device and start the background RX thread.
 * @note   Must be called before any send/recv/loopback commands.
 *         Safe to call only once; prints a warning if already initialised.
 * @param  argc: argument count (unused)
 * @param  argv: argument vector (unused)
 */
static void cmd_cdcuart_init(int argc, char** argv)
{
    if (g_ctx.dev) {
        printf("[cdcuart] already initialised\n");
        return;
    }

    memset(&g_ctx, 0, sizeof(g_ctx));

    g_ctx.dev = rvm_hal_device_open(CDC_UART_DEV_NAME);
    if (!g_ctx.dev) {
        printf("[cdcuart] failed to open %s\n", CDC_UART_DEV_NAME);
        return;
    }

    g_ctx.run_rx = 1;
    if (pthread_create(&g_ctx.rx_tid, NULL, rx_thread, NULL) != 0) {
        printf("[cdcuart] failed to create RX thread\n");
        rvm_hal_device_close(g_ctx.dev);
        g_ctx.dev = NULL;
        return;
    }

    printf("[cdcuart] init OK (dev=%s)\n", CDC_UART_DEV_NAME);
}
ALIOS_CLI_CMD_REGISTER(cmd_cdcuart_init, cdcuart_init, init USB CDC UART and start RX thread);

/**
 * @brief  Stop the RX thread, close the CDC UART device, and print statistics.
 * @note   Blocks until the RX thread exits (pthread_join).
 *         Prints cumulative tx_total, rx_total, and tx_errors on exit.
 * @param  argc: argument count (unused)
 * @param  argv: argument vector (unused)
 */
static void cmd_cdcuart_uninit(int argc, char** argv)
{
    if (!g_ctx.dev) {
        printf("[cdcuart] not initialised\n");
        return;
    }

    g_ctx.run_rx        = 0;
    g_ctx.loopback_mode = 0;
    pthread_join(g_ctx.rx_tid, NULL);

    rvm_hal_device_close(g_ctx.dev);
    g_ctx.dev = NULL;

    printf("[cdcuart] uninit OK (tx_total=%llu rx_total=%llu tx_errors=%u)\n",
           (unsigned long long)g_ctx.tx_total, (unsigned long long)g_ctx.rx_total, g_ctx.tx_errors);
}
ALIOS_CLI_CMD_REGISTER(cmd_cdcuart_uninit, cdcuart_uninit, stop RX thread and close CDC UART);

/**
 * @brief  Send a single text string to the host over CDC UART.
 * @note   A newline is appended automatically.
 *         Defaults to "hello from CDC UART" when no argument is given.
 *         The string is silently truncated to 253 characters if longer.
 * @param  argc: argument count
 * @param  argv: argv[1] – text string to send (optional)
 */
static void cmd_cdcuart_send(int argc, char** argv)
{
    if (!is_open())
        return;

    const char* msg = (argc >= 2) ? argv[1] : "hello from CDC UART\n";
    uint32_t len    = strlen(msg);
    char buf[256];

    if (len >= sizeof(buf) - 2) {
        len = sizeof(buf) - 2;
    }
    memcpy(buf, msg, len);
    buf[len]     = '\n';
    buf[len + 1] = '\0';
    len++;

    printf("[cdcuart] sending %u bytes: \"%.*s\"\n", len, (int)(len - 1), buf);

    if (cdc_send(buf, len, DEFAULT_TIMEOUT_MS) == 0) {
        printf("[cdcuart] send OK (tx_total=%llu)\n", (unsigned long long)g_ctx.tx_total);
    }
}
ALIOS_CLI_CMD_REGISTER(cmd_cdcuart_send, cdcuart_send, send a text string over CDC UART);

/**
 * @brief  Send N packets of S bytes back-to-back to stress-test the CDC UART.
 * @note   Packet layout: [seq_lo][seq_hi][0xAA][0x55][incremental payload...]
 *         The 2-byte little-endian sequence number allows the host to detect
 *         dropped or reordered packets caused by EP errors.
 *         A 50 ms pause is inserted after each failed send so the EP recovery
 *         logic (close+reopen in the driver) has time to complete.
 *         Prints throughput in kbps on completion.
 * @param  argc: argument count
 * @param  argv: argv[1] – packet count   (default: BURST_DEFAULT_N)
 *               argv[2] – packet size    (default: BURST_DEFAULT_SZ, range: 4..BURST_MAX_SZ)
 *               argv[3] – timeout per packet in ms (default: DEFAULT_TIMEOUT_MS)
 */
static void cmd_cdcuart_send_burst(int argc, char** argv)
{
    if (!is_open())
        return;

    int n          = (argc >= 2) ? atoi(argv[1]) : BURST_DEFAULT_N;
    int sz         = (argc >= 3) ? atoi(argv[2]) : BURST_DEFAULT_SZ;
    int timeout_ms = (argc >= 4) ? atoi(argv[3]) : DEFAULT_TIMEOUT_MS;
    int errs       = 0;

    if (n <= 0 || n > 100000) {
        printf("[cdcuart] count out of range (1..100000)\n");
        return;
    }
    if (sz < 4 || sz > BURST_MAX_SZ) {
        printf("[cdcuart] size out of range (4..%d)\n", BURST_MAX_SZ);
        return;
    }

    uint8_t* buf = (uint8_t*)aos_malloc(sz);
    if (!buf) {
        printf("[cdcuart] malloc failed\n");
        return;
    }

    printf("[cdcuart] burst: %d packets x %d bytes (timeout per pkt=%ums)\n", n, sz, timeout_ms);

    long long t0 = aos_now_ms();

    for (int i = 0; i < n; i++) {
        buf[0] = (uint8_t)(i & 0xFF);        /* sequence number low byte  */
        buf[1] = (uint8_t)((i >> 8) & 0xFF); /* sequence number high byte */
        buf[2] = 0xAA;                       /* start marker              */
        buf[3] = 0x55;                       /* start marker              */
        for (int j = 4; j < sz; j++) {
            buf[j] = (uint8_t)((i + j) & 0xFF);
        }

        if (cdc_send(buf, sz, timeout_ms) != 0) {
            errs++;
            /* give EP recovery (close+reopen in driver) time to finish */
            aos_msleep(50);
        }
    }

    long long elapsed = aos_now_ms() - t0;
    uint64_t bytes    = (uint64_t)(n - errs) * sz;
    uint32_t kbps     = (elapsed > 0) ? (uint32_t)(bytes * 8 / elapsed) : 0;

    printf(
        "[cdcuart] burst done: sent=%d errors=%d elapsed=%llums "
        "throughput~%u kbps (tx_total=%llu)\n",
        n - errs, errs, elapsed, kbps, (unsigned long long)g_ctx.tx_total);

    aos_free(buf);
}
ALIOS_CLI_CMD_REGISTER(cmd_cdcuart_send_burst, cdcuart_send_burst,
                       send N packets of S bytes back - to - back);

/**
 * @brief  Send N fixed 256-byte packets with an incrementing byte pattern.
 * @note   Every packet contains exactly bytes 0x00, 0x01, ..., 0xFF in order.
 *         The host can verify each packet byte-by-byte using the Python snippet
 *         in the file header to confirm end-to-end data integrity.
 *         A 50 ms pause is inserted after each failed send to allow EP recovery.
 * @param  argc: argument count
 * @param  argv: argv[1] – number of packets to send (default: 10, range: 1..100000)
 */
static void cmd_cdcuart_send_pattern(int argc, char** argv)
{
    if (!is_open())
        return;

    int n    = (argc >= 2) ? atoi(argv[1]) : 10;
    int errs = 0;

    if (n <= 0 || n > 100000) {
        printf("[cdcuart] count out of range (1..100000)\n");
        return;
    }

    uint8_t buf[PATTERN_PKT_SIZE];
    for (int i = 0; i < PATTERN_PKT_SIZE; i++) {
        buf[i] = (uint8_t)i;
    }

    printf("[cdcuart] pattern: %d x %d-byte packets (0x00..0xFF each)\n", n, PATTERN_PKT_SIZE);

    long long t0 = aos_now_ms();

    for (int i = 0; i < n; i++) {
        if (cdc_send(buf, PATTERN_PKT_SIZE, DEFAULT_TIMEOUT_MS) != 0) {
            errs++;
            /* give EP recovery (close+reopen in driver) time to finish */
            aos_msleep(50);
        }
    }

    long long elapsed = aos_now_ms() - t0;
    printf(
        "[cdcuart] pattern done: sent=%d errors=%d elapsed=%llums "
        "(tx_total=%llu)\n",
        n - errs, errs, elapsed, (unsigned long long)g_ctx.tx_total);
}
ALIOS_CLI_CMD_REGISTER(cmd_cdcuart_send_pattern, cdcuart_send_pattern,
                       send N x 256 - byte incrementing - pattern packets);

/**
 * @brief  Enable RX-to-TX loopback mode for a fixed duration.
 * @note   While active, the RX thread echoes every byte received from the host
 *         straight back over TX.  Type on the host terminal during this window;
 *         all input must appear echoed with no missing or garbled bytes,
 *         confirming both RX and TX paths are working correctly.
 *         Prints rx_total, tx_total, and tx_errors when the duration expires.
 * @param  argc: argument count
 * @param  argv: argv[1] – loopback duration in seconds (default: LOOPBACK_DEFAULT_S,
 * range: 1..3600)
 */
static void cmd_cdcuart_loopback(int argc, char** argv)
{
    if (!is_open())
        return;

    int secs = (argc >= 2) ? atoi(argv[1]) : LOOPBACK_DEFAULT_S;
    if (secs <= 0 || secs > 3600) {
        printf("[cdcuart] duration out of range (1..3600)\n");
        return;
    }

    printf("[cdcuart] loopback ON for %d seconds – type on the host terminal\n", secs);

    g_ctx.loopback_mode = 1;
    long long t0        = aos_now_ms();
    while (aos_now_ms() - t0 < (long long)secs * 1000) {
        aos_msleep(200);
    }
    g_ctx.loopback_mode = 0;

    printf("[cdcuart] loopback OFF (rx_total=%llu tx_total=%llu tx_errors=%u)\n",
           (unsigned long long)g_ctx.rx_total, (unsigned long long)g_ctx.tx_total, g_ctx.tx_errors);
}
ALIOS_CLI_CMD_REGISTER(cmd_cdcuart_loopback, cdcuart_loopback,
                       echo all received bytes back to host for N seconds);

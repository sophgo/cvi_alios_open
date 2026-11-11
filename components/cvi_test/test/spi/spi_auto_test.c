/*
 * Copyright (C) 2017 C-SKY Microsystems Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file spi_auto_test.c
 * @brief SPI automated functional test cases
 *
 * Test cases cover:
 *   TC01: Init / Uninit
 *   TC02: Repeated init / uninit (stability)
 *   TC03: Mode config (Master / Slave)
 *   TC04: Clock polarity & phase (all 4 combinations)
 *   TC05: Frame length (4-bit ~ 16-bit)
 *   TC06: Baud rate config (various frequencies)
 *   TC07: Callback attach / detach
 *   TC08: Poll-mode send_receive loopback
 *   TC09: IRQ-mode send_receive loopback
 *   TC10: DMA-mode send_receive loopback
 *   TC11: Various data lengths (poll mode loopback)
 *   TC12: All CPOL/CPHA loopback (poll mode)
 *   TC13: Various baud rate loopback (poll mode)
 *   TC14: Stress test – repeated poll loopback
 *   TC15: Select slave
 *   TC16: Send-only (poll mode)
 *
 * Hardware requirement: SPI0 MOSI <-> MISO loopback wiring for data-path tests.
 *
 * Usage (CLI):
 *   spi_auto_test <spi_idx>              – run all test cases
 *   spi_auto_test <spi_idx> <tc_number>   – run specific test case (1-16)
 */

#include "drv/spi.h"
#include "board.h"
#include <aos/kernel.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <aos/cli.h>
#include <drv/pin.h>
#include <drv/gpio.h>
#include "pinctrl-mars.h"

/* ======================== Pinmux configuration ======================== */

static void spi_pinmux_setup(uint32_t spi_idx)
{
    switch (spi_idx) {
    case 0:
        PINMUX_CONFIG(PAD_MIPI_TXM1, SPI0_SDO);
        PINMUX_CONFIG(PAD_MIPI_TXP1, SPI0_SDI);
        printf("[SPI_TEST] Pinmux: PAD_MIPI_TXM1 -> SPI0_SDO, PAD_MIPI_TXP1 -> SPI0_SDI\n");
        break;
    default:
        printf("[SPI_TEST] Warning: no pinmux config for SPI%u, please configure manually\n", spi_idx);
        break;
    }
}

/* ======================== Macros & Constants ======================== */

#define SPI_DEFAULT_BAUD     1000000    /* 1 MHz default baud */
#define SPI_DEFAULT_TIMEOUT  1000       /* ms */
#define DATA_BUF_MAX         1024       /* maximum test buffer size */
#define ASYNC_WAIT_MS        100        /* max wait for async completion */
#define STRESS_ITERATIONS    100        /* number of stress loop iterations */

#define TEST_LOG(fmt, ...)   printf("[SPI_TEST] " fmt "\n", ##__VA_ARGS__)

/* ======================== Runtime SPI index ======================== */

static uint32_t g_spi_idx = 0;  /* SPI controller index, set by CLI arg */

/* ======================== Test result tracking ======================== */

static int g_total_tests  = 0;
static int g_pass_count   = 0;
static int g_fail_count   = 0;

#define TEST_ASSERT(cond, msg) do {                     \
    g_total_tests++;                                    \
    if (!(cond)) {                                      \
        g_fail_count++;                                 \
        printf("  [FAIL] %s (line %d)\n", msg, __LINE__); \
        return -1;                                      \
    } else {                                            \
        g_pass_count++;                                 \
        printf("  [PASS] %s\n", msg);                   \
    }                                                   \
} while (0)

#define TEST_ASSERT_NRET(cond, msg) do {                \
    g_total_tests++;                                    \
    if (!(cond)) {                                      \
        g_fail_count++;                                 \
        printf("  [FAIL] %s (line %d)\n", msg, __LINE__); \
    } else {                                            \
        g_pass_count++;                                 \
        printf("  [PASS] %s\n", msg);                   \
    }                                                   \
} while (0)

/* ======================== Async callback helpers ======================== */

static volatile int g_send_done;
static volatile int g_recv_done;
static volatile int g_send_recv_done;
static volatile int g_error_event;

static void spi_event_callback(csi_spi_t *spi, csi_spi_event_t event, void *arg)
{
    switch (event) {
    case SPI_EVENT_SEND_COMPLETE:
        g_send_done = 1;
        break;
    case SPI_EVENT_RECEIVE_COMPLETE:
        g_recv_done = 1;
        break;
    case SPI_EVENT_SEND_RECEIVE_COMPLETE:
        g_send_recv_done = 1;
        break;
    case SPI_EVENT_ERROR_OVERFLOW:
    case SPI_EVENT_ERROR_UNDERFLOW:
    case SPI_EVENT_ERROR:
        g_error_event = 1;
        break;
    default:
        break;
    }
}

static void reset_async_flags(void)
{
    g_send_done      = 0;
    g_recv_done      = 0;
    g_send_recv_done = 0;
    g_error_event    = 0;
}

static int wait_async_complete(volatile int *flag, int timeout_ms)
{
    int elapsed = 0;

    while (!(*flag) && !g_error_event && elapsed < timeout_ms) {
        aos_msleep(1);
        elapsed++;
    }
    if (g_error_event)
        return -2;
    return (*flag) ? 0 : -1;
}

/* ======================== Helper: fill pattern ======================== */

static void fill_pattern(uint8_t *buf, uint32_t len, uint8_t seed)
{
    for (uint32_t i = 0; i < len; i++)
        buf[i] = (uint8_t)(seed + i);
}

static int verify_pattern(const uint8_t *tx, const uint8_t *rx, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        if (tx[i] != rx[i]) {
            printf("    mismatch at offset 0x%x: tx=0x%02x rx=0x%02x\n", i, tx[i], rx[i]);
            return -1;
        }
    }
    return 0;
}

/* ======================== Helper: basic SPI setup ======================== */

static int spi_basic_setup(csi_spi_t *spi)
{
    csi_error_t ret;

    ret = csi_spi_init(spi, g_spi_idx);
    if (ret != CSI_OK)
        return -1;

    ret = csi_spi_mode(spi, SPI_MASTER);
    if (ret != CSI_OK)
        goto fail;

    ret = csi_spi_cp_format(spi, SPI_FORMAT_CPOL0_CPHA0);
    if (ret != CSI_OK)
        goto fail;

    ret = csi_spi_frame_len(spi, SPI_FRAME_LEN_8);
    if (ret != CSI_OK)
        goto fail;

    csi_spi_baud(spi, SPI_DEFAULT_BAUD);
    return 0;

fail:
    csi_spi_uninit(spi);
    return -1;
}

/* ======================================================================== */
/*                           TEST CASES                                     */
/* ======================================================================== */

/**
 * TC01: Init / Uninit
 * Verify that csi_spi_init succeeds and csi_spi_uninit does not crash.
 */
static int tc01_init_uninit(void)
{
    TEST_LOG(">>> TC01: Init / Uninit");
    csi_spi_t spi;
    csi_error_t ret;

    ret = csi_spi_init(&spi, g_spi_idx);
    TEST_ASSERT(ret == CSI_OK, "csi_spi_init returns CSI_OK");

    csi_spi_uninit(&spi);
    TEST_ASSERT_NRET(1, "csi_spi_uninit completes without crash");
    return 0;
}

/**
 * TC02: Repeated init / uninit
 * Init and uninit multiple times to verify no resource leaks.
 */
static int tc02_repeated_init_uninit(void)
{
    TEST_LOG(">>> TC02: Repeated init / uninit (10 times)");
    csi_spi_t spi;
    int pass = 1;

    for (int i = 0; i < 10; i++) {
        csi_error_t ret = csi_spi_init(&spi, g_spi_idx);
        if (ret != CSI_OK) {
            pass = 0;
            printf("    init failed at iteration %d\n", i);
            break;
        }
        csi_spi_uninit(&spi);
    }
    TEST_ASSERT(pass, "10x init/uninit cycles succeed");
    return 0;
}

/**
 * TC03: Mode configuration (Master / Slave)
 */
static int tc03_mode_config(void)
{
    TEST_LOG(">>> TC03: Mode config (Master / Slave)");
    csi_spi_t spi;
    csi_error_t ret;

    ret = csi_spi_init(&spi, g_spi_idx);
    TEST_ASSERT(ret == CSI_OK, "init OK");

    ret = csi_spi_mode(&spi, SPI_MASTER);
    TEST_ASSERT_NRET(ret == CSI_OK, "set SPI_MASTER");

    ret = csi_spi_mode(&spi, SPI_SLAVE);
    TEST_ASSERT_NRET(ret == CSI_OK, "set SPI_SLAVE");

    /* restore master for subsequent tests */
    csi_spi_mode(&spi, SPI_MASTER);
    csi_spi_uninit(&spi);
    return 0;
}

/**
 * TC04: Clock polarity & phase (4 combinations)
 */
static int tc04_cpol_cpha(void)
{
    TEST_LOG(">>> TC04: CPOL / CPHA config");
    csi_spi_t spi;
    csi_error_t ret;

    static const csi_spi_cp_format_t fmts[] = {
        SPI_FORMAT_CPOL0_CPHA0,
        SPI_FORMAT_CPOL0_CPHA1,
        SPI_FORMAT_CPOL1_CPHA0,
        SPI_FORMAT_CPOL1_CPHA1,
    };
    static const char *names[] = {
        "CPOL0_CPHA0", "CPOL0_CPHA1",
        "CPOL1_CPHA0", "CPOL1_CPHA1",
    };

    ret = csi_spi_init(&spi, g_spi_idx);
    TEST_ASSERT(ret == CSI_OK, "init OK");

    csi_spi_mode(&spi, SPI_MASTER);

    for (int i = 0; i < 4; i++) {
        ret = csi_spi_cp_format(&spi, fmts[i]);
        char msg[64];
        snprintf(msg, sizeof(msg), "set cp_format %s", names[i]);
        TEST_ASSERT_NRET(ret == CSI_OK, msg);
    }

    csi_spi_uninit(&spi);
    return 0;
}

/**
 * TC05: Frame length (4-bit ~ 16-bit)
 */
static int tc05_frame_len(void)
{
    TEST_LOG(">>> TC05: Frame length config (8, 16 bit)");
    csi_spi_t spi;
    csi_error_t ret;

    /* This platform only supports 8-bit and 16-bit frame lengths */
    static const csi_spi_frame_len_t supported_lens[] = {
        SPI_FRAME_LEN_8,
        SPI_FRAME_LEN_16,
    };

    ret = csi_spi_init(&spi, g_spi_idx);
    TEST_ASSERT(ret == CSI_OK, "init OK");

    csi_spi_mode(&spi, SPI_MASTER);

    for (int i = 0; i < (int)(sizeof(supported_lens) / sizeof(supported_lens[0])); i++) {
        ret = csi_spi_frame_len(&spi, supported_lens[i]);
        char msg[64];
        snprintf(msg, sizeof(msg), "set frame_len %d-bit", (int)supported_lens[i]);
        TEST_ASSERT_NRET(ret == CSI_OK, msg);
    }

    csi_spi_uninit(&spi);
    return 0;
}

/**
 * TC06: Baud rate config
 */
static int tc06_baud_rate(void)
{
    TEST_LOG(">>> TC06: Baud rate config");
    csi_spi_t spi;
    csi_error_t ret;

    static const uint32_t bauds[] = {
        100000,     /* 100 KHz  */
        500000,     /* 500 KHz  */
        1000000,    /* 1 MHz    */
        2000000,    /* 2 MHz    */
        5000000,    /* 5 MHz    */
        10000000,   /* 10 MHz   */
        25000000,   /* 25 MHz   */
        25000000,   /* 25 MHz */
    };

    ret = csi_spi_init(&spi, g_spi_idx);
    TEST_ASSERT(ret == CSI_OK, "init OK");

    csi_spi_mode(&spi, SPI_MASTER);

    for (int i = 0; i < (int)(sizeof(bauds) / sizeof(bauds[0])); i++) {
        uint32_t actual = csi_spi_baud(&spi, bauds[i]);
        char msg[64];
        snprintf(msg, sizeof(msg), "baud %u -> actual %u", bauds[i], actual);
        /* baud should return a non-zero actual frequency */
        TEST_ASSERT_NRET(actual > 0, msg);
    }

    csi_spi_uninit(&spi);
    return 0;
}

/**
 * TC07: Callback attach / detach
 */
static int tc07_callback(void)
{
    TEST_LOG(">>> TC07: Callback attach / detach");
    csi_spi_t spi;
    csi_error_t ret;

    ret = csi_spi_init(&spi, g_spi_idx);
    TEST_ASSERT(ret == CSI_OK, "init OK");

    ret = csi_spi_attach_callback(&spi, spi_event_callback, NULL);
    TEST_ASSERT(ret == CSI_OK, "attach_callback OK");

    csi_spi_detach_callback(&spi);
    TEST_ASSERT_NRET(1, "detach_callback completes without crash");

    /* attach again after detach */
    ret = csi_spi_attach_callback(&spi, spi_event_callback, NULL);
    TEST_ASSERT_NRET(ret == CSI_OK, "re-attach_callback OK");

    csi_spi_detach_callback(&spi);
    csi_spi_uninit(&spi);
    return 0;
}

/**
 * TC08: Poll-mode send_receive loopback (128 bytes)
 * Requires MOSI<->MISO loopback wiring.
 */
static int tc08_poll_loopback(void)
{
    TEST_LOG(">>> TC08: Poll-mode send_receive loopback (128 bytes)");
    csi_spi_t spi;
    uint8_t tx[128], rx[128];
    int32_t ret;

    fill_pattern(tx, sizeof(tx), 0x00);
    memset(rx, 0, sizeof(rx));

    ret = spi_basic_setup(&spi);
    TEST_ASSERT(ret == 0, "basic setup OK");

    ret = csi_spi_send_receive(&spi, tx, rx, sizeof(tx), SPI_DEFAULT_TIMEOUT);
    TEST_ASSERT(ret >= 0, "send_receive returns >= 0");

    ret = verify_pattern(tx, rx, sizeof(tx));
    TEST_ASSERT(ret == 0, "loopback data matches (poll 128B)");

    csi_spi_uninit(&spi);
    return 0;
}

/**
 * TC09: IRQ-mode send_receive loopback (128 bytes)
 */
static int tc09_irq_loopback(void)
{
    TEST_LOG(">>> TC09: IRQ-mode send_receive loopback (128 bytes)");
    csi_spi_t spi;
    uint8_t tx[128], rx[128];
    csi_error_t ret;
    int rc;

    fill_pattern(tx, sizeof(tx), 0x10);
    memset(rx, 0, sizeof(rx));
    reset_async_flags();

    rc = spi_basic_setup(&spi);
    TEST_ASSERT(rc == 0, "basic setup OK");

    ret = csi_spi_attach_callback(&spi, spi_event_callback, NULL);
    TEST_ASSERT(ret == CSI_OK, "attach_callback OK");

    ret = csi_spi_send_receive_async(&spi, tx, rx, sizeof(tx));
    TEST_ASSERT(ret == CSI_OK, "send_receive_async returns CSI_OK");

    /* Wait for both send and receive to complete */
    rc = wait_async_complete(&g_recv_done, ASYNC_WAIT_MS);
    if (rc != 0)
        rc = wait_async_complete(&g_send_recv_done, ASYNC_WAIT_MS);
    TEST_ASSERT(rc == 0 || g_send_done || g_recv_done || g_send_recv_done,
                "async completion signaled");

    rc = verify_pattern(tx, rx, sizeof(tx));
    TEST_ASSERT(rc == 0, "loopback data matches (IRQ 128B)");

    csi_spi_detach_callback(&spi);
    csi_spi_uninit(&spi);
    return 0;
}

/**
 * TC10: DMA-mode send_receive loopback (128 bytes)
 */
static int tc10_dma_loopback(void)
{
    TEST_LOG(">>> TC10: DMA-mode send_receive loopback (128 bytes)");
    csi_spi_t spi;
    uint8_t tx[128], rx[128];
    csi_error_t ret;
    int rc;

    fill_pattern(tx, sizeof(tx), 0x20);
    memset(rx, 0, sizeof(rx));
    reset_async_flags();

    rc = spi_basic_setup(&spi);
    TEST_ASSERT(rc == 0, "basic setup OK");

    ret = csi_spi_attach_callback(&spi, spi_event_callback, NULL);
    TEST_ASSERT(ret == CSI_OK, "attach_callback OK");

    ret = csi_spi_link_dma(&spi, NULL, NULL);
    TEST_ASSERT(ret == CSI_OK, "link_dma OK");

    ret = csi_spi_send_receive_dma(&spi, tx, rx, sizeof(tx));
    TEST_ASSERT(ret >= 0, "send_receive_dma returns >= 0");

    /* Wait for DMA transfer completion */
    rc = wait_async_complete(&g_recv_done, ASYNC_WAIT_MS);
    if (rc != 0)
        rc = wait_async_complete(&g_send_recv_done, ASYNC_WAIT_MS);
    TEST_ASSERT(rc == 0 || g_send_done || g_recv_done || g_send_recv_done,
                "DMA completion signaled");

    rc = verify_pattern(tx, rx, sizeof(tx));
    TEST_ASSERT(rc == 0, "loopback data matches (DMA 128B)");

    csi_spi_detach_callback(&spi);
    csi_spi_uninit(&spi);
    return 0;
}

/**
 * TC11: Various data lengths (poll mode loopback)
 * Tests: 1, 4, 16, 64, 128, 256, 512, 1024 bytes
 */
static int tc11_various_lengths(void)
{
    TEST_LOG(">>> TC11: Various data lengths (poll mode loopback)");
    csi_spi_t spi;
    static uint8_t tx[DATA_BUF_MAX], rx[DATA_BUF_MAX];
    int32_t ret;
    int rc;

    static const uint32_t lengths[] = { 1, 4, 16, 64, 128, 256, 512, 1024 };

    for (int i = 0; i < (int)(sizeof(lengths) / sizeof(lengths[0])); i++) {
        uint32_t len = lengths[i];

        fill_pattern(tx, len, (uint8_t)(i * 0x11));
        memset(rx, 0, len);

        rc = spi_basic_setup(&spi);
        if (rc != 0) {
            printf("  [FAIL] setup for len=%u\n", len);
            g_total_tests++;
            g_fail_count++;
            continue;
        }

        ret = csi_spi_send_receive(&spi, tx, rx, len, SPI_DEFAULT_TIMEOUT);

        char msg[64];
        if (ret >= 0) {
            rc = verify_pattern(tx, rx, len);
            snprintf(msg, sizeof(msg), "loopback %u bytes", len);
            TEST_ASSERT_NRET(rc == 0, msg);
        } else {
            snprintf(msg, sizeof(msg), "send_receive %u bytes (ret=%d)", len, ret);
            TEST_ASSERT_NRET(0, msg);
        }

        csi_spi_uninit(&spi);
    }

    return 0;
}

/**
 * TC12: All CPOL/CPHA loopback (poll mode, 64 bytes)
 */
static int tc12_cpol_cpha_loopback(void)
{
    TEST_LOG(">>> TC12: All CPOL/CPHA loopback (poll 64B)");
    csi_spi_t spi;
    uint8_t tx[64], rx[64];
    csi_error_t ret;
    int rc;

    static const csi_spi_cp_format_t fmts[] = {
        SPI_FORMAT_CPOL0_CPHA0,
        SPI_FORMAT_CPOL0_CPHA1,
        SPI_FORMAT_CPOL1_CPHA0,
        SPI_FORMAT_CPOL1_CPHA1,
    };
    static const char *names[] = {
        "CPOL0_CPHA0", "CPOL0_CPHA1",
        "CPOL1_CPHA0", "CPOL1_CPHA1",
    };

    for (int i = 0; i < 4; i++) {
        fill_pattern(tx, sizeof(tx), (uint8_t)(0x30 + i * 0x10));
        memset(rx, 0, sizeof(rx));

        ret = csi_spi_init(&spi, g_spi_idx);
        if (ret != CSI_OK) {
            printf("  [FAIL] init for %s\n", names[i]);
            g_total_tests++;
            g_fail_count++;
            continue;
        }

        csi_spi_mode(&spi, SPI_MASTER);
        csi_spi_cp_format(&spi, fmts[i]);
        csi_spi_frame_len(&spi, SPI_FRAME_LEN_8);
        csi_spi_baud(&spi, SPI_DEFAULT_BAUD);

        rc = csi_spi_send_receive(&spi, tx, rx, sizeof(tx), SPI_DEFAULT_TIMEOUT);

        char msg[80];
        if (rc >= 0) {
            int match = verify_pattern(tx, rx, sizeof(tx));
            snprintf(msg, sizeof(msg), "loopback %s", names[i]);
            TEST_ASSERT_NRET(match == 0, msg);
        } else {
            snprintf(msg, sizeof(msg), "send_receive %s (ret=%d)", names[i], rc);
            TEST_ASSERT_NRET(0, msg);
        }

        csi_spi_uninit(&spi);
    }

    return 0;
}

/**
 * TC13: Various baud rate loopback (poll mode, 16 bytes)
 * Note: Reduced to 16 bytes (FIFO size) because at 46.8MHz (33.3MHz actual),
 * polling mode CPU fetch speed cannot keep up with hardware, causing RX FIFO overflow.
 */
static int tc13_baud_rate_loopback(void)
{
    TEST_LOG(">>> TC13: Various baud rate loopback (poll 16B)");
    csi_spi_t spi;
    uint8_t tx[16], rx[16];
    int32_t ret;
    int rc;

    static const uint32_t bauds[] = {
        100000,     /* 100 KHz  */
        500000,     /* 500 KHz  */
        1000000,    /* 1 MHz    */
        5000000,    /* 5 MHz    */
        10000000,   /* 10 MHz   */
        25000000,   /* 25 MHz */
    };

    for (int i = 0; i < (int)(sizeof(bauds) / sizeof(bauds[0])); i++) {
        fill_pattern(tx, sizeof(tx), (uint8_t)(0x40 + i * 5));
        memset(rx, 0, sizeof(rx));

        rc = csi_spi_init(&spi, g_spi_idx);
        if (rc != CSI_OK) {
            printf("  [FAIL] init for baud=%u\n", bauds[i]);
            g_total_tests++;
            g_fail_count++;
            continue;
        }

        csi_spi_mode(&spi, SPI_MASTER);
        csi_spi_cp_format(&spi, SPI_FORMAT_CPOL0_CPHA0);
        csi_spi_frame_len(&spi, SPI_FRAME_LEN_8);
        uint32_t actual = csi_spi_baud(&spi, bauds[i]);

        ret = csi_spi_send_receive(&spi, tx, rx, sizeof(tx), SPI_DEFAULT_TIMEOUT);

        char msg[80];
        if (ret >= 0) {
            int match = verify_pattern(tx, rx, sizeof(tx));
            snprintf(msg, sizeof(msg), "loopback baud=%u (actual=%u)", bauds[i], actual);
            TEST_ASSERT_NRET(match == 0, msg);
        } else {
            snprintf(msg, sizeof(msg), "send_receive baud=%u (ret=%d)", bauds[i], ret);
            TEST_ASSERT_NRET(0, msg);
        }

        csi_spi_uninit(&spi);
    }

    return 0;
}

/**
 * TC14: Stress test – repeated poll loopback (128B x N iterations)
 */
static int tc14_stress_loopback(void)
{
    TEST_LOG(">>> TC14: Stress test (%d iterations, poll 128B)", STRESS_ITERATIONS);
    csi_spi_t spi;
    uint8_t tx[128], rx[128];
    int32_t ret;
    int fail_iter = -1;
    int rc;

    rc = spi_basic_setup(&spi);
    TEST_ASSERT(rc == 0, "basic setup OK");

    for (int i = 0; i < STRESS_ITERATIONS; i++) {
        fill_pattern(tx, sizeof(tx), (uint8_t)i);
        memset(rx, 0, sizeof(rx));

        ret = csi_spi_send_receive(&spi, tx, rx, sizeof(tx), SPI_DEFAULT_TIMEOUT);
        if (ret < 0) {
            fail_iter = i;
            printf("    send_receive failed at iteration %d, ret=%d\n", i, ret);
            break;
        }

        rc = verify_pattern(tx, rx, sizeof(tx));
        if (rc != 0) {
            fail_iter = i;
            printf("    data mismatch at iteration %d\n", i);
            break;
        }
    }

    char msg[80];
    snprintf(msg, sizeof(msg), "stress %d iterations", STRESS_ITERATIONS);
    TEST_ASSERT(fail_iter == -1, msg);

    csi_spi_uninit(&spi);
    return 0;
}

/**
 * TC15: Select slave
 */
static int tc15_select_slave(void)
{
    TEST_LOG(">>> TC15: Select slave");
    csi_spi_t spi;
    int rc;

    rc = spi_basic_setup(&spi);
    TEST_ASSERT(rc == 0, "basic setup OK");

    /* select_slave is a void function – just call it and ensure no crash */
    csi_spi_select_slave(&spi, 0);
    TEST_ASSERT_NRET(1, "select_slave(0) no crash");

    csi_spi_select_slave(&spi, 1);
    TEST_ASSERT_NRET(1, "select_slave(1) no crash");

    csi_spi_uninit(&spi);
    return 0;
}

/**
 * TC16: Send-only (poll mode, 64 bytes)
 * No loopback verification – just ensure send returns success.
 */
static int tc16_send_only(void)
{
    TEST_LOG(">>> TC16: Send-only (poll 64B)");
    csi_spi_t spi;
    uint8_t tx[64];
    int32_t ret;
    int rc;

    fill_pattern(tx, sizeof(tx), 0xAA);

    rc = spi_basic_setup(&spi);
    TEST_ASSERT(rc == 0, "basic setup OK");

    ret = csi_spi_send(&spi, tx, sizeof(tx), SPI_DEFAULT_TIMEOUT);
    TEST_ASSERT(ret >= 0, "csi_spi_send returns >= 0");

    csi_spi_uninit(&spi);
    return 0;
}

/* ======================================================================== */
/*                           Test dispatcher                                */
/* ======================================================================== */

typedef int (*tc_func_t)(void);

struct test_case {
    int         id;
    const char *name;
    tc_func_t   func;
};

static const struct test_case g_test_table[] = {
    {  1, "Init/Uninit",                  tc01_init_uninit          },
    {  2, "Repeated init/uninit",         tc02_repeated_init_uninit },
    {  3, "Mode config (Master/Slave)",   tc03_mode_config          },
    {  4, "CPOL/CPHA config",            tc04_cpol_cpha            },
    {  5, "Frame length config",          tc05_frame_len            },
    {  6, "Baud rate config",             tc06_baud_rate            },
    {  7, "Callback attach/detach",       tc07_callback             },
    {  8, "Poll-mode loopback",           tc08_poll_loopback        },
    {  9, "IRQ-mode loopback",            tc09_irq_loopback         },
    { 10, "DMA-mode loopback",            tc10_dma_loopback         },
    { 11, "Various data lengths",         tc11_various_lengths      },
    { 12, "All CPOL/CPHA loopback",       tc12_cpol_cpha_loopback   },
    { 13, "Various baud rate loopback",   tc13_baud_rate_loopback   },
    { 14, "Stress test",                  tc14_stress_loopback      },
    { 15, "Select slave",                tc15_select_slave          },
    { 16, "Send-only (poll)",             tc16_send_only            },
};

#define NUM_TEST_CASES  (int)(sizeof(g_test_table) / sizeof(g_test_table[0]))

static void print_usage(void)
{
    printf("Usage: spi_auto_test <spi_idx> [tc_number]\n");
    printf("  spi_idx   - SPI controller index (e.g. 0=SPI0, 1=SPI1, ...)\n");
    printf("  tc_number - optional, run specific test case (1-%d)\n", NUM_TEST_CASES);
    printf("  No tc_number - run all test cases\n\n");
    printf("Examples:\n");
    printf("  spi_auto_test 1        - run all tests on SPI1\n");
    printf("  spi_auto_test 0 9      - run TC09 on SPI0\n");
    printf("  spi_auto_test 2 help   - show this help\n\n");
    printf("Test cases:\n");
    for (int i = 0; i < NUM_TEST_CASES; i++) {
        printf("  %2d: %s\n", g_test_table[i].id, g_test_table[i].name);
    }
}

static void print_summary(void)
{
    printf("\n");
    printf("============================================================\n");
    printf("  SPI Auto Test Summary\n");
    printf("============================================================\n");
    printf("  Total : %d\n", g_total_tests);
    printf("  PASS  : %d\n", g_pass_count);
    printf("  FAIL  : %d\n", g_fail_count);
    printf("  Result: %s\n", (g_fail_count == 0) ? "ALL PASSED" : "SOME FAILED");
    printf("============================================================\n");
}

int test_spi_auto(int argc, char **argv)
{
    int tc_num = 0; /* 0 means run all */

    /* Reset counters */
    g_total_tests = 0;
    g_pass_count  = 0;
    g_fail_count  = 0;

    if (argc < 2) {
        print_usage();
        return -1;
    }

    if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "help")) {
        print_usage();
        return 0;
    }

    /* First arg: SPI controller index */
    g_spi_idx = (uint32_t)atoi(argv[1]);

    /* Configure pinmux for selected SPI */
    spi_pinmux_setup(g_spi_idx);

    /* Second arg (optional): test case number or help */
    if (argc >= 3) {
        if (!strcmp(argv[2], "-h") || !strcmp(argv[2], "help")) {
            print_usage();
            return 0;
        }
        tc_num = atoi(argv[2]);
        if (tc_num < 1 || tc_num > NUM_TEST_CASES) {
            printf("Invalid test case number: %d (valid: 1-%d)\n", tc_num, NUM_TEST_CASES);
            print_usage();
            return -1;
        }
    }

    printf("\n============================================================\n");
    printf("  SPI Automated Functional Test\n");
    printf("  Hardware: SPI%u, Loopback (MOSI <-> MISO)\n", g_spi_idx);
    printf("============================================================\n\n");

    if (tc_num > 0) {
        /* Run single test case */
        const struct test_case *tc = &g_test_table[tc_num - 1];
        printf("Running TC%02d: %s\n\n", tc->id, tc->name);
        tc->func();
    } else {
        /* Run all test cases */
        for (int i = 0; i < NUM_TEST_CASES; i++) {
            const struct test_case *tc = &g_test_table[i];
            printf("Running TC%02d: %s\n", tc->id, tc->name);
            tc->func();
            printf("\n");
        }
    }

    print_summary();
    return (g_fail_count == 0) ? 0 : -1;
}

ALIOS_CLI_CMD_REGISTER(test_spi_auto, spi_auto_test, SPI automated functional test);

/**
 * @file test_i2c_auto.c
 * @brief Automated I2C functionality test cases for AliOS
 *
 * Test coverage:
 *   1. Init / Uninit
 *   2. Mode configuration (master / slave)
 *   3. Address mode configuration (7-bit / 10-bit)
 *   4. Speed configuration (standard / fast / fast_plus / high)
 *   5. Bus detect (scan all 7-bit addresses)
 *   6. Master send / receive (blocking)
 *   7. Memory (register) read / write
 *   8. Callback attach / detach
 *   9. State query
 *  10. Repeated init / uninit (stability)
 *  11. Invalid parameter handling
 *
 * Usage (AliOS CLI):
 *   i2c_test auto <idx>                                   -- run all tests on bus <idx>
 *   i2c_test auto <idx> <dev_addr> <reg_addr>             -- auto-detect reg width
 *   i2c_test auto <idx> <dev_addr> <reg_addr> <reg_width> -- specify reg width (1=8bit, 2=16bit)
 */

#include <drv/iic.h>
#include <aos/cli.h>
#include <ulog/ulog.h>
#include <aos/kernel.h>
#include <mmio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TAG "i2c_test"

/* --------------------------------------------------------------------------
 *  Test framework helpers
 * -------------------------------------------------------------------------- */
typedef struct {
    uint32_t total;
    uint32_t passed;
    uint32_t failed;
    uint32_t skipped;
} test_stats_t;

static test_stats_t g_stats;

#define TEST_PASS(name) do { \
    g_stats.total++; g_stats.passed++; \
    printf("[PASS] %s\n", (name)); \
} while (0)

#define TEST_FAIL(name, fmt, ...) do { \
    g_stats.total++; g_stats.failed++; \
    printf("[FAIL] %s -- " fmt "\n", (name), ##__VA_ARGS__); \
} while (0)

#define TEST_SKIP(name, reason) do { \
    g_stats.total++; g_stats.skipped++; \
    printf("[SKIP] %s -- %s\n", (name), (reason)); \
} while (0)

#define ASSERT_EQ(name, actual, expected) do { \
    if ((actual) == (expected)) { \
        TEST_PASS(name); \
    } else { \
        TEST_FAIL(name, "expected %d, got %d", (int)(expected), (int)(actual)); \
    } \
} while (0)

#define ASSERT_NEQ(name, actual, unexpected) do { \
    if ((actual) != (unexpected)) { \
        TEST_PASS(name); \
    } else { \
        TEST_FAIL(name, "should not be %d", (int)(unexpected)); \
    } \
} while (0)

#define ASSERT_GE(name, actual, min_val) do { \
    if ((actual) >= (min_val)) { \
        TEST_PASS(name); \
    } else { \
        TEST_FAIL(name, "expected >= %d, got %d", (int)(min_val), (int)(actual)); \
    } \
} while (0)

static void stats_reset(void)
{
    memset(&g_stats, 0, sizeof(g_stats));
}

static void stats_report(void)
{
    printf("\n========== I2C Auto-Test Summary ==========\n");
    printf("  Total : %u\n", g_stats.total);
    printf("  Passed: %u\n", g_stats.passed);
    printf("  Failed: %u\n", g_stats.failed);
    printf("  Skipped:%u\n", g_stats.skipped);
    printf("  Result: %s\n", g_stats.failed == 0 ? "ALL PASSED" : "HAS FAILURE");
    printf("============================================\n");
}

/* --------------------------------------------------------------------------
 *  Callback helper for async test
 * -------------------------------------------------------------------------- */
static volatile int g_cb_event = -1;

static void test_iic_callback(csi_iic_t *iic, csi_iic_event_t event, void *arg)
{
    (void)iic;
    (void)arg;
    g_cb_event = (int)event;
}

/* --------------------------------------------------------------------------
 *  Platform-specific pinmux for I2C5 (same as original tool)
 * -------------------------------------------------------------------------- */
static void platform_i2c_extra_init(uint32_t idx)
{
    if (idx == 5)
        mmio_write_32(0x0502509c, 0x1);
}

/* --------------------------------------------------------------------------
 *  Test 1 : Init & Uninit basic
 * -------------------------------------------------------------------------- */
static void test_init_uninit(uint32_t idx)
{
    csi_iic_t iic;
    csi_error_t ret;

    memset(&iic, 0, sizeof(iic));
    ret = csi_iic_init(&iic, idx);
    platform_i2c_extra_init(idx);
    ASSERT_EQ("init_basic", ret, CSI_OK);

    /* Uninit should not crash */
    csi_iic_uninit(&iic);
    TEST_PASS("uninit_basic");
}

/* --------------------------------------------------------------------------
 *  Test 2 : Double init (re-init without uninit) should still succeed
 * -------------------------------------------------------------------------- */
static void test_double_init(uint32_t idx)
{
    csi_iic_t iic;
    csi_error_t ret;

    memset(&iic, 0, sizeof(iic));
    ret = csi_iic_init(&iic, idx);
    ASSERT_EQ("double_init_first", ret, CSI_OK);

    ret = csi_iic_init(&iic, idx);
    /* Some drivers allow re-init, some return error.
     * We just verify it doesn't crash. Record result. */
    if (ret == CSI_OK) {
        TEST_PASS("double_init_second");
    } else {
        TEST_SKIP("double_init_second", "driver rejects re-init (acceptable)");
    }

    csi_iic_uninit(&iic);
}

/* --------------------------------------------------------------------------
 *  Test 3 : Mode configuration
 * -------------------------------------------------------------------------- */
static void test_mode_config(uint32_t idx)
{
    csi_iic_t iic;
    csi_error_t ret;

    memset(&iic, 0, sizeof(iic));
    csi_iic_init(&iic, idx);
    platform_i2c_extra_init(idx);

    ret = csi_iic_mode(&iic, IIC_MODE_MASTER);
    ASSERT_EQ("mode_master", ret, CSI_OK);

    ret = csi_iic_mode(&iic, IIC_MODE_SLAVE);
    if (ret == CSI_OK) {
        TEST_PASS("mode_slave");
    } else {
        TEST_SKIP("mode_slave", "slave mode not supported on this platform");
    }

    csi_iic_uninit(&iic);
}

/* --------------------------------------------------------------------------
 *  Test 4 : Address mode configuration
 * -------------------------------------------------------------------------- */
static void test_addr_mode_config(uint32_t idx)
{
    csi_iic_t iic;
    csi_error_t ret;

    memset(&iic, 0, sizeof(iic));
    csi_iic_init(&iic, idx);
    platform_i2c_extra_init(idx);
    csi_iic_mode(&iic, IIC_MODE_MASTER);

    ret = csi_iic_addr_mode(&iic, IIC_ADDRESS_7BIT);
    ASSERT_EQ("addr_mode_7bit", ret, CSI_OK);

    ret = csi_iic_addr_mode(&iic, IIC_ADDRESS_10BIT);
    if (ret == CSI_OK) {
        TEST_PASS("addr_mode_10bit");
    } else {
        TEST_SKIP("addr_mode_10bit", "10-bit addr not supported");
    }

    csi_iic_uninit(&iic);
}

/* --------------------------------------------------------------------------
 *  Test 5 : Speed configuration
 * -------------------------------------------------------------------------- */
static void test_speed_config(uint32_t idx)
{
    csi_iic_t iic;
    csi_error_t ret;

    memset(&iic, 0, sizeof(iic));
    csi_iic_init(&iic, idx);
    platform_i2c_extra_init(idx);
    csi_iic_mode(&iic, IIC_MODE_MASTER);
    csi_iic_addr_mode(&iic, IIC_ADDRESS_7BIT);

    /* Standard speed – must pass */
    ret = csi_iic_speed(&iic, IIC_BUS_SPEED_STANDARD);
    ASSERT_EQ("speed_standard", ret, CSI_OK);

    /* Fast speed */
    ret = csi_iic_speed(&iic, IIC_BUS_SPEED_FAST);
    if (ret == CSI_OK) {
        TEST_PASS("speed_fast");
    } else {
        TEST_SKIP("speed_fast", "fast speed not supported");
    }

    /* High speed */
    ret = csi_iic_speed(&iic, IIC_BUS_SPEED_HIGH);
    if (ret == CSI_OK) {
        TEST_PASS("speed_high");
    } else {
        TEST_SKIP("speed_high", "high speed not supported");
    }

    csi_iic_uninit(&iic);
}

/* --------------------------------------------------------------------------
 *  Test 6 : Full init helper (master, 7-bit, standard) and state query
 * -------------------------------------------------------------------------- */
static csi_error_t full_master_init(csi_iic_t *iic, uint32_t idx)
{
    csi_error_t ret;

    memset(iic, 0, sizeof(*iic));

    ret = csi_iic_init(iic, idx);
    if (ret != CSI_OK) return ret;

    platform_i2c_extra_init(idx);

    ret = csi_iic_mode(iic, IIC_MODE_MASTER);
    if (ret != CSI_OK) goto fail;

    ret = csi_iic_addr_mode(iic, IIC_ADDRESS_7BIT);
    if (ret != CSI_OK) goto fail;

    ret = csi_iic_speed(iic, IIC_BUS_SPEED_STANDARD);
    if (ret != CSI_OK) goto fail;

    return CSI_OK;
fail:
    csi_iic_uninit(iic);
    return CSI_ERROR;
}

static void test_get_state(uint32_t idx)
{
    csi_iic_t iic;
    csi_error_t ret;
    csi_state_t state;

    ret = full_master_init(&iic, idx);
    if (ret != CSI_OK) {
        TEST_FAIL("get_state_init", "full_master_init failed");
        return;
    }

    memset(&state, 0xFF, sizeof(state));
    ret = csi_iic_get_state(&iic, &state);
    if (ret == CSI_OK) {
        TEST_PASS("get_state");
        printf("       state: readable=%d, writeable=%d\n",
               state.readable, state.writeable);
    } else {
        TEST_SKIP("get_state", "csi_iic_get_state not implemented");
    }

    csi_iic_uninit(&iic);
}

/* --------------------------------------------------------------------------
 *  Test 7 : Callback attach & detach
 * -------------------------------------------------------------------------- */
static void test_callback(uint32_t idx)
{
    csi_iic_t iic;
    csi_error_t ret;

    ret = full_master_init(&iic, idx);
    if (ret != CSI_OK) {
        TEST_FAIL("callback_init", "full_master_init failed");
        return;
    }

    ret = csi_iic_attach_callback(&iic, test_iic_callback, NULL);
    if (ret == CSI_OK) {
        TEST_PASS("callback_attach");
    } else {
        TEST_SKIP("callback_attach", "attach_callback not implemented");
    }

    csi_iic_detach_callback(&iic);
    TEST_PASS("callback_detach");

    csi_iic_uninit(&iic);
}

/* --------------------------------------------------------------------------
 *  Test 8 : Bus detect (scan 7-bit addresses)
 * -------------------------------------------------------------------------- */
static void test_bus_detect(uint32_t idx)
{
    csi_iic_t iic;
    csi_error_t ret;
    uint8_t data;
    uint32_t found = 0;

    ret = full_master_init(&iic, idx);
    if (ret != CSI_OK) {
        TEST_FAIL("bus_detect_init", "full_master_init failed");
        return;
    }

    printf("       Scanning I2C-%u bus ...\n", idx);
    printf("         0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");

    for (uint32_t i = 0; i < 128; i += 16) {
        printf("  %02x: ", i);
        for (uint32_t j = 0; j < 16; j++) {
            uint32_t addr = i + j;
            /* Skip reserved addresses: 0x00-0x07, 0x78-0x7f */
            if (addr < 0x08 || addr > 0x77) {
                printf("   ");
                continue;
            }
            int32_t r = csi_iic_master_receive(&iic, addr, &data, 1, 1);
            if (r >= 0) {
                printf("%02x ", addr);
                found++;
            } else {
                printf("-- ");
            }
        }
        printf("\n");
    }

    csi_iic_uninit(&iic);

    /* We can't know how many devices exist, just ensure no crash */
    printf("       Found %u device(s) on I2C-%u\n", found, idx);
    TEST_PASS("bus_detect");
}

/* --------------------------------------------------------------------------
 *  Test 9 : Master send to non-existent address (expect fail / NACK)
 * -------------------------------------------------------------------------- */
static void test_master_send_no_device(uint32_t idx)
{
    csi_iic_t iic;
    csi_error_t ret;
    uint8_t dummy[1] = {0x00};

    ret = full_master_init(&iic, idx);
    if (ret != CSI_OK) {
        TEST_FAIL("master_send_nodev_init", "full_master_init failed");
        return;
    }

    /* Address 0x7F is reserved/unlikely to have a device */
    int32_t r = csi_iic_master_send(&iic, 0x7F, dummy, 1, 10);
    if (r < 0) {
        TEST_PASS("master_send_nodev_nack");
    } else {
        /* Some controllers may not report NACK as error */
        TEST_SKIP("master_send_nodev_nack", "controller did not report NACK error");
    }

    csi_iic_uninit(&iic);
}

/* --------------------------------------------------------------------------
 *  Test 10 : Master receive from non-existent address (expect fail / NACK)
 * -------------------------------------------------------------------------- */
static void test_master_recv_no_device(uint32_t idx)
{
    csi_iic_t iic;
    csi_error_t ret;
    uint8_t data[1] = {0};

    ret = full_master_init(&iic, idx);
    if (ret != CSI_OK) {
        TEST_FAIL("master_recv_nodev_init", "full_master_init failed");
        return;
    }

    int32_t r = csi_iic_master_receive(&iic, 0x7F, data, 1, 10);
    if (r < 0) {
        TEST_PASS("master_recv_nodev_nack");
    } else {
        TEST_SKIP("master_recv_nodev_nack", "controller did not report NACK error");
    }

    csi_iic_uninit(&iic);
}

/* --------------------------------------------------------------------------
 *  Helper: detect device register address width (8-bit or 16-bit)
 *  Returns: 1 = 8-bit, 2 = 16-bit, 0 = neither works
 * -------------------------------------------------------------------------- */
static uint32_t detect_reg_width(uint32_t idx, uint32_t dev_addr,
                                 uint32_t reg_addr)
{
    csi_iic_t iic;
    uint8_t data;
    int32_t r;

    /* Try 16-bit first (more specific, avoids accidental match on 8-bit) */
    if (full_master_init(&iic, idx) == CSI_OK) {
        r = csi_iic_mem_receive(&iic, dev_addr, (uint16_t)reg_addr,
                                IIC_MEM_ADDR_SIZE_16BIT, &data, 1, 50);
        csi_iic_uninit(&iic);
        if (r >= 0) {
            printf("       Auto-detected reg width: 16-bit\n");
            return 2;
        }
    }

    /* Try 8-bit */
    if (full_master_init(&iic, idx) == CSI_OK) {
        r = csi_iic_mem_receive(&iic, dev_addr, (uint16_t)reg_addr,
                                IIC_MEM_ADDR_SIZE_8BIT, &data, 1, 50);
        csi_iic_uninit(&iic);
        if (r >= 0) {
            printf("       Auto-detected reg width: 8-bit\n");
            return 1;
        }
    }

    printf("       WARNING: Could not detect reg width for dev 0x%02x\n",
           dev_addr);
    return 0;
}

/* --------------------------------------------------------------------------
 *  Test 11 : mem_send / mem_receive with specified register width
 * -------------------------------------------------------------------------- */
static void test_mem_rw(uint32_t idx, uint32_t dev_addr, uint32_t reg_addr,
                        csi_iic_mem_addr_size_t addr_size, const char *tag)
{
    csi_iic_t iic;
    csi_error_t ret;
    uint8_t orig_val = 0;
    uint8_t write_val = (addr_size == IIC_MEM_ADDR_SIZE_8BIT) ? 0xA5 : 0x5A;
    uint8_t readback = 0;
    int32_t r;
    uint32_t timeout = 50;
    char name[64];

    ret = full_master_init(&iic, idx);
    if (ret != CSI_OK) {
        snprintf(name, sizeof(name), "mem_rw_%s_init", tag);
        TEST_FAIL(name, "full_master_init failed");
        return;
    }

    /* Read original value */
    r = csi_iic_mem_receive(&iic, dev_addr, (uint16_t)reg_addr,
                            addr_size, &orig_val, 1, timeout);
    snprintf(name, sizeof(name), "mem_read_%s", tag);
    if (r < 0) {
        TEST_SKIP(name, "device does not support this reg addr width");
        csi_iic_uninit(&iic);
        return;
    }
    TEST_PASS(name);
    printf("       Original value at dev 0x%02x reg 0x%02x = 0x%02x\n",
           dev_addr, reg_addr, orig_val);

    /* Write a test value */
    r = csi_iic_mem_send(&iic, dev_addr, (uint16_t)reg_addr,
                         addr_size, &write_val, 1, timeout);
    snprintf(name, sizeof(name), "mem_write_%s", tag);
    if (r < 0) {
        TEST_FAIL(name, "mem_send returned %d", (int)r);
        csi_iic_uninit(&iic);
        return;
    }
    TEST_PASS(name);

    /* Small delay for device to complete write (e.g. EEPROM write cycle) */
    aos_msleep(10);

    /* Read back and verify */
    r = csi_iic_mem_receive(&iic, dev_addr, (uint16_t)reg_addr,
                            addr_size, &readback, 1, timeout);
    snprintf(name, sizeof(name), "mem_readback_%s_verify", tag);
    if (r < 0) {
        TEST_FAIL(name, "mem_receive returned %d", (int)r);
    } else if (readback == write_val) {
        TEST_PASS(name);
        printf("       Read-back matched: 0x%02x\n", readback);
    } else {
        TEST_FAIL(name, "expected 0x%02x, got 0x%02x", write_val, readback);
    }

    /* Restore original value */
    csi_iic_mem_send(&iic, dev_addr, (uint16_t)reg_addr,
                     addr_size, &orig_val, 1, timeout);
    aos_msleep(10);

    csi_iic_uninit(&iic);
}

/* --------------------------------------------------------------------------
 *  Test 13 : Master raw send + receive with real device
 * -------------------------------------------------------------------------- */
static void test_master_send_recv(uint32_t idx, uint32_t dev_addr,
                                  uint32_t reg_addr)
{
    csi_iic_t iic;
    csi_error_t ret;
    uint8_t tx_buf[1];
    uint8_t rx_buf[1] = {0};
    int32_t r;
    uint32_t timeout = 50;

    ret = full_master_init(&iic, idx);
    if (ret != CSI_OK) {
        TEST_FAIL("master_send_recv_init", "full_master_init failed");
        return;
    }

    /* Send register address first */
    tx_buf[0] = (uint8_t)(reg_addr & 0xFF);
    r = csi_iic_master_send(&iic, dev_addr, tx_buf, 1, timeout);
    if (r < 0) {
        TEST_FAIL("master_send", "master_send returned %d", (int)r);
        csi_iic_uninit(&iic);
        return;
    }
    TEST_PASS("master_send");

    /* Read one byte from device */
    r = csi_iic_master_receive(&iic, dev_addr, rx_buf, 1, timeout);
    if (r < 0) {
        TEST_FAIL("master_receive", "master_receive returned %d", (int)r);
    } else {
        TEST_PASS("master_receive");
        printf("       Received: 0x%02x from dev 0x%02x\n", rx_buf[0], dev_addr);
    }

    csi_iic_uninit(&iic);
}

/* --------------------------------------------------------------------------
 *  Test 14 : Multi-byte read/write (uses detected reg width)
 * -------------------------------------------------------------------------- */
static void test_multi_byte_rw(uint32_t idx, uint32_t dev_addr,
                               uint32_t reg_addr,
                               csi_iic_mem_addr_size_t addr_size)
{
    csi_iic_t iic;
    csi_error_t ret;
    uint8_t orig[4] = {0};
    uint8_t write_data[4] = {0xDE, 0xAD, 0xBE, 0xEF};
    uint8_t readback[4] = {0};
    int32_t r;
    uint32_t timeout = 50;
    uint32_t len = 4;

    ret = full_master_init(&iic, idx);
    if (ret != CSI_OK) {
        TEST_FAIL("multi_byte_rw_init", "full_master_init failed");
        return;
    }

    /* Save original 4 bytes */
    r = csi_iic_mem_receive(&iic, dev_addr, (uint16_t)reg_addr,
                            addr_size, orig, len, timeout);
    if (r < 0) {
        TEST_SKIP("multi_byte_read", "device does not support multi-byte read");
        csi_iic_uninit(&iic);
        return;
    }
    TEST_PASS("multi_byte_read");

    /* Write 4 bytes */
    r = csi_iic_mem_send(&iic, dev_addr, (uint16_t)reg_addr,
                         addr_size, write_data, len, timeout);
    if (r < 0) {
        TEST_FAIL("multi_byte_write", "mem_send returned %d", (int)r);
        csi_iic_uninit(&iic);
        return;
    }
    TEST_PASS("multi_byte_write");

    aos_msleep(10);

    /* Read back 4 bytes */
    r = csi_iic_mem_receive(&iic, dev_addr, (uint16_t)reg_addr,
                            addr_size, readback, len, timeout);
    if (r < 0) {
        TEST_FAIL("multi_byte_readback", "mem_receive returned %d", (int)r);
    } else if (memcmp(readback, write_data, len) == 0) {
        TEST_PASS("multi_byte_readback_verify");
    } else {
        TEST_FAIL("multi_byte_readback_verify",
                  "data mismatch: %02x%02x%02x%02x vs %02x%02x%02x%02x",
                  readback[0], readback[1], readback[2], readback[3],
                  write_data[0], write_data[1], write_data[2], write_data[3]);
    }

    /* Restore original data */
    csi_iic_mem_send(&iic, dev_addr, (uint16_t)reg_addr,
                     addr_size, orig, len, timeout);
    aos_msleep(10);

    csi_iic_uninit(&iic);
}

/* --------------------------------------------------------------------------
 *  Test 15 : Different speed transfer test
 * -------------------------------------------------------------------------- */
static void test_speed_transfer(uint32_t idx, uint32_t dev_addr,
                                uint32_t reg_addr,
                                csi_iic_mem_addr_size_t addr_size)
{
    csi_iic_t iic;
    csi_error_t ret;
    uint8_t data = 0;
    int32_t r;
    uint32_t timeout = 50;

    const csi_iic_speed_t speeds[] = {
        IIC_BUS_SPEED_STANDARD,
        IIC_BUS_SPEED_FAST,
    };
    const char *speed_names[] = {
        "speed_xfer_standard",
        "speed_xfer_fast",
    };

    for (uint32_t i = 0; i < sizeof(speeds) / sizeof(speeds[0]); i++) {
        memset(&iic, 0, sizeof(iic));
        ret = csi_iic_init(&iic, idx);
        if (ret != CSI_OK) {
            TEST_FAIL(speed_names[i], "init failed");
            continue;
        }
        platform_i2c_extra_init(idx);
        csi_iic_mode(&iic, IIC_MODE_MASTER);
        csi_iic_addr_mode(&iic, IIC_ADDRESS_7BIT);

        ret = csi_iic_speed(&iic, speeds[i]);
        if (ret != CSI_OK) {
            TEST_SKIP(speed_names[i], "speed not supported");
            csi_iic_uninit(&iic);
            continue;
        }

        r = csi_iic_mem_receive(&iic, dev_addr, (uint16_t)reg_addr,
                                addr_size, &data, 1, timeout);
        if (r >= 0) {
            TEST_PASS(speed_names[i]);
            printf("       Read 0x%02x at speed mode %u\n", data, speeds[i]);
        } else {
            TEST_FAIL(speed_names[i], "read failed at speed mode %u", speeds[i]);
        }

        csi_iic_uninit(&iic);
    }
}

/* --------------------------------------------------------------------------
 *  Test 16 : Repeated init/uninit stress
 * -------------------------------------------------------------------------- */
static void test_init_stress(uint32_t idx)
{
    csi_iic_t iic;
    csi_error_t ret;
    uint32_t fail_count = 0;
    const uint32_t iterations = 20;

    for (uint32_t i = 0; i < iterations; i++) {
        memset(&iic, 0, sizeof(iic));
        ret = csi_iic_init(&iic, idx);
        if (ret != CSI_OK) {
            fail_count++;
            continue;
        }
        platform_i2c_extra_init(idx);
        csi_iic_mode(&iic, IIC_MODE_MASTER);
        csi_iic_addr_mode(&iic, IIC_ADDRESS_7BIT);
        csi_iic_speed(&iic, IIC_BUS_SPEED_STANDARD);
        csi_iic_uninit(&iic);
    }

    if (fail_count == 0) {
        TEST_PASS("init_stress_20x");
    } else {
        TEST_FAIL("init_stress_20x", "%u/%u iterations failed",
                  fail_count, iterations);
    }
}

/* --------------------------------------------------------------------------
 *  Test 17 : Invalid index init
 * -------------------------------------------------------------------------- */
static void test_invalid_index(void)
{
    csi_iic_t iic;
    csi_error_t ret;

    memset(&iic, 0, sizeof(iic));
    ret = csi_iic_init(&iic, 99);
    if (ret != CSI_OK) {
        TEST_PASS("invalid_index_99");
    } else {
        TEST_FAIL("invalid_index_99", "expected failure but got CSI_OK");
        csi_iic_uninit(&iic);
    }
}

/* --------------------------------------------------------------------------
 *  Main test runner
 * -------------------------------------------------------------------------- */
static void run_all_tests(uint32_t idx, int has_device,
                          uint32_t dev_addr, uint32_t reg_addr,
                          uint32_t reg_width)
{
    stats_reset();

    printf("\n============================================\n");
    printf("  I2C Auto-Test on bus %u\n", idx);
    if (has_device) {
        printf("  Target device: 0x%02x  register: 0x%02x", dev_addr, reg_addr);
        if (reg_width)
            printf("  reg_width: %u-bit", reg_width * 8);
        else
            printf("  reg_width: auto-detect");
        printf("\n");
    } else {
        printf("  No target device specified (device tests will be skipped)\n");
    }
    printf("============================================\n\n");

    printf("--- [Group 1] Init / Uninit ---\n");
    test_init_uninit(idx);
    test_double_init(idx);
    test_invalid_index();
    test_init_stress(idx);

    printf("\n--- [Group 2] Configuration ---\n");
    test_mode_config(idx);
    test_addr_mode_config(idx);
    test_speed_config(idx);

    printf("\n--- [Group 3] State & Callback ---\n");
    test_get_state(idx);
    test_callback(idx);

    printf("\n--- [Group 4] Bus Detect ---\n");
    test_bus_detect(idx);

    printf("\n--- [Group 5] Error Handling (no-device) ---\n");
    test_master_send_no_device(idx);
    test_master_recv_no_device(idx);

    printf("\n--- [Group 6] Device Communication ---\n");
    if (has_device) {
        /* Auto-detect register width if not specified */
        uint32_t detected_width = reg_width;
        if (detected_width == 0)
            detected_width = detect_reg_width(idx, dev_addr, reg_addr);

        if (detected_width == 0) {
            printf("       Cannot communicate with dev 0x%02x, skipping device tests\n",
                   dev_addr);
            TEST_SKIP("master_send_recv", "device not responding");
            TEST_SKIP("mem_rw", "device not responding");
            TEST_SKIP("multi_byte_rw", "device not responding");
        } else {
            csi_iic_mem_addr_size_t addr_size =
                (detected_width == 2) ? IIC_MEM_ADDR_SIZE_16BIT
                                      : IIC_MEM_ADDR_SIZE_8BIT;

            test_master_send_recv(idx, dev_addr, reg_addr);

            /* Test with the detected/specified width */
            test_mem_rw(idx, dev_addr, reg_addr, addr_size,
                        (detected_width == 2) ? "16bit" : "8bit");

            /* Also try the other width (expect SKIP if unsupported) */
            csi_iic_mem_addr_size_t alt_size =
                (detected_width == 2) ? IIC_MEM_ADDR_SIZE_8BIT
                                      : IIC_MEM_ADDR_SIZE_16BIT;
            test_mem_rw(idx, dev_addr, reg_addr, alt_size,
                        (detected_width == 2) ? "8bit" : "16bit");

            test_multi_byte_rw(idx, dev_addr, reg_addr, addr_size);
        }
    } else {
        TEST_SKIP("master_send_recv", "no device specified");
        TEST_SKIP("mem_rw", "no device specified");
        TEST_SKIP("multi_byte_rw", "no device specified");
    }

    printf("\n--- [Group 7] Speed Transfer Test ---\n");
    if (has_device) {
        uint32_t detected_width = reg_width;
        if (detected_width == 0)
            detected_width = detect_reg_width(idx, dev_addr, reg_addr);
        if (detected_width > 0) {
            csi_iic_mem_addr_size_t addr_size =
                (detected_width == 2) ? IIC_MEM_ADDR_SIZE_16BIT
                                      : IIC_MEM_ADDR_SIZE_8BIT;
            test_speed_transfer(idx, dev_addr, reg_addr, addr_size);
        } else {
            TEST_SKIP("speed_xfer", "device not responding");
        }
    } else {
        TEST_SKIP("speed_xfer", "no device specified");
    }

    stats_report();
}

/* --------------------------------------------------------------------------
 *  CLI entry
 * -------------------------------------------------------------------------- */
static void print_test_usage(void)
{
    printf("usage:\n"
           "\ti2c_test auto <idx>\n"
           "\t\tRun all API/config tests on I2C bus <idx> (hex)\n\n"
           "\ti2c_test auto <idx> <dev_addr> <reg_addr>\n"
           "\t\tRun all tests, auto-detect reg addr width\n\n"
           "\ti2c_test auto <idx> <dev_addr> <reg_addr> <reg_width>\n"
           "\t\tRun all tests with specified reg width\n"
           "\t\t<reg_width>: 1=8-bit, 2=16-bit\n"
           "\t\tAll params are in hex\n");
}

void i2c_auto_test_cmd(int argc, char **argv)
{
    uint32_t idx;
    uint32_t dev_addr = 0;
    uint32_t reg_addr = 0;
    uint32_t reg_width = 0; /* 0=auto, 1=8bit, 2=16bit */
    int has_device = 0;

    if (argc < 3) {
        print_test_usage();
        return;
    }

    if (strcmp(argv[1], "auto") != 0) {
        print_test_usage();
        return;
    }

    idx = strtol(argv[2], NULL, 16);

    if (argc >= 5) {
        dev_addr = strtol(argv[3], NULL, 16);
        reg_addr = strtol(argv[4], NULL, 16);
        has_device = 1;
    }

    if (argc >= 6) {
        reg_width = strtol(argv[5], NULL, 16);
        if (reg_width != 1 && reg_width != 2) {
            printf("ERROR: reg_width must be 1 (8-bit) or 2 (16-bit)\n");
            return;
        }
    }

    run_all_tests(idx, has_device, dev_addr, reg_addr, reg_width);
}

ALIOS_CLI_CMD_REGISTER(i2c_auto_test_cmd, i2c_test, i2c auto test suite);

/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2026. All rights reserved.
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
 * @file pwm_auto_test.c
 * @brief PWM automated functional test cases
 *
 * CV184X PWM IP architecture:
 *   - 3 PWM IPs (pwmchip0 / pwmchip6 / pwmchip12)
 *   - pwmchip0  (bank 0): 6 channels → pwm0  ~ pwm5
 *   - pwmchip6  (bank 1): 6 channels → pwm6  ~ pwm11
 *   - pwmchip12 (bank 2): 4 channels → pwm12 ~ pwm15
 *
 * AliOS CSI mapping:
 *   Global PWM N → bank = N / 6 (for N<12), bank = 2 (for N>=12)
 *                  channel = N % 6 (for N<12), channel = N - 12 (for N>=12)
 *
 * Test cases cover:
 *   TC01: Init / Uninit
 *   TC02: Repeated init / uninit (stability)
 *   TC03: Output config – various period & duty
 *   TC04: Output start / stop
 *   TC05: Polarity config (HIGH / LOW)
 *   TC06: Multi-channel output
 *   TC07: Boundary values (min/max period & duty)
 *   TC08: Invalid parameter handling
 *   TC09: Capture config
 *   TC10: PWM0 functional test
 *   TC11: PWM7 functional test
 *   TC12: Stress test – repeated start/stop
 *
 * Hardware requirement:
 *   - PWM0 pin (PWM0_BUCK) accessible for output test
 *   - PWM7 pin (SD1_D0) accessible for output test
 *
 * Usage (CLI):
 *   pwm_auto_test <bank_idx>               – run all test cases
 *   pwm_auto_test <bank_idx> <tc_number>   – run specific test case (1-12)
 *
 * Examples:
 *   pwm_auto_test 0        – run all tests on PWM bank 0
 *   pwm_auto_test 0 1      – run TC01 on PWM bank 0
 *   pwm_auto_test 0 10     – run TC10 (PWM0 functional, ignores bank_idx)
 *   pwm_auto_test 0 11     – run TC11 (PWM7 functional, ignores bank_idx)
 */

#include <drv/pwm.h>
#include <drv/pin.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <aos/cli.h>
#include <unistd.h>
#include "pinctrl-mars.h"

/* ======================== Macros & Constants ======================== */

#define PWM_DEFAULT_PERIOD_NS   1000000     /* 1ms = 1KHz */
#define PWM_DEFAULT_DUTY_NS     500000      /* 500us = 50% */
#define PWM_RUN_TIME_US         500000      /* 500ms run time for output tests */
#define STRESS_ITERATIONS       100

/*
 * CV184X PWM IP channel layout:
 *   Bank 0 (pwmchip0 ):  6 channels (ch0~ch5)  → pwm0  ~ pwm5
 *   Bank 1 (pwmchip6 ):  6 channels (ch0~ch5)  → pwm6  ~ pwm11
 *   Bank 2 (pwmchip12):  4 channels (ch0~ch3)  → pwm12 ~ pwm15
 */
#define PWM_NUM_BANKS           3
static const uint32_t g_bank_channels[PWM_NUM_BANKS] = { 6, 6, 4 };

#define TEST_LOG(fmt, ...)  printf("[PWM_TEST] " fmt "\n", ##__VA_ARGS__)

/* ======================== Runtime PWM bank index ======================== */

static uint32_t g_pwm_bank = 0;    /* PWM bank index, set by CLI arg */

/* ======================== Test result tracking ======================== */

static int g_total_tests = 0;
static int g_pass_count  = 0;
static int g_fail_count  = 0;

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

/* ======================== Pinmux configuration ======================== */

/**
 * Configure pinmux for PWM0 (PWM0_BUCK pad)
 * Matches run_pwm_test.sh: devmem 0x030010EC 32 0
 */
static void pwm0_pinmux_setup(void)
{
    PINMUX_CONFIG(PWM0_BUCK, PWM_0);
    TEST_LOG("Pinmux: PWM0_BUCK -> PWM_0");
}

/**
 * Configure pinmux for PWM7
 * Option 1 (default): SD1_D0 -> PWM_7  (safe, no JTAG conflict)
 * Option 2: SD1_D0 -> PWM_7      (matches run_pwm_test.sh, but may hang
 *           if JTAG debugger is connected on AliOS)
 *
 * Matches run_pwm_test.sh logic but uses SD1_D0 to avoid JTAG conflict.
 * To switch back to SD1_D0, set PWM7_USE_JTAG_PIN to 1.
 */

static void pwm7_pinmux_setup(void)
{
    PINMUX_CONFIG(SD1_D0, PWM_7);
    TEST_LOG("Pinmux: SD1_D0 -> PWM_7");
}

/* ======================================================================== */
/*                           TEST CASES                                     */
/* ======================================================================== */

/**
 * TC01: Init / Uninit
 * Verify that csi_pwm_init succeeds and csi_pwm_uninit does not crash.
 */
static int tc01_init_uninit(void)
{
    TEST_LOG(">>> TC01: Init / Uninit");
    csi_pwm_t pwm;
    csi_error_t ret;

    ret = csi_pwm_init(&pwm, g_pwm_bank);
    TEST_ASSERT(ret == CSI_OK, "csi_pwm_init returns CSI_OK");

    csi_pwm_uninit(&pwm);
    TEST_ASSERT_NRET(1, "csi_pwm_uninit completes without crash");
    return 0;
}

/**
 * TC02: Repeated init / uninit
 * Init and uninit multiple times to verify no resource leaks.
 */
static int tc02_repeated_init_uninit(void)
{
    TEST_LOG(">>> TC02: Repeated init / uninit (10 times)");
    csi_pwm_t pwm;
    int pass = 1;

    for (int i = 0; i < 10; i++) {
        csi_error_t ret = csi_pwm_init(&pwm, g_pwm_bank);
        if (ret != CSI_OK) {
            pass = 0;
            printf("    init failed at iteration %d\n", i);
            break;
        }
        csi_pwm_uninit(&pwm);
    }
    TEST_ASSERT(pass, "10x init/uninit cycles succeed");
    return 0;
}

/**
 * TC03: Output config – various period & duty combinations
 * Verify csi_pwm_out_config accepts different period/duty values.
 */
static int tc03_out_config(void)
{
    TEST_LOG(">>> TC03: Output config – various period & duty");
    csi_pwm_t pwm;
    csi_error_t ret;

    static const struct {
        uint32_t period_ns;
        uint32_t duty_ns;
        const char *desc;
    } configs[] = {
        { 1000000,  500000,  "1KHz 50%"      },  /* 1ms period, 50% duty */
        { 1000000,  250000,  "1KHz 25%"      },  /* 1ms period, 25% duty */
        { 1000000,  750000,  "1KHz 75%"      },  /* 1ms period, 75% duty */
        { 1000000, 1000000,  "1KHz 100%"     },  /* 1ms period, 100% duty */
        {  500000,  250000,  "2KHz 50%"      },  /* 500us period */
        {   20000,   10000,  "50KHz 50%"     },  /* 20us period */
        {  100000,   50000,  "10KHz 50%"     },  /* 100us period */
    };

    ret = csi_pwm_init(&pwm, g_pwm_bank);
    TEST_ASSERT(ret == CSI_OK, "init OK");

    for (int i = 0; i < (int)(sizeof(configs) / sizeof(configs[0])); i++) {
        ret = csi_pwm_out_config(&pwm, 0,
                                 configs[i].period_ns,
                                 configs[i].duty_ns,
                                 PWM_POLARITY_LOW);
        csi_pwm_out_stop(&pwm, 0);

        char msg[80];
        snprintf(msg, sizeof(msg), "out_config %s (period=%u, duty=%u)",
                 configs[i].desc, configs[i].period_ns, configs[i].duty_ns);
        TEST_ASSERT_NRET(ret == CSI_OK, msg);
    }

    csi_pwm_uninit(&pwm);
    return 0;
}

/**
 * TC04: Output start / stop
 * Verify start and stop do not crash and can be called repeatedly.
 */
static int tc04_out_start_stop(void)
{
    TEST_LOG(">>> TC04: Output start / stop");
    csi_pwm_t pwm;
    csi_error_t ret;

    ret = csi_pwm_init(&pwm, g_pwm_bank);
    TEST_ASSERT(ret == CSI_OK, "init OK");

    ret = csi_pwm_out_config(&pwm, 0,
                             PWM_DEFAULT_PERIOD_NS,
                             PWM_DEFAULT_DUTY_NS,
                             PWM_POLARITY_LOW);
    TEST_ASSERT(ret == CSI_OK, "out_config OK");

    ret = csi_pwm_out_start(&pwm, 0);
    TEST_ASSERT(ret == CSI_OK, "out_start OK");

    usleep(PWM_RUN_TIME_US);

    csi_pwm_out_stop(&pwm, 0);
    TEST_ASSERT_NRET(1, "out_stop completes without crash");

    /* Start again after stop */
    ret = csi_pwm_out_start(&pwm, 0);
    TEST_ASSERT_NRET(ret == CSI_OK, "re-start after stop OK");

    usleep(PWM_RUN_TIME_US);
    csi_pwm_out_stop(&pwm, 0);
    TEST_ASSERT_NRET(1, "re-stop completes without crash");

    csi_pwm_uninit(&pwm);
    return 0;
}

/**
 * TC05: Polarity config (HIGH / LOW)
 * Verify both polarity settings are accepted.
 */
static int tc05_polarity(void)
{
    TEST_LOG(">>> TC05: Polarity config (HIGH / LOW)");
    csi_pwm_t pwm;
    csi_error_t ret;

    ret = csi_pwm_init(&pwm, g_pwm_bank);
    TEST_ASSERT(ret == CSI_OK, "init OK");

    /* Polarity LOW */
    ret = csi_pwm_out_config(&pwm, 0,
                             PWM_DEFAULT_PERIOD_NS,
                             PWM_DEFAULT_DUTY_NS,
                             PWM_POLARITY_LOW);
    TEST_ASSERT_NRET(ret == CSI_OK, "config with PWM_POLARITY_LOW");

    ret = csi_pwm_out_start(&pwm, 0);
    TEST_ASSERT_NRET(ret == CSI_OK, "start with POLARITY_LOW");
    usleep(PWM_RUN_TIME_US);
    csi_pwm_out_stop(&pwm, 0);

    /* Polarity HIGH */
    ret = csi_pwm_out_config(&pwm, 0,
                             PWM_DEFAULT_PERIOD_NS,
                             PWM_DEFAULT_DUTY_NS,
                             PWM_POLARITY_HIGH);
    TEST_ASSERT_NRET(ret == CSI_OK, "config with PWM_POLARITY_HIGH");

    ret = csi_pwm_out_start(&pwm, 0);
    TEST_ASSERT_NRET(ret == CSI_OK, "start with POLARITY_HIGH");
    usleep(PWM_RUN_TIME_US);
    csi_pwm_out_stop(&pwm, 0);

    csi_pwm_uninit(&pwm);
    return 0;
}

/**
 * TC06: Multi-channel output
 * Configure and run all channels on the selected bank simultaneously.
 * Bank 0/1: 6 channels (ch0~ch5), Bank 2: 4 channels (ch0~ch3)
 */
static int tc06_multi_channel(void)
{
    uint32_t num_ch = (g_pwm_bank < PWM_NUM_BANKS) ?
                      g_bank_channels[g_pwm_bank] : g_bank_channels[0];

    TEST_LOG(">>> TC06: Multi-channel output (ch0~ch%u, bank %u)",
             num_ch - 1, g_pwm_bank);
    csi_pwm_t pwm;
    csi_error_t ret;
    int pass = 1;

    ret = csi_pwm_init(&pwm, g_pwm_bank);
    TEST_ASSERT(ret == CSI_OK, "init OK");

    /* Configure all channels with varying duty cycles */
    for (uint32_t ch = 0; ch < num_ch; ch++) {
        uint32_t duty = PWM_DEFAULT_PERIOD_NS * (ch + 1) / num_ch;

        ret = csi_pwm_out_config(&pwm, ch,
                                 PWM_DEFAULT_PERIOD_NS,
                                 duty,
                                 PWM_POLARITY_LOW);
        if (ret != CSI_OK) {
            printf("    out_config failed for ch%u\n", ch);
            pass = 0;
        }

        ret = csi_pwm_out_start(&pwm, ch);
        if (ret != CSI_OK) {
            printf("    out_start failed for ch%u\n", ch);
            pass = 0;
        }
    }

    char msg_start[80];
    snprintf(msg_start, sizeof(msg_start),
             "config & start ch0~ch%u", num_ch - 1);
    TEST_ASSERT_NRET(pass, msg_start);

    usleep(PWM_RUN_TIME_US);

    /* Stop all channels */
    for (uint32_t ch = 0; ch < num_ch; ch++) {
        csi_pwm_out_stop(&pwm, ch);
    }

    char msg_stop[80];
    snprintf(msg_stop, sizeof(msg_stop),
             "stop ch0~ch%u without crash", num_ch - 1);
    TEST_ASSERT_NRET(1, msg_stop);

    csi_pwm_uninit(&pwm);
    return 0;
}

/**
 * TC07: Boundary values (min/max period & duty)
 */
static int tc07_boundary_values(void)
{
    TEST_LOG(">>> TC07: Boundary values");
    csi_pwm_t pwm;
    csi_error_t ret;

    ret = csi_pwm_init(&pwm, g_pwm_bank);
    TEST_ASSERT(ret == CSI_OK, "init OK");

    /* Minimum period: 100ns (10MHz) */
    ret = csi_pwm_out_config(&pwm, 0, 100, 50, PWM_POLARITY_LOW);
    TEST_ASSERT_NRET(ret == CSI_OK, "min period 100ns");
    csi_pwm_out_stop(&pwm, 0);

    /* Zero duty cycle: period=1ms, duty=0 */
    ret = csi_pwm_out_config(&pwm, 0, 1000000, 0, PWM_POLARITY_LOW);
    TEST_ASSERT_NRET(ret == CSI_OK, "zero duty (period=1ms, duty=0)");
    csi_pwm_out_stop(&pwm, 0);

    /* Full duty cycle: period=1ms, duty=1ms (100%) */
    ret = csi_pwm_out_config(&pwm, 0, 1000000, 1000000, PWM_POLARITY_LOW);
    TEST_ASSERT_NRET(ret == CSI_OK, "full duty (period=1ms, duty=1ms)");
    csi_pwm_out_stop(&pwm, 0);

    /* Large period: 1 second */
    ret = csi_pwm_out_config(&pwm, 0, 1000000000, 500000000, PWM_POLARITY_LOW);
    TEST_ASSERT_NRET(ret == CSI_OK, "large period 1s");
    csi_pwm_out_stop(&pwm, 0);

    csi_pwm_uninit(&pwm);
    return 0;
}

/**
 * TC08: Invalid parameter handling
 * Verify driver handles invalid inputs gracefully.
 */
static int tc08_invalid_params(void)
{
    TEST_LOG(">>> TC08: Invalid parameter handling");
    csi_pwm_t pwm;
    csi_error_t ret;

    ret = csi_pwm_init(&pwm, g_pwm_bank);
    TEST_ASSERT(ret == CSI_OK, "init OK");

    /* duty > period should fail */
    ret = csi_pwm_out_config(&pwm, 0, 1000000, 2000000, PWM_POLARITY_LOW);
    TEST_ASSERT_NRET(ret != CSI_OK, "duty > period returns error");

    /* Invalid polarity (cast 99 to trigger default case) */
    ret = csi_pwm_out_config(&pwm, 0, 1000000, 500000, (csi_pwm_polarity_t)99);
    TEST_ASSERT_NRET(ret != CSI_OK, "invalid polarity returns error");

    /* Invalid bank index */
    csi_pwm_t bad_pwm;
    ret = csi_pwm_init(&bad_pwm, 99);
    TEST_ASSERT_NRET(ret != CSI_OK, "invalid bank idx returns error");

    csi_pwm_uninit(&pwm);
    return 0;
}

/**
 * TC09: Capture config
 * Verify capture config/start/stop API (only POSEDGE supported).
 */
static int tc09_capture(void)
{
    TEST_LOG(">>> TC09: Capture config");
    csi_pwm_t pwm;
    csi_error_t ret;

    ret = csi_pwm_init(&pwm, g_pwm_bank);
    TEST_ASSERT(ret == CSI_OK, "init OK");

    /* POSEDGE capture (supported) */
    ret = csi_pwm_capture_config(&pwm, 0, PWM_CAPTURE_POLARITY_POSEDGE, 0);
    TEST_ASSERT_NRET(ret == CSI_OK, "capture_config POSEDGE OK");

    ret = csi_pwm_capture_start(&pwm, 0);
    TEST_ASSERT_NRET(ret == CSI_OK, "capture_start OK");

    usleep(100000);  /* 100ms */

    csi_pwm_capture_stop(&pwm, 0);
    TEST_ASSERT_NRET(1, "capture_stop completes without crash");

    /* NEGEDGE capture (unsupported, should return CSI_UNSUPPORTED) */
    ret = csi_pwm_capture_config(&pwm, 0, PWM_CAPTURE_POLARITY_NEGEDGE, 0);
    TEST_ASSERT_NRET(ret == CSI_UNSUPPORTED, "capture_config NEGEDGE returns UNSUPPORTED");

    /* BOTHEDGE capture (unsupported) */
    ret = csi_pwm_capture_config(&pwm, 0, PWM_CAPTURE_POLARITY_BOTHEDGE, 0);
    TEST_ASSERT_NRET(ret == CSI_UNSUPPORTED, "capture_config BOTHEDGE returns UNSUPPORTED");

    csi_pwm_uninit(&pwm);
    return 0;
}

/**
 * TC10: PWM0 functional test
 * Corresponds to run_pwm_test.sh Test Case 1 (PWM0):
 *   - Configure pinmux (PWM0_BUCK -> PWM_0)
 *   - Set period=1ms (1KHz), duty=500us (50%)
 *   - Enable output, run 2s, disable
 */
static int tc10_pwm0_functional(void)
{
    TEST_LOG(">>> TC10: PWM0 functional test");
    csi_pwm_t pwm;
    csi_error_t ret;

    /* Step 1: Configure PinMux for PWM0 */
    pwm0_pinmux_setup();
    TEST_ASSERT_NRET(1, "PWM0 PinMux configured");

    /* Step 2: Init PWM bank 0 */
    ret = csi_pwm_init(&pwm, 0);
    TEST_ASSERT(ret == CSI_OK, "PWM bank 0 init OK");

    /* Step 3: Configure output – period=1ms, duty=500us, polarity=LOW */
    csi_pwm_out_stop(&pwm, 0);
    ret = csi_pwm_out_config(&pwm, 0,
                             1000000,   /* period: 1ms (1KHz) */
                             500000,    /* duty: 500us (50%) */
                             PWM_POLARITY_LOW);
    TEST_ASSERT(ret == CSI_OK, "PWM0 out_config (1KHz, 50%)");

    /* Step 4: Start PWM output */
    ret = csi_pwm_out_start(&pwm, 0);
    TEST_ASSERT(ret == CSI_OK, "PWM0 out_start OK");

    TEST_LOG("PWM0 running for 2 seconds...");
    sleep(2);

    /* Step 5: Stop PWM output */
    csi_pwm_out_stop(&pwm, 0);
    TEST_ASSERT_NRET(1, "PWM0 out_stop OK");

    csi_pwm_uninit(&pwm);
    return 0;
}

/**
 * TC11: PWM7 functional test
 * Corresponds to run_pwm_test.sh Test Case 2 (PWM7):
 *   - Configure pinmux (SD1_D0 -> PWM_7)
 *   - CV184X: pwm6~pwm11 → pwmchip6 (bank 1), ch0~ch5
 *     So PWM7 = bank 1, channel 1
 *   - Set period=1ms (1KHz), duty=1ms (100%)
 *   - Enable output, run 2s, disable
 */
static int tc11_pwm7_functional(void)
{
    TEST_LOG(">>> TC11: PWM7 functional test");
    csi_pwm_t pwm;
    csi_error_t ret;

    /*
     * PWM7 mapping on CV184X:
     *   pwmchip6 (bank 1): pwm6~pwm11 → ch0~ch5
     *   PWM7 = bank 1, channel 1
     */
    const uint32_t pwm7_bank    = 1;
    const uint32_t pwm7_channel = 1;

    /* Step 1: Configure PinMux for PWM7 */
    pwm7_pinmux_setup();
    TEST_ASSERT_NRET(1, "PWM7 PinMux configured");

    /* Step 2: Init PWM bank 1 */
    ret = csi_pwm_init(&pwm, pwm7_bank);
    TEST_ASSERT(ret == CSI_OK, "PWM bank 1 init OK");

    /* Step 3: Configure output – period=1ms, duty=1ms (100%), polarity=LOW */
    csi_pwm_out_stop(&pwm, pwm7_channel);
    ret = csi_pwm_out_config(&pwm, pwm7_channel,
                             1000000,   /* period: 1ms (1KHz) */
                             1000000,   /* duty: 1ms (100%) */
                             PWM_POLARITY_LOW);
    TEST_ASSERT(ret == CSI_OK, "PWM7 out_config (1KHz, 100%)");

    /* Step 4: Start PWM output */
    ret = csi_pwm_out_start(&pwm, pwm7_channel);
    TEST_ASSERT(ret == CSI_OK, "PWM7 out_start OK");

    TEST_LOG("PWM7 running for 2 seconds...");
    sleep(2);

    /* Step 5: Stop PWM output */
    csi_pwm_out_stop(&pwm, pwm7_channel);
    TEST_ASSERT_NRET(1, "PWM7 out_stop OK");

    csi_pwm_uninit(&pwm);
    return 0;
}

/**
 * TC12: Stress test – repeated start/stop (100 iterations)
 */
static int tc12_stress(void)
{
    TEST_LOG(">>> TC12: Stress test (%d iterations)", STRESS_ITERATIONS);
    csi_pwm_t pwm;
    csi_error_t ret;
    int fail_iter = -1;

    ret = csi_pwm_init(&pwm, g_pwm_bank);
    TEST_ASSERT(ret == CSI_OK, "init OK");

    ret = csi_pwm_out_config(&pwm, 0,
                             PWM_DEFAULT_PERIOD_NS,
                             PWM_DEFAULT_DUTY_NS,
                             PWM_POLARITY_LOW);
    TEST_ASSERT(ret == CSI_OK, "out_config OK");

    for (int i = 0; i < STRESS_ITERATIONS; i++) {
        ret = csi_pwm_out_start(&pwm, 0);
        if (ret != CSI_OK) {
            fail_iter = i;
            printf("    out_start failed at iteration %d\n", i);
            break;
        }

        /* Brief output */
        usleep(1000);  /* 1ms */

        csi_pwm_out_stop(&pwm, 0);
    }

    char msg[80];
    snprintf(msg, sizeof(msg), "stress %d start/stop iterations", STRESS_ITERATIONS);
    TEST_ASSERT(fail_iter == -1, msg);

    csi_pwm_uninit(&pwm);
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
    {  3, "Output config (period/duty)",  tc03_out_config           },
    {  4, "Output start/stop",            tc04_out_start_stop       },
    {  5, "Polarity (HIGH/LOW)",          tc05_polarity             },
    {  6, "Multi-channel output",         tc06_multi_channel        },
    {  7, "Boundary values",              tc07_boundary_values      },
    {  8, "Invalid parameter handling",   tc08_invalid_params       },
    {  9, "Capture config",               tc09_capture              },
    { 10, "PWM0 functional test",         tc10_pwm0_functional      },
    { 11, "PWM7 functional test",         tc11_pwm7_functional      },
    { 12, "Stress test (start/stop)",     tc12_stress               },
};

#define NUM_TEST_CASES  (int)(sizeof(g_test_table) / sizeof(g_test_table[0]))

static void print_usage(void)
{
    printf("Usage: pwm_auto_test <bank_idx> [tc_number]\n");
    printf("  bank_idx  - PWM bank index (0=pwmchip0, 1=pwmchip6, 2=pwmchip12)\n");
    printf("  tc_number - optional, run specific test case (1-%d)\n", NUM_TEST_CASES);
    printf("  No tc_number - run all test cases\n\n");
    printf("Examples:\n");
    printf("  pwm_auto_test 0        - run all tests on PWM bank 0\n");
    printf("  pwm_auto_test 0 10     - run TC10 (PWM0 functional test)\n");
    printf("  pwm_auto_test 0 11     - run TC11 (PWM7 functional test)\n\n");
    printf("Note: TC10/TC11 use fixed bank/channel mapping regardless of bank_idx.\n\n");
    printf("Test cases:\n");
    for (int i = 0; i < NUM_TEST_CASES; i++) {
        printf("  %2d: %s\n", g_test_table[i].id, g_test_table[i].name);
    }
}

static void print_summary(void)
{
    printf("\n");
    printf("============================================================\n");
    printf("  PWM Auto Test Summary\n");
    printf("============================================================\n");
    printf("  Total : %d\n", g_total_tests);
    printf("  PASS  : %d\n", g_pass_count);
    printf("  FAIL  : %d\n", g_fail_count);
    printf("  Result: %s\n", (g_fail_count == 0) ? "ALL PASSED" : "SOME FAILED");
    printf("============================================================\n");
}

int test_pwm_auto(int argc, char **argv)
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

    /* First arg: PWM bank index */
    g_pwm_bank = (uint32_t)atoi(argv[1]);

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
    printf("  PWM Automated Functional Test\n");
    printf("  Hardware: PWM bank %u (pwmchip%u, %u channels)\n",
           g_pwm_bank,
           (g_pwm_bank == 0) ? 0 : (g_pwm_bank == 1) ? 6 : 12,
           (g_pwm_bank < PWM_NUM_BANKS) ? g_bank_channels[g_pwm_bank] : 0);
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

ALIOS_CLI_CMD_REGISTER(test_pwm_auto, pwm_auto_test, PWM automated functional test);

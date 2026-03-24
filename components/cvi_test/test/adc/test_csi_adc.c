#include <drv/adc.h>
#include <stdio.h>
#include <stdlib.h>
#include <aos/cli.h>
#include <aos/kernel.h>

/**
 * for example:
 * testadc 0 1  # test adc0, channel 1, 就是ADC通道1
 * testadc 1 1  # test adc1, channel 1, 就是ADC通道4
 * testadc 2 1  # test adc2, channel 1, 就是ADC通道7
 * testadc 3 1  # test rtc adc0, channel 0, 就是RTC域的ADC通道10
 * testadc 4 1  # test rtc adc1, channel 1, 就是RTC域的ADC通道13
 * testadc 3 1 1    # intr mode: test rtc adc0, channel 1 (interrupt)
 */

static volatile uint8_t g_adc_complete_flag = 0;
static uint32_t g_recv_buf[1] = {0};

// Callback function to handle ADC interrupt
static void user_adc_callback(csi_adc_t *adc, csi_adc_event_t event, void *arg)
{
    g_adc_complete_flag = 1;
}

void test_adc(int32_t argc, char **argv)
{
    csi_adc_t adc;
    int value;
    csi_error_t ret;

    if (argc < 3) {
        printf("usage: testadc <chip_id> <ch_id> [mode]\r\n");
        printf("       mode 0: polling (default)\r\n");
        printf("       mode 1: interrupt/async\r\n");
        return;
    }

    int chip_id = atoi(argv[1]);
    int ch_id = atoi(argv[2]);
    int mode = 0;

    if (argc >= 4) {
        mode = atoi(argv[3]);
    }

    printf("test adc: chip=%d, ch=%d, mode=%s\r\n", chip_id, ch_id, mode ? "interrupt" : "polling");

    // 1. Initialize ADC
    ret = csi_adc_init(&adc, chip_id);
    if (ret != CSI_OK) {
        printf("csi_adc_init failed\n");
        return;
    }

    // 2. Enable Channel
    // Note: If hang happens here, it's likely a register access/clock issue, not IRQ related
    ret = csi_adc_channel_enable(&adc, ch_id, true);
    if (ret != CSI_OK) {
        printf("csi_adc_channel_enable failed\n");
        goto err_uninit;
    }

    // 3. Trim (Calibration)
    csi_adc_trim(&adc);

    if (mode == 0) {
        /* --- Polling Mode (Original) --- */
        ret = csi_adc_start(&adc);
        if (ret != CSI_OK) {
            printf("csi_adc_start failed\n");
            goto err_uninit;
        }

        value = csi_adc_read(&adc);
        printf("adc get value: %d\n", value);

        csi_adc_stop(&adc);

    } else {
        /* --- Interrupt Mode (New) --- */
        g_adc_complete_flag = 0;
        g_recv_buf[0] = 0;

        // a. Configure Buffer (Required for cvi_adc_start_async)
        ret = csi_adc_set_buffer(&adc, g_recv_buf, 1);
        if (ret != CSI_OK) {
            printf("csi_adc_set_buffer failed\n");
            goto err_uninit;
        }

        // b. Attach Callback (Enables IRQ internally in driver)
        ret = csi_adc_attach_callback(&adc, user_adc_callback, NULL);
        if (ret != CSI_OK) {
            printf("csi_adc_attach_callback failed\n");
            goto err_uninit;
        }

        // c. Start Async
        ret = csi_adc_start_async(&adc);
        if (ret != CSI_OK) {
            printf("csi_adc_start_async failed\n");
            goto err_detach;
        }

        printf("ADC started async, waiting for IRQ...\n");

        // d. Wait for callback (Timeout protection recommended)
        int timeout_ms = 2000;
        while (!g_adc_complete_flag && timeout_ms > 0) {
            aos_msleep(10); // Sleep 10ms
            timeout_ms -= 10;
        }

        if (g_adc_complete_flag) {
            printf("adc get value (async): %d\n", g_recv_buf[0]);
        } else {
            printf("adc timeout! IRQ did not trigger.\n");
        }

        csi_adc_stop_async(&adc);

err_detach:
        csi_adc_detach_callback(&adc);
    }

    printf("test adc success.\r\n");

err_uninit:
    csi_adc_uninit(&adc);
}
ALIOS_CLI_CMD_REGISTER(test_adc, testadc, test adc function);

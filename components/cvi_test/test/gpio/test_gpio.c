/*
 * GPIO Test Program (AliOS)
 *
 * Features:
 *   1. Output test
 *   2. Input test
 *   3. Toggle test
 *   4. Interrupt test (irq)
 *   5. Loopback test - requires two GPIOs physically connected
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <drv/gpio.h>
#include <drv/irq.h>
#include <drv/pin.h>
#include <aos/cli.h>
#include <aos/kernel.h>
#include "cvi_gpio.h"
#include "pinctrl-mars.h"

#define GPIO_PIN_MASK(_gpio_num) (1U << (_gpio_num))

/* ========== Pinmux auto-configuration ========== */
/*
 * Auto-configure pinmux to GPIO function before use.
 * Add new entries here when testing other GPIO pins.
 */
static void set_gpio_pinmux(unsigned int bank, unsigned int pin)
{
	/* Bank 0: GPIOA */
	if (bank == 0) {
		switch (pin) {
		case 28: PINMUX_CONFIG(IIC0_SCL, XGPIOA_28); break;
		case 29: PINMUX_CONFIG(IIC0_SDA, XGPIOA_29); break;
		/* Add more GPIOA pins as needed, e.g.:
		 * case 18: PINMUX_CONFIG(JTAG_CPU_TCK, XGPIOA_18); break;
		 * case 19: PINMUX_CONFIG(JTAG_CPU_TMS, XGPIOA_19); break;
		 */
		default: return;  /* unknown pin, skip silently */
		}
	/* Bank 1: GPIOB */
	} else if (bank == 1) {
		switch (pin) {
		/* Add GPIOB pins as needed, e.g.:
		 * case 0: PINMUX_CONFIG(PWM0_BUCK, XGPIOB_0); break;
		 */
		default: return;
		}
	/* Bank 2: GPIOC */
	} else if (bank == 2) {
		switch (pin) {
		/* Add GPIOC pins as needed, e.g.:
		 * case 2: PINMUX_CONFIG(PAD_MIPIRX4N, XGPIOC_2); break;
		 */
		default: return;
		}
	} else {
		return;
	}
	aos_cli_printf("[PINMUX] bank%u pin%u -> GPIO mode\r\n", bank, pin);
}

/* ========== IRQ test globals ========== */
static volatile uint32_t g_irq_triggered = 0;
static volatile uint32_t g_irq_pin = 0;

static void gpio_irq_callback(csi_gpio_t *gpio, uint32_t pins, void *arg)
{
	(void)gpio;
	(void)arg;
	g_irq_triggered = 1;
	g_irq_pin = pins;
}

/* ========== Usage ========== */
static void print_usage(void)
{
	aos_cli_printf("\r\n");
	aos_cli_printf("========================================\r\n");
	aos_cli_printf("  GPIO Test Tool (AliOS)\r\n");
	aos_cli_printf("========================================\r\n");
	aos_cli_printf("Usage:\r\n");
	aos_cli_printf("  testgpio o <bank> <pin> <value>           - output test\r\n");
	aos_cli_printf("  testgpio i <bank> <pin>                   - input test\r\n");
	aos_cli_printf("  testgpio toggle <bank> <pin> <ms> <count> - toggle test\r\n");
	aos_cli_printf("  testgpio irq <bank> <pin> <mode> [timeout]  - interrupt test\r\n");
	aos_cli_printf("  testgpio loopback <out_bank> <out_pin> <in_bank> <in_pin>\r\n");
	aos_cli_printf("                                            - loopback test\r\n");
	aos_cli_printf("\r\n");
	aos_cli_printf("Parameters:\r\n");
	aos_cli_printf("  bank   : GPIO bank index (0, 1, 2, ...)\r\n");
	aos_cli_printf("  pin    : GPIO pin number (0~31)\r\n");
	aos_cli_printf("  value  : output value (0 or 1)\r\n");
	aos_cli_printf("  ms     : toggle interval (milliseconds)\r\n");
	aos_cli_printf("  count  : toggle count\r\n");
	aos_cli_printf("  mode   : interrupt trigger mode\r\n");
	aos_cli_printf("           rising  - rising edge\r\n");
	aos_cli_printf("           falling - falling edge\r\n");
	aos_cli_printf("  timeout: interrupt wait timeout (seconds, default 5)\r\n");
	aos_cli_printf("\r\n");
	aos_cli_printf("Examples:\r\n");
	aos_cli_printf("  testgpio o 0 18 1\r\n");
	aos_cli_printf("  testgpio i 0 18\r\n");
	aos_cli_printf("  testgpio toggle 0 18 100 20\r\n");
	aos_cli_printf("  testgpio irq 0 18 rising 5\r\n");
	aos_cli_printf("  testgpio loopback 0 28 0 29\r\n");
	aos_cli_printf("\r\n");
}

/* ========== Sub-command: output test ========== */
static void cmd_output(int32_t argc, char **argv)
{
	/* testgpio o <bank> <pin> <value> */
	if (argc < 5) {
		aos_cli_printf("Usage: testgpio o <bank> <pin> <value>\r\n");
		return;
	}

	csi_gpio_t gpio = {0};
	unsigned int gpio_bank = atoi(argv[2]);
	unsigned int gpio_pin  = atoi(argv[3]);
	unsigned int gpio_val  = atoi(argv[4]);

	set_gpio_pinmux(gpio_bank, gpio_pin);

	if (csi_gpio_init(&gpio, gpio_bank) != CSI_OK) {
		aos_cli_printf("[FAIL] csi_gpio_init(bank=%u) failed\r\n", gpio_bank);
		return;
	}

	if (csi_gpio_dir(&gpio, GPIO_PIN_MASK(gpio_pin), GPIO_DIRECTION_OUTPUT) != CSI_OK) {
		aos_cli_printf("[FAIL] csi_gpio_dir failed\r\n");
		csi_gpio_uninit(&gpio);
		return;
	}

	csi_gpio_write(&gpio, GPIO_PIN_MASK(gpio_pin), gpio_val ? GPIO_PIN_HIGH : GPIO_PIN_LOW);
	aos_cli_printf("[PASS] GPIO bank%u pin%u output %u\r\n", gpio_bank, gpio_pin, gpio_val);

	csi_gpio_uninit(&gpio);
}

/* ========== Sub-command: input test ========== */
static void cmd_input(int32_t argc, char **argv)
{
	/* testgpio i <bank> <pin> */
	if (argc < 4) {
		aos_cli_printf("Usage: testgpio i <bank> <pin>\r\n");
		return;
	}

	csi_gpio_t gpio = {0};
	unsigned int gpio_bank = atoi(argv[2]);
	unsigned int gpio_pin  = atoi(argv[3]);

	set_gpio_pinmux(gpio_bank, gpio_pin);

	if (csi_gpio_init(&gpio, gpio_bank) != CSI_OK) {
		aos_cli_printf("[FAIL] csi_gpio_init(bank=%u) failed\r\n", gpio_bank);
		return;
	}

	if (csi_gpio_dir(&gpio, GPIO_PIN_MASK(gpio_pin), GPIO_DIRECTION_INPUT) != CSI_OK) {
		aos_cli_printf("[FAIL] csi_gpio_dir failed\r\n");
		csi_gpio_uninit(&gpio);
		return;
	}

	uint32_t val = csi_gpio_read(&gpio, GPIO_PIN_MASK(gpio_pin));
	unsigned int level = (val != 0U) ? 1 : 0;
	aos_cli_printf("[PASS] GPIO bank%u pin%u input value: %u\r\n", gpio_bank, gpio_pin, level);

	csi_gpio_uninit(&gpio);
}

/* ========== Sub-command: toggle test ========== */
static void cmd_toggle(int32_t argc, char **argv)
{
	/* testgpio toggle <bank> <pin> <delay_ms> <count> */
	if (argc < 6) {
		aos_cli_printf("Usage: testgpio toggle <bank> <pin> <delay_ms> <count>\r\n");
		return;
	}

	csi_gpio_t gpio = {0};
	unsigned int gpio_bank = atoi(argv[2]);
	unsigned int gpio_pin  = atoi(argv[3]);
	unsigned int delay_ms  = atoi(argv[4]);
	unsigned int count     = atoi(argv[5]);

	set_gpio_pinmux(gpio_bank, gpio_pin);

	if (csi_gpio_init(&gpio, gpio_bank) != CSI_OK) {
		aos_cli_printf("[FAIL] csi_gpio_init(bank=%u) failed\r\n", gpio_bank);
		return;
	}

	if (csi_gpio_dir(&gpio, GPIO_PIN_MASK(gpio_pin), GPIO_DIRECTION_OUTPUT) != CSI_OK) {
		aos_cli_printf("[FAIL] csi_gpio_dir failed\r\n");
		csi_gpio_uninit(&gpio);
		return;
	}

	aos_cli_printf("Start toggle: bank%u pin%u, interval %u ms, count %u\r\n",
	               gpio_bank, gpio_pin, delay_ms, count);

	for (unsigned int i = 0; i < count; i++) {
		csi_gpio_toggle(&gpio, GPIO_PIN_MASK(gpio_pin));
		uint32_t val = csi_gpio_read(&gpio, GPIO_PIN_MASK(gpio_pin));
		aos_cli_printf("  toggle[%u]: level=%u\r\n", i + 1, (val != 0U) ? 1 : 0);
		aos_msleep(delay_ms);
	}

	aos_cli_printf("[PASS] GPIO bank%u pin%u toggle done\r\n", gpio_bank, gpio_pin);
	csi_gpio_uninit(&gpio);
}

/* ========== Sub-command: interrupt test ========== */
static void cmd_irq(int32_t argc, char **argv)
{
	/* testgpio irq <bank> <pin> <mode> [timeout_s] */
	if (argc < 5) {
		aos_cli_printf("Usage: testgpio irq <bank> <pin> <rising|falling> [timeout_s]\r\n");
		return;
	}

	csi_gpio_t gpio = {0};
	unsigned int gpio_bank = atoi(argv[2]);
	unsigned int gpio_pin  = atoi(argv[3]);
	const char *mode_str   = argv[4];
	unsigned int timeout_s = (argc >= 6) ? atoi(argv[5]) : 5;

	csi_gpio_irq_mode_t irq_mode;
	if (strcmp(mode_str, "rising") == 0) {
		irq_mode = GPIO_IRQ_MODE_RISING_EDGE;
	} else if (strcmp(mode_str, "falling") == 0) {
		irq_mode = GPIO_IRQ_MODE_FALLING_EDGE;
	} else {
		aos_cli_printf("[FAIL] Unsupported irq mode: %s (available: rising, falling)\r\n", mode_str);
		return;
	}

	set_gpio_pinmux(gpio_bank, gpio_pin);

	if (csi_gpio_init(&gpio, gpio_bank) != CSI_OK) {
		aos_cli_printf("[FAIL] csi_gpio_init(bank=%u) failed\r\n", gpio_bank);
		return;
	}

	if (csi_gpio_dir(&gpio, GPIO_PIN_MASK(gpio_pin), GPIO_DIRECTION_INPUT) != CSI_OK) {
		aos_cli_printf("[FAIL] csi_gpio_dir failed\r\n");
		csi_gpio_uninit(&gpio);
		return;
	}

	if (csi_gpio_irq_mode(&gpio, GPIO_PIN_MASK(gpio_pin), irq_mode) != CSI_OK) {
		aos_cli_printf("[FAIL] csi_gpio_irq_mode failed\r\n");
		csi_gpio_uninit(&gpio);
		return;
	}

	g_irq_triggered = 0;
	g_irq_pin = 0;

	if (csi_gpio_irq_register(&gpio, GPIO_PIN_MASK(gpio_pin), gpio_irq_callback, NULL) != CSI_OK) {
		aos_cli_printf("[FAIL] csi_gpio_irq_register failed\r\n");
		csi_gpio_uninit(&gpio);
		return;
	}

	if (csi_gpio_irq_enable(&gpio, GPIO_PIN_MASK(gpio_pin), true) != CSI_OK) {
		aos_cli_printf("[FAIL] csi_gpio_irq_enable failed\r\n");
		csi_gpio_irq_unregister(&gpio, GPIO_PIN_MASK(gpio_pin), NULL);
		csi_gpio_uninit(&gpio);
		return;
	}

	aos_cli_printf("Waiting for %s edge interrupt (bank%u pin%u), timeout %u s...\r\n",
	               mode_str, gpio_bank, gpio_pin, timeout_s);

	unsigned int elapsed_ms = 0;
	unsigned int timeout_ms = timeout_s * 1000;
	while (!g_irq_triggered && elapsed_ms < timeout_ms) {
		aos_msleep(10);
		elapsed_ms += 10;
	}

	csi_gpio_irq_enable(&gpio, GPIO_PIN_MASK(gpio_pin), false);
	csi_gpio_irq_unregister(&gpio, GPIO_PIN_MASK(gpio_pin), NULL);

	if (g_irq_triggered) {
		aos_cli_printf("[PASS] %s edge interrupt triggered (pin_mask=0x%x, %u ms)\r\n",
		               mode_str, g_irq_pin, elapsed_ms);
	} else {
		aos_cli_printf("[FAIL] %s edge interrupt timeout (not triggered within %u s)\r\n",
		               mode_str, timeout_s);
	}

	csi_gpio_uninit(&gpio);
}

/* ========== Sub-command: loopback test ========== */
static void cmd_loopback(int32_t argc, char **argv)
{
	/* testgpio loopback <out_bank> <out_pin> <in_bank> <in_pin> */
	if (argc < 6) {
		aos_cli_printf("Usage: testgpio loopback <out_bank> <out_pin> <in_bank> <in_pin>\r\n");
		return;
	}

	unsigned int out_bank = atoi(argv[2]);
	unsigned int out_pin  = atoi(argv[3]);
	unsigned int in_bank  = atoi(argv[4]);
	unsigned int in_pin   = atoi(argv[5]);

	/* Auto-configure pinmux for both output and input pins */
	set_gpio_pinmux(out_bank, out_pin);
	set_gpio_pinmux(in_bank, in_pin);

	csi_gpio_t gpio_out = {0};
	csi_gpio_t gpio_in_obj = {0};
	csi_gpio_t *gpio_in = NULL;
	int same_bank = (out_bank == in_bank);
	int test_pass = 1;
	int total = 0, passed = 0;

	/* Init output GPIO */
	if (csi_gpio_init(&gpio_out, out_bank) != CSI_OK) {
		aos_cli_printf("[FAIL] csi_gpio_init(out bank=%u) failed\r\n", out_bank);
		return;
	}

	/* Init input GPIO: reuse handle pointer if same bank */
	if (same_bank) {
		gpio_in = &gpio_out;
	} else {
		if (csi_gpio_init(&gpio_in_obj, in_bank) != CSI_OK) {
			aos_cli_printf("[FAIL] csi_gpio_init(in bank=%u) failed\r\n", in_bank);
			csi_gpio_uninit(&gpio_out);
			return;
		}
		gpio_in = &gpio_in_obj;
	}

	/* Set direction */
	if (csi_gpio_dir(&gpio_out, GPIO_PIN_MASK(out_pin), GPIO_DIRECTION_OUTPUT) != CSI_OK) {
		aos_cli_printf("[FAIL] Set output direction failed\r\n");
		goto cleanup;
	}
	if (csi_gpio_dir(gpio_in, GPIO_PIN_MASK(in_pin), GPIO_DIRECTION_INPUT) != CSI_OK) {
		aos_cli_printf("[FAIL] Set input direction failed\r\n");
		goto cleanup;
	}

	aos_cli_printf("\r\n--- Loopback Test: OUT(bank%u pin%u) -> IN(bank%u pin%u) ---\r\n",
	               out_bank, out_pin, in_bank, in_pin);

	/* Test 1: Output HIGH -> Read HIGH */
	total++;
	csi_gpio_write(&gpio_out, GPIO_PIN_MASK(out_pin), GPIO_PIN_HIGH);
	aos_msleep(10);
	{
		uint32_t val = csi_gpio_read(gpio_in, GPIO_PIN_MASK(in_pin));
		unsigned int level = (val != 0U) ? 1 : 0;
		if (level == 1) {
			aos_cli_printf("[PASS] Output HIGH -> Read HIGH\r\n");
			passed++;
		} else {
			aos_cli_printf("[FAIL] Output HIGH -> Read %u (expected 1)\r\n", level);
			test_pass = 0;
		}
	}

	/* Test 2: Output LOW -> Read LOW */
	total++;
	csi_gpio_write(&gpio_out, GPIO_PIN_MASK(out_pin), GPIO_PIN_LOW);
	aos_msleep(10);
	{
		uint32_t val = csi_gpio_read(gpio_in, GPIO_PIN_MASK(in_pin));
		unsigned int level = (val != 0U) ? 1 : 0;
		if (level == 0) {
			aos_cli_printf("[PASS] Output LOW -> Read LOW\r\n");
			passed++;
		} else {
			aos_cli_printf("[FAIL] Output LOW -> Read %u (expected 0)\r\n", level);
			test_pass = 0;
		}
	}

	/* Test 3: Toggle loopback verify */
	total++;
	csi_gpio_write(&gpio_out, GPIO_PIN_MASK(out_pin), GPIO_PIN_LOW);
	aos_msleep(10);
	{
		uint32_t val_before = csi_gpio_read(gpio_in, GPIO_PIN_MASK(in_pin));
		csi_gpio_toggle(&gpio_out, GPIO_PIN_MASK(out_pin));
		aos_msleep(10);
		uint32_t val_after = csi_gpio_read(gpio_in, GPIO_PIN_MASK(in_pin));

		unsigned int before = (val_before != 0U) ? 1 : 0;
		unsigned int after  = (val_after  != 0U) ? 1 : 0;

		if (before != after) {
			aos_cli_printf("[PASS] Toggle loopback: %u -> %u\r\n", before, after);
			passed++;
		} else {
			aos_cli_printf("[FAIL] Toggle loopback: level unchanged (%u -> %u)\r\n", before, after);
			test_pass = 0;
		}
	}

	/* Test 4: IRQ loopback - rising edge */
	total++;
	csi_gpio_write(&gpio_out, GPIO_PIN_MASK(out_pin), GPIO_PIN_LOW);
	aos_msleep(10);

	csi_gpio_irq_mode(gpio_in, GPIO_PIN_MASK(in_pin), GPIO_IRQ_MODE_RISING_EDGE);
	g_irq_triggered = 0;
	csi_gpio_irq_register(gpio_in, GPIO_PIN_MASK(in_pin), gpio_irq_callback, NULL);
	csi_gpio_irq_enable(gpio_in, GPIO_PIN_MASK(in_pin), true);
	aos_msleep(50);

	csi_gpio_write(&gpio_out, GPIO_PIN_MASK(out_pin), GPIO_PIN_HIGH);
	{
		unsigned int wait_ms = 0;
		while (!g_irq_triggered && wait_ms < 2000) {
			aos_msleep(10);
			wait_ms += 10;
		}
	}

	csi_gpio_irq_enable(gpio_in, GPIO_PIN_MASK(in_pin), false);
	csi_gpio_irq_unregister(gpio_in, GPIO_PIN_MASK(in_pin), NULL);

	if (g_irq_triggered) {
		aos_cli_printf("[PASS] Rising edge interrupt triggered\r\n");
		passed++;
	} else {
		aos_cli_printf("[FAIL] Rising edge interrupt timeout\r\n");
		test_pass = 0;
	}

	/* Test 5: IRQ loopback - falling edge */
	total++;
	/* Restore input direction (may be needed after irq test) */
	csi_gpio_dir(gpio_in, GPIO_PIN_MASK(in_pin), GPIO_DIRECTION_INPUT);
	csi_gpio_write(&gpio_out, GPIO_PIN_MASK(out_pin), GPIO_PIN_HIGH);
	aos_msleep(10);

	csi_gpio_irq_mode(gpio_in, GPIO_PIN_MASK(in_pin), GPIO_IRQ_MODE_FALLING_EDGE);
	g_irq_triggered = 0;
	csi_gpio_irq_register(gpio_in, GPIO_PIN_MASK(in_pin), gpio_irq_callback, NULL);
	csi_gpio_irq_enable(gpio_in, GPIO_PIN_MASK(in_pin), true);
	aos_msleep(50);

	csi_gpio_write(&gpio_out, GPIO_PIN_MASK(out_pin), GPIO_PIN_LOW);
	{
		unsigned int wait_ms = 0;
		while (!g_irq_triggered && wait_ms < 2000) {
			aos_msleep(10);
			wait_ms += 10;
		}
	}

	csi_gpio_irq_enable(gpio_in, GPIO_PIN_MASK(in_pin), false);
	csi_gpio_irq_unregister(gpio_in, GPIO_PIN_MASK(in_pin), NULL);

	if (g_irq_triggered) {
		aos_cli_printf("[PASS] Falling edge interrupt triggered\r\n");
		passed++;
	} else {
		aos_cli_printf("[FAIL] Falling edge interrupt timeout\r\n");
		test_pass = 0;
	}

	/* Loopback test summary */
	aos_cli_printf("\r\n--- Loopback Test Summary ---\r\n");
	aos_cli_printf("Total: %d, Passed: %d, Failed: %d\r\n", total, passed, total - passed);
	if (test_pass) {
		aos_cli_printf("[RESULT] Loopback Test PASS\r\n");
	} else {
		aos_cli_printf("[RESULT] Loopback Test FAIL\r\n");
	}

cleanup:
	csi_gpio_uninit(&gpio_out);
	if (!same_bank) {
		csi_gpio_uninit(&gpio_in_obj);
	}
}

/* ========== Main entry ========== */
void test_gpio(int32_t argc, char **argv)
{
	if (argc < 2) {
		print_usage();
		return;
	}

	if (strcmp(argv[1], "o") == 0) {
		cmd_output(argc, argv);
	} else if (strcmp(argv[1], "i") == 0) {
		cmd_input(argc, argv);
	} else if (strcmp(argv[1], "toggle") == 0) {
		cmd_toggle(argc, argv);
	} else if (strcmp(argv[1], "irq") == 0) {
		cmd_irq(argc, argv);
	} else if (strcmp(argv[1], "loopback") == 0) {
		cmd_loopback(argc, argv);
	} else if (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "-h") == 0) {
		print_usage();
	} else {
		aos_cli_printf("Unknown command: %s\r\n", argv[1]);
		print_usage();
	}
}
ALIOS_CLI_CMD_REGISTER(test_gpio, testgpio, test gpio function);

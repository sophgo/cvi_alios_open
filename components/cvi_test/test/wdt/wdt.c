#include <csi_core.h>
#include "aos/cli.h"
#include <aos/kernel.h>
#include <drv/wdt.h>

static void cvi_wdt_callback(csi_wdt_t *wdt,  void *arg)
{
	// unreachable
}

void cvi_wdt_test(int id, int stop)
{
	csi_wdt_t hd;
	csi_error_t ret;

	bool running;
	uint32_t left_time;
	int i = 0;

	printf("wdt%d autotest\n", id);

	ret = csi_wdt_init(&hd, id);
	if (ret) {
		printf("wdt init failed\n");
		return;
	}

	csi_wdt_set_timeout(&hd, 10000);

	csi_wdt_attach_callback(&hd, &cvi_wdt_callback, NULL); //unsupported

	csi_wdt_start(&hd);

	running = csi_wdt_is_running(&hd);
	if (!running) {
		printf("wdt isn't running\n");
		return;
	}

	while (i++ < 4) {
		left_time = csi_wdt_get_remaining_time(&hd);
		printf("unfeed left time: %d\n", left_time);
		aos_msleep(1000);
	}

	while (i++ < 8) {
		csi_wdt_feed(&hd);
		left_time = csi_wdt_get_remaining_time(&hd);
		printf("feed left time: %d\n", left_time);
		aos_msleep(1000);
	}

	if (stop) {
		csi_wdt_stop(&hd);
	}

	csi_wdt_uninit(&hd);
}

void autotest_wdt(int32_t argc, char **argv)
{
	int testcase;

	if (argc < 2) {
		printf("param invailed\n");
		return;
	}

	testcase = atoi(argv[1]);
	switch (testcase) {
	case 1: {
		cvi_wdt_test(0, 1);
		cvi_wdt_test(1, 1);
		cvi_wdt_test(2, 1);
		cvi_wdt_test(3, 1);
		cvi_wdt_test(4, 1);
		break;
	}
	case 2: {
		cvi_wdt_test(1, 0);
		break;
	}
	default:
		break;
	}
}

ALIOS_CLI_CMD_REGISTER(autotest_wdt, autotest_wdt, wdt autotest);

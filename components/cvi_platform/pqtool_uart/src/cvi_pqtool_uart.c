#include <unistd.h>
#include <aos/kernel.h>
#include <aos/cli.h>
#include <aos/console_uart.h>
#include <devices/uart.h>
#include "board.h"

#include "cvi_ispd2.h"
#include "raw_dump.h"

#include "cvi_pqtool_uart.h"

static uint16_t g_pqtool_uart_buf_size = 128;
// static aos_mutex_t g_pqtool_uart_mutex_handle;
static rvm_dev_t *g_pqtool_uart_handle;
static int g_console_need_restart = 0;

int get_console_need_restart(void)
{
	return g_console_need_restart;
}

int set_console_need_restart(int value)
{
	g_console_need_restart = value;
	return true;
}

void pqtool_uart_init(char *devname, uint32_t baud, uint16_t buf_size)
{
	rvm_hal_uart_config_t config;

	// aos_mutex_new(&g_pqtool_uart_mutex_handle);
	g_pqtool_uart_handle = rvm_hal_uart_open(devname);
	if (g_pqtool_uart_handle) {
		config.baud_rate = baud;
		config.mode = MODE_TX_RX;
		config.flow_control = FLOW_CONTROL_DISABLED;
		config.stop_bits = STOP_BITS_1;
		config.parity = PARITY_NONE;
		config.data_width = DATA_WIDTH_8BIT;
		rvm_hal_uart_config(g_pqtool_uart_handle, &config);
		rvm_hal_uart_set_type(g_pqtool_uart_handle, UART_TYPE_SYNC);
		g_pqtool_uart_buf_size = buf_size;
		printf("sucsess open %s!\n", devname);
	} else {
		devname[0] = 0;
		printf("fail to open %s!\n", devname);
	}
}

void pqtool_uart_deinit(void)
{
	if (g_pqtool_uart_handle) {
		rvm_hal_uart_close(g_pqtool_uart_handle);
		g_pqtool_uart_handle = NULL;
	}
	// aos_mutex_free(&g_pqtool_uart_mutex_handle);
}

void *pqtool_get_uart(void)
{
	return (void *)g_pqtool_uart_handle;
}

static void cmd_pqtool_uart(char *wbuf, int wbuf_len, int argc, char **argv)
{
#if CONFIG_PQTOOL_SUPPORT
	isp_daemon2_uninit();
	// cvi_raw_dump_uninit();
	printf("closed pqtool app.\n");
#endif

	int idx = 0;
	char *name = "uart";
	char pqtool_uart_devname[16] = {0};

	if (argc == 2) {
		idx = atoi(argv[1]);
	} else if (argc == 3) {
		name = argv[1];
		idx = atoi(argv[2]);
	}

	printf("starting pqtool app by uart...\n");

	sprintf(pqtool_uart_devname, "%s%d", name, idx);

	if (strcmp(console_get_devname(), pqtool_uart_devname) == 0) {
		console_deinit();
		set_console_need_restart(true);
	}

	pqtool_uart_init(pqtool_uart_devname, 115200, 512);

	if (g_pqtool_uart_handle)
		isp_daemon2_uart_init();

	return;
}

void cli_reg_cmd_pqtool_uart(void)
{
	static const struct cli_command cmd_info = {
		"pqtool_uart",
		"run pqtool app by uart",
		cmd_pqtool_uart
	};

	aos_cli_register_command(&cmd_info);
}

// -----------------------------------------------------------------------------
int write_uart(bool isConnectionByUart, int fd, const void *buf, int size)
{
	int send_len = 0;

	if (isConnectionByUart) {
		send_len =  rvm_hal_uart_send(g_pqtool_uart_handle, buf, size, AOS_WAIT_FOREVER);
	} else {
		send_len = write(fd, buf, size);
	}

	return send_len;
}

int receive_uart(void *data, uint32_t size, uint32_t timeout_ms)
{
	return rvm_hal_uart_recv(g_pqtool_uart_handle, data, size, timeout_ms);
}
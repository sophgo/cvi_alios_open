#include <aos/cli.h>
// #include "drv_usbd_class.h"
#include "devices/impl/uart_impl.h"
#include <sys/prctl.h>
#include <pthread.h>

struct cdc_acm_ctx {
	rvm_dev_t *dev;
	pthread_t thread_id;
	int run_flag;
};

static struct cdc_acm_ctx s_cdc_acm_ctx;

static void *test_cdc_acm_uart_recv(void *args)
{
	struct cdc_acm_ctx *ctx = (struct cdc_acm_ctx *)args;
	rvm_dev_t *dev = ctx->dev;
	uint8_t read_buf[64] = {0};
	uint32_t read_buf_len = 0;

	prctl(PR_SET_NAME, "test_cdc_acm_uart_recv");

	printf("uart recv start\n");

	while (ctx->run_flag) {
		read_buf_len = 0;
		read_buf_len = rvm_hal_uart_recv(dev, read_buf, sizeof(read_buf), 500);
		if (read_buf_len > 0) {
			// handle data
			printf("recv_len:%d\n", read_buf_len);

			for (int i = 0; i < read_buf_len; i++) {
				printf("%x ", read_buf[i]);
			}
			printf("\n\n");

			memset(read_buf, 0, sizeof(read_buf));
		} else {
			aos_msleep(100);
		}
	}

	printf("uart recv exit\n");
	return 0;
}

static void test_cdc_acm_uart_send(int argc, char **argv)
{
	if (!s_cdc_acm_ctx.dev) {
		printf("cdc acm uart not init\n");
		return;
	}

	uint8_t send_buf[64] = {0};
	uint32_t size = sizeof(send_buf);
	for (int i = 0; i < size; i++) {
		send_buf[i] = i;
	}
	rvm_hal_uart_send(s_cdc_acm_ctx.dev, send_buf, size, 500);
}
ALIOS_CLI_CMD_REGISTER(test_cdc_acm_uart_send, test_cdc_acm_uart_send, test_cdc_acm_uart_send);


static void test_cdc_acm_uart_init(int argc, char **argv)
{
	if (s_cdc_acm_ctx.dev) {
		printf("cdc acm uart already init\n");
		return;
	}

	s_cdc_acm_ctx.dev = rvm_hal_device_open("usb_serial0");
	if (!s_cdc_acm_ctx.dev) {
		printf("usb_serial0 dev is null\n");
		return;
	}

	s_cdc_acm_ctx.run_flag = 1;
	if (pthread_create(&s_cdc_acm_ctx.thread_id, NULL, test_cdc_acm_uart_recv, (void *)&s_cdc_acm_ctx) != 0) {
		printf("create test_cdc_acm_uart_recv failed\n");
		return;
	}
}
ALIOS_CLI_CMD_REGISTER(test_cdc_acm_uart_init, test_cdc_acm_uart_init, test_cdc_acm_uart_init);

static void test_cdc_acm_uart_uninit(int argc, char **argv)
{
	if (!s_cdc_acm_ctx.dev) {
		printf("cdc acm uart not init\n");
		return;
	}

	s_cdc_acm_ctx.run_flag = 0;
	pthread_join(s_cdc_acm_ctx.thread_id, NULL);
	rvm_hal_device_close(s_cdc_acm_ctx.dev);
	s_cdc_acm_ctx.dev = NULL;
}
ALIOS_CLI_CMD_REGISTER(test_cdc_acm_uart_uninit, test_cdc_acm_uart_uninit, test_cdc_acm_uart_uninit);

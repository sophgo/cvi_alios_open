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
* See the License for the specific language governing permissions an
* limitations under the License.
*/

#include "drv/spi.h"
#include "board.h"
#include <aos/kernel.h>
#include <stdio.h>
#include <ulog/ulog.h>
#include <aos/cli.h>
#include "drv/spi.h"
#include "debug/debug_dumpsys.h"
#include <drv/pin.h>
#include <drv/gpio.h>
#include <posix/timer.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

extern int32_t csi_spi_send_receive(csi_spi_t *spi, const void *data_out, void *data_in, uint32_t size, uint32_t timeout);
void hex_dump(char *buf, int len, int addr);
#define DATA_LEN	128
int send_finished = 1;
int receive_finished = 1;

void hex_dump(char *buf, int len, int addr)
{
	int i, j, k, extra_len;
	char binstr[128];

	extra_len = 0;
	for (i = 0; i < len; i++) {
		if ((i % 16) == 0) {
			extra_len = 0;
			extra_len += sprintf(binstr + extra_len, "%08x -", i + addr);
			extra_len += sprintf(binstr + extra_len, " %02x", (unsigned char)buf[i]);
		} else if ((i % 16) == 15) {
			extra_len += sprintf(binstr + extra_len, " %02x", (unsigned char)buf[i]);
			extra_len += sprintf(binstr + extra_len, "  ");
			for (j = i - 15; j <= i; j++)
				extra_len += sprintf(binstr + extra_len, "%c", ('!' < buf[j] && buf[j] <= '~') ? buf[j] : '.');
			printf("%s\n", binstr);
		} else {
			extra_len += sprintf(binstr + extra_len, " %02x", (unsigned char)buf[i]);
		}
	}

	if ((i % 16) != 0) {
		k = 16 - (i % 16);
		for (j = 0; j < k; j++)
			extra_len += sprintf(binstr + extra_len, "   ");
		extra_len += sprintf(binstr + extra_len, "  ");
		k = 16 - k;
		for (j = i - k; j < i; j++)
			extra_len += sprintf(binstr + extra_len, "%c", ('!' < buf[j] && buf[j] <= '~') ? buf[j] : '.');
		printf("%s\n", binstr);
	}
}

void spi_callback_func(csi_spi_t *spi, csi_spi_event_t event, void *arg)
{
	if (event == SPI_EVENT_RECEIVE_COMPLETE)
		receive_finished = 0;

	if (event == SPI_EVENT_SEND_COMPLETE)
		send_finished = 0;
}

int config_spi_transfer(csi_spi_t *spi_handler)
{
	int spi_ret = 0;

	spi_ret = csi_spi_init(spi_handler, 0);
	if (spi_ret == CSI_ERROR) {
		printf("spi init failed returned, actual %d\n", spi_ret);
		return -1;
	}

	// test csi_spi_mode
	spi_ret = csi_spi_mode(spi_handler, SPI_MASTER);
	if (spi_ret == CSI_ERROR) {
		printf("set spi mode failed, actual ret %d\n", spi_ret);
		return -1;
	}

	// test csi_spi_cp_format
	spi_ret = csi_spi_cp_format(spi_handler, SPI_FORMAT_CPOL0_CPHA0);
	if (spi_ret == CSI_ERROR) {
		printf("set spi format failed returned, actual ret %d\n", spi_ret);
		return -1;
	}

	// test csi_spi_frame_len
	spi_ret = csi_spi_frame_len(spi_handler, SPI_FRAME_LEN_8);
	if (spi_ret == CSI_ERROR) {
		printf("set spi frame len failed, actual ret %d\n", spi_ret);
		return -1;
	}

	// test csi_spi_baud
	csi_spi_baud(spi_handler, 1000000);
	return spi_ret;
}

int32_t spi_send_receive_async(csi_spi_t *spi, const void *data_out, void *data_in, uint32_t size, uint32_t timeout)
{
	return csi_spi_send_receive_async(spi, data_out, data_in, size);
}

int32_t spi_send_receive_dma(csi_spi_t *spi, const void *data_out, void *data_in, uint32_t size, uint32_t timeout)
{
	return csi_spi_send_receive_dma(spi, data_out, data_in, size);
}

struct spi_test_function {
	char *desc;
	int32_t (*func)(csi_spi_t *spi, const void *data_out,
                void *data_in, uint32_t size, uint32_t timeout);
};

const struct spi_test_function spi_test_case[6] = {
	{ .desc = "poll mode", .func = csi_spi_send_receive },
	{ .desc = "irq mode",  .func = spi_send_receive_async },
	{ .desc = "dma mode",  .func = spi_send_receive_dma },
	{ .desc = NULL,             .func = NULL }
};

int test_spi_interface(int argc, char **argv)
{

	uint8_t send_buffer[DATA_LEN] = {0};
	uint8_t receive_buffer[DATA_LEN] = {0};
	char op[16] = {0};
	csi_error_t spi_ret = 0;
	csi_spi_t spi_handler;
	int i;
	const struct spi_test_function *t_case = spi_test_case;

	for (i = 0; i < DATA_LEN; i++)
		send_buffer[i] = i;

	printf("--------------------- show src buffer data ------------------------\n");
	hex_dump((char *)send_buffer, DATA_LEN, 0);
	printf("\n");

	for (; t_case->desc; t_case++) {
		printf("######################## start %s test ##############################\n", t_case->desc);
		spi_ret = config_spi_transfer(&spi_handler);
		if (spi_ret) {
			printf("set spi config failed, actual ret %d\n", spi_ret);
			goto un_init;
		}

		if (!strcmp(t_case->desc, "irq mode")) {
			spi_ret = csi_spi_attach_callback(&spi_handler, spi_callback_func, NULL);
			if (spi_ret) {
				printf("set spi config failed, actual ret %d\n", spi_ret);
				goto un_init;
			}
		}

		if (!strcmp(t_case->desc, "dma mode")) {

			spi_ret = csi_spi_attach_callback(&spi_handler, spi_callback_func, NULL);
			if (spi_ret) {
				printf("set spi config failed, actual ret %d\n", spi_ret);
				goto un_init;
			}

			spi_ret = csi_spi_link_dma(&spi_handler, NULL, NULL);
			if (spi_ret) {
				printf("link dma failed!\n");
				goto un_init;
			}
		}

		spi_ret = t_case->func(&spi_handler, send_buffer, receive_buffer, DATA_LEN, 1000);
		if (spi_ret) {
			printf("spi send and receive test failed, actual ret %d\n", spi_ret);
			goto un_init;
		}

		/* only for async such as irq and dma */
		if (!strcmp(t_case->desc, "irq mode") || !strcmp(t_case->desc, "dma mode")) {
			aos_msleep(4);
			printf("receive_finished:%d, send_finished:%d\n", receive_finished, send_finished);
			while((receive_finished || send_finished));
		}

			for (i = 0; i < DATA_LEN; i++) {
				if (send_buffer[i] != receive_buffer[i]) {
					strcpy(op, "failed");
					printf("data is different, at 0x%x pos\n", i);
					break;
				}
				strcpy(op, "success");
			}

		hex_dump((char *)receive_buffer, DATA_LEN, 0);
		printf("\n######################## %s test %s! ##############################\n", t_case->desc, op);
		printf("\n");
		memset(receive_buffer, 0x0, DATA_LEN);
un_init:
		csi_spi_uninit(&spi_handler);
	}

	return spi_ret;
}
ALIOS_CLI_CMD_REGISTER(test_spi_interface, spi_loopback_test, loopback test);


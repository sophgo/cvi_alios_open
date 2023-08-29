#include <stdio.h>
#include <stdlib.h>
#include <drv/tempsen.h>

void test_tempsen(int32_t argc, char **argv)
{
	csi_tempsen_t tps;

	csi_tempsen_init(&tps);
	unsigned int temp;
	for (int i = 0; i < 10; ++i) {
		temp = csi_tempsen_read_temp(&tps);
		printf("temp = %d mC\n", temp);
		mdelay(1000);
	}
	csi_tempsen_uninit(&tps);
}
ALIOS_CLI_CMD_REGISTER(test_tempsen, test_tempsen, test tempsen function);

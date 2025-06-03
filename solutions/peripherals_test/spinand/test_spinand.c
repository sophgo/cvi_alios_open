#include "board.h"
#include <aos/kernel.h>
#include <stdio.h>
#include <ulog/ulog.h>
#include <aos/cli.h>
//#include "cvi_sys.h"
#include "drv/spinand.h"
#include "drv/spi.h"
#include "debug/debug_dumpsys.h"
#include <drv/pin.h>
#include <drv/gpio.h>
#include <drv/adc.h>
#include <drv/pwm.h>
#include <drv/wdt.h>
#include <posix/timer.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include "yoc/partition.h"
#include "yoc/partition_device.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pinctrl-mars.h>
#include <cvi_spinand.h>

static void _SpiNandPinmux(void)
{
	PINMUX_CONFIG(EMMC_CLK, SPINAND_CLK);
	PINMUX_CONFIG(EMMC_CMD, SPINAND_MISO);
	PINMUX_CONFIG(EMMC_DAT1, SPINAND_CS);
	PINMUX_CONFIG(EMMC_DAT0, SPINAND_MOSI);
	PINMUX_CONFIG(EMMC_DAT2, SPINAND_HOLD);
	PINMUX_CONFIG(EMMC_DAT3, SPINAND_WP);
}

static int32_t spinand_flash_test(void)
{
	static int32_t ret = 0;


	// spinand_op_param_t *spinand_param = (spinand_op_param_t *)args;
	csi_spinand_t *spinand;
	csi_spinand_dev_params_t flash_info;

	void *gpio_cs_callback = NULL;


	spinand = malloc(sizeof(csi_spinand_t));
	ret = csi_spinand_qspi_init(spinand, 0, gpio_cs_callback);
	if(ret){
		return ret;
	}

	ret = csi_spinand_get_flash_info(spinand, &flash_info);
	if(ret<0){
		return ret;
	}
	SPINAND_PRINTK_LEVEL = SPINAND_INFO;
	spinand_printf(SPINAND_INFO, "*********************************************\n");
	spinand_printf(SPINAND_INFO, "flash_info.model_name = %s\n", flash_info.model_name);
	spinand_printf(SPINAND_INFO, "flash_info.page_size = 0x%x\n", flash_info.page_size);
	spinand_printf(SPINAND_INFO, "flash_info.oob_size = 0x%x\n", flash_info.oob_size);
	spinand_printf(SPINAND_INFO, "flash_info.pages_per_block = %d\n", flash_info.pages_per_block);
	spinand_printf(SPINAND_INFO, "flash_info.max_bad_blocks = %d\n", flash_info.max_bad_blocks);
	spinand_printf(SPINAND_INFO, "flash_info.total_blocks = %d\n", flash_info.total_blocks);
	spinand_printf(SPINAND_INFO, "********************************************\n\n");

	int32_t flash_size = flash_info.page_size*flash_info.pages_per_block*flash_info.total_blocks;
	int32_t test_buf_size = flash_info.page_size*2;//*flash_info.pages_per_block*2;
	uint64_t last_fail_addr = 0;
	spinand_printf(SPINAND_ERR, "test erase\n");
	ret = csi_spinand_erase(spinand, 0, flash_size, &last_fail_addr);
	if(ret){
		return ret;
	}
	if(last_fail_addr){
		spinand_printf(SPINAND_ERR, "error erase at 0x%x\n", last_fail_addr);
		return CSI_ERROR;
	}

	spinand_printf(SPINAND_ERR, "test write and read\n");
	uint64_t test_buf_addr = (uint64_t)malloc(test_buf_size+0x40);
	if(test_buf_addr == 0){
		spinand_printf(SPINAND_ERR, "error at malloc test_buf\n");
		return CSI_ERROR;
	}
	uint8_t *test_buf = (uint8_t *)((test_buf_addr + 0x40)&(~0x3f));
	spinand_printf(SPINAND_ERR, "test_buf addr = %p\n", test_buf);

	if(csi_spinand_read(spinand, 0, (void *)test_buf, test_buf_size) != test_buf_size){
		spinand_printf(SPINAND_ERR, "error read\n");
		return CSI_ERROR;
	}
	for(uint32_t i=0; i<test_buf_size; i++){
		if(test_buf[i] != 0xff){
			spinand_printf(SPINAND_WARN, "warning at 0x%x, val=0x%x\n", i, test_buf[i]);
			return CSI_ERROR;
		}
	}

	for(uint32_t i=0,j=0; i<test_buf_size; i++){
		if(i%flash_info.page_size ==0){
			j++;
		}
		test_buf[i] = ((i&0xff) + j);
	}
	if(csi_spinand_write(spinand, 0, (void *)test_buf, test_buf_size) != test_buf_size){
		spinand_printf(SPINAND_ERR, "error write\n");
		return CSI_ERROR;
	}

	memset(test_buf, 0x0, test_buf_size);
	if(csi_spinand_read(spinand, 0, (void *)test_buf, test_buf_size) != test_buf_size){
		spinand_printf(SPINAND_ERR, "error read\n");
		return CSI_ERROR;
	}
	for(uint32_t i=0,j=0; i<test_buf_size; i++){
		if(i%flash_info.page_size ==0){
			j++;
		}
		if(test_buf[i] != (((i&0xff) + j)&0xff)){
			spinand_printf(SPINAND_WARN, "warning read i=0x%x, val=0x%x, expect=0x%x\n", i, test_buf[i], ((i&0xff) + j));
		}
	}

	spinand_printf(SPINAND_ERR, "test write_oob and read_oob\n");
	for(uint32_t page=0; page<flash_info.pages_per_block*flash_info.total_blocks; page++){
		ret = csi_spinand_read_spare_data(spinand, page*flash_info.page_size, 0, (void *)test_buf, flash_info.oob_size);
		if(ret<0){
			return ret;
		}
		for(uint32_t i=0; i<flash_info.oob_size; i++){
			if(test_buf[i] != 0xff){
				spinand_printf(SPINAND_ERR, "warning page=0x%x, i=%d, test_buf[i]=0x%x\n", page, i, test_buf[i]);
			}
		}
	}
	
	for(uint32_t page=0; page<flash_info.pages_per_block*flash_info.total_blocks; page++){
		memset(test_buf, page, flash_info.oob_size);
		ret = csi_spinand_write_spare_data(spinand, page*flash_info.page_size, 0, (void *)test_buf, flash_info.oob_size);
		if(ret<0){
			return ret;
		}
	}
	for(uint32_t page=0; page<flash_info.pages_per_block*flash_info.total_blocks; page++){
		ret = csi_spinand_read_spare_data(spinand, page*flash_info.page_size, 0, (void *)test_buf, flash_info.oob_size);
		if(ret<0){
			return ret;
		}
		for(uint32_t i=0; i<flash_info.oob_size; i++){
			if(test_buf[i] != (page&0xff)){
				spinand_printf(SPINAND_ERR, "error page=%d, i=%d, test_buf[i]=0x%x\n", page, i, test_buf[i]);
			}
		}
	}

	spinand_printf(SPINAND_ERR, "test block mark bad\n");
	ret = csi_spinand_erase(spinand, 0, flash_size, &last_fail_addr);
	if(ret){
		return ret;
	}
	if(last_fail_addr){
		spinand_printf(SPINAND_ERR, "error erase at 0x%x\n", last_fail_addr);
		return CSI_ERROR;
	}
	for(uint32_t i=0; i<flash_info.total_blocks; i++){
		ret = csi_spinand_block_is_bad(spinand, flash_info.page_size * flash_info.pages_per_block * i);
		if(ret == 0){
			// spinand_printf(SPINAND_ERR, "block %d is good\n", i);
		}else if(ret == 1){
			spinand_printf(SPINAND_ERR, "block %d is bad\n", i);
		}else{
			spinand_printf(SPINAND_ERR, "error at block %d\n", i);
			return ret;
		}
	}

	for(uint8_t i=0; i<20; i++){
		ret = csi_spinand_block_mark_bad(spinand, flash_info.page_size * flash_info.pages_per_block * i);
		if(ret<0){
			spinand_printf(SPINAND_ERR, "block mark bad error\n");
			return ret;
		}
		ret = csi_spinand_block_is_bad(spinand, flash_info.page_size * flash_info.pages_per_block * i);
		if(ret == 0){
			// spinand_printf(SPINAND_ERR, "block %d is good\n", i);
			spinand_printf(SPINAND_ERR, "block mark bad error\n", i);
		}else if(ret == 1){
			// spinand_printf(SPINAND_ERR, "block %d is bad\n", i);
		}else{
			spinand_printf(SPINAND_ERR, "error at block %d\n", i);
			return ret;
		}
	}

	spinand_printf(SPINAND_ERR, "test erase\n");
	ret = csi_spinand_erase(spinand, 0, flash_size, &last_fail_addr);
	if(ret){
		return ret;
	}
	if(last_fail_addr){
		spinand_printf(SPINAND_ERR, "error erase at 0x%x\n", last_fail_addr);
		return CSI_ERROR;
	}

	if(csi_spinand_read(spinand, 0, (void *)test_buf, test_buf_size) != test_buf_size){
		spinand_printf(SPINAND_ERR, "error read\n");
		return CSI_ERROR;
	}
	for(uint32_t i=0; i<test_buf_size; i++){
		if(test_buf[i] != 0xff){
			spinand_printf(SPINAND_WARN, "warning at 0x%x, val=0x%x\n", i, test_buf[i]);
			return CSI_ERROR;
		}
	}
	for(uint32_t page=0; page<flash_info.pages_per_block*flash_info.total_blocks; page++){
		ret = csi_spinand_read_spare_data(spinand, page*flash_info.page_size, 0, (void *)test_buf, flash_info.oob_size);
		if(ret<0){
			return ret;
		}
		for(uint32_t i=0; i<flash_info.oob_size; i++){
			if(test_buf[i] != 0xff){
				spinand_printf(SPINAND_ERR, "warning page=%d, i=%d, test_buf[i]=0x%x\n", page, i, test_buf[i]);
			}
		}
	}

	ret = csi_spinand_reset(spinand);
	if(ret){
		return ret;
	}

	csi_spinand_qspi_uninit(spinand);
	spinand_printf(SPINAND_ERR, "exit\n");
	return ret;
// csi_error_t csi_spinand_set_xfer_mode(csi_spinand_t *spinand,csi_spinand_xfer_t xfer_mode);
// uint32_t csi_spinand_frequence(csi_spinand_t *spinand, uint32_t hz);
}


static int test_spinand_cmd(int argc, char **argv)
{
	static int32_t ret = 0;

	_SpiNandPinmux();
	ret = spinand_flash_test();

	if (ret < 0) {
		spinand_printf(SPINAND_ERR, "nand flash test failed!\n");
		return -1;
	}
	spinand_printf(SPINAND_ERR, "test success\n");

	return ret;
}
ALIOS_CLI_CMD_REGISTER(test_spinand_cmd, nand_test, nand read/write test);
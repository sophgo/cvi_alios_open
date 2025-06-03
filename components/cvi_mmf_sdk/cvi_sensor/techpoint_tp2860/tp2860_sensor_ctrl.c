#include <unistd.h>
#include <stdlib.h>
#include <cvi_comm_video.h>
#include <pthread.h>
#include <signal.h>
#include "cvi_sns_ctrl.h"
#include "tp2860_cmos_ex.h"
#include "sensor_i2c.h"

const CVI_U8 tp2860_i2c_addr = 0x45;        /* I2C slave address of tp2860, SA0=0:0x44, SA0=1:0x45*/
const CVI_U32 tp2860_addr_byte = 1;
const CVI_U32 tp2860_data_byte = 1;
//static pthread_t g_tp2860_thid;


#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF 64

#define tp2860_BLUE_SCREEN 0


enum CVI_GPIO_NUM_E {
CVI_GPIOD_00 = 404,
CVI_GPIOD_01,   CVI_GPIOD_02,   CVI_GPIOD_03,   CVI_GPIOD_04,   CVI_GPIOD_05,
CVI_GPIOD_06,   CVI_GPIOD_07,   CVI_GPIOD_08,   CVI_GPIOD_09,   CVI_GPIOD_10,
CVI_GPIOD_11,
CVI_GPIOC_00 = 416,
CVI_GPIOC_01,   CVI_GPIOC_02,   CVI_GPIOC_03,   CVI_GPIOC_04,   CVI_GPIOC_05,
CVI_GPIOC_06,   CVI_GPIOC_07,   CVI_GPIOC_08,   CVI_GPIOC_09,   CVI_GPIOC_10,
CVI_GPIOC_11,   CVI_GPIOC_12,   CVI_GPIOC_13,   CVI_GPIOC_14,   CVI_GPIOC_15,
CVI_GPIOC_16,   CVI_GPIOC_17,   CVI_GPIOC_18,   CVI_GPIOC_19,   CVI_GPIOC_20,
CVI_GPIOC_21,   CVI_GPIOC_22,   CVI_GPIOC_23,   CVI_GPIOC_24,   CVI_GPIOC_25,
CVI_GPIOC_26,   CVI_GPIOC_27,   CVI_GPIOC_28,   CVI_GPIOC_29,   CVI_GPIOC_30,
CVI_GPIOC_31,
CVI_GPIOB_00 = 448,
CVI_GPIOB_01,   CVI_GPIOB_02,   CVI_GPIOB_03,   CVI_GPIOB_04,   CVI_GPIOB_05,
CVI_GPIOB_06,   CVI_GPIOB_07,   CVI_GPIOB_08,   CVI_GPIOB_09,   CVI_GPIOB_10,
CVI_GPIOB_11,   CVI_GPIOB_12,   CVI_GPIOB_13,   CVI_GPIOB_14,   CVI_GPIOB_15,
CVI_GPIOB_16,   CVI_GPIOB_17,   CVI_GPIOB_18,   CVI_GPIOB_19,   CVI_GPIOB_20,
CVI_GPIOB_21,   CVI_GPIOB_22,   CVI_GPIOB_23,   CVI_GPIOB_24,   CVI_GPIOB_25,
CVI_GPIOB_26,   CVI_GPIOB_27,   CVI_GPIOB_28,   CVI_GPIOB_29,   CVI_GPIOB_30,
CVI_GPIOB_31,
CVI_GPIOA_00 = 480,
CVI_GPIOA_01,   CVI_GPIOA_02,   CVI_GPIOA_03,   CVI_GPIOA_04,   CVI_GPIOA_05,
CVI_GPIOA_06,   CVI_GPIOA_07,   CVI_GPIOA_08,   CVI_GPIOA_09,   CVI_GPIOA_10,
CVI_GPIOA_11,   CVI_GPIOA_12,   CVI_GPIOA_13,   CVI_GPIOA_14,   CVI_GPIOA_15,
CVI_GPIOA_16,   CVI_GPIOA_17,   CVI_GPIOA_18,   CVI_GPIOA_19,   CVI_GPIOA_20,
CVI_GPIOA_21,   CVI_GPIOA_22,   CVI_GPIOA_23,   CVI_GPIOA_24,   CVI_GPIOA_25,
CVI_GPIOA_26,   CVI_GPIOA_27,   CVI_GPIOA_28,   CVI_GPIOA_29,   CVI_GPIOA_30,
CVI_GPIOA_31,
};

#define CVI_GPIO_MIN CVI_GPIOD_00
#define CVI_GPIO_MAX CVI_GPIOA_31

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF 64
static int TP2860_GPIO_Export(unsigned int gpio)
{
	int fd, len;
	char buf[MAX_BUF];

	fd = open(SYSFS_GPIO_DIR"/export", O_WRONLY);
	if (fd < 0) {
		perror("gpio/export");
		return fd;
	}

	len = snprintf(buf, sizeof(buf), "%d", gpio);
	write(fd, buf, len);
	close(fd);

	return 0;
}

static int TP2860_GPIO_SetDirection(unsigned int gpio, unsigned int out_flag)
{
	int fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR"/gpio%d/direction", gpio);
	if (access(buf, 0) == -1)
		TP2860_GPIO_Export(gpio);

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/direction");
		return fd;
	}
	//printf("mark %d , %s\n",out_flag, buf);
	if (out_flag)
		write(fd, "out", 4);
	else
		write(fd, "in", 3);

	close(fd);
	return 0;
}
static int TP2860_GPIO_SetValue(unsigned int gpio, unsigned int value)
{
	int fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR"/gpio%d/value", gpio);
	if (access(buf, 0) == -1)
		TP2860_GPIO_Export(gpio);

	TP2860_GPIO_SetDirection(gpio, 1); //output

	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		perror("gpio/set-value");
		return fd;
	}

	if (value)
		write(fd, "1", 2);
	else
		write(fd, "0", 2);

	close(fd);
	return 0;
}

int tp2860_sys_init(VI_PIPE ViPipe)
{
	(void) ViPipe;

	//PR2K_RST
	if (TP2860_GPIO_SetValue(CVI_GPIOB_12, 1) != 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "set reset gpio error!\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

int tp2860_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_auntp2860_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_auntp2860_AddrInfo[ViPipe].s8I2cAddr);
}

int tp2860_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_auntp2860_BusInfo[ViPipe].s8I2cDev);
}

int tp2860_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_auntp2860_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_auntp2860_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							tp2860_addr_byte, tp2860_data_byte);
}

int tp2860_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_auntp2860_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_auntp2860_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							tp2860_i2c_addr, (CVI_U32)data, tp2860_addr_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void tp2860_init_setting(VI_PIPE ViPipe, CVI_U8 mode)
{

	if (mode == TP2860_MODE_1080P_25P) {
		tp2860_write_register(ViPipe, 0x40, 0x00); //select decoder page
		tp2860_write_register(ViPipe, 0x42, 0x00); //common setting for all format
		tp2860_write_register(ViPipe, 0x4c, 0x43); //common setting for all format
		tp2860_write_register(ViPipe, 0x4e, 0x1d); //common setting for dvp output
		tp2860_write_register(ViPipe, 0x54, 0x04); //common setting for dvp output
		tp2860_write_register(ViPipe, 0xf6, 0x00); //common setting for all format
		tp2860_write_register(ViPipe, 0xf7, 0x44); //common setting for dvp output
		tp2860_write_register(ViPipe, 0xfa, 0x00); //common setting for dvp output
		tp2860_write_register(ViPipe, 0x1b, 0x01); //common setting for dvp output
		tp2860_write_register(ViPipe, 0x41, 0x00); //video MUX select
		tp2860_write_register(ViPipe, 0x40, 0x08); //common setting for all format
		tp2860_write_register(ViPipe, 0x13, 0xef); //common setting for dvp output
		tp2860_write_register(ViPipe, 0x14, 0x41); //common setting for dvp output
		tp2860_write_register(ViPipe, 0x15, 0x02); //common setting for dvp output
		tp2860_write_register(ViPipe, 0x40, 0x00); //select decoder page
		tp2860_write_register(ViPipe, 0x40, 0x08); //select mipi page
		tp2860_write_register(ViPipe, 0x12, 0x54);
		tp2860_write_register(ViPipe, 0x13, 0xef);
		tp2860_write_register(ViPipe, 0x14, 0x41);
		tp2860_write_register(ViPipe, 0x15, 0x02);
		tp2860_write_register(ViPipe, 0x23, 0x1E); //common setting for dvp output
		tp2860_write_register(ViPipe, 0x40, 0x00);
		tp2860_write_register(ViPipe, 0x40, 0x00); //back to decoder page
		tp2860_write_register(ViPipe, 0x40, 0x00); //back to decoder page
		tp2860_write_register(ViPipe, 0x02, 0xc8);
		tp2860_write_register(ViPipe, 0x07, 0xc0);
		tp2860_write_register(ViPipe, 0x0b, 0xc0);
		tp2860_write_register(ViPipe, 0x0c, 0x03);
		tp2860_write_register(ViPipe, 0x0d, 0x50);
		tp2860_write_register(ViPipe, 0x15, 0x03);
		tp2860_write_register(ViPipe, 0x16, 0xd2);
		tp2860_write_register(ViPipe, 0x17, 0x80);
		tp2860_write_register(ViPipe, 0x18, 0x29);
		tp2860_write_register(ViPipe, 0x19, 0x38);
		tp2860_write_register(ViPipe, 0x1a, 0x47);
		tp2860_write_register(ViPipe, 0x1c, 0x0a);  //1920*1080, 25fps
		tp2860_write_register(ViPipe, 0x1d, 0x50);
		tp2860_write_register(ViPipe, 0x20, 0x30);
		tp2860_write_register(ViPipe, 0x21, 0x84);
		tp2860_write_register(ViPipe, 0x22, 0x36);
		tp2860_write_register(ViPipe, 0x23, 0x3c);
		tp2860_write_register(ViPipe, 0x2b, 0x60);
		tp2860_write_register(ViPipe, 0x2c, 0x2a);
		tp2860_write_register(ViPipe, 0x2d, 0x30);
		tp2860_write_register(ViPipe, 0x2e, 0x70);
		tp2860_write_register(ViPipe, 0x30, 0x48);
		tp2860_write_register(ViPipe, 0x31, 0xbb);
		tp2860_write_register(ViPipe, 0x32, 0x2e);
		tp2860_write_register(ViPipe, 0x33, 0x90);
		tp2860_write_register(ViPipe, 0x35, 0x05);
		tp2860_write_register(ViPipe, 0x38, 0x00);
		tp2860_write_register(ViPipe, 0x39, 0x1C);
		tp2860_write_register(ViPipe, 0x02, 0xcc);
		tp2860_write_register(ViPipe, 0x0d, 0x73);
		tp2860_write_register(ViPipe, 0x15, 0x01);
		tp2860_write_register(ViPipe, 0x16, 0xf0);
		tp2860_write_register(ViPipe, 0x18, 0x2a);
		tp2860_write_register(ViPipe, 0x20, 0x3c);
		tp2860_write_register(ViPipe, 0x21, 0x46);
		tp2860_write_register(ViPipe, 0x25, 0xfe);
		tp2860_write_register(ViPipe, 0x26, 0x0d);
		tp2860_write_register(ViPipe, 0x2c, 0x3a);
		tp2860_write_register(ViPipe, 0x2d, 0x54);
		tp2860_write_register(ViPipe, 0x2e, 0x40);
		tp2860_write_register(ViPipe, 0x30, 0xa5);
		tp2860_write_register(ViPipe, 0x31, 0x86);
		tp2860_write_register(ViPipe, 0x32, 0xfb);
		tp2860_write_register(ViPipe, 0x33, 0x60);
#if TP2860_BLUE_SCREEN
		tp2860_write_register(ViPipe, 0x40, 0x00);
		tp2860_write_register(ViPipe, 0x2A, 0x3C);
#endif
	printf("ViPipe:%d,===tp2860 1080P 25fps 10bit LINE Init OK!===\n", ViPipe);
	}

}

void tp2860_init(VI_PIPE ViPipe)
{
	if (tp2860_sys_init(ViPipe) != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "TP2860 sys init fail\n");
		return;
	}

	delay_ms(20);

	if (tp2860_i2c_init(ViPipe) != CVI_SUCCESS) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "TP2860 i2c init fail\n");
		return;
	}

	CVI_TRACE_SNS(CVI_DBG_DEBUG, "Loading Techpoint tp2860 sensor\n");

	//check sensor chip id
	tp2860_write_register(ViPipe, 0x40, 0x0);
	if (tp2860_read_register(ViPipe, 0xfe) != 0x28 ||
		tp2860_read_register(ViPipe, 0xff) != 0x60) {
		CVI_TRACE_SNS(CVI_DBG_DEBUG, "read tp2860 chip id fail\n");
		return;
	}
	if (g_pasttp2860[ViPipe]->u8ImgMode == TP2860_MODE_1080P_25P)
		CVI_TRACE_SNS(CVI_DBG_DEBUG, "Techpoint tp2860 1080 25FPS\n");

	tp2860_init_setting(ViPipe, g_pasttp2860[ViPipe]->u8ImgMode);
	delay_ms(800);


}



#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>
#include "cv2002_cmos_ex.h"

#define CV2002_CHIP_ID_ADDR_H	0x3003
#define CV2002_CHIP_ID_ADDR_L	0x3002
#define CV2002_CHIP_ID			0x2004

static void cv2002_linear_1080P30_init(VI_PIPE ViPipe);
static void cv2002_linear_720P30_init(VI_PIPE ViPipe);
static void cv2002_wdr_1080p30_2to1_init(VI_PIPE ViPipe);
static void cv2002_wdr_720p30_2to1_init(VI_PIPE ViPipe);

CVI_U8 cv2002_i2c_addr = 0x36;
const CVI_U32 cv2002_addr_byte = 2;
const CVI_U32 cv2002_data_byte = 1;
static int g_fd[VI_MAX_PIPE_NUM] = {[0 ... (VI_MAX_PIPE_NUM - 1)] = -1};

int cv2002_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunCV2002_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int cv2002_i2c_exit(VI_PIPE ViPipe)
{
	if (g_fd[ViPipe] >= 0) {
		close(g_fd[ViPipe]);
		g_fd[ViPipe] = -1;
		return CVI_SUCCESS;
	}
	return CVI_FAILURE;
}

int cv2002_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunCV2002_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, cv2002_i2c_addr, (CVI_U32)addr, cv2002_addr_byte, cv2002_data_byte);
}

int cv2002_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunCV2002_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, cv2002_i2c_addr, (CVI_U32)addr, cv2002_addr_byte,
				(CVI_U32)data, cv2002_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void cv2002_standby(VI_PIPE ViPipe)
{
	cv2002_write_register(ViPipe, 0x3000, 0x1);

	printf("%s\n", __func__);
}

void cv2002_restart(VI_PIPE ViPipe)
{
	cv2002_write_register(ViPipe, 0x3000, 0x01);
	delay_ms(20);
	cv2002_write_register(ViPipe, 0x3000, 0x00);

	printf("%s\n", __func__);
}

void cv2002_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastCV2002[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		cv2002_write_register(ViPipe,
				g_pastCV2002[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastCV2002[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

int cv2002_probe(VI_PIPE ViPipe)
{
	int nVal;
	int nVal2;

	usleep(50);
	if (cv2002_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = cv2002_read_register(ViPipe, CV2002_CHIP_ID_ADDR_H);
	nVal2 = cv2002_read_register(ViPipe, CV2002_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != CV2002_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void cv2002_init(VI_PIPE ViPipe)
{
	WDR_MODE_E        enWDRMode;
	CVI_U8            u8ImgMode;

	enWDRMode   = g_pastCV2002[ViPipe]->enWDRMode;
	u8ImgMode   = g_pastCV2002[ViPipe]->u8ImgMode;

	cv2002_i2c_init(ViPipe);

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		if (u8ImgMode == CV2002_MODE_1920X1080P30_WDR) {
			cv2002_wdr_1080p30_2to1_init(ViPipe);
		} else if(u8ImgMode == CV2002_MODE_1280X720P30_WDR) {
			cv2002_wdr_720p30_2to1_init(ViPipe);
		}
	} else if (u8ImgMode == CV2002_MODE_1920X1080P30) {
			cv2002_linear_1080P30_init(ViPipe);
	} else if (u8ImgMode == CV2002_MODE_1280X720P30) {
			cv2002_linear_720P30_init(ViPipe);
	}
	g_pastCV2002[ViPipe]->bInit = CVI_TRUE;
}

void cv2002_exit(VI_PIPE ViPipe)
{
	cv2002_i2c_exit(ViPipe);
}

static void cv2002_linear_1080P30_init(VI_PIPE ViPipe)
{
	delay_ms(10);

	// linear 30fps
	cv2002_write_register(ViPipe, 0x3029, 0x00);
	cv2002_write_register(ViPipe, 0x302A, 0x00);
	cv2002_write_register(ViPipe, 0x3300, 0x01);
	cv2002_write_register(ViPipe, 0x3401, 0x01);
	cv2002_write_register(ViPipe, 0x3440, 0x03);
	cv2002_write_register(ViPipe, 0x3442, 0x00);
	cv2002_write_register(ViPipe, 0x3806, 0x01);
	cv2002_write_register(ViPipe, 0x3158, 0x01);
	cv2002_write_register(ViPipe, 0x3159, 0x01);
	cv2002_write_register(ViPipe, 0x315A, 0x01);
	cv2002_write_register(ViPipe, 0x315B, 0x01);
	cv2002_write_register(ViPipe, 0x35B3, 0x10);
	cv2002_write_register(ViPipe, 0x3148, 0x64);
	cv2002_write_register(ViPipe, 0x3670, 0x00);
	cv2002_write_register(ViPipe, 0x3679, 0x02);
	cv2002_write_register(ViPipe, 0x3330, 0x00);
	cv2002_write_register(ViPipe, 0x320e, 0x02);
	cv2002_write_register(ViPipe, 0x3804, 0x15);
	cv2002_write_register(ViPipe, 0x35a1, 0x06);
	cv2002_write_register(ViPipe, 0x35a8, 0x06);
	cv2002_write_register(ViPipe, 0x35a9, 0x06);
	cv2002_write_register(ViPipe, 0x35aa, 0x06);
	cv2002_write_register(ViPipe, 0x35ab, 0x06);
	cv2002_write_register(ViPipe, 0x35ac, 0x06);
	cv2002_write_register(ViPipe, 0x35ad, 0x06);
	cv2002_write_register(ViPipe, 0x35ae, 0x07);
	cv2002_write_register(ViPipe, 0x3141, 0x01);	//SPLIT_GAIN_EN
	cv2002_write_register(ViPipe, 0x301C, 0x00);
	cv2002_write_register(ViPipe, 0x3030, 0x01);
	cv2002_write_register(ViPipe, 0x3038, 0x00);
	cv2002_write_register(ViPipe, 0x3039, 0x00);
	cv2002_write_register(ViPipe, 0x303A, 0x88);	//1928
	cv2002_write_register(ViPipe, 0x303B, 0x07);
	cv2002_write_register(ViPipe, 0x3034, 0x00);
	cv2002_write_register(ViPipe, 0x3035, 0x00);
	cv2002_write_register(ViPipe, 0x3036, 0x40);	//1088
	cv2002_write_register(ViPipe, 0x3037, 0x04);
	cv2002_write_register(ViPipe, 0x305C, 0x2A);	//HDR_RDS1
	cv2002_write_register(ViPipe, 0x305D, 0x02);
	cv2002_write_register(ViPipe, 0x3020, 0x88);	//FRAME_LENGTH
	cv2002_write_register(ViPipe, 0x3021, 0x13);
	cv2002_write_register(ViPipe, 0x3048, 0x34);	//SHUTTER0
	cv2002_write_register(ViPipe, 0x3049, 0x02);
	cv2002_write_register(ViPipe, 0x3024, 0x70);	//LINE_LENGTH
	cv2002_write_register(ViPipe, 0x3025, 0x02);
	cv2002_write_register(ViPipe, 0x3908, 0x4E);	//PLL.
	cv2002_write_register(ViPipe, 0x390A, 0x02);	//PLL.


	cv2002_default_reg_init(ViPipe);
	delay_ms(100);
	cv2002_write_register(ViPipe, 0x3000, 0x00);

	printf("ViPipe:%d,===CV2002 1080P 30fps 10bit LINEAR Init OK!===\n", ViPipe);
}

static void cv2002_linear_720P30_init(VI_PIPE ViPipe)
{
	delay_ms(10);

	// linear 30fps
	cv2002_write_register(ViPipe, 0x3029, 0x00);
	cv2002_write_register(ViPipe, 0x302A, 0x00);
	cv2002_write_register(ViPipe, 0x3300, 0x01);
	cv2002_write_register(ViPipe, 0x3401, 0x01);
	cv2002_write_register(ViPipe, 0x3440, 0x03);
	cv2002_write_register(ViPipe, 0x3442, 0x00);
	cv2002_write_register(ViPipe, 0x3806, 0x01);
	cv2002_write_register(ViPipe, 0x3158, 0x01);
	cv2002_write_register(ViPipe, 0x3159, 0x01);
	cv2002_write_register(ViPipe, 0x315A, 0x01);
	cv2002_write_register(ViPipe, 0x315B, 0x01);
	cv2002_write_register(ViPipe, 0x35B3, 0x10);
	cv2002_write_register(ViPipe, 0x3148, 0x64);
	cv2002_write_register(ViPipe, 0x3670, 0x00);
	cv2002_write_register(ViPipe, 0x3679, 0x02);
	cv2002_write_register(ViPipe, 0x3330, 0x00);
	cv2002_write_register(ViPipe, 0x320e, 0x02);
	cv2002_write_register(ViPipe, 0x3804, 0x15);
	cv2002_write_register(ViPipe, 0x35a1, 0x06);
	cv2002_write_register(ViPipe, 0x35a8, 0x06);
	cv2002_write_register(ViPipe, 0x35a9, 0x06);
	cv2002_write_register(ViPipe, 0x35aa, 0x06);
	cv2002_write_register(ViPipe, 0x35ab, 0x06);
	cv2002_write_register(ViPipe, 0x35ac, 0x06);
	cv2002_write_register(ViPipe, 0x35ad, 0x06);
	cv2002_write_register(ViPipe, 0x35ae, 0x07);
	cv2002_write_register(ViPipe, 0x3141, 0x01);	//SPLIT_GAIN_EN
	cv2002_write_register(ViPipe, 0x301C, 0x00);
	cv2002_write_register(ViPipe, 0x3030, 0x01);
	cv2002_write_register(ViPipe, 0x3038, 0x00);
	cv2002_write_register(ViPipe, 0x3039, 0x00);
	cv2002_write_register(ViPipe, 0x303A, 0x08);	//1288
	cv2002_write_register(ViPipe, 0x303B, 0x05);
	cv2002_write_register(ViPipe, 0x3034, 0x00);
	cv2002_write_register(ViPipe, 0x3035, 0x00);
	cv2002_write_register(ViPipe, 0x3036, 0xD8);	//728
	cv2002_write_register(ViPipe, 0x3037, 0x02);
	cv2002_write_register(ViPipe, 0x305C, 0x2A);	//HDR_RDS1
	cv2002_write_register(ViPipe, 0x305D, 0x02);
	cv2002_write_register(ViPipe, 0x3020, 0x88);	//FRAME_LENGTH
	cv2002_write_register(ViPipe, 0x3021, 0x13);
	cv2002_write_register(ViPipe, 0x3048, 0x34);	//SHUTTER0
	cv2002_write_register(ViPipe, 0x3049, 0x02);
	cv2002_write_register(ViPipe, 0x3024, 0x70);	//LINE_LENGTH
	cv2002_write_register(ViPipe, 0x3025, 0x02);
	cv2002_write_register(ViPipe, 0x3908, 0x4E);	//PLL.
	cv2002_write_register(ViPipe, 0x390A, 0x02);	//PLL.

	cv2002_write_register(ViPipe, 0x301C, 0x04); // WCROP_MODE
	cv2002_write_register(ViPipe, 0x303C, 0xb0); // Y_WCROP_STA_L
	cv2002_write_register(ViPipe, 0x303D, 0x00); // Y_WCROP_STA_H
	cv2002_write_register(ViPipe, 0x303E, 0xe0); // Y_WCROP_HEIGHT_L
	cv2002_write_register(ViPipe, 0x303F, 0x02); // Y_WCROP_HEIGHT_H
	cv2002_write_register(ViPipe, 0x3030, 0x01); // DCROP_MODE
	cv2002_write_register(ViPipe, 0x3038, 0x44); // X_CROP_STA_L
	cv2002_write_register(ViPipe, 0x3039, 0x01); // X_CROP_STA_H
	cv2002_write_register(ViPipe, 0x303A, 0x08); // X_CROP_WIDTH_L
	cv2002_write_register(ViPipe, 0x303B, 0x05); // X_CROP_WIDTH_H
	cv2002_write_register(ViPipe, 0x3034, 0x08); // Y_DCROP_STA_L
	cv2002_write_register(ViPipe, 0x3035, 0x00); // Y_DCROP_STA_H
	cv2002_write_register(ViPipe, 0x3036, 0xd8); // Y_DCROP_HEIGHT_L
	cv2002_write_register(ViPipe, 0x3037, 0x02); // Y_DCROP_HEIGHT_H

	cv2002_default_reg_init(ViPipe);
	delay_ms(100);
	cv2002_write_register(ViPipe, 0x3000, 0x00);

	printf("ViPipe:%d,===CV2002 720P 30fps 10bit LINEAR Init OK!===\n", ViPipe);
}


static void cv2002_wdr_1080p30_2to1_init(VI_PIPE ViPipe)
{
	delay_ms(10);
	// wdr 30fps
	cv2002_write_register(ViPipe, 0x301D, 0x05);	// stagger mode
	cv2002_write_register(ViPipe, 0x3020, 0x70);
	cv2002_write_register(ViPipe, 0x3021, 0x12);
	cv2002_write_register(ViPipe, 0x3022, 0x00);
	cv2002_write_register(ViPipe, 0x3024, 0x6C);
	cv2002_write_register(ViPipe, 0x3025, 0x02);
	cv2002_write_register(ViPipe, 0x3029, 0x00);
	cv2002_write_register(ViPipe, 0x302A, 0x00);
	cv2002_write_register(ViPipe, 0x3048, 0x28);
	cv2002_write_register(ViPipe, 0x3049, 0x01);
	cv2002_write_register(ViPipe, 0x304A, 0x00);
	cv2002_write_register(ViPipe, 0x3185, 0x18);
	cv2002_write_register(ViPipe, 0x3300, 0x01);
	cv2002_write_register(ViPipe, 0x3401, 0x01);
	cv2002_write_register(ViPipe, 0x3440, 0x01);
	cv2002_write_register(ViPipe, 0x3442, 0x00);
	cv2002_write_register(ViPipe, 0x3806, 0x02);
	cv2002_write_register(ViPipe, 0x3908, 0x6E);
	cv2002_write_register(ViPipe, 0x3909, 0x00);
	cv2002_write_register(ViPipe, 0x3158, 0x01);
	cv2002_write_register(ViPipe, 0x3159, 0x01);
	cv2002_write_register(ViPipe, 0x315A, 0x01);
	cv2002_write_register(ViPipe, 0x315B, 0x01);
	cv2002_write_register(ViPipe, 0x3148, 0x64);
	cv2002_write_register(ViPipe, 0x3670, 0x00);
	cv2002_write_register(ViPipe, 0x3679, 0x02);
	cv2002_write_register(ViPipe, 0x35b3, 0x10);
	cv2002_write_register(ViPipe, 0x320E, 0x02);
	cv2002_write_register(ViPipe, 0x3804, 0x15);
	cv2002_write_register(ViPipe, 0x35a1, 0x06);
	cv2002_write_register(ViPipe, 0x35a8, 0x06);
	cv2002_write_register(ViPipe, 0x35a9, 0x06);
	cv2002_write_register(ViPipe, 0x35aa, 0x06);
	cv2002_write_register(ViPipe, 0x35ab, 0x06);
	cv2002_write_register(ViPipe, 0x35ac, 0x06);
	cv2002_write_register(ViPipe, 0x35ad, 0x06);
	cv2002_write_register(ViPipe, 0x35ae, 0x07);
	cv2002_write_register(ViPipe, 0x35af, 0x07);
	cv2002_write_register(ViPipe, 0x333B, 0x01);
	cv2002_write_register(ViPipe, 0x3338, 0x1E);
	cv2002_write_register(ViPipe, 0x3339, 0x00);
	cv2002_write_register(ViPipe, 0x3330, 0x00);	//no EBD
	cv2002_write_register(ViPipe, 0x3141, 0x01);	//SPLIT_GAIN_EN
	cv2002_write_register(ViPipe, 0x301C, 0x00);
	cv2002_write_register(ViPipe, 0x3030, 0x01);
	cv2002_write_register(ViPipe, 0x3038, 0x00);
	cv2002_write_register(ViPipe, 0x3039, 0x00);
	cv2002_write_register(ViPipe, 0x303A, 0x88);	//1928
	cv2002_write_register(ViPipe, 0x303B, 0x07);
	cv2002_write_register(ViPipe, 0x3034, 0x00);
	cv2002_write_register(ViPipe, 0x3035, 0x00);
	cv2002_write_register(ViPipe, 0x3036, 0x40);	//1088
	cv2002_write_register(ViPipe, 0x3037, 0x04);
	//HDR Related
	cv2002_write_register(ViPipe, 0x305C, 0x2A);	//HDR_RDS1 = 554
	cv2002_write_register(ViPipe, 0x305D, 0x02);
	cv2002_write_register(ViPipe, 0x3020, 0x88);	//FRAME_LENGTH
	cv2002_write_register(ViPipe, 0x3021, 0x13);
	cv2002_write_register(ViPipe, 0x3048, 0x34);	//SHUTTER0
	cv2002_write_register(ViPipe, 0x3049, 0x02);
	cv2002_write_register(ViPipe, 0x3024, 0x70);	//LINE_LENGTH
	cv2002_write_register(ViPipe, 0x3025, 0x02);
	cv2002_write_register(ViPipe, 0x3908, 0x4E);	//PLL.
	cv2002_write_register(ViPipe, 0x390A, 0x02);	//PLL.

	cv2002_default_reg_init(ViPipe);
	delay_ms(100);
	cv2002_write_register(ViPipe, 0x3000, 0x00);

	printf("ViPipe:%d,===CV2002 1080P 30fps 10bit WDR2TO1 Init OK!===\n", ViPipe);
}

static void cv2002_wdr_720p30_2to1_init(VI_PIPE ViPipe)
{
	delay_ms(10);
	// wdr 30fps
	cv2002_write_register(ViPipe, 0x301D, 0x05);	// stagger mode
	cv2002_write_register(ViPipe, 0x3020, 0x70);
	cv2002_write_register(ViPipe, 0x3021, 0x12);
	cv2002_write_register(ViPipe, 0x3022, 0x00);
	cv2002_write_register(ViPipe, 0x3024, 0x6C);
	cv2002_write_register(ViPipe, 0x3025, 0x02);
	cv2002_write_register(ViPipe, 0x3029, 0x00);
	cv2002_write_register(ViPipe, 0x302A, 0x00);
	cv2002_write_register(ViPipe, 0x3048, 0x28);
	cv2002_write_register(ViPipe, 0x3049, 0x01);
	cv2002_write_register(ViPipe, 0x304A, 0x00);
	cv2002_write_register(ViPipe, 0x3185, 0x18);
	cv2002_write_register(ViPipe, 0x3300, 0x01);
	cv2002_write_register(ViPipe, 0x3401, 0x01);
	cv2002_write_register(ViPipe, 0x3440, 0x01);
	cv2002_write_register(ViPipe, 0x3442, 0x00);
	cv2002_write_register(ViPipe, 0x3806, 0x02);
	cv2002_write_register(ViPipe, 0x3908, 0x6E);
	cv2002_write_register(ViPipe, 0x3909, 0x00);
	cv2002_write_register(ViPipe, 0x3158, 0x01);
	cv2002_write_register(ViPipe, 0x3159, 0x01);
	cv2002_write_register(ViPipe, 0x315A, 0x01);
	cv2002_write_register(ViPipe, 0x315B, 0x01);
	cv2002_write_register(ViPipe, 0x3148, 0x64);
	cv2002_write_register(ViPipe, 0x3670, 0x00);
	cv2002_write_register(ViPipe, 0x3679, 0x02);
	cv2002_write_register(ViPipe, 0x35b3, 0x10);
	cv2002_write_register(ViPipe, 0x320E, 0x02);
	cv2002_write_register(ViPipe, 0x3804, 0x15);
	cv2002_write_register(ViPipe, 0x35a1, 0x06);
	cv2002_write_register(ViPipe, 0x35a8, 0x06);
	cv2002_write_register(ViPipe, 0x35a9, 0x06);
	cv2002_write_register(ViPipe, 0x35aa, 0x06);
	cv2002_write_register(ViPipe, 0x35ab, 0x06);
	cv2002_write_register(ViPipe, 0x35ac, 0x06);
	cv2002_write_register(ViPipe, 0x35ad, 0x06);
	cv2002_write_register(ViPipe, 0x35ae, 0x07);
	cv2002_write_register(ViPipe, 0x35af, 0x07);
	cv2002_write_register(ViPipe, 0x333B, 0x01);
	cv2002_write_register(ViPipe, 0x3338, 0x1E);
	cv2002_write_register(ViPipe, 0x3339, 0x00);
	cv2002_write_register(ViPipe, 0x3330, 0x00);	//no EBD
	cv2002_write_register(ViPipe, 0x3141, 0x01);	//SPLIT_GAIN_EN
	cv2002_write_register(ViPipe, 0x301C, 0x00);
	cv2002_write_register(ViPipe, 0x3030, 0x01);
	cv2002_write_register(ViPipe, 0x3038, 0x00);
	cv2002_write_register(ViPipe, 0x3039, 0x00);
	cv2002_write_register(ViPipe, 0x303A, 0x88);	//1928
	cv2002_write_register(ViPipe, 0x303B, 0x07);
	cv2002_write_register(ViPipe, 0x3034, 0x00);
	cv2002_write_register(ViPipe, 0x3035, 0x00);
	cv2002_write_register(ViPipe, 0x3036, 0x40);	//1088
	cv2002_write_register(ViPipe, 0x3037, 0x04);
	//HDR Related
	cv2002_write_register(ViPipe, 0x305C, 0x2A);	//HDR_RDS1 = 554
	cv2002_write_register(ViPipe, 0x305D, 0x02);
	cv2002_write_register(ViPipe, 0x3020, 0x88);	//FRAME_LENGTH
	cv2002_write_register(ViPipe, 0x3021, 0x13);
	cv2002_write_register(ViPipe, 0x3048, 0x34);	//SHUTTER0
	cv2002_write_register(ViPipe, 0x3049, 0x02);
	cv2002_write_register(ViPipe, 0x3024, 0x70);	//LINE_LENGTH
	cv2002_write_register(ViPipe, 0x3025, 0x02);
	cv2002_write_register(ViPipe, 0x3908, 0x4E);	//PLL.
	cv2002_write_register(ViPipe, 0x390A, 0x02);	//PLL.

	cv2002_write_register(ViPipe, 0x301C, 0x04); // WCROP_MODE
	cv2002_write_register(ViPipe, 0x303C, 0xb0); // Y_WCROP_STA_L
	cv2002_write_register(ViPipe, 0x303D, 0x00); // Y_WCROP_STA_H
	cv2002_write_register(ViPipe, 0x303E, 0xe0); // Y_WCROP_HEIGHT_L
	cv2002_write_register(ViPipe, 0x303F, 0x02); // Y_WCROP_HEIGHT_H
	cv2002_write_register(ViPipe, 0x3030, 0x01); // DCROP_MODE
	cv2002_write_register(ViPipe, 0x3038, 0x44); // X_CROP_STA_L
	cv2002_write_register(ViPipe, 0x3039, 0x01); // X_CROP_STA_H
	cv2002_write_register(ViPipe, 0x303A, 0x08); // X_CROP_WIDTH_L
	cv2002_write_register(ViPipe, 0x303B, 0x05); // X_CROP_WIDTH_H
	cv2002_write_register(ViPipe, 0x3034, 0x08); // Y_DCROP_STA_L
	cv2002_write_register(ViPipe, 0x3035, 0x00); // Y_DCROP_STA_H
	cv2002_write_register(ViPipe, 0x3036, 0xd8); // Y_DCROP_HEIGHT_L
	cv2002_write_register(ViPipe, 0x3037, 0x02); // Y_DCROP_HEIGHT_H

	cv2002_default_reg_init(ViPipe);
	delay_ms(100);
	cv2002_write_register(ViPipe, 0x3000, 0x00);

	printf("ViPipe:%d,===CV2002 720P 30fps 10bit WDR2TO1 Init OK!===\n", ViPipe);
}

#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "cv2003_cmos_ex.h"
#include "sensor_i2c.h"

#define CV2003_CHIP_ID_ADDR_H	0x307A
#define CV2003_CHIP_ID_ADDR_L	0x3138
#define CV2003_CHIP_ID			0x0203

static void cv2003_linear_1080P30_init(VI_PIPE ViPipe);
static void CV2003_1L_linear_1080P30_init(VI_PIPE ViPipe);
static void cv2003_1l_slave_high_linear_1080P30_init(VI_PIPE ViPipe);
static void cv2003_1l_slave_low_linear_1080P30_init(VI_PIPE ViPipe);
static void cv2003_1l_master_swtich_linear_1080P15_init(VI_PIPE ViPipe);
static void cv2003_1l_slave_swtich_linear_1080P15_init(VI_PIPE ViPipe);

const CVI_U32 cv2003_addr_byte = 2;
const CVI_U32 cv2003_data_byte = 1;

int cv2003_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunCV2003_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunCV2003_AddrInfo[ViPipe].s8I2cAddr);
}

int cv2003_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunCV2003_BusInfo[ViPipe].s8I2cDev);
}

int cv2003_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunCV2003_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunCV2003_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							cv2003_addr_byte, cv2003_data_byte);
}

int cv2003_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunCV2003_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunCV2003_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							cv2003_addr_byte, (CVI_U32)data, cv2003_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void cv2003_standby(VI_PIPE ViPipe)
{
	cv2003_write_register(ViPipe, 0x3000, 0x1);

	printf("%s\n", __func__);
}

void cv2003_restart(VI_PIPE ViPipe)
{
	cv2003_write_register(ViPipe, 0x3000, 0x01);
	delay_ms(20);
	cv2003_write_register(ViPipe, 0x3000, 0x00);

	printf("%s\n", __func__);
}

void cv2003_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastCV2003[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		cv2003_write_register(ViPipe,
				g_pastCV2003[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastCV2003[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

int cv2003_probe(VI_PIPE ViPipe)
{
	int nVal;
	int nVal2;

	usleep(50);
	if (cv2003_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = cv2003_read_register(ViPipe, CV2003_CHIP_ID_ADDR_H);
	nVal2 = cv2003_read_register(ViPipe, CV2003_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != CV2003_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void cv2003_init(VI_PIPE ViPipe)
{
	CVI_U8 u8ImgMode;

	u8ImgMode = g_pastCV2003[ViPipe]->u8ImgMode;

	cv2003_i2c_init(ViPipe);

	if (u8ImgMode == CV2003_MODE_1920X1080P30) {
		cv2003_linear_1080P30_init(ViPipe);
	} else if (u8ImgMode == CV2003_MODE_1920X1080P30_1L) {
		CV2003_1L_linear_1080P30_init(ViPipe);
	} else if (u8ImgMode == CV2003_MODE_1920X1080P30_1L_SALVE_HIGH_ACTIVE) {
		cv2003_1l_slave_high_linear_1080P30_init(ViPipe);
	} else if (u8ImgMode == CV2003_MODE_1920X1080P30_1L_SALVE_LOW_ACTIVE) {
		cv2003_1l_slave_low_linear_1080P30_init(ViPipe);
	} else if (u8ImgMode == CV2003_MODE_1920X1080P15_1L_MASTER_SWITCH) {
		cv2003_1l_master_swtich_linear_1080P15_init(ViPipe);
	} else if (u8ImgMode == CV2003_MODE_1920X1080P15_1L_SLAVE_SWITCH) {
		cv2003_1l_slave_swtich_linear_1080P15_init(ViPipe);
	}
	

	g_pastCV2003[ViPipe]->bInit = CVI_TRUE;
}

static void cv2003_linear_1080P30_init(VI_PIPE ViPipe)
{
	delay_ms(10);

	//30fps
	cv2003_write_register(ViPipe, 0x3300, 0x03);
	cv2003_write_register(ViPipe, 0x3422, 0xBF);
	cv2003_write_register(ViPipe, 0x3401, 0x00);
	cv2003_write_register(ViPipe, 0x3440, 0x01);
	cv2003_write_register(ViPipe, 0x3442, 0x00);
	// cv2003_write_register(ViPipe, 0x3460, 0x03);//drive capability
	cv2003_write_register(ViPipe, 0x3806, 0x00);
	cv2003_write_register(ViPipe, 0x3908, 0x5F);
	cv2003_write_register(ViPipe, 0x3909, 0x00);
	cv2003_write_register(ViPipe, 0x3929, 0x01);
	cv2003_write_register(ViPipe, 0x3158, 0x01);
	cv2003_write_register(ViPipe, 0x3159, 0x01);
	cv2003_write_register(ViPipe, 0x315A, 0x01);
	cv2003_write_register(ViPipe, 0x315B, 0x01);
	cv2003_write_register(ViPipe, 0x35b3, 0x15);
	cv2003_write_register(ViPipe, 0x3148, 0x64);
	cv2003_write_register(ViPipe, 0x3031, 0x00);
	cv2003_write_register(ViPipe, 0x3118, 0x01);
	cv2003_write_register(ViPipe, 0x3119, 0x06);
	cv2003_write_register(ViPipe, 0x3670, 0x00);
	cv2003_write_register(ViPipe, 0x3679, 0x02);
	cv2003_write_register(ViPipe, 0x3330, 0x00);
	cv2003_write_register(ViPipe, 0x320e, 0x02);
	cv2003_write_register(ViPipe, 0x3804, 0x10);
	cv2003_write_register(ViPipe, 0x35a1, 0x06);
	cv2003_write_register(ViPipe, 0x35a8, 0x06);
	cv2003_write_register(ViPipe, 0x35a9, 0x06);
	cv2003_write_register(ViPipe, 0x35aa, 0x06);
	cv2003_write_register(ViPipe, 0x35ab, 0x06);
	cv2003_write_register(ViPipe, 0x35ac, 0x06);
	cv2003_write_register(ViPipe, 0x35ad, 0x06);
	cv2003_write_register(ViPipe, 0x35ae, 0x07);
	cv2003_write_register(ViPipe, 0x35af, 0x07);
	cv2003_write_register(ViPipe, 0x333B, 0x01);
	cv2003_write_register(ViPipe, 0x3338, 0x08);
	cv2003_write_register(ViPipe, 0x3339, 0x00);
	cv2003_write_register(ViPipe, 0x3144, 0x20);
	cv2003_write_register(ViPipe, 0x301c, 0x00);
	cv2003_write_register(ViPipe, 0x3030, 0x01);
	cv2003_write_register(ViPipe, 0x3020, 0xCA);
	cv2003_write_register(ViPipe, 0x3021, 0x08);
	cv2003_write_register(ViPipe, 0x3024, 0x80);
	cv2003_write_register(ViPipe, 0x3025, 0x02);
	cv2003_write_register(ViPipe, 0x3038, 0x00);
	cv2003_write_register(ViPipe, 0x3039, 0x00);
	cv2003_write_register(ViPipe, 0x303A, 0x88);
	cv2003_write_register(ViPipe, 0x303B, 0x07);
	cv2003_write_register(ViPipe, 0x3034, 0x00);
	cv2003_write_register(ViPipe, 0x3035, 0x00);
	cv2003_write_register(ViPipe, 0x3036, 0x40);
	cv2003_write_register(ViPipe, 0x3037, 0x04);
	cv2003_write_register(ViPipe, 0x3908, 0x48);
	cv2003_write_register(ViPipe, 0x390A, 0x02);

	//slave mode
	// cv2003_write_register(ViPipe, 0x3001, 0x01);
	// cv2003_write_register(ViPipe, 0x307A, 0x02);
	// cv2003_write_register(ViPipe, 0x306D, 0x0F);

	cv2003_default_reg_init(ViPipe);
	delay_ms(100);
	cv2003_write_register(ViPipe, 0x3000, 0x00);

	printf("ViPipe:%d,===CV2003 1080P 30fps 12bit LINEAR Init OK!===\n", ViPipe);
}

static void CV2003_1L_linear_1080P30_init(VI_PIPE ViPipe)
{
	delay_ms(10);

	//30fps
	cv2003_write_register(ViPipe, 0x3300, 0x03);
	cv2003_write_register(ViPipe, 0x3422, 0xBF);
	cv2003_write_register(ViPipe, 0x3401, 0x00);
	cv2003_write_register(ViPipe, 0x3440, 0x01);
	cv2003_write_register(ViPipe, 0x3442, 0x00);
	// cv2003_write_register(ViPipe, 0x3460, 0x03);//drive capability
	cv2003_write_register(ViPipe, 0x3806, 0x00);
	cv2003_write_register(ViPipe, 0x3908, 0x5F);
	cv2003_write_register(ViPipe, 0x3909, 0x00);
	cv2003_write_register(ViPipe, 0x3929, 0x01);
	cv2003_write_register(ViPipe, 0x3158, 0x01);
	cv2003_write_register(ViPipe, 0x3159, 0x01);
	cv2003_write_register(ViPipe, 0x315A, 0x01);
	cv2003_write_register(ViPipe, 0x315B, 0x01);
	cv2003_write_register(ViPipe, 0x35b3, 0x15);
	cv2003_write_register(ViPipe, 0x3148, 0x64);
	cv2003_write_register(ViPipe, 0x3031, 0x00);
	cv2003_write_register(ViPipe, 0x3118, 0x01);
	cv2003_write_register(ViPipe, 0x3119, 0x06);
	cv2003_write_register(ViPipe, 0x3670, 0x00);
	cv2003_write_register(ViPipe, 0x3679, 0x02);
	cv2003_write_register(ViPipe, 0x3330, 0x00);
	cv2003_write_register(ViPipe, 0x320e, 0x02);
	cv2003_write_register(ViPipe, 0x3804, 0x10);
	cv2003_write_register(ViPipe, 0x35a1, 0x06);
	cv2003_write_register(ViPipe, 0x35a8, 0x06);
	cv2003_write_register(ViPipe, 0x35a9, 0x06);
	cv2003_write_register(ViPipe, 0x35aa, 0x06);
	cv2003_write_register(ViPipe, 0x35ab, 0x06);
	cv2003_write_register(ViPipe, 0x35ac, 0x06);
	cv2003_write_register(ViPipe, 0x35ad, 0x06);
	cv2003_write_register(ViPipe, 0x35ae, 0x07);
	cv2003_write_register(ViPipe, 0x35af, 0x07);
	cv2003_write_register(ViPipe, 0x333B, 0x01);
	cv2003_write_register(ViPipe, 0x3338, 0x08);
	cv2003_write_register(ViPipe, 0x3339, 0x00);
	cv2003_write_register(ViPipe, 0x3144, 0x20);
	cv2003_write_register(ViPipe, 0x301c, 0x00);
	cv2003_write_register(ViPipe, 0x3030, 0x01);
	cv2003_write_register(ViPipe, 0x3020, 0xCA);
	cv2003_write_register(ViPipe, 0x3021, 0x08);
	cv2003_write_register(ViPipe, 0x3024, 0x80);
	cv2003_write_register(ViPipe, 0x3025, 0x02);
	cv2003_write_register(ViPipe, 0x3038, 0x00);
	cv2003_write_register(ViPipe, 0x3039, 0x00);
	cv2003_write_register(ViPipe, 0x303A, 0x88);
	cv2003_write_register(ViPipe, 0x303B, 0x07);
	cv2003_write_register(ViPipe, 0x3034, 0x00);
	cv2003_write_register(ViPipe, 0x3035, 0x00);
	cv2003_write_register(ViPipe, 0x3036, 0x40);
	cv2003_write_register(ViPipe, 0x3037, 0x04);
	cv2003_write_register(ViPipe, 0x3908, 0x48);
	cv2003_write_register(ViPipe, 0x390A, 0x02);

	//slave mode
	// cv2003_write_register(ViPipe, 0x3001, 0x01);
	// cv2003_write_register(ViPipe, 0x307A, 0x02);
	// cv2003_write_register(ViPipe, 0x306D, 0x0F);

	cv2003_default_reg_init(ViPipe);
	delay_ms(100);
	cv2003_write_register(ViPipe, 0x3000, 0x00);

	printf("ViPipe:%d,===CV2003_1L 1080P 30fps 12bit LINEAR Init OK!===\n", ViPipe);
}

static void cv2003_1l_slave_high_linear_1080P30_init(VI_PIPE ViPipe)
{
	delay_ms(10);

	//15fps
	cv2003_write_register(ViPipe, 0x3300, 0x03);
	cv2003_write_register(ViPipe, 0x3422, 0xBF);
	cv2003_write_register(ViPipe, 0x3401, 0x00);

	cv2003_write_register(ViPipe, 0x3440, 0x01);
	cv2003_write_register(ViPipe, 0x3442, 0x00);
	// cv2003_write_register(ViPipe, 0x3460, 0x03);//drive capability
	cv2003_write_register(ViPipe, 0x3806, 0x00);
	cv2003_write_register(ViPipe, 0x3908, 0x5F);
	cv2003_write_register(ViPipe, 0x3909, 0x00);
	cv2003_write_register(ViPipe, 0x3929, 0x01);
	cv2003_write_register(ViPipe, 0x3158, 0x01);
	cv2003_write_register(ViPipe, 0x3159, 0x01);
	cv2003_write_register(ViPipe, 0x315A, 0x01);
	cv2003_write_register(ViPipe, 0x315B, 0x01);
	cv2003_write_register(ViPipe, 0x35b3, 0x15);
	cv2003_write_register(ViPipe, 0x3148, 0x64);
	cv2003_write_register(ViPipe, 0x3031, 0x00);
	cv2003_write_register(ViPipe, 0x3118, 0x01);
	cv2003_write_register(ViPipe, 0x3119, 0x06);
	cv2003_write_register(ViPipe, 0x3670, 0x00);
	cv2003_write_register(ViPipe, 0x3679, 0x02);
	cv2003_write_register(ViPipe, 0x3330, 0x00);
	cv2003_write_register(ViPipe, 0x320e, 0x02);
	cv2003_write_register(ViPipe, 0x3804, 0x10);
	cv2003_write_register(ViPipe, 0x35a1, 0x06);
	cv2003_write_register(ViPipe, 0x35a8, 0x06);
	cv2003_write_register(ViPipe, 0x35a9, 0x06);
	cv2003_write_register(ViPipe, 0x35aa, 0x06);
	cv2003_write_register(ViPipe, 0x35ab, 0x06);
	cv2003_write_register(ViPipe, 0x35ac, 0x06);
	cv2003_write_register(ViPipe, 0x35ad, 0x06);
	cv2003_write_register(ViPipe, 0x35ae, 0x07);
	cv2003_write_register(ViPipe, 0x35af, 0x07);
	cv2003_write_register(ViPipe, 0x333B, 0x01);
	cv2003_write_register(ViPipe, 0x3338, 0x08);
	cv2003_write_register(ViPipe, 0x3339, 0x00);
	cv2003_write_register(ViPipe, 0x3144, 0x20);
	cv2003_write_register(ViPipe, 0x301c, 0x00);
	cv2003_write_register(ViPipe, 0x3030, 0x01);
	cv2003_write_register(ViPipe, 0x3020, 0x94);
	cv2003_write_register(ViPipe, 0x3021, 0x11);
	cv2003_write_register(ViPipe, 0x3024, 0x80);
	cv2003_write_register(ViPipe, 0x3025, 0x02);
	cv2003_write_register(ViPipe, 0x3038, 0x00);
	cv2003_write_register(ViPipe, 0x3039, 0x00);
	cv2003_write_register(ViPipe, 0x303A, 0x88);
	cv2003_write_register(ViPipe, 0x303B, 0x07);
	cv2003_write_register(ViPipe, 0x3034, 0x00);
	cv2003_write_register(ViPipe, 0x3035, 0x00);
	cv2003_write_register(ViPipe, 0x3036, 0x40);
	cv2003_write_register(ViPipe, 0x3037, 0x04);
	cv2003_write_register(ViPipe, 0x3908, 0x48);
	cv2003_write_register(ViPipe, 0x390A, 0x02);

	//slave mode
	cv2003_write_register(ViPipe, 0x3001, 0x01);
	cv2003_write_register(ViPipe, 0x307A, 0x02);
	cv2003_write_register(ViPipe, 0x306D, 0x0F);
	//high
	cv2003_write_register(ViPipe, 0x3078, 0x00);

	cv2003_default_reg_init(ViPipe);
	delay_ms(100);
	cv2003_write_register(ViPipe, 0x3000, 0x00);

	printf("ViPipe:%d ===CV2003 1L SLAVE HIGH ACTIVE 1080P 30fps 12bit LINEAR Init OK!===\n", ViPipe);
}

static void cv2003_1l_slave_low_linear_1080P30_init(VI_PIPE ViPipe)
{
	delay_ms(10);

	//15fps
	cv2003_write_register(ViPipe, 0x3300, 0x03);
	cv2003_write_register(ViPipe, 0x3422, 0xBF);
	cv2003_write_register(ViPipe, 0x3401, 0x00);
	//CHAGE MIPI DATA<==>MIPI CLK
	// cv2003_write_register(ViPipe, 0x342E,0x00);
	// cv2003_write_register(ViPipe, 0x342F,0x01);
	// cv2003_write_register(ViPipe, 0x3430,0x02);
	// cv2003_write_register(ViPipe, 0x3431,0x03);

	cv2003_write_register(ViPipe, 0x3440, 0x01);
	cv2003_write_register(ViPipe, 0x3442, 0x00);
	// cv2003_write_register(ViPipe, 0x3460, 0x03);//drive capability
	cv2003_write_register(ViPipe, 0x3806, 0x00);
	cv2003_write_register(ViPipe, 0x3908, 0x5F);
	cv2003_write_register(ViPipe, 0x3909, 0x00);
	cv2003_write_register(ViPipe, 0x3929, 0x01);
	cv2003_write_register(ViPipe, 0x3158, 0x01);
	cv2003_write_register(ViPipe, 0x3159, 0x01);
	cv2003_write_register(ViPipe, 0x315A, 0x01);
	cv2003_write_register(ViPipe, 0x315B, 0x01);
	cv2003_write_register(ViPipe, 0x35b3, 0x15);
	cv2003_write_register(ViPipe, 0x3148, 0x64);
	cv2003_write_register(ViPipe, 0x3031, 0x00);
	cv2003_write_register(ViPipe, 0x3118, 0x01);
	cv2003_write_register(ViPipe, 0x3119, 0x06);
	cv2003_write_register(ViPipe, 0x3670, 0x00);
	cv2003_write_register(ViPipe, 0x3679, 0x02);
	cv2003_write_register(ViPipe, 0x3330, 0x00);
	cv2003_write_register(ViPipe, 0x320e, 0x02);
	cv2003_write_register(ViPipe, 0x3804, 0x10);
	cv2003_write_register(ViPipe, 0x35a1, 0x06);
	cv2003_write_register(ViPipe, 0x35a8, 0x06);
	cv2003_write_register(ViPipe, 0x35a9, 0x06);
	cv2003_write_register(ViPipe, 0x35aa, 0x06);
	cv2003_write_register(ViPipe, 0x35ab, 0x06);
	cv2003_write_register(ViPipe, 0x35ac, 0x06);
	cv2003_write_register(ViPipe, 0x35ad, 0x06);
	cv2003_write_register(ViPipe, 0x35ae, 0x07);
	cv2003_write_register(ViPipe, 0x35af, 0x07);
	cv2003_write_register(ViPipe, 0x333B, 0x01);
	cv2003_write_register(ViPipe, 0x3338, 0x08);
	cv2003_write_register(ViPipe, 0x3339, 0x00);
	cv2003_write_register(ViPipe, 0x3144, 0x20);
	cv2003_write_register(ViPipe, 0x301c, 0x00);
	cv2003_write_register(ViPipe, 0x3030, 0x01);
	cv2003_write_register(ViPipe, 0x3020, 0x94);
	cv2003_write_register(ViPipe, 0x3021, 0x11);
	cv2003_write_register(ViPipe, 0x3024, 0x80);
	cv2003_write_register(ViPipe, 0x3025, 0x02);
	cv2003_write_register(ViPipe, 0x3038, 0x00);
	cv2003_write_register(ViPipe, 0x3039, 0x00);
	cv2003_write_register(ViPipe, 0x303A, 0x88);
	cv2003_write_register(ViPipe, 0x303B, 0x07);
	cv2003_write_register(ViPipe, 0x3034, 0x00);
	cv2003_write_register(ViPipe, 0x3035, 0x00);
	cv2003_write_register(ViPipe, 0x3036, 0x40);
	cv2003_write_register(ViPipe, 0x3037, 0x04);
	cv2003_write_register(ViPipe, 0x3908, 0x48);
	cv2003_write_register(ViPipe, 0x390A, 0x02);

	//slave1 mode
	cv2003_write_register(ViPipe, 0x3001, 0x01);
	cv2003_write_register(ViPipe, 0x307A, 0x02);
	cv2003_write_register(ViPipe, 0x306D, 0x0F);
	//low
	cv2003_write_register(ViPipe, 0x3078, 0x04);

	cv2003_default_reg_init(ViPipe);
	delay_ms(100);
	cv2003_write_register(ViPipe, 0x3000, 0x00);

	printf("ViPipe:%d ===CV2003 1L SLAVE LOW ACTIVE 1080P 30fps 12bit LINEAR Init OK!===\n", ViPipe);
}


static void cv2003_1l_master_swtich_linear_1080P15_init(VI_PIPE ViPipe)
{
	delay_ms(10);

	//15fps
	cv2003_write_register(ViPipe, 0x3300, 0x03 );
	cv2003_write_register(ViPipe, 0x3422, 0xBF );
	cv2003_write_register(ViPipe, 0x3401, 0x00 );
	cv2003_write_register(ViPipe, 0x3440, 0x01 );
	cv2003_write_register(ViPipe, 0x3442, 0x00 );
	cv2003_write_register(ViPipe, 0x3806, 0x00 );
	cv2003_write_register(ViPipe, 0x3908, 0x5F );
	cv2003_write_register(ViPipe, 0x3909, 0x00 );
	cv2003_write_register(ViPipe, 0x3929, 0x01 );
	cv2003_write_register(ViPipe, 0x3158, 0x01 );
	cv2003_write_register(ViPipe, 0x3159, 0x01 );
	cv2003_write_register(ViPipe, 0x315A, 0x01 );
	cv2003_write_register(ViPipe, 0x315B, 0x01 );
	cv2003_write_register(ViPipe, 0x35b3, 0x15 );
	cv2003_write_register(ViPipe, 0x3148, 0x64 );
	cv2003_write_register(ViPipe, 0x3031, 0x00 );
	cv2003_write_register(ViPipe, 0x3118, 0x01 );
	cv2003_write_register(ViPipe, 0x3119, 0x06 );
	cv2003_write_register(ViPipe, 0x3670, 0x00 );
	cv2003_write_register(ViPipe, 0x3679, 0x02 );
	cv2003_write_register(ViPipe, 0x3330, 0x00 );
	cv2003_write_register(ViPipe, 0x320e, 0x02 );
	cv2003_write_register(ViPipe, 0x3804, 0x10 );
	cv2003_write_register(ViPipe, 0x35a1, 0x06 );
	cv2003_write_register(ViPipe, 0x35a8, 0x06 );
	cv2003_write_register(ViPipe, 0x35a9, 0x06 );
	cv2003_write_register(ViPipe, 0x35aa, 0x06 );
	cv2003_write_register(ViPipe, 0x35ab, 0x06 );
	cv2003_write_register(ViPipe, 0x35ac, 0x06 );
	cv2003_write_register(ViPipe, 0x35ad, 0x06 );
	cv2003_write_register(ViPipe, 0x35ae, 0x07 );
	cv2003_write_register(ViPipe, 0x35af, 0x07 );
	cv2003_write_register(ViPipe, 0x333B, 0x01 );
	cv2003_write_register(ViPipe, 0x3338, 0x08 );
	cv2003_write_register(ViPipe, 0x3339, 0x00 );
	cv2003_write_register(ViPipe, 0x3144, 0x20 );
	cv2003_write_register(ViPipe, 0x301c, 0x00 );
	cv2003_write_register(ViPipe, 0x3030, 0x01 );
	cv2003_write_register(ViPipe, 0x3020, 0x4C );
	cv2003_write_register(ViPipe, 0x3021, 0x1D );
	cv2003_write_register(ViPipe, 0x3024, 0x80 );
	cv2003_write_register(ViPipe, 0x3025, 0x02 );
	cv2003_write_register(ViPipe, 0x3038, 0x00 );
	cv2003_write_register(ViPipe, 0x3039, 0x00 );
	cv2003_write_register(ViPipe, 0x303A, 0x88 );
	cv2003_write_register(ViPipe, 0x303B, 0x07 );
	cv2003_write_register(ViPipe, 0x3034, 0x00 );
	cv2003_write_register(ViPipe, 0x3035, 0x00 );
	cv2003_write_register(ViPipe, 0x3036, 0x40 );
	cv2003_write_register(ViPipe, 0x3037, 0x04 );
	cv2003_write_register(ViPipe, 0x3908, 0x78 );
	cv2003_write_register(ViPipe, 0x390A, 0x02 );
	// master mode
	cv2003_write_register(ViPipe, 0x306C, 0x01 );
	cv2003_write_register(ViPipe, 0x3c54, 0x87 );
	cv2003_write_register(ViPipe, 0x3c56, 0x87 );
	cv2003_write_register(ViPipe, 0x3780, 0x80 );

	cv2003_default_reg_init(ViPipe);
	delay_ms(100);
	cv2003_write_register(ViPipe, 0x3000, 0x00);

	printf("ViPipe:%d,===CV2003 1L MASTER SWITCH 1080P 15fps 12bit LINEAR Init OK!===\n", ViPipe);
}

static void cv2003_1l_slave_swtich_linear_1080P15_init(VI_PIPE ViPipe)
{
	delay_ms(10);

	//15fps
	cv2003_write_register(ViPipe, 0x3300, 0x03 );
	cv2003_write_register(ViPipe, 0x3422, 0xBF );
	cv2003_write_register(ViPipe, 0x3401, 0x00 );
	cv2003_write_register(ViPipe, 0x3440, 0x01 );
	cv2003_write_register(ViPipe, 0x3442, 0x00 );
	cv2003_write_register(ViPipe, 0x3806, 0x00 );
	cv2003_write_register(ViPipe, 0x3908, 0x5F );
	cv2003_write_register(ViPipe, 0x3909, 0x00 );
	cv2003_write_register(ViPipe, 0x3929, 0x01 );
	cv2003_write_register(ViPipe, 0x3158, 0x01 );
	cv2003_write_register(ViPipe, 0x3159, 0x01 );
	cv2003_write_register(ViPipe, 0x315A, 0x01 );
	cv2003_write_register(ViPipe, 0x315B, 0x01 );
	cv2003_write_register(ViPipe, 0x35b3, 0x15 );
	cv2003_write_register(ViPipe, 0x3148, 0x64 );
	cv2003_write_register(ViPipe, 0x3031, 0x00 );
	cv2003_write_register(ViPipe, 0x3118, 0x01 );
	cv2003_write_register(ViPipe, 0x3119, 0x06 );
	cv2003_write_register(ViPipe, 0x3670, 0x00 );
	cv2003_write_register(ViPipe, 0x3679, 0x02 );
	cv2003_write_register(ViPipe, 0x3330, 0x00 );
	cv2003_write_register(ViPipe, 0x320e, 0x02 );
	cv2003_write_register(ViPipe, 0x3804, 0x10 );
	cv2003_write_register(ViPipe, 0x35a1, 0x06 );
	cv2003_write_register(ViPipe, 0x35a8, 0x06 );
	cv2003_write_register(ViPipe, 0x35a9, 0x06 );
	cv2003_write_register(ViPipe, 0x35aa, 0x06 );
	cv2003_write_register(ViPipe, 0x35ab, 0x06 );
	cv2003_write_register(ViPipe, 0x35ac, 0x06 );
	cv2003_write_register(ViPipe, 0x35ad, 0x06 );
	cv2003_write_register(ViPipe, 0x35ae, 0x07 );
	cv2003_write_register(ViPipe, 0x35af, 0x07 );
	cv2003_write_register(ViPipe, 0x333B, 0x01 );
	cv2003_write_register(ViPipe, 0x3338, 0x08 );
	cv2003_write_register(ViPipe, 0x3339, 0x00 );
	cv2003_write_register(ViPipe, 0x3144, 0x20 );
	cv2003_write_register(ViPipe, 0x301c, 0x00 );
	cv2003_write_register(ViPipe, 0x3030, 0x01 );
	cv2003_write_register(ViPipe, 0x3020, 0x5E );
	cv2003_write_register(ViPipe, 0x3021, 0x1A );
	cv2003_write_register(ViPipe, 0x3024, 0x80 );
	cv2003_write_register(ViPipe, 0x3025, 0x02 );
	cv2003_write_register(ViPipe, 0x3038, 0x00 );
	cv2003_write_register(ViPipe, 0x3039, 0x00 );
	cv2003_write_register(ViPipe, 0x303A, 0x88 );
	cv2003_write_register(ViPipe, 0x303B, 0x07 );
	cv2003_write_register(ViPipe, 0x3034, 0x00 );
	cv2003_write_register(ViPipe, 0x3035, 0x00 );
	cv2003_write_register(ViPipe, 0x3036, 0x40 );
	cv2003_write_register(ViPipe, 0x3037, 0x04 );
	cv2003_write_register(ViPipe, 0x3908, 0x6C );
	cv2003_write_register(ViPipe, 0x390A, 0x02 );
	// slave mode
	cv2003_write_register(ViPipe, 0x3001, 0x01 );
	cv2003_write_register(ViPipe, 0x306D, 0x03 );
	cv2003_write_register(ViPipe, 0x306C, 0x01 );
	cv2003_write_register(ViPipe, 0x3c54, 0x87 );
	cv2003_write_register(ViPipe, 0x3c56, 0x87 );
	cv2003_write_register(ViPipe, 0x3780, 0x80 );

	cv2003_default_reg_init(ViPipe);
	delay_ms(100);
	cv2003_write_register(ViPipe, 0x3000, 0x00);

	printf("ViPipe:%d,===CV2003 1L SLAVE SWITCH 1080P 15fps 12bit LINEAR Init OK!===\n", ViPipe);
}
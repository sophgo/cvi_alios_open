#include <unistd.h>

#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sensor_i2c.h"

#include "mis2032_cmos_ex.h"

#define MIS2032_CHIP_ID_ADDR_H		0x3000
#define MIS2032_CHIP_ID_ADDR_L		0x3001
#define MIS2032_CHIP_ID			0x2009
static void mis2032_linear_1080p25_init(VI_PIPE ViPipe);
static void mis2032_wdr_1080p25_init(VI_PIPE ViPipe);

CVI_U8 mis2032_i2c_addr = 0x30;        /* I2C Address of MIS2032 */
const CVI_U32 mis2032_addr_byte = 2;
const CVI_U32 mis2032_data_byte = 1;
// static int g_fd[VI_MAX_PIPE_NUM] = {[0 ... (VI_MAX_PIPE_NUM - 1)] = -1};

int mis2032_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunMIS2032_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int mis2032_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunMIS2032_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);

}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

int mis2032_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunMIS2032_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, mis2032_i2c_addr, (CVI_U32)addr, mis2032_addr_byte, mis2032_data_byte);
}

int mis2032_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunMIS2032_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, mis2032_i2c_addr, (CVI_U32)addr, mis2032_addr_byte,
		(CVI_U32)data, mis2032_data_byte);
}

void mis2032_standby(VI_PIPE ViPipe)
{
	mis2032_write_register(ViPipe, 0x3006, 0x02);
}

void mis2032_restart(VI_PIPE ViPipe)
{
	mis2032_write_register(ViPipe, 0x3006, 0x00);
}

void mis2032_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastMIS2032[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		mis2032_write_register(ViPipe,
				g_pastMIS2032[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastMIS2032[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

int mis2032_probe(VI_PIPE ViPipe)
{
	int nVal;
	int nVal2;
	int nVal3;

	usleep(50);
	if (mis2032_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = mis2032_read_register(ViPipe, MIS2032_CHIP_ID_ADDR_H);
	nVal2 = mis2032_read_register(ViPipe, MIS2032_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}
	nVal3 = (((nVal & 0xFF) << 8) | (nVal2 & 0xFF));
	if (nVal3 != MIS2032_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! true chip id [%x]  read chip id [%x]\n", MIS2032_CHIP_ID, nVal3);
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}


void mis2032_init(VI_PIPE ViPipe)
{
	WDR_MODE_E        enWDRMode;
	CVI_U8            u8ImgMode;

	enWDRMode   = g_pastMIS2032[ViPipe]->enWDRMode;
	u8ImgMode   = g_pastMIS2032[ViPipe]->u8ImgMode;

	mis2032_i2c_init(ViPipe);

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		if (u8ImgMode == MIS2032_MODE_1080P25_WDR)
			mis2032_wdr_1080p25_init(ViPipe);
	} else {
		if (u8ImgMode == MIS2032_MODE_1080P25)
			mis2032_linear_1080p25_init(ViPipe);
	}

	g_pastMIS2032[ViPipe]->bInit = CVI_TRUE;
}

void mis2032_exit(VI_PIPE ViPipe)
{
	mis2032_i2c_exit(ViPipe);
}

static void mis2032_linear_1080p25_init(VI_PIPE ViPipe)
{
	mis2032_write_register(ViPipe, 0x300b, 0x01);
	mis2032_write_register(ViPipe, 0x3006, 0x02);
	delay_ms(40);
	mis2032_write_register(ViPipe, 0x330c, 0x01);
	mis2032_write_register(ViPipe, 0x3b00, 0x07);
	mis2032_write_register(ViPipe, 0x3b01, 0xff);
	mis2032_write_register(ViPipe, 0x3a00, 0x80);
	mis2032_write_register(ViPipe, 0x3300, 0x7b);
	mis2032_write_register(ViPipe, 0x3301, 0x01);
	mis2032_write_register(ViPipe, 0x3302, 0x01);
	mis2032_write_register(ViPipe, 0x3303, 0x03);
	mis2032_write_register(ViPipe, 0x3309, 0x01);
	mis2032_write_register(ViPipe, 0x3307, 0x01);
	mis2032_write_register(ViPipe, 0x330b, 0x0a);
	mis2032_write_register(ViPipe, 0x3021, 0x02);
	mis2032_write_register(ViPipe, 0x3201, 0xca);
	mis2032_write_register(ViPipe, 0x3200, 0x0b);
	mis2032_write_register(ViPipe, 0x3203, 0x98);
	mis2032_write_register(ViPipe, 0x3202, 0x08);
	mis2032_write_register(ViPipe, 0x3205, 0x00);
	mis2032_write_register(ViPipe, 0x3204, 0x00);
	mis2032_write_register(ViPipe, 0x3207, 0x3b);
	mis2032_write_register(ViPipe, 0x3206, 0x04);
	mis2032_write_register(ViPipe, 0x3209, 0x08);
	mis2032_write_register(ViPipe, 0x3208, 0x00);
	mis2032_write_register(ViPipe, 0x320b, 0x87);
	mis2032_write_register(ViPipe, 0x320a, 0x07);
	mis2032_write_register(ViPipe, 0x3102, 0x00);
	mis2032_write_register(ViPipe, 0x3105, 0x00);
	mis2032_write_register(ViPipe, 0x3108, 0x00);
	mis2032_write_register(ViPipe, 0x3007, 0x00);
	mis2032_write_register(ViPipe, 0x300a, 0x01);
	mis2032_write_register(ViPipe, 0x3014, 0x00);
	mis2032_write_register(ViPipe, 0x330f, 0x00);
	mis2032_write_register(ViPipe, 0x310f, 0x00);
	mis2032_write_register(ViPipe, 0x3986, 0x02);
	mis2032_write_register(ViPipe, 0x3986, 0x02);
	mis2032_write_register(ViPipe, 0x3900, 0x00);
	mis2032_write_register(ViPipe, 0x3902, 0x13);
	mis2032_write_register(ViPipe, 0x3901, 0x00);
	mis2032_write_register(ViPipe, 0x3904, 0x4c);
	mis2032_write_register(ViPipe, 0x3903, 0x09);
	mis2032_write_register(ViPipe, 0x3906, 0xff);
	mis2032_write_register(ViPipe, 0x3905, 0x1f);
	mis2032_write_register(ViPipe, 0x3908, 0xff);
	mis2032_write_register(ViPipe, 0x3907, 0x1f);
	mis2032_write_register(ViPipe, 0x390a, 0xb3);
	mis2032_write_register(ViPipe, 0x3909, 0x02);
	mis2032_write_register(ViPipe, 0x390c, 0xf7);
	mis2032_write_register(ViPipe, 0x390b, 0x05);
	mis2032_write_register(ViPipe, 0x390e, 0x36);
	mis2032_write_register(ViPipe, 0x390d, 0x09);
	mis2032_write_register(ViPipe, 0x3910, 0xff);
	mis2032_write_register(ViPipe, 0x390f, 0x1f);
	mis2032_write_register(ViPipe, 0x3911, 0x01);
	mis2032_write_register(ViPipe, 0x3917, 0x00);
	mis2032_write_register(ViPipe, 0x3916, 0x00);
	mis2032_write_register(ViPipe, 0x3919, 0xa1);
	mis2032_write_register(ViPipe, 0x3918, 0x01);
	mis2032_write_register(ViPipe, 0x3913, 0x13);
	mis2032_write_register(ViPipe, 0x3912, 0x00);
	mis2032_write_register(ViPipe, 0x3915, 0xc6);
	mis2032_write_register(ViPipe, 0x3914, 0x02);
	mis2032_write_register(ViPipe, 0x391b, 0x00);
	mis2032_write_register(ViPipe, 0x391a, 0x00);
	mis2032_write_register(ViPipe, 0x391d, 0x48);
	mis2032_write_register(ViPipe, 0x391c, 0x09);
	mis2032_write_register(ViPipe, 0x391f, 0xff);
	mis2032_write_register(ViPipe, 0x391e, 0x1f);
	mis2032_write_register(ViPipe, 0x3921, 0xff);
	mis2032_write_register(ViPipe, 0x3920, 0x1f);
	mis2032_write_register(ViPipe, 0x3923, 0x00);
	mis2032_write_register(ViPipe, 0x3922, 0x00);
	mis2032_write_register(ViPipe, 0x3925, 0xb9);
	mis2032_write_register(ViPipe, 0x3924, 0x02);
	mis2032_write_register(ViPipe, 0x394c, 0x00);
	mis2032_write_register(ViPipe, 0x394e, 0x82);
	mis2032_write_register(ViPipe, 0x394d, 0x00);
	mis2032_write_register(ViPipe, 0x3950, 0x95);
	mis2032_write_register(ViPipe, 0x394f, 0x00);
	mis2032_write_register(ViPipe, 0x3952, 0x70);
	mis2032_write_register(ViPipe, 0x3951, 0x00);
	mis2032_write_register(ViPipe, 0x3954, 0xea);
	mis2032_write_register(ViPipe, 0x3953, 0x02);
	mis2032_write_register(ViPipe, 0x3927, 0x00);
	mis2032_write_register(ViPipe, 0x3926, 0x00);
	mis2032_write_register(ViPipe, 0x3929, 0xdf);
	mis2032_write_register(ViPipe, 0x3928, 0x00);
	mis2032_write_register(ViPipe, 0x392b, 0xd1);
	mis2032_write_register(ViPipe, 0x392a, 0x01);
	mis2032_write_register(ViPipe, 0x392d, 0xa1);
	mis2032_write_register(ViPipe, 0x392c, 0x02);
	mis2032_write_register(ViPipe, 0x392f, 0x66);
	mis2032_write_register(ViPipe, 0x392e, 0x06);
	mis2032_write_register(ViPipe, 0x3931, 0x36);
	mis2032_write_register(ViPipe, 0x3930, 0x09);
	mis2032_write_register(ViPipe, 0x3933, 0x36);
	mis2032_write_register(ViPipe, 0x3932, 0x09);
	mis2032_write_register(ViPipe, 0x3935, 0x36);
	mis2032_write_register(ViPipe, 0x3934, 0x09);
	mis2032_write_register(ViPipe, 0x3937, 0x36);
	mis2032_write_register(ViPipe, 0x3936, 0x09);
	mis2032_write_register(ViPipe, 0x3939, 0x36);
	mis2032_write_register(ViPipe, 0x3938, 0x09);
	mis2032_write_register(ViPipe, 0x393b, 0x36);
	mis2032_write_register(ViPipe, 0x393a, 0x09);
	mis2032_write_register(ViPipe, 0x3991, 0x40);
	mis2032_write_register(ViPipe, 0x3990, 0x00);
	mis2032_write_register(ViPipe, 0x3993, 0x56);
	mis2032_write_register(ViPipe, 0x3992, 0x09);
	mis2032_write_register(ViPipe, 0x3995, 0xff);
	mis2032_write_register(ViPipe, 0x3994, 0x1f);
	mis2032_write_register(ViPipe, 0x3997, 0x00);
	mis2032_write_register(ViPipe, 0x3996, 0x00);
	mis2032_write_register(ViPipe, 0x393d, 0x82);
	mis2032_write_register(ViPipe, 0x393c, 0x00);
	mis2032_write_register(ViPipe, 0x393f, 0xd1);
	mis2032_write_register(ViPipe, 0x393e, 0x01);
	mis2032_write_register(ViPipe, 0x3941, 0xc6);
	mis2032_write_register(ViPipe, 0x3940, 0x02);
	mis2032_write_register(ViPipe, 0x3943, 0x66);
	mis2032_write_register(ViPipe, 0x3942, 0x06);
	mis2032_write_register(ViPipe, 0x3945, 0x00);
	mis2032_write_register(ViPipe, 0x3944, 0x00);
	mis2032_write_register(ViPipe, 0x3947, 0x04);
	mis2032_write_register(ViPipe, 0x3946, 0x01);
	mis2032_write_register(ViPipe, 0x3949, 0x04);
	mis2032_write_register(ViPipe, 0x3948, 0x01);
	mis2032_write_register(ViPipe, 0x394b, 0x3c);
	mis2032_write_register(ViPipe, 0x394a, 0x09);
	mis2032_write_register(ViPipe, 0x395a, 0x00);
	mis2032_write_register(ViPipe, 0x3959, 0x00);
	mis2032_write_register(ViPipe, 0x395c, 0x0a);
	mis2032_write_register(ViPipe, 0x395b, 0x00);
	mis2032_write_register(ViPipe, 0x395e, 0x9f);
	mis2032_write_register(ViPipe, 0x395d, 0x02);
	mis2032_write_register(ViPipe, 0x3960, 0xf7);
	mis2032_write_register(ViPipe, 0x395f, 0x05);
	mis2032_write_register(ViPipe, 0x3956, 0x0a);
	mis2032_write_register(ViPipe, 0x3955, 0x00);
	mis2032_write_register(ViPipe, 0x3958, 0x3c);
	mis2032_write_register(ViPipe, 0x3957, 0x09);
	mis2032_write_register(ViPipe, 0x3962, 0x00);
	mis2032_write_register(ViPipe, 0x3961, 0x00);
	mis2032_write_register(ViPipe, 0x3964, 0x95);
	mis2032_write_register(ViPipe, 0x3963, 0x00);
	mis2032_write_register(ViPipe, 0x3966, 0x00);
	mis2032_write_register(ViPipe, 0x3965, 0x00);
	mis2032_write_register(ViPipe, 0x3968, 0x82);
	mis2032_write_register(ViPipe, 0x3967, 0x00);
	mis2032_write_register(ViPipe, 0x3989, 0x00);
	mis2032_write_register(ViPipe, 0x3988, 0x00);
	mis2032_write_register(ViPipe, 0x398b, 0xba);
	mis2032_write_register(ViPipe, 0x398a, 0x00);
	mis2032_write_register(ViPipe, 0x398d, 0x00);
	mis2032_write_register(ViPipe, 0x398c, 0x00);
	mis2032_write_register(ViPipe, 0x398f, 0x95);
	mis2032_write_register(ViPipe, 0x398e, 0x00);
	mis2032_write_register(ViPipe, 0x396a, 0x6e);
	mis2032_write_register(ViPipe, 0x3969, 0x09);
	mis2032_write_register(ViPipe, 0x396d, 0x00);
	mis2032_write_register(ViPipe, 0x396c, 0x01);
	mis2032_write_register(ViPipe, 0x396f, 0x60);
	mis2032_write_register(ViPipe, 0x396e, 0x00);
	mis2032_write_register(ViPipe, 0x3971, 0x60);
	mis2032_write_register(ViPipe, 0x3970, 0x00);
	mis2032_write_register(ViPipe, 0x3973, 0x60);
	mis2032_write_register(ViPipe, 0x3972, 0x00);
	mis2032_write_register(ViPipe, 0x3975, 0x60);
	mis2032_write_register(ViPipe, 0x3974, 0x00);
	mis2032_write_register(ViPipe, 0x3977, 0x60);
	mis2032_write_register(ViPipe, 0x3976, 0x00);
	mis2032_write_register(ViPipe, 0x3979, 0xa0);
	mis2032_write_register(ViPipe, 0x3978, 0x01);
	mis2032_write_register(ViPipe, 0x397b, 0xa0);
	mis2032_write_register(ViPipe, 0x397a, 0x01);
	mis2032_write_register(ViPipe, 0x397d, 0xa0);
	mis2032_write_register(ViPipe, 0x397c, 0x01);
	mis2032_write_register(ViPipe, 0x397f, 0xa0);
	mis2032_write_register(ViPipe, 0x397e, 0x01);
	mis2032_write_register(ViPipe, 0x3981, 0xa0);
	mis2032_write_register(ViPipe, 0x3980, 0x01);
	mis2032_write_register(ViPipe, 0x3983, 0xa0);
	mis2032_write_register(ViPipe, 0x3982, 0x01);
	mis2032_write_register(ViPipe, 0x3985, 0xa0);
	mis2032_write_register(ViPipe, 0x3984, 0x05);
	mis2032_write_register(ViPipe, 0x3c42, 0x03);
	mis2032_write_register(ViPipe, 0x3012, 0x2b);
	mis2032_write_register(ViPipe, 0x3600, 0x63);
	mis2032_write_register(ViPipe, 0x3609, 0x10);
	mis2032_write_register(ViPipe, 0x3630, 0x00);
	mis2032_write_register(ViPipe, 0x3631, 0xFF);
	mis2032_write_register(ViPipe, 0x3632, 0xFF);
	mis2032_write_register(ViPipe, 0x364e, 0x63);
	mis2032_write_register(ViPipe, 0x3657, 0x10);
	mis2032_write_register(ViPipe, 0x367e, 0x00);
	mis2032_write_register(ViPipe, 0x367f, 0xFF);
	mis2032_write_register(ViPipe, 0x3680, 0xFF);
	mis2032_write_register(ViPipe, 0x369c, 0x63);
	mis2032_write_register(ViPipe, 0x36A5, 0x10);
	mis2032_write_register(ViPipe, 0x36cc, 0x00);
	mis2032_write_register(ViPipe, 0x36cd, 0xFF);
	mis2032_write_register(ViPipe, 0x36ce, 0xFF);
	mis2032_write_register(ViPipe, 0x3a00, 0x00);
	mis2032_write_register(ViPipe, 0x3706, 0x04); //blc
	mis2032_write_register(ViPipe, 0x3707, 0x00);
	mis2032_write_register(ViPipe, 0x3708, 0x00);
	mis2032_write_register(ViPipe, 0x3709, 0x00);
	mis2032_write_register(ViPipe, 0x370a, 0x00);
	mis2032_write_register(ViPipe, 0x370b, 0x00);
	mis2032_write_register(ViPipe, 0x390c, 0xd0);
	mis2032_write_register(ViPipe, 0x210b, 0x00);
	mis2032_write_register(ViPipe, 0x3021, 0x00);
	mis2032_write_register(ViPipe, 0x3a04, 0x03);
	mis2032_write_register(ViPipe, 0x3a05, 0x78);
	mis2032_write_register(ViPipe, 0x3a0a, 0x3a);
	mis2032_write_register(ViPipe, 0x3a0d, 0x17);
	mis2032_write_register(ViPipe, 0x3a2a, 0x14);
	mis2032_write_register(ViPipe, 0x3a2e, 0x10);
	mis2032_write_register(ViPipe, 0x3a14, 0x00);
	mis2032_write_register(ViPipe, 0x3a1c, 0x01);
	mis2032_write_register(ViPipe, 0x3a36, 0x01);
	mis2032_write_register(ViPipe, 0x3a07, 0x56);
	mis2032_write_register(ViPipe, 0x3a35, 0x07);
	mis2032_write_register(ViPipe, 0x3a30, 0x52);
	mis2032_write_register(ViPipe, 0x3a31, 0x35);
	mis2032_write_register(ViPipe, 0x3a32, 0x00);
	mis2032_write_register(ViPipe, 0x3a19, 0x08);
	mis2032_write_register(ViPipe, 0x3a1a, 0x08);
	mis2032_write_register(ViPipe, 0x3a36, 0x01);

	mis2032_write_register(ViPipe, 0x3006, 0x00);

	// mis2032_write_register(ViPipe, 0x300b, 0x01);

	// mis2032_write_register(ViPipe, 0x3006, 0x02);
	// delay_ms(40);
	// mis2032_write_register(ViPipe, 0x3006, 0x00);

	mis2032_default_reg_init(ViPipe);


	delay_ms(100);

	printf("ViPipe:%d,===MIS2032 1080P 25fps 10bit LINEAR Init OK!===\n", ViPipe);
}
/* wdr mode1080p25 */
static void mis2032_wdr_1080p25_init(VI_PIPE ViPipe)
{
	/* [ParaList] */
	mis2032_write_register(ViPipe, 0x300b, 0x01);
	mis2032_write_register(ViPipe, 0x3006, 0x02);

	delay_ms(23);//延时23ms
	mis2032_write_register(ViPipe, 0x330c, 0x01);
	/*inc driver power*/
	mis2032_write_register(ViPipe, 0x3b00, 0x01); //0x07
	mis2032_write_register(ViPipe, 0x3b01, 0xff); //0xff
	mis2032_write_register(ViPipe, 0x3b03, 0x2f);

	mis2032_write_register(ViPipe, 0x3a00, 0x80);
	mis2032_write_register(ViPipe, 0x3300, 0x7b);
	mis2032_write_register(ViPipe, 0x3301, 0x01);
	mis2032_write_register(ViPipe, 0x3302, 0x01);
	mis2032_write_register(ViPipe, 0x3303, 0x03);
	mis2032_write_register(ViPipe, 0x3309, 0x01);
	mis2032_write_register(ViPipe, 0x3307, 0x01);
	mis2032_write_register(ViPipe, 0x330b, 0x0a);
	mis2032_write_register(ViPipe, 0x3021, 0x02);
	mis2032_write_register(ViPipe, 0x3201, 0xe5); //0xe5
	mis2032_write_register(ViPipe, 0x3200, 0x05); //0x05
	mis2032_write_register(ViPipe, 0x3203, 0x98);
	mis2032_write_register(ViPipe, 0x3202, 0x08);
	mis2032_write_register(ViPipe, 0x3205, 0x00);
	mis2032_write_register(ViPipe, 0x3204, 0x00);
	mis2032_write_register(ViPipe, 0x3207, 0x3b);
	mis2032_write_register(ViPipe, 0x3206, 0x04);
	mis2032_write_register(ViPipe, 0x3209, 0x08);
	mis2032_write_register(ViPipe, 0x3208, 0x00);
	mis2032_write_register(ViPipe, 0x320b, 0x87);
	mis2032_write_register(ViPipe, 0x320a, 0x07);
	mis2032_write_register(ViPipe, 0x3102, 0x00);
	mis2032_write_register(ViPipe, 0x3105, 0x00);
	mis2032_write_register(ViPipe, 0x3108, 0x00);
	mis2032_write_register(ViPipe, 0x3007, 0x00);
	mis2032_write_register(ViPipe, 0x300a, 0x01);
	mis2032_write_register(ViPipe, 0x3014, 0x00);
	mis2032_write_register(ViPipe, 0x330f, 0x00);
	mis2032_write_register(ViPipe, 0x310f, 0x01);
	mis2032_write_register(ViPipe, 0x3986, 0x02);
	mis2032_write_register(ViPipe, 0x3986, 0x02);
	mis2032_write_register(ViPipe, 0x3900, 0x00);
	mis2032_write_register(ViPipe, 0x3902, 0x13);
	mis2032_write_register(ViPipe, 0x3901, 0x00);
	mis2032_write_register(ViPipe, 0x3904, 0x4c);
	mis2032_write_register(ViPipe, 0x3903, 0x09);
	mis2032_write_register(ViPipe, 0x3906, 0xff);
	mis2032_write_register(ViPipe, 0x3905, 0x1f);
	mis2032_write_register(ViPipe, 0x3908, 0xff);
	mis2032_write_register(ViPipe, 0x3907, 0x1f);
	mis2032_write_register(ViPipe, 0x390a, 0xb3);
	mis2032_write_register(ViPipe, 0x3909, 0x02);
	mis2032_write_register(ViPipe, 0x390c, 0xf7);
	mis2032_write_register(ViPipe, 0x390b, 0x05);
	mis2032_write_register(ViPipe, 0x390e, 0x36);
	mis2032_write_register(ViPipe, 0x390d, 0x09);
	mis2032_write_register(ViPipe, 0x3910, 0xff);
	mis2032_write_register(ViPipe, 0x390f, 0x1f);
	mis2032_write_register(ViPipe, 0x3911, 0x01);
	mis2032_write_register(ViPipe, 0x3917, 0x00);
	mis2032_write_register(ViPipe, 0x3916, 0x00);
	mis2032_write_register(ViPipe, 0x3919, 0xa1);
	mis2032_write_register(ViPipe, 0x3918, 0x01);
	mis2032_write_register(ViPipe, 0x3913, 0x13);
	mis2032_write_register(ViPipe, 0x3912, 0x00);
	mis2032_write_register(ViPipe, 0x3915, 0xc6);
	mis2032_write_register(ViPipe, 0x3914, 0x02);
	mis2032_write_register(ViPipe, 0x391b, 0x00);
	mis2032_write_register(ViPipe, 0x391a, 0x00);
	mis2032_write_register(ViPipe, 0x391d, 0x48);
	mis2032_write_register(ViPipe, 0x391c, 0x09);
	mis2032_write_register(ViPipe, 0x391f, 0xff);
	mis2032_write_register(ViPipe, 0x391e, 0x1f);
	mis2032_write_register(ViPipe, 0x3921, 0xff);
	mis2032_write_register(ViPipe, 0x3920, 0x1f);
	mis2032_write_register(ViPipe, 0x3923, 0x00);
	mis2032_write_register(ViPipe, 0x3922, 0x00);
	mis2032_write_register(ViPipe, 0x3925, 0xb9);
	mis2032_write_register(ViPipe, 0x3924, 0x02);
	mis2032_write_register(ViPipe, 0x394c, 0x00);
	mis2032_write_register(ViPipe, 0x394e, 0x82);
	mis2032_write_register(ViPipe, 0x394d, 0x00);
	mis2032_write_register(ViPipe, 0x3950, 0x95);
	mis2032_write_register(ViPipe, 0x394f, 0x00);
	mis2032_write_register(ViPipe, 0x3952, 0x70);
	mis2032_write_register(ViPipe, 0x3951, 0x00);
	mis2032_write_register(ViPipe, 0x3954, 0xea);
	mis2032_write_register(ViPipe, 0x3953, 0x02);
	mis2032_write_register(ViPipe, 0x3927, 0x00);
	mis2032_write_register(ViPipe, 0x3926, 0x00);
	mis2032_write_register(ViPipe, 0x3929, 0xdf);
	mis2032_write_register(ViPipe, 0x3928, 0x00);
	mis2032_write_register(ViPipe, 0x392b, 0xd1);
	mis2032_write_register(ViPipe, 0x392a, 0x01);
	mis2032_write_register(ViPipe, 0x392d, 0xa1);
	mis2032_write_register(ViPipe, 0x392c, 0x02);
	mis2032_write_register(ViPipe, 0x392f, 0x66);
	mis2032_write_register(ViPipe, 0x392e, 0x06);
	mis2032_write_register(ViPipe, 0x3931, 0x36);
	mis2032_write_register(ViPipe, 0x3930, 0x09);
	mis2032_write_register(ViPipe, 0x3933, 0x36);
	mis2032_write_register(ViPipe, 0x3932, 0x09);
	mis2032_write_register(ViPipe, 0x3935, 0x36);
	mis2032_write_register(ViPipe, 0x3934, 0x09);
	mis2032_write_register(ViPipe, 0x3937, 0x36);
	mis2032_write_register(ViPipe, 0x3936, 0x09);
	mis2032_write_register(ViPipe, 0x3939, 0x36);
	mis2032_write_register(ViPipe, 0x3938, 0x09);
	mis2032_write_register(ViPipe, 0x393b, 0x36);
	mis2032_write_register(ViPipe, 0x393a, 0x09);
	mis2032_write_register(ViPipe, 0x3991, 0x40);
	mis2032_write_register(ViPipe, 0x3990, 0x00);
	mis2032_write_register(ViPipe, 0x3993, 0x56);
	mis2032_write_register(ViPipe, 0x3992, 0x09);
	mis2032_write_register(ViPipe, 0x3995, 0xff);
	mis2032_write_register(ViPipe, 0x3994, 0x1f);
	mis2032_write_register(ViPipe, 0x3997, 0x00);
	mis2032_write_register(ViPipe, 0x3996, 0x00);
	mis2032_write_register(ViPipe, 0x393d, 0x82);
	mis2032_write_register(ViPipe, 0x393c, 0x00);
	mis2032_write_register(ViPipe, 0x393f, 0xd1);
	mis2032_write_register(ViPipe, 0x393e, 0x01);
	mis2032_write_register(ViPipe, 0x3941, 0xc6);
	mis2032_write_register(ViPipe, 0x3940, 0x02);
	mis2032_write_register(ViPipe, 0x3943, 0x66);
	mis2032_write_register(ViPipe, 0x3942, 0x06);
	mis2032_write_register(ViPipe, 0x3945, 0x00);
	mis2032_write_register(ViPipe, 0x3944, 0x00);
	mis2032_write_register(ViPipe, 0x3947, 0x04);
	mis2032_write_register(ViPipe, 0x3946, 0x01);
	mis2032_write_register(ViPipe, 0x3949, 0x04);
	mis2032_write_register(ViPipe, 0x3948, 0x01);
	mis2032_write_register(ViPipe, 0x394b, 0x3c);
	mis2032_write_register(ViPipe, 0x394a, 0x09);
	mis2032_write_register(ViPipe, 0x395a, 0x00);
	mis2032_write_register(ViPipe, 0x3959, 0x00);
	mis2032_write_register(ViPipe, 0x395c, 0x0a);
	mis2032_write_register(ViPipe, 0x395b, 0x00);
	mis2032_write_register(ViPipe, 0x395e, 0x9f);
	mis2032_write_register(ViPipe, 0x395d, 0x02);
	mis2032_write_register(ViPipe, 0x3960, 0xf7);
	mis2032_write_register(ViPipe, 0x395f, 0x05);
	mis2032_write_register(ViPipe, 0x3956, 0x0a);
	mis2032_write_register(ViPipe, 0x3955, 0x00);
	mis2032_write_register(ViPipe, 0x3958, 0x3c);
	mis2032_write_register(ViPipe, 0x3957, 0x09);
	mis2032_write_register(ViPipe, 0x3962, 0x00);
	mis2032_write_register(ViPipe, 0x3961, 0x00);
	mis2032_write_register(ViPipe, 0x3964, 0x95);
	mis2032_write_register(ViPipe, 0x3963, 0x00);
	mis2032_write_register(ViPipe, 0x3966, 0x00);
	mis2032_write_register(ViPipe, 0x3965, 0x00);
	mis2032_write_register(ViPipe, 0x3968, 0x82);
	mis2032_write_register(ViPipe, 0x3967, 0x00);
	mis2032_write_register(ViPipe, 0x3989, 0x00);
	mis2032_write_register(ViPipe, 0x3988, 0x00);
	mis2032_write_register(ViPipe, 0x398b, 0xba);
	mis2032_write_register(ViPipe, 0x398a, 0x00);
	mis2032_write_register(ViPipe, 0x398d, 0x00);
	mis2032_write_register(ViPipe, 0x398c, 0x00);
	mis2032_write_register(ViPipe, 0x398f, 0x95);
	mis2032_write_register(ViPipe, 0x398e, 0x00);
	mis2032_write_register(ViPipe, 0x396a, 0x6e);
	mis2032_write_register(ViPipe, 0x3969, 0x09);
	mis2032_write_register(ViPipe, 0x396d, 0x00);
	mis2032_write_register(ViPipe, 0x396c, 0x01);
	mis2032_write_register(ViPipe, 0x396f, 0x60);
	mis2032_write_register(ViPipe, 0x396e, 0x00);
	mis2032_write_register(ViPipe, 0x3971, 0x60);
	mis2032_write_register(ViPipe, 0x3970, 0x00);
	mis2032_write_register(ViPipe, 0x3973, 0x60);
	mis2032_write_register(ViPipe, 0x3972, 0x00);
	mis2032_write_register(ViPipe, 0x3975, 0x60);
	mis2032_write_register(ViPipe, 0x3974, 0x00);
	mis2032_write_register(ViPipe, 0x3977, 0x60);
	mis2032_write_register(ViPipe, 0x3976, 0x00);
	mis2032_write_register(ViPipe, 0x3979, 0xa0);
	mis2032_write_register(ViPipe, 0x3978, 0x01);
	mis2032_write_register(ViPipe, 0x397b, 0xa0);
	mis2032_write_register(ViPipe, 0x397a, 0x01);
	mis2032_write_register(ViPipe, 0x397d, 0xa0);
	mis2032_write_register(ViPipe, 0x397c, 0x01);
	mis2032_write_register(ViPipe, 0x397f, 0xa0);
	mis2032_write_register(ViPipe, 0x397e, 0x01);
	mis2032_write_register(ViPipe, 0x3981, 0xa0);
	mis2032_write_register(ViPipe, 0x3980, 0x01);
	mis2032_write_register(ViPipe, 0x3983, 0xa0);
	mis2032_write_register(ViPipe, 0x3982, 0x01);
	mis2032_write_register(ViPipe, 0x3985, 0xa0);
	mis2032_write_register(ViPipe, 0x3984, 0x05);
	mis2032_write_register(ViPipe, 0x3c42, 0x03);
	mis2032_write_register(ViPipe, 0x3012, 0x2b);
	mis2032_write_register(ViPipe, 0x3600, 0x63);
	mis2032_write_register(ViPipe, 0x3609, 0x10);
	mis2032_write_register(ViPipe, 0x3630, 0x00);
	mis2032_write_register(ViPipe, 0x3631, 0xFF);
	mis2032_write_register(ViPipe, 0x3632, 0xFF);
	mis2032_write_register(ViPipe, 0x364e, 0x63);
	mis2032_write_register(ViPipe, 0x3657, 0x10);
	mis2032_write_register(ViPipe, 0x367e, 0x00);
	mis2032_write_register(ViPipe, 0x367f, 0xFF);
	mis2032_write_register(ViPipe, 0x3680, 0xFF);
	mis2032_write_register(ViPipe, 0x369c, 0x63);
	mis2032_write_register(ViPipe, 0x36A5, 0x10);
	mis2032_write_register(ViPipe, 0x36cc, 0x00);
	mis2032_write_register(ViPipe, 0x36cd, 0xFF);
	mis2032_write_register(ViPipe, 0x36ce, 0xFF);
	mis2032_write_register(ViPipe, 0x3a00, 0x00);
	mis2032_write_register(ViPipe, 0x3706, 0x04); //blc
	mis2032_write_register(ViPipe, 0x3707, 0x00);
	mis2032_write_register(ViPipe, 0x3708, 0x04);
	mis2032_write_register(ViPipe, 0x3709, 0x00);
	mis2032_write_register(ViPipe, 0x370a, 0x00);
	mis2032_write_register(ViPipe, 0x370b, 0x00);
	mis2032_write_register(ViPipe, 0x390c, 0xd0);
	mis2032_write_register(ViPipe, 0x210b, 0x00);
	mis2032_write_register(ViPipe, 0x3021, 0x00);
	mis2032_write_register(ViPipe, 0x3a04, 0x03);
	mis2032_write_register(ViPipe, 0x3a05, 0x78);
	mis2032_write_register(ViPipe, 0x3a0a, 0x3a);
	mis2032_write_register(ViPipe, 0x3a0d, 0x17);
	mis2032_write_register(ViPipe, 0x3a2a, 0x14);
	mis2032_write_register(ViPipe, 0x3a2e, 0x10);
	mis2032_write_register(ViPipe, 0x3a14, 0x00);
	mis2032_write_register(ViPipe, 0x3a1c, 0x01);
	mis2032_write_register(ViPipe, 0x3a36, 0x01);
	mis2032_write_register(ViPipe, 0x3a07, 0x56);
	mis2032_write_register(ViPipe, 0x3a35, 0x07);
	mis2032_write_register(ViPipe, 0x3a30, 0x52);
	mis2032_write_register(ViPipe, 0x3a31, 0x35);
	mis2032_write_register(ViPipe, 0x3a32, 0x00);
	mis2032_write_register(ViPipe, 0x3a19, 0x08);
	mis2032_write_register(ViPipe, 0x3a1a, 0x08);
	mis2032_write_register(ViPipe, 0x3a36, 0x01);

	mis2032_write_register(ViPipe, 0x3006, 0x00);

	// mis2032_write_register(ViPipe, 0x300b, 0x01);

	// mis2032_write_register(ViPipe, 0x3006, 0x02);
	// delay_ms(23);
	// mis2032_write_register(ViPipe, 0x3006, 0x00);

	mis2032_default_reg_init(ViPipe);


	delay_ms(100);

	printf("ViPipe:%d,===MIS2032 WDR1080P 25fps 10bit Init OK!===\n", ViPipe);
}


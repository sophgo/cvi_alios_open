#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "ar2020_cmos_ex.h"
#include "sensor_i2c.h"

static void ar2020_linear_3840p30_init(VI_PIPE ViPipe);

const CVI_U32 ar2020_addr_byte = 2;
const CVI_U32 ar2020_data_byte = 2;

int ar2020_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunAR2020_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunAr2020_AddrInfo[ViPipe].s8I2cAddr);
}

int ar2020_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunAR2020_BusInfo[ViPipe].s8I2cDev);
}

int ar2020_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunAR2020_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunAr2020_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							ar2020_addr_byte, ar2020_data_byte);
}

int ar2020_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunAR2020_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunAr2020_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							ar2020_addr_byte, (CVI_U32)data, ar2020_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void ar2020_standby(VI_PIPE ViPipe)
{
	ar2020_write_register(ViPipe, 0x0100, 0x00);
}

void ar2020_restart(VI_PIPE ViPipe)
{
	ar2020_write_register(ViPipe, 0x0100, 0x00);
	delay_ms(20);
	ar2020_write_register(ViPipe, 0x0100, 0x01);
}

void ar2020_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastAR2020[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		ar2020_write_register(ViPipe,
				g_pastAR2020[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastAR2020[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void ar2020_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 val = 0;

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		break;
	case ISP_SNS_MIRROR:
		val |= 0x01;
		break;
	case ISP_SNS_FLIP:
		val |= 0x02;
		break;
	case ISP_SNS_MIRROR_FLIP:
		val |= 0x3;
		break;
	default:
		return;
	}
	ar2020_write_register(ViPipe, 0x0101, val);
}

#define AR2020_CHIP_ID_HI_ADDR		0x3107
#define AR2020_CHIP_ID_LO_ADDR		0x3108
#define AR2020_CHIP_ID			    0x9c42

int ar2020_probe(VI_PIPE ViPipe)
{
	//int nVal;
	//CVI_U16 chip_id;

	//if (ar2020_i2c_init(ViPipe) != CVI_SUCCESS)
	//	return CVI_FAILURE;

	//delay_ms(5);

	//nVal = ar2020_read_register(ViPipe, AR2020_CHIP_ID_HI_ADDR);
	//if (nVal < 0) {
	//	CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
	//	return nVal;
	//}
	//chip_id = (nVal & 0xFF) << 8;
	//nVal = ar2020_read_register(ViPipe, AR2020_CHIP_ID_LO_ADDR);
	//if (nVal < 0) {
	//	CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
	//	return nVal;
	//}
	//chip_id |= (nVal & 0xFF);

	//if (chip_id != AR2020_CHIP_ID) {
	//	CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
	//	return CVI_FAILURE;
	//}
	printf("%d\n", ViPipe);
	return CVI_SUCCESS;
}


void ar2020_init(VI_PIPE ViPipe)
{
	ar2020_i2c_init(ViPipe);

	ar2020_linear_3840p30_init(ViPipe);

	g_pastAR2020[ViPipe]->bInit = CVI_TRUE;
}

/* 3840P30 and 3840P25 */
static void ar2020_linear_3840p30_init(VI_PIPE ViPipe)
{
	ar2020_write_register(ViPipe, 0x44D6, 0xF206);
	ar2020_write_register(ViPipe, 0x0100, 0x0000);
	ar2020_write_register(ViPipe, 0x0304, 0x0002);
	ar2020_write_register(ViPipe, 0x0306, 0x0067);
	ar2020_write_register(ViPipe, 0x0300, 0x0006);
	ar2020_write_register(ViPipe, 0x0302, 0x0001);
	ar2020_write_register(ViPipe, 0x030C, 0x0007);
	ar2020_write_register(ViPipe, 0x030E, 0x0118);
	ar2020_write_register(ViPipe, 0x0308, 0x000A);
	ar2020_write_register(ViPipe, 0x030A, 0x0001);
	ar2020_write_register(ViPipe, 0x0344, 0x0008);
	ar2020_write_register(ViPipe, 0x0348, 0x1407);
	ar2020_write_register(ViPipe, 0x0346, 0x0008);
	ar2020_write_register(ViPipe, 0x034A, 0x0F07);
	ar2020_write_register(ViPipe, 0x034C, 0x1400);
	ar2020_write_register(ViPipe, 0x034E, 0x0F00);
	ar2020_write_register(ViPipe, 0x0380, 0x0001);
	ar2020_write_register(ViPipe, 0x0382, 0x0001);
	ar2020_write_register(ViPipe, 0x0386, 0x0001);
	ar2020_write_register(ViPipe, 0x0384, 0x0001);
	ar2020_write_register(ViPipe, 0x0900, 0x0011);

	ar2020_write_register(ViPipe, 0x0342, 0x5B90);
	ar2020_write_register(ViPipe, 0x0340, 0x0F1E);
	ar2020_write_register(ViPipe, 0x0202, 0x079E);
	ar2020_write_register(ViPipe, 0x0112, 0x0A0A);

	ar2020_write_register(ViPipe, 0x0114, 0x0300);
	ar2020_write_register(ViPipe, 0x0800, 0x0a05);
	ar2020_write_register(ViPipe, 0x0802, 0x0807);
	ar2020_write_register(ViPipe, 0x0804, 0x0805);
	ar2020_write_register(ViPipe, 0x0806, 0x1805);
	ar2020_write_register(ViPipe, 0x082A, 0x0a0b);
	ar2020_write_register(ViPipe, 0x082C, 0x0900);

	ar2020_write_register(ViPipe, 0x3F06, 0x00C0);
	ar2020_write_register(ViPipe, 0x3F0A, 0xA000);
	ar2020_write_register(ViPipe, 0x3F0C, 0x0007);
	ar2020_write_register(ViPipe, 0x3F20, 0x8008);
	ar2020_write_register(ViPipe, 0x3F1E, 0x0000);
	ar2020_write_register(ViPipe, 0x4000, 0x0114);
	ar2020_write_register(ViPipe, 0x4002, 0x1A25);
	ar2020_write_register(ViPipe, 0x4004, 0x3DFF);
	ar2020_write_register(ViPipe, 0x4006, 0xFFFF);
	ar2020_write_register(ViPipe, 0x4008, 0x0A35);
	ar2020_write_register(ViPipe, 0x400A, 0x10EF);
	ar2020_write_register(ViPipe, 0x400C, 0x3003);
	ar2020_write_register(ViPipe, 0x400E, 0x30D8);
	ar2020_write_register(ViPipe, 0x4010, 0xF003);
	ar2020_write_register(ViPipe, 0x4012, 0xB5F0);
	ar2020_write_register(ViPipe, 0x4014, 0x0085);
	ar2020_write_register(ViPipe, 0x4016, 0xF004);
	ar2020_write_register(ViPipe, 0x4018, 0x9A89);
	ar2020_write_register(ViPipe, 0x401A, 0xF000);
	ar2020_write_register(ViPipe, 0x401C, 0x9997);
	ar2020_write_register(ViPipe, 0x401E, 0xF000);
	ar2020_write_register(ViPipe, 0x4020, 0x30C0);
	ar2020_write_register(ViPipe, 0x4022, 0xF000);
	ar2020_write_register(ViPipe, 0x4024, 0x82F0);
	ar2020_write_register(ViPipe, 0x4026, 0x0030);
	ar2020_write_register(ViPipe, 0x4028, 0x18F0);
	ar2020_write_register(ViPipe, 0x402A, 0x0320);
	ar2020_write_register(ViPipe, 0x402C, 0x58F0);
	ar2020_write_register(ViPipe, 0x402E, 0x089C);
	ar2020_write_register(ViPipe, 0x4030, 0xF010);
	ar2020_write_register(ViPipe, 0x4032, 0x99B6);
	ar2020_write_register(ViPipe, 0x4034, 0xF003);
	ar2020_write_register(ViPipe, 0x4036, 0xB498);
	ar2020_write_register(ViPipe, 0x4038, 0xA096);
	ar2020_write_register(ViPipe, 0x403A, 0xF000);
	ar2020_write_register(ViPipe, 0x403C, 0xA2F0);
	ar2020_write_register(ViPipe, 0x403E, 0x00A2);
	ar2020_write_register(ViPipe, 0x4040, 0xF008);
	ar2020_write_register(ViPipe, 0x4042, 0x9DF0);
	ar2020_write_register(ViPipe, 0x4044, 0x209D);
	ar2020_write_register(ViPipe, 0x4046, 0x8C08);
	ar2020_write_register(ViPipe, 0x4048, 0x08F0);
	ar2020_write_register(ViPipe, 0x404A, 0x0036);
	ar2020_write_register(ViPipe, 0x404C, 0x008F);
	ar2020_write_register(ViPipe, 0x404E, 0xF000);
	ar2020_write_register(ViPipe, 0x4050, 0x88F0);
	ar2020_write_register(ViPipe, 0x4052, 0x0488);
	ar2020_write_register(ViPipe, 0x4054, 0xF000);
	ar2020_write_register(ViPipe, 0x4056, 0x3600);
	ar2020_write_register(ViPipe, 0x4058, 0xF000);
	ar2020_write_register(ViPipe, 0x405A, 0x83F0);
	ar2020_write_register(ViPipe, 0x405C, 0x0290);
	ar2020_write_register(ViPipe, 0x405E, 0xF000);
	ar2020_write_register(ViPipe, 0x4060, 0x8BF0);
	ar2020_write_register(ViPipe, 0x4062, 0x2EA3);
	ar2020_write_register(ViPipe, 0x4064, 0xF000);
	ar2020_write_register(ViPipe, 0x4066, 0xA3F0);
	ar2020_write_register(ViPipe, 0x4068, 0x089D);
	ar2020_write_register(ViPipe, 0x406A, 0xF075);
	ar2020_write_register(ViPipe, 0x406C, 0x3003);
	ar2020_write_register(ViPipe, 0x406E, 0x4070);
	ar2020_write_register(ViPipe, 0x4070, 0x216D);
	ar2020_write_register(ViPipe, 0x4072, 0x1CF6);
	ar2020_write_register(ViPipe, 0x4074, 0x8B00);
	ar2020_write_register(ViPipe, 0x4076, 0x5186);
	ar2020_write_register(ViPipe, 0x4078, 0x1300);
	ar2020_write_register(ViPipe, 0x407A, 0x0205);
	ar2020_write_register(ViPipe, 0x407C, 0x36D8);
	ar2020_write_register(ViPipe, 0x407E, 0xF002);
	ar2020_write_register(ViPipe, 0x4080, 0x8387);
	ar2020_write_register(ViPipe, 0x4082, 0xF006);
	ar2020_write_register(ViPipe, 0x4084, 0x8702);
	ar2020_write_register(ViPipe, 0x4086, 0x0D02);
	ar2020_write_register(ViPipe, 0x4088, 0x05F0);
	ar2020_write_register(ViPipe, 0x408A, 0x0383);
	ar2020_write_register(ViPipe, 0x408C, 0xF001);
	ar2020_write_register(ViPipe, 0x408E, 0x87F0);
	ar2020_write_register(ViPipe, 0x4090, 0x0213);
	ar2020_write_register(ViPipe, 0x4092, 0x0036);
	ar2020_write_register(ViPipe, 0x4094, 0xD887);
	ar2020_write_register(ViPipe, 0x4096, 0x020D);
	ar2020_write_register(ViPipe, 0x4098, 0xE0E0);
	ar2020_write_register(ViPipe, 0x409A, 0xE0E0);
	ar2020_write_register(ViPipe, 0x409C, 0xE0E0);
	ar2020_write_register(ViPipe, 0x409E, 0xE0E0);
	ar2020_write_register(ViPipe, 0x40A0, 0xF000);
	ar2020_write_register(ViPipe, 0x40A2, 0x0401);
	ar2020_write_register(ViPipe, 0x40A4, 0xF008);
	ar2020_write_register(ViPipe, 0x40A6, 0x82F0);
	ar2020_write_register(ViPipe, 0x40A8, 0x0883);
	ar2020_write_register(ViPipe, 0x40AA, 0xF009);
	ar2020_write_register(ViPipe, 0x40AC, 0x85F0);
	ar2020_write_register(ViPipe, 0x40AE, 0x2985);
	ar2020_write_register(ViPipe, 0x40B0, 0x87F0);
	ar2020_write_register(ViPipe, 0x40B2, 0x2A87);
	ar2020_write_register(ViPipe, 0x40B4, 0xF63E);
	ar2020_write_register(ViPipe, 0x40B6, 0x88F0);
	ar2020_write_register(ViPipe, 0x40B8, 0x0801);
	ar2020_write_register(ViPipe, 0x40BA, 0x40F0);
	ar2020_write_register(ViPipe, 0x40BC, 0x0800);
	ar2020_write_register(ViPipe, 0x40BE, 0x48F0);
	ar2020_write_register(ViPipe, 0x40C0, 0x0882);
	ar2020_write_register(ViPipe, 0x40C2, 0xF008);
	ar2020_write_register(ViPipe, 0x40C4, 0x0401);
	ar2020_write_register(ViPipe, 0x40C6, 0xF008);
	ar2020_write_register(ViPipe, 0x40C8, 0xE0E0);
	ar2020_write_register(ViPipe, 0x40CA, 0xE0E0);
	ar2020_write_register(ViPipe, 0x40CC, 0xE0E0);
	ar2020_write_register(ViPipe, 0x40CE, 0xE0E0);
	ar2020_write_register(ViPipe, 0x40D0, 0xF000);
	ar2020_write_register(ViPipe, 0x40D2, 0x0401);
	ar2020_write_register(ViPipe, 0x40D4, 0xF015);
	ar2020_write_register(ViPipe, 0x40D6, 0x002C);
	ar2020_write_register(ViPipe, 0x40D8, 0xF00E);
	ar2020_write_register(ViPipe, 0x40DA, 0x85F0);
	ar2020_write_register(ViPipe, 0x40DC, 0x0687);
	ar2020_write_register(ViPipe, 0x40DE, 0xF002);
	ar2020_write_register(ViPipe, 0x40E0, 0x87F0);
	ar2020_write_register(ViPipe, 0x40E2, 0x61E8);
	ar2020_write_register(ViPipe, 0x40E4, 0x3900);
	ar2020_write_register(ViPipe, 0x40E6, 0xF005);
	ar2020_write_register(ViPipe, 0x40E8, 0x3480);
	ar2020_write_register(ViPipe, 0x40EA, 0xF000);
	ar2020_write_register(ViPipe, 0x40EC, 0x3240);
	ar2020_write_register(ViPipe, 0x40EE, 0xF000);
	ar2020_write_register(ViPipe, 0x40F0, 0x3900);
	ar2020_write_register(ViPipe, 0x40F2, 0xF00E);
	ar2020_write_register(ViPipe, 0x40F4, 0x3900);
	ar2020_write_register(ViPipe, 0x40F6, 0xF000);
	ar2020_write_register(ViPipe, 0x40F8, 0x3240);
	ar2020_write_register(ViPipe, 0x40FA, 0xF000);
	ar2020_write_register(ViPipe, 0x40FC, 0x3480);
	ar2020_write_register(ViPipe, 0x40FE, 0xF005);
	ar2020_write_register(ViPipe, 0x4100, 0xC0E6);
	ar2020_write_register(ViPipe, 0x4102, 0xF004);
	ar2020_write_register(ViPipe, 0x4104, 0x3900);
	ar2020_write_register(ViPipe, 0x4106, 0xF003);
	ar2020_write_register(ViPipe, 0x4108, 0xB0F0);
	ar2020_write_register(ViPipe, 0x410A, 0x0083);
	ar2020_write_register(ViPipe, 0x410C, 0xF000);
	ar2020_write_register(ViPipe, 0x410E, 0x86F0);
	ar2020_write_register(ViPipe, 0x4110, 0x0086);
	ar2020_write_register(ViPipe, 0x4112, 0xF089);
	ar2020_write_register(ViPipe, 0x4114, 0xB0F0);
	ar2020_write_register(ViPipe, 0x4116, 0x00E9);
	ar2020_write_register(ViPipe, 0x4118, 0xF000);
	ar2020_write_register(ViPipe, 0x411A, 0x8AF0);
	ar2020_write_register(ViPipe, 0x411C, 0x0000);
	ar2020_write_register(ViPipe, 0x411E, 0x05F0);
	ar2020_write_register(ViPipe, 0x4120, 0x00E0);
	ar2020_write_register(ViPipe, 0x4122, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4124, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4126, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4128, 0x0A35);
	ar2020_write_register(ViPipe, 0x412A, 0x10EF);
	ar2020_write_register(ViPipe, 0x412C, 0x3003);
	ar2020_write_register(ViPipe, 0x412E, 0x30D8);
	ar2020_write_register(ViPipe, 0x4130, 0xF005);
	ar2020_write_register(ViPipe, 0x4132, 0x85F0);
	ar2020_write_register(ViPipe, 0x4134, 0x049A);
	ar2020_write_register(ViPipe, 0x4136, 0x89F0);
	ar2020_write_register(ViPipe, 0x4138, 0x0099);
	ar2020_write_register(ViPipe, 0x413A, 0x97F0);
	ar2020_write_register(ViPipe, 0x413C, 0x0030);
	ar2020_write_register(ViPipe, 0x413E, 0xC0F0);
	ar2020_write_register(ViPipe, 0x4140, 0x0082);
	ar2020_write_register(ViPipe, 0x4142, 0xF000);
	ar2020_write_register(ViPipe, 0x4144, 0x3018);
	ar2020_write_register(ViPipe, 0x4146, 0xF002);
	ar2020_write_register(ViPipe, 0x4148, 0xB520);
	ar2020_write_register(ViPipe, 0x414A, 0x58F0);
	ar2020_write_register(ViPipe, 0x414C, 0x089C);
	ar2020_write_register(ViPipe, 0x414E, 0xF010);
	ar2020_write_register(ViPipe, 0x4150, 0x99B6);
	ar2020_write_register(ViPipe, 0x4152, 0xF003);
	ar2020_write_register(ViPipe, 0x4154, 0xB498);
	ar2020_write_register(ViPipe, 0x4156, 0xA096);
	ar2020_write_register(ViPipe, 0x4158, 0xF000);
	ar2020_write_register(ViPipe, 0x415A, 0xA2F0);
	ar2020_write_register(ViPipe, 0x415C, 0x00A2);
	ar2020_write_register(ViPipe, 0x415E, 0xF008);
	ar2020_write_register(ViPipe, 0x4160, 0x9DF0);
	ar2020_write_register(ViPipe, 0x4162, 0x209D);
	ar2020_write_register(ViPipe, 0x4164, 0x8C08);
	ar2020_write_register(ViPipe, 0x4166, 0x08F0);
	ar2020_write_register(ViPipe, 0x4168, 0x0036);
	ar2020_write_register(ViPipe, 0x416A, 0x008F);
	ar2020_write_register(ViPipe, 0x416C, 0x88F0);
	ar2020_write_register(ViPipe, 0x416E, 0x0188);
	ar2020_write_register(ViPipe, 0x4170, 0x3600);
	ar2020_write_register(ViPipe, 0x4172, 0xF000);
	ar2020_write_register(ViPipe, 0x4174, 0x83F0);
	ar2020_write_register(ViPipe, 0x4176, 0x0290);
	ar2020_write_register(ViPipe, 0x4178, 0xF001);
	ar2020_write_register(ViPipe, 0x417A, 0x8BF0);
	ar2020_write_register(ViPipe, 0x417C, 0x2DA3);
	ar2020_write_register(ViPipe, 0x417E, 0xF000);
	ar2020_write_register(ViPipe, 0x4180, 0xA3F0);
	ar2020_write_register(ViPipe, 0x4182, 0x089D);
	ar2020_write_register(ViPipe, 0x4184, 0xF06D);
	ar2020_write_register(ViPipe, 0x4186, 0x4070);
	ar2020_write_register(ViPipe, 0x4188, 0x3003);
	ar2020_write_register(ViPipe, 0x418A, 0x214D);
	ar2020_write_register(ViPipe, 0x418C, 0x1FF6);
	ar2020_write_register(ViPipe, 0x418E, 0x0851);
	ar2020_write_register(ViPipe, 0x4190, 0x0245);
	ar2020_write_register(ViPipe, 0x4192, 0x9D36);
	ar2020_write_register(ViPipe, 0x4194, 0xD8F0);
	ar2020_write_register(ViPipe, 0x4196, 0x0083);
	ar2020_write_register(ViPipe, 0x4198, 0xF000);
	ar2020_write_register(ViPipe, 0x419A, 0x87F0);
	ar2020_write_register(ViPipe, 0x419C, 0x0087);
	ar2020_write_register(ViPipe, 0x419E, 0xF000);
	ar2020_write_register(ViPipe, 0x41A0, 0x36D8);
	ar2020_write_register(ViPipe, 0x41A2, 0x020D);
	ar2020_write_register(ViPipe, 0x41A4, 0x0205);
	ar2020_write_register(ViPipe, 0x41A6, 0xF000);
	ar2020_write_register(ViPipe, 0x41A8, 0x36D8);
	ar2020_write_register(ViPipe, 0x41AA, 0xF000);
	ar2020_write_register(ViPipe, 0x41AC, 0x83F0);
	ar2020_write_register(ViPipe, 0x41AE, 0x0087);
	ar2020_write_register(ViPipe, 0x41B0, 0xF000);
	ar2020_write_register(ViPipe, 0x41B2, 0x87F0);
	ar2020_write_register(ViPipe, 0x41B4, 0x0036);
	ar2020_write_register(ViPipe, 0x41B6, 0xD802);
	ar2020_write_register(ViPipe, 0x41B8, 0x0D02);
	ar2020_write_register(ViPipe, 0x41BA, 0x05F0);
	ar2020_write_register(ViPipe, 0x41BC, 0x0036);
	ar2020_write_register(ViPipe, 0x41BE, 0xD8F0);
	ar2020_write_register(ViPipe, 0x41C0, 0x0083);
	ar2020_write_register(ViPipe, 0x41C2, 0xF000);
	ar2020_write_register(ViPipe, 0x41C4, 0x87F0);
	ar2020_write_register(ViPipe, 0x41C6, 0x0087);
	ar2020_write_register(ViPipe, 0x41C8, 0xF000);
	ar2020_write_register(ViPipe, 0x41CA, 0x36D8);
	ar2020_write_register(ViPipe, 0x41CC, 0x020D);
	ar2020_write_register(ViPipe, 0x41CE, 0x0205);
	ar2020_write_register(ViPipe, 0x41D0, 0xF000);
	ar2020_write_register(ViPipe, 0x41D2, 0x36D8);
	ar2020_write_register(ViPipe, 0x41D4, 0xF000);
	ar2020_write_register(ViPipe, 0x41D6, 0x83F0);
	ar2020_write_register(ViPipe, 0x41D8, 0x0087);
	ar2020_write_register(ViPipe, 0x41DA, 0xF000);
	ar2020_write_register(ViPipe, 0x41DC, 0x8713);
	ar2020_write_register(ViPipe, 0x41DE, 0x0036);
	ar2020_write_register(ViPipe, 0x41E0, 0xD802);
	ar2020_write_register(ViPipe, 0x41E2, 0x0DE0);
	ar2020_write_register(ViPipe, 0x41E4, 0xE0E0);
	ar2020_write_register(ViPipe, 0x41E6, 0xE0E0);
	ar2020_write_register(ViPipe, 0x41E8, 0x9F13);
	ar2020_write_register(ViPipe, 0x41EA, 0x0041);
	ar2020_write_register(ViPipe, 0x41EC, 0x80F3);
	ar2020_write_register(ViPipe, 0x41EE, 0xF213);
	ar2020_write_register(ViPipe, 0x41F0, 0x00F0);
	ar2020_write_register(ViPipe, 0x41F2, 0x13B8);
	ar2020_write_register(ViPipe, 0x41F4, 0xF04C);
	ar2020_write_register(ViPipe, 0x41F6, 0x9FF0);
	ar2020_write_register(ViPipe, 0x41F8, 0x00B7);
	ar2020_write_register(ViPipe, 0x41FA, 0xF006);
	ar2020_write_register(ViPipe, 0x41FC, 0x0035);
	ar2020_write_register(ViPipe, 0x41FE, 0x10AF);
	ar2020_write_register(ViPipe, 0x4200, 0x3003);
	ar2020_write_register(ViPipe, 0x4202, 0x30C0);
	ar2020_write_register(ViPipe, 0x4204, 0xB2F0);
	ar2020_write_register(ViPipe, 0x4206, 0x01B5);
	ar2020_write_register(ViPipe, 0x4208, 0xF001);
	ar2020_write_register(ViPipe, 0x420A, 0x85F0);
	ar2020_write_register(ViPipe, 0x420C, 0x0292);
	ar2020_write_register(ViPipe, 0x420E, 0xF000);
	ar2020_write_register(ViPipe, 0x4210, 0x9A8B);
	ar2020_write_register(ViPipe, 0x4212, 0xF000);
	ar2020_write_register(ViPipe, 0x4214, 0x9997);
	ar2020_write_register(ViPipe, 0x4216, 0xF007);
	ar2020_write_register(ViPipe, 0x4218, 0xB6F0);
	ar2020_write_register(ViPipe, 0x421A, 0x0020);
	ar2020_write_register(ViPipe, 0x421C, 0x5830);
	ar2020_write_register(ViPipe, 0x421E, 0xC040);
	ar2020_write_register(ViPipe, 0x4220, 0x1282);
	ar2020_write_register(ViPipe, 0x4222, 0xF005);
	ar2020_write_register(ViPipe, 0x4224, 0x9CF0);
	ar2020_write_register(ViPipe, 0x4226, 0x01B2);
	ar2020_write_register(ViPipe, 0x4228, 0xF008);
	ar2020_write_register(ViPipe, 0x422A, 0xB8F0);
	ar2020_write_register(ViPipe, 0x422C, 0x0799);
	ar2020_write_register(ViPipe, 0x422E, 0xF005);
	ar2020_write_register(ViPipe, 0x4230, 0x98F0);
	ar2020_write_register(ViPipe, 0x4232, 0x0296);
	ar2020_write_register(ViPipe, 0x4234, 0xA2F0);
	ar2020_write_register(ViPipe, 0x4236, 0x00A2);
	ar2020_write_register(ViPipe, 0x4238, 0xF008);
	ar2020_write_register(ViPipe, 0x423A, 0x9DF0);
	ar2020_write_register(ViPipe, 0x423C, 0x02A1);
	ar2020_write_register(ViPipe, 0x423E, 0xF01F);
	ar2020_write_register(ViPipe, 0x4240, 0x1009);
	ar2020_write_register(ViPipe, 0x4242, 0x2220);
	ar2020_write_register(ViPipe, 0x4244, 0x0808);
	ar2020_write_register(ViPipe, 0x4246, 0xF000);
	ar2020_write_register(ViPipe, 0x4248, 0x3600);
	ar2020_write_register(ViPipe, 0x424A, 0xF000);
	ar2020_write_register(ViPipe, 0x424C, 0x88F0);
	ar2020_write_register(ViPipe, 0x424E, 0x0788);
	ar2020_write_register(ViPipe, 0x4250, 0x3600);
	ar2020_write_register(ViPipe, 0x4252, 0xF000);
	ar2020_write_register(ViPipe, 0x4254, 0x83F0);
	ar2020_write_register(ViPipe, 0x4256, 0x0290);
	ar2020_write_register(ViPipe, 0x4258, 0xF016);
	ar2020_write_register(ViPipe, 0x425A, 0x8BF0);
	ar2020_write_register(ViPipe, 0x425C, 0x11A3);
	ar2020_write_register(ViPipe, 0x425E, 0xF000);
	ar2020_write_register(ViPipe, 0x4260, 0xA3F0);
	ar2020_write_register(ViPipe, 0x4262, 0x089D);
	ar2020_write_register(ViPipe, 0x4264, 0xF002);
	ar2020_write_register(ViPipe, 0x4266, 0xA1F0);
	ar2020_write_register(ViPipe, 0x4268, 0x20A1);
	ar2020_write_register(ViPipe, 0x426A, 0xF006);
	ar2020_write_register(ViPipe, 0x426C, 0x4300);
	ar2020_write_register(ViPipe, 0x426E, 0xF049);
	ar2020_write_register(ViPipe, 0x4270, 0x4014);
	ar2020_write_register(ViPipe, 0x4272, 0x8B8E);
	ar2020_write_register(ViPipe, 0x4274, 0x9DF0);
	ar2020_write_register(ViPipe, 0x4276, 0x0802);
	ar2020_write_register(ViPipe, 0x4278, 0x02F0);
	ar2020_write_register(ViPipe, 0x427A, 0x00A6);
	ar2020_write_register(ViPipe, 0x427C, 0xF013);
	ar2020_write_register(ViPipe, 0x427E, 0xB283);
	ar2020_write_register(ViPipe, 0x4280, 0x9C36);
	ar2020_write_register(ViPipe, 0x4282, 0x00F0);
	ar2020_write_register(ViPipe, 0x4284, 0x0636);
	ar2020_write_register(ViPipe, 0x4286, 0x009C);
	ar2020_write_register(ViPipe, 0x4288, 0xF008);
	ar2020_write_register(ViPipe, 0x428A, 0x8BF0);
	ar2020_write_register(ViPipe, 0x428C, 0x0083);
	ar2020_write_register(ViPipe, 0x428E, 0xA0F0);
	ar2020_write_register(ViPipe, 0x4290, 0x0630);
	ar2020_write_register(ViPipe, 0x4292, 0x18F0);
	ar2020_write_register(ViPipe, 0x4294, 0x02A3);
	ar2020_write_register(ViPipe, 0x4296, 0xF000);
	ar2020_write_register(ViPipe, 0x4298, 0xA3F0);
	ar2020_write_register(ViPipe, 0x429A, 0x0243);
	ar2020_write_register(ViPipe, 0x429C, 0x00F0);
	ar2020_write_register(ViPipe, 0x429E, 0x049D);
	ar2020_write_register(ViPipe, 0x42A0, 0xF078);
	ar2020_write_register(ViPipe, 0x42A2, 0x3018);
	ar2020_write_register(ViPipe, 0x42A4, 0xF000);
	ar2020_write_register(ViPipe, 0x42A6, 0x9D82);
	ar2020_write_register(ViPipe, 0x42A8, 0xF000);
	ar2020_write_register(ViPipe, 0x42AA, 0x9030);
	ar2020_write_register(ViPipe, 0x42AC, 0xC0F0);
	ar2020_write_register(ViPipe, 0x42AE, 0x1130);
	ar2020_write_register(ViPipe, 0x42B0, 0xC0F0);
	ar2020_write_register(ViPipe, 0x42B2, 0x0082);
	ar2020_write_register(ViPipe, 0x42B4, 0xF001);
	ar2020_write_register(ViPipe, 0x42B6, 0x1009);
	ar2020_write_register(ViPipe, 0x42B8, 0xF02A);
	ar2020_write_register(ViPipe, 0x42BA, 0xA2F0);
	ar2020_write_register(ViPipe, 0x42BC, 0x00A2);
	ar2020_write_register(ViPipe, 0x42BE, 0x3018);
	ar2020_write_register(ViPipe, 0x42C0, 0xF007);
	ar2020_write_register(ViPipe, 0x42C2, 0x9DF0);
	ar2020_write_register(ViPipe, 0x42C4, 0x1C8C);
	ar2020_write_register(ViPipe, 0x42C6, 0xF005);
	ar2020_write_register(ViPipe, 0x42C8, 0x301F);
	ar2020_write_register(ViPipe, 0x42CA, 0x216D);
	ar2020_write_register(ViPipe, 0x42CC, 0x0A51);
	ar2020_write_register(ViPipe, 0x42CE, 0x1FEA);
	ar2020_write_register(ViPipe, 0x42D0, 0x8640);
	ar2020_write_register(ViPipe, 0x42D2, 0xE29F);
	ar2020_write_register(ViPipe, 0x42D4, 0xF009);
	ar2020_write_register(ViPipe, 0x42D6, 0x0005);
	ar2020_write_register(ViPipe, 0x42D8, 0xF000);
	ar2020_write_register(ViPipe, 0x42DA, 0x30C0);
	ar2020_write_register(ViPipe, 0x42DC, 0xF001);
	ar2020_write_register(ViPipe, 0x42DE, 0x83F0);
	ar2020_write_register(ViPipe, 0x42E0, 0x0036);
	ar2020_write_register(ViPipe, 0x42E2, 0x00F0);
	ar2020_write_register(ViPipe, 0x42E4, 0x0087);
	ar2020_write_register(ViPipe, 0x42E6, 0xF007);
	ar2020_write_register(ViPipe, 0x42E8, 0x87F0);
	ar2020_write_register(ViPipe, 0x42EA, 0x0036);
	ar2020_write_register(ViPipe, 0x42EC, 0xC0F0);
	ar2020_write_register(ViPipe, 0x42EE, 0x0000);
	ar2020_write_register(ViPipe, 0x42F0, 0x0DF0);
	ar2020_write_register(ViPipe, 0x42F2, 0x0000);
	ar2020_write_register(ViPipe, 0x42F4, 0x05F0);
	ar2020_write_register(ViPipe, 0x42F6, 0x0030);
	ar2020_write_register(ViPipe, 0x42F8, 0xC0F0);
	ar2020_write_register(ViPipe, 0x42FA, 0x0183);
	ar2020_write_register(ViPipe, 0x42FC, 0xF000);
	ar2020_write_register(ViPipe, 0x42FE, 0x3600);
	ar2020_write_register(ViPipe, 0x4300, 0xF000);
	ar2020_write_register(ViPipe, 0x4302, 0x87F0);
	ar2020_write_register(ViPipe, 0x4304, 0x0787);
	ar2020_write_register(ViPipe, 0x4306, 0xF000);
	ar2020_write_register(ViPipe, 0x4308, 0x36C0);
	ar2020_write_register(ViPipe, 0x430A, 0xF000);
	ar2020_write_register(ViPipe, 0x430C, 0x000F);
	ar2020_write_register(ViPipe, 0x430E, 0xF42A);
	ar2020_write_register(ViPipe, 0x4310, 0x4180);
	ar2020_write_register(ViPipe, 0x4312, 0x1300);
	ar2020_write_register(ViPipe, 0x4314, 0x9FF0);
	ar2020_write_register(ViPipe, 0x4316, 0x00E0);
	ar2020_write_register(ViPipe, 0x4318, 0xE0E0);
	ar2020_write_register(ViPipe, 0x431A, 0xE0E0);
	ar2020_write_register(ViPipe, 0x431C, 0xE0E0);
	ar2020_write_register(ViPipe, 0x431E, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4320, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4322, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4324, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4326, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4328, 0xE0E0);
	ar2020_write_register(ViPipe, 0x432A, 0xE0E0);
	ar2020_write_register(ViPipe, 0x432C, 0xE0E0);
	ar2020_write_register(ViPipe, 0x432E, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4330, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4332, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4334, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4336, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4338, 0xE0E0);
	ar2020_write_register(ViPipe, 0x433A, 0xE0E0);
	ar2020_write_register(ViPipe, 0x433C, 0xE0E0);
	ar2020_write_register(ViPipe, 0x433E, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4340, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4342, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4344, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4346, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4348, 0xE0E0);
	ar2020_write_register(ViPipe, 0x434A, 0xE0E0);
	ar2020_write_register(ViPipe, 0x434C, 0xE0E0);
	ar2020_write_register(ViPipe, 0x434E, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4350, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4352, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4354, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4356, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4358, 0xE0E0);
	ar2020_write_register(ViPipe, 0x435A, 0xE0E0);
	ar2020_write_register(ViPipe, 0x435C, 0xE0E0);
	ar2020_write_register(ViPipe, 0x435E, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4360, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4362, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4364, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4366, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4368, 0xE0E0);
	ar2020_write_register(ViPipe, 0x436A, 0xE0E0);
	ar2020_write_register(ViPipe, 0x436C, 0xE0E0);
	ar2020_write_register(ViPipe, 0x436E, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4370, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4372, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4374, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4376, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4378, 0xE0E0);
	ar2020_write_register(ViPipe, 0x437A, 0xE0E0);
	ar2020_write_register(ViPipe, 0x437C, 0xE0E0);
	ar2020_write_register(ViPipe, 0x437E, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4380, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4382, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4384, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4386, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4388, 0xE0E0);
	ar2020_write_register(ViPipe, 0x438A, 0xE0E0);
	ar2020_write_register(ViPipe, 0x438C, 0xE0E0);
	ar2020_write_register(ViPipe, 0x438E, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4390, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4392, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4394, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4396, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4398, 0xE0E0);
	ar2020_write_register(ViPipe, 0x439A, 0xE0E0);
	ar2020_write_register(ViPipe, 0x439C, 0xE0E0);
	ar2020_write_register(ViPipe, 0x439E, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43A0, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43A2, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43A4, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43A6, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43A8, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43AA, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43AC, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43AE, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43B0, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43B2, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43B4, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43B6, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43B8, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43BA, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43BC, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43BE, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43C0, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43C2, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43C4, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43C6, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43C8, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43CA, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43CC, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43CE, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43D0, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43D2, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43D4, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43D6, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43D8, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43DA, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43DC, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43DE, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43E0, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43E2, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43E4, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43E6, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43E8, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43EA, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43EC, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43EE, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43F0, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43F2, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43F4, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43F6, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43F8, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43FA, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43FC, 0xE0E0);
	ar2020_write_register(ViPipe, 0x43FE, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4400, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4402, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4404, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4406, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4408, 0xE0E0);
	ar2020_write_register(ViPipe, 0x440A, 0xE0E0);
	ar2020_write_register(ViPipe, 0x440C, 0xE0E0);
	ar2020_write_register(ViPipe, 0x440E, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4410, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4412, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4414, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4416, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4418, 0xE0E0);
	ar2020_write_register(ViPipe, 0x441A, 0xE0E0);
	ar2020_write_register(ViPipe, 0x441C, 0xE0E0);
	ar2020_write_register(ViPipe, 0x441E, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4420, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4422, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4424, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4426, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4428, 0xE0E0);
	ar2020_write_register(ViPipe, 0x442A, 0xE0E0);
	ar2020_write_register(ViPipe, 0x442C, 0xE0E0);
	ar2020_write_register(ViPipe, 0x442E, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4430, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4432, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4434, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4436, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4438, 0xE0E0);
	ar2020_write_register(ViPipe, 0x443A, 0xE0E0);
	ar2020_write_register(ViPipe, 0x443C, 0xE0E0);
	ar2020_write_register(ViPipe, 0x443E, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4440, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4442, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4444, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4446, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4448, 0xE0E0);
	ar2020_write_register(ViPipe, 0x444A, 0xE0E0);
	ar2020_write_register(ViPipe, 0x444C, 0xE0E0);
	ar2020_write_register(ViPipe, 0x444E, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4450, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4452, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4454, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4456, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4458, 0xE0E0);
	ar2020_write_register(ViPipe, 0x445A, 0xE0E0);
	ar2020_write_register(ViPipe, 0x445C, 0xE0E0);
	ar2020_write_register(ViPipe, 0x445E, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4460, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4462, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4464, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4466, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4468, 0xE0E0);
	ar2020_write_register(ViPipe, 0x446A, 0xE0E0);
	ar2020_write_register(ViPipe, 0x446C, 0xE0E0);
	ar2020_write_register(ViPipe, 0x446E, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4470, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4472, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4474, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4476, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4478, 0xE0E0);
	ar2020_write_register(ViPipe, 0x447A, 0xE0E0);
	ar2020_write_register(ViPipe, 0x447C, 0xE0E0);
	ar2020_write_register(ViPipe, 0x447E, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4480, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4482, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4484, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4486, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4488, 0xE0E0);
	ar2020_write_register(ViPipe, 0x448A, 0xE0E0);
	ar2020_write_register(ViPipe, 0x448C, 0xE0E0);
	ar2020_write_register(ViPipe, 0x448E, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4490, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4492, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4494, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4496, 0xE0E0);
	ar2020_write_register(ViPipe, 0x4498, 0xE0E0);
	ar2020_write_register(ViPipe, 0x449A, 0xE0E0);
	ar2020_write_register(ViPipe, 0x449C, 0xE0E0);
	ar2020_write_register(ViPipe, 0x449E, 0xE0E0);
	ar2020_write_register(ViPipe, 0x44A0, 0xE0E0);
	ar2020_write_register(ViPipe, 0x44A2, 0xE0E0);
	ar2020_write_register(ViPipe, 0x44A4, 0xE0E0);
	ar2020_write_register(ViPipe, 0x44A6, 0xE0E0);
	ar2020_write_register(ViPipe, 0x44A8, 0xE0E0);
	ar2020_write_register(ViPipe, 0x44AA, 0xE0E0);
	ar2020_write_register(ViPipe, 0x44AC, 0xE0E0);
	ar2020_write_register(ViPipe, 0x44AE, 0xE0E0);
	ar2020_write_register(ViPipe, 0x44B0, 0xE0E0);
	ar2020_write_register(ViPipe, 0x44B2, 0xE0E0);
	ar2020_write_register(ViPipe, 0x44B4, 0xE0E0);
	ar2020_write_register(ViPipe, 0x5500, 0x0000);
	ar2020_write_register(ViPipe, 0x5502, 0x0002);
	ar2020_write_register(ViPipe, 0x5504, 0x0006);
	ar2020_write_register(ViPipe, 0x5506, 0x0009);
	ar2020_write_register(ViPipe, 0x5508, 0x000F);
	ar2020_write_register(ViPipe, 0x550A, 0x0010);
	ar2020_write_register(ViPipe, 0x550C, 0x0011);
	ar2020_write_register(ViPipe, 0x550E, 0x0012);
	ar2020_write_register(ViPipe, 0x5510, 0x0019);
	ar2020_write_register(ViPipe, 0x5512, 0x0020);
	ar2020_write_register(ViPipe, 0x5514, 0x0021);
	ar2020_write_register(ViPipe, 0x5516, 0x0023);
	ar2020_write_register(ViPipe, 0x5518, 0x0026);
	ar2020_write_register(ViPipe, 0x551A, 0x002B);
	ar2020_write_register(ViPipe, 0x551C, 0x002F);
	ar2020_write_register(ViPipe, 0x551E, 0x0030);
	ar2020_write_register(ViPipe, 0x5400, 0x0100);
	ar2020_write_register(ViPipe, 0x5402, 0x2106);
	ar2020_write_register(ViPipe, 0x5404, 0x1101);
	ar2020_write_register(ViPipe, 0x5406, 0x3106);
	ar2020_write_register(ViPipe, 0x5408, 0x7100);
	ar2020_write_register(ViPipe, 0x540A, 0x8107);
	ar2020_write_register(ViPipe, 0x540C, 0xB101);
	ar2020_write_register(ViPipe, 0x540E, 0xD101);
	ar2020_write_register(ViPipe, 0x5410, 0xF12E);
	ar2020_write_register(ViPipe, 0x5412, 0xF112);
	ar2020_write_register(ViPipe, 0x5414, 0xF184);
	ar2020_write_register(ViPipe, 0x5416, 0xF224);
	ar2020_write_register(ViPipe, 0x5418, 0xF306);
	ar2020_write_register(ViPipe, 0x541A, 0xF446);
	ar2020_write_register(ViPipe, 0x541C, 0xF609);
	ar2020_write_register(ViPipe, 0x541E, 0xF887);
	ar2020_write_register(ViPipe, 0x5420, 0xFC0B);
	ar2020_write_register(ViPipe, 0x5422, 0xFC0B);
	ar2020_write_register(ViPipe, 0x5424, 0xFFFA);
	ar2020_write_register(ViPipe, 0x5426, 0x5557);
	ar2020_write_register(ViPipe, 0x5428, 0x0005);
	ar2020_write_register(ViPipe, 0x542A, 0xA550);
	ar2020_write_register(ViPipe, 0x542C, 0xAAAA);
	ar2020_write_register(ViPipe, 0x542E, 0x000A);
	ar2020_write_register(ViPipe, 0x5460, 0x2269);
	ar2020_write_register(ViPipe, 0x5462, 0x0B87);
	ar2020_write_register(ViPipe, 0x5464, 0x0B87);
	ar2020_write_register(ViPipe, 0x5466, 0x0983);
	ar2020_write_register(ViPipe, 0x5498, 0x225E);
	ar2020_write_register(ViPipe, 0x549A, 0xBCAA);
	ar2020_write_register(ViPipe, 0x549C, 0xBCAA);
	ar2020_write_register(ViPipe, 0x549E, 0xBDAA);
	ar2020_write_register(ViPipe, 0x3060, 0xFF01);
	ar2020_write_register(ViPipe, 0x44BA, 0x0050);
	ar2020_write_register(ViPipe, 0x44BC, 0xBCAA);
	ar2020_write_register(ViPipe, 0x44C0, 0x4070);
	ar2020_write_register(ViPipe, 0x44C4, 0x04D0);
	ar2020_write_register(ViPipe, 0x44C6, 0x17E2);
	ar2020_write_register(ViPipe, 0x44C8, 0xEA43);
	ar2020_write_register(ViPipe, 0x44CA, 0x000E);
	ar2020_write_register(ViPipe, 0x44CC, 0x7777);
	ar2020_write_register(ViPipe, 0x44CE, 0x8BA4);
	ar2020_write_register(ViPipe, 0x44D0, 0x1735);
	ar2020_write_register(ViPipe, 0x44D2, 0x0B87);
	ar2020_write_register(ViPipe, 0x44D4, 0x0000);
	ar2020_write_register(ViPipe, 0x44D6, 0xF206);
	ar2020_write_register(ViPipe, 0x44D8, 0xAAFA);
	ar2020_write_register(ViPipe, 0x44DA, 0xE001);
	ar2020_write_register(ViPipe, 0x44DE, 0x9BBC);
	ar2020_write_register(ViPipe, 0x44E0, 0x283C);
	ar2020_write_register(ViPipe, 0x44E2, 0x2821);
	ar2020_write_register(ViPipe, 0x44E4, 0x8000);
	ar2020_write_register(ViPipe, 0x44E6, 0x503F);
	ar2020_write_register(ViPipe, 0x32A4, 0x0000);
	ar2020_write_register(ViPipe, 0x328E, 0x0004);
	ar2020_write_register(ViPipe, 0x333C, 0x0001);
	ar2020_write_register(ViPipe, 0x301A, 0x0000);
	ar2020_write_register(ViPipe, 0x3600, 0x94DF);
	ar2020_write_register(ViPipe, 0x3616, 0x0000);
	ar2020_write_register(ViPipe, 0x3700, 0x0001);
	ar2020_write_register(ViPipe, 0x3980, 0x0003);
	ar2020_write_register(ViPipe, 0x36C0, 0x0001);
	ar2020_write_register(ViPipe, 0x36DE, 0x002A);
	ar2020_write_register(ViPipe, 0x301A, 0x0008);
	ar2020_write_register(ViPipe, 0x3060, 0x0000);
	ar2020_write_register(ViPipe, 0x3982, 0xAC70);
	ar2020_write_register(ViPipe, 0x3984, 0xFA98);
	ar2020_write_register(ViPipe, 0x3986, 0xFC3F);
	ar2020_write_register(ViPipe, 0x3988, 0xAC70);
	ar2020_write_register(ViPipe, 0x398A, 0xFA98);
	ar2020_write_register(ViPipe, 0x398C, 0xFC3F);
	ar2020_write_register(ViPipe, 0x3980, 0x0003);
	ar2020_write_register(ViPipe, 0x3060, 0xFF01);
	ar2020_write_register(ViPipe, 0x3340, 0x0C60);
	ar2020_write_register(ViPipe, 0x3340, 0x1C60);

	ar2020_default_reg_init(ViPipe);

	ar2020_write_register(ViPipe, 0x0100, 0x0100);

	ar2020_write_register(ViPipe, 0x44D6, 0xB206);
	ar2020_write_register(ViPipe, 0x3062, 0x000C);
	ar2020_write_register(ViPipe, 0x0202, 0x0F1D);

	printf("ViPipe:%d,===AR2020 3840P 60fps 10bit LINE Init OK!===\n", ViPipe);
}


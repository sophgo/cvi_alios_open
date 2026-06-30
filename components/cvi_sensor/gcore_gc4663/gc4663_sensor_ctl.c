#include <unistd.h>

#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sensor_i2c.h"

#include "gc4663_cmos_ex.h"

#define GC4663_CHIP_ID_ADDR_H	0x03f0
#define GC4663_CHIP_ID_ADDR_L	0x03f1
#define GC4663_CHIP_ID		0x4653

static void gc4663_linear_1440p30_init(VI_PIPE ViPipe);
static void gc4663_hdr_1440p30_init(VI_PIPE ViPipe);

CVI_U8 gc4663_i2c_addr = 0x29;
const CVI_U32 gc4663_addr_byte = 2;
const CVI_U32 gc4663_data_byte = 1;

int gc4663_i2c_init(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc4663_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_init(i2c_id);
}

int gc4663_i2c_exit(VI_PIPE ViPipe)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc4663_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_exit(i2c_id);

}

int gc4663_read_register(VI_PIPE ViPipe, int addr)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc4663_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_read(i2c_id, gc4663_i2c_addr, (CVI_U32)addr, gc4663_addr_byte, gc4663_data_byte);
}

int gc4663_write_register(VI_PIPE ViPipe, int addr, int data)
{
	CVI_U8 i2c_id = (CVI_U8)g_aunGc4663_BusInfo[ViPipe].s8I2cDev;

	return sensor_i2c_write(i2c_id, gc4663_i2c_addr, (CVI_U32)addr, gc4663_addr_byte,
		(CVI_U32)data, gc4663_data_byte);
}

static void delay_ms(int ms)
{
	udelay(ms * 1000);
}

void gc4663_standby(VI_PIPE ViPipe)
{
	/* TODO: GC4663 standby */
	gc4663_write_register(ViPipe, 0x0100, 0x00);
	gc4663_write_register(ViPipe, 0x031c, 0xc7);
	gc4663_write_register(ViPipe, 0x0317, 0x01);

	printf("gc4663_standby\n");
}

void gc4663_restart(VI_PIPE ViPipe)
{
	/* TODO: GC4663 restart */
	gc4663_write_register(ViPipe, 0x0317, 0x00);
	gc4663_write_register(ViPipe, 0x031c, 0xc6);
	gc4663_write_register(ViPipe, 0x0100, 0x09);

	printf("gc4663_restart\n");
}

void gc4663_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastGc4663[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		gc4663_write_register(ViPipe,
				g_pastGc4663[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastGc4663[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

int gc4663_probe(VI_PIPE ViPipe)
{
	int nVal;
	int nVal2;

	usleep(50);
	if (gc4663_i2c_init(ViPipe) != CVI_SUCCESS)
		return CVI_FAILURE;

	nVal  = gc4663_read_register(ViPipe, GC4663_CHIP_ID_ADDR_H);
	nVal2 = gc4663_read_register(ViPipe, GC4663_CHIP_ID_ADDR_L);
	if (nVal < 0 || nVal2 < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
		return nVal;
	}

	if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != GC4663_CHIP_ID) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
		return CVI_FAILURE;
	}

	return CVI_SUCCESS;
}

void gc4663_init(VI_PIPE ViPipe)
{
	WDR_MODE_E enWDRMode = g_pastGc4663[ViPipe]->enWDRMode;
	CVI_U8 u8ImgMode = g_pastGc4663[ViPipe]->u8ImgMode;

	gc4663_i2c_init(ViPipe);

	if (enWDRMode == WDR_MODE_2To1_LINE) {
		if (u8ImgMode == GC4663_MODE_2560X1440P30_WDR)
			gc4663_hdr_1440p30_init(ViPipe);
		else
			CVI_TRACE_SNS(CVI_DBG_ERR, "Unsupport Sensor Mode!\n");
	} else {
		gc4663_linear_1440p30_init(ViPipe);
	}

	g_pastGc4663[ViPipe]->bInit = CVI_TRUE;
}

void gc4663_exit(VI_PIPE ViPipe)
{
	gc4663_i2c_exit(ViPipe);
}

static void gc4663_linear_1440p30_init(VI_PIPE ViPipe)
{
	gc4663_write_register(ViPipe, 0x03fe, 0xf0);
	gc4663_write_register(ViPipe, 0x03fe, 0x00);
	gc4663_write_register(ViPipe, 0x0317, 0x00);
	gc4663_write_register(ViPipe, 0x0320, 0x77);
	gc4663_write_register(ViPipe, 0x0324, 0xc4);
	gc4663_write_register(ViPipe, 0x0326, 0x42);
	gc4663_write_register(ViPipe, 0x0327, 0x03);
	gc4663_write_register(ViPipe, 0x0321, 0x10);
	gc4663_write_register(ViPipe, 0x0314, 0x50);
	gc4663_write_register(ViPipe, 0x0334, 0x40);
	gc4663_write_register(ViPipe, 0x0335, 0xd1);
	gc4663_write_register(ViPipe, 0x0336, 0x70);
	gc4663_write_register(ViPipe, 0x0337, 0x82);
	gc4663_write_register(ViPipe, 0x0315, 0x33);
	gc4663_write_register(ViPipe, 0x031c, 0xce);
	gc4663_write_register(ViPipe, 0x0287, 0x18);
	gc4663_write_register(ViPipe, 0x0084, 0x00);
	gc4663_write_register(ViPipe, 0x0087, 0x50);
	gc4663_write_register(ViPipe, 0x029d, 0x08);
	gc4663_write_register(ViPipe, 0x0290, 0x00);
	// gc4663_write_register(ViPipe, 0x0340, 0x06);//VTS 1600
	// gc4663_write_register(ViPipe, 0x0341, 0x40);
	gc4663_write_register(ViPipe, 0x0340, 0x0c);
	gc4663_write_register(ViPipe, 0x0341, 0x80); //VTS 3200
	gc4663_write_register(ViPipe, 0x0345, 0x06);
	gc4663_write_register(ViPipe, 0x034b, 0xb0);
	gc4663_write_register(ViPipe, 0x0352, 0x08);
	gc4663_write_register(ViPipe, 0x0354, 0x08);
	gc4663_write_register(ViPipe, 0x02d1, 0xc0);
	gc4663_write_register(ViPipe, 0x023c, 0x04);
	gc4663_write_register(ViPipe, 0x0238, 0xb4);
	gc4663_write_register(ViPipe, 0x0223, 0xfb);
	gc4663_write_register(ViPipe, 0x0232, 0xc4);
	gc4663_write_register(ViPipe, 0x0279, 0x53);
	gc4663_write_register(ViPipe, 0x02d3, 0x01);
	gc4663_write_register(ViPipe, 0x0243, 0x06);
	gc4663_write_register(ViPipe, 0x02ce, 0xbf);
	gc4663_write_register(ViPipe, 0x02ee, 0x30);
	gc4663_write_register(ViPipe, 0x026f, 0x70);
	gc4663_write_register(ViPipe, 0x0257, 0x09);
	gc4663_write_register(ViPipe, 0x0211, 0x02);
	gc4663_write_register(ViPipe, 0x0219, 0x09);
	gc4663_write_register(ViPipe, 0x023f, 0x2d);
	gc4663_write_register(ViPipe, 0x0518, 0x00);
	gc4663_write_register(ViPipe, 0x0519, 0x14);
	gc4663_write_register(ViPipe, 0x0515, 0x18);
	gc4663_write_register(ViPipe, 0x02d9, 0x50);
	gc4663_write_register(ViPipe, 0x02da, 0x02);
	gc4663_write_register(ViPipe, 0x02db, 0xe8);
	gc4663_write_register(ViPipe, 0x02e6, 0x20);
	gc4663_write_register(ViPipe, 0x021b, 0x10);
	gc4663_write_register(ViPipe, 0x0252, 0x22);
	gc4663_write_register(ViPipe, 0x024e, 0x22);
	gc4663_write_register(ViPipe, 0x02c4, 0x01);
	gc4663_write_register(ViPipe, 0x021d, 0x17);
	gc4663_write_register(ViPipe, 0x024a, 0x01);
	gc4663_write_register(ViPipe, 0x02ca, 0x02);
	gc4663_write_register(ViPipe, 0x0262, 0x10);
	gc4663_write_register(ViPipe, 0x029a, 0x20);
	gc4663_write_register(ViPipe, 0x021c, 0x0e);
	gc4663_write_register(ViPipe, 0x0298, 0x03);
	gc4663_write_register(ViPipe, 0x029c, 0x00);
	gc4663_write_register(ViPipe, 0x027e, 0x14);
	gc4663_write_register(ViPipe, 0x02c2, 0x10);
	gc4663_write_register(ViPipe, 0x0540, 0x20);
	gc4663_write_register(ViPipe, 0x0546, 0x01);
	gc4663_write_register(ViPipe, 0x0548, 0x01);
	gc4663_write_register(ViPipe, 0x0544, 0x01);
	gc4663_write_register(ViPipe, 0x0242, 0x36);
	gc4663_write_register(ViPipe, 0x02c0, 0x36);
	gc4663_write_register(ViPipe, 0x02c3, 0x4d);
	gc4663_write_register(ViPipe, 0x02e4, 0x10);
	gc4663_write_register(ViPipe, 0x022e, 0x00);
	gc4663_write_register(ViPipe, 0x027b, 0x3f);
	gc4663_write_register(ViPipe, 0x0269, 0x0f);
	gc4663_write_register(ViPipe, 0x02d2, 0x40);
	gc4663_write_register(ViPipe, 0x027c, 0x08);
	gc4663_write_register(ViPipe, 0x023a, 0x2e);
	gc4663_write_register(ViPipe, 0x0245, 0xce);
	gc4663_write_register(ViPipe, 0x0530, 0x3f);
	gc4663_write_register(ViPipe, 0x0531, 0x02);
	gc4663_write_register(ViPipe, 0x0228, 0x50);
	gc4663_write_register(ViPipe, 0x02ab, 0x00);
	gc4663_write_register(ViPipe, 0x0250, 0x00);
	gc4663_write_register(ViPipe, 0x0221, 0x50);
	gc4663_write_register(ViPipe, 0x02ac, 0x00);
	gc4663_write_register(ViPipe, 0x02a5, 0x02);
	gc4663_write_register(ViPipe, 0x0260, 0x0b);
	gc4663_write_register(ViPipe, 0x0216, 0x04);
	gc4663_write_register(ViPipe, 0x0299, 0x1C);
	gc4663_write_register(ViPipe, 0x021a, 0x98);
	gc4663_write_register(ViPipe, 0x0266, 0xd0);
	gc4663_write_register(ViPipe, 0x0020, 0x01);
	gc4663_write_register(ViPipe, 0x0021, 0x05);
	gc4663_write_register(ViPipe, 0x0022, 0xc0);
	gc4663_write_register(ViPipe, 0x0023, 0x08);
	gc4663_write_register(ViPipe, 0x0098, 0x10);
	gc4663_write_register(ViPipe, 0x009a, 0xb0);
	gc4663_write_register(ViPipe, 0x02bb, 0x0d);
	gc4663_write_register(ViPipe, 0x02a3, 0x02);
	gc4663_write_register(ViPipe, 0x02a4, 0x02);
	gc4663_write_register(ViPipe, 0x021e, 0x02);
	gc4663_write_register(ViPipe, 0x024f, 0x08);
	gc4663_write_register(ViPipe, 0x028c, 0x08);
	gc4663_write_register(ViPipe, 0x0532, 0x3f);
	gc4663_write_register(ViPipe, 0x0533, 0x02);
	gc4663_write_register(ViPipe, 0x0277, 0x70);
	gc4663_write_register(ViPipe, 0x0276, 0xc0);
	gc4663_write_register(ViPipe, 0x0239, 0xc0);
	gc4663_write_register(ViPipe, 0x0200, 0x00);
	gc4663_write_register(ViPipe, 0x0201, 0x5f);
	gc4663_write_register(ViPipe, 0x0202, 0x05);
	gc4663_write_register(ViPipe, 0x0203, 0xf0);
	gc4663_write_register(ViPipe, 0x0205, 0xc0);
	gc4663_write_register(ViPipe, 0x02b0, 0x68);
	gc4663_write_register(ViPipe, 0x000f, 0x00);
	gc4663_write_register(ViPipe, 0x0006, 0xe0);
	gc4663_write_register(ViPipe, 0x0002, 0xa9);
	gc4663_write_register(ViPipe, 0x0004, 0x01);
	gc4663_write_register(ViPipe, 0x000f, 0x00);
	gc4663_write_register(ViPipe, 0x0060, 0x40);
	gc4663_write_register(ViPipe, 0x0218, 0x02);
	gc4663_write_register(ViPipe, 0x0342, 0x05);
	gc4663_write_register(ViPipe, 0x0343, 0x5f);
	gc4663_write_register(ViPipe, 0x03fe, 0x10);
	gc4663_write_register(ViPipe, 0x03fe, 0x00);
	gc4663_write_register(ViPipe, 0x0106, 0x78);
	gc4663_write_register(ViPipe, 0x0107, 0x09);
	gc4663_write_register(ViPipe, 0x0108, 0x0c);
	gc4663_write_register(ViPipe, 0x0114, 0x01);
	gc4663_write_register(ViPipe, 0x0115, 0x12);
	gc4663_write_register(ViPipe, 0x0180, 0x4f);
	gc4663_write_register(ViPipe, 0x0181, 0x30);
	gc4663_write_register(ViPipe, 0x0182, 0x05);
	gc4663_write_register(ViPipe, 0x0185, 0x01);
	gc4663_write_register(ViPipe, 0x03fe, 0x10);
	gc4663_write_register(ViPipe, 0x03fe, 0x00);
	gc4663_write_register(ViPipe, 0x0100, 0x09);
	//otp
	gc4663_write_register(ViPipe, 0x0080, 0x02);
	gc4663_write_register(ViPipe, 0x0097, 0x0a);
	gc4663_write_register(ViPipe, 0x0098, 0x10);
	gc4663_write_register(ViPipe, 0x0099, 0x05);
	gc4663_write_register(ViPipe, 0x009a, 0xb0);
	gc4663_write_register(ViPipe, 0x0317, 0x08);
	gc4663_write_register(ViPipe, 0x0a67, 0x80);
	gc4663_write_register(ViPipe, 0x0a70, 0x03);
	gc4663_write_register(ViPipe, 0x0a82, 0x00);
	gc4663_write_register(ViPipe, 0x0a83, 0x10);
	gc4663_write_register(ViPipe, 0x0a80, 0x2b);
	gc4663_write_register(ViPipe, 0x05be, 0x00);
	gc4663_write_register(ViPipe, 0x05a9, 0x01);
	gc4663_write_register(ViPipe, 0x0313, 0x80);
	gc4663_write_register(ViPipe, 0x05be, 0x01);
	gc4663_write_register(ViPipe, 0x0317, 0x00);
	gc4663_write_register(ViPipe, 0x0a67, 0x00);

	gc4663_default_reg_init(ViPipe);
	delay_ms(10);

	printf("ViPipe:%d,===GC4663 1440P 30fps 10bit LINEAR Init OK!===\n", ViPipe);
}

static void gc4663_hdr_1440p30_init(VI_PIPE ViPipe)
{
	gc4663_write_register(ViPipe, 0x03fe, 0xf0);
	gc4663_write_register(ViPipe, 0x03fe, 0x00);
	gc4663_write_register(ViPipe, 0x0317, 0x00);
	gc4663_write_register(ViPipe, 0x0320, 0x77);
	gc4663_write_register(ViPipe, 0x0324, 0xc4);
	gc4663_write_register(ViPipe, 0x0326, 0x42);
	gc4663_write_register(ViPipe, 0x0327, 0x03);
	gc4663_write_register(ViPipe, 0x0321, 0x10);
	gc4663_write_register(ViPipe, 0x0314, 0x50);
	gc4663_write_register(ViPipe, 0x0334, 0x40);
	gc4663_write_register(ViPipe, 0x0335, 0xd1);
	gc4663_write_register(ViPipe, 0x0336, 0x70);
	gc4663_write_register(ViPipe, 0x0337, 0x82);
	gc4663_write_register(ViPipe, 0x0315, 0x33);
	gc4663_write_register(ViPipe, 0x031c, 0xce);
	gc4663_write_register(ViPipe, 0x0287, 0x18);
	gc4663_write_register(ViPipe, 0x0084, 0x00);
	gc4663_write_register(ViPipe, 0x0087, 0x50);
	gc4663_write_register(ViPipe, 0x029d, 0x08);
	gc4663_write_register(ViPipe, 0x0290, 0x00);
	gc4663_write_register(ViPipe, 0x0340, 0x06);
	gc4663_write_register(ViPipe, 0x0341, 0x40);
	gc4663_write_register(ViPipe, 0x0345, 0x06);
	gc4663_write_register(ViPipe, 0x034b, 0xb0);
	gc4663_write_register(ViPipe, 0x0352, 0x08);
	gc4663_write_register(ViPipe, 0x0354, 0x08);
	gc4663_write_register(ViPipe, 0x02d1, 0xc0);
	gc4663_write_register(ViPipe, 0x023c, 0x04);
	gc4663_write_register(ViPipe, 0x0238, 0xb4);
	gc4663_write_register(ViPipe, 0x0223, 0xfb);
	gc4663_write_register(ViPipe, 0x0232, 0xc4);
	gc4663_write_register(ViPipe, 0x0279, 0x53);
	gc4663_write_register(ViPipe, 0x02d3, 0x01);
	gc4663_write_register(ViPipe, 0x0243, 0x06);
	gc4663_write_register(ViPipe, 0x02ce, 0xbf);
	gc4663_write_register(ViPipe, 0x02ee, 0x30);
	gc4663_write_register(ViPipe, 0x026f, 0x70);
	gc4663_write_register(ViPipe, 0x0257, 0x09);
	gc4663_write_register(ViPipe, 0x0211, 0x02);
	gc4663_write_register(ViPipe, 0x0219, 0x09);
	gc4663_write_register(ViPipe, 0x023f, 0x2d);
	gc4663_write_register(ViPipe, 0x0518, 0x00);
	gc4663_write_register(ViPipe, 0x0519, 0x14);
	gc4663_write_register(ViPipe, 0x0515, 0x18);
	gc4663_write_register(ViPipe, 0x02d9, 0x50);
	gc4663_write_register(ViPipe, 0x02da, 0x02);
	gc4663_write_register(ViPipe, 0x02db, 0xe8);
	gc4663_write_register(ViPipe, 0x02e6, 0x20);
	gc4663_write_register(ViPipe, 0x021b, 0x10);
	gc4663_write_register(ViPipe, 0x0252, 0x22);
	gc4663_write_register(ViPipe, 0x024e, 0x22);
	gc4663_write_register(ViPipe, 0x02c4, 0x01);
	gc4663_write_register(ViPipe, 0x021d, 0x17);
	gc4663_write_register(ViPipe, 0x024a, 0x01);
	gc4663_write_register(ViPipe, 0x02ca, 0x02);
	gc4663_write_register(ViPipe, 0x0262, 0x10);
	gc4663_write_register(ViPipe, 0x029a, 0x20);
	gc4663_write_register(ViPipe, 0x021c, 0x0e);
	gc4663_write_register(ViPipe, 0x0298, 0x03);
	gc4663_write_register(ViPipe, 0x029c, 0x00);
	gc4663_write_register(ViPipe, 0x027e, 0x14);
	gc4663_write_register(ViPipe, 0x02c2, 0x10);
	gc4663_write_register(ViPipe, 0x0540, 0x20);
	gc4663_write_register(ViPipe, 0x0546, 0x01);
	gc4663_write_register(ViPipe, 0x0548, 0x01);
	gc4663_write_register(ViPipe, 0x0544, 0x01);
	gc4663_write_register(ViPipe, 0x0242, 0x36);
	gc4663_write_register(ViPipe, 0x02c0, 0x36);
	gc4663_write_register(ViPipe, 0x02c3, 0x4d);
	gc4663_write_register(ViPipe, 0x02e4, 0x10);
	gc4663_write_register(ViPipe, 0x022e, 0x00);
	gc4663_write_register(ViPipe, 0x027b, 0x3f);
	gc4663_write_register(ViPipe, 0x0269, 0x0f);
	gc4663_write_register(ViPipe, 0x02d2, 0x40);
	gc4663_write_register(ViPipe, 0x027c, 0x08);
	gc4663_write_register(ViPipe, 0x023a, 0x2e);
	gc4663_write_register(ViPipe, 0x0245, 0xce);
	gc4663_write_register(ViPipe, 0x0530, 0x3f);
	gc4663_write_register(ViPipe, 0x0531, 0x02);
	gc4663_write_register(ViPipe, 0x0228, 0x50);
	gc4663_write_register(ViPipe, 0x02ab, 0x00);
	gc4663_write_register(ViPipe, 0x0250, 0x00);
	gc4663_write_register(ViPipe, 0x0221, 0x50);
	gc4663_write_register(ViPipe, 0x02ac, 0x00);
	gc4663_write_register(ViPipe, 0x02a5, 0x02);
	gc4663_write_register(ViPipe, 0x0260, 0x0b);
	gc4663_write_register(ViPipe, 0x0216, 0x04);
	gc4663_write_register(ViPipe, 0x0299, 0x1C);
	gc4663_write_register(ViPipe, 0x021a, 0x98);
	gc4663_write_register(ViPipe, 0x0266, 0xd0);
	gc4663_write_register(ViPipe, 0x0020, 0x01);
	gc4663_write_register(ViPipe, 0x0021, 0x05);
	gc4663_write_register(ViPipe, 0x0022, 0xc0);
	gc4663_write_register(ViPipe, 0x0023, 0x08);
	gc4663_write_register(ViPipe, 0x0098, 0x10);
	gc4663_write_register(ViPipe, 0x009a, 0xb0);
	gc4663_write_register(ViPipe, 0x02bb, 0x0d);
	gc4663_write_register(ViPipe, 0x02a3, 0x02);
	gc4663_write_register(ViPipe, 0x02a4, 0x02);
	gc4663_write_register(ViPipe, 0x021e, 0x02);
	gc4663_write_register(ViPipe, 0x024f, 0x08);
	gc4663_write_register(ViPipe, 0x028c, 0x08);
	gc4663_write_register(ViPipe, 0x0532, 0x3f);
	gc4663_write_register(ViPipe, 0x0533, 0x02);
	gc4663_write_register(ViPipe, 0x0277, 0x70);
	gc4663_write_register(ViPipe, 0x0276, 0xc0);
	gc4663_write_register(ViPipe, 0x0239, 0xc0);
	gc4663_write_register(ViPipe, 0x0200, 0x00);
	gc4663_write_register(ViPipe, 0x0201, 0x50);
	gc4663_write_register(ViPipe, 0x0202, 0x05);
	gc4663_write_register(ViPipe, 0x0203, 0x00);
	gc4663_write_register(ViPipe, 0x0205, 0xc0);
	gc4663_write_register(ViPipe, 0x02b0, 0x68);
	gc4663_write_register(ViPipe, 0x000f, 0x00);
	gc4663_write_register(ViPipe, 0x0006, 0xe0);
	gc4663_write_register(ViPipe, 0x0002, 0xa9);
	gc4663_write_register(ViPipe, 0x0004, 0x01);
	gc4663_write_register(ViPipe, 0x0060, 0x40);
	gc4663_write_register(ViPipe, 0x0218, 0x12);
	gc4663_write_register(ViPipe, 0x0342, 0x05);
	gc4663_write_register(ViPipe, 0x0343, 0x5f);
	gc4663_write_register(ViPipe, 0x03fe, 0x10);
	gc4663_write_register(ViPipe, 0x03fe, 0x00);
	gc4663_write_register(ViPipe, 0x0106, 0x78);
	gc4663_write_register(ViPipe, 0x0107, 0x89);
	gc4663_write_register(ViPipe, 0x0108, 0x0c);
	gc4663_write_register(ViPipe, 0x0114, 0x01);
	gc4663_write_register(ViPipe, 0x0115, 0x12);
	gc4663_write_register(ViPipe, 0x0180, 0x4f);
	gc4663_write_register(ViPipe, 0x0181, 0x30);
	gc4663_write_register(ViPipe, 0x0182, 0x05);
	gc4663_write_register(ViPipe, 0x0185, 0x01);
	gc4663_write_register(ViPipe, 0x03fe, 0x10);
	gc4663_write_register(ViPipe, 0x03fe, 0x00);
	gc4663_write_register(ViPipe, 0x0100, 0x09);

	//otp
	gc4663_write_register(ViPipe, 0x0080, 0x02);
	gc4663_write_register(ViPipe, 0x0097, 0x0a);
	gc4663_write_register(ViPipe, 0x0098, 0x10);
	gc4663_write_register(ViPipe, 0x0099, 0x05);
	gc4663_write_register(ViPipe, 0x009a, 0xb0);
	gc4663_write_register(ViPipe, 0x0317, 0x08);
	gc4663_write_register(ViPipe, 0x0a67, 0x80);
	gc4663_write_register(ViPipe, 0x0a70, 0x03);
	gc4663_write_register(ViPipe, 0x0a82, 0x00);
	gc4663_write_register(ViPipe, 0x0a83, 0x10);
	gc4663_write_register(ViPipe, 0x0a80, 0x2b);
	gc4663_write_register(ViPipe, 0x05be, 0x00);
	gc4663_write_register(ViPipe, 0x05a9, 0x01);
	gc4663_write_register(ViPipe, 0x0313, 0x80);
	gc4663_write_register(ViPipe, 0x05be, 0x01);
	gc4663_write_register(ViPipe, 0x0317, 0x00);
	gc4663_write_register(ViPipe, 0x0a67, 0x00);

	gc4663_default_reg_init(ViPipe);
	delay_ms(10);

	printf("ViPipe:%d,===GC4663 1440P 30fps 10bit HDR Init OK!===\n", ViPipe);
}

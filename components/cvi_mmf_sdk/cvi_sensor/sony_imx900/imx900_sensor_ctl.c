#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "imx900_cmos_ex.h"
#include "sensor_i2c.h"

static void imx900_color_3M70_init(VI_PIPE ViPipe);
static void imx900_mono_3M70_init(VI_PIPE ViPipe);

const CVI_U32 imx900_addr_byte = 2;
const CVI_U32 imx900_data_byte = 1;
int imx900_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunImx900_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx900_AddrInfo[ViPipe].s8I2cAddr);
}

int imx900_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunImx900_BusInfo[ViPipe].s8I2cDev);
}

int imx900_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunImx900_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx900_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							imx900_addr_byte, imx900_data_byte);
}

int imx900_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunImx900_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx900_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							imx900_addr_byte, (CVI_U32)data, imx900_data_byte);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

void imx900_standby(VI_PIPE ViPipe)
{
	imx900_write_register(ViPipe, 0x3000, 0x01); /* STANDBY */
}

void imx900_restart(VI_PIPE ViPipe)
{
	imx900_write_register(ViPipe, 0x3000, 0x00); /* standby */
}

void imx900_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastImx900[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		imx900_write_register(ViPipe,
				g_pastImx900[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastImx900[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void imx900_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	(void)(ViPipe);
	(void)(eSnsMirrorFlip);
	CVI_TRACE_SNS(CVI_DBG_INFO, "imx900 does not support independent flipping and mirroring\n");
}
#define IMX900_AQR 900
#define IMX900_AMR 1924
void imx900_init(VI_PIPE ViPipe)
{
	WDR_MODE_E enWDRMode;
	CVI_U8 u8ImgMode;
	CVI_U16 u16SensorIdentify = 0;

	enWDRMode   = g_pastImx900[ViPipe]->enWDRMode;
	u8ImgMode   = g_pastImx900[ViPipe]->u8ImgMode;

	imx900_i2c_init(ViPipe);
	imx900_restart(ViPipe);
	delay_ms(12);
	u16SensorIdentify = ((imx900_read_register(ViPipe, 0x3817) << 8) | imx900_read_register(ViPipe, 0x3816)) >> 5;
	if (u16SensorIdentify == IMX900_AQR) {
		CVI_TRACE_SNS(CVI_DBG_INFO, "This sensor is IMX900_AQR\n");
	} else if (u16SensorIdentify == IMX900_AMR) {
		CVI_TRACE_SNS(CVI_DBG_INFO, "This sensor is IMX900_AMR\n");
	}

	if (enWDRMode == WDR_MODE_NONE) {
		if (u8ImgMode == IMX900_MODE_3M70_COLOR)
			imx900_color_3M70_init(ViPipe);
		else if (u8ImgMode == IMX900_MODE_3M70_MONO)
			imx900_mono_3M70_init(ViPipe);
		else {
		}
	} else {
		CVI_TRACE_SNS(CVI_DBG_ERR, "error WDR Mode\n");
	}

	g_pastImx900[ViPipe]->bInit = CVI_TRUE;
}

// trigger mode setting
static void imx900_color_3M70_init(VI_PIPE ViPipe)
{
	delay_ms(4);
	imx900_write_register(ViPipe, 0x3000, 0x01); // STANDBY
	imx900_write_register(ViPipe, 0x3014, 0x1E); // INCKSEL_ST0
	imx900_write_register(ViPipe, 0x3015, 0x92); // INCKSEL_ST1
	imx900_write_register(ViPipe, 0x3016, 0xE0); // INCKSEL_ST2
	imx900_write_register(ViPipe, 0x3017, 0x01); // INCKSEL_ST3
	imx900_write_register(ViPipe, 0x3018, 0xB6); // INCKSEL_ST4
	imx900_write_register(ViPipe, 0x3019, 0x00); // INCKSEL_ST5
	imx900_write_register(ViPipe, 0x301C, 0xB6); // INCKSEL_ST6
	imx900_write_register(ViPipe, 0x301D, 0x00); // INCKSEL_ST7
	imx900_write_register(ViPipe, 0x303A, 0x15); // I2CSPICK
	imx900_write_register(ViPipe, 0x30E2, 0x11); // GMRWT
	imx900_write_register(ViPipe, 0x30E3, 0x27); // GMTWT
	imx900_write_register(ViPipe, 0x30E5, 0x02); // GAINDLY
	imx900_write_register(ViPipe, 0x30E6, 0x01); // GSDLY
	imx900_write_register(ViPipe, 0x3200, 0x11); // MONOSEL  ADBIT
	imx900_write_register(ViPipe, 0x32B6, 0x3A); //
	imx900_write_register(ViPipe, 0x3312, 0x39); //
	imx900_write_register(ViPipe, 0x3430, 0x01); // ODBIT
	imx900_write_register(ViPipe, 0x34D4, 0x78); //
	imx900_write_register(ViPipe, 0x34D5, 0x27); //
	imx900_write_register(ViPipe, 0x34D8, 0xA9); //
	imx900_write_register(ViPipe, 0x34D9, 0x5A); //
	imx900_write_register(ViPipe, 0x34F9, 0x12); //
	imx900_write_register(ViPipe, 0x3502, 0x08); // GAIN_RTS
	imx900_write_register(ViPipe, 0x3528, 0x00); //
	imx900_write_register(ViPipe, 0x352A, 0x00); //
	imx900_write_register(ViPipe, 0x352C, 0x00); //
	imx900_write_register(ViPipe, 0x352E, 0x00); //
	imx900_write_register(ViPipe, 0x3542, 0x03); //
	imx900_write_register(ViPipe, 0x3549, 0x2A); //
	imx900_write_register(ViPipe, 0x354A, 0x20); //
	imx900_write_register(ViPipe, 0x354B, 0x0C); //
	imx900_write_register(ViPipe, 0x359C, 0x19); //
	imx900_write_register(ViPipe, 0x359E, 0x24); //
	imx900_write_register(ViPipe, 0x35B4, 0xF0); // BLKLEVEL
	imx900_write_register(ViPipe, 0x35EA, 0xF0); //
	imx900_write_register(ViPipe, 0x35F4, 0x03); //
	imx900_write_register(ViPipe, 0x35F8, 0x01); //
	imx900_write_register(ViPipe, 0x3600, 0x00); //
	imx900_write_register(ViPipe, 0x3614, 0x00); //
	imx900_write_register(ViPipe, 0x362A, 0xEC); //
	imx900_write_register(ViPipe, 0x362B, 0x1F); //
	imx900_write_register(ViPipe, 0x362E, 0xF8); //
	imx900_write_register(ViPipe, 0x362F, 0x1F); //
	imx900_write_register(ViPipe, 0x3630, 0x5C); //
	imx900_write_register(ViPipe, 0x3648, 0xC6); //
	imx900_write_register(ViPipe, 0x364A, 0xEC); //
	imx900_write_register(ViPipe, 0x364B, 0x1F); //
	imx900_write_register(ViPipe, 0x364C, 0xDE); //
	imx900_write_register(ViPipe, 0x364E, 0xF8); //
	imx900_write_register(ViPipe, 0x364F, 0x1F); //
	imx900_write_register(ViPipe, 0x3652, 0xEC); //
	imx900_write_register(ViPipe, 0x3653, 0x1F); //
	imx900_write_register(ViPipe, 0x3656, 0xF8); //
	imx900_write_register(ViPipe, 0x3657, 0x1F); //
	imx900_write_register(ViPipe, 0x3658, 0x5C); //
	imx900_write_register(ViPipe, 0x3670, 0xC6); //
	imx900_write_register(ViPipe, 0x3672, 0xEC); //
	imx900_write_register(ViPipe, 0x3673, 0x1F); //
	imx900_write_register(ViPipe, 0x3674, 0xDE); //
	imx900_write_register(ViPipe, 0x3676, 0xF8); //
	imx900_write_register(ViPipe, 0x3677, 0x1F); //
	imx900_write_register(ViPipe, 0x367A, 0xEC); //
	imx900_write_register(ViPipe, 0x367B, 0x1F); //
	imx900_write_register(ViPipe, 0x367E, 0xF8); //
	imx900_write_register(ViPipe, 0x367F, 0x1F); //
	imx900_write_register(ViPipe, 0x3698, 0xC6); //
	imx900_write_register(ViPipe, 0x369A, 0xEC); //
	imx900_write_register(ViPipe, 0x369B, 0x1F); //
	imx900_write_register(ViPipe, 0x369C, 0xDE); //
	imx900_write_register(ViPipe, 0x369E, 0xF8); //
	imx900_write_register(ViPipe, 0x369F, 0x1F); //
	imx900_write_register(ViPipe, 0x36A8, 0x11); //
	imx900_write_register(ViPipe, 0x36A9, 0x1D); //
	imx900_write_register(ViPipe, 0x36B0, 0x28); //
	imx900_write_register(ViPipe, 0x36B1, 0x00); //
	imx900_write_register(ViPipe, 0x36B2, 0xF8); //
	imx900_write_register(ViPipe, 0x36B3, 0x1F); //
	imx900_write_register(ViPipe, 0x36BC, 0x28); //
	imx900_write_register(ViPipe, 0x36BD, 0x00); //
	imx900_write_register(ViPipe, 0x36BE, 0xF8); //
	imx900_write_register(ViPipe, 0x36BF, 0x1F); //
	imx900_write_register(ViPipe, 0x36D4, 0xEF); //
	imx900_write_register(ViPipe, 0x36D5, 0x01); //
	imx900_write_register(ViPipe, 0x36D6, 0x94); //
	imx900_write_register(ViPipe, 0x36D7, 0x03); //
	imx900_write_register(ViPipe, 0x36D8, 0xEF); //
	imx900_write_register(ViPipe, 0x36D9, 0x01); //
	imx900_write_register(ViPipe, 0x36DA, 0x94); //
	imx900_write_register(ViPipe, 0x36DB, 0x03); //
	imx900_write_register(ViPipe, 0x36DC, 0x9B); //
	imx900_write_register(ViPipe, 0x36DD, 0x09); //
	imx900_write_register(ViPipe, 0x36DE, 0x57); //
	imx900_write_register(ViPipe, 0x36DF, 0x11); //
	imx900_write_register(ViPipe, 0x36E0, 0xEB); //
	imx900_write_register(ViPipe, 0x36E1, 0x17); //
	imx900_write_register(ViPipe, 0x36E2, 0x0C); // GMRWT2
	imx900_write_register(ViPipe, 0x36E3, 0x17); // GMRWT3
	imx900_write_register(ViPipe, 0x37AC, 0x0E); //
	imx900_write_register(ViPipe, 0x37AE, 0x14); //
	imx900_write_register(ViPipe, 0x38E8, 0x82); //
	imx900_write_register(ViPipe, 0x3904, 0x02); // LANESEL
	imx900_write_register(ViPipe, 0x3C98, 0x80); // BASECK_FREQ
	imx900_write_register(ViPipe, 0x3C99, 0x09); //
	imx900_write_register(ViPipe, 0x3CA8, 0x5F); // THS_PREPARE
	imx900_write_register(ViPipe, 0x3CAA, 0xAF); // TCLK_POST
	imx900_write_register(ViPipe, 0x3CAC, 0x5F); // THS_TRAIL
	imx900_write_register(ViPipe, 0x3CAE, 0xAF); // THS_ZERO
	imx900_write_register(ViPipe, 0x3CAF, 0x00); //
	imx900_write_register(ViPipe, 0x3CB0, 0x5F); // TCLK_PREPARE
	imx900_write_register(ViPipe, 0x3CB2, 0x5F); // TCLK_TRAIL
	imx900_write_register(ViPipe, 0x3CB4, 0x4F); // TLPX
	imx900_write_register(ViPipe, 0x3CB6, 0x9F); // TCLK_ZERO
	imx900_write_register(ViPipe, 0x3CB7, 0x01); //
	imx900_write_register(ViPipe, 0x3CBA, 0x9F); // THS_EXIT
	imx900_write_register(ViPipe, 0x4100, 0x02); // INCKSEL_N0
	imx900_write_register(ViPipe, 0x4110, 0x02); // INCKSEL_D0
	imx900_write_register(ViPipe, 0x4111, 0x8A); // INCKSEL_D1
	imx900_write_register(ViPipe, 0x5038, 0x00); //
	imx900_write_register(ViPipe, 0x5039, 0x00); //
	imx900_write_register(ViPipe, 0x503A, 0xF6); //
	imx900_write_register(ViPipe, 0x505C, 0x96); // INCKSEL_STB0
	imx900_write_register(ViPipe, 0x505D, 0x02); // INCKSEL_STB1
	imx900_write_register(ViPipe, 0x505E, 0x96); // INCKSEL_STB2
	imx900_write_register(ViPipe, 0x505F, 0x02); // INCKSEL_STB3
	imx900_write_register(ViPipe, 0x5078, 0x09); //
	imx900_write_register(ViPipe, 0x507B, 0x11); //
	imx900_write_register(ViPipe, 0x507C, 0xFF); //
	imx900_write_register(ViPipe, 0x531C, 0x48); //
	imx900_write_register(ViPipe, 0x531E, 0x52); //
	imx900_write_register(ViPipe, 0x5320, 0x48); //
	imx900_write_register(ViPipe, 0x5322, 0x52); //
	imx900_write_register(ViPipe, 0x5324, 0x48); //
	imx900_write_register(ViPipe, 0x5326, 0x52); //
	imx900_write_register(ViPipe, 0x5328, 0x48); //
	imx900_write_register(ViPipe, 0x532A, 0x52); //
	imx900_write_register(ViPipe, 0x532C, 0x48); //
	imx900_write_register(ViPipe, 0x532E, 0x52); //
	imx900_write_register(ViPipe, 0x5330, 0x48); //
	imx900_write_register(ViPipe, 0x5332, 0x52); //
	imx900_write_register(ViPipe, 0x5334, 0x48); //
	imx900_write_register(ViPipe, 0x5336, 0x52); //
	imx900_write_register(ViPipe, 0x5338, 0x48); //
	imx900_write_register(ViPipe, 0x533A, 0x52); //
	imx900_write_register(ViPipe, 0x54D0, 0x40); // INCKSEL_STB4
	imx900_write_register(ViPipe, 0x54D1, 0x01); // INCKSEL_STB5
	imx900_write_register(ViPipe, 0x54D2, 0x81); // INCKSEL_STB6
	imx900_write_register(ViPipe, 0x54D3, 0x01); // INCKSEL_STB7
	imx900_write_register(ViPipe, 0x54D4, 0x15); // INCKSEL_STB8
	imx900_write_register(ViPipe, 0x54D5, 0x01); // INCKSEL_STB9
	imx900_write_register(ViPipe, 0x54D6, 0x00); // INCKSEL_STB10
	imx900_write_register(ViPipe, 0x5545, 0xA7); //
	imx900_write_register(ViPipe, 0x5546, 0x14); //
	imx900_write_register(ViPipe, 0x5547, 0x14); //
	imx900_write_register(ViPipe, 0x5548, 0x14); //
	imx900_write_register(ViPipe, 0x5550, 0x0A); //
	imx900_write_register(ViPipe, 0x5551, 0x0A); //
	imx900_write_register(ViPipe, 0x5552, 0x0A); //
	imx900_write_register(ViPipe, 0x5553, 0x6A); //
	imx900_write_register(ViPipe, 0x5572, 0x1F); //
	imx900_write_register(ViPipe, 0x5589, 0x0E); //
	imx900_write_register(ViPipe, 0x5613, 0x8F); //
	imx900_write_register(ViPipe, 0x5704, 0x0E); //
	imx900_write_register(ViPipe, 0x5705, 0x14); //
	imx900_write_register(ViPipe, 0x5832, 0x54); //
	imx900_write_register(ViPipe, 0x5836, 0x54); //
	imx900_write_register(ViPipe, 0x583A, 0x54); //
	imx900_write_register(ViPipe, 0x583E, 0x54); //
	imx900_write_register(ViPipe, 0x5842, 0x54); //
	imx900_write_register(ViPipe, 0x5846, 0x54); //
	imx900_write_register(ViPipe, 0x584A, 0x54); //
	imx900_write_register(ViPipe, 0x584E, 0x54); //
	imx900_write_register(ViPipe, 0x5852, 0x54); //
	imx900_write_register(ViPipe, 0x5856, 0x54); //
	imx900_write_register(ViPipe, 0x585A, 0x54); //
	imx900_write_register(ViPipe, 0x585E, 0x54); //
	imx900_write_register(ViPipe, 0x5862, 0x54); //
	imx900_write_register(ViPipe, 0x5866, 0x54); //
	imx900_write_register(ViPipe, 0x586A, 0x54); //
	imx900_write_register(ViPipe, 0x586E, 0x54); //
	imx900_write_register(ViPipe, 0x5872, 0x54); //
	imx900_write_register(ViPipe, 0x5876, 0x54); //
	imx900_write_register(ViPipe, 0x587A, 0x54); //
	imx900_write_register(ViPipe, 0x587E, 0x54); //
	imx900_write_register(ViPipe, 0x5882, 0x54); //
	imx900_write_register(ViPipe, 0x5886, 0x54); //
	imx900_write_register(ViPipe, 0x588A, 0x54); //
	imx900_write_register(ViPipe, 0x588E, 0x54); //
	imx900_write_register(ViPipe, 0x5902, 0xB0); //
	imx900_write_register(ViPipe, 0x5903, 0x04); //
	imx900_write_register(ViPipe, 0x590A, 0xB0); //
	imx900_write_register(ViPipe, 0x590B, 0x04); //
	imx900_write_register(ViPipe, 0x590C, 0xB0); //
	imx900_write_register(ViPipe, 0x590D, 0x09); //
	imx900_write_register(ViPipe, 0x590E, 0xC4); //
	imx900_write_register(ViPipe, 0x590F, 0x09); //
	imx900_write_register(ViPipe, 0x5934, 0x96); // INCKSEL_STB11
	imx900_write_register(ViPipe, 0x5935, 0x02); // INCKSEL_STB12
	imx900_write_register(ViPipe, 0x5936, 0x96); // INCKSEL_STB13
	imx900_write_register(ViPipe, 0x5937, 0x02); // INCKSEL_STB14
	imx900_write_register(ViPipe, 0x5939, 0x08); //
	imx900_write_register(ViPipe, 0x59AC, 0x00); // INCKSEL_STB15
	imx900_write_register(ViPipe, 0x59AE, 0x56); // INCKSEL_STB16
	imx900_write_register(ViPipe, 0x59AF, 0x01); // INCKSEL_STB17
	imx900_write_register(ViPipe, 0x5B4D, 0x24); //
	imx900_write_register(ViPipe, 0x5B81, 0x36); //
	imx900_write_register(ViPipe, 0x5BB5, 0x09); //
	imx900_write_register(ViPipe, 0x5BB8, 0x5C); // INCKSEL_STB18
	imx900_write_register(ViPipe, 0x5BBA, 0x3A); // INCKSEL_STB19
	imx900_write_register(ViPipe, 0x5BBC, 0xC5); // INCKSEL_STB20
	imx900_write_register(ViPipe, 0x5BBE, 0x0B); // INCKSEL_STB22
	imx900_write_register(ViPipe, 0x5BBF, 0x02); // INCKSEL_STB23
	imx900_write_register(ViPipe, 0x5BC0, 0x74); // INCKSEL_STB24
	imx900_write_register(ViPipe, 0x5BC1, 0x02); // INCKSEL_STB25
	imx900_write_register(ViPipe, 0x5BC2, 0x90); // INCKSEL_STB26
	imx900_write_register(ViPipe, 0x5BC3, 0x01); // INCKSEL_STB27
	imx900_write_register(ViPipe, 0x5BC9, 0x11); //
	imx900_write_register(ViPipe, 0x5BCC, 0x00); // INCKSEL_STB28
	imx900_write_register(ViPipe, 0x5BD8, 0x00); //
	imx900_write_register(ViPipe, 0x5BD9, 0x00); //
	imx900_write_register(ViPipe, 0x5BDC, 0x1D); //
	imx900_write_register(ViPipe, 0x5BDD, 0x00); //
	imx900_write_register(ViPipe, 0x5BE0, 0x1E); //
	imx900_write_register(ViPipe, 0x5BE1, 0x00); //
	imx900_write_register(ViPipe, 0x5BE4, 0x3B); //
	imx900_write_register(ViPipe, 0x5BE5, 0x00); //
	imx900_write_register(ViPipe, 0x5BE8, 0x3C); //
	imx900_write_register(ViPipe, 0x5BE9, 0x00); //
	imx900_write_register(ViPipe, 0x5BEC, 0x59); //
	imx900_write_register(ViPipe, 0x5BED, 0x00); //
	imx900_write_register(ViPipe, 0x5BF0, 0x5A); //
	imx900_write_register(ViPipe, 0x5BF1, 0x00); //
	imx900_write_register(ViPipe, 0x5BF4, 0x77); //
	imx900_write_register(ViPipe, 0x5BF5, 0x00); //
	imx900_write_register(ViPipe, 0x5C00, 0x00); //
	imx900_write_register(ViPipe, 0x5E04, 0x13); //
	imx900_write_register(ViPipe, 0x5E05, 0x05); //
	imx900_write_register(ViPipe, 0x5E06, 0x02); //
	imx900_write_register(ViPipe, 0x5E07, 0x00); //
	imx900_write_register(ViPipe, 0x5E14, 0x14); //
	imx900_write_register(ViPipe, 0x5E15, 0x05); //
	imx900_write_register(ViPipe, 0x5E16, 0x01); //
	imx900_write_register(ViPipe, 0x5E17, 0x00); //
	imx900_write_register(ViPipe, 0x5E34, 0x08); //
	imx900_write_register(ViPipe, 0x5E35, 0x05); //
	imx900_write_register(ViPipe, 0x5E36, 0x02); //
	imx900_write_register(ViPipe, 0x5E37, 0x00); //
	imx900_write_register(ViPipe, 0x5E44, 0x09); //
	imx900_write_register(ViPipe, 0x5E45, 0x05); //
	imx900_write_register(ViPipe, 0x5E46, 0x01); //
	imx900_write_register(ViPipe, 0x5E47, 0x00); //
	imx900_write_register(ViPipe, 0x5E98, 0x7C); //
	imx900_write_register(ViPipe, 0x5E99, 0x09); //
	imx900_write_register(ViPipe, 0x5EB8, 0x7E); //
	imx900_write_register(ViPipe, 0x5EB9, 0x09); //
	imx900_write_register(ViPipe, 0x5EC8, 0x18); //
	imx900_write_register(ViPipe, 0x5EC9, 0x09); //
	imx900_write_register(ViPipe, 0x5ECA, 0xE8); //
	imx900_write_register(ViPipe, 0x5ECB, 0x03); //
	imx900_write_register(ViPipe, 0x5ED8, 0x1A); //
	imx900_write_register(ViPipe, 0x5ED9, 0x09); //
	imx900_write_register(ViPipe, 0x5EDA, 0xE6); //
	imx900_write_register(ViPipe, 0x5EDB, 0x03); //
	imx900_write_register(ViPipe, 0x5F08, 0x18); //
	imx900_write_register(ViPipe, 0x5F09, 0x09); //
	imx900_write_register(ViPipe, 0x5F0A, 0xE8); //
	imx900_write_register(ViPipe, 0x5F0B, 0x03); //
	imx900_write_register(ViPipe, 0x5F18, 0x1A); //
	imx900_write_register(ViPipe, 0x5F19, 0x09); //
	imx900_write_register(ViPipe, 0x5F1A, 0xE6); //
	imx900_write_register(ViPipe, 0x5F1B, 0x03); //
	imx900_write_register(ViPipe, 0x5F3A, 0xE8); //
	imx900_write_register(ViPipe, 0x5F3B, 0x03); //
	imx900_write_register(ViPipe, 0x5F48, 0x1A); //
	imx900_write_register(ViPipe, 0x5F49, 0x09); //
	imx900_write_register(ViPipe, 0x5F4A, 0xE6); //
	imx900_write_register(ViPipe, 0x5F4B, 0x03); //
	imx900_write_register(ViPipe, 0x5F68, 0x18); //
	imx900_write_register(ViPipe, 0x5F69, 0x09); //
	imx900_write_register(ViPipe, 0x5F6A, 0xE8); //
	imx900_write_register(ViPipe, 0x5F6B, 0x03); //
	imx900_write_register(ViPipe, 0x5F78, 0x1A); //
	imx900_write_register(ViPipe, 0x5F79, 0x09); //
	imx900_write_register(ViPipe, 0x5F7A, 0xE6); //
	imx900_write_register(ViPipe, 0x5F7B, 0x03); //
	imx900_write_register(ViPipe, 0x60B4, 0x1E); //
	imx900_write_register(ViPipe, 0x60C0, 0x1F); //
	imx900_write_register(ViPipe, 0x6178, 0x7C); //
	imx900_write_register(ViPipe, 0x6179, 0x09); //
	imx900_write_register(ViPipe, 0x6198, 0x7E); //
	imx900_write_register(ViPipe, 0x6199, 0x09); //
	imx900_write_register(ViPipe, 0x6278, 0x18); //
	imx900_write_register(ViPipe, 0x6279, 0x09); //
	imx900_write_register(ViPipe, 0x627A, 0xE8); //
	imx900_write_register(ViPipe, 0x627B, 0x03); //
	imx900_write_register(ViPipe, 0x6288, 0x1A); //
	imx900_write_register(ViPipe, 0x6289, 0x09); //
	imx900_write_register(ViPipe, 0x628A, 0xE6); //
	imx900_write_register(ViPipe, 0x628B, 0x03); //
	imx900_write_register(ViPipe, 0x62A8, 0x18); //
	imx900_write_register(ViPipe, 0x62A9, 0x09); //
	imx900_write_register(ViPipe, 0x62AA, 0xE8); //
	imx900_write_register(ViPipe, 0x62AB, 0x03); //
	imx900_write_register(ViPipe, 0x62B8, 0x1A); //
	imx900_write_register(ViPipe, 0x62B9, 0x09); //
	imx900_write_register(ViPipe, 0x62D8, 0x18); //
	imx900_write_register(ViPipe, 0x62D9, 0x09); //
	imx900_write_register(ViPipe, 0x62DA, 0xE8); //
	imx900_write_register(ViPipe, 0x62DB, 0x03); //
	imx900_write_register(ViPipe, 0x62E8, 0x1A); //
	imx900_write_register(ViPipe, 0x62E9, 0x09); //
	imx900_write_register(ViPipe, 0x62EA, 0xE6); //
	imx900_write_register(ViPipe, 0x62EB, 0x03); //
	imx900_write_register(ViPipe, 0x6318, 0x18); //
	imx900_write_register(ViPipe, 0x6319, 0x09); //
	imx900_write_register(ViPipe, 0x631A, 0xE8); //
	imx900_write_register(ViPipe, 0x631B, 0x03); //
	imx900_write_register(ViPipe, 0x6328, 0x1A); //
	imx900_write_register(ViPipe, 0x6329, 0x09); //
	imx900_write_register(ViPipe, 0x632A, 0xE6); //
	imx900_write_register(ViPipe, 0x632B, 0x03); //
	imx900_write_register(ViPipe, 0x6398, 0x1E); //
	imx900_write_register(ViPipe, 0x63A4, 0x1F); //
	imx900_write_register(ViPipe, 0x6501, 0x01); //
	imx900_write_register(ViPipe, 0x6505, 0x00); //
	imx900_write_register(ViPipe, 0x6508, 0x00); //
	imx900_write_register(ViPipe, 0x650C, 0x01); //
	imx900_write_register(ViPipe, 0x6510, 0x00); //
	imx900_write_register(ViPipe, 0x6514, 0x01); //
	imx900_write_register(ViPipe, 0x6519, 0x01); //
	imx900_write_register(ViPipe, 0x651D, 0x00); //
	imx900_write_register(ViPipe, 0x6528, 0x00); //
	imx900_write_register(ViPipe, 0x652C, 0x01); //
	imx900_write_register(ViPipe, 0x6531, 0x01); //
	imx900_write_register(ViPipe, 0x6535, 0x00); //
	imx900_write_register(ViPipe, 0x6538, 0x00); //
	imx900_write_register(ViPipe, 0x653C, 0x01); //
	imx900_write_register(ViPipe, 0x6541, 0x01); //
	imx900_write_register(ViPipe, 0x6545, 0x00); //
	imx900_write_register(ViPipe, 0x6549, 0x01); //
	imx900_write_register(ViPipe, 0x654D, 0x00); //
	imx900_write_register(ViPipe, 0x6558, 0x00); //
	imx900_write_register(ViPipe, 0x655C, 0x01); //
	imx900_write_register(ViPipe, 0x6560, 0x00); //
	imx900_write_register(ViPipe, 0x6564, 0x01); //
	imx900_write_register(ViPipe, 0x6571, 0x01); //
	imx900_write_register(ViPipe, 0x6575, 0x00); //
	imx900_write_register(ViPipe, 0x6579, 0x01); //
	imx900_write_register(ViPipe, 0x657D, 0x00); //
	imx900_write_register(ViPipe, 0x6588, 0x00); //
	imx900_write_register(ViPipe, 0x658C, 0x01); //
	imx900_write_register(ViPipe, 0x6590, 0x00); //
	imx900_write_register(ViPipe, 0x6594, 0x01); //
	imx900_write_register(ViPipe, 0x6598, 0x00); //
	imx900_write_register(ViPipe, 0x659C, 0x01); //
	imx900_write_register(ViPipe, 0x65A0, 0x00); //
	imx900_write_register(ViPipe, 0x65A4, 0x01); //
	imx900_write_register(ViPipe, 0x65B0, 0x00); //
	imx900_write_register(ViPipe, 0x65B4, 0x01); //
	imx900_write_register(ViPipe, 0x65B9, 0x00); //
	imx900_write_register(ViPipe, 0x65BD, 0x00); //
	imx900_write_register(ViPipe, 0x65C1, 0x00); //
	imx900_write_register(ViPipe, 0x65C9, 0x00); //
	imx900_write_register(ViPipe, 0x65CC, 0x00); //
	imx900_write_register(ViPipe, 0x65D0, 0x00); //
	imx900_write_register(ViPipe, 0x65D4, 0x00); //
	imx900_write_register(ViPipe, 0x65DC, 0x00); //
	imx900_write_register(ViPipe, 0x3400, 0x09); //
	imx900_write_register(ViPipe, 0x3000, 0x00); // stream on

	imx900_default_reg_init(ViPipe);

	printf("ViPipe:%d,===IMX900 3M 70fps 12bit Color Init OK!===\n", ViPipe);
}

// trigger mode setting
static void imx900_mono_3M70_init(VI_PIPE ViPipe)
{
	delay_ms(4);

	imx900_write_register(ViPipe, 0x3000, 0x01); /* STANDBY */
	imx900_write_register(ViPipe, 0x3014, 0x1E); // INCKSEL_ST0
	imx900_write_register(ViPipe, 0x3015, 0x92); // INCKSEL_ST1
	imx900_write_register(ViPipe, 0x3016, 0xE0); // INCKSEL_ST2
	imx900_write_register(ViPipe, 0x3017, 0x01); // INCKSEL_ST3
	imx900_write_register(ViPipe, 0x3018, 0xB6); // INCKSEL_ST4
	imx900_write_register(ViPipe, 0x3019, 0x00); // INCKSEL_ST5
	imx900_write_register(ViPipe, 0x301C, 0xB6); // INCKSEL_ST6
	imx900_write_register(ViPipe, 0x301D, 0x00); // INCKSEL_ST7
	imx900_write_register(ViPipe, 0x303A, 0x15); // I2CSPICK
	imx900_write_register(ViPipe, 0x30E2, 0x11); // GMRWT
	imx900_write_register(ViPipe, 0x30E3, 0x27); // GMTWT
	imx900_write_register(ViPipe, 0x30E5, 0x02); // GAINDLY
	imx900_write_register(ViPipe, 0x30E6, 0x01); // GSDLY
	imx900_write_register(ViPipe, 0x3200, 0x15); // MONOSEL  ADBIT
	imx900_write_register(ViPipe, 0x32B6, 0x3A); //
	imx900_write_register(ViPipe, 0x3312, 0x39); //
	imx900_write_register(ViPipe, 0x3430, 0x01); // ODBIT
	imx900_write_register(ViPipe, 0x34D4, 0x78); //
	imx900_write_register(ViPipe, 0x34D5, 0x27); //
	imx900_write_register(ViPipe, 0x34D8, 0xA9); //
	imx900_write_register(ViPipe, 0x34D9, 0x5A); //
	imx900_write_register(ViPipe, 0x34F9, 0x12); //
	imx900_write_register(ViPipe, 0x3502, 0x08); // GAIN_RTS
	imx900_write_register(ViPipe, 0x3528, 0x00); //
	imx900_write_register(ViPipe, 0x352A, 0x00); //
	imx900_write_register(ViPipe, 0x352C, 0x00); //
	imx900_write_register(ViPipe, 0x352E, 0x00); //
	imx900_write_register(ViPipe, 0x3542, 0x03); //
	imx900_write_register(ViPipe, 0x3549, 0x2A); //
	imx900_write_register(ViPipe, 0x354A, 0x20); //
	imx900_write_register(ViPipe, 0x354B, 0x0C); //
	imx900_write_register(ViPipe, 0x359C, 0x19); //
	imx900_write_register(ViPipe, 0x359E, 0x24); //
	imx900_write_register(ViPipe, 0x35B4, 0xF0); // BLKLEVEL
	imx900_write_register(ViPipe, 0x35EA, 0xF0); //
	imx900_write_register(ViPipe, 0x35F4, 0x03); //
	imx900_write_register(ViPipe, 0x35F8, 0x01); //
	imx900_write_register(ViPipe, 0x3600, 0x00); //
	imx900_write_register(ViPipe, 0x3614, 0x00); //
	imx900_write_register(ViPipe, 0x362A, 0xEC); //
	imx900_write_register(ViPipe, 0x362B, 0x1F); //
	imx900_write_register(ViPipe, 0x362E, 0xF8); //
	imx900_write_register(ViPipe, 0x362F, 0x1F); //
	imx900_write_register(ViPipe, 0x3630, 0x5C); //
	imx900_write_register(ViPipe, 0x3648, 0xC6); //
	imx900_write_register(ViPipe, 0x364A, 0xEC); //
	imx900_write_register(ViPipe, 0x364B, 0x1F); //
	imx900_write_register(ViPipe, 0x364C, 0xDE); //
	imx900_write_register(ViPipe, 0x364E, 0xF8); //
	imx900_write_register(ViPipe, 0x364F, 0x1F); //
	imx900_write_register(ViPipe, 0x3652, 0xEC); //
	imx900_write_register(ViPipe, 0x3653, 0x1F); //
	imx900_write_register(ViPipe, 0x3656, 0xF8); //
	imx900_write_register(ViPipe, 0x3657, 0x1F); //
	imx900_write_register(ViPipe, 0x3658, 0x5C); //
	imx900_write_register(ViPipe, 0x3670, 0xC6); //
	imx900_write_register(ViPipe, 0x3672, 0xEC); //
	imx900_write_register(ViPipe, 0x3673, 0x1F); //
	imx900_write_register(ViPipe, 0x3674, 0xDE); //
	imx900_write_register(ViPipe, 0x3676, 0xF8); //
	imx900_write_register(ViPipe, 0x3677, 0x1F); //
	imx900_write_register(ViPipe, 0x367A, 0xEC); //
	imx900_write_register(ViPipe, 0x367B, 0x1F); //
	imx900_write_register(ViPipe, 0x367E, 0xF8); //
	imx900_write_register(ViPipe, 0x367F, 0x1F); //
	imx900_write_register(ViPipe, 0x3698, 0xC6); //
	imx900_write_register(ViPipe, 0x369A, 0xEC); //
	imx900_write_register(ViPipe, 0x369B, 0x1F); //
	imx900_write_register(ViPipe, 0x369C, 0xDE); //
	imx900_write_register(ViPipe, 0x369E, 0xF8); //
	imx900_write_register(ViPipe, 0x369F, 0x1F); //
	imx900_write_register(ViPipe, 0x36A8, 0x11); //
	imx900_write_register(ViPipe, 0x36A9, 0x1D); //
	imx900_write_register(ViPipe, 0x36B0, 0x28); //
	imx900_write_register(ViPipe, 0x36B1, 0x00); //
	imx900_write_register(ViPipe, 0x36B2, 0xF8); //
	imx900_write_register(ViPipe, 0x36B3, 0x1F); //
	imx900_write_register(ViPipe, 0x36BC, 0x28); //
	imx900_write_register(ViPipe, 0x36BD, 0x00); //
	imx900_write_register(ViPipe, 0x36BE, 0xF8); //
	imx900_write_register(ViPipe, 0x36BF, 0x1F); //
	imx900_write_register(ViPipe, 0x36D4, 0xEF); //
	imx900_write_register(ViPipe, 0x36D5, 0x01); //
	imx900_write_register(ViPipe, 0x36D6, 0x94); //
	imx900_write_register(ViPipe, 0x36D7, 0x03); //
	imx900_write_register(ViPipe, 0x36D8, 0xEF); //
	imx900_write_register(ViPipe, 0x36D9, 0x01); //
	imx900_write_register(ViPipe, 0x36DA, 0x94); //
	imx900_write_register(ViPipe, 0x36DB, 0x03); //
	imx900_write_register(ViPipe, 0x36DC, 0x9B); //
	imx900_write_register(ViPipe, 0x36DD, 0x09); //
	imx900_write_register(ViPipe, 0x36DE, 0x57); //
	imx900_write_register(ViPipe, 0x36DF, 0x11); //
	imx900_write_register(ViPipe, 0x36E0, 0xEB); //
	imx900_write_register(ViPipe, 0x36E1, 0x17); //
	imx900_write_register(ViPipe, 0x36E2, 0x0C); // GMRWT2
	imx900_write_register(ViPipe, 0x36E3, 0x17); // GMRWT3
	imx900_write_register(ViPipe, 0x37AC, 0x0E); //
	imx900_write_register(ViPipe, 0x37AE, 0x14); //
	imx900_write_register(ViPipe, 0x38E8, 0x82); //
	imx900_write_register(ViPipe, 0x3904, 0x02); // LANESEL
	imx900_write_register(ViPipe, 0x3C98, 0x80); // BASECK_FREQ
	imx900_write_register(ViPipe, 0x3C99, 0x09); //
	imx900_write_register(ViPipe, 0x3CA8, 0x4F); // THS_PREPARE
	imx900_write_register(ViPipe, 0x3CAA, 0x9F); // TCLK_POST
	imx900_write_register(ViPipe, 0x3CAC, 0x4F); // THS_TRAIL
	imx900_write_register(ViPipe, 0x3CAE, 0x9F); // THS_ZERO
	imx900_write_register(ViPipe, 0x3CAF, 0x00); //
	imx900_write_register(ViPipe, 0x3CB0, 0x4F); // TCLK_PREPARE
	imx900_write_register(ViPipe, 0x3CB2, 0x4F); // TCLK_TRAIL
	imx900_write_register(ViPipe, 0x3CB4, 0x3F); // TLPX
	imx900_write_register(ViPipe, 0x3CB6, 0x4F); // TCLK_ZERO
	imx900_write_register(ViPipe, 0x3CB7, 0x01); //
	imx900_write_register(ViPipe, 0x3CBA, 0x7F); // THS_EXIT
	imx900_write_register(ViPipe, 0x4100, 0x02); // INCKSEL_N0
	imx900_write_register(ViPipe, 0x4110, 0x02); // INCKSEL_D0
	imx900_write_register(ViPipe, 0x4111, 0x88); // INCKSEL_D1
	imx900_write_register(ViPipe, 0x5038, 0x00); //
	imx900_write_register(ViPipe, 0x5039, 0x00); //
	imx900_write_register(ViPipe, 0x503A, 0xF6); //
	imx900_write_register(ViPipe, 0x505C, 0x96); // INCKSEL_STB0
	imx900_write_register(ViPipe, 0x505D, 0x02); // INCKSEL_STB1
	imx900_write_register(ViPipe, 0x505E, 0x96); // INCKSEL_STB2
	imx900_write_register(ViPipe, 0x505F, 0x02); // INCKSEL_STB3
	imx900_write_register(ViPipe, 0x5078, 0x09); //
	imx900_write_register(ViPipe, 0x507B, 0x11); //
	imx900_write_register(ViPipe, 0x507C, 0xFF); //
	imx900_write_register(ViPipe, 0x531C, 0x48); //
	imx900_write_register(ViPipe, 0x531E, 0x52); //
	imx900_write_register(ViPipe, 0x5320, 0x48); //
	imx900_write_register(ViPipe, 0x5322, 0x52); //
	imx900_write_register(ViPipe, 0x5324, 0x48); //
	imx900_write_register(ViPipe, 0x5326, 0x52); //
	imx900_write_register(ViPipe, 0x5328, 0x48); //
	imx900_write_register(ViPipe, 0x532A, 0x52); //
	imx900_write_register(ViPipe, 0x532C, 0x48); //
	imx900_write_register(ViPipe, 0x532E, 0x52); //
	imx900_write_register(ViPipe, 0x5330, 0x48); //
	imx900_write_register(ViPipe, 0x5332, 0x52); //
	imx900_write_register(ViPipe, 0x5334, 0x48); //
	imx900_write_register(ViPipe, 0x5336, 0x52); //
	imx900_write_register(ViPipe, 0x5338, 0x48); //
	imx900_write_register(ViPipe, 0x533A, 0x52); //
	imx900_write_register(ViPipe, 0x54D0, 0x40); // INCKSEL_STB4
	imx900_write_register(ViPipe, 0x54D1, 0x01); // INCKSEL_STB5
	imx900_write_register(ViPipe, 0x54D2, 0x81); // INCKSEL_STB6
	imx900_write_register(ViPipe, 0x54D3, 0x01); // INCKSEL_STB7
	imx900_write_register(ViPipe, 0x54D4, 0x15); // INCKSEL_STB8
	imx900_write_register(ViPipe, 0x54D5, 0x01); // INCKSEL_STB9
	imx900_write_register(ViPipe, 0x54D6, 0x00); // INCKSEL_STB10
	imx900_write_register(ViPipe, 0x5545, 0xA7); //
	imx900_write_register(ViPipe, 0x5546, 0x14); //
	imx900_write_register(ViPipe, 0x5547, 0x14); //
	imx900_write_register(ViPipe, 0x5548, 0x14); //
	imx900_write_register(ViPipe, 0x5550, 0x0A); //
	imx900_write_register(ViPipe, 0x5551, 0x0A); //
	imx900_write_register(ViPipe, 0x5552, 0x0A); //
	imx900_write_register(ViPipe, 0x5553, 0x6A); //
	imx900_write_register(ViPipe, 0x5572, 0x1F); //
	imx900_write_register(ViPipe, 0x5589, 0x0E); //
	imx900_write_register(ViPipe, 0x5613, 0x8F); //
	imx900_write_register(ViPipe, 0x5704, 0x0E); //
	imx900_write_register(ViPipe, 0x5705, 0x14); //
	imx900_write_register(ViPipe, 0x5832, 0x54); //
	imx900_write_register(ViPipe, 0x5836, 0x54); //
	imx900_write_register(ViPipe, 0x583A, 0x54); //
	imx900_write_register(ViPipe, 0x583E, 0x54); //
	imx900_write_register(ViPipe, 0x5842, 0x54); //
	imx900_write_register(ViPipe, 0x5846, 0x54); //
	imx900_write_register(ViPipe, 0x584A, 0x54); //
	imx900_write_register(ViPipe, 0x584E, 0x54); //
	imx900_write_register(ViPipe, 0x5852, 0x54); //
	imx900_write_register(ViPipe, 0x5856, 0x54); //
	imx900_write_register(ViPipe, 0x585A, 0x54); //
	imx900_write_register(ViPipe, 0x585E, 0x54); //
	imx900_write_register(ViPipe, 0x5862, 0x54); //
	imx900_write_register(ViPipe, 0x5866, 0x54); //
	imx900_write_register(ViPipe, 0x586A, 0x54); //
	imx900_write_register(ViPipe, 0x586E, 0x54); //
	imx900_write_register(ViPipe, 0x5872, 0x54); //
	imx900_write_register(ViPipe, 0x5876, 0x54); //
	imx900_write_register(ViPipe, 0x587A, 0x54); //
	imx900_write_register(ViPipe, 0x587E, 0x54); //
	imx900_write_register(ViPipe, 0x5882, 0x54); //
	imx900_write_register(ViPipe, 0x5886, 0x54); //
	imx900_write_register(ViPipe, 0x588A, 0x54); //
	imx900_write_register(ViPipe, 0x588E, 0x54); //
	imx900_write_register(ViPipe, 0x5902, 0xB0); //
	imx900_write_register(ViPipe, 0x5903, 0x04); //
	imx900_write_register(ViPipe, 0x590A, 0xB0); //
	imx900_write_register(ViPipe, 0x590B, 0x04); //
	imx900_write_register(ViPipe, 0x590C, 0xB0); //
	imx900_write_register(ViPipe, 0x590D, 0x09); //
	imx900_write_register(ViPipe, 0x590E, 0xC4); //
	imx900_write_register(ViPipe, 0x590F, 0x09); //
	imx900_write_register(ViPipe, 0x5934, 0x96); // INCKSEL_STB11
	imx900_write_register(ViPipe, 0x5935, 0x02); // INCKSEL_STB12
	imx900_write_register(ViPipe, 0x5936, 0x96); // INCKSEL_STB13
	imx900_write_register(ViPipe, 0x5937, 0x02); // INCKSEL_STB14
	imx900_write_register(ViPipe, 0x5939, 0x08); //
	imx900_write_register(ViPipe, 0x59AC, 0x00); // INCKSEL_STB15
	imx900_write_register(ViPipe, 0x59AE, 0x56); // INCKSEL_STB16
	imx900_write_register(ViPipe, 0x59AF, 0x01); // INCKSEL_STB17
	imx900_write_register(ViPipe, 0x5B4D, 0x24); //
	imx900_write_register(ViPipe, 0x5B81, 0x36); //
	imx900_write_register(ViPipe, 0x5BB5, 0x09); //
	imx900_write_register(ViPipe, 0x5BB8, 0x5C); // INCKSEL_STB18
	imx900_write_register(ViPipe, 0x5BBA, 0x3A); // INCKSEL_STB19
	imx900_write_register(ViPipe, 0x5BBC, 0xC5); // INCKSEL_STB20
	imx900_write_register(ViPipe, 0x5BBE, 0x0B); // INCKSEL_STB22
	imx900_write_register(ViPipe, 0x5BBF, 0x02); // INCKSEL_STB23
	imx900_write_register(ViPipe, 0x5BC0, 0x74); // INCKSEL_STB24
	imx900_write_register(ViPipe, 0x5BC1, 0x02); // INCKSEL_STB25
	imx900_write_register(ViPipe, 0x5BC2, 0x90); // INCKSEL_STB26
	imx900_write_register(ViPipe, 0x5BC3, 0x01); // INCKSEL_STB27
	imx900_write_register(ViPipe, 0x5BC9, 0x11); //
	imx900_write_register(ViPipe, 0x5BCC, 0x00); // INCKSEL_STB28
	imx900_write_register(ViPipe, 0x5BD8, 0x00); //
	imx900_write_register(ViPipe, 0x5BD9, 0x00); //
	imx900_write_register(ViPipe, 0x5BDC, 0x1D); //
	imx900_write_register(ViPipe, 0x5BDD, 0x00); //
	imx900_write_register(ViPipe, 0x5BE0, 0x1E); //
	imx900_write_register(ViPipe, 0x5BE1, 0x00); //
	imx900_write_register(ViPipe, 0x5BE4, 0x3B); //
	imx900_write_register(ViPipe, 0x5BE5, 0x00); //
	imx900_write_register(ViPipe, 0x5BE8, 0x3C); //
	imx900_write_register(ViPipe, 0x5BE9, 0x00); //
	imx900_write_register(ViPipe, 0x5BEC, 0x59); //
	imx900_write_register(ViPipe, 0x5BED, 0x00); //
	imx900_write_register(ViPipe, 0x5BF0, 0x5A); //
	imx900_write_register(ViPipe, 0x5BF1, 0x00); //
	imx900_write_register(ViPipe, 0x5BF4, 0x77); //
	imx900_write_register(ViPipe, 0x5BF5, 0x00); //
	imx900_write_register(ViPipe, 0x5C00, 0x00); //
	imx900_write_register(ViPipe, 0x5E04, 0x13); //
	imx900_write_register(ViPipe, 0x5E05, 0x05); //
	imx900_write_register(ViPipe, 0x5E06, 0x02); //
	imx900_write_register(ViPipe, 0x5E07, 0x00); //
	imx900_write_register(ViPipe, 0x5E14, 0x14); //
	imx900_write_register(ViPipe, 0x5E15, 0x05); //
	imx900_write_register(ViPipe, 0x5E16, 0x01); //
	imx900_write_register(ViPipe, 0x5E17, 0x00); //
	imx900_write_register(ViPipe, 0x5E34, 0x08); //
	imx900_write_register(ViPipe, 0x5E35, 0x05); //
	imx900_write_register(ViPipe, 0x5E36, 0x02); //
	imx900_write_register(ViPipe, 0x5E37, 0x00); //
	imx900_write_register(ViPipe, 0x5E44, 0x09); //
	imx900_write_register(ViPipe, 0x5E45, 0x05); //
	imx900_write_register(ViPipe, 0x5E46, 0x01); //
	imx900_write_register(ViPipe, 0x5E47, 0x00); //
	imx900_write_register(ViPipe, 0x5E98, 0x7C); //
	imx900_write_register(ViPipe, 0x5E99, 0x09); //
	imx900_write_register(ViPipe, 0x5EB8, 0x7E); //
	imx900_write_register(ViPipe, 0x5EB9, 0x09); //
	imx900_write_register(ViPipe, 0x5EC8, 0x18); //
	imx900_write_register(ViPipe, 0x5EC9, 0x09); //
	imx900_write_register(ViPipe, 0x5ECA, 0xE8); //
	imx900_write_register(ViPipe, 0x5ECB, 0x03); //
	imx900_write_register(ViPipe, 0x5ED8, 0x1A); //
	imx900_write_register(ViPipe, 0x5ED9, 0x09); //
	imx900_write_register(ViPipe, 0x5EDA, 0xE6); //
	imx900_write_register(ViPipe, 0x5EDB, 0x03); //
	imx900_write_register(ViPipe, 0x5F08, 0x18); //
	imx900_write_register(ViPipe, 0x5F09, 0x09); //
	imx900_write_register(ViPipe, 0x5F0A, 0xE8); //
	imx900_write_register(ViPipe, 0x5F0B, 0x03); //
	imx900_write_register(ViPipe, 0x5F18, 0x1A); //
	imx900_write_register(ViPipe, 0x5F19, 0x09); //
	imx900_write_register(ViPipe, 0x5F1A, 0xE6); //
	imx900_write_register(ViPipe, 0x5F1B, 0x03); //
	imx900_write_register(ViPipe, 0x5F3A, 0xE8); //
	imx900_write_register(ViPipe, 0x5F3B, 0x03); //
	imx900_write_register(ViPipe, 0x5F48, 0x1A); //
	imx900_write_register(ViPipe, 0x5F49, 0x09); //
	imx900_write_register(ViPipe, 0x5F4A, 0xE6); //
	imx900_write_register(ViPipe, 0x5F4B, 0x03); //
	imx900_write_register(ViPipe, 0x5F68, 0x18); //
	imx900_write_register(ViPipe, 0x5F69, 0x09); //
	imx900_write_register(ViPipe, 0x5F6A, 0xE8); //
	imx900_write_register(ViPipe, 0x5F6B, 0x03); //
	imx900_write_register(ViPipe, 0x5F78, 0x1A); //
	imx900_write_register(ViPipe, 0x5F79, 0x09); //
	imx900_write_register(ViPipe, 0x5F7A, 0xE6); //
	imx900_write_register(ViPipe, 0x5F7B, 0x03); //
	imx900_write_register(ViPipe, 0x60B4, 0x1E); //
	imx900_write_register(ViPipe, 0x60C0, 0x1F); //
	imx900_write_register(ViPipe, 0x6178, 0x7C); //
	imx900_write_register(ViPipe, 0x6179, 0x09); //
	imx900_write_register(ViPipe, 0x6198, 0x7E); //
	imx900_write_register(ViPipe, 0x6199, 0x09); //
	imx900_write_register(ViPipe, 0x6278, 0x18); //
	imx900_write_register(ViPipe, 0x6279, 0x09); //
	imx900_write_register(ViPipe, 0x627A, 0xE8); //
	imx900_write_register(ViPipe, 0x627B, 0x03); //
	imx900_write_register(ViPipe, 0x6288, 0x1A); //
	imx900_write_register(ViPipe, 0x6289, 0x09); //
	imx900_write_register(ViPipe, 0x628A, 0xE6); //
	imx900_write_register(ViPipe, 0x628B, 0x03); //
	imx900_write_register(ViPipe, 0x62A8, 0x18); //
	imx900_write_register(ViPipe, 0x62A9, 0x09); //
	imx900_write_register(ViPipe, 0x62AA, 0xE8); //
	imx900_write_register(ViPipe, 0x62AB, 0x03); //
	imx900_write_register(ViPipe, 0x62B8, 0x1A); //
	imx900_write_register(ViPipe, 0x62B9, 0x09); //
	imx900_write_register(ViPipe, 0x62D8, 0x18); //
	imx900_write_register(ViPipe, 0x62D9, 0x09); //
	imx900_write_register(ViPipe, 0x62DA, 0xE8); //
	imx900_write_register(ViPipe, 0x62DB, 0x03); //
	imx900_write_register(ViPipe, 0x62EA, 0xE6); //
	imx900_write_register(ViPipe, 0x62EB, 0x03); //
	imx900_write_register(ViPipe, 0x6318, 0x18); //
	imx900_write_register(ViPipe, 0x6319, 0x09); //
	imx900_write_register(ViPipe, 0x631A, 0xE8); //
	imx900_write_register(ViPipe, 0x631B, 0x03); //
	imx900_write_register(ViPipe, 0x6328, 0x1A); //
	imx900_write_register(ViPipe, 0x6329, 0x09); //
	imx900_write_register(ViPipe, 0x632A, 0xE6); //
	imx900_write_register(ViPipe, 0x632B, 0x03); //
	imx900_write_register(ViPipe, 0x6398, 0x1E); //
	imx900_write_register(ViPipe, 0x63A4, 0x1F); //
	imx900_write_register(ViPipe, 0x6501, 0x01); //
	imx900_write_register(ViPipe, 0x6505, 0x00); //
	imx900_write_register(ViPipe, 0x6508, 0x00); //
	imx900_write_register(ViPipe, 0x650C, 0x01); //
	imx900_write_register(ViPipe, 0x6510, 0x00); //
	imx900_write_register(ViPipe, 0x6514, 0x01); //
	imx900_write_register(ViPipe, 0x6519, 0x01); //
	imx900_write_register(ViPipe, 0x651D, 0x00); //
	imx900_write_register(ViPipe, 0x6528, 0x00); //
	imx900_write_register(ViPipe, 0x652C, 0x01); //
	imx900_write_register(ViPipe, 0x6531, 0x01); //
	imx900_write_register(ViPipe, 0x6535, 0x00); //
	imx900_write_register(ViPipe, 0x6538, 0x00); //
	imx900_write_register(ViPipe, 0x653C, 0x01); //
	imx900_write_register(ViPipe, 0x6541, 0x01); //
	imx900_write_register(ViPipe, 0x6545, 0x00); //
	imx900_write_register(ViPipe, 0x6549, 0x01); //
	imx900_write_register(ViPipe, 0x654D, 0x00); //
	imx900_write_register(ViPipe, 0x6558, 0x00); //
	imx900_write_register(ViPipe, 0x655C, 0x01); //
	imx900_write_register(ViPipe, 0x6560, 0x00); //
	imx900_write_register(ViPipe, 0x6564, 0x01); //
	imx900_write_register(ViPipe, 0x6571, 0x01); //
	imx900_write_register(ViPipe, 0x6575, 0x00); //
	imx900_write_register(ViPipe, 0x6579, 0x01); //
	imx900_write_register(ViPipe, 0x657D, 0x00); //
	imx900_write_register(ViPipe, 0x6588, 0x00); //
	imx900_write_register(ViPipe, 0x658C, 0x01); //
	imx900_write_register(ViPipe, 0x6590, 0x00); //
	imx900_write_register(ViPipe, 0x6594, 0x01); //
	imx900_write_register(ViPipe, 0x6598, 0x00); //
	imx900_write_register(ViPipe, 0x659C, 0x01); //
	imx900_write_register(ViPipe, 0x65A0, 0x00); //
	imx900_write_register(ViPipe, 0x65A4, 0x01); //
	imx900_write_register(ViPipe, 0x65B0, 0x00); //
	imx900_write_register(ViPipe, 0x65B4, 0x01); //
	imx900_write_register(ViPipe, 0x65B9, 0x00); //
	imx900_write_register(ViPipe, 0x65BD, 0x00); //
	imx900_write_register(ViPipe, 0x65C1, 0x00); //
	imx900_write_register(ViPipe, 0x65C9, 0x00); //
	imx900_write_register(ViPipe, 0x65CC, 0x00); //
	imx900_write_register(ViPipe, 0x65D0, 0x00); //
	imx900_write_register(ViPipe, 0x65D4, 0x00); //
	imx900_write_register(ViPipe, 0x65DC, 0x00); //
	imx900_write_register(ViPipe, 0x3400, 0x09); //
	imx900_write_register(ViPipe, 0x3000, 0x00); // stream on

	imx900_default_reg_init(ViPipe);

	printf("ViPipe:%d,===IMX900 3M 70fps 12bit Mono Init OK!===\n", ViPipe);
}

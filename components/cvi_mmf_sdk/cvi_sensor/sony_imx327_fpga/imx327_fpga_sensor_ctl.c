#include <unistd.h>
#include <cvi_comm_video.h>
#include "cvi_sns_ctrl.h"
#include "imx327_fpga_cmos_ex.h"
#include "sensor_i2c.h"

static void imx327_fpga_wdr_720p30_2to1_init(VI_PIPE ViPipe);
static void imx327_fpga_linear_720p30_init(VI_PIPE ViPipe);

const CVI_U8 imx327_fpga_i2c_addr = 0x1A;
const CVI_U32 imx327_fpga_addr_byte = 2;
const CVI_U32 imx327_fpga_data_byte = 1;
int imx327_fpga_i2c_init(VI_PIPE ViPipe)
{
	return sensor_i2c_init(ViPipe, (CVI_U8)g_aunImx327_fpga_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx327_fpga_AddrInfo[ViPipe].s8I2cAddr);
}

int imx327_fpga_i2c_exit(VI_PIPE ViPipe)
{
	return sensor_i2c_exit(ViPipe, (CVI_U8)g_aunImx327_fpga_BusInfo[ViPipe].s8I2cDev);
}

static void delay_ms(int ms)
{
	usleep(ms * 1000);
}

int imx327_fpga_read_register(VI_PIPE ViPipe, int addr)
{
	return sensor_i2c_read(ViPipe, (CVI_U8)g_aunImx327_fpga_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx327_fpga_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							imx327_fpga_addr_byte, imx327_fpga_data_byte);
}

int imx327_fpga_write_register(VI_PIPE ViPipe, int addr, int data)
{
	return sensor_i2c_write(ViPipe, (CVI_U8)g_aunImx327_fpga_BusInfo[ViPipe].s8I2cDev,
							(CVI_U8)g_aunImx327_fpga_AddrInfo[ViPipe].s8I2cAddr, (CVI_U32)addr,
							imx327_fpga_addr_byte, (CVI_U32)data, imx327_fpga_data_byte);
}

void imx327_fpga_standby(VI_PIPE ViPipe)
{
	imx327_fpga_write_register(ViPipe, 0x3000, 0x01); /* STANDBY */
	imx327_fpga_write_register(ViPipe, 0x3002, 0x01); /* XTMSTA */
}

void imx327_fpga_restart(VI_PIPE ViPipe)
{
	imx327_fpga_write_register(ViPipe, 0x3000, 0x00); /* standby */
	delay_ms(20);
	imx327_fpga_write_register(ViPipe, 0x3002, 0x00); /* master mode start */
	imx327_fpga_write_register(ViPipe, 0x304b, 0x0a);
}

void imx327_fpga_default_reg_init(VI_PIPE ViPipe)
{
	CVI_U32 i;

	for (i = 0; i < g_pastImx327_fpga[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum; i++) {
		imx327_fpga_write_register(ViPipe,
				g_pastImx327_fpga[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
				g_pastImx327_fpga[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
	}
}

void imx327_fpga_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
	CVI_U8 val = imx327_fpga_read_register(ViPipe, 0x3007) & ~0x3;

	switch (eSnsMirrorFlip) {
	case ISP_SNS_NORMAL:
		break;
	case ISP_SNS_MIRROR:
		val |= 0x2;
		break;
	case ISP_SNS_FLIP:
		val |= 0x1;
		break;
	case ISP_SNS_MIRROR_FLIP:
		val |= 0x3;
		break;
	default:
		return;
	}

	imx327_fpga_write_register(ViPipe, 0x3007, val);
}

void imx327_fpga_init(VI_PIPE ViPipe)
{
	WDR_MODE_E       enWDRMode;

	enWDRMode   = g_pastImx327_fpga[ViPipe]->enWDRMode;

	imx327_fpga_i2c_init(ViPipe);

	if (enWDRMode == WDR_MODE_2To1_LINE)
		imx327_fpga_wdr_720p30_2to1_init(ViPipe);
	else
		imx327_fpga_linear_720p30_init(ViPipe);

	g_pastImx327_fpga[ViPipe]->bInit = CVI_TRUE;
}

static void imx327_fpga_linear_720p30_init(VI_PIPE ViPipe)
{
	imx327_fpga_write_register(ViPipe, 0x3003, 0x01); /* SW RESET */
	delay_ms(4);
	imx327_fpga_write_register(ViPipe, 0x3000, 0x01); //STANDBY
	imx327_fpga_write_register(ViPipe, 0x3001, 0x00); //HOLD
	imx327_fpga_write_register(ViPipe, 0x3002, 0x00); //XMSTA
	imx327_fpga_write_register(ViPipe, 0x3005, 0x00); //ADBIT 10bit
//	imx327_fpga_write_register(ViPipe, 0x3007, 0x10); //VREVERS, ...
	imx327_fpga_write_register(ViPipe, 0x3007, 0x11); //V VREVERS, ...
	imx327_fpga_write_register(ViPipe, 0x3009, 0x02); //
	imx327_fpga_write_register(ViPipe, 0x300A, 0x3C); //BLKLEVEL
	//imx327_fpga_write_register(ViPipe, 0x300C, 0x00); //WDMODE [0] 0:Normal, 1:DOL, WDSEL [5:4] 1:DOL 2 frames
	imx327_fpga_write_register(ViPipe, 0x3011, 0x02);
//	imx327_fpga_write_register(ViPipe, 0x3014, 0x16); //GAIN, 0x2A=>12.6dB TBD
	imx327_fpga_write_register(ViPipe, 0x3014, 0x00); //GAIN, 0x0=>0 dB TBD
	//imx327_fpga_write_register(ViPipe, 0x3016, 0x08); //[TODO] sony's secrect register?
	imx327_fpga_write_register(ViPipe, 0x3018, 0xEE); //VMAX[7:0] TBD=750
	imx327_fpga_write_register(ViPipe, 0x3019, 0x02); //VMAX[15:8]
	imx327_fpga_write_register(ViPipe, 0x301A, 0x00); //VMAX[17:16]:=:0x301A[1:0]
//	imx327_fpga_write_register(ViPipe, 0x301C, 0xC8); //HMAX[7:0], TBD=6600
//	imx327_fpga_write_register(ViPipe, 0x301D, 0x19); //HMAX[15:8]
	imx327_fpga_write_register(ViPipe, 0x301C, 0xB0); //HMAX[7:0], TBD=19800
	imx327_fpga_write_register(ViPipe, 0x301D, 0x9A); //HMAX[15:8]
//	imx327_fpga_write_register(ViPipe, 0x3020, 0x8C); //SHS[7:0], TBD
//	imx327_fpga_write_register(ViPipe, 0x3021, 0x01); //SHS[15:8]
//	imx327_fpga_write_register(ViPipe, 0x3022, 0x00); //SHS[19:16]
	imx327_fpga_write_register(ViPipe, 0x3020, 0x00); //SHS[7:0], TBD
	imx327_fpga_write_register(ViPipe, 0x3021, 0x01); //SHS[15:8]
	imx327_fpga_write_register(ViPipe, 0x3022, 0x00); //SHS[19:16]
	//imx327_fpga_write_register(ViPipe, 0x3024, 0x00); //SHS2[7:0], TBD
	//imx327_fpga_write_register(ViPipe, 0x3025, 0x00); //SHS2[15:8]
	//imx327_fpga_write_register(ViPipe, 0x3026, 0x00); //SHS2[19:16]
	//imx327_fpga_write_register(ViPipe, 0x3030, 0x00); //RHS1[7:0], TBD
	//imx327_fpga_write_register(ViPipe, 0x3031, 0x00); //RHS1[15:8]
	//imx327_fpga_write_register(ViPipe, 0x3032, 0x00); //RHS1[19:16]
	//imx327_fpga_write_register(ViPipe, 0x3045, 0x00); //DOLSCDEN [0] 1: pattern1 0: pattern2
	//DOLSYDINFOEN[1] 1: embed the id code into 4th of sync
	//code. 0: disable
	//HINFOEN [2] 1: insert id code after 4th sync code 0: disable
	imx327_fpga_write_register(ViPipe, 0x3046, 0x00);
	imx327_fpga_write_register(ViPipe, 0x304B, 0x0A);
	imx327_fpga_write_register(ViPipe, 0x305C, 0x20); //INCKSEL1
	imx327_fpga_write_register(ViPipe, 0x305D, 0x00); //INCKSEL2
	imx327_fpga_write_register(ViPipe, 0x305E, 0x20); //INCKSEL3
	imx327_fpga_write_register(ViPipe, 0x305F, 0x01); //INCKSEL4
	imx327_fpga_write_register(ViPipe, 0x309E, 0x4A);
	imx327_fpga_write_register(ViPipe, 0x309F, 0x4A);
	imx327_fpga_write_register(ViPipe, 0x30D2, 0x19);
	imx327_fpga_write_register(ViPipe, 0x30D7, 0x03);
	//imx327_fpga_write_register(ViPipe, 0x3106, 0x00); //DOLHBFIXEN[7] 0: pattern1 1: pattern2
	imx327_fpga_write_register(ViPipe, 0x3129, 0x1D);
	imx327_fpga_write_register(ViPipe, 0x313B, 0x61);
	imx327_fpga_write_register(ViPipe, 0x315E, 0x1A);
	imx327_fpga_write_register(ViPipe, 0x3164, 0x1A);
	imx327_fpga_write_register(ViPipe, 0x317C, 0x12);
	imx327_fpga_write_register(ViPipe, 0x31EC, 0x37);
	imx327_fpga_write_register(ViPipe, 0x3405, 0x10); //Repetition
	imx327_fpga_write_register(ViPipe, 0x3407, 0x01); //physical_lane_nl
	imx327_fpga_write_register(ViPipe, 0x3414, 0x04); //opb_size_v
	imx327_fpga_write_register(ViPipe, 0x3418, 0xD9); //y_out_size
	imx327_fpga_write_register(ViPipe, 0x3419, 0x02); //y_out_size
	imx327_fpga_write_register(ViPipe, 0x3441, 0x0A); //csi_dt_fmt
	imx327_fpga_write_register(ViPipe, 0x3442, 0x0A); //csi_dt_fmt
	imx327_fpga_write_register(ViPipe, 0x3443, 0x01); //csi_lane_mode
	imx327_fpga_write_register(ViPipe, 0x3444, 0x20); //extck_freq
	imx327_fpga_write_register(ViPipe, 0x3445, 0x25); //extck_freq
	imx327_fpga_write_register(ViPipe, 0x3446, 0x4F); //tclkpost
	imx327_fpga_write_register(ViPipe, 0x3447, 0x00); //tclkpost
	imx327_fpga_write_register(ViPipe, 0x3448, 0x80); //thszero
	imx327_fpga_write_register(ViPipe, 0x3449, 0x00); //thszero
	imx327_fpga_write_register(ViPipe, 0x344A, 0x17); //thsprepare
	imx327_fpga_write_register(ViPipe, 0x344B, 0x00); //thsprepare
	imx327_fpga_write_register(ViPipe, 0x344C, 0x17); //tclktrail
	imx327_fpga_write_register(ViPipe, 0x344D, 0x00); //tclktrail
	imx327_fpga_write_register(ViPipe, 0x344E, 0x80); //thstrail
	imx327_fpga_write_register(ViPipe, 0x344F, 0x00); //thstrail
	imx327_fpga_write_register(ViPipe, 0x3450, 0x57); //tclkzero
	imx327_fpga_write_register(ViPipe, 0x3451, 0x00); //tclkzero
	imx327_fpga_write_register(ViPipe, 0x3452, 0x17); //tclkprepare
	imx327_fpga_write_register(ViPipe, 0x3453, 0x00); //tckkprepare
	imx327_fpga_write_register(ViPipe, 0x3454, 0x17); //tlpx
	imx327_fpga_write_register(ViPipe, 0x3455, 0x00); //tlpx
	imx327_fpga_write_register(ViPipe, 0x3472, 0x1C); //x_out_size
	imx327_fpga_write_register(ViPipe, 0x3473, 0x05); //x_out_size
	imx327_fpga_write_register(ViPipe, 0x3480, 0x49); //incksel7

	imx327_fpga_default_reg_init(ViPipe);

	imx327_fpga_write_register(ViPipe, 0x3000, 0x00); /* standby */
	delay_ms(20);
	imx327_fpga_write_register(ViPipe, 0x3002, 0x00); /* master mode start */
	imx327_fpga_write_register(ViPipe, 0x304b, 0x0a);

	printf("ViPipe:%d,===IMX327 720P 30fps 10bit LINE Init for FPGA OK!===\n", ViPipe);
}

static void imx327_fpga_wdr_720p30_2to1_init(VI_PIPE ViPipe)
{
	imx327_fpga_write_register(ViPipe, 0x3003, 0x01); //SW Reset
	delay_ms(3);
	imx327_fpga_write_register(ViPipe, 0x3000, 0x01); //STANDBY
	imx327_fpga_write_register(ViPipe, 0x3001, 0x00); //HOLD
	imx327_fpga_write_register(ViPipe, 0x3002, 0x00); //XMSTA
	imx327_fpga_write_register(ViPipe, 0x3005, 0x00); //ADBIT=10bit
	imx327_fpga_write_register(ViPipe, 0x3007, 0x11); //VREVERS, ...
	imx327_fpga_write_register(ViPipe, 0x3009, 0x02); //
	imx327_fpga_write_register(ViPipe, 0x300A, 0x3C); //BLKLEVEL
	imx327_fpga_write_register(ViPipe, 0x300C, 0x11); //WDMODE [0] 0:Normal, 1:DOL, WDSEL [5:4] 1:DOL 2 frames
	imx327_fpga_write_register(ViPipe, 0x3011, 0x02);
	imx327_fpga_write_register(ViPipe, 0x3014, 0x16); //GAIN, 0x2A=>12.6dB TBD
	//imx327_fpga_write_register(ViPipe, 0x3016, 0x08); //[TODO] sony's secrect register?
	imx327_fpga_write_register(ViPipe, 0x3018, 0xEE); //VMAX[7:0]
	imx327_fpga_write_register(ViPipe, 0x3019, 0x02); //VMAX[15:8]
	imx327_fpga_write_register(ViPipe, 0x301A, 0x00); //VMAX[17:16]:=:0x301A[1:0]
	//imx327_fpga_write_register(ViPipe, 0x301C, 0xC8); //HMAX[7:0], TBD=6600
	//imx327_fpga_write_register(ViPipe, 0x301D, 0x19); //HMAX[15:8]
	imx327_fpga_write_register(ViPipe, 0x301C, 0x58); //HMAX[7:0], TBD=19800
	imx327_fpga_write_register(ViPipe, 0x301D, 0x4D); //HMAX[15:8]
	imx327_fpga_write_register(ViPipe, 0x3020, 0x02); //SHS[7:0], TBD
	imx327_fpga_write_register(ViPipe, 0x3021, 0x00); //SHS[15:8]
	imx327_fpga_write_register(ViPipe, 0x3022, 0x00); //SHS[19:16]
	imx327_fpga_write_register(ViPipe, 0x3024, 0x1B); //SHS2[7:0], TBD
	imx327_fpga_write_register(ViPipe, 0x3025, 0x05); //SHS2[15:8]
	imx327_fpga_write_register(ViPipe, 0x3026, 0x00); //SHS2[19:16]
	imx327_fpga_write_register(ViPipe, 0x3030, 0x09); //RHS1[7:0], TBD
	imx327_fpga_write_register(ViPipe, 0x3031, 0x00); //RHS1[15:8]
	imx327_fpga_write_register(ViPipe, 0x3032, 0x00); //RHS1[19:16]
	imx327_fpga_write_register(ViPipe, 0x3045, 0x05); //DOLSCDEN [0] 1: pattern1 0: pattern2
	//DOLSYDINFOEN[1] 1: embed the id code into 4th of sync
	//code. 0: disable
	//HINFOEN [2] 1: insert id code after 4th sync code 0: disable
	imx327_fpga_write_register(ViPipe, 0x3046, 0x00);
	imx327_fpga_write_register(ViPipe, 0x304B, 0x0A);
	imx327_fpga_write_register(ViPipe, 0x305C, 0x20); //INCKSEL1
	imx327_fpga_write_register(ViPipe, 0x305D, 0x00); //INCKSEL2
	imx327_fpga_write_register(ViPipe, 0x305E, 0x20); //INCKSEL3
	imx327_fpga_write_register(ViPipe, 0x305F, 0x01); //INCKSEL4
	imx327_fpga_write_register(ViPipe, 0x309E, 0x4A);
	imx327_fpga_write_register(ViPipe, 0x309F, 0x4A);
	imx327_fpga_write_register(ViPipe, 0x30D2, 0x19);
	imx327_fpga_write_register(ViPipe, 0x30D7, 0x03);
	imx327_fpga_write_register(ViPipe, 0x3106, 0x11); //DOLHBFIXEN[7] 0: pattern1 1: pattern2
	imx327_fpga_write_register(ViPipe, 0x3129, 0x1D);
	imx327_fpga_write_register(ViPipe, 0x313B, 0x61);
	imx327_fpga_write_register(ViPipe, 0x315E, 0x1A);
	imx327_fpga_write_register(ViPipe, 0x3164, 0x1A);
	imx327_fpga_write_register(ViPipe, 0x317C, 0x12);
	imx327_fpga_write_register(ViPipe, 0x31EC, 0x37);
	imx327_fpga_write_register(ViPipe, 0x3405, 0x10); //Repetition
	imx327_fpga_write_register(ViPipe, 0x3407, 0x01); //physical_lane_nl
	imx327_fpga_write_register(ViPipe, 0x3414, 0x04); //opb_size_v
	imx327_fpga_write_register(ViPipe, 0x3415, 0x00); //NULL0_SIZE_V, set to 00h when DOL
	imx327_fpga_write_register(ViPipe, 0x3418, 0xC6); //y_out_size
	imx327_fpga_write_register(ViPipe, 0x3419, 0x05); //y_out_size
	imx327_fpga_write_register(ViPipe, 0x3441, 0x0A); //csi_dt_fmt
	imx327_fpga_write_register(ViPipe, 0x3442, 0x0A); //csi_dt_fmt
	imx327_fpga_write_register(ViPipe, 0x3443, 0x01); //csi_lane_mode
	imx327_fpga_write_register(ViPipe, 0x3444, 0x20); //extck_freq
	imx327_fpga_write_register(ViPipe, 0x3445, 0x25); //extck_freq
	imx327_fpga_write_register(ViPipe, 0x3446, 0x4F); //tclkpost
	imx327_fpga_write_register(ViPipe, 0x3447, 0x00); //tclkpost
	imx327_fpga_write_register(ViPipe, 0x3448, 0x80); //thszero
	imx327_fpga_write_register(ViPipe, 0x3449, 0x00); //thszero
	imx327_fpga_write_register(ViPipe, 0x344A, 0x17); //thsprepare
	imx327_fpga_write_register(ViPipe, 0x344B, 0x00); //thsprepare
	imx327_fpga_write_register(ViPipe, 0x344C, 0x17); //tclktrail
	imx327_fpga_write_register(ViPipe, 0x344D, 0x00); //tclktrail
	imx327_fpga_write_register(ViPipe, 0x344E, 0x80); //thstrail
	imx327_fpga_write_register(ViPipe, 0x344F, 0x00); //thstrail
	imx327_fpga_write_register(ViPipe, 0x3450, 0x57); //tclkzero
	imx327_fpga_write_register(ViPipe, 0x3451, 0x00); //tclkzero
	imx327_fpga_write_register(ViPipe, 0x3452, 0x17); //tclkprepare
	imx327_fpga_write_register(ViPipe, 0x3453, 0x00); //tckkprepare
	imx327_fpga_write_register(ViPipe, 0x3454, 0x17); //tlpx
	imx327_fpga_write_register(ViPipe, 0x3455, 0x00); //tlpx
	imx327_fpga_write_register(ViPipe, 0x3472, 0x20); //x_out_size
	imx327_fpga_write_register(ViPipe, 0x3473, 0x05); //x_out_size
	imx327_fpga_write_register(ViPipe, 0x347B, 0x23); //
	imx327_fpga_write_register(ViPipe, 0x3480, 0x49); //incksel7

	imx327_fpga_default_reg_init(ViPipe);
	imx327_fpga_write_register(ViPipe, 0x3000, 0x00); //STANDBY
	delay_ms(20); //delay
	imx327_fpga_write_register(ViPipe, 0x3002, 0x00); /* master mode start */
	imx327_fpga_write_register(ViPipe, 0x304b, 0x0a);

	printf("===Imx327 sensor 720P15fps 10bit 2to1 WDR(30fps->15fps) init success!=====\n");
}

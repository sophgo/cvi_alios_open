#include "cvi_sns_ctrl.h"
#include "cvi_comm_video.h"
#include "cvi_sns_ctrl.h"
#include "drv/common.h"
#include "sensor_i2c.h"
#include <unistd.h>
#include "ov9732_cmos_ex.h"

static void ov9732_linear_720p30_init(VI_PIPE ViPipe);

CVI_U8 ov9732_i2c_addr = 0x36; /* I2C Address of OV9732 */
const CVI_U32 ov9732_addr_byte = 2;
const CVI_U32 ov9732_data_byte = 1;

#define OV9732_CHIP_ID_ADDR_H 0x300A
#define OV9732_CHIP_ID_ADDR_L 0x300B
#define OV9732_CHIP_ID 0x9732

int ov9732_i2c_init(VI_PIPE ViPipe)
{
        CVI_U8 i2c_id = (CVI_U8)g_aunOv9732_BusInfo[ViPipe].s8I2cDev;

        return sensor_i2c_init(i2c_id);
}

int ov9732_i2c_exit(VI_PIPE ViPipe)
{
        CVI_U8 i2c_id = (CVI_U8)g_aunOv9732_BusInfo[ViPipe].s8I2cDev;

        return sensor_i2c_exit(i2c_id);
}

int ov9732_read_register(VI_PIPE ViPipe, int addr)
{
        CVI_U8 i2c_id = (CVI_U8)g_aunOv9732_BusInfo[ViPipe].s8I2cDev;

        return sensor_i2c_read(i2c_id, ov9732_i2c_addr, (CVI_U32)addr, ov9732_addr_byte, ov9732_data_byte);
}

int ov9732_write_register(VI_PIPE ViPipe, int addr, int data)
{
        CVI_U8 i2c_id = (CVI_U8)g_aunOv9732_BusInfo[ViPipe].s8I2cDev;

        return sensor_i2c_write(i2c_id, ov9732_i2c_addr, (CVI_U32)addr, ov9732_addr_byte,
                                (CVI_U32)data, ov9732_data_byte);
}

static void delay_ms(int ms)
{
        usleep(ms * 1000);
}

void ov9732_standby(VI_PIPE ViPipe)
{
        ov9732_write_register(ViPipe, 0x0100, 0x00); /* STANDBY */
}

void ov9732_restart(VI_PIPE ViPipe)
{
        ov9732_write_register(ViPipe, 0x0100, 0x01); /* STREAMING */
}

void ov9732_default_reg_init(VI_PIPE ViPipe)
{
        CVI_U32 i;
        CVI_U32 start = 1;
        CVI_U32 end = g_pastOv9732[ViPipe]->astSyncInfo[0].snsCfg.u32RegNum - 3;

        for (i = start; i < end; i++)
        {
                ov9732_write_register(ViPipe,
                                      g_pastOv9732[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32RegAddr,
                                      g_pastOv9732[ViPipe]->astSyncInfo[0].snsCfg.astI2cData[i].u32Data);
        }
}

#define OV9732_MIRROR_FLIP 0x3820
void ov9732_mirror_flip(VI_PIPE ViPipe, ISP_SNS_MIRRORFLIP_TYPE_E eSnsMirrorFlip)
{
        CVI_U8 value;

        value = ov9732_read_register(ViPipe, OV9732_MIRROR_FLIP);
        value &= ~(0x3 << 2);
        switch (eSnsMirrorFlip)
        {
        case ISP_SNS_NORMAL:
                value |= 0x00 << 2;
                break;
        case ISP_SNS_MIRROR:
                value |= 0x02 << 2;
                break;
        case ISP_SNS_FLIP:
                value |= 0x01 << 2;
                break;
        case ISP_SNS_MIRROR_FLIP:
                value |= 0x03 << 2;
                break;
        default:
                return;
        }
        ov9732_write_register(ViPipe, OV9732_MIRROR_FLIP, value);
}

int ov9732_probe(VI_PIPE ViPipe)
{
        int nVal, nVal2;

        usleep(5);
        if (ov9732_i2c_init(ViPipe) != CVI_SUCCESS)
                return CVI_FAILURE;
        nVal = ov9732_read_register(ViPipe, OV9732_CHIP_ID_ADDR_H);
           usleep(5);
        nVal2 = ov9732_read_register(ViPipe, OV9732_CHIP_ID_ADDR_L);
        if (nVal < 0 || nVal2 < 0)
        {
                CVI_TRACE_SNS(CVI_DBG_ERR, "read sensor id error.\n");
                return nVal;
        }

        if ((((nVal & 0xFF) << 8) | (nVal2 & 0xFF)) != OV9732_CHIP_ID)
        {
                CVI_TRACE_SNS(CVI_DBG_ERR, "Sensor ID Mismatch! Use the wrong sensor??\n");
                return CVI_FAILURE;
        }
        return CVI_SUCCESS;
}

void ov9732_init(VI_PIPE ViPipe)
{
        ov9732_i2c_init(ViPipe);

        delay_ms(10);

        ov9732_linear_720p30_init(ViPipe);

        g_pastOv9732[ViPipe]->bInit = CVI_TRUE;
}

void ov9732_exit(VI_PIPE ViPipe)
{
        ov9732_i2c_exit(ViPipe);
}

/* 720P30 */
static void ov9732_linear_720p30_init(VI_PIPE ViPipe)
{
        ov9732_write_register(ViPipe, 0x0103, 0x01);
        ov9732_write_register(ViPipe, 0x0100, 0x00);
        ov9732_write_register(ViPipe, 0x3001, 0x00);
        ov9732_write_register(ViPipe, 0x3002, 0x00);
        ov9732_write_register(ViPipe, 0x3007, 0x1f);
        ov9732_write_register(ViPipe, 0x3008, 0xff);
        ov9732_write_register(ViPipe, 0x3009, 0x02);
        ov9732_write_register(ViPipe, 0x3010, 0x00);
        ov9732_write_register(ViPipe, 0x3011, 0x08);
        ov9732_write_register(ViPipe, 0x3014, 0x22);
        ov9732_write_register(ViPipe, 0x301e, 0x15);
        ov9732_write_register(ViPipe, 0x3030, 0x19);
        ov9732_write_register(ViPipe, 0x3080, 0x02);
        ov9732_write_register(ViPipe, 0x3081, 0x3c);
        ov9732_write_register(ViPipe, 0x3082, 0x04);
        ov9732_write_register(ViPipe, 0x3083, 0x00);
        ov9732_write_register(ViPipe, 0x3084, 0x02);
        ov9732_write_register(ViPipe, 0x3085, 0x01);
        ov9732_write_register(ViPipe, 0x3086, 0x01);
        ov9732_write_register(ViPipe, 0x3089, 0x01);
        ov9732_write_register(ViPipe, 0x308a, 0x00);
        ov9732_write_register(ViPipe, 0x3103, 0x01);
        ov9732_write_register(ViPipe, 0x3600, 0xf6);
        ov9732_write_register(ViPipe, 0x3601, 0x72);
        ov9732_write_register(ViPipe, 0x3605, 0x66);
        ov9732_write_register(ViPipe, 0x3610, 0x0c);
        ov9732_write_register(ViPipe, 0x3611, 0x60);
        ov9732_write_register(ViPipe, 0x3612, 0x35);
        ov9732_write_register(ViPipe, 0x3654, 0x10);
        ov9732_write_register(ViPipe, 0x3655, 0x77);
        ov9732_write_register(ViPipe, 0x3656, 0x77);
        ov9732_write_register(ViPipe, 0x3657, 0x07);
        ov9732_write_register(ViPipe, 0x3658, 0x22);
        ov9732_write_register(ViPipe, 0x3659, 0x22);
        ov9732_write_register(ViPipe, 0x365a, 0x02);
        ov9732_write_register(ViPipe, 0x3700, 0x1f);
        ov9732_write_register(ViPipe, 0x3701, 0x10);
        ov9732_write_register(ViPipe, 0x3702, 0x0c);
        ov9732_write_register(ViPipe, 0x3703, 0x0b);
        ov9732_write_register(ViPipe, 0x3704, 0x3c);
        ov9732_write_register(ViPipe, 0x3705, 0x51);
        ov9732_write_register(ViPipe, 0x370d, 0x20);
        ov9732_write_register(ViPipe, 0x3710, 0x0d);
        ov9732_write_register(ViPipe, 0x3782, 0x58);
        ov9732_write_register(ViPipe, 0x3783, 0x60);
        ov9732_write_register(ViPipe, 0x3784, 0x05);
        ov9732_write_register(ViPipe, 0x3785, 0x55);
        ov9732_write_register(ViPipe, 0x37c0, 0x07);
        ov9732_write_register(ViPipe, 0x3800, 0x00);
        ov9732_write_register(ViPipe, 0x3801, 0x04);
        ov9732_write_register(ViPipe, 0x3802, 0x00);
        ov9732_write_register(ViPipe, 0x3803, 0x04);
        ov9732_write_register(ViPipe, 0x3804, 0x05);
        ov9732_write_register(ViPipe, 0x3805, 0x0b);
        ov9732_write_register(ViPipe, 0x3806, 0x02);
        ov9732_write_register(ViPipe, 0x3807, 0xdb);
        ov9732_write_register(ViPipe, 0x3808, 0x05);
        ov9732_write_register(ViPipe, 0x3809, 0x00);
        ov9732_write_register(ViPipe, 0x380a, 0x02);
        ov9732_write_register(ViPipe, 0x380b, 0xd0);
        ov9732_write_register(ViPipe, 0x380c, 0x05);
        ov9732_write_register(ViPipe, 0x380d, 0xdc);
        ov9732_write_register(ViPipe, 0x380e, 0x03);
        ov9732_write_register(ViPipe, 0x380f, 0x20);
        ov9732_write_register(ViPipe, 0x3810, 0x00);
        ov9732_write_register(ViPipe, 0x3811, 0x04);
        ov9732_write_register(ViPipe, 0x3812, 0x00);
        ov9732_write_register(ViPipe, 0x3813, 0x04);
        ov9732_write_register(ViPipe, 0x3816, 0x00);
        ov9732_write_register(ViPipe, 0x3817, 0x00);
        ov9732_write_register(ViPipe, 0x3818, 0x00);
        ov9732_write_register(ViPipe, 0x3819, 0x04);
        ov9732_write_register(ViPipe, 0x3820, 0x10);
        ov9732_write_register(ViPipe, 0x3821, 0x00);
        ov9732_write_register(ViPipe, 0x382c, 0x06);
        ov9732_write_register(ViPipe, 0x3500, 0x00);
        ov9732_write_register(ViPipe, 0x3501, 0x31);
        ov9732_write_register(ViPipe, 0x3502, 0x00);
        ov9732_write_register(ViPipe, 0x3503, 0x03);
        ov9732_write_register(ViPipe, 0x3504, 0x00);
        ov9732_write_register(ViPipe, 0x3505, 0x00);
        ov9732_write_register(ViPipe, 0x3509, 0x10);
        ov9732_write_register(ViPipe, 0x350a, 0x00);
        ov9732_write_register(ViPipe, 0x350b, 0x40);
        ov9732_write_register(ViPipe, 0x3d00, 0x00);
        ov9732_write_register(ViPipe, 0x3d01, 0x00);
        ov9732_write_register(ViPipe, 0x3d02, 0x00);
        ov9732_write_register(ViPipe, 0x3d03, 0x00);
        ov9732_write_register(ViPipe, 0x3d04, 0x00);
        ov9732_write_register(ViPipe, 0x3d05, 0x00);
        ov9732_write_register(ViPipe, 0x3d06, 0x00);
        ov9732_write_register(ViPipe, 0x3d07, 0x00);
        ov9732_write_register(ViPipe, 0x3d08, 0x00);
        ov9732_write_register(ViPipe, 0x3d09, 0x00);
        ov9732_write_register(ViPipe, 0x3d0a, 0x00);
        ov9732_write_register(ViPipe, 0x3d0b, 0x00);
        ov9732_write_register(ViPipe, 0x3d0c, 0x00);
        ov9732_write_register(ViPipe, 0x3d0d, 0x00);
        ov9732_write_register(ViPipe, 0x3d0e, 0x00);
        ov9732_write_register(ViPipe, 0x3d0f, 0x00);
        ov9732_write_register(ViPipe, 0x3d80, 0x00);
        ov9732_write_register(ViPipe, 0x3d81, 0x00);
        ov9732_write_register(ViPipe, 0x3d82, 0x38);
        ov9732_write_register(ViPipe, 0x3d83, 0xa4);
        ov9732_write_register(ViPipe, 0x3d84, 0x00);
        ov9732_write_register(ViPipe, 0x3d85, 0x00);
        ov9732_write_register(ViPipe, 0x3d86, 0x1f);
        ov9732_write_register(ViPipe, 0x3d87, 0x03);
        ov9732_write_register(ViPipe, 0x3d8b, 0x00);
        ov9732_write_register(ViPipe, 0x3d8f, 0x00);
        ov9732_write_register(ViPipe, 0x4001, 0xe0);
        ov9732_write_register(ViPipe, 0x4004, 0x00);
        ov9732_write_register(ViPipe, 0x4005, 0x02);
        ov9732_write_register(ViPipe, 0x4006, 0x01);
        ov9732_write_register(ViPipe, 0x4007, 0x40);
        ov9732_write_register(ViPipe, 0x4009, 0x0b);
        ov9732_write_register(ViPipe, 0x4300, 0x03);
        ov9732_write_register(ViPipe, 0x4301, 0xff);
        ov9732_write_register(ViPipe, 0x4304, 0x00);
        ov9732_write_register(ViPipe, 0x4305, 0x00);
        ov9732_write_register(ViPipe, 0x4309, 0x00);
        ov9732_write_register(ViPipe, 0x4600, 0x00);
        ov9732_write_register(ViPipe, 0x4601, 0x04);
        ov9732_write_register(ViPipe, 0x4800, 0x00);
        ov9732_write_register(ViPipe, 0x4805, 0x00);
        ov9732_write_register(ViPipe, 0x4821, 0x50);
        ov9732_write_register(ViPipe, 0x4823, 0x50);
        ov9732_write_register(ViPipe, 0x4837, 0x2d);
        ov9732_write_register(ViPipe, 0x4a00, 0x00);
        ov9732_write_register(ViPipe, 0x4f00, 0x80);
        ov9732_write_register(ViPipe, 0x4f01, 0x10);
        ov9732_write_register(ViPipe, 0x4f02, 0x00);
        ov9732_write_register(ViPipe, 0x4f03, 0x00);
        ov9732_write_register(ViPipe, 0x4f04, 0x00);
        ov9732_write_register(ViPipe, 0x4f05, 0x00);
        ov9732_write_register(ViPipe, 0x4f06, 0x00);
        ov9732_write_register(ViPipe, 0x4f07, 0x00);
        ov9732_write_register(ViPipe, 0x4f08, 0x00);
        ov9732_write_register(ViPipe, 0x4f09, 0x00);
        ov9732_write_register(ViPipe, 0x5000, 0x07);
        ov9732_write_register(ViPipe, 0x500c, 0x00);
        ov9732_write_register(ViPipe, 0x500d, 0x00);
        ov9732_write_register(ViPipe, 0x500e, 0x00);
        ov9732_write_register(ViPipe, 0x500f, 0x00);
        ov9732_write_register(ViPipe, 0x5010, 0x00);
        ov9732_write_register(ViPipe, 0x5011, 0x00);
        ov9732_write_register(ViPipe, 0x5012, 0x00);
        ov9732_write_register(ViPipe, 0x5013, 0x00);
        ov9732_write_register(ViPipe, 0x5014, 0x00);
        ov9732_write_register(ViPipe, 0x5015, 0x00);
        ov9732_write_register(ViPipe, 0x5016, 0x00);
        ov9732_write_register(ViPipe, 0x5017, 0x00);
        ov9732_write_register(ViPipe, 0x5080, 0x00);
        ov9732_write_register(ViPipe, 0x5180, 0x01);
        ov9732_write_register(ViPipe, 0x5181, 0x00);
        ov9732_write_register(ViPipe, 0x5182, 0x01);
        ov9732_write_register(ViPipe, 0x5183, 0x00);
        ov9732_write_register(ViPipe, 0x5184, 0x01);
        ov9732_write_register(ViPipe, 0x5185, 0x00);
        ov9732_write_register(ViPipe, 0x5708, 0x06);
        ov9732_write_register(ViPipe, 0x5781, 0x0e);
        ov9732_write_register(ViPipe, 0x5783, 0x0f);
        ov9732_write_register(ViPipe, 0x3603, 0x70);
        ov9732_write_register(ViPipe, 0x3620, 0x1e);
        ov9732_write_register(ViPipe, 0x400a, 0x01);
        ov9732_write_register(ViPipe, 0x400b, 0xc0);
        ov9732_write_register(ViPipe, 0x0100, 0x01);
        ov9732_default_reg_init(ViPipe);
        delay_ms(100);
        printf("ViPipe:%d,===OV9732 720P 30fps 10bit LINE Init OK!\n", ViPipe);
}

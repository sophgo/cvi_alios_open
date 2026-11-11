#include "motor_i2c.h"
#include "cn3927elc.h"

static uint8_t g_i2c_num;
static int32_t g_motor_pos = CN3927ELC_MIN_POS;

static int32_t nr_lsc_mode_init(uint8_t t_src)
{
    int ret = CSI_OK;
    uint8_t device_addr = CN3927ELC_IIC_ADDR;
    uint8_t byte1 = 0x00;
    uint8_t byte2 = 0x00;
    uint8_t i2c_num = g_i2c_num;
    /* Ringing setting ON */
    byte1 = 0xEC;
    byte2 = 0xA3;
    ret = motor_i2c_write(i2c_num, device_addr, byte1, IIC_MEM_ADDR_SIZE_8BIT, &byte2, 1);
    if(ret != CSI_OK){
        printf("motor_i2c_write err i2c_num:%d, reg:0x%x, val:0x%x\n", i2c_num, byte1, byte2);
    }
    /* EDLC/DLC and MCLK[1:0] setting */
    byte1 = 0xA1;
    byte2 = 0x05;
    ret = motor_i2c_write(i2c_num, device_addr, byte1, IIC_MEM_ADDR_SIZE_8BIT, &byte2, 1);
    if(ret != CSI_OK){
        printf("motor_i2c_write err i2c_num:%d, reg:0x%x, val:0x%x\n", i2c_num, byte1, byte2);
    }
    /* T_SRC[4:0] setting */
    byte1 = 0xF2;
    byte2 = (t_src << 3) & 0xF8;
    ret = motor_i2c_write(i2c_num, device_addr, byte1, IIC_MEM_ADDR_SIZE_8BIT, &byte2, 1);
    if(ret != CSI_OK){
        printf("motor_i2c_write err i2c_num:%d, reg:0x%x, val:0x%x\n", i2c_num, byte1, byte2);
    }
    /* Ringing setting OFF */
    byte1 = 0xDC;
    byte2 = 0x51;
    ret = motor_i2c_write(i2c_num, device_addr, byte1, IIC_MEM_ADDR_SIZE_8BIT, &byte2, 1);
    if(ret != CSI_OK){
        printf("motor_i2c_write err i2c_num:%d, reg:0x%x, val:0x%x\n", i2c_num, byte1, byte2);
    }
    return ret;
}

static int32_t nr_lsc_mode_set_step(uint32_t target_step, 
        uint8_t code_per_step, uint8_t step_period)
{
    int ret = CSI_OK;
    uint8_t device_addr = CN3927ELC_IIC_ADDR;
    uint8_t i2c_num = g_i2c_num;
    uint8_t byte1 = (target_step >> 4) & 0x3F;
    uint8_t byte2 = ((uint8_t)((target_step & 0x0F) << 4)) | ((code_per_step << 2) & 0x0C) | (step_period & 0x03);
    ret = motor_i2c_write(i2c_num, device_addr, byte1, IIC_MEM_ADDR_SIZE_8BIT, &byte2, 1);
    if(ret != CSI_OK){
        printf("motor_i2c_write err i2c_num:%d, reg:0x%x, val:0x%x\n", i2c_num, byte1, byte2);
    }
    return ret;
}

int32_t cn3927elc_init(uint8_t i2c_num)
{
    printf("motor init\n");
    g_motor_pos = 0;
    g_i2c_num = i2c_num;
    motor_i2c_init(i2c_num);
	nr_lsc_mode_init(0x0F);
    return CSI_OK;
}

int32_t cn3927elc_deinit(uint8_t i2c_num)
{
    printf("motor deinit\n");
    motor_i2c_exit(i2c_num);
    return CSI_OK;
}

int32_t cn3927elc_cw(uint32_t step)
{
    int32_t ret = CSI_OK;
    uint32_t target_step = 0;
    g_motor_pos += step;
    if(g_motor_pos < CN3927ELC_MIN_POS){
        g_motor_pos = CN3927ELC_MIN_POS;
    }else if(g_motor_pos > CN3927ELC_MAX_POS){
        g_motor_pos = CN3927ELC_MAX_POS;
    }else{
        g_motor_pos = g_motor_pos;
    }
    target_step = (g_motor_pos < 0? 0 : g_motor_pos);
	ret = nr_lsc_mode_set_step(target_step, 1, 0);
    return ret;
}

int32_t cn3927elc_ccw(uint32_t step){
    int32_t ret = CSI_OK;
    uint32_t target_step = 0;
    g_motor_pos -= step;
    if(g_motor_pos < CN3927ELC_MIN_POS){
        g_motor_pos = CN3927ELC_MIN_POS;
    }else if(g_motor_pos > CN3927ELC_MAX_POS){
        g_motor_pos = CN3927ELC_MAX_POS;
    }else{
        g_motor_pos = g_motor_pos;
    }
    target_step = (g_motor_pos < 0? 0 : g_motor_pos);
	ret = nr_lsc_mode_set_step(target_step, 1, 0);
    return ret;
}

int32_t cn3927elc_get_pos(uint32_t* pos){
    int32_t ret = CSI_OK; 
    *pos = g_motor_pos;
    return ret;
}


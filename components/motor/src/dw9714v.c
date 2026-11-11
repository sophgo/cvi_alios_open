#include "motor_i2c.h"
#include "dw9714v.h"

#define DW9714V_NR_MODE_ENABLE 1

static uint8_t g_i2c_num;
static int32_t g_motor_pos = DW9714V_MIN_POS;

#if DW9714V_NR_MODE_ENABLE
static int32_t nr_lsc_mode_init(uint8_t t_src)
{
    int ret = CSI_OK;
    uint8_t device_addr = DW9714V_IIC_ADDR;
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
    uint8_t device_addr = DW9714V_IIC_ADDR;
    uint8_t i2c_num = g_i2c_num;
    uint8_t byte1 = (target_step >> 4) & 0x3F;
    uint8_t byte2 = ((uint8_t)((target_step & 0x0F) << 4)) | ((code_per_step << 2) & 0x0C) | (step_period & 0x03);
    ret = motor_i2c_write(i2c_num, device_addr, byte1, IIC_MEM_ADDR_SIZE_8BIT, &byte2, 1);
    if(ret != CSI_OK){
        printf("motor_i2c_write err i2c_num:%d, reg:0x%x, val:0x%x\n", i2c_num, byte1, byte2);
    }
    return ret;
}
#else
static int32_t adv_sac_mode_init(void)
{
    int ret = CSI_OK;
    uint8_t i2c_num = g_i2c_num;
    uint8_t device_addr = DW9714V_IIC_ADDR;
    uint8_t bytes[2] = {0};
    /* enter advanced mode */
    bytes[0] = 0xED;
    bytes[1] = 0xAB;
    ret = motor_i2c_write(i2c_num, bytes[0], bytes[1], IIC_MEM_ADDR_SIZE_8BIT, bytes, 2);
    if(ret != CSI_OK){
        printf("motor_i2c_write err i2c_num:%d, reg:0x%x, val:0x%x\n", i2c_num, bytes[0], bytes[1]);
    }
    bytes[0] = 0x00;
    /* RING_EN:SAC */
    bytes[0] |= 0x80;
    /* NRC_INF[1:0]: */
    bytes[0] |= (0x01 << 5);
    /* NRC_TIME:64.22ms */
    bytes[0] |= (0x01 << 4);
    /* SAC_MODE[3:0]:SAC2 */
    bytes[0] &= 0xF0;
    ret = motor_i2c_write(i2c_num, device_addr, DW9714V_REG_SAC_CFG_ADDR, IIC_MEM_ADDR_SIZE_8BIT, &bytes[0], 1);
    if(ret != CSI_OK){
        printf("motor_i2c_write err i2c_num:%d, reg:0x%x, val:0x%x\n", i2c_num, device_addr, bytes[0]);
    }
    /* PRESC[1:0]:0x00 */
    bytes[0] = 0x00;
    ret = motor_i2c_write(i2c_num, device_addr, DW9714V_REG_PRESC_ADDR, IIC_MEM_ADDR_SIZE_8BIT, &bytes[0], 1);
    if(ret != CSI_OK){
        printf("motor_i2c_write err i2c_num:%d, reg:0x%x, val:0x%x\n", i2c_num, device_addr, bytes[0]);
    }
    /* SACT[6:0]: */
    bytes[0] = 0x08;
    ret = motor_i2c_write(i2c_num, device_addr, DW9714V_REG_SACT_ADDR, IIC_MEM_ADDR_SIZE_8BIT, &bytes[0], 1);
    if(ret != CSI_OK){
        printf("motor_i2c_write err i2c_num:%d, reg:0x%x, val:0x%x\n", i2c_num, device_addr, bytes[0]);
    }
    /* PRESET[6:0]: 350 = 1 0101 1110 */
    bytes[0] = 0x5E;
    ret = motor_i2c_write(i2c_num, device_addr, DW9714V_REG_PRESET_ADDR, IIC_MEM_ADDR_SIZE_8BIT, &bytes[0], 1);
    if(ret != CSI_OK){
        printf("motor_i2c_write err i2c_num:%d, reg:0x%x, val:0x%x\n", i2c_num, device_addr, bytes[0]);
    }
    return ret;
}

static int32_t adv_mode_set_step(uint32_t target_step)
{
    int ret = CSI_OK;
    uint8_t i2c_num = g_i2c_num;
    uint8_t device_addr = DW9714V_IIC_ADDR;
    uint8_t byte_lsb = target_step & 0xFF;
    uint8_t byte_msb = (uint8_t)((target_step >> 8) & 0x03);
    ret = motor_i2c_write(i2c_num, device_addr, DW9714V_REG_VCM_MSB_ADDR, IIC_MEM_ADDR_SIZE_8BIT, &byte_msb, 1);
    if(ret != CSI_OK){
        printf("motor_i2c_write err i2c_num:%d, reg:0x%x, val:0x%x\n", i2c_num, device_addr, byte_msb);
    }
    ret = motor_i2c_write(i2c_num, device_addr, DW9714V_REG_VCM_LSB_ADDR, IIC_MEM_ADDR_SIZE_8BIT, &byte_lsb, 1);
    if(ret != CSI_OK){
        printf("motor_i2c_write err i2c_num:%d, reg:0x%x, val:0x%x\n", i2c_num, device_addr, byte_lsb);
    }
    return ret;
}

static int32_t adv_nrc_enable(int is_start)
{
    int ret = CSI_OK;
    uint8_t i2c_num = g_i2c_num;
    uint8_t device_addr = DW9714V_IIC_ADDR;
    uint8_t byte = 0x02;
    if(is_start){
        byte &= 0xFE; /* start */
    }else{
        byte |= 0x01; /* landing */
    }
    ret = motor_i2c_write(i2c_num, device_addr, DW9714V_REG_NRC_ADDR, IIC_MEM_ADDR_SIZE_8BIT, &byte, 1);
    if(ret != CSI_OK){
        printf("motor_i2c_write err i2c_num:%d, reg:0x%x, val:0x%x\n", i2c_num, device_addr, byte);
    }
    return ret;
}
#if 0
static int32_t adv_nrc_disable(void){
    int ret = CSI_OK;
    uint8_t i2c_num = g_i2c_num;
    uint8_t device_addr = DW9714V_IIC_ADDR;
    uint8_t byte = 0x00;
    ret = motor_i2c_write(i2c_num, device_addr, DW9714V_REG_NRC_ADDR, IIC_MEM_ADDR_SIZE_8BIT, &byte, 1);
    if(ret != CSI_OK){
        printf("motor_i2c_write err i2c_num:%d, reg:0x%x, val:0x%x\n", i2c_num, device_addr, byte);
    }
    return ret;
}
#endif
#endif

int32_t dw9714v_init(uint8_t i2c_num)
{
    printf("motor init\n");
    g_motor_pos = 0;
    g_i2c_num = i2c_num;
    motor_i2c_init(i2c_num);
#if DW9714V_NR_MODE_ENABLE
    nr_lsc_mode_init(0x0F);
#else
    adv_sac_mode_init();
    /* nrc start */
    adv_nrc_enable(1);
#endif
    return CSI_OK;
}

int32_t dw9714v_deinit(uint8_t i2c_num)
{
    printf("motor deinit\n");
    motor_i2c_exit(i2c_num);
#if DW9714V_NR_MODE_ENABLE
    nr_lsc_mode_init(0x0F);
#else
    /* nrc landing */
    adv_nrc_enable(0);
#endif
    return CSI_OK;
}

int32_t dw9714v_cw(uint32_t step)
{
    int32_t ret = CSI_OK;
    uint32_t target_step = 0;
    g_motor_pos += step;
    if(g_motor_pos < DW9714V_MIN_POS){
        g_motor_pos = DW9714V_MIN_POS;
    }else if(g_motor_pos > DW9714V_MAX_POS){
        g_motor_pos = DW9714V_MAX_POS;
    }else{
        g_motor_pos = g_motor_pos;
    }
    target_step = (g_motor_pos < 0? 0 : g_motor_pos);
#if DW9714V_NR_MODE_ENABLE
    ret = nr_lsc_mode_set_step(target_step, 1, 0);
#else
    ret = adv_mode_set_step(target_step);
#endif
    return ret;
}

int32_t dw9714v_ccw(uint32_t step){
    int32_t ret = CSI_OK;
    uint32_t target_step = 0;
    g_motor_pos -= step;
    if(g_motor_pos < DW9714V_MIN_POS){
        g_motor_pos = DW9714V_MIN_POS;
    }else if(g_motor_pos > DW9714V_MAX_POS){
        g_motor_pos = DW9714V_MAX_POS;
    }else{
        g_motor_pos = g_motor_pos;
    }
    target_step = (g_motor_pos < 0? 0 : g_motor_pos);
#if DW9714V_NR_MODE_ENABLE
    ret = nr_lsc_mode_set_step(target_step, 1, 0);
#else
    ret = adv_mode_set_step(target_step);
#endif
    return ret;
}

int32_t dw9714v_get_pos(uint32_t* pos){
    int32_t ret = CSI_OK; 
    *pos = g_motor_pos;
    return ret;
}
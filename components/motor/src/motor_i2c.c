#include "motor_i2c.h"

#define IIC_MAX 5
#define MOTOR_I2C_DENUG 0

static csi_iic_t iic_adapter;

int32_t motor_i2c_init(uint8_t i2c_num)
{
    csi_iic_t *iic;

    if (i2c_num >= IIC_MAX) {
        return CSI_ERROR;
    }

    iic = &iic_adapter;

    if (csi_iic_init(iic, i2c_num)) {
        printf("iic-%d csi_iic_init error", i2c_num);
        return CSI_ERROR;
    }

    if (csi_iic_mode(iic, IIC_MODE_MASTER)) {
        printf("iic-%d csi_iic_mode error", i2c_num);
        goto INIT_FAIL;
    }

    if (csi_iic_addr_mode(iic, IIC_ADDRESS_7BIT)) {
        printf("iic-%d csi_iic_addr_mode error", i2c_num);
        goto INIT_FAIL;
    }

    if (csi_iic_speed(iic, IIC_BUS_SPEED_FAST)) {
        printf("iic-%d csi_iic_speed error", i2c_num);
        goto INIT_FAIL;
    }

    return CSI_OK;

INIT_FAIL:
    csi_iic_uninit(iic);
    return CSI_ERROR;
}

int32_t motor_i2c_exit(uint8_t i2c_num)
{
    csi_iic_t *iic;

    if (i2c_num >= IIC_MAX) {
        return CSI_ERROR;
    }

    iic = &iic_adapter;

    csi_iic_uninit(iic);

    return CSI_OK;
}

int32_t motor_i2c_read(uint8_t i2c_num, uint8_t device_addr,
            uint32_t reg_addr, csi_iic_mem_addr_size_t reg_addr_len,
            uint8_t *pdata, uint32_t data_len)
{
    int32_t ret = CSI_OK;
    uint32_t timeout = 100;
    csi_iic_t *iic;

    if (i2c_num >= IIC_MAX) {
        return CSI_ERROR;
    }

    iic = &iic_adapter;

    ret = csi_iic_mem_receive(iic, device_addr, reg_addr,
                    reg_addr_len, pdata, data_len, timeout);
    if (ret != data_len) {
        printf("I2C_READ error!\n");
        return CSI_ERROR;
    }
#if MOTOR_I2C_DENUG
    uint32_t i = 0;
    printf("i2c r 0x%x = ", reg_addr);
    for(i = 0; i < data_len; i++){
        printf("0x%2x ", pdata[i]);
    }
    printf("\n");
#endif
    return ret;
}

int32_t motor_i2c_write(uint8_t i2c_num, uint8_t device_addr,
            uint32_t reg_addr, csi_iic_mem_addr_size_t reg_addr_len,
            uint8_t *pdata, uint32_t data_len)
{
    int32_t ret = CSI_OK;
    uint32_t timeout = 10;
    csi_iic_t *iic;

    if (i2c_num >= IIC_MAX) {
        return CSI_ERROR;
    }

    iic = &iic_adapter;

    ret = csi_iic_mem_send(iic, device_addr, reg_addr, reg_addr_len, pdata, data_len, timeout);
    if (ret != data_len) {
        printf("I2C_WRITE error!\n");
        return CSI_ERROR;
    }
#if MOTOR_I2C_DENUG
    uint32_t i = 0;
    printf("i2c w 0x%x = ", reg_addr);
    for(i = 0; i < data_len; i++){
        printf("0x%2x ", pdata[i]);
    }
    printf("\n");
#endif
    return CSI_OK;
}
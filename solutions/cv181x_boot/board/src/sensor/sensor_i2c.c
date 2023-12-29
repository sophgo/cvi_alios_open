#include <drv/iic.h>
// #include "cvi_type.h"

#define IIC_MAX 5



static csi_iic_t iic_adapter_list[5] = {0};

int sensor_i2c_init(uint8_t i2c_id)
{
	csi_iic_t *iic;

	if (i2c_id >= IIC_MAX) {
		return -1;
	}

	iic = &iic_adapter_list[i2c_id];

	if (csi_iic_init(iic, i2c_id)) {
		printf("iic-%d csi_iic_init error", i2c_id);
		return CSI_ERROR;
	}

	if (csi_iic_mode(iic, IIC_MODE_MASTER)) {
		printf("iic-%d csi_iic_mode error", i2c_id);
		goto INIT_FAIL;
	}

	if (csi_iic_addr_mode(iic, IIC_ADDRESS_7BIT)) {
		printf("iic-%d csi_iic_addr_mode error", i2c_id);
		goto INIT_FAIL;
	}

	if (csi_iic_speed(iic, IIC_BUS_SPEED_FAST)) {
		printf("iic-%d csi_iic_speed error", i2c_id);
		goto INIT_FAIL;
	}

	return 0;

INIT_FAIL:
	csi_iic_uninit(iic);
	return -1;
}

int sensor_i2c_exit(uint8_t i2c_id)
{
	csi_iic_t *iic;

	if (i2c_id >= IIC_MAX) {
		return -1;
	}

	iic = &iic_adapter_list[i2c_id];

	csi_iic_uninit(iic);

	return 0;
}

int sensor_i2c_read(uint8_t i2c_id, uint8_t snsr_i2c_addr,
			uint32_t addr,uint32_t snsr_addr_byte,
			uint32_t snsr_data_byte)
{
	int ret = 0;
	uint8_t buf[8];
	uint8_t idx = 0;
	uint32_t data = 0;
	uint32_t timeout = 1;
	csi_iic_t *iic;
	csi_iic_mem_addr_size_t reg_addr_len;

	if (i2c_id >= IIC_MAX) {
		return -1;
	}

	iic = &iic_adapter_list[i2c_id];

	switch (snsr_addr_byte) {
	default:
	case 0x1:
		reg_addr_len = IIC_MEM_ADDR_SIZE_8BIT;
		break;
	case 0x2:
		reg_addr_len = IIC_MEM_ADDR_SIZE_16BIT;
		break;
	}

	ret = csi_iic_mem_receive(iic, snsr_i2c_addr, addr,
					reg_addr_len, buf, snsr_data_byte, timeout);
	if (ret != snsr_data_byte) {
		printf("I2C_READ error!\n");
		return -1;
	}

	if (snsr_data_byte == 2) {
		data = buf[idx++] << 8;
		data += buf[idx++];
	} else if (snsr_data_byte == 1) {
		data = buf[idx++];
	}
	// printf("i2c r 0x%x = 0x%02x\n", addr, data);
	return data;
}

int sensor_i2c_write(uint8_t i2c_id, uint8_t snsr_i2c_addr,
			uint32_t addr, uint32_t snsr_addr_byte,
			uint32_t data, uint32_t snsr_data_byte)
{
	int ret = 0;
	uint8_t buf[8];
	uint8_t idx = 0;
	uint32_t timeout = 1;
	csi_iic_t *iic;
	csi_iic_mem_addr_size_t reg_addr_len;

	if (i2c_id >= IIC_MAX) {
		return -1;
	}

	iic = &iic_adapter_list[i2c_id];

	switch (snsr_addr_byte) {
	default:
	case 0x1:
		reg_addr_len = IIC_MEM_ADDR_SIZE_8BIT;
		break;
	case 0x2:
		reg_addr_len = IIC_MEM_ADDR_SIZE_16BIT;
		break;
	}

	if (snsr_data_byte == 2) {
		buf[idx] = (data >> 8) & 0xff;
		idx++;
		buf[idx] = data & 0xff;
		idx++;
	} else if (snsr_data_byte == 1) {
		buf[idx] = data & 0xff;
		idx++;
	}

	ret = csi_iic_mem_send(iic, snsr_i2c_addr, addr, reg_addr_len, buf, snsr_data_byte, timeout);
	if (ret != snsr_data_byte) {
		printf("I2C_WRITE error!\n");
		return -1;
	}
	// printf("i2c w 0x%x = 0x%2x\n", addr, data);
	return 0;
}


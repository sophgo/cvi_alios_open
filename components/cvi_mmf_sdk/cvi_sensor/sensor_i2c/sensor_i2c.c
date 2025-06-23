#include "sensor_i2c.h"

#if defined(CONFIG_DUAL_OS)
static csi_iic_t iic_adapter_list[IIC_MAX] = {0};

int sensor_i2c_init(CVI_S32 ViPipe, CVI_U8 i2c_id, CVI_U8 snsr_i2c_addr)
{
	csi_iic_t *iic;

	if (i2c_id >= IIC_MAX) {
		return CVI_FAILURE;
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

	return CVI_SUCCESS;

INIT_FAIL:
	csi_iic_uninit(iic);
	return CVI_FAILURE;
}

int sensor_i2c_exit(CVI_S32 ViPipe, CVI_U8 i2c_id)
{
	csi_iic_t *iic;

	if (i2c_id >= IIC_MAX) {
		return CVI_FAILURE;
	}

	iic = &iic_adapter_list[i2c_id];

	csi_iic_uninit(iic);

	return CVI_SUCCESS;
}

int sensor_i2c_read(CVI_S32 ViPipe, CVI_U8 i2c_id, CVI_U8 snsr_i2c_addr,
			CVI_U32 addr,CVI_U32 snsr_addr_byte,
			CVI_U32 snsr_data_byte)
{
	int ret = CVI_SUCCESS;
	CVI_U8 buf[8];
	CVI_U8 idx = 0;
	CVI_U32 data = 0;
	CVI_U32 timeout = 100;
	csi_iic_t *iic;
	csi_iic_mem_addr_size_t reg_addr_len;

	if (i2c_id >= IIC_MAX) {
		return CVI_FAILURE;
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
		return CVI_FAILURE;
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

int sensor_i2c_write(CVI_S32 ViPipe, CVI_U8 i2c_id, CVI_U8 snsr_i2c_addr,
			CVI_U32 addr, CVI_U32 snsr_addr_byte,
			CVI_U32 data, CVI_U32 snsr_data_byte)
{
	int ret = CVI_SUCCESS;
	CVI_U8 buf[8];
	CVI_U8 idx = 0;
	CVI_U32 timeout = 10;
	csi_iic_t *iic;
	csi_iic_mem_addr_size_t reg_addr_len;

	if (i2c_id >= IIC_MAX) {
		return CVI_FAILURE;
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
		return CVI_FAILURE;
	}
	// printf("i2c w 0x%x = 0x%2x\n", addr, data);
	return CVI_SUCCESS;
}

#else
static int g_fd[IIC_MAX] = {[0 ... (IIC_MAX - 1)] = -1};

int sensor_i2c_init(CVI_S32 ViPipe, CVI_U8 i2c_id, CVI_U8 snsr_i2c_addr)
{
	char acDevFile[16] = {0};

	if (g_fd[ViPipe] >= 0)
		return CVI_SUCCESS;
	int ret;

	snprintf(acDevFile, sizeof(acDevFile),  "/dev/i2c-%u", i2c_id);

	g_fd[ViPipe] = open(acDevFile, O_RDWR, 0600);

	if (g_fd[ViPipe] < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "Open /dev/cvi_i2c_drv-%u error!\n", i2c_id);
		return CVI_FAILURE;
	}

	ret = ioctl(g_fd[ViPipe], I2C_SLAVE_FORCE, snsr_i2c_addr);
	if (ret < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "I2C_SLAVE_FORCE error!\n");
		close(g_fd[ViPipe]);
		g_fd[ViPipe] = -1;
		return ret;
	}

	return CVI_SUCCESS;
}

int sensor_i2c_exit(CVI_S32 ViPipe, CVI_U8 i2c_id)
{
	UNUSED(i2c_id);
	if (g_fd[ViPipe] >= 0) {
		close(g_fd[ViPipe]);
		g_fd[ViPipe] = -1;
		return CVI_SUCCESS;
	}
	return CVI_FAILURE;
}

int sensor_i2c_read(CVI_S32 ViPipe, CVI_U8 i2c_id, CVI_U8 snsr_i2c_addr,
			CVI_U32 addr,CVI_U32 snsr_addr_byte,
			CVI_U32 snsr_data_byte)
{
	UNUSED(i2c_id);
	UNUSED(snsr_i2c_addr);
	int ret, data;
	CVI_U8 buf[8];
	CVI_U8 idx = 0;

	if (g_fd[ViPipe] < 0)
		return CVI_FAILURE;

	if (snsr_addr_byte == 2)
		buf[idx++] = (addr >> 8) & 0xff;

	//add address byte 0
	buf[idx++] = addr & 0xff;

	ret = write(g_fd[ViPipe], buf, snsr_addr_byte);
	if (ret < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "I2C_WRITE error!\n");
		return ret;
	}

	buf[0] = 0;
	buf[1] = 0;
	ret = read(g_fd[ViPipe], buf, snsr_data_byte);
	if (ret < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "I2C_READ error!\n");
		return ret;
	}

	//pack read back data
	data = 0;
	if (snsr_data_byte == 2) {
		data = buf[0] << 8;
		data += buf[1];
	} else {
		data = buf[0];
	}

	CVI_TRACE_SNS(CVI_DBG_DEBUG, "i2c r 0x%x = 0x%x\n", addr, data);
	return data;
}

int sensor_i2c_write(CVI_S32 ViPipe, CVI_U8 i2c_id, CVI_U8 snsr_i2c_addr,
			CVI_U32 addr, CVI_U32 snsr_addr_byte,
			CVI_U32 data, CVI_U32 snsr_data_byte)
{
	UNUSED(i2c_id);
	UNUSED(snsr_i2c_addr);
	CVI_U8 idx = 0;
	int ret;
	CVI_U8 buf[8];

	if (g_fd[ViPipe] < 0)
		return CVI_SUCCESS;

	if (snsr_addr_byte == 2) {
		buf[idx] = (addr >> 8) & 0xff;
		idx++;
		buf[idx] = addr & 0xff;
		idx++;
	} else if (snsr_addr_byte == 1) {
		buf[idx] = addr & 0xff;
		idx++;
	}

	if (snsr_data_byte == 1) {
		buf[idx] = data & 0xff;
		idx++;
	} else if (snsr_data_byte == 2) {
		buf[idx] = (data >> 8) & 0xff;
		idx++;
		buf[idx] = data & 0xff;
		idx++;
	}

	ret = write(g_fd[ViPipe], buf, snsr_addr_byte + snsr_data_byte);
	if (ret < 0) {
		CVI_TRACE_SNS(CVI_DBG_ERR, "I2C_WRITE error!\n");
		return CVI_FAILURE;
	}
	CVI_TRACE_SNS(CVI_DBG_DEBUG, "i2c w 0x%x 0x%x\n", addr, data);

	return CVI_SUCCESS;
}
#endif
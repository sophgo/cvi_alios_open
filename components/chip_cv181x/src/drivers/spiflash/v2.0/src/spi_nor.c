/*
* Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
*/
#include "spi_nor.h"
#include "drv/common.h"
#ifndef CONFIG_KERNEL_NONE
#include "aos/kernel.h"
#endif

/* Used when the "_ext_id" is two bytes at most */
#define INFO(_jedec_id, _ext_id, _sector_size, _n_sectors, _flags)      \
	.id = {                                                 \
		((_jedec_id) >> 16) & 0xff,                     \
		((_jedec_id) >> 8) & 0xff,                      \
		(_jedec_id) & 0xff,                             \
		((_ext_id) >> 8) & 0xff,                        \
		(_ext_id) & 0xff,                               \
	},                                              \
	.id_len = (!(_jedec_id) ? 0 : (3 + ((_ext_id) ? 2 : 0))),       \
	.sector_size = (_sector_size),                          \
	.n_sectors = (_n_sectors),                              \
	.page_size = 256,                                       \
	.flags = (_flags),

const struct flash_info spi_flash_ids[] = {
	/* The CVITEK Support List Of Spi Nor Flash */
	{ "JY25VQ128A", INFO(0x1c4018, 0x0, 64 * 1024, 256,
			RD_FULL | WR_FULL | SECT_4K) },
	{ "JY25VQ64A", INFO(0x4a4017, 0x0, 64 * 1024, 128,
			RD_FULL | WR_FULL | SECT_4K) },
	/* Juyang 32M Nor Flash(JY25VQ256A) uses the same wafers as MXIC */
	{ "MX25L25645G", INFO(0xc22019, 0x0, 64 * 1024, 512,
			RD_FULL | WR_FULL | SECT_4K | NOR_4B_OPCODES) },
	{ "MX25L12835F", INFO(0xc22018, 0x0, 64 * 1024, 256,
			RD_FULL | WR_FULL | SECT_4K) },
	{ "EN25QH128A", INFO(0x1c7018, 0x0, 64 * 1024, 256,
			RD_FULL | WR_FULL | SECT_4K) },
	{ "EN25QX128A", INFO(0x1c7118, 0x0, 64 * 1024, 256,
			RD_FULL | WR_FULL | SECT_4K) },
	{ "EN25QH256A", INFO(0x1c7019, 0x0, 64 * 1024, 512,
			RD_FULL | WR_FULL | SECT_4K | NOR_4B_OPCODES) },
	{ "W25Q256JV-IQ", INFO(0xef4019, 0x0, 64 * 1024, 512,
			RD_FULL | WR_FULL | SECT_4K | NOR_4B_OPCODES) },
	{ "W25Q128JV-IQ", INFO(0xef4018, 0x0, 64 * 1024, 256,
			RD_FULL | WR_FULL | SECT_4K) },
	{ "W25Q64JV-IQ",   INFO(0xef4017, 0x0, 64 * 1024, 128,
			RD_FULL | WR_FULL | SECT_4K) },
	{ "GD25Q128E", INFO(0xc84018, 0x0, 64 * 1024, 256,
			RD_FULL | WR_FULL | SECT_4K) },
	{ "GD25Q256E", INFO(0xc84019, 0x0, 64 * 1024, 512,
			RD_FULL | WR_FULL | SECT_4K | NOR_4B_OPCODES) },
	{ "GD25Q64E",   INFO(0xc84017, 0x0, 64 * 1024, 128,
			RD_FULL | WR_FULL | SECT_4K) },
	{ "XM25QH64C", INFO(0x204017, 0x0, 64 * 1024, 128,
			RD_FULL | WR_FULL | SECT_4K) },
	{ "XM25QH128C", INFO(0x204018, 0x0, 64 * 1024, 256,
			RD_FULL | WR_FULL | SECT_4K) },
	{ "XM25QH256C", INFO(0x204019, 0x0, 64 * 1024, 512,
			RD_FULL | WR_FULL | SECT_4K | NOR_4B_OPCODES) },
	{ "XM25QH256B", INFO(0x206019, 0x0, 64 * 1024, 512,
			RD_FULL | WR_FULL | SECT_4K | NOR_4B_OPCODES) },
	{ "IS25LP128F", INFO(0x9d4018, 0x0, 64 * 1024, 256,
			RD_FULL | WR_FULL | SECT_4K) },
	{ "IS25LP256E", INFO(0x9d6019, 0x0, 64 * 1024, 512,
			RD_FULL | WR_FULL | SECT_4K | NOR_4B_OPCODES) },
	{ "KH25L6433FM2I", INFO(0xc22017, 0x0, 64 * 1024, 128,
			RD_FULL | WR_FULL | SECT_4K) },
	//{ "KH25L12833FM2I", INFO(0xc22018, 0x0, 64 * 1024, 256,
	//      RD_FULL | SECT_4K) },
	{ "ZB25VQ64B", INFO(0x5E4017, 0x0, 64 * 1024, 128,
			RD_FULL | WR_FULL | SECT_4K) },

	{ "ZB25VQ128A", INFO(0x5E4018, 0x0, 64 * 1024, 256,
			RD_FULL | WR_FULL | SECT_4K) },
	{ "S25FL128L", INFO(0x016018, 0x0, 64 * 1024, 256,
		RD_FULL | WR_QPP | SECT_4K | _10_DUMMY_CYCLE) },
	{ "S25FL256L", INFO(0x016019, 0x0, 64 * 1024, 512,
		RD_FULL | WR_QPP | SECT_4K | NOR_4B_OPCODES |
			_10_DUMMY_CYCLE) },
	{ "XT25F64F", INFO(0x0B4017, 0x0, 64 * 1024, 128,
		RD_FULL | WR_QPP | SECT_4K) },
	{ "XT25F128F", INFO(0x0B4018, 0x0, 64 * 1024, 256,
		RD_FULL | WR_QPP | SECT_4K) },
	{ "FM25Q64", INFO(0xF83217, 0x0, 64 * 1024, 128,
		RD_FULL | WR_QPP | SECT_4K) },
	{ "BY25Q128AS", INFO(0x684018, 0x0, 64 * 1024, 256,
		RD_FULL | WR_QPP | SECT_4K) },
	{ "BY25Q256FS", INFO(0x684919, 0x0, 64 * 1024, 512,
		RD_FULL | WR_QPP | SECT_4K | NOR_4B_OPCODES) },
	{ "PY25Q128HA", INFO(0x852018, 0x0, 64 * 1024, 256,
		RD_FULL | WR_QPP | SECT_4K) },
	{ "P25Q64SH", INFO(0x856017, 0x0, 64 * 1024, 128,
		RD_FULL | WR_QPP | SECT_4K) },
	{ "GD25F64F", INFO(0xc84317, 0x0, 64 * 1024, 128,
		RD_FULL | WR_QPP | SECT_4K | _10_DUMMY_CYCLE | NO_QE | ADJUST_DUMMY) },

	{},     /* Empty entry to terminate the list */

};

const struct flash_info *spi_nor_read_id(struct spi_nor *nor)
{
	int                     tmp, offset = 0;
	uint8_t                 id[SPI_NOR_MAX_ID_LEN];
	int8_t                  buff[SPI_NOR_MAX_ID_LEN * 3];
	const struct flash_info *info;
	int32_t 				ret;

	memset(id, 0x0, SPI_NOR_MAX_ID_LEN);
	tmp = nor->read_reg(nor, CVI_SPINOR_OP_RDID, id, SPI_NOR_MAX_ID_LEN);
	if (tmp < 0) {
		printf("error %d reading JEDEC ID\n", tmp);
		return NULL;
	}

	for (tmp = 0; tmp < SPI_NOR_MAX_ID_LEN; tmp++) {

		if (tmp != 0 && id[tmp] == id[0])
			break;

		if (id[tmp] != 0xff && id[tmp] != 0) {
			ret = sprintf((char *)(buff + offset), "%x ", id[tmp]);
			if (ret < 0) {
				printf("[%s] sprintf failed, please check!\n", __func__);
				return NULL;
			}
			offset += ret;
		}
	}


#ifndef CONFIG_KERNEL_NONE
#ifdef CONFIG_DEBUG
	printf("spinor: ID = %s\n", buff);
#endif
#endif

	info = spi_flash_ids;
	for (; info->name != NULL; info++) {
		if (info->id_len) {
			if (!memcmp(info->id, id, info->id_len)) {
				return info;
			}
		}
	}
	return NULL;
}

int spi_nor_read_unique_id(struct spi_nor *nor, uint8_t *id, uint32_t id_len)
{
	int                     tmp, offset = 0;
	int8_t                  buff[16 * 3];
	int32_t 				ret;

	memset(id, 0x0, id_len);
	tmp = nor->read_reg(nor, 0x4b, id, id_len);
	if (tmp < 0) {
		printf("error %d reading unique ID\n", tmp);
		return -1;
	}

	for (tmp = 0; tmp < id_len; tmp++) {

		if (tmp != 0 && id[tmp] == id[0])
			break;

		if (id[tmp] != 0xff && id[tmp] != 0) {
			ret = sprintf((char *)(buff + offset), "%x ", id[tmp]);
			if (ret < 0) {
				printf("[%s] sprintf failed, please check!\n", __func__);
				return -1;
			}
			offset += ret;
		}
	}

	printf("spinor: unique ID = %s\n", buff);
	return 0;
}


void print_message(struct spi_nor *nor)
{
	int i;
	struct flash_info *info = nor->info;
	spi_tran_conf_t   *read_op = &nor->read_op;
	spi_tran_conf_t   *write_op = &nor->write_op;

#ifdef CONFIG_DEBUG
	printf("\tnor flash:%s\n", info->name);
	printf("\tID:");
	for (i = 0; i < info->id_len; i++)
		printf("%x ",info->id[i]);
	printf("\n");

	printf("\tflags: %#x\n", info->flags);
	printf("\tcmd mode: %s\n", ((info->flags & NOR_4B_OPCODES) ? "4B_OPCIDE" : "3B_OPCODES"));
	printf("\terase opcode: 0x%x\n", nor->erase_opcode);
	printf("\terase size: 0x%x\n", nor->erase_size);

	printf("\tread opcode: 0x%x\n", read_op->cmd.opcode);
	printf("\tread addr buswidth: 0x%x\n", read_op->addr.buswidth);
	printf("\tread data buswidth: 0x%x\n", read_op->data.buswidth);
	printf("\tread dummy clks: 0x%x\n", read_op->dummy.clks);

	printf("\tprogram opcode: 0x%x\n", write_op->cmd.opcode);
	printf("\tprogram addr buswidth: 0x%x\n", write_op->addr.buswidth);
	printf("\tprogram data buswidth: 0x%x\n", write_op->data.buswidth);
#endif

}

static uint8_t spi_nor_convert_opcode(uint8_t opcode,
		const uint8_t table[][2], int size)
{
	int i;

	for (i = 0; i < size; i++)
		if (table[i][0] == opcode)
			return table[i][1];

	/* No conversion found, keep input op code. */
	return opcode;
}

void match_read_op(struct spi_nor *nor, uint32_t flags)
{
	struct flash_info *info = nor->info;
	spi_tran_conf_t   *read = &nor->read_op;

	read->addr.nbytes = nor->addr_width;

	/* all flash support 1_1_1 read */
	read->dummy.clks = 0;
	read->cmd.buswidth = 1;
	read->addr.buswidth = 1;
	read->data.buswidth = 1;
	read->cmd.opcode = CVI_SPINOR_OP_READ;

	/* maybe need to diff */
	if (info->flags & RD_DUALIO) {
		read->dummy.clks = 8;
		read->addr.buswidth = 1;
		read->data.buswidth = 2;
		read->cmd.opcode = CVI_SPINOR_OP_READ_1_1_2;
	}

	if (info->flags & RD_QUAD) {
		read->dummy.clks = 8;
		read->addr.buswidth = 1;
		read->data.buswidth = 4;
		read->cmd.opcode = CVI_SPINOR_OP_READ_1_1_4;
	}

	if (info->flags & RD_QUADIO) {
		read->dummy.clks = 6;
		read->addr.buswidth = 4;
		read->data.buswidth = 4;
		read->cmd.opcode = CVI_SPINOR_OP_READ_1_4_4;
		if (info->flags & _10_DUMMY_CYCLE)
			read->dummy.clks = 10;
		else
			read->dummy.clks = 6;
	}
}

void match_write_op(struct spi_nor *nor, uint32_t flags)
{
	struct flash_info *info = nor->info;
	spi_tran_conf_t   *write = &nor->write_op;

	write->addr.nbytes = nor->addr_width;

	/* all flash support 1_1_1 write */
	write->cmd.buswidth = 1;
	write->addr.buswidth = 1;
	write->data.buswidth = 1;
	write->cmd.opcode = CVI_SPINOR_OP_READ;

	if (info->flags & WR_QPP) {
		write->addr.buswidth = 1;
		write->data.buswidth = 4;
		write->cmd.opcode = CVI_SPINOR_OP_PP_1_1_4;
	}
}

static uint8_t spi_nor_convert_3to4_read(uint8_t opcode)
{
	static const uint8_t spi_nor_3to4_read[][2] = {
		{ CVI_SPINOR_OP_READ,       CVI_SPINOR_OP_READ_4B },
		{ CVI_SPINOR_OP_READ_FAST,  CVI_SPINOR_OP_READ_FAST_4B },
		{ CVI_SPINOR_OP_READ_1_1_2, CVI_SPINOR_OP_READ_1_1_2_4B },
		{ CVI_SPINOR_OP_READ_1_2_2, CVI_SPINOR_OP_READ_1_2_2_4B },
		{ CVI_SPINOR_OP_READ_1_1_4, CVI_SPINOR_OP_READ_1_1_4_4B },
		{ CVI_SPINOR_OP_READ_1_4_4, CVI_SPINOR_OP_READ_1_4_4_4B },
	};

	return spi_nor_convert_opcode(opcode, spi_nor_3to4_read, 6);
}

static inline uint8_t spi_nor_convert_3to4_program(uint8_t opcode)
{
	static const uint8_t spi_nor_3to4_program[][2] = {
		{ CVI_SPINOR_OP_PP,         CVI_SPINOR_OP_PP_4B },
		{ CVI_SPINOR_OP_PP_1_1_4,   CVI_SPINOR_OP_PP_1_1_4_4B },
		{ CVI_SPINOR_OP_PP_1_4_4,   CVI_SPINOR_OP_PP_1_4_4_4B },
	};

	return spi_nor_convert_opcode(opcode, spi_nor_3to4_program, 3);
}

static inline uint8_t spi_nor_convert_3to4_erase(uint8_t opcode)
{
	static const uint8_t spi_nor_3to4_erase[][2] = {
		{ CVI_SPINOR_OP_BE_4K,      CVI_SPINOR_OP_BE_4K_4B },
		{ CVI_SPINOR_OP_BE_32K,     CVI_SPINOR_OP_BE_32K_4B },
		{ CVI_SPINOR_OP_SE,         CVI_SPINOR_OP_SE_4B },
	};

	return spi_nor_convert_opcode(opcode, spi_nor_3to4_erase, 3);
}

/*  common operation for spi nor flash */
int write_enable(struct spi_nor *nor)
{
	return nor->write_reg(nor, CVI_SPINOR_OP_WREN, NULL, 0);
}

static inline int set_4byte(struct spi_nor *nor,
		const struct flash_info *info, int enable)
{
	int status;
	uint8_t cmd;

	write_enable(nor);
	cmd = enable ? CVI_SPINOR_OP_EN4B : CVI_SPINOR_OP_EX4B;
	status = nor->write_reg(nor, cmd, NULL, 0);
	return status;
}

static inline int write_disable(struct spi_nor *nor)
{
	return nor->write_reg(nor, CVI_SPINOR_OP_WRDI, NULL, 0);
}

static int read_sr(struct spi_nor *nor, uint8_t *val)
{
	int ret = 0;
	ret = nor->read_reg(nor, CVI_SPINOR_OP_RDSR, val, 1);
	if (ret < 0) {
		printf("error %d reading CR\n", ret);
		return ret;
	}

	return ret;
}

static int read_sr2(struct spi_nor *nor, uint8_t *val)
{
	int ret = 0;

	ret = nor->read_reg(nor, CVI_SPINOR_OP_RDCR, val, 1);
	if (ret < 0) {
		printf("error %d reading SR\n", (int) ret);
		return ret;
	}

	return ret;
}

static int read_sr3(struct spi_nor *nor, uint8_t *val)
{
	int ret = 0;

	ret = nor->read_reg(nor, 0x15, val, 1);
	if (ret < 0) {
		printf("error %d reading SR\n", (int) ret);
		return ret;
	}

	return ret;
}

/*
 * return value : 1 means that flash is ready, 0 means that flash is busy.
 */
static int spi_nor_ready(struct spi_nor *nor)
{
	int ret;
	uint8_t sr;
	ret = read_sr(nor, &sr);
	if (ret < 0)
		return ret;

	return !(sr & CVI_SR_WIP);
}
#if 1
static int spi_nor_wait_till_ready_with_timeout(struct spi_nor *nor, enum write_type op,
								uint32_t base_time, uint64_t time)
{
	uint64_t time_count = 0;
	int timeout = 0;
	int ret;

	while (!timeout) {

		if (time_count == time)
			timeout = 1;

		ret = spi_nor_ready(nor);
		if (ret < 0)
			return ret;

		if (ret)
			return 0;

		if (op == ERASE)
#ifndef CONFIG_KERNEL_NONE
			aos_msleep(base_time);
#else
			mdelay(base_time);
#endif
		else
			udelay(base_time);

		time_count += base_time;
	}
	printf("%s wait timeout!!!\n",__func__);
	return -1;
}
#else
static int spi_nor_wait_till_ready_with_timeout(struct spi_nor *nor, enum write_type op,
								uint32_t base_time, uint64_t time)
{
	int ret = 0;

	while (!ret) {

		ret = spi_nor_ready(nor);
		if (ret < 0)
			return ret;

		if (ret)
			return 0;

	}
	printf("%s wait timeout!!!\n",__func__);
	return -1;
}
#endif
static int write_sr2(struct spi_nor *nor, uint8_t sr)
{
	ssize_t ret;
	write_enable(nor);
	ret = nor->write_reg(nor, CVI_SPINOR_OP_WRSR2, &sr, 1);
	if (ret < 0) {
		return -1;
	}
	ret = spi_nor_ready(nor);
	if (ret) {
		return ret;
	}
	return 0;
}

static int write_sr3(struct spi_nor *nor, uint8_t sr)
{
	ssize_t ret;
	write_enable(nor);
	ret = nor->write_reg(nor, 0x11, &sr, 1);
	if (ret < 0) {
		return -1;
	}
	ret = spi_nor_ready(nor);
	if (ret) {
		return ret;
	}
	return 0;
}

static int write_sr(struct spi_nor *nor, uint8_t sr)
{
	ssize_t ret;

	write_enable(nor);

	ret = nor->write_reg(nor, CVI_SPINOR_OP_WRSR, &sr, 1);
	if (ret < 0)
		return -1;

	ret = spi_nor_ready(nor);
	if (ret) {
		return ret;
	}
	return 0;
}

int set_dummy(struct spi_nor *nor)
{
	int ret;
	uint8_t val = 0;
	ret = read_sr3(nor, &val);
	if (ret < 0)
		return ret;

	if ((val & 0x3) == 0x3) {
		return 0;
	}

	write_enable(nor);
	write_sr3(nor, val | 0x3);
	ret = spi_nor_ready(nor);
	if (ret)
		return ret;

	val = 0;
	ret = read_sr3(nor, &val);
	if (!(ret >= 0 && (val & 0x3))) {
		printf("SPINOR: SR3[0-1] dummy clk bit was set failed!\n");
		return -1;
	}

	return 0;
}

/*
 *   read sr2 : 35h
 *   write sr2: 31h
 *   qe bit pos is 1
 */
static int quad_enable_SR2_bit1(struct spi_nor *nor)
{
	uint8_t sr;
	int ret;

	ret = read_sr2(nor, &sr);
	if (ret < 0) {
		printf("error while reading status register2\n");
		return -1;
	}
	/* return if QE has be set already */
	if (sr & BIT(1))
		return 0;
	ret = write_sr2(nor, sr | BIT(1));
	if (ret < 0)
		return ret;
	/* read SR and check it */
	ret = read_sr2(nor, &sr);
	if (!(ret >= 0 && (sr & BIT(1)))) {
		printf("SF: SR QE bit was set failed\n");
		return -1;
	}
	return 0;
}

/*
 *   read sr : 05h
 *   write sr: 01h
 *   qe bit pos is 6
 */
static int quad_enable_SR_bit6(struct spi_nor *nor)
{
	uint8_t sr;
	int ret;

	ret = read_sr(nor, &sr);
	if (ret < 0) {
		printf("error while reading status register2\n");
		return -1;
	}

	/* return if QE has be set already */
	if (sr & BIT(6))
		return 0;

	ret = write_sr(nor, sr | BIT(6));
	if (ret < 0)
		return ret;

	/* read SR and check it */
	ret = read_sr(nor, &sr);
	if (!(ret >= 0 && (sr & BIT(6)))) {
		printf("SF: SR QE bit was set failed\n");
		return -1;
	}
	return 0;
}

static int set_quad_mode(struct spi_nor *nor)
{
	struct flash_info *info = nor->info;

	switch (JEDEC_MFR(info)) {
		case SNOR_MFR_ISSI:
		case SNOR_MFR_MACRONIX:
			return quad_enable_SR_bit6(nor);

		case SNOR_MFR_EON:
		case SNOR_MFR_GIGADEVICE:
		case SNOR_MFR_WINBOND:
		case SNOR_MFR_JUYANG:
		case SNOR_MFR_ZBIT:
		case SNOR_MFR_XMC:
			if (info->id[1] == 0x60) {
				return quad_enable_SR_bit6(nor);
			}
		case SNOR_MFR_XTX:
		case SNOR_MFR_FM:
		case SNOR_MFR_SPANSION:
		case SNOR_MFR_PY:
		case SNOR_MFR_FUDAN:
		case SNOR_MFR_BOYA:
			return quad_enable_SR2_bit1(nor);

		default:
			printf("SF: Need set QEB func for %02x flash\n",
					JEDEC_MFR(info));
			return -1;
	}
	return 0;
}

static uint32_t erase_chip(struct spi_nor *nor)
{
	return nor->write_reg(nor, CVI_SPINOR_OP_CHIP_ERASE, NULL, 0);
}

static int spi_nor_erase_sector(struct spi_nor *nor, uint8_t opcode, uint32_t addr)
{
	uint8_t buf[SPI_NOR_MAX_ADDR_WIDTH];
	int i;
	/*
	 * Default implementation, if driver doesn't have a specialized HW
	 * control
	 */
	for (i = nor->addr_width - 1; i >= 0; i--) {
		buf[i] = addr & 0xff;
		addr >>= 8;
	}

	return nor->write_reg(nor, opcode, buf, nor->addr_width);
}

#define ALIGNED(_to, _size) (!((_to) & ((_size) - 1)))
struct erase_op {
	uint8_t opcode;
	uint32_t size;
	uint32_t time;	// ms
	char *desc;
};

/* - Sector erase time: 80ms typical */
/* - Half Block erase time 200ms typical */
/* - Block erase time 400ms typical */
/* - Chip erase time: 60 Seconds typical */
const struct erase_op erase_op[4] = {
	{CVI_SPINOR_OP_SE, 0x10000, 400, "block erase"},
	{CVI_SPINOR_OP_BE_32K, 0x8000, 200, "half block erase"},
	{CVI_SPINOR_OP_BE_4K, 0x1000, 80, "sector erase"},
	{0}
};

int adjust_erase_argumen(struct spi_nor *nor, struct erase_op *op, uint32_t to, uint32_t len)
{
	const struct erase_op *tmp_op = erase_op;

	if (to & (0x1000 - 1)) {
		printf("offset is not aligned to 4K, can not do erase operation\n");
		return -1;
	}

	if (len & (0x1000 - 1)) {
		printf("erase size is not aligned to 4K, can not do erase operation\n");
		return -1;
	}

	for (; tmp_op->opcode; tmp_op++) {
		if (ALIGNED(to, tmp_op->size) && len >= tmp_op->size) {
			nor->erase_opcode = tmp_op->opcode;
			nor->erase_size = tmp_op->size;
			memcpy(op, tmp_op, sizeof(struct erase_op));
			break;
		}
	}
	//printf("[%s]:==> %s\n", __func__, tmp_op->desc);

	if (nor->addr_width == 4 && (nor->info->flags & NOR_4B_OPCODES)) {
		nor->erase_opcode = spi_nor_convert_3to4_erase(nor->erase_opcode);
		op->opcode = spi_nor_convert_3to4_erase(op->opcode);
	}
	return 0;
}

int cvi_spif_erase(struct spi_nor *nor, uint32_t to, uint32_t len)
{

	int ret;
	struct flash_info *info = nor->info;
	uint32_t erasesize = 0;

	if (len == info->sector_size * info->n_sectors) {
		write_enable(nor);
		if (erase_chip(nor))
			ret = -1;
		/* 120s for chip erase */
		ret = spi_nor_wait_till_ready_with_timeout(nor, ERASE, 1000, 60 * 2 * 1000);
		if (ret) {
			printf("erase all chip failed!\n");
			return ret;
		}

	} else {

		while (len) {
			struct erase_op op = {0};
			if (adjust_erase_argumen(nor, &op, to , len))
				return -1;

			write_enable(nor);
			ret = spi_nor_erase_sector(nor, op.opcode, to);
			if (ret)
				return ret;

			erasesize = op.size;
			to += erasesize;
			len -= erasesize;

			ret = spi_nor_wait_till_ready_with_timeout(nor, ERASE, 40, op.time);
			if (ret) {
				printf("erase a sector failed!\n");
				return ret;
			}
		}
	}
	write_disable(nor);
	return 0;
}

static inline uint32_t hweight32(uint32_t w)

{
	uint32_t res = (w & 0x55555555) + ((w >> 1) & 0x55555555);

	res = (res & 0x33333333) + ((res >> 2) & 0x33333333);

	res = (res & 0x0F0F0F0F) + ((res >> 4) & 0x0F0F0F0F);

	res = (res & 0x00FF00FF) + ((res >> 8) & 0x00FF00FF);

	return (res & 0x0000FFFF) + ((res >> 16) & 0x0000FFFF);

}

int spi_nor_write(struct spi_nor *nor, uint32_t to, const void *buf, uint32_t len)
{
	uint32_t page_offset, page_remain, i;
	int ret;

	for (i = 0; i < len; ) {
		uint32_t written;
		uint32_t addr = to + i;

		/*
		 * If page_size is a power of two, the offset can be quickly
		 * calculated with an AND operation. On the other cases we
		 * need to do a modulus operation (more expensive).
		 * Power of two numbers have only one bit set and we can use
		 * the instruction hweight32 to detect if we need to do a
		 * modulus (do_div()) or not.
		 */
		if (hweight32(nor->page_size) == 1) {
			page_offset = addr & (nor->page_size - 1);
		} else {
			uint32_t aux = addr;
			page_offset = aux % nor->page_size;
		}
		/* the size of data remaining on the first page */
		if ((nor->page_size - page_offset) > (len - i))
			page_remain = (len - i);
		else
			page_remain = (nor->page_size - page_offset);

		write_enable(nor);
		ret = nor->write(nor, addr, page_remain, buf + i);
		if (ret < 0)
			return ret;
		written = ret;
		/* 0.5-2ms for a page */
		ret = spi_nor_wait_till_ready_with_timeout(nor, WRITE, 500, 6000);
		if (ret)
			return ret;

		i += written;
		if (written != page_remain) {
			printf("While writing %#x bytes written %#x bytes\n",
				page_remain, written);
			ret = -1;
			return ret;
		}
	}
	return (int)i;
}

int spi_nor_rescan(struct spi_nor *nor)
{
	int ret;
	const struct flash_info *info = NULL;
	spi_tran_conf_t         *read_op = &nor->read_op;
	spi_tran_conf_t         *write_op = &nor->write_op;

	info = spi_nor_read_id(nor);
	if (info == NULL) {
		printf("can not found a nor flash in support list!!!\n");
		return -1;
	}

	nor->info = (struct flash_info *)info;
	nor->addr_width= (info->sector_size * info->n_sectors > _16M) ? 4 : 3;

	match_read_op(nor, info->flags);
	match_write_op(nor, info->flags);

	if (!(info->flags & NO_QE) && (info->flags & (RD_QUADIO | WR_QUAD | RD_QUAD)))
		set_quad_mode(nor);

	if (info->flags & ADJUST_DUMMY)
		set_dummy(nor);

#ifndef USE_4K_ERASE_SECTION
	nor->erase_opcode = CVI_SPINOR_OP_SE;
	nor->erase_size = info->sector_size;
#else
	nor->erase_opcode = CVI_SPINOR_OP_BE_4K;
	nor->erase_size = _4K;
#endif

	nor->page_size = 256;

	if (nor->addr_width == 4 && (info->flags & NOR_4B_OPCODES)) {
		read_op->cmd.opcode = spi_nor_convert_3to4_read(read_op->cmd.opcode);
		write_op->cmd.opcode = spi_nor_convert_3to4_program(write_op->cmd.opcode);
		nor->erase_opcode = spi_nor_convert_3to4_erase(nor->erase_opcode);
	} else if (nor->addr_width == 4 && !(info->flags & NOR_4B_OPCODES)) {
		ret = set_4byte(nor, info, 1);
		if (ret)
			printf("set 4 bytes for flash failed!\n");
	}
	return 0;
}

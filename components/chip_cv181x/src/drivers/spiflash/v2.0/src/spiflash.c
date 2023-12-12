/*
* Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
*/
#include "drv/spiflash.h"
#include "cvitek_spi_nor.h"
#include "spi_nor.h"

static struct cvi_spif g_cvi_spif = {0};

int hal_spiflash_init(void)
{
	struct cvi_spif *spif = NULL;
	spif = &g_cvi_spif;

	if (spif->io_base != NULL)
		return 0;

	if (spif->io_base == NULL)
		spif->io_base = (void *)SPI_NOR_REGBASE;
#ifdef	CONFIG_HS_SPEED_SUPPORT_SPINORFLASH
	cvi_spif_clk_setup(spif, 0);
#else
	/* set clk to 75M */
	cvi_spif_clk_setup(spif, 1);
#endif
	cvi_spif_setup_flash(spif);

	if (spi_nor_rescan(&spif->nor)) {
		printf("scan flash failed!\n");
		return -1;
	}
	return 0;
}

csi_error_t csi_spiflash_spi_init(csi_spiflash_t *spiflash, uint32_t spi_idx, void *spi_cs_callback)
{
	CSI_PARAM_CHK(spiflash,  CSI_ERROR);
	struct cvi_spif *spif = NULL;
	uint32_t reg;

	spif = &g_cvi_spif;

	memset(spif, 0x0, sizeof(struct cvi_spif));
	spiflash->priv = spif;

	if (spif->io_base != NULL)
		return 0;

	if (spif->io_base == NULL)
		spif->io_base = (void *)SPI_NOR_REGBASE;

	/* open gate clk for spi nor flash */
	reg = readl((void *)0x03002004);
	writel(reg | 0x1, (void *)0x03002004);
	/* open gate clk for spi nor flash */
	reg = readl((void *)0x03002004);
	writel(reg | 0x1, (void *)0x03002004);

	/* set clk to 75M */
	cvi_spif_clk_setup(spif, 1);
	cvi_spif_setup_flash(spif);

	if (spi_nor_rescan(&spif->nor)) {
		printf("scan flash failed!\n");
		return -1;
	}
	return 0;
}

extern int spi_nor_read_unique_id(struct spi_nor *nor, uint8_t *id, uint32_t id_len);
csi_error_t csi_spiflash_get_unique_id(csi_spiflash_t *spiflash, void *buffer, uint32_t len)
{
	struct cvi_spif *spif = spiflash->priv;
	return spi_nor_read_unique_id(&spif->nor, (uint8_t *)buffer, len);
}

void csi_spiflash_spi_uninit(csi_spiflash_t *spiflash)
{
	return;
}

csi_error_t csi_spiflash_get_flash_info(csi_spiflash_t *spiflash, csi_spiflash_info_t *flash_info)
{
	struct cvi_spif *spif = spiflash->priv;
	struct spi_nor *nor = &spif->nor;
	struct flash_info *info = nor->info;
	uint8_t *id = info->id;
	int i; 

	memset(flash_info, 0, sizeof(csi_spiflash_info_t));
	flash_info->flash_name = info->name;

	for (i = 0; i < info->id_len; i++)
		flash_info->flash_id |= id[i] << (info->id_len - 1 - i);

	flash_info->flash_size = info->sector_size * info->n_sectors;
	flash_info->page_size = info->page_size;
	flash_info->sector_size = nor->erase_size;
	return 0;
}

int hal_spiflash_read(struct cvi_spif *spif, uint32_t offset, void *data, uint32_t size)
{
#define _32K      (0x8000UL)

	struct spi_nor *nor = &spif->nor;
	int32_t len = 0;
	uint32_t read_size;
	int ret;

	if (nor->read == NULL) {
		printf("there is no read operation!\n");
		return -1;
	}

	while (size) {
		if (size >= _32K)
			read_size = _32K;
		else
			read_size = size & (_32K - 1);

		ret = nor->read(nor, offset, read_size, data);
		if (ret < 0) {
			printf("read data failed, ret:%d\n", ret);
			return -1;
		}
		offset += read_size;
		data += read_size;
		len += read_size;
		size -= read_size;
	}
	return len;
}

int32_t csi_spiflash_read(csi_spiflash_t *spiflash, uint32_t offset, void *data, uint32_t size)
{
	int ret;
	struct cvi_spif *spif = spiflash->priv;
	ret = hal_spiflash_read(spif, offset, data, size);
	return ret;
}

int32_t hal_spiflash_program(struct cvi_spif *spif, uint32_t offset, const void *data, uint32_t size)
{
	struct spi_nor *nor = &spif->nor;
	return spi_nor_write(nor, offset, data, size);
}

int32_t csi_spiflash_program(csi_spiflash_t *spiflash, uint32_t offset, const void *data, uint32_t size)
{
	struct cvi_spif *spif = spiflash->priv;
	struct spi_nor *nor = &spif->nor;
	int ret;

	ret = spi_nor_write(nor, offset, data, size);
	return ret;
}

int32_t hal_spiflash_read_reg(struct cvi_spif *spif, uint8_t cmd_code, uint8_t *data, uint32_t size)
{
	struct spi_nor *nor = &spif->nor;

	if (nor->read_reg == NULL) {
		printf("there is no read register operation!\n");
		return -1;
	}

	return nor->read_reg(nor, cmd_code, data, size);
}

csi_error_t csi_spiflash_read_reg(csi_spiflash_t *spiflash, uint8_t cmd_code, uint8_t *data, uint32_t size)
{
	struct cvi_spif *spif = spiflash->priv;
	return hal_spiflash_read_reg(spif, cmd_code, data, size);
}


int32_t hal_spiflash_write_reg(struct cvi_spif *spif, uint8_t cmd_code, uint8_t *data, uint32_t size)
{
	struct spi_nor *nor = &spif->nor;

	if (nor->write_reg == NULL) {
		printf("there is no write register operation!\n");
		return -1;
	}

	return nor->write_reg(nor, cmd_code, data, size);
}

csi_error_t csi_spiflash_write_reg(csi_spiflash_t *spiflash, uint8_t cmd_code, 
		uint8_t *data, uint32_t size)
{
	struct cvi_spif *spif = spiflash->priv;
	return hal_spiflash_write_reg(spif, cmd_code, data, size);
}

csi_error_t csi_spiflash_lock(csi_spiflash_t *spiflash, uint32_t offset, uint32_t size)
{
	printf("do not support lock operation\n");
	return 0;
}

int32_t hal_spiflash_erase(struct cvi_spif *spif, uint32_t offset, uint32_t size)
{
	struct spi_nor *nor = &spif->nor;

	if (nor->erase == NULL) {
		printf("there is no erase operation!\n");
		return -1;
	}

	return nor->erase(nor, offset, size);
}

csi_error_t csi_spiflash_erase(csi_spiflash_t *spiflash, uint32_t offset, uint32_t size)
{
	struct cvi_spif *spif = spiflash->priv;
	return hal_spiflash_erase(spif, offset, size);
}

csi_error_t csi_spiflash_unlock(csi_spiflash_t *spiflash, uint32_t offset, uint32_t size)
{
	return 0;
}

int csi_spiflash_is_locked(csi_spiflash_t *spiflash, uint32_t offset, uint32_t size)
{
	return 0;
}

csi_error_t csi_spiflash_config_data_line(csi_spiflash_t *spiflash,
		csi_spiflash_data_line_t line)
{
	return 0;
}


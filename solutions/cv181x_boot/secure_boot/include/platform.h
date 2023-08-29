/*
 * Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#ifndef __ASSEMBLY__

typedef unsigned long           uintptr_t;
typedef unsigned long           uint64_t;
typedef	unsigned int	        uint32_t;
typedef	unsigned char	        uint8_t;
typedef	unsigned short	        uint16_t;

struct spi_nand_info_t {
	uint32_t version;
	uint32_t id;
	uint32_t page_size;
	uint32_t spare_size;
	uint32_t block_size;
	uint32_t pages_per_block;
	uint32_t fip_block_cnt;
	uint8_t pages_per_block_shift;
	uint8_t badblock_pos;
	uint8_t dummy_data1[2];
	uint32_t flags;
	uint8_t ecc_en_feature_offset;
	uint8_t ecc_en_mask;
	uint8_t ecc_status_offset;
	uint8_t ecc_status_mask;
	uint8_t ecc_status_shift;
	uint8_t ecc_status_uncorr_val;
	uint8_t dummy_data2[2];
	uint32_t erase_count; // erase count for sys base block
	uint8_t sck_l;
	uint8_t sck_h;
	uint16_t max_freq;
	uint32_t sample_param;
	uint8_t xtal_switch;
	uint8_t dummy_data3[71];
};

struct spinor_info_t {
	uint32_t ctrl;
	uint32_t dly_ctrl;
	uint32_t tran_csr;
	uint32_t opt;
	uint32_t reserved_1;
	uint32_t reserved_2;
	uint32_t reserved_3;
	uint32_t reserved_4;
	uint32_t reserved_5;
} __packed;

struct chip_conf {
	uint32_t reg;
	uint32_t value;
} __packed;

struct fip_flags {
	struct {
		uint8_t rsa_size : 2;
		uint8_t scs : 2;
		uint8_t encrypted : 2;
		uint8_t reserved1 : 2;
	};
	uint8_t reserved2[7];
} __packed;

struct fip_param1 {
	uint64_t magic1;
	uint32_t magic2;
	uint32_t param_cksum;
	struct spi_nand_info_t nand_info;
	struct spinor_info_t spinor_info;
	struct fip_flags fip_flags;
	uint32_t chip_conf_size;
	uint32_t blcp_img_cksum;
	uint32_t blcp_img_size;
	uint32_t blcp_img_runaddr;
	uint32_t blcp_param_loadaddr;
	uint32_t blcp_param_size;
	uint32_t bl2_img_cksum;
	uint32_t bl2_img_size;
	uint32_t bld_img_size;
	uint32_t param2_loadaddr;
	uint32_t reserved1;
	struct chip_conf chip_conf[95];
	uint8_t bl_ek[32];
	uint8_t root_pk[512];
	uint8_t bl_pk[512];
	uint8_t bl_pk_sig[512];
	uint8_t chip_conf_sig[512];
	uint8_t bl2_img_sig[512];
	uint8_t blcp_img_sig[512];
} __packed __aligned(__alignof__(unsigned int));

#define BIT(nr)				(1UL << (nr))

#define SEC_SUBSYS_BASE 0x02000000
#define SEC_CRYPTODMA_BASE (SEC_SUBSYS_BASE + 0x00060000)

#if defined(CONFIG_CV180X) && (CONFIG_CV180X > 0)
#define TPU_SRAM_BASE 0x3C000000 // Shadow_tpu_mem
#define BL_RAM_BASE TPU_SRAM_BASE
#define BOOT_LOG_BUF_BASE (BL_RAM_BASE)
#define BOOT_LOG_BUF_SIZE 0x2000
#define PARAM1_BASE (BOOT_LOG_BUF_BASE + BOOT_LOG_BUF_SIZE)
#elif defined(CONFIG_CV181X) && (CONFIG_CV181X > 0)
#define TPU_SRAM_BASE 0x0C000000
#define BL_RAM_BASE TPU_SRAM_BASE
#define BL2_BASE (BL_RAM_BASE)
#define BL2_SIZE (0x37000)
#define BOOT_LOG_BUF_BASE (BL2_BASE + BL2_SIZE)
#define BOOT_LOG_BUF_SIZE 0x2000
#define PARAM1_BASE (BOOT_LOG_BUF_BASE + BOOT_LOG_BUF_SIZE)
#endif

#endif /* __ASSEMBLY__ */

#endif /* __PLATFORM_H__ */

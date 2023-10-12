#ifndef __SECURITY_H__
#define __SECURITY_H__

#include <efuse.h>
#include <mmio.h>

#define SHA256_SIZE 32
#define AES128_SIZE 16
#define RSA_E_BYTES 4
#define RSA_N_BYTES (2048 / 8)
#define IMAGE_PAD_SIZE 512

int dec_verify_image(const void *image, size_t size, size_t dec_skip,
					struct fip_param1 *fip_param1);

static inline int security_is_tee_enabled(void)
{
	int value = mmio_read_32(EFUSE_SCS_CONFIG_REG);

	value = (value & (0x3 << BIT_TEE_SCS_ENABLE)) >> BIT_TEE_SCS_ENABLE;
	return value;
}

static inline int security_is_tee_encrypted(void)
{
	int value = mmio_read_32(EFUSE_SCS_CONFIG_REG);

	value = (value & (0x3 << BIT_BOOT_LOADER_ENCRYPTION)) >> BIT_BOOT_LOADER_ENCRYPTION;
	return value;
}

static inline int security_pad(int image_size)
{
	image_size += RSA_N_BYTES;
	if (image_size % IMAGE_PAD_SIZE)
		image_size = (image_size + IMAGE_PAD_SIZE) & (~(IMAGE_PAD_SIZE - 1));

	return image_size;
}

#endif /* __SECURITY_H__ */

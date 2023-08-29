#include <stdio.h>
#include <string.h>
#include <platform.h>
#include <mmio.h>
#include <errno.h>
#include <security.h>
#include <cryptodma.h>

#include <bigdigits.h>
#include <tomcrypt.h>
#include <core/core_rv64.h>

extern uint32_t csi_tick_get_ms(void);

struct fip_param1 *g_fip_param1 = (void *)PARAM1_BASE;

static DIGIT_T rsa_e[RSA_N_BYTES / sizeof(DIGIT_T)] = { 0x10001 };
static DIGIT_T current_pk[RSA_N_BYTES / sizeof(DIGIT_T)];
static DIGIT_T current_sig[RSA_N_BYTES / sizeof(DIGIT_T)];
static DIGIT_T decrypted_sig[RSA_N_BYTES / sizeof(DIGIT_T)];
static uint8_t current_digest[SHA256_SIZE];

int cryptodma_aes_decrypt(const void *plain, const void *encrypted, uint64_t len, uint8_t *key, uint8_t *iv)
{
	__aligned(32) uint32_t dma_descriptor[22] = { 0 };

	uint32_t status;
	uint32_t ts;

	uint64_t dest = (uintptr_t)plain;
	uint64_t src = (uintptr_t)encrypted;
#if defined(CONFIG_DEBUG) && (CONFIG_DEBUG > 0)
	printf("AES/0x%lx/0x%lx/0x%lx\n", src, dest, len);
#endif
	// Prepare descriptor
	dma_descriptor[CRYPTODMA_CTRL] = DES_USE_DESCRIPTOR_IV | DES_USE_DESCRIPTOR_KEY | DES_USE_AES | 0xF;
	dma_descriptor[CRYPTODMA_CIPHER] = AES_KEY_MODE << 3 | CBC_ENABLE << 1 | DECRYPT_ENABLE;

	dma_descriptor[CRYPTODMA_SRC_ADDR_L] = (uint32_t)(src & 0xFFFFFFFF);
	dma_descriptor[CRYPTODMA_SRC_ADDR_H] = (uint32_t)(src >> 32);

	dma_descriptor[CRYPTODMA_DST_ADDR_L] = (uint32_t)(dest & 0xFFFFFFFF);
	dma_descriptor[CRYPTODMA_DST_ADDR_H] = (uint32_t)(dest >> 32);

	dma_descriptor[CRYPTODMA_DATA_AMOUNT_L] = (uint32_t)(len & 0xFFFFFFFF);
	dma_descriptor[CRYPTODMA_DATA_AMOUNT_H] = (uint32_t)(len >> 32);
	
	memcpy(&dma_descriptor[CRYPTODMA_KEY], key, 16);
	memcpy(&dma_descriptor[CRYPTODMA_IV], iv, 16);

	// Set cryptodma control
	mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_INT_MASK, 0x3);
	mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_L,
		      (uint32_t)((uintptr_t)dma_descriptor & 0xFFFFFFFF));
	mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DES_BASE_H,
		      (uint32_t)((uintptr_t)dma_descriptor >> 32));

	status = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL);

	csi_dcache_clean_invalid_range((uint64_t *)dma_descriptor, sizeof(dma_descriptor));
	csi_dcache_clean_invalid_range((uint64_t *)plain, len);
	csi_dcache_clean_invalid_range((uint64_t *)encrypted, len);

	// Clear interrupt
	mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT, 0x3);
	// Trigger cryptodma engine
	mmio_write_32(SEC_CRYPTODMA_BASE + CRYPTODMA_DMA_CTRL,
		      DMA_WRITE_MAX_BURST << 24 | DMA_READ_MAX_BURST << 16 | DMA_DESCRIPTOR_MODE << 1 | DMA_ENABLE);

	ts = csi_tick_get_ms();
	do {
		status = mmio_read_32(SEC_CRYPTODMA_BASE + CRYPTODMA_WR_INT);
#if defined(CONFIG_DEBUG) && (CONFIG_DEBUG > 0)		
		printf("INT status 0x%x\n", status);
#endif		
		if ((csi_tick_get_ms() - ts) >= 100) {
#if defined(CONFIG_DEBUG) && (CONFIG_DEBUG > 0)			
			printf("Decryption timeout\n");
#endif
			return -EIO;
		}
	} while (status == 0);

	return 0;
}

void bytes_reverse(void *buf, size_t n)
{
	uint8_t t, *a = buf;
	int i, j;

	for (i = 0, j = n - 1; i < j; i++, j--) {
		t = a[i];
		a[i] = a[j];
		a[j] = t;
	}
}

int verify_rsa(const void *message, size_t n, const void *sig, void *rsa_n, size_t rsa_nbytes)
{
	hash_state md;
#if defined(CONFIG_DEBUG) && (CONFIG_DEBUG > 0)
	printf("VI/0x%lx/0x%lx\n", (uintptr_t)message, n);
#endif
	mpConvFromOctets(current_pk, rsa_nbytes / sizeof(DIGIT_T), rsa_n, rsa_nbytes);
	mpConvFromOctets(current_sig, rsa_nbytes / sizeof(DIGIT_T), sig, rsa_nbytes);
	/** Computes y = x^e mod m */
	mpModExp(decrypted_sig, current_sig, rsa_e, current_pk, rsa_nbytes / sizeof(DIGIT_T));

	sha256_init(&md);
	sha256_process(&md, message, n);
	sha256_done(&md, current_digest);

	bytes_reverse(current_digest, sizeof(current_digest));

	if (memcmp(current_digest, decrypted_sig, SHA256_SIZE))
		return -EFAULT;

	return 0;
}

/*
 * image (size) = body (size - RSA_N_BYTES) + sig (RSA_N_BYTES)
 */
int dec_verify_image(const void *image, size_t size, size_t dec_skip, struct fip_param1 *fip_param1)
{
	const void *sig = image + size - RSA_N_BYTES;
	uint8_t iv[AES128_SIZE] = { 0 };

	if (size <= RSA_N_BYTES) {
#if defined(CONFIG_DEBUG) && (CONFIG_DEBUG > 0)		
		printf("image size <= signature size\n");
#endif		
		return -EFAULT;
	}

	if (!security_is_tee_enabled())
		return 0;

	if (security_is_tee_encrypted()) {
#if defined(CONFIG_DEBUG) && (CONFIG_DEBUG > 0)		
		printf("DI/0x%lx/0x%lx\n", (uintptr_t)image, size);
#endif
		cryptodma_aes_decrypt(image + dec_skip, image + dec_skip, size - dec_skip, g_fip_param1->bl_ek, iv);
	}

	size -= RSA_N_BYTES;

	return verify_rsa(image, size, sig, g_fip_param1->bl_pk, RSA_N_BYTES);
}

#ifndef LINUX_MMC_SDIO_FUNC_H
#define LINUX_MMC_SDIO_FUNC_H
#include "sdio.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef unsigned short __u16;
typedef unsigned int __u32;
typedef unsigned long long __u64;

struct sdio_func {
	sdio_card_t		*card;
	unsigned int		num;

	unsigned char		class;
	unsigned short		vendor;	
	unsigned short		device;	

	unsigned		max_blksize;
	unsigned		cur_blksize;

	unsigned		enable_timeout;

	unsigned int		state;
#define SDIO_STATE_PRESENT	(1<<0)

	unsigned char			*tmpbuf;

	unsigned		num_info;
	const char		**info;
    void        *driver_data;
};

/* sdio io operations */
extern u8 h3861_sdio_readb(struct sdio_func *function, unsigned int addr, int *err_ret);
extern u16 h3861_sdio_readw(struct sdio_func *function, unsigned int addr, int *err_ret);
extern u32 h3861_sdio_readl(struct sdio_func *function, unsigned int addr, int *err_ret);

/* sdio mem function */
extern int h3861_sdio_readsb(struct sdio_func *function, void *dst, unsigned int addr, int count);
extern int h3861_sdio_writesb(struct sdio_func *function, unsigned int addr, void *src, int count);

/* sdio write w/b/l */
extern u8 h3861_sdio_writeb_readb(struct sdio_func *function, u8 write_byte, unsigned int addr, int *err_ret);
extern void h3861_sdio_writeb(struct sdio_func *function, u8 b, unsigned int addr, int *err_ret);
extern void h3861_sdio_writew(struct sdio_func *function, u16 b, unsigned int addr, int *err_ret);
extern void h3861_sdio_writel(struct sdio_func *function, u32 b, unsigned int addr, int *err_ret);

/* sdio io memcpy */
extern int h3861_sdio_memcpy_fromio(struct sdio_func *function, void *dst, unsigned int addr, int count);
extern int h3861_sdio_memcpy_toio(struct sdio_func *function, unsigned int addr, void *src, int count);

/* sdio db operation */
extern unsigned char h3861_sdio_f0_readb(struct sdio_func *function, unsigned int addr, int *err_ret);
extern void h3861_sdio_f0_writeb(struct sdio_func *function, unsigned char b, unsigned int addr, int *err_ret);

/* sdio readb/w/l */
extern u8 sdio_readb(struct sdio_func *function, unsigned int addr, int *err_ret);
extern u16 sdio_readw(struct sdio_func *function, unsigned int addr, int *err_ret);
extern u32 sdio_readl(struct sdio_func *function, unsigned int addr, int *err_ret);



/* sdio function */
extern void sdio_claim_host(struct sdio_func *function);
extern void sdio_release_host(struct sdio_func *function);

extern int sdio_enable_func(struct sdio_func *function);
extern int sdio_disable_func(struct sdio_func *function);

extern int sdio_set_block_size(struct sdio_func *function, unsigned blksz);

extern int sdio_release_irq(struct sdio_func *function);

extern unsigned int sdio_align_size(struct sdio_func *function, unsigned int sz);



/* sdio crc check */
extern void sdio_retune_crc_disable(struct sdio_func *function);
extern void sdio_retune_crc_enable(struct sdio_func *function);

extern void sdio_retune_hold_now(struct sdio_func *function);
extern void sdio_retune_release(struct sdio_func *function);

#endif /* LINUX_MMC_SDIO_FUNC_H */

/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *  include/linux/mmc/sdio_func.h
 *
 *  Copyright 2007-2008 Pierre Ossman
 */

#ifndef LINUX_MMC_SDIO_FUNC_H
#define LINUX_MMC_SDIO_FUNC_H
#include "sdio.h"
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef unsigned short __u16;
typedef unsigned int __u32;

struct sdio_func {
	sdio_card_t		*card;		/* the card this device belongs to */
//	struct device		dev;		/* the device */
//	sdio_irq_handler_t	*irq_handler;	/* IRQ callback */
	unsigned int		num;		/* function number */

	unsigned char		class;		/* standard interface class */
	unsigned short		vendor;		/* vendor id */
	unsigned short		device;		/* device id */

	unsigned		max_blksize;	/* maximum block size */
	unsigned		cur_blksize;	/* current block size */

	unsigned		enable_timeout;	/* max enable timeout in msec */

	unsigned int		state;		/* function state */
#define SDIO_STATE_PRESENT	(1<<0)		/* present in sysfs */

	unsigned char			*tmpbuf;	/* DMA:able scratch buffer */

	unsigned		num_info;	/* number of info strings */
	const char		**info;		/* info strings */
    void        *driver_data;

//	struct sdio_func_tuple *tuples;
};

/*
 * SDIO I/O operations
 */
extern void sdio_claim_host(struct sdio_func *func);
extern void sdio_release_host(struct sdio_func *func);

extern int sdio_enable_func(struct sdio_func *func);
extern int sdio_disable_func(struct sdio_func *func);

extern int sdio_set_block_size(struct sdio_func *func, unsigned blksz);

extern int sdio_release_irq(struct sdio_func *func);

extern unsigned int sdio_align_size(struct sdio_func *func, unsigned int sz);

extern u8 sdio_readb(struct sdio_func *func, unsigned int addr, int *err_ret);
extern u16 sdio_readw(struct sdio_func *func, unsigned int addr, int *err_ret);
extern u32 sdio_readl(struct sdio_func *func, unsigned int addr, int *err_ret);

extern int sdio_memcpy_fromio(struct sdio_func *func, void *dst,
	unsigned int addr, int count);
extern int sdio_readsb(struct sdio_func *func, void *dst,
	unsigned int addr, int count);

extern void sdio_writeb(struct sdio_func *func, u8 b,
	unsigned int addr, int *err_ret);
extern void sdio_writew(struct sdio_func *func, u16 b,
	unsigned int addr, int *err_ret);
extern void sdio_writel(struct sdio_func *func, u32 b,
	unsigned int addr, int *err_ret);

extern u8 sdio_writeb_readb(struct sdio_func *func, u8 write_byte,
	unsigned int addr, int *err_ret);

extern int sdio_memcpy_toio(struct sdio_func *func, unsigned int addr,
	void *src, int count);
extern int sdio_writesb(struct sdio_func *func, unsigned int addr,
	void *src, int count);

extern unsigned char sdio_f0_readb(struct sdio_func *func,
	unsigned int addr, int *err_ret);
extern void sdio_f0_writeb(struct sdio_func *func, unsigned char b,
	unsigned int addr, int *err_ret);

extern void sdio_retune_crc_disable(struct sdio_func *func);
extern void sdio_retune_crc_enable(struct sdio_func *func);

extern void sdio_retune_hold_now(struct sdio_func *func);
extern void sdio_retune_release(struct sdio_func *func);

#endif /* LINUX_MMC_SDIO_FUNC_H */

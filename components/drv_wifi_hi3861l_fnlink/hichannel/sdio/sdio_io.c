// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  linux/drivers/mmc/core/sdio_io.c
 *
 *  Copyright 2007-2008 Pierre Ossman
 */

//#include <linux/export.h>
//include <linux/kernel.h>
//#include <linux/mmc/host.h>
//#include <linux/mmc/card.h>
//#include <linux/mmc/sdio.h>
//#include <linux/mmc/sdio_func.h>

//#include "sdio_ops.h"
//#include "core.h"
//#include "card.h"
//#include "host.h"
#include <stdio.h>
#include "sdio_host.h"
#include "sdio_io.h"
#include "sdio_func.h"
//#include "csi_kernel.h"
/*
typedef __u16  __le16;
typedef __u16  __be16;
typedef __u32  __le32;
typedef __u32  __be32;
*/
/*
static  __le32 cpu_to_le32(const __u32 *p)
{
	return (__le32)*p;
}
static  __le16 cpu_to_le16(const __u16 *p)
{
	return (__le16)*p;
}
*/
static  __u32 le32_to_cpup(const __le32 *p)
{
	return (__u32)*p;
}
static  __u16 le16_to_cpup(const __le16 *p)
{
	return (__u16)*p;
}


/*
static  __le32 cpu_to_le32(const __u32 p)
{
 return (__le32)p;
}
static  __u32 le32_to_cpup(const __le32 *p)
{
 return (__u32)*p;
}
static  __le16 cpu_to_le16(const __u16 p)
{
 return (__le16)p;
}
static  __u16 le16_to_cpup(const __le16 *p)
{
 return (__u16)*p;
}
*/

#ifndef FALSE			/* in case these macros already exist */
#define FALSE	0		/* values of boolean */
#endif
#ifndef TRUE
#define TRUE	1
#endif

#ifndef BIT
#define BIT(n)  (1UL << (n))
#endif

#define min(a, b) (((a) < (b)) ? (a) : (b))

extern aos_mutex_t g_sdio_hsot_mutex;

void sdio_claim_host(struct sdio_func *func)
{
    aos_mutex_lock(&g_sdio_hsot_mutex, AOS_WAIT_FOREVER);
}

void sdio_release_host(struct sdio_func *func)
{
    aos_mutex_unlock(&g_sdio_hsot_mutex);
}

int mmc_io_rw_direct(sdio_card_t *card, int write, unsigned fn,
	unsigned addr, u8 in, u8 *out)
{
    int ret = -1;
    u8 tmp_byte = in;
    
    if(write == 0) {
        ret= SDIO_IO_Read_Direct(card, fn, addr, out);
    } else if(write == 1) {
        ret = SDIO_IO_Write_Direct(card, fn, addr, &tmp_byte, TRUE);
        if((ret == 0) && (out != NULL)) {
            *out = tmp_byte;
        }
    }

    return ret;
}

extern aos_mutex_t g_sdio_tx_rx_mutex;
extern aos_mutex_t  g_sdio_cmd_mutex;

int mmc_io_rw_extended(sdio_card_t *card, int write, unsigned fn,
    unsigned addr, int incr_addr, u8 *buf, unsigned blocks, unsigned blksz)
{
    int ret;

    uint32_t size = blocks ? blocks : blksz;
    uint32_t block_mode = blocks ? BIT(27) : 0;
    uint32_t incr_mode = incr_addr ? BIT(26) : 0;
    uint32_t op_code = incr_mode | block_mode;

    //printf("****origin write start mmc_io_rw_extended*****\n");
    //dump(buf, 192);
    //printf("size %d\r\n", size);
    //printf("****origin write end mmc_io_rw_extended*****\n");
    //printf("blocks 0x%x blksz 0x%x\r\n",blocks,blksz);
    //printf("write 0x%x size 0x%x block_mode 0x%x incr_mode 0x%x op_code 0x%x\r\n",write,size,block_mode,incr_mode,op_code);
    aos_mutex_lock(&g_sdio_cmd_mutex, AOS_WAIT_FOREVER);
    if (write == 0) {
        //aos_mutex_lock(&g_sdio_tx_rx_mutex, AOS_WAIT_FOREVER);
        //printf("r\r\n");
        ret = SDIO_IO_Read_Extended(card, fn, addr, buf, size, op_code);
        //aos_mutex_unlock(&g_sdio_tx_rx_mutex);
    } else {
        //aos_mutex_lock(&g_sdio_tx_rx_mutex, AOS_WAIT_FOREVER);
        //printf("w\r\n");
        ret = SDIO_IO_Write_Extended(card, fn, addr, buf, size, op_code);
        //aos_mutex_unlock(&g_sdio_tx_rx_mutex);
    }

    if (ret != kStatus_Success) {
        printf("sdio_io_rw_extended_byte error write=0x%x,ret=%d\r\n", write, ret);

        aos_mutex_unlock(&g_sdio_cmd_mutex);
        return -1;
    }
    aos_mutex_unlock(&g_sdio_cmd_mutex);

    return ret;
}

/**
 *	sdio_enable_func - enables a SDIO function for usage
 *	@func: SDIO function to enable
 *
 *	Powers up and activates a SDIO function so that register
 *	access is possible.
 */
#if 0 
int sdio_enable_func(struct sdio_func *func)
{
    int ret = 0xdead;
    
    if (func->num == 1) {
        ret = SDIO_SelectIO(func->card, kSDIO_FunctionNum1);
    } else {
        ret = SDIO_SelectIO(func->card, kSDIO_FunctionNum0);
    }

    printf("func %s line %d func->num 0x%x ret %d\r\n",__func__,__LINE__,func->num, ret);
    ret = 0xdead;
    ret = SDIO_EnableIO(func->card, func->num, 1);
    printf("func %s line %d func->num 0x%x ret %d\r\n",__func__,__LINE__,func->num,ret);

    return ret;
} 
#else
int sdio_enable_func(struct sdio_func *func)
{
	int ret;
	unsigned char reg =0xff;
	unsigned long timeout;

	if (!func)
		return -EINVAL;

	ret = mmc_io_rw_direct(func->card, 0, 0, SDIO_CCCR_IOEx, 0, &reg);
	if (ret)
		goto err;

	reg |= 1 << func->num;

	ret = mmc_io_rw_direct(func->card, 1, 0, SDIO_CCCR_IOEx, reg, NULL);
	if (ret)
		goto err;

	timeout = 1000;

	while (1) {
		ret = mmc_io_rw_direct(func->card, 0, 0, SDIO_CCCR_IORx, 0, &reg);
		if (ret)
			goto err;
		if (reg & (1 << func->num))
			break;
		ret = -ETIME;
		timeout--;
		extern int usleep(__useconds_t usec);
		usleep(1000);
		if (!timeout)
			goto err;
	}

	printf("SDIO: Enabled device %d\n", func->num);

	return 0;

err:
	printf("SDIO: Failed to enable device %d\n", func->num);
	return ret;
}
#endif
/**
 *	sdio_disable_func - disable a SDIO function
 *	@func: SDIO function to disable
 *
 *	Powers down and deactivates a SDIO function. Register access
 *	to this function will fail until the function is reenabled.
 */
int sdio_disable_func(struct sdio_func *func)
{
	int ret;
	unsigned char reg;

	if (!func)
		return -EINVAL;

	ret = mmc_io_rw_direct(func->card, 0, 0, SDIO_CCCR_IOEx, 0, &reg);
	if (ret)
		goto err;

	reg &= ~(1 << func->num);

	ret = mmc_io_rw_direct(func->card, 1, 0, SDIO_CCCR_IOEx, reg, NULL);
	if (ret)
		goto err;

	printf("SDIO: Disabled device %d\n", func->num);

	return 0;

err:
	printf("SDIO: Failed to disable device %d\n", func->num);
	return -EIO;
}

/**
 *	sdio_set_block_size - set the block size of an SDIO function
 *	@func: SDIO function to change
 *	@blksz: new block size or 0 to use the default.
 *
 *	The default block size is the largest supported by both the function
 *	and the host, with a maximum of 512 to ensure that arbitrarily sized
 *	data transfer use the optimal (least) number of commands.
 *
 *	A driver may call this to override the default block size set by the
 *	core. This can be used to set a block size greater than the maximum
 *	that reported by the card; it is the driver's responsibility to ensure
 *	it uses a value that the card supports.
 *
 *	Returns 0 on success, -EINVAL if the host does not support the
 *	requested block size, or -EIO (etc.) if one of the resultant FBR block
 *	size register writes failed.
 *
 */
int sdio_set_block_size(struct sdio_func *func, unsigned blksz)
{
	int ret;
#if 0
    if (func->num == 1) {
        SDIO_SelectIO(&SDIO_Card, kSDIO_FunctionNum1);
    } else {
        SDIO_SelectIO(&SDIO_Card, kSDIO_FunctionNum0);
    }

    static int i = 0;

    if (func->num == 1 && i == 0) {
        ret = SDIO_EnableIO(&SDIO_Card, kSDIO_FunctionNum1, 1);

        if (ret != kStatus_Success) {
            printf("SDIO_EnableIO error\n");
        }

        printf("start get capability\n");
        ret = SDIO_GetCardCapability(&SDIO_Card, kSDIO_FunctionNum1);

        if (ret != kStatus_Success) {
            printf("SDIO_GetCardCapability error\n");
        }

        printf("start read cis\n");
        ret = SDIO_ReadCIS(&SDIO_Card, kSDIO_FunctionNum1, g_tupleList, 3);

        if (ret != kStatus_Success) {
            printf("SDIO_ReadCIS error\n");
        }

        i++;
    }
#else
    ret = SDIO_SetBlockSize(func->card, func->num, blksz);

    if (ret != kStatus_Success) {
        printf("SDIO_SetBlockSize error\n");
        return  1;
    }
#endif    
	func->cur_blksize = blksz;

	return 0;
}

/* Split an arbitrarily sized data transfer into several
 * IO_RW_EXTENDED commands. */
static int sdio_io_rw_ext_helper(struct sdio_func *func, int write,
	unsigned addr, int incr_addr, u8 *buf, unsigned size)
{
	unsigned remainder = size;
	unsigned max_blocks;
	int ret;

	if (!func || (func->num > 7))
		return -EINVAL;

    //printf("func->card->cccrflags 0x%x size 0x%x func->cur_blksize 0x%x func->num 0x%x\r\n",func->card->cccrflags,size,func->cur_blksize,func->num);
	/* Do the bulk of the transfer using block mode (if supported). */
	if (func->card->cccrflags & kSDIO_CCCRSupportMultiBlock) {
		/* Blocks per command is limited by host count, host transfer
		 * size and the maximum for IO_RW_EXTENDED of 511 blocks. */
		//max_blocks = min(func->card->host->max_blk_count, 511u);
        max_blocks = min(512, 511u);
		while (remainder >= func->cur_blksize) {
			unsigned blocks;

			blocks = remainder / func->cur_blksize;
			if (blocks > max_blocks)
				blocks = max_blocks;
			size = blocks * func->cur_blksize;

			ret = mmc_io_rw_extended(func->card, write,
				func->num, addr, incr_addr, buf,
				blocks, func->cur_blksize);
			if (ret)
            {
                printf("wdp-1000 ret=[0x%x]\r\n", ret);
                return ret;
            }
			remainder -= size;
			buf += size;
			if (incr_addr)
				addr += size;
		}
	}

	/* Write the remainder using byte mode. */
	while (remainder > 0) {
		size = min(remainder, 512);

		/* Indicate byte mode by setting "blocks" = 0 */
		ret = mmc_io_rw_extended(func->card, write, func->num, addr,
			 incr_addr, buf, 0, size);
		if (ret)
        {
            printf("wdp-1001 ret=[0x%x]\r\n", ret);
            return ret;
        }
		remainder -= size;
		buf += size;
		if (incr_addr)
			addr += size;
	}
	return 0;
}

/**
 *	sdio_readb - read a single byte from a SDIO function
 *	@func: SDIO function to access
 *	@addr: address to read
 *	@err_ret: optional status value from transfer
 *
 *	Reads a single byte from the address space of a given SDIO
 *	function. If there is a problem reading the address, 0xff
 *	is returned and @err_ret will contain the error code.
 */
u8 sdio_readb(struct sdio_func *func, unsigned int addr, int *err_ret)
{
	int ret;
	u8 val;

	if (!func) {
		if (err_ret)
			*err_ret = -EINVAL;
		return 0xFF;
	}

	ret = mmc_io_rw_direct(func->card, 0, func->num, addr, 0, &val);
	if (err_ret)
		*err_ret = ret;
	if (ret)
		return 0xFF;

	return val;
}

/**
 *	sdio_writeb - write a single byte to a SDIO function
 *	@func: SDIO function to access
 *	@b: byte to write
 *	@addr: address to write to
 *	@err_ret: optional status value from transfer
 *
 *	Writes a single byte to the address space of a given SDIO
 *	function. @err_ret will contain the status of the actual
 *	transfer.
 */
void sdio_writeb(struct sdio_func *func, u8 b, unsigned int addr, int *err_ret)
{
	int ret;

	if (!func) {
		if (err_ret)
			*err_ret = -EINVAL;
		return;
	}

	ret = mmc_io_rw_direct(func->card, 1, func->num, addr, b, NULL);
	if (err_ret)
		*err_ret = ret;
}

/**
 *	sdio_writeb_readb - write and read a byte from SDIO function
 *	@func: SDIO function to access
 *	@write_byte: byte to write
 *	@addr: address to write to
 *	@err_ret: optional status value from transfer
 *
 *	Performs a RAW (Read after Write) operation as defined by SDIO spec -
 *	single byte is written to address space of a given SDIO function and
 *	response is read back from the same address, both using single request.
 *	If there is a problem with the operation, 0xff is returned and
 *	@err_ret will contain the error code.
 */
u8 sdio_writeb_readb(struct sdio_func *func, u8 write_byte,
	unsigned int addr, int *err_ret)
{
	int ret;
	u8 val;

	ret = mmc_io_rw_direct(func->card, 1, func->num, addr,
			write_byte, &val);
	if (err_ret)
		*err_ret = ret;
	if (ret)
		return 0xff;

	return val;
}

/**
 *	sdio_memcpy_fromio - read a chunk of memory from a SDIO function
 *	@func: SDIO function to access
 *	@dst: buffer to store the data
 *	@addr: address to begin reading from
 *	@count: number of bytes to read
 *
 *	Reads from the address space of a given SDIO function. Return
 *	value indicates if the transfer succeeded or not.
 */
int sdio_memcpy_fromio(struct sdio_func *func, void *dst,
	unsigned int addr, int count)
{
	return sdio_io_rw_ext_helper(func, 0, addr, 1, dst, count);
}

/**
 *	sdio_memcpy_toio - write a chunk of memory to a SDIO function
 *	@func: SDIO function to access
 *	@addr: address to start writing to
 *	@src: buffer that contains the data to write
 *	@count: number of bytes to write
 *
 *	Writes to the address space of a given SDIO function. Return
 *	value indicates if the transfer succeeded or not.
 */
int sdio_memcpy_toio(struct sdio_func *func, unsigned int addr,
	void *src, int count)
{
	return sdio_io_rw_ext_helper(func, 1, addr, 1, src, count);
}

/**
 *	sdio_readsb - read from a FIFO on a SDIO function
 *	@func: SDIO function to access
 *	@dst: buffer to store the data
 *	@addr: address of (single byte) FIFO
 *	@count: number of bytes to read
 *
 *	Reads from the specified FIFO of a given SDIO function. Return
 *	value indicates if the transfer succeeded or not.
 */
int sdio_readsb(struct sdio_func *func, void *dst, unsigned int addr,
	int count)
{
	return sdio_io_rw_ext_helper(func, 0, addr, 0, dst, count);
}

/**
 *	sdio_writesb - write to a FIFO of a SDIO function
 *	@func: SDIO function to access
 *	@addr: address of (single byte) FIFO
 *	@src: buffer that contains the data to write
 *	@count: number of bytes to write
 *
 *	Writes to the specified FIFO of a given SDIO function. Return
 *	value indicates if the transfer succeeded or not.
 */
int sdio_writesb(struct sdio_func *func, unsigned int addr, void *src,
	int count)
{
	return sdio_io_rw_ext_helper(func, 1, addr, 0, src, count);
}

/**
 *	sdio_readw - read a 16 bit integer from a SDIO function
 *	@func: SDIO function to access
 *	@addr: address to read
 *	@err_ret: optional status value from transfer
 *
 *	Reads a 16 bit integer from the address space of a given SDIO
 *	function. If there is a problem reading the address, 0xffff
 *	is returned and @err_ret will contain the error code.
 */
u16 sdio_readw(struct sdio_func *func, unsigned int addr, int *err_ret)
{
	int ret;

	ret = sdio_memcpy_fromio(func, func->tmpbuf, addr, 2);
	if (err_ret)
		*err_ret = ret;
	if (ret)
		return 0xFFFF;

	return le16_to_cpup((u16 *)func->tmpbuf);
}

/**
 *	sdio_writew - write a 16 bit integer to a SDIO function
 *	@func: SDIO function to access
 *	@b: integer to write
 *	@addr: address to write to
 *	@err_ret: optional status value from transfer
 *
 *	Writes a 16 bit integer to the address space of a given SDIO
 *	function. @err_ret will contain the status of the actual
 *	transfer.
 */
void sdio_writew(struct sdio_func *func, u16 b, unsigned int addr, int *err_ret)
{
	int ret;

	*(u16 *)func->tmpbuf = cpu_to_le16(b);

	ret = sdio_memcpy_toio(func, addr, func->tmpbuf, 2);
	if (err_ret)
		*err_ret = ret;
}

/**
 *	sdio_readl - read a 32 bit integer from a SDIO function
 *	@func: SDIO function to access
 *	@addr: address to read
 *	@err_ret: optional status value from transfer
 *
 *	Reads a 32 bit integer from the address space of a given SDIO
 *	function. If there is a problem reading the address,
 *	0xffffffff is returned and @err_ret will contain the error
 *	code.
 */
u32 sdio_readl(struct sdio_func *func, unsigned int addr, int *err_ret)
{
	int ret;

	ret = sdio_memcpy_fromio(func, func->tmpbuf, addr, 4);
	if (err_ret)
		*err_ret = ret;
	if (ret)
		return 0xFFFFFFFF;

	return le32_to_cpup((u32 *)func->tmpbuf);
}

/**
 *	sdio_writel - write a 32 bit integer to a SDIO function
 *	@func: SDIO function to access
 *	@b: integer to write
 *	@addr: address to write to
 *	@err_ret: optional status value from transfer
 *
 *	Writes a 32 bit integer to the address space of a given SDIO
 *	function. @err_ret will contain the status of the actual
 *	transfer.
 */
void sdio_writel(struct sdio_func *func, u32 b, unsigned int addr, int *err_ret)
{
	int ret;

	*(u32 *)func->tmpbuf = cpu_to_le32(b);

	ret = sdio_memcpy_toio(func, addr, func->tmpbuf, 4);
	if (err_ret)
		*err_ret = ret;
}

/**
 *	sdio_f0_readb - read a single byte from SDIO function 0
 *	@func: an SDIO function of the card
 *	@addr: address to read
 *	@err_ret: optional status value from transfer
 *
 *	Reads a single byte from the address space of SDIO function 0.
 *	If there is a problem reading the address, 0xff is returned
 *	and @err_ret will contain the error code.
 */
unsigned char sdio_f0_readb(struct sdio_func *func, unsigned int addr,
	int *err_ret)
{
	int ret;
	unsigned char val;

	if (!func) {
		if (err_ret)
			*err_ret = -EINVAL;
		return 0xFF;
	}

	ret = mmc_io_rw_direct(func->card, 0, 0, addr, 0, &val);
	if (err_ret)
		*err_ret = ret;
	if (ret)
		return 0xFF;

	return val;
}

/**
 *	sdio_f0_writeb - write a single byte to SDIO function 0
 *	@func: an SDIO function of the card
 *	@b: byte to write
 *	@addr: address to write to
 *	@err_ret: optional status value from transfer
 *
 *	Writes a single byte to the address space of SDIO function 0.
 *	@err_ret will contain the status of the actual transfer.
 *
 *	Only writes to the vendor specific CCCR registers (0xF0 -
 *	0xFF) are permiited; @err_ret will be set to -EINVAL for *
 *	writes outside this range.
 */
void sdio_f0_writeb(struct sdio_func *func, unsigned char b, unsigned int addr,
	int *err_ret)
{
	int ret;

	if (!func) {
		if (err_ret)
			*err_ret = -EINVAL;
		return;
	}

	if (addr < 0xF0 || addr > 0xFF) {
		if (err_ret)
			*err_ret = -EINVAL;
		return;
	}

	ret = mmc_io_rw_direct(func->card, 1, 0, addr, b, NULL);
	if (err_ret)
		*err_ret = ret;
}

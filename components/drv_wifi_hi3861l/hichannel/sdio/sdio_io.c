
//#include "sdio_ops.h"
//#include "core.h"
//#include "card.h"
//#include "host.h"
#include <stdio.h>
#include <errno.h>
#include "sdio_host.h"
#include "sdio_func.h"
//#include "csi_kernel.h"

typedef __u16  __le16;
typedef __u16  __be16;
typedef __u32  __le32;
typedef __u32  __be32;
typedef __u64  __le64;
typedef __u64  __be64;
#define cpu_to_le16(cvi_x) ((__le16)(__u16)(cvi_x))
#define cpu_to_le32(cvi_x) ((__le32)(__u32)(cvi_x))

#define SDIO_CCCR_IOEx		0x02
#define SDIO_CCCR_IORx		0x03


static  __u32 le32_to_cpup(const __le32 *p)
{
 return (__u32)*p;
}
// static  __le16 cpu_to_le16(const __u16 p)
// {
//  return (__le16)p;
// }
static  __u16 le16_to_cpup(const __le16 *p)
{
 return (__u16)*p;
}


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

void sdio_claim_host(struct sdio_func *function)
{
    aos_mutex_lock(&g_sdio_hsot_mutex, AOS_WAIT_FOREVER);
}

void sdio_release_host(struct sdio_func *function)
{
    aos_mutex_unlock(&g_sdio_hsot_mutex);
}

int h3861_mmc_io_write_direct(sdio_card_t *card, int write, unsigned fn,
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

int h3861_mmc_io_write_ext(sdio_card_t *card, int write, unsigned fn,
    unsigned addr, int incr_addr, u8 *buf, unsigned blocks, unsigned blksz)
{
    int ret;

    uint32_t size = blocks ? blocks : blksz;
    uint32_t block_mode = blocks ? BIT(27) : 0;
    uint32_t incr_mode = incr_addr ? BIT(26) : 0;
    uint32_t op_code = incr_mode | block_mode;

    //printf("****origin write start h3861_mmc_io_write_ext*****\n");
    //dump(buf, 192);
    //printf("size %d\r\n", size);
    //printf("****origin write end h3861_mmc_io_write_ext*****\n");
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
 *	@function: SDIO function to enable
 *
 *	Powers up and activates a SDIO function so that register
 *	access is possible.
 */
#if 0 
int sdio_enable_func(struct sdio_func *function)
{
    int ret = 0xdead;
    
    if (function->num == 1) {
        ret = SDIO_SelectIO(function->card, kSDIO_FunctionNum1);
    } else {
        ret = SDIO_SelectIO(function->card, kSDIO_FunctionNum0);
    }

    printf("function %s line %d function->num 0x%x ret %d\r\n",__func__,__LINE__,function->num, ret);
    ret = 0xdead;
    ret = SDIO_EnableIO(function->card, function->num, 1);
    printf("function %s line %d function->num 0x%x ret %d\r\n",__func__,__LINE__,function->num,ret);

    return ret;
} 
#else
int sdio_enable_func(struct sdio_func *function)
{
	int ret;
	unsigned char reg =0xff;
	unsigned long timeout;

	if (!function)
		return -EINVAL;

	// printf("SDIO: Enabling device %d...\n", function->num);

	ret = h3861_mmc_io_write_direct(function->card, 0, 0, SDIO_CCCR_IOEx, 0, &reg);
	if (ret)
		goto err;

    // printf("reg 0x%x\r\n",reg);
    
	reg |= 1 << function->num;

	ret = h3861_mmc_io_write_direct(function->card, 1, 0, SDIO_CCCR_IOEx, reg, NULL);
	if (ret)
		goto err;

	timeout = 1000;

	while (1) {
		ret = h3861_mmc_io_write_direct(function->card, 0, 0, SDIO_CCCR_IORx, 0, &reg);
		if (ret)
			goto err;
		if (reg & (1 << function->num))
			break;
		ret = -ETIME;
        timeout--;
		extern int usleep(__useconds_t usec);
        usleep(1000);
		if (!timeout)
			goto err;
	}

	// printf("SDIO: Enabled device %d\n", function->num);

	return 0;

err:
	printf("SDIO: Failed to enable device %d\n", function->num);
	return ret;
}
#endif

int sdio_disable_func(struct sdio_func *function)
{
	int ret;
	unsigned char reg;

	if (!function)
		return -EINVAL;

	printf("3861 sdio: disable device %d...\n", function->num);

	ret = h3861_mmc_io_write_direct(function->card, 0, 0, SDIO_CCCR_IOEx, 0, &reg);
	if (ret)
		goto err;

	reg &= ~(1 << function->num);

	ret = h3861_mmc_io_write_direct(function->card, 1, 0, SDIO_CCCR_IOEx, reg, NULL);
	if (ret)
		goto err;

	printf("3861 sdio: disable device %d\n", function->num);

	return 0;

err:
	printf("3861 sdio: Failed to disable device %d\n", function->num);
	return -EIO;
}

int sdio_set_block_size(struct sdio_func *function, unsigned blksz)
{
	int ret;
#if 0
    if (function->num == 1) {
        SDIO_SelectIO(&SDIO_Card, kSDIO_FunctionNum1);
    } else {
        SDIO_SelectIO(&SDIO_Card, kSDIO_FunctionNum0);
    }

    static int i = 0;

    if (function->num == 1 && i == 0) {
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
    ret = SDIO_SetBlockSize(function->card, function->num, blksz);

    if (ret != kStatus_Success) {
        printf("SDIO_SetBlockSize error\n");
        return  1;
    }
#endif    
	function->cur_blksize = blksz;

	return 0;
}

static int h3861_io_write_ext(struct sdio_func *function, int write,
	unsigned addr, int incr_addr, u8 *buf, unsigned size)
{
	unsigned remain_size = size;
	unsigned max_blocks_size;
	int ret;

	if (!function || (function->num > 7))
		return -EINVAL;

	if (function->card->cccrflags & kSDIO_CCCRSupportMultiBlock) {
        max_blocks_size = min(512, 511u);
		while (remain_size >= function->cur_blksize) {
			unsigned blocks;

			blocks = remain_size / function->cur_blksize;
			if (blocks > max_blocks_size)
				blocks = max_blocks_size;
			size = blocks * function->cur_blksize;

			ret = h3861_mmc_io_write_ext(function->card, write,
				function->num, addr, incr_addr, buf,
				blocks, function->cur_blksize);
			if (ret)
            {
                printf("h3861_mmc_io_write_ext ret=[0x%x]\r\n", ret);
                return ret;
            }
			remain_size -= size;
			buf += size;
			if (incr_addr)
				addr += size;
		}
	}

	while (remain_size > 0) {
		size = min(remain_size, 512);

		ret = h3861_mmc_io_write_ext(function->card, write, function->num, addr,
			 incr_addr, buf, 0, size);
		if (ret)
        {
            printf("h3861_mmc_io_write_ext ret=[0x%x]\r\n", ret);
            return ret;
        }
		remain_size -= size;
		buf += size;
		if (incr_addr)
			addr += size;
	}
	return 0;
}

u8 h3861_sdio_readb(struct sdio_func *function, unsigned int addr, int *err_ret)
{
	int ret;
	u8 val;

	if (!function) {
		if (err_ret)
			*err_ret = -EINVAL;
		return 0xff;
	}

	ret = h3861_mmc_io_write_direct(function->card, 0, function->num, addr, 0, &val);
	if (err_ret)
		*err_ret = ret;
	if (ret)
		return 0xff;

	return val;
}

void h3861_sdio_writeb(struct sdio_func *function, u8 b, unsigned int addr, int *err_ret)
{
	int ret;

	if (!function) {
		if (err_ret)
			*err_ret = -EINVAL;
		return;
	}

	ret = h3861_mmc_io_write_direct(function->card, 1, function->num, addr, b, NULL);
	if (err_ret)
		*err_ret = ret;
}

u8 h3861_sdio_writeb_readb(struct sdio_func *function, u8 write_byte,
	unsigned int addr, int *err_ret)
{
	int ret;
	u8 val;

	ret = h3861_mmc_io_write_direct(function->card, 1, function->num, addr,
			write_byte, &val);
	if (err_ret)
		*err_ret = ret;
	if (ret)
		return 0xff;

	return val;
}

int h3861_sdio_memcpy_fromio(struct sdio_func *function, void *dst,
	unsigned int addr, int count)
{
	return h3861_io_write_ext(function, 0, addr, 1, dst, count);
}

int h3861_sdio_memcpy_toio(struct sdio_func *function, unsigned int addr,
	void *src, int count)
{
	return h3861_io_write_ext(function, 1, addr, 1, src, count);
}

int h3861_sdio_readsb(struct sdio_func *function, void *dst, unsigned int addr,
	int count)
{
	return h3861_io_write_ext(function, 0, addr, 0, dst, count);
}

int h3861_sdio_writesb(struct sdio_func *function, unsigned int addr, void *src,
	int count)
{
	return h3861_io_write_ext(function, 1, addr, 0, src, count);
}

u16 h3861_sdio_readw(struct sdio_func *function, unsigned int addr, int *err_ret)
{
	int ret;

	ret = h3861_sdio_memcpy_fromio(function, function->tmpbuf, addr, 2);
	if (err_ret)
		*err_ret = ret;
	if (ret)
		return 0xffff;

	return le16_to_cpup((u16 *)function->tmpbuf);
}

void h3861_sdio_writew(struct sdio_func *function, u16 b, unsigned int addr, int *err_ret)
{
	int ret;

	*(u16 *)function->tmpbuf = cpu_to_le16(b);

	ret = h3861_sdio_memcpy_toio(function, addr, function->tmpbuf, 2);
	if (err_ret)
		*err_ret = ret;
}

u32 h3861_sdio_readl(struct sdio_func *function, unsigned int addr, int *err_ret)
{
	int ret;

	ret = h3861_sdio_memcpy_fromio(function, function->tmpbuf, addr, 4);
	if (err_ret)
		*err_ret = ret;
	if (ret)
		return 0xffffffff;

	return le32_to_cpup((u32 *)function->tmpbuf);
}

void h3861_sdio_writel(struct sdio_func *function, u32 b, unsigned int addr, int *err_ret)
{
	int ret;

	*(u32 *)function->tmpbuf = cpu_to_le32(b);

	ret = h3861_sdio_memcpy_toio(function, addr, function->tmpbuf, 4);
	if (err_ret)
		*err_ret = ret;
}

unsigned char h3861_sdio_f0_readb(struct sdio_func *function, unsigned int addr,
	int *err_ret)
{
	int ret;
	unsigned char val;

	if (!function) {
		if (err_ret)
			*err_ret = -EINVAL;
		return 0xff;
	}

	ret = h3861_mmc_io_write_direct(function->card, 0, 0, addr, 0, &val);
	if (err_ret)
		*err_ret = ret;
	if (ret)
		return 0xff;

	return val;
}

void h3861_sdio_f0_writeb(struct sdio_func *function, unsigned char b, unsigned int addr,
	int *err_ret)
{
	int ret;

	if (!function) {
		if (err_ret)
			*err_ret = -EINVAL;
		return;
	}

	if (addr < 0xf0 || addr > 0xff) {
		if (err_ret)
			*err_ret = -EINVAL;
		return;
	}

	ret = h3861_mmc_io_write_direct(function->card, 1, 0, addr, b, NULL);
	if (err_ret)
		*err_ret = ret;
}

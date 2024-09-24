#ifndef _CVI_SNSR_I2C_H_
#define _CVI_SNSR_I2C_H_

#include "vi_snsr.h"
#include "pthread.h"
#include "k_spin_lock.h"
#include <aos/aos.h>
#include <drv/iic.h>

/* Maximum number of I2C devices supported. */
#define I2C_MAX_NUM         5

/* Maximum number of messages that can be sent in a single I2C transaction. */
#define I2C_MAX_MSG_NUM     32

/* Buffer size for I2C messages, calculated as four times the maximum message count. */
#define I2C_BUF_SIZE        (I2C_MAX_MSG_NUM << 2)  /* Equivalent to I2C_MAX_MSG_NUM * 4 */


int vip_sys_register_cmm_cb(unsigned long cmm, void *hdlr, void *cb);

struct i2c_msg {
	__u16 addr;	/* slave address			*/
	__u16 flags;
#define I2C_M_RD		0x0001	/* read data, from slave to master */
					/* I2C_M_RD is guaranteed to be 0x0001! */
#define I2C_M_TEN		0x0010	/* this is a ten bit chip address */
#define I2C_M_DMA_SAFE		0x0200	/* the buffer of this message is DMA safe */
					/* makes only sense in kernelspace */
					/* userspace buffers are copied anyway */
#define I2C_M_RECV_LEN		0x0400	/* length will be first received byte */
#define I2C_M_NO_RD_ACK		0x0800	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_IGNORE_NAK	0x1000	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_REV_DIR_ADDR	0x2000	/* if I2C_FUNC_PROTOCOL_MANGLING */
#define I2C_M_NOSTART		0x4000	/* if I2C_FUNC_NOSTART */
#define I2C_M_STOP		0x8000	/* if I2C_FUNC_PROTOCOL_MANGLING */
	__u16 len;		/* msg length				*/
	__u8 *buf;		/* pointer to msg data			*/
};

/* Structure representing the context for an I2C communication. */
struct cvi_i2c_ctx {
    csi_iic_t        *master_iic;           /* Pointer to the I2C master interface. */
    struct i2c_msg   msg[I2C_MAX_MSG_NUM];   /* Array of I2C messages for transactions. */
    uint8_t          *buf;                    /* Buffer for storing data during I2C operations. */
    uint32_t         msg_idx;                 /* Index for tracking the current message. */
    uint16_t         addr_bytes;              /* Number of bytes used for the I2C address. */
    uint16_t         reg_addr;                /* Register address for the I2C device. */
    uint16_t         data_bytes;              /* Number of bytes of data to be read/written. */
};

/* Structure representing an I2C device with synchronization mechanisms. */
struct cvi_i2c_dev {
    kspinlock_t      lock;                    /* Spinlock for thread-safe access. */
    aos_mutex_t      mutex;                   /* Mutex for managing access to the I2C device. */
    struct cvi_i2c_ctx ctx[I2C_MAX_NUM];      /* Array of I2C contexts for multiple devices. */
};


/* Magic number for I2C sensor IOCTL commands. */
#define CVI_SNS_I2C_IOC_MAGIC    'i'

/* IOCTL command for writing to an I2C sensor. */
#define CVI_SNS_I2C_WRITE        0x1000

/* IOCTL command for queuing burst I2C transactions. */
#define CVI_SNS_I2C_BURST_QUEUE  0x1001

/* IOCTL command for executing burst I2C transactions. */
#define CVI_SNS_I2C_BURST_FIRE   0x1002


/*
 * Probes the I2C sensor, initializing the device and preparing it for operation.
 * Returns 0 on success, or a negative error code on failure.
 */
int cvi_snsr_i2c_probe(void);

/*
 * Removes the I2C sensor, cleaning up resources and unregistering the device.
 * Returns 0 on success, or a negative error code on failure.
 */
int cvi_snsr_i2c_remove(void);


#endif

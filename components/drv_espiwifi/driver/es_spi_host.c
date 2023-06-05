/*
* Copyright (C) 2017 C-SKY Microsystems Co., Ltd. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the 'License');
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an 'AS IS' BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions an
* limitations under the License.
*/

#include "drv/spi.h"
#include "board.h"
#include <aos/kernel.h>
#include <stdio.h>              
#include <ulog/ulog.h>
#include <aos/cli.h>
#include "drv/spi.h"
#include "debug/debug_dumpsys.h"
#include <drv/pin.h>
#include <drv/gpio.h>
#include <posix/timer.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <drv/irq.h>
#include "pinctrl-mars.h"
#include <es_spi_host.h>
#include <platform_thread.h>
#include <aos/aos.h>
#include "netvreg.h"

/* Static memory for static creation */
static aos_queue_t queue_handle;                          /* queue handle */
#define SPI_MASTER_MAX_PACKETLEN 2048
uint8_t g_drop_mem[SPI_MASTER_MAX_PACKETLEN] ={0};
unsigned int g_drop_state;
int es_send_finished = 1;
int es_receive_finished = 1;
unsigned char g_msg_buff[4096];

#define WAIT_FOREVER    0xffffffff
#define SPI_MQUEUE_SIZE  1024
#define DEFAULT_MAX_MSG_SIZE 1024
#define SPI_TX_NAME      "spitx_task"
#define SPI_RX_NAME      "spirx_task"
#define TASKA_PRIO       31
#define TASKA_STACKSIZE 1024
#define ES_SPI_CFG_DATA_LEN 32

#define SPI_CS_GPIO 0
csi_gpio_t spi_cs_gpio = {0};
csi_spi_t spi_tran_handler;
csi_gpio_t gpio ;

typedef struct {
    int gpioValue;
	aos_sem_t spiBusSem;
	aos_sem_t spiIrqSem;
	aos_mutex_t spi_mutex;
	aos_task_t	 spi_tx_handle;
	aos_task_t    spi_rx_handle;
	spi_service_func_t funcset;
} spi_master_priv_t;

spi_master_priv_t g_spi_priv;


#define GPIO_PIN_MASK(_gpio_num) (1 << _gpio_num)

void es_spi_callback_func( csi_spi_t *spi, csi_spi_event_t event, void *arg)
{
	if (event == SPI_EVENT_RECEIVE_COMPLETE)
		es_receive_finished = 0;
	if (event == SPI_EVENT_SEND_COMPLETE)
		es_send_finished = 0;
	#if SPI_CS_GPIO
		if(!es_send_finished && !es_receive_finished)
		csi_gpio_write(&spi_cs_gpio, GPIO_PIN_MASK(18), 1);	//拉高CS
	#endif
}

void es_spi2Pinmux(void)
{
	PINMUX_CONFIG(SD1_CLK, SPI2_SCK);
	PINMUX_CONFIG(SD1_CMD, SPI2_SDO);
	PINMUX_CONFIG(SD1_D0, SPI2_SDI);
	PINMUX_CONFIG(SD1_D2, PWR_GPIO_19);
#if SPI_CS_GPIO
	PINMUX_CONFIG(SD1_D3, PWR_GPIO_18);
#else
	PINMUX_CONFIG(SD1_D3, SPI2_CS_X);
#endif
}

#if SPI_CS_GPIO
void spi_cs_gpio_init()
{
	csi_error_t ret;
	
	ret = csi_gpio_init(&spi_cs_gpio, 4);
	if (ret != CSI_OK)
    {
	    printf("csi_gpio_dir failed\r\n");
		return;
    }
	ret = csi_gpio_dir(&spi_cs_gpio, GPIO_PIN_MASK(18), GPIO_DIRECTION_OUTPUT);
	if (ret != CSI_OK)
    {
	    printf("csi_gpio_dir failed\r\n");
		return;
    }	
	csi_gpio_write(&spi_cs_gpio, GPIO_PIN_MASK(18), 1);
}
#endif

int es_config_spi_init(csi_spi_t *spi_handler)
{
	int spi_ret = 0;
	spi_ret = csi_spi_init(spi_handler, 2);
	if (spi_ret == CSI_ERROR) {
		printf("spi init failed returned, actual %d\n", spi_ret);
		return -1;
	}
	// test csi_spi_mode
	spi_ret = csi_spi_mode(spi_handler, SPI_MASTER);
	if (spi_ret == CSI_ERROR) {
		printf("set spi mode failed, actual ret %d\n", spi_ret);
		return -1;
	}
	// test csi_spi_cp_format
	spi_ret = csi_spi_cp_format(spi_handler, SPI_FORMAT_CPOL0_CPHA0);
	if (spi_ret == CSI_ERROR) {
		printf("set spi format failed returned, actual ret %d\n", spi_ret);
		return -1;
	}
	// test csi_spi_frame_len
	spi_ret = csi_spi_frame_len(spi_handler, SPI_FRAME_LEN_8);
	if (spi_ret == CSI_ERROR) {
		printf("set spi frame len failed, actual ret %d\n", spi_ret);
		return -1;
	}
	// test csi_spi_baud
	csi_spi_baud(spi_handler, 10000000);
	return spi_ret;
}

static void spi_gpio_irq(csi_gpio_t *gpio)
{
	spi_master_priv_t *priv = &g_spi_priv;
	aos_cli_printf("goto %s() \n", __func__);
	csi_irq_disable((uint32_t)gpio->dev.irq_num);
	aos_sem_signal(priv->spiIrqSem);
}

int spi_gpio_irq_init()
{
    int ret;
	int dir = GPIO_DIRECTION_INPUT; 
	int gpio_pin = 19;
	PINMUX_CONFIG(SD1_D2, PWR_GPIO_19);
	ret=csi_gpio_init(&gpio, 4);
	if (ret != CSI_OK)
    { 	
	    printf("csi_gpio_init failed\r\n");
		return -1;
    }
	ret = csi_gpio_mode(&gpio, 1 << gpio_pin, GPIO_MODE_PULLUP);
	ret = csi_gpio_dir(&gpio, 1 << gpio_pin, dir);
    if (ret != CSI_OK)
    {
	    printf("csi_gpio_dir failed\r\n");
		return -2;
    }
	csi_gpio_irq_mode(&gpio, 1 << gpio_pin, GPIO_IRQ_MODE_HIGH_LEVEL);
	csi_gpio_attach_callback(&gpio, spi_gpio_irq, NULL);
	csi_gpio_irq_enable(&gpio, 1 << gpio_pin, true);
	return 0;
}

int es_spi_read_status()
{
	csi_error_t spi_ret;
	uint8_t sta_buf[6] = {0};
    unsigned char cmdbuff[6] = {ES_SPI_CMD_STATE, ES_SPI_CMD_DUMMY,0,0,0,0};
	#if SPI_CS_GPIO
	csi_gpio_write(&spi_cs_gpio, GPIO_PIN_MASK(18), 0);
	#endif
	spi_ret = csi_spi_send_receive_async(&spi_tran_handler, cmdbuff, sta_buf, sizeof(sta_buf));
	if (spi_ret < 0) {
		printf("set spi send failed, actual ret %d\n", spi_ret);
	}
	aos_msleep(4);
	while((es_receive_finished || es_send_finished));
	return (sta_buf[2] | (sta_buf[3] << 8) | (sta_buf[4] << 16) | (sta_buf[5] << 24));
}

int es_spi_send_data(spi_register_t *spi_cfg,unsigned char *send_data,unsigned char *receive_data, unsigned int len)
{
	csi_error_t spi_ret;
	unsigned char cfgBuff[ES_SPI_CFG_DATA_LEN] = {ES_SPI_CMD_WRITE, ES_SPI_CMD_DUMMY};
	uint8_t *rece_buf = (uint8_t*)malloc(2048);
	unsigned char tx_Buff[2048] = {ES_SPI_CMD_WRITE, ES_SPI_CMD_DUMMY};
	spi_register_t *spicfg = (spi_register_t *)&cfgBuff[2];
	unsigned int cfglen = 2;
	unsigned int dmalen = 0;
	cfglen += sizeof(spi_register_t);
	spicfg->evt =  spi_cfg->evt;
	spicfg->len =  len;
	spicfg->type = ES_SPI_CONTROL_WRITE;

	spi_ret = csi_spi_attach_callback(&spi_tran_handler, es_spi_callback_func, NULL);
	if (spi_ret < 0) 
	{
		printf("[%s %d]set spi config failed, actual ret %d\n", __func__, __LINE__, spi_ret);
	}
#if SPI_CS_GPIO
	csi_gpio_write(&spi_cs_gpio, GPIO_PIN_MASK(18), 0);
#endif
	spi_ret = csi_spi_send_receive_async(&spi_tran_handler, cfgBuff, rece_buf, cfglen);
	if (spi_ret < 0) {
		printf("[%s %d]set spi config failed, actual ret %d\n", __func__, __LINE__, spi_ret);
	}
	aos_msleep(4);
	while((es_receive_finished || es_send_finished));
	dmalen = es_spi_read_status();
    dmalen &= 0xFFFF;
	printf("datalen=%d\n", dmalen);
    if (dmalen == 0) {
        /* SPI SLAVE MAY NO ENOUGH MEM TO RECIEVE DATA, TRY AGAIN */
        return -2;
    }

    if (dmalen != len) {
        printf("DMA LEN 0x%x NOT EQUAL DLEN 0x%x\n", dmalen, len);
        return -1;
    }
	
	memcpy(&tx_Buff[2], send_data, len);
#if SPI_CS_GPIO
	csi_gpio_write(&spi_cs_gpio, GPIO_PIN_MASK(18), 0);
#endif
	spi_ret = csi_spi_send_receive_async(&spi_tran_handler, tx_Buff, rece_buf, len + 2);
	if (spi_ret < 0) {
		printf("set spi send failed, actual ret %d\n", spi_ret);
	}
	aos_msleep(4);
	while((es_receive_finished || es_send_finished));
	return spi_ret;
}

int es_spi_read_data(unsigned char *receive_data, unsigned int len)
{
	csi_error_t spi_ret;
	spi_master_priv_t *priv = &g_spi_priv;
	unsigned int cnt = 0;
	unsigned int status = 0;
	unsigned char send_cfgBuff[ES_SPI_CFG_DATA_LEN] = {ES_SPI_CMD_WRITE, ES_SPI_CMD_DUMMY};
	unsigned char read_cfgBuff[ES_SPI_CFG_DATA_LEN] = {ES_SPI_CMD_READ,ES_SPI_CMD_DUMMY};
	unsigned char send_read[ES_SPI_CFG_DATA_LEN] = {0};

	spi_register_t *spicfg = (spi_register_t *)&send_cfgBuff[2];
	unsigned int cfglen = 2;
	cfglen += sizeof(spi_register_t);
    spicfg->evt = 0x200;
    spicfg->len = 0;
    spicfg->type = ES_SPI_CONTROL_READ;

//	es_config_spi_init(&spi_handler);
	spi_ret = csi_spi_attach_callback(&spi_tran_handler, es_spi_callback_func, NULL);
	if (spi_ret) 
	{
		printf("[%s %d]set spi config failed, actual ret %d\n", __func__, __LINE__, spi_ret);
	}
#if SPI_CS_GPIO
		csi_gpio_write(&spi_cs_gpio, GPIO_PIN_MASK(18), 0);
#endif
	spi_ret = csi_spi_send_receive_async(&spi_tran_handler, send_cfgBuff, send_read, cfglen);
	if (spi_ret < 0) {
		printf("[%s %d]set spi config failed, actual ret %d\n", __func__, __LINE__, spi_ret);
	}
	aos_msleep(4);
	while((es_receive_finished || es_send_finished));
	do {
        status = es_spi_read_status();
        if (cnt++ > ES_SPI_STATE_TIMEOUT) {
            printf("%s[%d]READ SPI SLAVE STATE TIMEOUT(%d)\n", __func__, __LINE__, cnt);
            return -1;
        }
    } while ((status & BIT16) != BIT16);

	if ((status & SPI_SERVICE_CONTROL_MSG) == SPI_SERVICE_CONTROL_MSG) 
	{
		spicfg->evt = SPI_SERVICE_TYPE_MSG;
		spicfg->len = status & 0xFFF;
	} 
	else if ((status & SPI_SERVICE_CONTROL_INT) == SPI_SERVICE_CONTROL_INT) 
	{
		spicfg->evt = SPI_SERVICE_TYPE_INT;
	} 
	else
	{
		spicfg->len = status & 0xFFFF;
	}
	
	if (spicfg->len == 0 && spicfg->evt != SPI_SERVICE_TYPE_INT) {
		aos_sem_signal(&priv->spiBusSem);
		printf( "read 0x%x may something error\n", spicfg->evt);
		return -1;
	}

	if (spicfg->evt == SPI_SERVICE_TYPE_INT) 
	{
        aos_sem_signal(priv->spiBusSem);
        return -1;
    }
#if SPI_CS_GPIO
		csi_gpio_write(&spi_cs_gpio, GPIO_PIN_MASK(18), 0);
#endif
	spi_ret = csi_spi_send_receive_async(&spi_tran_handler, read_cfgBuff, receive_data, len);
	if (spi_ret < 0) {
		printf("set spi send failed, actual ret %d\n", spi_ret);
	}
	aos_msleep(4);
	while((es_receive_finished || es_send_finished));

//	g_drop_state = 1;
//    g_drop_smem.memAddr = (unsigned int)g_drop_mem;
//    g_drop_smem.memLen = (spicfg->len + 1) & (~1);
//    g_drop_smem.memOffset = 0;
//    g_drop_smem.memType = 0x200;
//    priv->currMem = &g_drop_smem;
//	spi_ret = csi_spi_send_receive_async(&spi_handler, read_cfgBuff, receive_data, len);
//	if (spi_ret < 0) {
//		printf("set spi send failed, actual ret %d\n", spi_ret);
//	}
//	aos_msleep(4);
//	while((es_receive_finished || es_send_finished));
	return spi_ret;
}

int es_spi_get_info_data(unsigned int len, unsigned char *skb)
{
	csi_error_t spi_ret;
	int cnt = 0;
	spi_master_priv_t *priv = &g_spi_priv;
	unsigned char send_cfgBuff[ES_SPI_CFG_DATA_LEN] = {ES_SPI_CMD_WRITE, ES_SPI_CMD_DUMMY};
	unsigned char send_data[80] = {ES_SPI_CMD_READ, ES_SPI_CMD_DUMMY};
	spi_register_t *spicfg = (spi_register_t *)&send_cfgBuff[2];
	unsigned int cfglen = 2;
	cfglen += sizeof(spi_register_t);
    spicfg->evt = SPI_SERVICE_TYPE_INFO;
    spicfg->len = len;
    spicfg->type = ES_SPI_CONTROL_READ;

	spi_ret = aos_sem_wait(&priv->spiBusSem, WAIT_FOREVER);
	if (spi_ret < 0) 
	{
	   printf("spi bus recv info sem take error.\n");
	   return -1;
	}

	spi_ret = csi_spi_attach_callback(&spi_tran_handler, es_spi_callback_func, NULL);
	if (spi_ret) 
	{
		printf("set spi config failed, actual ret %d\n", spi_ret);
		return spi_ret;
	}
#if SPI_CS_GPIO
	csi_gpio_write(&spi_cs_gpio, GPIO_PIN_MASK(18), 0);
#endif
	spi_ret = csi_spi_send_receive_async(&spi_tran_handler, send_cfgBuff, skb, cfglen);
	if (spi_ret < 0) {
		printf("set spi send failed, actual ret %d\n", spi_ret);
		return -1;
	}
	aos_msleep(4);
	while((es_receive_finished || es_send_finished));

	do
	{
		spi_ret = es_spi_read_status();
//		printf("spi_ret= 0x%x\n", spi_ret);
		if (spi_ret & 0x10000) {
			aos_sem_signal(priv->spiBusSem);
			break;
		}
		if (cnt > 10) 
		{
            aos_msleep(1);
        }
	}while(cnt++ < 1000);
		
	if ((spi_ret & 0x10000) == 0)
	{
		aos_sem_signal(priv->spiBusSem);
		printf("spi read info state timeout.\n");
		return -1;
	}

	if (spicfg->len == 0)
	{
		aos_sem_signal(priv->spiBusSem);
		printf("read info may something error.\n");
		return -1;
	}

#if SPI_CS_GPIO
	csi_gpio_write(&spi_cs_gpio, GPIO_PIN_MASK(18), 0);
#endif	
	spi_ret = csi_spi_send_receive_async(&spi_tran_handler, send_data, skb, len + 2);
	if (spi_ret < 0) {
		printf("set spi send failed, actual ret %d\n", spi_ret);
	}
	aos_msleep(4);
	while((es_receive_finished || es_send_finished));
//	spi_ret = skb[2] | (skb[3] << 8) | (skb[4] << 16) | (skb[5] << 24);
	return spi_ret;
}


int es_spi_write_info_data(int offset, unsigned char *send_data, unsigned int len)
{
	csi_error_t spi_ret;
	unsigned char cfgBuff[ES_SPI_CFG_DATA_LEN] = {ES_SPI_CMD_WRITE, ES_SPI_CMD_DUMMY};
	unsigned char tx_Buff[2048] = {ES_SPI_CMD_WRITE, ES_SPI_CMD_DUMMY};
	unsigned char receive_data[32] = {0};
	spi_register_t *spicfg = (spi_register_t *)&cfgBuff[2];
	unsigned int cfglen = 2;
	cfglen += sizeof(spi_register_t);
	spicfg->evt = offset;
	spicfg->len = len;
	spicfg->type = ES_SPI_CONTROL_WRITE;

//	es_config_spi_init(&spi_handler);
	spi_ret = csi_spi_attach_callback(&spi_tran_handler, es_spi_callback_func, NULL);
	if (spi_ret) 
	{
		printf("set spi config failed, actual ret %d\n", spi_ret);
	}
#if SPI_CS_GPIO
	csi_gpio_write(&spi_cs_gpio, GPIO_PIN_MASK(18), 0);
#endif
	spi_ret = csi_spi_send_receive_async(&spi_tran_handler, cfgBuff, receive_data, cfglen);
	if (spi_ret < 0) {
		printf("set spi send failed, actual ret %d\n", spi_ret);
	}
	aos_msleep(4);
	while((es_receive_finished || es_send_finished));
	
	
	memcpy(&tx_Buff[6], send_data, len);
#if SPI_CS_GPIO
		csi_gpio_write(&spi_cs_gpio, GPIO_PIN_MASK(18), 0);
#endif
	spi_ret = csi_spi_send_receive_async(&spi_tran_handler, tx_Buff, receive_data, len + 6);
	if (spi_ret < 0) {
		printf("set spi send failed, actual ret %d\n", spi_ret);
	}
	aos_msleep(4);
	while((es_receive_finished || es_send_finished));
	return spi_ret;
}

#if 0	
int es_spi_ioctl(unsigned int cmd, unsigned int* arg, unsigned int len)
{
//	unsigned char receive_data[1024] = {0};
	switch(cmd)
	{
		case SPI_IOC_RD_IP:
			*arg = es_spi_get_info_data(0x14, len);
			break;
		case SPI_IOC_RD_MASK:
			*arg = es_spi_get_info_data(0x18, len);
			break;
		case SPI_IOC_RD_MACADDR0:
			*arg = es_spi_get_info_data(0x1c, len);
			break;
		case SPI_IOC_RD_MACADDR1:
			*arg = es_spi_get_info_data(0x20, len);
			break;
		case SPI_IOC_RD_GW0:
			*arg = es_spi_get_info_data(0x24, len);
			break;
		case SPI_IOC_RD_GW1:
			*arg = es_spi_get_info_data(0x28, len);
			break;
		case SPI_IOC_RD_GW2:
			*arg = es_spi_get_info_data(0x2c, len);
			break;
		case SPI_IOC_RD_GW3:
			*arg = es_spi_get_info_data(0x30, len);
			break;
		case SPI_IOC_RD_DNS0:
			*arg = es_spi_get_info_data(0x34, len);
			break;
		case SPI_IOC_RD_DNS1:
			*arg = es_spi_get_info_data(0x38, len);
			break;
		case SPI_IOC_RD_DNS2:
			*arg = es_spi_get_info_data(0x3c, len);
			break;
		case SPI_IOC_RD_DNS3:
			*arg = es_spi_get_info_data(0x40, len);
			break;
		case SPI_IOC_INVALID_PORT_WR:
			es_spi_write_info_data(0x44, (unsigned char*)arg, len);
			break;
		case SPI_IOC_INVALID_PORT_RD:
			*arg = es_spi_get_info_data(0x44, len);
			break;
		case SPI_IOC_FE_VERSION:
			*arg = es_spi_get_info_data(0x48, len);
			break;
		case SPI_IOC_STATUS:
			*arg = es_spi_get_info_data(0x00, len);
			break;
		case SPI_IOC_OTA:	
//			es_spi_send_data(ES_SPI_DATA_TYPE_OTA, (unsigned char*)arg,receive_data, len);
			break;
	}
	return 0;
}
#endif

void spi_rx_thread(void *param)
{
	int ret;
	int len = 512;
	unsigned char read_data[512] = {0};
	spi_master_priv_t *priv = &g_spi_priv;
	do
	{
		priv->gpioValue = csi_gpio_read(&gpio, 1 << 19);
		if(priv->gpioValue == 0)
		{
			csi_gpio_irq_enable(&gpio, 1 << 19, true);
			printf("[%s %d]spi rx\n", __func__, __LINE__);
			ret = aos_sem_wait(priv->spiIrqSem, WAIT_FOREVER);
			if(ret < 0)
			{
				printf("spi rx sem take error.\n");
			}
		}
		ret = aos_sem_wait(priv->spiBusSem, WAIT_FOREVER);
		if(ret < 0)
		{
			printf("spi bus recv sem take error.\n");
		}
		es_spi_read_data(read_data, len);
	}while(1);

	printf("spi rx thread exit.\n");	
	aos_task_exit(0);
}

void spi_tx_thread(void *param)
{
	spi_master_priv_t *priv = &g_spi_priv;
	spi_register_t spi_cfg;
	size_t tx_size;
	uint8_t *rx_message_buf;
	rx_message_buf = (uint8_t*)malloc(2048);
	int ret;
	do
	{
//		printf( "spi queue  %p.\n", priv->spiBusSem);
		ret = aos_queue_recv(&queue_handle, AOS_WAIT_FOREVER, (void *)rx_message_buf, &tx_size);
		printf( "spi queue recv succ %p.\n", priv->spiBusSem);
		if (ret < 0) 
		{
            printf( "spi queue recv error.\n");
            break;
        }
		#if 0
		int i;
		for(i = 0; i<tx_size;i++)
		{
			printf("rx_buf[%d]=0x%x\n",i,rx_message_buf[i]);
		}
		#endif
RETRY:	
		ret = aos_sem_wait(&priv->spiBusSem, WAIT_FOREVER);
		if (ret < 0) 
		{
            printf( "spi bus send sem take error.\n");
            return;
        }
		spi_cfg.evt = 0x100;	
		//spi send package
		ret = es_spi_send_data(&spi_cfg,rx_message_buf,NULL,tx_size);
		if (ret == -2)
		{
			goto RETRY;
		}
	}while(1);
}

int spi_service_init(void)
{
	int ret = 0;
	uint8_t *queue_buffer;
	spi_master_priv_t *priv = &g_spi_priv;
	queue_buffer = aos_malloc(2048);
    ret = aos_queue_new(&queue_handle, queue_buffer, 2048, 512);
    if (ret != 0) {
        printf("create queue error\n");
        return -1;
    }
	aos_sem_new(&priv->spiBusSem, 1);
	aos_sem_new(&priv->spiIrqSem, 0);
	aos_mutex_new(&priv->spi_mutex);
	spi_gpio_irq_init();
	es_spi2Pinmux();
	#if SPI_CS_GPIO
	spi_cs_gpio_init();
	#endif
	es_config_spi_init(&spi_tran_handler);
//	printf("[%s %d]===>%p\n", __func__, __LINE__, priv->spiBusSem);
	ret = aos_task_new_ext(&priv->spi_tx_handle, SPI_TX_NAME, spi_tx_thread, NULL, 10*1024, 31);
    if (ret != 0)
	{
		aos_queue_free(&queue_handle);
        printf("create %s error\n", SPI_TX_NAME);
        return -1;
    }
//	ret = aos_task_new_ext(&priv->spi_rx_handle, SPI_RX_NAME, spi_rx_thread, NULL, 10*1024, 31);
//    if (ret != 0)
//	{
//		aos_queue_free(&queue_handle);
//        printf("create %s error\n", SPI_RX_NAME);
//        return -1;
//    }

	return 0;
}

#if 0
void test_es_spi_tx(int argc, char **argv)
{
	int i;
	uint8_t *send_buffer;
	int status = 0;
	send_buffer = (uint8_t*)malloc(2048);
	for(i = 0; i<1024;i++)
	{
		send_buffer[i] = i%256;
	}
	status = aos_queue_send(&queue_handle, (void *)send_buffer, 512);
    if (status != 0) {
        printf("send buf queue error!\n");
    }
}
ALIOS_CLI_CMD_REGISTER(test_es_spi_tx, spi_tx, test_es_spi_tx);
#endif

void test_es_spi_tx(int argc, char **argv)
{
	#if 0
	unsigned int len = 1024;
	spi_register_t spi_cfg;
	uint8_t send_buffer[1026] = {0};
	uint8_t receive_buffer[1026] = {0};
	int i;
	for(i = 0; i<1024;i++)
	{
		send_buffer[i] = i%256;
	}
	spi_cfg.evt = 0x100;	
	es_spi_send_data(&spi_cfg, send_buffer,receive_buffer,len);
	#endif
	vnet_reg_get_peer_value();
}
ALIOS_CLI_CMD_REGISTER(test_es_spi_tx, spi_tx, test_es_spi_tx);

#if 0
void test_es_spi_rx(int argc, char **argv)
{
	int len = 1024;
	uint8_t send_buffer[1024] = {0};
	uint8_t receive_buffer[1024] = {0};
	int i;
	es_spi_read_data(send_buffer,receive_buffer, len);
	for(i = 0; i<1024;i++)
	{
		printf("buff[%d]=0x%x\n", i,receive_buffer[i]);
	}
}
ALIOS_CLI_CMD_REGISTER(test_es_spi_rx, spi_rx, test_es_spi_rx);

void test_es_spi_info(int argc, char **argv)
{
	unsigned int buff[15] = {0};
	unsigned int len = 4;
	es_spi_ioctl(SPI_IOC_RD_IP, &buff[0],len);
	printf("ipaddr=0x%x\n", buff[0]);
	es_spi_ioctl(SPI_IOC_RD_MASK, &buff[1],len);
	printf("mask=0x%x\n", buff[1]);
	es_spi_ioctl(SPI_IOC_RD_MACADDR0, &buff[2],len);
	printf("mac0=0x%x\n", buff[2]);
	es_spi_ioctl(SPI_IOC_RD_MACADDR1, &buff[3],len);
	printf("mac1=0x%x\n", buff[3]);
	es_spi_ioctl(SPI_IOC_RD_GW0, &buff[4],len);
	printf("gw0=0x%x\n", buff[4]);
	es_spi_ioctl(SPI_IOC_RD_GW1, &buff[5],len);
	printf("gw1=0x%x\n", buff[5]);
	es_spi_ioctl(SPI_IOC_RD_GW2, &buff[6],len);
	printf("gw2=0x%x\n", buff[6]);
	es_spi_ioctl(SPI_IOC_RD_GW3, &buff[7],len);
	printf("gw3=0x%x\n", buff[7]);
	es_spi_ioctl(SPI_IOC_RD_DNS0, &buff[8],len);
	printf("dns0=0x%x\n", buff[8]);
	es_spi_ioctl(SPI_IOC_RD_DNS1, &buff[9],len);
	printf("dns1=0x%x\n", buff[9]);
	es_spi_ioctl(SPI_IOC_RD_DNS2, &buff[10],len);
	printf("dns2=0x%x\n", buff[10]);
	es_spi_ioctl(SPI_IOC_RD_DNS3, &buff[11],len);
	printf("dns3=0x%x\n", buff[11]);
	buff[12]=0x8090;
	es_spi_ioctl(SPI_IOC_INVALID_PORT_WR, &buff[12],len);
	es_spi_ioctl(SPI_IOC_INVALID_PORT_RD, &buff[13],len);
	printf("port=0x%x\n", buff[13]);

}
ALIOS_CLI_CMD_REGISTER(test_es_spi_info, spi_info, test_es_spi_info);
#endif

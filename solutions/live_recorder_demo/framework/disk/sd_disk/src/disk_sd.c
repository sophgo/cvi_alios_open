 /*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name:disk.c
 * function:
 * 1、sd hotplug;
 * 2、Sd card read/write rate test.
 * 3、get sd card Capacity
 * 4、format sd card
 */
#include <yoc/init.h>
#include <aos/cli.h>
#include <ff.h>
#include <soc.h>
#include "sys/prctl.h"
#include "pthread.h"
#include "semaphore.h"
#include "pinctrl-mars.h"
#include "disk_sd.h"
#include "fatfs_vfs.h"
#include "vfs.h"

#define SDPATH SD_FATFS_MOUNTPOINT"/"
#define SDHOTPLUGNUMBER 5
static STORAGE_STATUS_E g_storageStatus = STORAGE_NOEXIT;
static sem_t g_sem_sd_irq;
static pthread_t sd_hotplug_thread;
static csi_gpio_t  s_sdcd_irqgpio = {0};
static int (*g_sdhotplugcb[SDHOTPLUGNUMBER])(int);
#define WT_DB_PLUG      10	//Waiting time for Insert Debounce
#define WT_DB_UNPLUG    10	//Waiting time for Unplug Debounce
#define PIN_MASK(_gpio_num) (1 << _gpio_num)
#define GPIO_CD_PIN     13

static int _sdhci_cd_debounce(int waitms, int pre_plug_status)
{
	int curr_plug_status = pre_plug_status;
	int difftime = 0;

	while(difftime < waitms)
	{
		mdelay(1);
		difftime++;

		curr_plug_status = csi_gpio_read(&s_sdcd_irqgpio, PIN_MASK(GPIO_CD_PIN));

		if (pre_plug_status != curr_plug_status)
			break;
	}
	return curr_plug_status;
}

void *sd_hotplug_process(void *param)
{
	static int sd_status = 0;
	int ret = 0;
	prctl(PR_SET_NAME, "sd_hp_thread");

	for(;;)
	{
		sem_wait(&g_sem_sd_irq);

LABEL_LOOP_HOTPLUG:

		ret = csi_gpio_read(&s_sdcd_irqgpio, PIN_MASK(GPIO_CD_PIN));
		if (ret == 0) {
			if (0 != _sdhci_cd_debounce(WT_DB_PLUG, ret))
				goto LABEL_LOOP_HOTPLUG;

			csi_gpio_irq_mode(&s_sdcd_irqgpio, PIN_MASK(GPIO_CD_PIN), GPIO_IRQ_MODE_RISING_EDGE);
			if (sd_status == 1)
				vfs_fatfs_unregister();

			printf("SD inserted, register fatfs\n");
			ret = vfs_fatfs_register();
			if (ret != 0)
				sd_status = 0;
			else
				sd_status = 1;
		} else {
			if (0 == _sdhci_cd_debounce(WT_DB_UNPLUG, ret))
				goto LABEL_LOOP_HOTPLUG;

			csi_gpio_irq_mode(&s_sdcd_irqgpio, PIN_MASK(GPIO_CD_PIN), GPIO_IRQ_MODE_FALLING_EDGE);
			
			printf("SD removed, unregister fatfs\n");
			ret = vfs_fatfs_unregister();
			if (ret == 0)
				sd_status = 0;
		}
		csi_gpio_irq_enable(&s_sdcd_irqgpio, PIN_MASK(GPIO_CD_PIN), true);
		for(int i = 0; i < SDHOTPLUGNUMBER; i++) {
			if(g_sdhotplugcb[i]) {
				g_sdhotplugcb[i](sd_status);
			}
		}
	}
}

int sd_hotplugRegisterCb(int (*cb)(int))
{
    for(int i = 0; i < SDHOTPLUGNUMBER; i++) {
        if(g_sdhotplugcb[i] == NULL) {
            g_sdhotplugcb[i] = cb;
            return 0;
        }
    }
    return -1;
}

static void sd_irq_sem_hotplug()
{
	csi_gpio_irq_enable(&s_sdcd_irqgpio, PIN_MASK(GPIO_CD_PIN), false);
	sem_post(&g_sem_sd_irq);
}

int8_t app_sd_detect_check(void)
{
	int sd_detect = 0;

	PINMUX_CONFIG(SD0_CD, XGPIOA_13);
	csi_gpio_init(&s_sdcd_irqgpio, 0);
	csi_gpio_mode(&s_sdcd_irqgpio , PIN_MASK(GPIO_CD_PIN), GPIO_MODE_PULLUP);
	csi_gpio_dir(&s_sdcd_irqgpio , PIN_MASK(GPIO_CD_PIN), GPIO_DIRECTION_INPUT);
	sd_detect = csi_gpio_read(&s_sdcd_irqgpio, PIN_MASK(GPIO_CD_PIN));
	if (!sd_detect)
		csi_gpio_irq_mode(&s_sdcd_irqgpio, PIN_MASK(GPIO_CD_PIN), GPIO_IRQ_MODE_RISING_EDGE);
	else
		csi_gpio_irq_mode(&s_sdcd_irqgpio, PIN_MASK(GPIO_CD_PIN), GPIO_IRQ_MODE_FALLING_EDGE);
	csi_gpio_attach_callback(&s_sdcd_irqgpio, sd_irq_sem_hotplug, NULL);
	csi_gpio_irq_enable(&s_sdcd_irqgpio, PIN_MASK(GPIO_CD_PIN), true);

	sem_init(&g_sem_sd_irq, 0, 0);

	int ret = pthread_create(&sd_hotplug_thread, NULL, sd_hotplug_process, NULL);
	if (ret != 0) {
		printf("Sd hotplug pthread create failed, ret:%d\n", ret);
		return ret;
	}

	return (sd_detect == 0) ? 1 : 0;
}

STORAGE_STATUS_E DISK_SdGetStatus(CVI_BOOL isCheck)
{
    if(isCheck == CVI_TRUE) {
        if(access(SDPATH,F_OK) == 0){
            g_storageStatus = STORAGE_NORMAL;
        }
        else
            g_storageStatus = STORAGE_NOEXIT;
    }
    return g_storageStatus;
}

CVI_S32 DISK_SdFormat()
{
    CVI_S32 s32Ret = CVI_SUCCESS;
    CVI_U8 opt = FM_FAT32;
    printf("%s\n",__func__);

    if(access(SDPATH,F_OK) != 0) {
        printf("%s sdpath  umount\n",__func__);
        return CVI_FAILURE;
    }
    g_storageStatus = STORAGE_FORMAT;
    char *buffer = aos_malloc(FF_MAX_SS);
    if(buffer) {
        s32Ret = f_mkfs("", opt , 0, buffer, FF_MAX_SS);
        aos_free(buffer);
        if(s32Ret != CVI_SUCCESS) {
            g_storageStatus = STORAGE_UNFORMAT;
            printf("The DISK_SdFormat err ret %d\n",s32Ret);
        } else {
            printf("The DISK_SdFormat success\n");
            g_storageStatus = STORAGE_NORMAL;
        }
    }
    return s32Ret;
}

CVI_S32 DISK_SdGetCapacity(CVI_DOUBLE *Total,CVI_DOUBLE *Free)
{
    if(DISK_SdGetStatus(CVI_TRUE)!= STORAGE_NORMAL) {
        return 0;
    }
    struct aos_statfs fsinfo;
    aos_statfs(SDPATH, &fsinfo);

    *Total = (CVI_DOUBLE)fsinfo.f_blocks *fsinfo.f_bsize /(1024 *1024);//单位MB
    *Free = (CVI_DOUBLE)fsinfo.f_bfree *fsinfo.f_bsize /(1024 *1024);//单位MB
    return CVI_SUCCESS;
}


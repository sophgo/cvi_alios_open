/*
 * Copyright (C) 2019-2020 Alibaba Group Holding Limited
 */
#include "app_config.h"
#include "app_main.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <soc.h>
#include <drv/tick.h>
#include <boot.h>
#include <boot_wrapper.h>
#include <yoc/partition.h>
#include <yoc/partition_device.h>
#include <decompress.h>
#include <mmio.h>
#include "sensor.h"
#if defined(CONFIG_OTA_AB) && (CONFIG_OTA_AB > 0)
#include <bootab.h>
#endif
#if defined(CONFIG_SECUREBOOT) && (CONFIG_SECUREBOOT > 0)
#include <security.h>
#endif

#define LOAD_DDR_ADRR  0x84000000
#define MAX_YOC_SIZE   0xA00000
#define DECOMP_DST_SIZE (16 << 20)

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
extern unsigned long long timer_get_boot_us(void);
#define TIME_RECORDS_ADDR 0x0e000010

/**
boot_main *(unsigned short *)(TIME_RECORDS_ADDR+16);
load_yoc_start  *(unsigned short *)(TIME_RECORDS_ADDR+18);
load_yoc_end  *(unsigned short *)(TIME_RECORDS_ADDR+20);
boot_end * (unsigned short *)(TIME_RECORDS_ADDR+22);
 */

void boot_load_and_jump(void)
{
    const char *jump_to = "prim";
    unsigned long static_addr;
    unsigned long load_addr;
    unsigned long *p_load_addr = &load_addr;
    uint32_t image_size;
    partition_t part;
    partition_info_t *part_info;

#if defined(CONFIG_USE_COMPRESS) && (CONFIG_USE_COMPRESS > 0)
    int yoc_size = 0;
    int i = 0;
    char *str = malloc(10);
    unsigned long uzip_addr = DECOMP_BUF_ADDR;
    enum COMPRESS_TYPE comp_type = COMP_LZ4;
    p_load_addr = &uzip_addr;
#endif

#if defined(CONFIG_OTA_AB) && (CONFIG_OTA_AB > 0)
    char *ab;
    char *pre_slot;
    char j2part[16];

    ab = (char *)bootab_get_current_ab();
    if (ab == NULL) {
        printf("select valid prim[ab] failed, panic !\n");
        goto fail;
    }
    snprintf(j2part, sizeof(j2part), "prim%s", ab);
    pre_slot = (char *)bootab_fallback(ab);
    if (pre_slot) {
        printf("#########fallback to prim%s\n", pre_slot);
        snprintf(j2part, sizeof(j2part), "prim%s", pre_slot);
    }

    printf("load img & jump to [%s]\n", j2part);
    part = partition_open(j2part);
    part_info = partition_info_get(part);
    if (part_info == NULL) {
        goto fail;
    }
    partition_close(part);
    if (mtb_image_verify(j2part)) {
        goto fail;
    }
    jump_to = j2part;
#endif /*CONFIG_OTA_AB*/

#if defined(CONFIG_DEBUG) && (CONFIG_DEBUG > 0)
    printf("load img & jump to [%s]\n", jump_to);
#endif
    *(unsigned short *)(TIME_RECORDS_ADDR+18) = DIV_ROUND_UP(timer_get_boot_us(), 1000);
    part = partition_open(jump_to);
    part_info = partition_info_get(part);

    static_addr = part_info->start_addr + part_info->base_addr;
    load_addr = part_info->load_addr;
    image_size = part_info->image_size;
#if defined(CONFIG_OTA_AB) && (CONFIG_OTA_AB > 0)
    image_size = part_info->length;
#endif

#if defined(CONFIG_SECUREBOOT) && (CONFIG_SECUREBOOT > 0)
    if (security_is_tee_enabled()) {
        image_size = security_pad(image_size);
    }
#endif

#if defined(CONFIG_DEBUG) && (CONFIG_DEBUG > 0)
    printf("load&jump 0x%lx,0x%lx,%d\n", static_addr, load_addr, image_size);
#endif
    if (static_addr != load_addr) {
#if defined(CONFIG_DEBUG) && (CONFIG_DEBUG > 0)
        printf("start copy %d bytes\n", image_size);
        extern uint32_t csi_tick_get_ms(void);
        printf("##cur_ms:%d\n", csi_tick_get_ms());
#endif
    partition_read(part, 0, (void *)(*p_load_addr), image_size);
    //partition_flash_read(NULL, static_addr, (void *)uzip_addr, image_size);

#if defined(CONFIG_SECUREBOOT) && (CONFIG_SECUREBOOT > 0)
    int ret = dec_verify_image((void *)(*p_load_addr), image_size, 0, NULL);
    if (ret < 0) {
        printf("decrypt and verify %s failed : %d\n", jump_to, ret);
        return;
    }
#endif

#if defined(CONFIG_USE_COMPRESS) && (CONFIG_USE_COMPRESS > 0)
    if (memcmp((uint8_t *)(uzip_addr), "ULZ4", 4) == 0) {
        while(*(uint8_t *)(uzip_addr + 4 + i) != 0x0a)
            i++;
        memcpy(str,(uint8_t *)(uzip_addr + 4),i);
        yoc_size = atoi(str);
#if defined(CONFIG_DEBUG) && (CONFIG_DEBUG > 0)
        printf("str value: %s yoc_size: %d\n",str,yoc_size);
#endif
        uzip_addr = uzip_addr + 5 + i;

        if (comp_type) {
            size_t dst_size = DECOMP_DST_SIZE;
            void *dst = (void *)(uintptr_t)load_addr;

            int ret = decompress(dst, &dst_size, (void *)uzip_addr, yoc_size,comp_type);
            if (ret < 0) {
            	printf("Failed to decompress (%d/%lu)\n", ret, dst_size);
            }
        }
    }else{
        memcpy((void *)load_addr,(void *)uzip_addr,image_size);
        //if (partition_split_and_copy(part, 0)) {
        //   DBG_PRINT("decompress and copy prim bin failed.\n");
        //   //goto fail;
        //}
    }
#endif


#if defined(CONFIG_DEBUG) && (CONFIG_DEBUG > 0)
        printf("##cur_ms:%d\n", csi_tick_get_ms());
        printf("all copy over..\n");
#endif
    } else {
        printf("xip...\n");
    }
    partition_close(part);
    *(unsigned short *)(TIME_RECORDS_ADDR+20) = DIV_ROUND_UP(timer_get_boot_us(), 1000);
    csi_dcache_clean_invalid();
    csi_icache_invalid();

    void (*func)(void);
    if (memcmp((uint8_t *)(load_addr + 4), "CSKY", 4) == 0) {
#if defined(CONFIG_DEBUG) && (CONFIG_DEBUG > 0)
        printf("j m\n");
#endif
        func = (void (*)(void))((unsigned long *)load_addr);
    } else {
        func = (void (*)(void))(*(unsigned long *)load_addr);
    }
    printf("j 0x%08lx\n", (unsigned long)(*func));
    // printf("##cur_ms:%d\n", csi_tick_get_ms());

    csi_tick_uninit();

    *(unsigned short *)(TIME_RECORDS_ADDR+22) = DIV_ROUND_UP(timer_get_boot_us(), 1000);
    (*func)();
    while(1) {;}

#if defined(CONFIG_OTA_AB) && (CONFIG_OTA_AB > 0)
fail:
    printf("jump failed. reboot.\n");
    mdelay(200);
    boot_sys_reboot();
#endif
}

extern void clk_axi4_reconfig(void);

void boot_sys_reboot(void)
{
    extern void drv_reboot(void);
    drv_reboot();
}

int main(int argc, char *argv[0])
{
    int ret;

    *(unsigned short *)(TIME_RECORDS_ADDR+16) = DIV_ROUND_UP(timer_get_boot_us(), 1000);

    extern void board_yoc_init(void);
    board_yoc_init();

#if CONFIG_SENSOR_QUICK_STARTUP
    sensor_init();
#endif
#if CONFIG_QUICK_STARTUP_SUPPORT
    clk_axi4_reconfig();
#endif
    ret = boot_main(argc, argv);
    return ret;
}

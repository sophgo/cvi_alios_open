#include "common_yocsystem.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <aos/kv.h>
#include <debug/dbg.h>
#include <aos/cli.h>
#include <uservice/uservice.h>
#include <yoc/partition.h>
#include <yoc/init.h>
#include <drv/dma.h>
#include "board.h"
#include "fatfs_vfs.h"
#include "littlefs_vfs.h"
#include "vfs.h"
#include "disk_sd.h"
#include "vfs_api.h"
#include "littlefs_vfs.h"
#include "debug/debug_cli_cmd.h"
#if (CONFIG_SUPPORT_RAMFS == 1)
#include "ramfs.h"
#endif
void YOC_SYSTEM_FsVfsInit()
{
    int ret;
	//int fatfs_en = 0;

    ret = aos_vfs_init();
	if (ret != 0)
		LOGE("app", "aos_vfs_init failed(%d).\n", ret);
	//fatfs_en = app_sd_detect_check();
	//LOGE("app", fatfs_en ? "fatfs enable.\n" : "fatfs disable.\n");
#if CONFIG_SUPPORT_NORFLASH
    ret = partition_init();
    if(ret >= 0)
    {
        if((ret = vfs_lfs_register_with_path("lfs", "/music")) != 0) {
            printf("vfs_lfs_register_with_path %d failed \r\n", ret);
        } else {
            printf("vfs_lfs_register_with_path lfs success \r\n");
        }
        ret = aos_kv_init("kv");
        if (ret != 0) {
            LOGE("app", "aos_kv_init failed(%d)", ret);
        } else
            LOGE("app", "aos_kv_init succeed.");
    }
    else
        printf("partition_init failed! ret = %d\n" , ret);
#endif
#if (CONFIG_SUPPORT_RAMFS == 1)
    ramfs_register("/ramfs");
#endif
}


static void stduart_init(void)
{
    extern void console_init(int idx, uint32_t baud, uint16_t buf_size);
    console_init(CONSOLE_UART_IDX, 115200, 512);
}

extern void  cxx_system_init(void);
void YOC_SYSTEM_Init(void)
{
    cxx_system_init();
    board_init();
    stduart_init();
    printf("###YoC###[%s,%s]\n", __DATE__, __TIME__);
    //printf("cpu clock is %dHz\n", soc_get_cpu_freq(0));
}

extern void cli_reg_cmd_adb_config(void);
extern void cli_reg_cmd_ps(void);
extern void cli_reg_cmd_iperf(void);
extern void cli_reg_cmd_ls(void);
extern void cli_reg_cmd_ifconfig(void);
extern void cli_reg_cmd_ping(void);
extern void cli_reg_cmd_cat(void);
void YOC_SYSTEM_ToolInit()
{
    //CLI放后面注册 先起流媒体
    aos_cli_init();
    debug_cli_cmd_init();
    cli_reg_cmd_ps();
    cli_reg_cmd_ls();
    cli_reg_cmd_iperf();
    cli_reg_cmd_ifconfig();
    cli_reg_cmd_ping();
#if (CONFIG_SUPPORT_TEST_TYDB_ADB == 1)
    cli_reg_cmd_adb_config();
#endif
    cli_reg_cmd_cat();
    ulog_init();
    aos_set_log_level(AOS_LL_DEBUG);
    event_service_init(NULL);
}

void cli_dump_isp_param(int argc,char **argv)
{
    char * pIspString = getenv("ISPPQPARAM");
    if(pIspString) {
        printf("********************************\n");
        printf("%s\n",pIspString);
        printf("********************************\n");
    } else {
        printf("%s pIspString is null\n",__func__);
    }
}
ALIOS_CLI_CMD_REGISTER(cli_dump_isp_param,dump_isp_pqparm,dump_isp_pqparm);

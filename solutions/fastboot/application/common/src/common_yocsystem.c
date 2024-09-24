#include "common_yocsystem.h"
#include <stdbool.h>
#include <debug/dbg.h>
#include <aos/cli.h>
#include <yoc/init.h>
#include <drv/dma.h>
#include "board.h"
#include "debug/debug_cli_cmd.h"

#if CONFIG_DUALOS_NO_CROP
#include <aos/kv.h>
#include <uservice/uservice.h>
#include <yoc/partition.h>
#include "fatfs_vfs.h"
#include "littlefs_vfs.h"
#include "vfs.h"
#include "disk_sd.h"
#include "vfs_api.h"
#include "littlefs_vfs.h"

void YOC_SYSTEM_FsVfsInit()
{
    int ret;
	int fatfs_en = 0;

    ret = aos_vfs_init();
	if (ret != 0)
		LOGE("app", "aos_vfs_init failed(%d).\n", ret);

	fatfs_en = app_sd_detect_check();
	LOGE("app", fatfs_en ? "fatfs enable.\n" : "fatfs disable.\n");

	if (fatfs_en == 1) {
		ret = vfs_fatfs_register();
		if (ret != 0)
			LOGE("app", "fatfs register failed(%d).\n", ret);
	}

#if CONFIG_SUPPORT_NORFLASH
    ret = partition_init();
    if(ret >= 0)
    {
        //ret = vfs_lfs_register("spiffs");
        //if (ret != 0) {
        //    LOGE("app", "lfs register failed(%d)", ret);
        //} else
        //    LOGE("app", "lfs register succeed.");
        ret = aos_kv_init("kv");
        if (ret != 0) {
            LOGE("app", "aos_kv_init failed(%d)", ret);
        } else
            LOGE("app", "aos_kv_init succeed.");
    }
    else
        printf("partition_init failed! ret = %d\n" , ret);
#endif

}
#endif //#if CONFIG_DUALOS_NO_CROP
#ifdef CONFIG_C906L_DMA_ENABLE
static csi_dma_t dma;
#endif
static void stduart_init(void)
{
    extern void console_init(int idx, uint32_t baud, uint16_t buf_size);
    console_init(1, 115200, 512);
}

extern void  cxx_system_init(void);
void YOC_SYSTEM_Init(void)
{
    cxx_system_init();
    board_init();
    stduart_init();
    printf("###YoC###[%s,%s]\n", __DATE__, __TIME__);
    //printf("cpu clock is %dHz\n", soc_get_cpu_freq(0));
#ifdef CONFIG_C906L_DMA_ENABLE
    csi_dma_init(&dma, 0);
#endif
}

extern void cli_reg_cmd_ps(void);

void YOC_SYSTEM_ToolInit()
{
    //CLI放后面注册 先起流媒体
    aos_cli_init();
    debug_cli_cmd_init();
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

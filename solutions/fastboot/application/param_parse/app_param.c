#include "app_param.h"
#include "custom_param.h"
#include "minIni.h"
#include "sensor_cfg.h"
#include "cvi_ipcm.h"
#include "drv/tick.h"
#include "cvi_comm_rc.h"

#define APP_PARAM_STRING_NAME_LEN 64
#define DUMP_DEBUG 0

typedef struct _PARTITION_CHECK_HAED_S
{
    unsigned int magic_number;
    unsigned int crc;
    unsigned int header_ver;
    unsigned int length;
    unsigned int reserved;
    unsigned int package_number;
    unsigned int package_length[10];
}PARTITION_CHECK_HAED_S;

static int _param_check_head(unsigned char * buffer)
{
    PARTITION_CHECK_HAED_S * pstParam = (PARTITION_CHECK_HAED_S *)(buffer);
    if (buffer == NULL) {
        return -1;
    }
    if (pstParam->magic_number != 0xDEADBEEF) {
        return -1;
    }
    return 0;
}

static unsigned int stringHash(char * str)
{
    unsigned int hash = 5381;
    while(*str) {
        hash += (hash << 5) + (*str++);
    }
    return (hash & 0x7FFFFFFF);
}

unsigned int as32SensorType[SNS_TYPE_WDR_BUTT] = {0};
unsigned int as32PixFormat[PIXEL_FORMAT_MAX] = {0};
unsigned int as32VideoFormat[VIDEO_FORMAT_MAX] = {0};
unsigned int as32AspetRatio[ASPECT_RATIO_MAX] = {0};
unsigned int as32ViVpssMode[VI_VPSS_MODE_BUTT] = {0};
unsigned int as32VpssMode[VPSS_MODE_BUTT] = {0};
unsigned int as32VpssAenInput[VPSS_INPUT_BUTT] = {0};
unsigned int as32VpssCrop[VPSS_CROP_BUTT] = {0};
unsigned int as32ViWdrMode[WDR_MODE_MAX] = {0};
unsigned int as32CompressMode[COMPRESS_MODE_BUTT] = {0};
unsigned int as32Mclk[RX_MAC_CLK_BUTT] = {0};
unsigned int as32MclkFreq[CAMPLL_FREQ_NUM] = {0};
unsigned int as32ModId[CVI_ID_BUTT] = {0};
unsigned int as32EnType[PT_BUTT] = {0};
unsigned int as32VencRcMode[VENC_RC_MODE_BUTT] = {0};
unsigned int as32VencBindMode[VENC_BIND_BUTT] = {0};

/*
* Customized selection of media modules that need to be init in AliOS
*/
static int parse_ModuleParam(char* buffer)
{
    char _section[64] = {0};
    if (buffer == NULL) {
        return -1;
    }

    PARAM_MODULE_CFG_S * pstModuleCtx = PARAM_GET_MODULE_CFG();
    if (pstModuleCtx == NULL) {
        aos_debug_printf("PARAM_MODULE_CFG_S is null!\r\n");
        return -1;
    }

    memset(_section, 0, sizeof(_section));
    snprintf(_section, sizeof(_section), "module_config");

    pstModuleCtx->alios_sys_mode = (CVI_BOOL)ini_getl(_section, "alios_sys_mode", 0, buffer);
    pstModuleCtx->alios_vi_mode = (CVI_BOOL)ini_getl(_section, "alios_vi_mode", 0, buffer);
    pstModuleCtx->alios_vpss_mode = (CVI_BOOL)ini_getl(_section, "alios_vpss_mode", 0, buffer);
    pstModuleCtx->alios_venc_mode = (CVI_BOOL)ini_getl(_section, "alios_venc_mode", 0, buffer);
    pstModuleCtx->alios_vo_mode = (CVI_BOOL)ini_getl(_section, "alios_vo_mode", 0, buffer);

#if (DUMP_DEBUG == 1)
        aos_debug_printf("alios_sys_mode= %d\r\n", pstModuleCtx->alios_sys_mode);
        aos_debug_printf("alios_vi_mode= %d\r\n", pstModuleCtx->alios_vi_mode);
        aos_debug_printf("alios_vpss_mode= %d\r\n", pstModuleCtx->alios_vpss_mode);
        aos_debug_printf("alios_venc_mode= %d\r\n", pstModuleCtx->alios_venc_mode);
        aos_debug_printf("alios_vo_mode= %d\r\n", pstModuleCtx->alios_vo_mode);
#endif

    return 0;
}

/*
* Parse param.bin parameters into local PARAM_SYS_CFG_S
*/
static int parse_SysParam(char * buffer)
{
    char _section[64] = {0};
    char _stringPrase[64] = {0};
    if (buffer == NULL ) {
        return -1;
    }

    PARAM_SYS_CFG_S *pstSysCtx = PARAM_GET_SYS_CFG();
    if (pstSysCtx == NULL){
        aos_debug_printf("PARAM_SYS_CFG_S is null!\r\n");
        return -1;
    }

    /**
    * Parse PARAM_SBM_CFG_S in PARAM_SYS_CFG_S
    */
    pstSysCtx->u8SbmCnt = ini_getl("slice_config", "slice_cnt", 0, buffer);
    if (pstSysCtx->u8SbmCnt != 0) {
        pstSysCtx->pstSbmCfg = (PARAM_SBM_CFG_S *)malloc(sizeof(PARAM_SBM_CFG_S)*pstSysCtx->u8SbmCnt);
        if (pstSysCtx->pstSbmCfg) {
            memset(pstSysCtx->pstSbmCfg, 0, sizeof(PARAM_SBM_CFG_S)*pstSysCtx->u8SbmCnt);
        } else {
#if (DUMP_DEBUG == 1)
            aos_debug_printf("pstSysCtx->pstSbmCfg is NULL.\r\n");
#endif
            return -1;
        }
    }
    for (int i = 0; i < pstSysCtx->u8SbmCnt; i++) {
        snprintf(_section, sizeof(_section), "slice_buff_%d", i);
        pstSysCtx->pstSbmCfg[i].bEnable = ini_getl(_section, "bEnable", 0, buffer);
        if (pstSysCtx->pstSbmCfg[i].bEnable == 1) {
            pstSysCtx->pstSbmCfg[i].s32SbmGrp      = ini_getl(_section, "s32SbmGrp", 0, buffer);
            pstSysCtx->pstSbmCfg[i].s32SbmChn      = ini_getl(_section, "s32SbmChn", 0, buffer);
            pstSysCtx->pstSbmCfg[i].s32WrapBufLine = ini_getl(_section, "s32WrapBufLine", 0, buffer);
            pstSysCtx->pstSbmCfg[i].s32WrapBufSize = ini_getl(_section, "s32WrapBufSize", 0, buffer);
        }
    }

    /**
    * Parse PARAM_SWITCH_CFG_S in PARAM_SYS_CFG_S
    */
    pstSysCtx->stSwitchCfg.bMipiSwitchEnable = ini_getl("mipi_switch", "mipi_switch_mode", 0, buffer);
    if (pstSysCtx->stSwitchCfg.bMipiSwitchEnable == 1) {
        snprintf(_section, sizeof(_section), "mipi_switch");
        pstSysCtx->stSwitchCfg.u32MipiSwitchGpioIdx = ini_getl(_section, "mipiswitch_gpioid", 0, buffer);
        pstSysCtx->stSwitchCfg.u32MipiSwitchGpio = ini_getl(_section, "mipiswitch_gpio", 0, buffer);
        pstSysCtx->stSwitchCfg.bMipiSwitchPull = ini_getl(_section, "mipiswitch_pull", 0, buffer);
        pstSysCtx->stSwitchCfg.u32SwitchPipe0 = ini_getl(_section, "switch_pipe0", 0, buffer);
        pstSysCtx->stSwitchCfg.u32SwitchPipe1 = ini_getl(_section, "switch_pipe1", 0, buffer);
    }

    /**
    * Parse VPSS_MODE_S (VPSS_MODE_DUAL / VPSS_MODE_SINGLE)
    */
    ini_gets("vpss_mode", "enMode", "", _stringPrase, sizeof(_stringPrase), buffer);
    long _vpss_mode = stringHash(_stringPrase);
    for (int i = 0; i < VPSS_MODE_BUTT; i++) {
        if (_vpss_mode == as32VpssMode[i]) {
            pstSysCtx->stVPSSMode.enMode = i;
            break;
        }
    }

    /**
    * Parse VPSS_INPUT_E inside VPSS_MODE_S
    */
    for (int i = 0; i < VPSS_IP_NUM ; i++) {
        snprintf(_section, sizeof(_section), "vpss_dev%d", i);
        memset(_stringPrase, 0 , sizeof(_stringPrase));
        ini_gets(_section, "aenInput", "", _stringPrase, sizeof(_stringPrase), buffer);
        long aenInput = stringHash(_stringPrase);
        for (int j = 0; j < sizeof(as32VpssAenInput)/sizeof(as32VpssAenInput[0]); j++) {
            if (aenInput == as32VpssAenInput[j]) {
                pstSysCtx->stVPSSMode.aenInput[i] = j;
            }
        }
        pstSysCtx->stVPSSMode.ViPipe[i] = ini_getl(_section, "ViPipe", 0, buffer);
    }

    /**
    * Parse VI_VPSS_MODE_E inside VI_VPSS_MODE_S
    * For dual sensor
    */
    for (int i = 0; i < VI_MAX_PIPE_NUM; i++) {
        snprintf(_section, sizeof(_section), "vi_vpss_mode_%d", i);
        int vi_vpss_pipe = ini_getl(_section, "vi_vpss_pipe", 0, buffer);
        memset(_stringPrase, 0, sizeof(_stringPrase));
        ini_gets(_section, "enMode", "", _stringPrase, sizeof(_stringPrase), buffer);
        long vi_vpss_mode = stringHash(_stringPrase);
        for (int j = 0; j < sizeof(as32ViVpssMode)/sizeof(as32ViVpssMode[0]); j++) {
            if (vi_vpss_mode == as32ViVpssMode[j]) {
                pstSysCtx->stVIVPSSMode.aenMode[vi_vpss_pipe] = j;
            }
        }
    }

    /**
    * Parse PARAM_VB_CFG_S
    */
    pstSysCtx->u8VbPoolCnt = ini_getl("vb_config", "vb_pool_cnt", 0, buffer);
    if (pstSysCtx->u8VbPoolCnt != 0) {
        pstSysCtx->pstVbPool = (PARAM_VB_CFG_S *)malloc(sizeof(PARAM_VB_CFG_S)*pstSysCtx->u8VbPoolCnt);
    }
    for (int i = 0; i < pstSysCtx->u8VbPoolCnt; i++) {
        snprintf(_section, sizeof(_section), "vb_pool_%d", i);
        pstSysCtx->pstVbPool[i].u16width = ini_getl(_section, "frame_width", 0, buffer);
        pstSysCtx->pstVbPool[i].u16height = ini_getl(_section, "frame_height", 0, buffer);
        memset(_stringPrase, 0 , sizeof(_stringPrase));
        ini_gets(_section, "compress_mode", "", _stringPrase, sizeof(_stringPrase), buffer);
        long _compress_mode = stringHash(_stringPrase);
        for (unsigned int j = 0; j < sizeof(as32CompressMode)/sizeof(as32CompressMode[0]); j++) {
            if (_compress_mode == as32CompressMode[j]) {
                pstSysCtx->pstVbPool[i].enCmpMode = j;
                break;
            }
        }
        memset(_stringPrase, 0 , sizeof(_stringPrase));
        ini_gets(_section, "frame_fmt", "", _stringPrase, sizeof(_stringPrase), buffer);
        long _frame_fmt = stringHash(_stringPrase);
        for (unsigned int j = 0; j < sizeof(as32PixFormat)/sizeof(as32PixFormat[0]); j++) {
            if (_frame_fmt == as32PixFormat[j]) {
                pstSysCtx->pstVbPool[i].fmt = j;
                break;
            }
        }
        pstSysCtx->pstVbPool[i].enBitWidth = DATA_BITWIDTH_8;//default DATA_BITWIDTH_8
        pstSysCtx->pstVbPool[i].u8VbBlkCnt = ini_getl(_section, "blk_cnt", 0, buffer);
    }

#if (DUMP_DEBUG == 1)
    aos_debug_printf("The pstSysCtx->u8SbmCnt is %d \r\n", pstSysCtx->u8SbmCnt);
    for (int i = 0; i < pstSysCtx->u8SbmCnt; i++) {
        aos_debug_printf("The pstSysCtx->pstSbmCfg[%d].bEnable is %d \r\n", i, pstSysCtx->pstSbmCfg[i].bEnable);
        aos_debug_printf("The pstSysCtx->pstSbmCfg[%d].s32SbmGrp is %d \r\n", i, pstSysCtx->pstSbmCfg[i].s32SbmGrp);
        aos_debug_printf("The pstSysCtx->pstSbmCfg[%d].s32SbmChn is %d \r\n", i, pstSysCtx->pstSbmCfg[i].s32SbmChn);
        aos_debug_printf("The pstSysCtx->pstSbmCfg[%d].s32WrapBufLine is %d \r\n", i, pstSysCtx->pstSbmCfg[i].s32WrapBufLine);
        aos_debug_printf("The pstSysCtx->pstSbmCfg[%d].s32WrapBufSize is %d \r\n", i, pstSysCtx->pstSbmCfg[i].s32WrapBufSize);
    }
    aos_debug_printf("The pstSysCtx->bMipiSwitchEnable is %d \r\n", pstSysCtx->stSwitchCfg.bMipiSwitchEnable);
    aos_debug_printf("The pstSysCtx->u32MipiSwitchGpioIdx is %d \r\n", pstSysCtx->stSwitchCfg.u32MipiSwitchGpioIdx);
    aos_debug_printf("The pstSysCtx->u32MipiSwitchGpio is %d \r\n", pstSysCtx->stSwitchCfg.u32MipiSwitchGpio);
    aos_debug_printf("The pstSysCtx->bMipiSwitchPull is %d \r\n", pstSysCtx->stSwitchCfg.bMipiSwitchPull);
    aos_debug_printf("The pstSysCtx->u32SwitchPipe0 is %d \r\n", pstSysCtx->stSwitchCfg.u32SwitchPipe0);
    aos_debug_printf("The pstSysCtx->u32SwitchPipe1 is %d \r\n", pstSysCtx->stSwitchCfg.u32SwitchPipe1);
    aos_debug_printf("The pstSysCtx->u8ViCnt is %d \r\n", pstSysCtx->u8ViCnt);
    aos_debug_printf("The pstSysCtx->stVIVPSSMode.aenMode[0] is %d \r\n", pstSysCtx->stVIVPSSMode.aenMode[0]);
    aos_debug_printf("The pstSysCtx->stVIVPSSMode.aenMode[1] is %d \r\n", pstSysCtx->stVIVPSSMode.aenMode[1]);
    aos_debug_printf("The pstSysCtx->stVIVPSSMode.aenMode[3] is %d \r\n", pstSysCtx->stVIVPSSMode.aenMode[3]);
    aos_debug_printf("The pstSysCtx->stVPSSMode.enMode is %d \r\n", pstSysCtx->stVPSSMode.enMode);
    aos_debug_printf("The pstSysCtx->stVPSSMode.ViPipe[0] is %d \r\n", pstSysCtx->stVPSSMode.ViPipe[0]);
    aos_debug_printf("The pstSysCtx->stVPSSMode.ViPipe[1] is %d \r\n", pstSysCtx->stVPSSMode.ViPipe[1]);
    aos_debug_printf("The pstSysCtx->stVPSSMode.aenInput[0] is %d \r\n", pstSysCtx->stVPSSMode.aenInput[0]);
    aos_debug_printf("The pstSysCtx->stVPSSMode.aenInput[1] is %d \r\n", pstSysCtx->stVPSSMode.aenInput[1]);
    aos_debug_printf("The pstSysCtx->u8VbPoolCnt is %d \r\n", pstSysCtx->u8VbPoolCnt);
    for (int i = 0 ; i < pstSysCtx->u8VbPoolCnt; i++) {
        aos_debug_printf("pstSysCtx->pstVbPool[%d].u16width %d \r\n", i,pstSysCtx->pstVbPool[i].u16width);
        aos_debug_printf("pstSysCtx->pstVbPool[%d].u16height %d \r\n", i,pstSysCtx->pstVbPool[i].u16height);
        aos_debug_printf("pstSysCtx->pstVbPool[%d].fmt %d \r\n", i,pstSysCtx->pstVbPool[i].fmt);
        aos_debug_printf("pstSysCtx->pstVbPool[%d].enBitWidth %d \r\n", i,pstSysCtx->pstVbPool[i].enBitWidth);
        aos_debug_printf("pstSysCtx->pstVbPool[%d].enCmpMode %d \r\n", i,pstSysCtx->pstVbPool[i].enCmpMode);
        aos_debug_printf("pstSysCtx->pstVbPool[%d].u8VbBlkCnt %d \r\n", i,pstSysCtx->pstVbPool[i].u8VbBlkCnt);
    }
#endif

    return 0;
}

static int parse_ViParam(char * buffer)
{
    char _section[64] = {0};
    char _key[64] = {0};
    char _stringPrase[64] = {0};

    if (buffer == NULL) {
        return -1;
    }

    PARAM_VI_CFG_S * pstViCtx = PARAM_GET_VI_CFG();
    if (pstViCtx == NULL) {
        aos_debug_printf("PARAM_VI_CFG_S is null!\r\n");
        return -1;
    }

    /**
    * Parse sensor_cnt in VI context
    */
    pstViCtx->u32WorkSnsCnt = ini_getl("vi_config", "sensor_cnt", 0, buffer);
    if (pstViCtx->u32WorkSnsCnt == 0 ) {
#if (DUMP_DEBUG == 1)
        aos_debug_printf("parse ViParam error \r\n");
#endif
        return -1;
    }

    /**
    * malloc sub structure in PARAM_VI_CFG_S
    * PARAM_VI_CFG_S is the VI context
    */
    pstViCtx->pstSensorCfg = (PARAM_SNS_CFG_S *)malloc(sizeof(PARAM_SNS_CFG_S) * pstViCtx->u32WorkSnsCnt);
    pstViCtx->pstDevInfo = (PARAM_DEV_CFG_S *)malloc(sizeof(PARAM_DEV_CFG_S) * pstViCtx->u32WorkSnsCnt);
    pstViCtx->pstPipeInfo = (PARAM_PIPE_CFG_S *)malloc(sizeof(PARAM_PIPE_CFG_S) * pstViCtx->u32WorkSnsCnt);
    pstViCtx->pstChnInfo = (PARAM_CHN_CFG_S *)malloc(sizeof(PARAM_CHN_CFG_S) * pstViCtx->u32WorkSnsCnt);
    pstViCtx->pstIspCfg = (PARAM_ISP_CFG_S *)malloc(sizeof(PARAM_ISP_CFG_S) * pstViCtx->u32WorkSnsCnt);
    if (pstViCtx->pstSensorCfg) {
        memset(pstViCtx->pstSensorCfg, 0 , sizeof(PARAM_SNS_CFG_S) * pstViCtx->u32WorkSnsCnt);
    }
    if (pstViCtx->pstDevInfo) {
        memset(pstViCtx->pstDevInfo, 0 , sizeof(PARAM_DEV_CFG_S) * pstViCtx->u32WorkSnsCnt);
    }
    if (pstViCtx->pstPipeInfo) {
        memset(pstViCtx->pstPipeInfo, 0 , sizeof(PARAM_PIPE_CFG_S) * pstViCtx->u32WorkSnsCnt);
    }
    if (pstViCtx->pstChnInfo) {
        memset(pstViCtx->pstChnInfo, 0 , sizeof(PARAM_CHN_CFG_S) * pstViCtx->u32WorkSnsCnt);
    }
    if (pstViCtx->pstIspCfg) {
        memset(pstViCtx->pstIspCfg, 0 , sizeof(PARAM_ISP_CFG_S) * pstViCtx->u32WorkSnsCnt);
    }

    /**
    * Parse sub structure in PARAM_VI_CFG_S
    * PARAM_VI_CFG_S is the VI context
    */
    for(unsigned int i = 0; i < pstViCtx->u32WorkSnsCnt; i++) {
        /**
        * Parse pstSensorCfg in VI context
        */
        snprintf(_section, sizeof(_section), "sensor_config%d", i );
        pstViCtx->pstSensorCfg[i].s32Framerate = (int)(ini_getl(_section, "framerate", 0, buffer));
        pstViCtx->pstSensorCfg[i].MipiDev = ini_getl(_section, "mipi_dev", 0, buffer);
        pstViCtx->pstSensorCfg[i].s32BusId = ini_getl(_section, "bus_id", 0, buffer);
        pstViCtx->pstSensorCfg[i].s8I2cDev = pstViCtx->pstSensorCfg[i].s32BusId;
        pstViCtx->pstSensorCfg[i].s32I2cAddr = (int)ini_getl(_section, "sns_i2c_addr", 0, buffer);
        for (unsigned int j = 0; j < (MIPI_LANE_NUM + 1); j++) {
            snprintf(_key, sizeof(_key), "laneid%d", j);
            pstViCtx->pstSensorCfg[i].as16LaneId[j] = (short int)ini_getl(_section, _key, -1, buffer);
            snprintf(_key, sizeof(_key), "swap%d", j);
            pstViCtx->pstSensorCfg[i].as8PNSwap[j] = (char)ini_getl(_section, _key, -1, buffer);
        }
        pstViCtx->pstSensorCfg[i].bMclkEn = (short int)ini_getl(_section, "mclk_en", -1, buffer);
        if (pstViCtx->pstSensorCfg[i].bMclkEn == CVI_TRUE) {
            memset(_stringPrase, 0, sizeof(_stringPrase));
            ini_gets(_section, "mclk", "", _stringPrase, sizeof(_stringPrase), buffer);
            unsigned int _mclk = stringHash(_stringPrase);
            for (unsigned int j = 0 ; j < sizeof(as32Mclk)/sizeof(as32Mclk[0]); j++) {
                if (_mclk == as32Mclk[j]) {
                    pstViCtx->pstSensorCfg[i].u8Mclk = j;
                    break;
                }
            }
            memset(_stringPrase, 0, sizeof(_stringPrase));
            ini_gets(_section, "mclk_freq", "", _stringPrase, sizeof(_stringPrase), buffer);
            unsigned int _mclk_freq = stringHash(_stringPrase);
            for (unsigned int j = 0 ; j < sizeof(as32MclkFreq)/sizeof(as32MclkFreq[0]); j++) {
                if (_mclk_freq == as32MclkFreq[j]) {
                    pstViCtx->pstSensorCfg[i].u8MclkFreq = j;
                    break;
                }
            }
            pstViCtx->pstSensorCfg[i].u8MclkCam = (short int)ini_getl(_section, "mclk_cam", -1, buffer);
        }
        pstViCtx->pstSensorCfg[i].u8Orien = (short int)ini_getl(_section, "orien", -1, buffer);
        pstViCtx->pstSensorCfg[i].bHwSync = (short int)ini_getl(_section, "hw_sync", -1, buffer);
        pstViCtx->pstSensorCfg[i].u32Rst_port_idx = (short int)ini_getl(_section, "rst_port_idx", -1, buffer);
        pstViCtx->pstSensorCfg[i].u32Rst_pol = (short int)ini_getl(_section, "rst_pol", -1, buffer);
        pstViCtx->pstSensorCfg[i].u32Rst_pin = (short int)ini_getl(_section, "rst_pin", -1, buffer);
        pstViCtx->pstSensorCfg[i].u8Rotation = (short int)ini_getl(_section, "rotation", 0, buffer);
        memset(_stringPrase, 0 , sizeof(_stringPrase));
        ini_gets(_section, "sns_type", "", _stringPrase, sizeof(_stringPrase), buffer);
        unsigned int _sns_type = stringHash(_stringPrase);
        for (unsigned int j = 0; j < sizeof(as32SensorType)/sizeof(as32SensorType[0]); j++) {
            if (_sns_type == as32SensorType[j]) {
                pstViCtx->pstSensorCfg[i].enSnsType = j;
                set_sensor_type(j, i);
                break;
            }
        }

        /**
        * Parse pstChnInfo in VI context
        */
        snprintf(_section, sizeof(_section), "vi_cfg_dev%d", i);
        pstViCtx->pstChnInfo[i].s32ChnId =  (int)ini_getl(_section, "videv", -1, buffer);
        memset(_stringPrase, 0 , sizeof(_stringPrase));
        ini_gets(_section, "wdrmode", "", _stringPrase, sizeof(_stringPrase), buffer);
        int _wdrmode = stringHash(_stringPrase);
        for (unsigned int j = 0; j < sizeof(as32ViWdrMode)/sizeof(as32ViWdrMode[0]); j++) {
            if (_wdrmode == as32ViWdrMode[j]) {
                pstViCtx->pstChnInfo[i].enWDRMode = j;
                break;
            }
        }

        /**
        * Parse pstPipeInfo in VI context
        */
        snprintf(_section, sizeof(_section), "vi_cfg_pipe%d", i);
        for (unsigned int j = 0; j < VI_MAX_PIPE_NUM; j++) {
            snprintf(_key, sizeof(_key), "apipe%d", j);
            pstViCtx->pstPipeInfo[i].pipe[j] =  (int)ini_getl(_section, _key, -1, buffer);
        }

        /**
        * Parse pstChnInfo in VI context
        */
        snprintf(_section, sizeof(_section), "vi_cfg_chn%d", i);
        pstViCtx->pstChnInfo[i].enDynamicRange = DYNAMIC_RANGE_SDR8;//default
        pstViCtx->pstChnInfo[i].bYuvBypassPath = 0;//yuv sensor use
        pstViCtx->pstChnInfo[i].f32Fps = (int)ini_getl(_section, "fps", -1, buffer);
        pstViCtx->pstChnInfo[i].u32Width = (int)ini_getl(_section, "width", 0, buffer);
        pstViCtx->pstChnInfo[i].u32Height = (int)ini_getl(_section, "height", 0, buffer);
        pstViCtx->pstChnInfo[i].enVideoFormat = VIDEO_FORMAT_LINEAR;
        memset(_stringPrase, 0 , sizeof(_stringPrase));
        ini_gets(_section, "compress_mode", "", _stringPrase, sizeof(_stringPrase), buffer);
        int _compress_mode = stringHash(_stringPrase);
        for (unsigned int j = 0; j < sizeof(as32CompressMode)/sizeof(as32CompressMode[0]); j++) {
            if (_compress_mode == as32CompressMode[j]) {
                pstViCtx->pstChnInfo[i].enCompressMode = j;
                break;
            }
        }

        /**
        * Parse pstIspCfg in VI context
        */
        pstViCtx->pstIspCfg[i].s8ByPassNum = ini_getl(_section, "bypassnum", 5 , buffer);

        /**
        * Parse bFastConverge in VI context
        */
        pstViCtx->bFastConverge = (bool)ini_getl("fastConverge", "enable", 0, buffer);
        if (pstViCtx->bFastConverge == CVI_TRUE) {
            pstViCtx->pstIspCfg[i].s8FastConvergeAvailableNode = ini_getl("fastConverge", "availableNode", 0, buffer);
            for (int j = 0; j < pstViCtx->pstIspCfg[i].s8FastConvergeAvailableNode; j++) {
                snprintf(_key, sizeof(_key), "firstfFrLuma%d", j);
                pstViCtx->pstIspCfg[i].as16firstFrLuma[j] = ini_getl("fastConverge", _key, 0, buffer);
                snprintf(_key, sizeof(_key), "targetBv%d", j);
                pstViCtx->pstIspCfg[i].as16targetBv[j] = ini_getl("fastConverge", _key, 0, buffer);
            }
        }
    }

#if (DUMP_DEBUG == 1)
    aos_debug_printf("pstViCtx->u32WorkSnsCnt is %d \r\n", pstViCtx->u32WorkSnsCnt);
    aos_debug_printf("pstViCtx->u32IspSceneNum is %d \r\n", pstViCtx->u32IspSceneNum);
    aos_debug_printf("pstViCtx->bFastConverge is %d \r\n", pstViCtx->bFastConverge);
    for (int i = 0; i < pstViCtx->u32WorkSnsCnt; i++) {
        aos_debug_printf("pstViCtx->pstSensorCfg[%d].s32Framerate %d \r\n", i,pstViCtx->pstSensorCfg[i].s32Framerate);
        aos_debug_printf("pstViCtx->pstSensorCfg[%d].enWDRMode %d \r\n", i,pstViCtx->pstSensorCfg[i].enWDRMode);
        aos_debug_printf("pstViCtx->pstSensorCfg[%d].s8I2cDev %d \r\n", i,pstViCtx->pstSensorCfg[i].s8I2cDev);
        aos_debug_printf("pstViCtx->pstSensorCfg[%d].s32BusId %d \r\n", i,pstViCtx->pstSensorCfg[i].s32BusId);
        aos_debug_printf("pstViCtx->pstSensorCfg[%d].s32I2cAddr %d \r\n", i,pstViCtx->pstSensorCfg[i].s32I2cAddr);
        aos_debug_printf("pstViCtx->pstSensorCfg[%d].MipiDev %d \r\n", i,pstViCtx->pstSensorCfg[i].MipiDev);
        aos_debug_printf("pstViCtx->pstSensorCfg[%d].bMclkEn %d \r\n", i,pstViCtx->pstSensorCfg[i].bMclkEn);
        aos_debug_printf("pstViCtx->pstSensorCfg[%d].u8Mclk %d \r\n", i,pstViCtx->pstSensorCfg[i].u8Mclk);
        aos_debug_printf("pstViCtx->pstSensorCfg[%d].u8MclkCam %d \r\n", i,pstViCtx->pstSensorCfg[i].u8MclkCam);
        aos_debug_printf("pstViCtx->pstSensorCfg[%d].u8MclkFreq %d \r\n", i,pstViCtx->pstSensorCfg[i].u8MclkFreq);
        aos_debug_printf("pstViCtx->pstSensorCfg[%d].u8Orien %d \r\n", i,pstViCtx->pstSensorCfg[i].u8Orien);
        aos_debug_printf("pstViCtx->pstSensorCfg[%d].bHwSync %d \r\n", i,pstViCtx->pstSensorCfg[i].bHwSync);
        aos_debug_printf("pstViCtx->pstSensorCfg[%d].u32Rst_port_idx %d \r\n", i,pstViCtx->pstSensorCfg[i].u32Rst_port_idx);
        aos_debug_printf("pstViCtx->pstSensorCfg[%d].u32Rst_pin %d \r\n", i,pstViCtx->pstSensorCfg[i].u32Rst_pin);
        aos_debug_printf("pstViCtx->pstSensorCfg[%d].u32Rst_pol %d \r\n", i,pstViCtx->pstSensorCfg[i].u32Rst_pol);
        aos_debug_printf("pstViCtx->pstSensorCfg[%d].u8Rotation %d \r\n", i,pstViCtx->pstSensorCfg[i].u8Rotation);
        aos_debug_printf("pstViCtx->pstSensorCfg[%d].enSnsType %d \r\n", i,pstViCtx->pstSensorCfg[i].enSnsType);


        //vi param
        aos_debug_printf("pstViCtx->pstChnInfo[%d].s32ChnId %d \r\n", i,pstViCtx->pstChnInfo[i].s32ChnId);
        aos_debug_printf("pstViCtx->pstChnInfo[%d].enWDRMode %d \r\n", i,pstViCtx->pstChnInfo[i].enWDRMode);
        aos_debug_printf("pstViCtx->pstChnInfo[%d].enDynamicRange %d \r\n", i,pstViCtx->pstChnInfo[i].enDynamicRange);
        aos_debug_printf("pstViCtx->pstChnInfo[%d].bYuvBypassPath %d \r\n", i,pstViCtx->pstChnInfo[i].bYuvBypassPath);
        aos_debug_printf("pstViCtx->pstChnInfo[%d].f32Fps %d \r\n", i,(int)pstViCtx->pstChnInfo[i].f32Fps);
        aos_debug_printf("pstViCtx->pstChnInfo[%d].u32Width %d \r\n", i,pstViCtx->pstChnInfo[i].u32Width);
        aos_debug_printf("pstViCtx->pstChnInfo[%d].u32Height %d \r\n", i,pstViCtx->pstChnInfo[i].u32Height);
        aos_debug_printf("pstViCtx->pstChnInfo[%d].enVideoFormat %d \r\n", i,pstViCtx->pstChnInfo[i].enVideoFormat);
        aos_debug_printf("pstViCtx->pstChnInfo[%d].enCompressMode %d \r\n", i,pstViCtx->pstChnInfo[i].enCompressMode);

        for (unsigned int j = 0; j < VI_MAX_PIPE_NUM; j++) {
            aos_debug_printf("pstViCtx->pstPipeInfo[%d].pipe[%d] %d \r\n", i, j, pstViCtx->pstPipeInfo[i].pipe[j]);
        }

        aos_debug_printf("pstViCtx->pstIspCfg[%d].s8FastConvergeAvailableNode %d \r\n", i,pstViCtx->pstIspCfg[i].s8FastConvergeAvailableNode);
        for (int j = 0; j < pstViCtx->pstIspCfg[i].s8FastConvergeAvailableNode; j++) {
                aos_debug_printf("pstViCtx->pstIspCfg[%d].as16firstFrLuma[%d] %d\r\n", i,j, pstViCtx->pstIspCfg[i].as16firstFrLuma[j]);
                aos_debug_printf("pstViCtx->pstIspCfg[%d].as16targetBv[%d] %d\r\n", i,j, pstViCtx->pstIspCfg[i].as16targetBv[j]);
        }
    }
#endif
    return 0;
}

/*
* Parse param.bin parameters into local PARAM_VPSS_CFG_S
*/
static int parse_VpssParam(char * buffer)
{
    char _section[64] = {0};
    char _stringPrase[64] = {0};
    long _pixel_fmt = 0;
    long _chn_pixel_fmt = 0;
    long _video_fmt = 0;
    long _aspect_ratio = 0;
    long _src_mod_id = 0;
    long _dst_mod_id = 0;

    if (buffer == NULL) {
        return -1;
    }

    PARAM_VPSS_CFG_S * pstVpssCtx = PARAM_GET_VPSS_CFG();
    if (pstVpssCtx == NULL) {
        aos_debug_printf("PARAM_VPSS_CFG_S is null!\r\n");
        return -1;
    }

    memset(_section, 0, sizeof(_section));
    snprintf(_section, sizeof(_section), "vpss_config");

    pstVpssCtx->u8GrpCnt = ini_getl(_section, "vpss_grp", 0, buffer);
    if (pstVpssCtx->u8GrpCnt == 0) {
#if (DUMP_DEBUG == 1)
        aos_debug_printf("Vpss GrpCnt=0. \n");
#endif
        return 0;
    }
    pstVpssCtx->pstVpssGrpCfg = (PARAM_VPSS_GRP_CFG_S *)malloc(sizeof(PARAM_VPSS_GRP_CFG_S)*pstVpssCtx->u8GrpCnt);
    if (pstVpssCtx->pstVpssGrpCfg) {
        memset(pstVpssCtx->pstVpssGrpCfg, 0, sizeof(PARAM_VPSS_GRP_CFG_S)*pstVpssCtx->u8GrpCnt);
    } else {
#if (DUMP_DEBUG == 1)
        aos_debug_printf("pstVpssCtx->pstVpssGrpCfg is NULL. \n");
#endif
        return -1;
    }

    for (unsigned int grp_idx = 0; grp_idx < pstVpssCtx->u8GrpCnt; grp_idx++) {
        memset(_section, 0, sizeof(_section));
        snprintf(_section, sizeof(_section), "vpssgrp%d", grp_idx);
        pstVpssCtx->pstVpssGrpCfg[grp_idx].bEnable = (bool)ini_getl(_section, "grp_enable", 0, buffer);
        if (pstVpssCtx->pstVpssGrpCfg[grp_idx].bEnable == CVI_FALSE)
            continue;
        pstVpssCtx->pstVpssGrpCfg[grp_idx].VpssGrp  = (VPSS_GRP)grp_idx;
        pstVpssCtx->pstVpssGrpCfg[grp_idx].u8ChnCnt = ini_getl(_section, "chn_cnt", 0, buffer);

        VPSS_GRP_ATTR_S *pstVpssGrpAttr = &pstVpssCtx->pstVpssGrpCfg[grp_idx].stVpssGrpAttr;
        memset(_stringPrase, 0, sizeof(_stringPrase));
        ini_gets(_section, "pixel_fmt", "", _stringPrase, sizeof(_stringPrase), buffer);
        _pixel_fmt = stringHash(_stringPrase);
        for (unsigned int i = 0 ; i < PIXEL_FORMAT_MAX; i++) {
            if (_pixel_fmt == as32PixFormat[i]) {
                pstVpssGrpAttr->enPixelFormat = i;
                break;
            }
        }
        pstVpssGrpAttr->stFrameRate.s32SrcFrameRate = ini_getl(_section, "src_framerate", 0, buffer);
        pstVpssGrpAttr->stFrameRate.s32DstFrameRate = ini_getl(_section, "dst_framerate", 0, buffer);
        pstVpssGrpAttr->u8VpssDev                   = ini_getl(_section, "vpss_dev", 0, buffer);
        pstVpssGrpAttr->u32MaxW                     = ini_getl(_section, "max_w", 0, buffer);
        pstVpssGrpAttr->u32MaxH                     = ini_getl(_section, "max_h", 0, buffer);

        VPSS_CROP_INFO_S *pstVpssGrpCropInfo = &pstVpssCtx->pstVpssGrpCfg[grp_idx].stVpssGrpCropInfo;
        pstVpssGrpCropInfo->bEnable = (bool)ini_getl(_section, "crop_en", 0, buffer);
        if (pstVpssGrpCropInfo->bEnable){
            memset(_stringPrase, 0, sizeof(_stringPrase));
            ini_gets(_section, "crop_coor", "", _stringPrase, sizeof(_stringPrase), buffer);
            long _crop_coor = stringHash(_stringPrase);
            for (unsigned int i = 0 ; i < VPSS_CROP_BUTT; i++) {
                if (_crop_coor == as32VpssCrop[i]) {
                    pstVpssGrpCropInfo->enCropCoordinate = i;
                    break;
                }
            }
            pstVpssGrpCropInfo->stCropRect.s32X      = ini_getl(_section, "crop_rect_x", 0, buffer);
            pstVpssGrpCropInfo->stCropRect.s32Y      = ini_getl(_section, "crop_rect_y", 0, buffer);
            pstVpssGrpCropInfo->stCropRect.u32Width  = ini_getl(_section, "crop_rect_w", 0, buffer);
            pstVpssGrpCropInfo->stCropRect.u32Height = ini_getl(_section, "crop_rect_h", 0, buffer);
        }

        pstVpssCtx->pstVpssGrpCfg[grp_idx].bBindMode = ini_getl(_section, "bind_mode", 0, buffer);
        if (pstVpssCtx->pstVpssGrpCfg[grp_idx].bBindMode) {
            memset(_stringPrase, 0, sizeof(_stringPrase));
            ini_gets(_section, "src_mod_id", "", _stringPrase, sizeof(_stringPrase), buffer);
            _src_mod_id = stringHash(_stringPrase);
            for (unsigned int i = 0 ; i < CVI_ID_BUTT; i++) {
                if (_src_mod_id == as32ModId[i]) {
                    pstVpssCtx->pstVpssGrpCfg[grp_idx].astChn[0].enModId = i;
                    break;
                }
            }
            pstVpssCtx->pstVpssGrpCfg[grp_idx].astChn[0].s32DevId = ini_getl(_section, "src_dev_id", 0, buffer);
            pstVpssCtx->pstVpssGrpCfg[grp_idx].astChn[0].s32ChnId = ini_getl(_section, "src_chn_id", 0, buffer);
            pstVpssCtx->pstVpssGrpCfg[grp_idx].astChn[1].enModId  = ini_getl(_section, "dst_mod_id", 0, buffer);
            memset(_stringPrase, 0, sizeof(_stringPrase));
            ini_gets(_section, "dst_mod_id", "", _stringPrase, sizeof(_stringPrase), buffer);
            _dst_mod_id = stringHash(_stringPrase);
            for (unsigned int i = 0 ; i < CVI_ID_BUTT; i++) {
                if (_dst_mod_id == as32ModId[i]) {
                    pstVpssCtx->pstVpssGrpCfg[grp_idx].astChn[1].enModId = i;
                    break;
                }
            }
            pstVpssCtx->pstVpssGrpCfg[grp_idx].astChn[1].s32DevId = ini_getl(_section, "dst_dev_id", 0, buffer);
            pstVpssCtx->pstVpssGrpCfg[grp_idx].astChn[1].s32ChnId = ini_getl(_section, "dst_chn_id", 0, buffer);
        }
        pstVpssCtx->pstVpssGrpCfg[grp_idx].s32BindVidev = ini_getl(_section, "bind_vi_dev", -1, buffer);
        pstVpssCtx->pstVpssGrpCfg[grp_idx].u8ViRotation = ini_getl(_section, "vi_rotation", 0, buffer);

        if (pstVpssCtx->pstVpssGrpCfg[grp_idx].u8ChnCnt == 0) {
#if (DUMP_DEBUG == 1)
            aos_debug_printf("Vpss Grp:%d, chnCnt:0. \n", grp_idx);
#endif
            continue;
        }
        pstVpssCtx->pstVpssGrpCfg[grp_idx].pstChnCfg =
            (PARAM_VPSS_CHN_CFG_S *)malloc(sizeof(PARAM_VPSS_CHN_CFG_S) * pstVpssCtx->pstVpssGrpCfg[grp_idx].u8ChnCnt);
        if (pstVpssCtx->pstVpssGrpCfg[grp_idx].pstChnCfg) {
            memset(pstVpssCtx->pstVpssGrpCfg[grp_idx].pstChnCfg, 0
                , sizeof(PARAM_VPSS_CHN_CFG_S) * pstVpssCtx->pstVpssGrpCfg[grp_idx].u8ChnCnt);
        } else {
#if (DUMP_DEBUG == 1)
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg is NULL. \n", grp_idx);
#endif
            return -1;
        }
        for (unsigned int chn_idx = 0; chn_idx < pstVpssCtx->pstVpssGrpCfg[grp_idx].u8ChnCnt; chn_idx++) {
            memset(_section, 0, sizeof(_section));
            snprintf(_section, sizeof(_section), "vpssgrp%d.chn%d", grp_idx, chn_idx);
            PARAM_VPSS_CHN_CFG_S *pstVpssChnCfg = &pstVpssCtx->pstVpssGrpCfg[grp_idx].pstChnCfg[chn_idx];
            pstVpssChnCfg->abChnEnable = ini_getl(_section, "chn_enable", 0, buffer);
            if (!pstVpssChnCfg->abChnEnable)
                continue;

            VPSS_CHN_ATTR_S *pstVpssChnAttr = &pstVpssCtx->pstVpssGrpCfg[grp_idx].pstChnCfg[chn_idx].stVpssChnAttr;
            pstVpssChnAttr->u32Width                    = ini_getl(_section, "width", 0, buffer);
            pstVpssChnAttr->u32Height                   = ini_getl(_section, "height", 0, buffer);

            memset(_stringPrase, 0, sizeof(_stringPrase));
            ini_gets(_section, "video_fmt", "", _stringPrase, sizeof(_stringPrase), buffer);
            _video_fmt = stringHash(_stringPrase);
            for (unsigned int i = 0 ; i < VIDEO_FORMAT_MAX; i++) {
                if (_video_fmt == as32VideoFormat[i]) {
                    pstVpssChnAttr->enVideoFormat = i;
                    break;
                }
            }

            memset(_stringPrase, 0, sizeof(_stringPrase));
            ini_gets(_section, "chn_pixel_fmt", "", _stringPrase, sizeof(_stringPrase), buffer);
            _chn_pixel_fmt = stringHash(_stringPrase);
            for (unsigned int i = 0 ; i < PIXEL_FORMAT_MAX; i++) {
                if (_chn_pixel_fmt == as32PixFormat[i]) {
                    pstVpssChnAttr->enPixelFormat = i;
                    break;
                }
            }
            pstVpssChnAttr->stFrameRate.s32SrcFrameRate = ini_getl(_section, "src_framerate", 0, buffer);
            pstVpssChnAttr->stFrameRate.s32DstFrameRate = ini_getl(_section, "dst_framerate", 0, buffer);
            pstVpssChnAttr->u32Depth                    = ini_getl(_section, "depth", 0, buffer);
            pstVpssChnAttr->bMirror                     = ini_getl(_section, "mirror", 0, buffer);
            pstVpssChnAttr->bFlip                       = ini_getl(_section, "flip", 0, buffer);

            memset(_stringPrase, 0, sizeof(_stringPrase));
            ini_gets(_section, "aspectratio", "", _stringPrase, sizeof(_stringPrase), buffer);
            _aspect_ratio = stringHash(_stringPrase);
            for (unsigned int i = 0 ; i < ASPECT_RATIO_MAX; i++) {
                if (_aspect_ratio == as32AspetRatio[i]) {
                    pstVpssChnAttr->stAspectRatio.enMode = i;
                    break;
                }
            }
            if (pstVpssChnAttr->stAspectRatio.enMode == ASPECT_RATIO_MANUAL) { /*ASPECT_RATIO_MANUAL*/
                pstVpssChnAttr->stAspectRatio.stVideoRect.s32X      = ini_getl(_section, "s32x", 0, buffer);
                pstVpssChnAttr->stAspectRatio.stVideoRect.s32Y      = ini_getl(_section, "s32y", 0, buffer);
                pstVpssChnAttr->stAspectRatio.stVideoRect.u32Width  = ini_getl(_section, "rec_width", 0, buffer);
                pstVpssChnAttr->stAspectRatio.stVideoRect.u32Height = ini_getl(_section, "rec_heigh", 0, buffer);
                pstVpssChnAttr->stAspectRatio.bEnableBgColor        = (bool)ini_getl(_section, "en_color", 0, buffer);
                if (pstVpssChnAttr->stAspectRatio.bEnableBgColor != CVI_FALSE)
                    pstVpssChnAttr->stAspectRatio.u32BgColor = ini_getl(_section, "color", 0, buffer);
            }
            pstVpssChnAttr->stNormalize.bEnable = (bool)ini_getl(_section, "normalize", 0, buffer);

            VPSS_CROP_INFO_S *pstVpssChnCropInfo = &pstVpssCtx->pstVpssGrpCfg[grp_idx].pstChnCfg[chn_idx].stVpssChnCropInfo;
            pstVpssChnCropInfo->bEnable = (bool)ini_getl(_section, "crop_en", 0, buffer);
            if (pstVpssChnCropInfo->bEnable) {
                pstVpssChnCropInfo->enCropCoordinate     = ini_getl(_section, "crop_coor", 0, buffer);
                pstVpssChnCropInfo->stCropRect.s32X      = ini_getl(_section, "crop_rect_x", 0, buffer);
                pstVpssChnCropInfo->stCropRect.s32Y      = ini_getl(_section, "crop_rect_y", 0, buffer);
                pstVpssChnCropInfo->stCropRect.u32Width  = ini_getl(_section, "crop_rect_w", 0, buffer);
                pstVpssChnCropInfo->stCropRect.u32Height = ini_getl(_section, "crop_rect_h", 0, buffer);
            }

            pstVpssChnCfg->u8VpssAttachEnable = ini_getl(_section, "attach_en", 0, buffer);
            if (pstVpssChnCfg->u8VpssAttachEnable) {
                pstVpssChnCfg->u8VpssAttachId = ini_getl(_section, "attach_pool", 0, buffer);
            }

            pstVpssChnCfg->u8Rotation = ini_getl(_section, "rotation", 0, buffer);
        }
    }

#if (DUMP_DEBUG == 1)
    aos_debug_printf("pstVpssCtx->u8GrpCnt:%d. \r\n", pstVpssCtx->u8GrpCnt);
    for (int i = 0; i < pstVpssCtx->u8GrpCnt; i++) {
        aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].bEnable:%d. \r\n"
            , i, pstVpssCtx->pstVpssGrpCfg[i].bEnable);
        if (pstVpssCtx->pstVpssGrpCfg[i].bEnable) {
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].VpssGrp:%d. \r\n"
                , i, pstVpssCtx->pstVpssGrpCfg[i].VpssGrp);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].stVpssGrpAttr.enPixelFormat:%d. \r\n"
                , i, pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpAttr.enPixelFormat);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].stVpssGrpAttr.stFrameRate:%d. \r\n"
                , i, pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpAttr.stFrameRate.s32SrcFrameRate);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].stVpssGrpAttr.dst_framerate:%d. \r\n"
                , i, pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpAttr.stFrameRate.s32DstFrameRate);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].stVpssGrpAttr.u8VpssDev:%d. \r\n"
                , i, pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpAttr.u8VpssDev);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].stVpssGrpAttr.u32MaxW:%d. \r\n"
                , i, pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpAttr.u32MaxW);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].stVpssGrpAttr.u32MaxH:%d. \r\n"
                , i, pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpAttr.u32MaxH);

            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].stVpssGrpCropInfo.bEnable:%d. \r\n"
                , i, pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpCropInfo.bEnable);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].stVpssGrpCropInfo.enCropCoordinate:%d. \r\n"
                , i, pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpCropInfo.enCropCoordinate);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].stVpssGrpCropInfo.stCropRect.s32X:%d. \r\n"
                , i, pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpCropInfo.stCropRect.s32X);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].stVpssGrpCropInfo.stCropRect.s32Y:%d. \r\n"
                , i, pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpCropInfo.stCropRect.s32Y);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].stVpssGrpCropInfo.stCropRect.u32Width:%d. \r\n"
                , i, pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpCropInfo.stCropRect.u32Width);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].stVpssGrpCropInfo.stCropRect.u32Height:%d. \r\n"
                , i, pstVpssCtx->pstVpssGrpCfg[i].stVpssGrpCropInfo.stCropRect.u32Height);

            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].bBindMode:%d. \r\n", i, pstVpssCtx->pstVpssGrpCfg[i].bBindMode);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].s32BindVidev:%d. \r\n", i, pstVpssCtx->pstVpssGrpCfg[i].s32BindVidev);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].astChn[0].enModId:%d. \r\n", i, pstVpssCtx->pstVpssGrpCfg[i].astChn[0].enModId);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].astChn[0].s32DevId:%d. \r\n", i, pstVpssCtx->pstVpssGrpCfg[i].astChn[0].s32DevId);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].astChn[0].s32ChnId:%d. \r\n", i, pstVpssCtx->pstVpssGrpCfg[i].astChn[0].s32ChnId);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].astChn[1].enModId:%d. \r\n", i, pstVpssCtx->pstVpssGrpCfg[i].astChn[1].enModId);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].astChn[1].s32DevId:%d. \r\n", i, pstVpssCtx->pstVpssGrpCfg[i].astChn[1].s32DevId);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].astChn[1].s32ChnId:%d. \r\n", i, pstVpssCtx->pstVpssGrpCfg[i].astChn[1].s32ChnId);

            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].u8ChnCnt:%d. \r\n", i, pstVpssCtx->pstVpssGrpCfg[i].u8ChnCnt);
        }
        for (int j = 0; j < pstVpssCtx->pstVpssGrpCfg[i].u8ChnCnt; j++) {
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].abChnEnable:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].abChnEnable);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.u32Width:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.u32Width);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.u32Height:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.u32Height);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.enVideoFormat:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.enVideoFormat);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.enPixelFormat:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.enPixelFormat);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.stFrameRate.s32SrcFrameRate:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.stFrameRate.s32SrcFrameRate);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.stFrameRate.s32DstFrameRate:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.stFrameRate.s32DstFrameRate);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.u32Depth:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.u32Depth);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.bMirror:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.bMirror);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.bFlip:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.bFlip);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.stAspectRatio.enMode:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.stAspectRatio.enMode);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.stAspectRatio.stVideoRect.s32X:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.stAspectRatio.stVideoRect.s32X);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.stAspectRatio.stVideoRect.s32Y:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.stAspectRatio.stVideoRect.s32Y);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.stAspectRatio.stVideoRect.u32Width:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.stAspectRatio.stVideoRect.u32Width);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.stAspectRatio.stVideoRect.u32Height:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.stAspectRatio.stVideoRect.u32Height);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.stAspectRatio.bEnableBgColor:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.stAspectRatio.bEnableBgColor);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.stAspectRatio.u32BgColor:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.stAspectRatio.u32BgColor);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnAttr.stNormalize.bEnable:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnAttr.stNormalize.bEnable);

            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnCropInfo.bEnable:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnCropInfo.bEnable);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnCropInfo.enCropCoordinate:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnCropInfo.enCropCoordinate);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnCropInfo.stCropRect.s32X:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnCropInfo.stCropRect.s32X);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnCropInfo.stCropRect.s32Y:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnCropInfo.stCropRect.s32Y);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnCropInfo.stCropRect.u32Width:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnCropInfo.stCropRect.u32Width);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnCropInfo.stCropRect.u32Height:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnCropInfo.stCropRect.u32Height);

            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnBufWrap.bEnable:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnBufWrap.bEnable);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnBufWrap.u32BufLine:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnBufWrap.u32BufLine);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].stVpssChnBufWrap.u32WrapBufferSize:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].stVpssChnBufWrap.u32WrapBufferSize);

            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].u8VpssAttachEnable:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].u8VpssAttachEnable);
            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].u8VpssAttachId:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].u8VpssAttachId);

            aos_debug_printf("pstVpssCtx->pstVpssGrpCfg[%d].pstChnCfg[%d].u8Rotation:%d. \r\n"
                , i, j, pstVpssCtx->pstVpssGrpCfg[i].pstChnCfg[j].u8Rotation);
        }
    }
#endif

    return 0;
}

/*
* Parse param.bin into local PARAM_VENC_CFG_S
*/
static int parse_VencParam(char * buffer)
{
    char _section[64] = {0};
    char _stringPrase[64] = {0};
    long _bindMode = 0;
    long _modId = 0;
    long _enType = 0;
    long _rcMode = 0;

    if (buffer == NULL) {
        return -1;
    }

    PARAM_VENC_CFG_S * pstVencCtx = PARAM_GET_VENC_CFG();
    if (pstVencCtx == NULL) {
        aos_debug_printf("PARAM_VENC_CFG_S is null!\r\n");
        return -1;
    }

    memset(_section, 0, sizeof(_section));
    snprintf(_section, sizeof(_section), "venc_config");

    pstVencCtx->s32VencChnCnt = ini_getl(_section, "chn_num", 0, buffer);
    if (pstVencCtx->s32VencChnCnt == 0) {
#if (DUMP_DEBUG == 1)
        aos_debug_printf("venc chn=0. \r\n");
#endif
        return 0;
    }

    pstVencCtx->pstVencChnCfg =
        (PARAM_VENC_CHN_CFG_S *)malloc(sizeof(PARAM_VENC_CHN_CFG_S) * pstVencCtx->s32VencChnCnt);
    if (pstVencCtx->pstVencChnCfg) {
        memset(pstVencCtx->pstVencChnCfg, 0, sizeof(PARAM_VENC_CHN_CFG_S) * pstVencCtx->s32VencChnCnt);
    } else {
#if (DUMP_DEBUG == 1)
        aos_debug_printf("pstVencCtx->pstVencChnCfg is NULL. \n");
#endif
        return -1;
    }

    /*
    * Parse into PARAM_VENCCHN_PARAM_S
    */
    for (unsigned int venc_idx = 0; venc_idx < pstVencCtx->s32VencChnCnt; venc_idx++) {
        memset(_section, 0, sizeof(_section));
        snprintf(_section, sizeof(_section), "vencchn%d", venc_idx);

        PARAM_VENCCHN_PARAM_S *pstChnParam = &pstVencCtx->pstVencChnCfg[venc_idx].stChnParam;
        pstChnParam->bEnable = (bool)ini_getl(_section, "bEnable", 0, buffer);
        if (!pstChnParam->bEnable)
            continue;

        pstChnParam->u8VencChn           = venc_idx;

        /*
        * Parse venc bind relation
        */
        memset(_stringPrase, 0, sizeof(_stringPrase));
        ini_gets(_section, "bind_mode", "", _stringPrase, sizeof(_stringPrase), buffer);
        _bindMode = stringHash(_stringPrase);
        for (unsigned int i = 0 ; i < VENC_BIND_BUTT; i++) {
            if (_bindMode == as32VencBindMode[i]) {
                pstChnParam->enBindMode = i;
                break;
            }
        }

        /*
        * Parse when VENC_BIND_VPSS or VENC_BIND_VI
        */
        if (pstChnParam->enBindMode != VENC_BIND_DISABLE){
            /*Parse source channel*/
            memset(_stringPrase, 0, sizeof(_stringPrase));
            ini_gets(_section, "src_mod_id", "", _stringPrase, sizeof(_stringPrase), buffer);
            _modId = stringHash(_stringPrase);
            for (unsigned int i = 0 ; i < CVI_ID_BUTT; i++) {
                if (_modId == as32ModId[i]) {
                    pstChnParam->astChn[0].enModId = i;
                    break;
                }
            }
            pstChnParam->astChn[0].s32DevId = ini_getl(_section, "src_dev_id", 0, buffer);
            pstChnParam->astChn[0].s32ChnId = ini_getl(_section, "src_chn_id", 0, buffer);

            /*Parse destination channel*/
            memset(_stringPrase, 0, sizeof(_stringPrase));
            ini_gets(_section, "dst_mod_id", "", _stringPrase, sizeof(_stringPrase), buffer);
            _modId = stringHash(_stringPrase);
            for (unsigned int i = 0 ; i < CVI_ID_BUTT; i++) {
                if (_modId == as32ModId[i]) {
                    pstChnParam->astChn[1].enModId = i;
                    break;
                }
            }
            pstChnParam->astChn[1].s32DevId = ini_getl(_section, "dst_dev_id", 0, buffer);
            pstChnParam->astChn[1].s32ChnId = ini_getl(_section, "dst_chn_id", 0, buffer);
        }

        pstChnParam->u8Profile           = ini_getl(_section, "profile", 0, buffer);
        pstChnParam->u16Width            = ini_getl(_section, "width", 0, buffer);
        pstChnParam->u16Height           = ini_getl(_section, "height", 0, buffer);
        pstChnParam->u8EsBufQueueEn      = ini_getl(_section, "EsBufQueueEn", 0, buffer);

        memset(_stringPrase, 0, sizeof(_stringPrase));
        ini_gets(_section, "en_type", "", _stringPrase, sizeof(_stringPrase), buffer);
        _enType = stringHash(_stringPrase);
        for (unsigned int i = 0 ; i < PT_BUTT; i++) {
            if (_enType == as32EnType[i]) {
                pstChnParam->u16EnType = i;
                break;
            }
        }

        CVI_U32 temp_bufsize             = ini_getl(_section, "bitStreamBuf", 0, buffer);
        pstChnParam->u32BitStreamBufSize = (temp_bufsize << 10);
        pstChnParam->StreamTo            = ini_getl(_section, "send_to", 0, buffer);
        pstChnParam->u8EntropyEncModeI   = 0;
        pstChnParam->u8EntropyEncModeP   = 0;

        /*
        * Parse into PARAM_GOP_PARAM_S
        */
        PARAM_GOP_PARAM_S *pstGopParam = &pstVencCtx->pstVencChnCfg[venc_idx].stGopParam;
        pstGopParam->u16gopMode  = ini_getl(_section, "gop_mode", 0, buffer);
        pstGopParam->s8IPQpDelta = ini_getl(_section, "NormalP_IPQpDelta", 0, buffer);

        /*
        * Parse into PARAM_RC_PARAM_S
        */
        PARAM_RC_PARAM_S *pstRcParam = &pstVencCtx->pstVencChnCfg[venc_idx].stRcParam;
        pstRcParam->u16Gop               = ini_getl(_section, "gop", 0, buffer);
        pstRcParam->u8SrcFrameRate       = ini_getl(_section, "src_framerate", 0, buffer);
        pstRcParam->u8DstFrameRate       = ini_getl(_section, "dst_framerate", 0, buffer);
        pstRcParam->u16BitRate           = ini_getl(_section, "bit_rate", 0, buffer);
        pstRcParam->u8Qfactor            = ini_getl(_section, "Qfactor", 0, buffer);
        pstRcParam->u32MaxBitRate        = ini_getl(_section, "max_bitrate", 0, buffer);
        pstRcParam->u8VariFpsEn          = ini_getl(_section, "VariFpsEn", 0, buffer);
        pstRcParam->u8StartTime          = ini_getl(_section, "start_time", 0, buffer);
        memset(_stringPrase, 0, sizeof(_stringPrase));
        ini_gets(_section, "rc_mode", "", _stringPrase, sizeof(_stringPrase), buffer);
        _rcMode = stringHash(_stringPrase);
        for (unsigned int i = 0 ; i < VENC_RC_MODE_BUTT; i++) {
            if (_rcMode == as32VencRcMode[i]) {
                pstRcParam->u16RcMode = i;
                break;
            }
        }
        pstRcParam->u16FirstFrmstartQp   = ini_getl(_section, "firstFrmstartQp", 0, buffer);
        pstRcParam->u16InitialDelay      = ini_getl(_section, "initialDelay", 0, buffer);
        pstRcParam->u16ThrdLv            = ini_getl(_section, "ThrdLv", 0, buffer);
        pstRcParam->u16BgDeltaQp         = ini_getl(_section, "BgDeltaQp", 0, buffer);
        pstRcParam->u8MinIprop           = ini_getl(_section, "MinIprop", 0, buffer);
        pstRcParam->u8MaxIprop           = ini_getl(_section, "MaxIprop", 0, buffer);
        pstRcParam->u8MinIqp             = ini_getl(_section, "MinIQp", 0, buffer);
        pstRcParam->u8MaxIqp             = ini_getl(_section, "MaxIQp", 0, buffer);
        pstRcParam->u8MinQp              = ini_getl(_section, "MinQp", 0, buffer);
        pstRcParam->u8MaxQp              = ini_getl(_section, "MaxQp", 0, buffer);
        pstRcParam->u8MaxReEncodeTimes   = ini_getl(_section, "MaxReEncodeTimes", 0, buffer);
        pstRcParam->u8QpMapEn            = ini_getl(_section, "QpMapEn", 0, buffer);
        pstRcParam->u8ChangePos          = ini_getl(_section, "ChangePos", 0, buffer);
        pstRcParam->bSingleCore          = ini_getl(_section, "single_core", 0, buffer);
        pstRcParam->u32FixedIQp          = ini_getl(_section, "fixed_IQP", 0, buffer);
        pstRcParam->u32FixedPQp          = ini_getl(_section, "fixed_QPQ", 0, buffer);
        pstRcParam->u32Duration          = ini_getl(_section, "file_duration", 0, buffer);
        pstRcParam->s32MinStillPercent   = ini_getl(_section, "MinStillPercent", 0, buffer);
        pstRcParam->u32MaxStillQP        = ini_getl(_section, "MaxStillQP", 0, buffer);
        pstRcParam->u32MinStillPSNR      = ini_getl(_section, "MinStillPSNR", 0, buffer);
        pstRcParam->u32MotionSensitivity = ini_getl(_section, "MotionSensitivity", 0, buffer);
        pstRcParam->s32AvbrFrmLostOpen   = ini_getl(_section, "AvbrFrmLostOpen", 0, buffer);
        pstRcParam->s32AvbrFrmGap        = ini_getl(_section, "AvbrFrmGap", 0, buffer);
        pstRcParam->s32AvbrPureStillThr  = ini_getl(_section, "AvbrPureStillThr", 0, buffer);
        pstRcParam->s16Quality           = ini_getl(_section, "quality", 0, buffer);
        pstRcParam->s16MCUPerECS         = ini_getl(_section, "MCUPerECS", 0, buffer);
    }

#if (DUMP_DEBUG == 1)
    aos_debug_printf("pstVencCtx->s32VencChnCnt:%d \r\n", pstVencCtx->s32VencChnCnt);
    for (unsigned int venc_idx = 0; venc_idx < pstVencCtx->s32VencChnCnt; venc_idx++) {
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam: \r\n", venc_idx);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.VencChn:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.u8VencChn);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.bEnable:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.bEnable);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.bStart:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.bStart);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.enBindMode:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.enBindMode);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.astChn[0].enModId:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.astChn[0].enModId);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.astChn[0].s32DevId:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.astChn[0].s32DevId);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.astChn[0].s32ChnId:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.astChn[0].s32ChnId);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.astChn[1].enModId:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.astChn[1].enModId);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.astChn[1].s32DevId:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.astChn[1].s32DevId);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.astChn[1].s32ChnId:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.astChn[1].s32ChnId);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.u8Profile:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.u8Profile);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.u16Width:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.u16Width);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.u16Height:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.u16Height);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.u8EsBufQueueEn:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.u8EsBufQueueEn);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.u16EnType:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.u16EnType);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.u32BitStreamBufSize:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.u32BitStreamBufSize);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.StreamTo:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.StreamTo);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.u8EntropyEncModeI:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.u8EntropyEncModeI);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stChnParam.u8EntropyEncModeP:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stChnParam.u8EntropyEncModeP);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stGopParam.u16gopMode:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stGopParam.u16gopMode);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stGopParam.s8IPQpDelta:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stGopParam.s8IPQpDelta);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8DstFrameRate:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8DstFrameRate);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u16BitRate:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u16BitRate);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8Qfactor:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8Qfactor);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u32MaxBitRate:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u32MaxBitRate);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8VariFpsEn:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8VariFpsEn);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8StartTime:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8StartTime);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u16RcMode:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u16RcMode);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u16FirstFrmstartQp:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u16FirstFrmstartQp);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u16InitialDelay:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u16InitialDelay);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u16ThrdLv:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u16ThrdLv);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u16BgDeltaQp:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u16BgDeltaQp);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8MinIprop:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8MinIprop);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8MaxIprop:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8MaxIprop);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8MinIqp:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8MinIqp);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8MaxIqp:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8MaxIqp);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8MinQp:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8MinIqp);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8MaxQp:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8MaxQp);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8MaxReEncodeTimes:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8MaxReEncodeTimes);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8QpMapEn:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8QpMapEn);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u8ChangePos:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u8ChangePos);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.bSingleCore:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.bSingleCore);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u32FixedIQp:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u32FixedIQp);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u32FixedPQp:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u32FixedPQp);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u32Duration:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u32Duration);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.s32MinStillPercent:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.s32MinStillPercent);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u32MaxStillQP:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u32MaxStillQP);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u32MinStillPSNR:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u32MinStillPSNR);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.u32MotionSensitivity:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.u32MotionSensitivity);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.s32AvbrFrmLostOpen:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.s32AvbrFrmLostOpen);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.s32AvbrFrmGap:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.s32AvbrFrmGap);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.s32AvbrPureStillThr:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.s32AvbrPureStillThr);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.s16Quality:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.s16Quality);
        aos_debug_printf("pstVencCtx->pstVencChnCfg[%d].stRcParam.s16MCUPerECS:%d \r\n"
            , venc_idx, pstVencCtx->pstVencChnCfg[venc_idx].stRcParam.s16MCUPerECS);
    }
#endif

    /*
    * Parse into PARAM_ROI_PARAM_S
    */
    memset(_section, 0, sizeof(_section));
    snprintf(_section, sizeof(_section), "roi_config");
    pstVencCtx->s8RoiNumber = ini_getl(_section, "max_num", 0, buffer);
    if (pstVencCtx->s8RoiNumber > 0) {
        pstVencCtx->pstRoiParam =
            (PARAM_ROI_PARAM_S *)malloc(sizeof(PARAM_ROI_PARAM_S) * pstVencCtx->s8RoiNumber);
    }
    if (pstVencCtx->pstRoiParam) {
        memset(pstVencCtx->pstRoiParam, 0, sizeof(PARAM_ROI_PARAM_S) * pstVencCtx->s8RoiNumber);
    } else {
#if (DUMP_DEBUG == 1)
        aos_debug_printf("pstVencCtx->pstRoiParam is NULL. \n");
#endif
        return 0;
    }
    for (unsigned int roi_idx = 0; roi_idx < pstVencCtx->s8RoiNumber; roi_idx++) {
        memset(_section, 0, sizeof(_section));
        snprintf(_section, sizeof(_section), "roi_index%d", roi_idx);
        PARAM_ROI_PARAM_S *pstRoiParam = &pstVencCtx->pstRoiParam[roi_idx];
        pstRoiParam->bEnable          = (bool)ini_getl(_section, "roi_bEnable", 0, buffer);
        if (!pstRoiParam->bEnable)
            continue;
        pstRoiParam->u8Index          = roi_idx;
        pstRoiParam->VencChn          = ini_getl(_section, "venc", 0, buffer);
        pstRoiParam->bAbsQp           = ini_getl(_section, "absqp", 0, buffer);
        pstRoiParam->s32Qp            = ini_getl(_section, "qp", 0, buffer);
        pstRoiParam->s32X             = ini_getl(_section, "x", 0, buffer);
        pstRoiParam->s32Y             = ini_getl(_section, "y", 0, buffer);
        pstRoiParam->u32Width         = ini_getl(_section, "width", 0, buffer);
        pstRoiParam->u32Height        = ini_getl(_section, "height", 0, buffer);
    }

#if (DUMP_DEBUG == 1)
    aos_debug_printf("pstVencCtx->s8RoiNumber:%d \r\n", pstVencCtx->s8RoiNumber);
    for (unsigned int roi_idx = 0; roi_idx < pstVencCtx->s8RoiNumber; roi_idx++) {
        aos_debug_printf("pstVencCtx->pstRoiParam[%d].bEnable:%d \r\n"
            , roi_idx, pstVencCtx->pstRoiParam[roi_idx].bEnable);
        aos_debug_printf("pstVencCtx->pstRoiParam[%d].u8Index:%d \r\n"
            , roi_idx, pstVencCtx->pstRoiParam[roi_idx].u8Index);
        aos_debug_printf("pstVencCtx->pstRoiParam[%d].bAbsQp:%d \r\n"
            , roi_idx, pstVencCtx->pstRoiParam[roi_idx].bAbsQp);
        aos_debug_printf("pstVencCtx->pstRoiParam[%d].s32Qp:%d \r\n"
            , roi_idx, pstVencCtx->pstRoiParam[roi_idx].s32Qp);
        aos_debug_printf("pstVencCtx->pstRoiParam[%d].s32x:%d \r\n"
            , roi_idx, pstVencCtx->pstRoiParam[roi_idx].s32X);
        aos_debug_printf("pstVencCtx->pstRoiParam[%d].s32Y:%d \r\n"
            , roi_idx, pstVencCtx->pstRoiParam[roi_idx].s32Y);
        aos_debug_printf("pstVencCtx->pstRoiParam[%d].u32Width:%d \r\n"
            , roi_idx, pstVencCtx->pstRoiParam[roi_idx].u32Width);
        aos_debug_printf("pstVencCtx->pstRoiParam[%d].u32Height:%d \r\n"
            , roi_idx, pstVencCtx->pstRoiParam[roi_idx].u32Height);
    }
#endif

    return 0;
}

/*
* Accelerate parsed parameter string matching.
* Only need to parse important parameters here.
*/
int iniParseHashMapInit()
{
    //sensor type map
    as32SensorType[SONY_IMX327_MIPI_2M_30FPS_12BIT] = stringHash("SONY_IMX327_MIPI_2M_30FPS_12BIT");
    as32SensorType[SONY_IMX307_MIPI_2M_30FPS_12BIT] = stringHash("SONY_IMX307_MIPI_2M_30FPS_12BIT");
    as32SensorType[SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT] = stringHash("SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT");
    as32SensorType[GCORE_GC1054_MIPI_1M_30FPS_10BIT] = stringHash("GCORE_GC1054_MIPI_1M_30FPS_10BIT");
    as32SensorType[GCORE_GC2053_MIPI_2M_30FPS_10BIT] = stringHash("GCORE_GC2053_MIPI_2M_30FPS_10BIT");
    as32SensorType[GCORE_GC2093_MIPI_2M_30FPS_10BIT] = stringHash("GCORE_GC2093_MIPI_2M_30FPS_10BIT");
    as32SensorType[GCORE_GC02M1_MIPI_2M_30FPS_10BIT] = stringHash("GCORE_GC02M1_MIPI_2M_30FPS_10BIT");
    as32SensorType[GCORE_GC02M1_SLAVE_MIPI_2M_30FPS_10BIT] = stringHash("GCORE_GC02M1_SLAVE_MIPI_2M_30FPS_10BIT");
    as32SensorType[GCORE_GC4653_MIPI_4M_30FPS_10BIT] = stringHash("GCORE_GC4653_MIPI_4M_30FPS_10BIT");
    as32SensorType[GCORE_GC2053_1L_MIPI_2M_30FPS_10BIT] = stringHash("GCORE_GC2053_1L_MIPI_2M_30FPS_10BIT");
    as32SensorType[SMS_SC2331_1L_MIPI_2M_30FPS_10BIT] = stringHash("SMS_SC2331_1L_MIPI_2M_30FPS_10BIT");
    as32SensorType[SMS_SC2331_1L_SLAVE_MIPI_2M_30FPS_10BIT] = stringHash("SMS_SC2331_1L_SLAVE_MIPI_2M_30FPS_10BIT");
    as32SensorType[SMS_SC2331_1L_SLAVE1_MIPI_2M_30FPS_10BIT] = stringHash("SMS_SC2331_1L_SLAVE1_MIPI_2M_30FPS_10BIT");

    //vpss mode map
    as32AspetRatio[ASPECT_RATIO_NONE] = stringHash("ASPECT_RATIO_NONE");
    as32AspetRatio[ASPECT_RATIO_AUTO] = stringHash("ASPECT_RATIO_AUTO");
    as32AspetRatio[ASPECT_RATIO_MANUAL] = stringHash("ASPECT_RATIO_MANUAL");
    //vpss mode map
    as32VideoFormat[VIDEO_FORMAT_LINEAR] = stringHash("VIDEO_FORMAT_LINEAR");
    //vpss mode map
    as32VpssAenInput[VPSS_INPUT_MEM] = stringHash("VPSS_INPUT_MEM");
    as32VpssAenInput[VPSS_INPUT_ISP] = stringHash("VPSS_INPUT_ISP");
    as32VpssMode[VPSS_MODE_SINGLE] = stringHash("VPSS_MODE_SINGLE");
    as32VpssMode[VPSS_MODE_DUAL] = stringHash("VPSS_MODE_DUAL");
    as32VpssMode[VPSS_MODE_RGNEX] = stringHash("VPSS_MODE_RGNEX");
    //vi-vpss mode map
    as32ViVpssMode[VI_OFFLINE_VPSS_OFFLINE] = stringHash("VI_OFFLINE_VPSS_OFFLINE");
    as32ViVpssMode[VI_OFFLINE_VPSS_ONLINE] = stringHash("VI_OFFLINE_VPSS_ONLINE");
    as32ViVpssMode[VI_ONLINE_VPSS_OFFLINE] = stringHash("VI_ONLINE_VPSS_OFFLINE");
    as32ViVpssMode[VI_ONLINE_VPSS_ONLINE] = stringHash("VI_ONLINE_VPSS_ONLINE");
    // vpss crop coordinate
    as32VpssCrop[VPSS_CROP_RATIO_COOR] = stringHash("VPSS_CROP_RATIO_COOR");
    as32VpssCrop[VPSS_CROP_ABS_COOR] = stringHash("VPSS_CROP_ABS_COOR");
    //PixFormat map
    as32PixFormat[PIXEL_FORMAT_NV21] = stringHash("PIXEL_FORMAT_NV21");
    as32PixFormat[PIXEL_FORMAT_NV12] = stringHash("PIXEL_FORMAT_NV12");
    as32PixFormat[PIXEL_FORMAT_RGB_888] = stringHash("PIXEL_FORMAT_RGB_888");
    as32PixFormat[PIXEL_FORMAT_BGR_888] = stringHash("PIXEL_FORMAT_BGR_888");
    as32PixFormat[PIXEL_FORMAT_RGB_888_PLANAR] = stringHash("PIXEL_FORMAT_RGB_888_PLANAR");
    as32PixFormat[PIXEL_FORMAT_BGR_888_PLANAR] = stringHash("PIXEL_FORMAT_BGR_888_PLANAR");
    as32PixFormat[PIXEL_FORMAT_ARGB_1555] = stringHash("PIXEL_FORMAT_ARGB_1555");
    as32PixFormat[PIXEL_FORMAT_ARGB_4444] = stringHash("PIXEL_FORMAT_ARGB_4444");
    as32PixFormat[PIXEL_FORMAT_ARGB_8888] = stringHash("PIXEL_FORMAT_ARGB_8888");
    as32PixFormat[PIXEL_FORMAT_YUV_400] = stringHash("PIXEL_FORMAT_YUV_400");
    as32PixFormat[PIXEL_FORMAT_YUV_PLANAR_444] = stringHash("PIXEL_FORMAT_YUV_PLANAR_444");
    as32PixFormat[PIXEL_FORMAT_YUV_PLANAR_420] = stringHash("PIXEL_FORMAT_YUV_PLANAR_420");
    as32PixFormat[PIXEL_FORMAT_YUV_PLANAR_422] = stringHash("PIXEL_FORMAT_YUV_PLANAR_422");
    //wdr mode
    as32ViWdrMode[WDR_MODE_NONE] = stringHash("WDR_MODE_NONE");
    as32ViWdrMode[WDR_MODE_BUILT_IN] = stringHash("WDR_MODE_BUILT_IN");
    as32ViWdrMode[WDR_MODE_QUDRA] = stringHash("WDR_MODE_QUDRA");
    as32ViWdrMode[WDR_MODE_2To1_LINE] = stringHash("WDR_MODE_2To1_LINE");
    as32ViWdrMode[WDR_MODE_2To1_FRAME] = stringHash("WDR_MODE_2To1_FRAME");
    as32ViWdrMode[WDR_MODE_2To1_FRAME_FULL_RATE] = stringHash("WDR_MODE_2To1_FRAME_FULL_RATE");
    as32ViWdrMode[WDR_MODE_3To1_LINE] = stringHash("WDR_MODE_3To1_LINE");
    as32ViWdrMode[WDR_MODE_3To1_FRAME] = stringHash("WDR_MODE_3To1_FRAME");
    as32ViWdrMode[WDR_MODE_3To1_FRAME_FULL_RATE] = stringHash("WDR_MODE_3To1_FRAME_FULL_RATE");
    as32ViWdrMode[WDR_MODE_4To1_LINE] = stringHash("WDR_MODE_4To1_LINE");
    as32ViWdrMode[WDR_MODE_4To1_FRAME] = stringHash("WDR_MODE_4To1_FRAME");
    as32ViWdrMode[WDR_MODE_4To1_FRAME_FULL_RATE] = stringHash("WDR_MODE_4To1_FRAME_FULL_RATE");
    //compress mode
    as32CompressMode[COMPRESS_MODE_NONE] = stringHash("COMPRESS_MODE_NONE");
    as32CompressMode[COMPRESS_MODE_TILE] = stringHash("COMPRESS_MODE_TILE");
    as32CompressMode[COMPRESS_MODE_LINE] = stringHash("COMPRESS_MODE_LINE");
    as32CompressMode[COMPRESS_MODE_FRAME] = stringHash("COMPRESS_MODE_FRAME");
    //mclk
    as32Mclk[RX_MAC_CLK_200M] = stringHash("RX_MAC_CLK_200M");
    as32Mclk[RX_MAC_CLK_300M] = stringHash("RX_MAC_CLK_300M");
    as32Mclk[RX_MAC_CLK_400M] = stringHash("RX_MAC_CLK_400M");
    as32Mclk[RX_MAC_CLK_500M] = stringHash("RX_MAC_CLK_500M");
    as32Mclk[RX_MAC_CLK_600M] = stringHash("RX_MAC_CLK_600M");
    //mclk frequency
    as32MclkFreq[CAMPLL_FREQ_NONE] = stringHash("CAMPLL_FREQ_NONE");
    as32MclkFreq[CAMPLL_FREQ_37P125M] = stringHash("CAMPLL_FREQ_37P125M");
    as32MclkFreq[CAMPLL_FREQ_25M] = stringHash("CAMPLL_FREQ_25M");
    as32MclkFreq[CAMPLL_FREQ_27M] = stringHash("CAMPLL_FREQ_27M");
    as32MclkFreq[CAMPLL_FREQ_24M] = stringHash("CAMPLL_FREQ_24M");
    as32MclkFreq[CAMPLL_FREQ_26M] = stringHash("CAMPLL_FREQ_26M");
    // mod ID
    as32ModId[CVI_ID_VDEC] = stringHash("CVI_ID_VDEC");
    as32ModId[CVI_ID_VPSS] = stringHash("CVI_ID_VPSS");
    as32ModId[CVI_ID_VENC] = stringHash("CVI_ID_VENC");
    as32ModId[CVI_ID_VO]   = stringHash("CVI_ID_VO");
    as32ModId[CVI_ID_VI]   = stringHash("CVI_ID_VI");
    // venc encrypt type
    as32EnType[PT_AAC]   = stringHash("PT_AAC");
    as32EnType[PT_MP3]   = stringHash("PT_MP3");
    as32EnType[PT_H264]  = stringHash("PT_H264");
    as32EnType[PT_H265]  = stringHash("PT_H265");
    as32EnType[PT_JPEG]  = stringHash("PT_JPEG");
    as32EnType[PT_MJPEG] = stringHash("PT_MJPEG");
    // venc rc mode
    as32VencRcMode[VENC_RC_MODE_H264CBR]    = stringHash("VENC_RC_MODE_H264CBR");
    as32VencRcMode[VENC_RC_MODE_H264VBR]    = stringHash("VENC_RC_MODE_H264VBR");
    as32VencRcMode[VENC_RC_MODE_H264AVBR]   = stringHash("VENC_RC_MODE_H264AVBR");
    as32VencRcMode[VENC_RC_MODE_H264QVBR]   = stringHash("VENC_RC_MODE_H264QVBR");
    as32VencRcMode[VENC_RC_MODE_MJPEGCBR]   = stringHash("VENC_RC_MODE_MJPEGCBR");
    as32VencRcMode[VENC_RC_MODE_MJPEGVBR]   = stringHash("VENC_RC_MODE_MJPEGVBR");
    as32VencRcMode[VENC_RC_MODE_H265CBR]    = stringHash("VENC_RC_MODE_H265CBR");
    as32VencRcMode[VENC_RC_MODE_H265VBR]    = stringHash("VENC_RC_MODE_H265VBR");
    as32VencRcMode[VENC_RC_MODE_H265AVBR]   = stringHash("VENC_RC_MODE_H265AVBR");
    as32VencRcMode[VENC_RC_MODE_H265QVBR]   = stringHash("VENC_RC_MODE_H265QVBR");
    // venc bind mode
    as32VencBindMode[VENC_BIND_DISABLE] = stringHash("VENC_BIND_DISABLE");
    as32VencBindMode[VENC_BIND_VI]      = stringHash("VENC_BIND_VI");
    as32VencBindMode[VENC_BIND_VPSS]    = stringHash("VENC_BIND_VPSS");

#if (DUMP_DEBUG == 1)
    aos_debug_printf("The GCORE_GC2053_MIPI_2M_30FPS_10BIT %d \r\n", as32SensorType[GCORE_GC2053_MIPI_2M_30FPS_10BIT]);
    aos_debug_printf("The SONY_IMX327_MIPI_2M_30FPS_12BIT %d \r\n", as32SensorType[SONY_IMX327_MIPI_2M_30FPS_12BIT]);
    aos_debug_printf("The SONY_IMX307_MIPI_2M_30FPS_12BIT %d \r\n", as32SensorType[SONY_IMX307_MIPI_2M_30FPS_12BIT]);
    aos_debug_printf("The SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT %d \r\n", as32SensorType[SONY_IMX307_SLAVE_MIPI_2M_30FPS_12BIT]);
    aos_debug_printf("The GCORE_GC1054_MIPI_1M_30FPS_10BIT %d \r\n", as32SensorType[GCORE_GC1054_MIPI_1M_30FPS_10BIT]);
    aos_debug_printf("The GCORE_GC2093_MIPI_2M_30FPS_10BIT %d \r\n", as32SensorType[GCORE_GC2093_MIPI_2M_30FPS_10BIT]);
    aos_debug_printf("The GCORE_GC02M1_MIPI_2M_30FPS_10BIT %d \r\n", as32SensorType[GCORE_GC02M1_MIPI_2M_30FPS_10BIT]);
    aos_debug_printf("The GCORE_GC02M1_SLAVE_MIPI_2M_30FPS_10BIT %d \r\n", as32SensorType[GCORE_GC02M1_SLAVE_MIPI_2M_30FPS_10BIT]);
    aos_debug_printf("The GCORE_GC4653_MIPI_4M_30FPS_10BIT %d \r\n", as32SensorType[GCORE_GC4653_MIPI_4M_30FPS_10BIT]);
    aos_debug_printf("The GCORE_GC2053_1L_MIPI_2M_30FPS_10BIT %d \r\n", as32SensorType[GCORE_GC2053_1L_MIPI_2M_30FPS_10BIT]);
    aos_debug_printf("The SMS_SC2331_1L_MIPI_2M_30FPS_10BIT %d \r\n", as32SensorType[SMS_SC2331_1L_MIPI_2M_30FPS_10BIT]);
    aos_debug_printf("The SMS_SC2331_1L_SLAVE_MIPI_2M_30FPS_10BIT %d \r\n", as32SensorType[SMS_SC2331_1L_SLAVE_MIPI_2M_30FPS_10BIT]);
    aos_debug_printf("The SMS_SC2331_1L_SLAVE1_MIPI_2M_30FPS_10BIT %d \r\n", as32SensorType[SMS_SC2331_1L_SLAVE1_MIPI_2M_30FPS_10BIT]);

    aos_debug_printf("The ASPECT_RATIO_NONE %d \r\n", as32AspetRatio[ASPECT_RATIO_NONE]);
    aos_debug_printf("The ASPECT_RATIO_AUTO %d \r\n", as32AspetRatio[ASPECT_RATIO_AUTO]);
    aos_debug_printf("The ASPECT_RATIO_MANUAL %d \r\n", as32AspetRatio[ASPECT_RATIO_MANUAL]);

    aos_debug_printf("The as32VpssAenInput[VPSS_INPUT_MEM] %d \r\n", as32VpssAenInput[VPSS_INPUT_MEM]);
    aos_debug_printf("The as32VpssAenInput[VPSS_INPUT_ISP] %d \r\n", as32VpssAenInput[VPSS_INPUT_ISP]);
    aos_debug_printf("The VPSS_MODE_SINGLE %d \r\n", as32VpssMode[VPSS_MODE_SINGLE]);
    aos_debug_printf("The VPSS_MODE_DUAL %d \r\n", as32VpssMode[VPSS_MODE_DUAL]);
    aos_debug_printf("The VI_OFFLINE_VPSS_OFFLINE %d \r\n", as32ViVpssMode[VI_OFFLINE_VPSS_OFFLINE]);
    aos_debug_printf("The VI_OFFLINE_VPSS_ONLINE %d \r\n", as32ViVpssMode[VI_OFFLINE_VPSS_ONLINE]);
    aos_debug_printf("The VI_ONLINE_VPSS_OFFLINE %d \r\n", as32ViVpssMode[VI_ONLINE_VPSS_OFFLINE]);
    aos_debug_printf("The VI_ONLINE_VPSS_ONLINE %d \r\n", as32ViVpssMode[VI_ONLINE_VPSS_ONLINE]);
    aos_debug_printf("The VPSS_CROP_RATIO_COOR %d \r\n", as32VpssCrop[VPSS_CROP_RATIO_COOR]);
    aos_debug_printf("The VPSS_CROP_ABS_COOR %d \r\n", as32VpssCrop[VPSS_CROP_ABS_COOR]);

    aos_debug_printf("The VIDEO_FORMAT_LINEAR %d \r\n", as32VideoFormat[VIDEO_FORMAT_LINEAR]);
    aos_debug_printf("The PIXEL_FORMAT_NV21 %d \r\n", as32PixFormat[PIXEL_FORMAT_NV21]);
    aos_debug_printf("The PIXEL_FORMAT_NV12 %d \r\n", as32PixFormat[PIXEL_FORMAT_NV12]);
    aos_debug_printf("The PIXEL_FORMAT_RGB_888 %d \r\n", as32PixFormat[PIXEL_FORMAT_RGB_888]);
    aos_debug_printf("The PIXEL_FORMAT_BGR_888 %d \r\n", as32PixFormat[PIXEL_FORMAT_BGR_888]);
    aos_debug_printf("The PIXEL_FORMAT_RGB_888_PLANAR %d \r\n", as32PixFormat[PIXEL_FORMAT_RGB_888_PLANAR]);
    aos_debug_printf("The PIXEL_FORMAT_BGR_888_PLANAR %d \r\n", as32PixFormat[PIXEL_FORMAT_BGR_888_PLANAR]);
    aos_debug_printf("The PIXEL_FORMAT_ARGB_1555 %d \r\n", as32PixFormat[PIXEL_FORMAT_ARGB_1555]);
    aos_debug_printf("The PIXEL_FORMAT_ARGB_4444 %d \r\n", as32PixFormat[PIXEL_FORMAT_ARGB_4444]);
    aos_debug_printf("The PIXEL_FORMAT_ARGB_8888 %d \r\n", as32PixFormat[PIXEL_FORMAT_ARGB_8888]);
    aos_debug_printf("The PIXEL_FORMAT_YUV_400 %d \r\n", as32PixFormat[PIXEL_FORMAT_YUV_400]);
    aos_debug_printf("The PIXEL_FORMAT_YUV_PLANAR_444 %d \r\n", as32PixFormat[PIXEL_FORMAT_YUV_PLANAR_444]);
    aos_debug_printf("The PIXEL_FORMAT_YUV_PLANAR_420 %d \r\n", as32PixFormat[PIXEL_FORMAT_YUV_PLANAR_420]);
    aos_debug_printf("The PIXEL_FORMAT_YUV_PLANAR_422 %d \r\n", as32PixFormat[PIXEL_FORMAT_YUV_PLANAR_422]);

    aos_debug_printf("The COMPRESS_MODE_NONE %d \r\n", as32CompressMode[COMPRESS_MODE_NONE]);
    aos_debug_printf("The COMPRESS_MODE_TILE %d \r\n", as32CompressMode[COMPRESS_MODE_TILE]);
    aos_debug_printf("The COMPRESS_MODE_LINE %d \r\n", as32CompressMode[COMPRESS_MODE_LINE]);
    aos_debug_printf("The COMPRESS_MODE_FRAME %d \r\n", as32CompressMode[COMPRESS_MODE_FRAME]);

    aos_debug_printf("The WDR_MODE_NONE %d \r\n", as32ViWdrMode[WDR_MODE_NONE]);
    aos_debug_printf("The WDR_MODE_BUILT_IN %d \r\n", as32ViWdrMode[WDR_MODE_BUILT_IN]);
    aos_debug_printf("The WDR_MODE_QUDRA %d \r\n", as32ViWdrMode[WDR_MODE_QUDRA]);
    aos_debug_printf("The WDR_MODE_2To1_LINE %d \r\n", as32ViWdrMode[WDR_MODE_2To1_LINE]);
    aos_debug_printf("The WDR_MODE_2To1_FRAME %d \r\n", as32ViWdrMode[WDR_MODE_2To1_FRAME]);
    aos_debug_printf("The WDR_MODE_2To1_FRAME_FULL_RATE %d \r\n", as32ViWdrMode[WDR_MODE_2To1_FRAME_FULL_RATE]);
    aos_debug_printf("The WDR_MODE_3To1_LINE %d \r\n", as32ViWdrMode[WDR_MODE_3To1_LINE]);
    aos_debug_printf("The WDR_MODE_3To1_FRAME %d \r\n", as32ViWdrMode[WDR_MODE_3To1_FRAME]);
    aos_debug_printf("The WDR_MODE_3To1_FRAME_FULL_RATE %d \r\n", as32ViWdrMode[WDR_MODE_3To1_FRAME_FULL_RATE]);
    aos_debug_printf("The WDR_MODE_4To1_LINE %d \r\n", as32ViWdrMode[WDR_MODE_4To1_LINE]);
    aos_debug_printf("The WDR_MODE_4To1_FRAME %d \r\n", as32ViWdrMode[WDR_MODE_4To1_FRAME]);
    aos_debug_printf("The WDR_MODE_4To1_FRAME_FULL_RATE %d \r\n", as32ViWdrMode[WDR_MODE_4To1_FRAME_FULL_RATE]);

    aos_debug_printf("The RX_MAC_CLK_200M %d \r\n", as32Mclk[RX_MAC_CLK_200M]);
    aos_debug_printf("The RX_MAC_CLK_300M %d \r\n", as32Mclk[RX_MAC_CLK_300M]);
    aos_debug_printf("The RX_MAC_CLK_400M %d \r\n", as32Mclk[RX_MAC_CLK_400M]);
    aos_debug_printf("The RX_MAC_CLK_500M %d \r\n", as32Mclk[RX_MAC_CLK_500M]);
    aos_debug_printf("The RX_MAC_CLK_600M %d \r\n", as32Mclk[RX_MAC_CLK_600M]);

    aos_debug_printf("The CAMPLL_FREQ_NONE %d \r\n", as32MclkFreq[CAMPLL_FREQ_NONE]);
    aos_debug_printf("The CAMPLL_FREQ_37P125M %d \r\n", as32MclkFreq[CAMPLL_FREQ_37P125M]);
    aos_debug_printf("The CAMPLL_FREQ_25M %d \r\n", as32MclkFreq[CAMPLL_FREQ_25M]);
    aos_debug_printf("The CAMPLL_FREQ_27M %d \r\n", as32MclkFreq[CAMPLL_FREQ_27M]);
    aos_debug_printf("The CAMPLL_FREQ_24M %d \r\n", as32MclkFreq[CAMPLL_FREQ_24M]);
    aos_debug_printf("The CAMPLL_FREQ_26M %d \r\n", as32MclkFreq[CAMPLL_FREQ_26M]);

    aos_debug_printf("The CVI_ID_VDEC %d \r\n", as32ModId[CVI_ID_VDEC]);
    aos_debug_printf("The CVI_ID_VPSS %d \r\n", as32ModId[CVI_ID_VPSS]);
    aos_debug_printf("The CVI_ID_VENC %d \r\n", as32ModId[CVI_ID_VENC]);
    aos_debug_printf("The CVI_ID_VO %d \r\n", as32ModId[CVI_ID_VO]);
    aos_debug_printf("The CVI_ID_VI %d \r\n", as32ModId[CVI_ID_VI]);

    aos_debug_printf("The PT_AAC %d \r\n", as32EnType[PT_AAC]);
    aos_debug_printf("The PT_MP3 %d \r\n", as32EnType[PT_MP3]);
    aos_debug_printf("The PT_H264 %d \r\n", as32EnType[PT_H264]);
    aos_debug_printf("The PT_H265 %d \r\n", as32EnType[PT_H265]);
    aos_debug_printf("The PT_JPEG %d \r\n", as32EnType[PT_JPEG]);
    aos_debug_printf("The PT_MJPEG %d \r\n", as32EnType[PT_MJPEG]);

    aos_debug_printf("The VENC_RC_MODE_H264CBR %d \r\n", as32VencRcMode[VENC_RC_MODE_H264CBR]);
    aos_debug_printf("The VENC_RC_MODE_H264VBR %d \r\n", as32VencRcMode[VENC_RC_MODE_H264VBR]);
    aos_debug_printf("The VENC_RC_MODE_H264AVBR %d \r\n", as32VencRcMode[VENC_RC_MODE_H264AVBR]);
    aos_debug_printf("The VENC_RC_MODE_H264QVBR %d \r\n", as32VencRcMode[VENC_RC_MODE_H264QVBR]);
    aos_debug_printf("The VENC_RC_MODE_MJPEGCBR %d \r\n", as32VencRcMode[VENC_RC_MODE_MJPEGCBR]);
    aos_debug_printf("The VENC_RC_MODE_MJPEGVBR %d \r\n", as32VencRcMode[VENC_RC_MODE_MJPEGVBR]);
    aos_debug_printf("The VENC_RC_MODE_H265CBR %d \r\n", as32VencRcMode[VENC_RC_MODE_H265CBR]);
    aos_debug_printf("The VENC_RC_MODE_H265VBR %d \r\n", as32VencRcMode[VENC_RC_MODE_H265VBR]);
    aos_debug_printf("The VENC_RC_MODE_H265AVBR %d \r\n", as32VencRcMode[VENC_RC_MODE_H265AVBR]);
    aos_debug_printf("The VENC_RC_MODE_H265QVBR %d \r\n", as32VencRcMode[VENC_RC_MODE_H265QVBR]);

    aos_debug_printf("The VENC_BIND_DISABLE %d \r\n", as32VencBindMode[VENC_BIND_DISABLE]);
    aos_debug_printf("The VENC_BIND_VI %d \r\n", as32VencBindMode[VENC_BIND_VI]);
    aos_debug_printf("The VENC_BIND_VPSS %d \r\n", as32VencBindMode[VENC_BIND_VPSS]);

#endif
    return 0;
}

/*
* Parse pqbin parameters into local PARAM_VI_CFG_S
*/
static int parse_PqParam()
{
    PARAM_VI_CFG_S * pstViCtx = PARAM_GET_VI_CFG();
    if (pstViCtx == NULL) {
        aos_debug_printf("PARAM_VI_CFG_S is null!\r\n");
        return -1;
    }

    #if (DUMP_DEBUG == 1)
    aos_debug_printf("CVI_IPCM_GetPQBinQddr() 0x%x\r\n", CVI_IPCM_GetPQBinQddr());
    #endif
    PARTITION_CHECK_HAED_S * pstPqParam = (PARTITION_CHECK_HAED_S *)((unsigned long)CVI_IPCM_GetPQBinQddr());
    if (_param_check_head((unsigned char *)pstPqParam) != 0) {
        return -1;
    }
    unsigned char * pPqBuffer = (unsigned char *)pstPqParam + sizeof(PARTITION_CHECK_HAED_S);
    for (unsigned int i = 0; i < pstPqParam->package_number; i++) {
        pstViCtx->pstIspCfg[i / 2].stPQBinDes[i % 2].pIspBinData = pPqBuffer;
        pstViCtx->pstIspCfg[i / 2].stPQBinDes[i % 2].u32IspBinDataLen = pstPqParam->package_length[i];
        #if (DUMP_DEBUG == 1)
        aos_debug_printf("pPqBuffer %p\r\n", pPqBuffer);
        aos_debug_printf("pstViCtx->pstIspCfg[%d].stPQBinDes[%d].pIspBinData %x\r\n", i/2, i %2, *pPqBuffer);
        aos_debug_printf("pstViCtx->pstIspCfg[%d].stPQBinDes[%d].u32IspBinDataLen %d\r\n", i/2, i %2, pstViCtx->pstIspCfg[i / 2].stPQBinDes[i % 2].u32IspBinDataLen);
        #endif
        pPqBuffer += pstPqParam->package_length[i];
    }

    return 0;
}

/*need to check*/
// static void _param_init()
// {
//     PARAM_MANAGER_CFG_S * pstParam = PARAM_GET_MANAGER_CFG();
//     pstParam->pstSysCtx = &stSysCtx;
//     pstParam->pstViCtx = &stViCtx;
//     pstParam->pstVpssCfg = &stVpssCtx;
//     pstParam->pstVencCfg = &stVencCtx;
//     pstParam->pstVoCfg = NULL;
// }

/*
* Complete parsing param.bin & pqbin into global PARAM_MANAGER_CFG_S
*/
int APP_PARAM_Parse()
{
    int ret = 0;

    /*
    * Get param.bin
    */
    #if (DUMP_DEBUG == 1)
    aos_debug_printf("CVI_IPCM_GetParamBinAddr() 0x%x\r\n", CVI_IPCM_GetParamBinAddr());
    #endif

    char * ini_string = (char *)((unsigned long)CVI_IPCM_GetParamBinAddr());
    if (_param_check_head((unsigned char *)ini_string) != 0) {
        aos_debug_printf("%s err \r\n", __func__);
        return -1;
    }
    ini_string += sizeof(PARTITION_CHECK_HAED_S);

    /*need to check*/
    // _param_init();

    /*
    * Accelerate parsed parameter string matching.
    */
    ret = iniParseHashMapInit();
    if (ret) {
        aos_debug_printf("iniParseHashMapInit fail!\r\n");
        return -1;
    }

    /*
    * Convert paramters from local PARAM_MODULE_CFG_S to global PARAM_MODULE_CFG_S
    */
    ret = parse_ModuleParam(ini_string);
    if (ret) {
        aos_debug_printf("parse_ModuleParam fail!\r\n");
        return -1;
    }

    PARAM_MODULE_CFG_S * pstModuleCtx = PARAM_GET_MODULE_CFG();
    if (pstModuleCtx == NULL) {
        aos_debug_printf("PARAM_MODULE_CFG_S is null!\r\n");
        return -1;
    }

    if (pstModuleCtx->alios_sys_mode) {
        ret = parse_SysParam(ini_string);
        if (ret) {
            aos_debug_printf("parse_SysParam fail!\r\n");
            return -1;
        }
    }

    if (pstModuleCtx->alios_vi_mode) {
        ret = parse_ViParam(ini_string);
        if (ret) {
            aos_debug_printf("parse_ViParam fail!\r\n");
            return -1;
        }
    }

    if (pstModuleCtx->alios_vpss_mode) {
        ret = parse_VpssParam(ini_string);
        if (ret) {
            aos_debug_printf("parse_VpssParam fail!\r\n");
            return -1;
        }
    }

    if (pstModuleCtx->alios_venc_mode) {
        ret = parse_VencParam(ini_string);
        if (ret) {
            aos_debug_printf("parse_VencParam fail!\r\n");
            return -1;
        }
    }

    ret = parse_PqParam();
    if (ret) {
        aos_debug_printf("parse_PqParam fail!\r\n");
        return -1;
    }

    return ret;
}


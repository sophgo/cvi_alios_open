#include "custom_param.h"
#include "ir_custom_param.h"
#include "fc_custom_media.h"


typedef enum _PARAM_PIPELINE_E{
    PARAM_RGB_PIPELINE_E,
    PARAM_IR_PIPELINE_E,
}PARAM_PIPELINE_E;

static PARAM_PIPELINE_E s_pipelinemode = 0;

PARAM_MANAGER_CFG_S  g_stManagerCtx = {
    .pstSysCtx = NULL,
    .pstViCtx = NULL,
    .pstVpssCfg = NULL,
    .pstVoCfg = NULL,
    .pstVencCfg = NULL,
};

PARAM_MANAGER_CFG_S * PARAM_GET_MANAGER_CFG(void)
{
    return &g_stManagerCtx;
}

int PARAM_GET_MANAGER_CFG_PIPE()
{
    return s_pipelinemode;
}

void PARAM_SET_MANAGER_CFG_PIPE(int pipeline)
{
    //这里进行pipeline切换预处理
    switch(pipeline) {
        case PARAM_IR_PIPELINE_E:
        CustomEvent_IRGpioSet(1);
        s_pipelinemode = pipeline;
        g_stManagerCtx.pstSysCtx = IR_PARAM_GET_SYS_CFG();
        g_stManagerCtx.pstViCtx = IR_PARAM_GET_VI_CFG();
        g_stManagerCtx.pstVpssCfg = IR_PARAM_GET_VPSS_CFG();
        g_stManagerCtx.pstVencCfg = IR_PARAM_GET_VENC_CFG();
        g_stManagerCtx.pstVoCfg = PARAM_GET_VO_CFG();
        break;
        case PARAM_RGB_PIPELINE_E:
        CustomEvent_IRGpioSet(0);
        s_pipelinemode = pipeline;
        g_stManagerCtx.pstSysCtx = PARAM_GET_SYS_CFG();
        g_stManagerCtx.pstViCtx = PARAM_GET_VI_CFG();
        g_stManagerCtx.pstVpssCfg = PARAM_GET_VPSS_CFG();
        g_stManagerCtx.pstVencCfg = PARAM_GET_VENC_CFG();
        g_stManagerCtx.pstVoCfg = PARAM_GET_VO_CFG();
        break;
    }
}

void PARAM_INIT_MANAGER_CFG()
{
//决定初始化采用哪套pipeline
#if (CONFIG_APP_SENSOR_IR_USE == 1)
    PARAM_SET_MANAGER_CFG_PIPE(PARAM_IR_PIPELINE_E);
#else
    PARAM_SET_MANAGER_CFG_PIPE(PARAM_RGB_PIPELINE_E);
#endif
}
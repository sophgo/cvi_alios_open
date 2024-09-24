# Alios新Sensor适配流程
    (1) components/cvi_mmf_sdk/cvi_sensor目录下实现新Sensor驱动，
        建议基于已有Sensor修改，详细实现步骤请参考《Sensor调试指南》。

    (2) components/cvi_mmf_sdk/cvi_sensor/package.yaml ->
        build_config:
            include:
        下增加新Sensor驱动路径
        source_file:
        下增加新Sensor驱动源码

    (3) solutions/normboot/package.yaml ->
        def_config:
        下增加Sensor配置项如 CONFIG_SENSOR_GCORE_GC4653: 1。

    (4) components/cvi_mmf_sdk/cvi_middleware/include/cvi_sns_ctrl.h 中
        找到SNS_TYPE_E并添加Sensor类型，并增加新Sensor的Obj结构体外部引用如:

        #if CONFIG_SENSOR_GCORE_GC4653
        extern ISP_SNS_OBJ_S stSnsGc4653_Obj;
        #endif

    (5) solutions/normboot/package.yaml 中选择要使用的Sensor
        example单路GC4653：
        (SNS_TYPE_E)GCORE_GC4653_MIPI_4M_30FPS_10BIT = 11
        那么设置如下：
        CONFIG_SNS0_TYPE: 11
        CONFIG_SNS1_TYPE: 0

    (6) components/cvi_mmf_sdk/cvi_sensor/sensor_cfg/sensor_cfg.c 中
        ISP_SNS_OBJ_S *getSnsObj(SNS_TYPE_E enSnsType); -> 增加返回新Sensor的Obj结构体
        CVI_S32 getPicSize(CVI_S32 dev_id, SNS_SIZE_S *pstSize); -> 增加新Sensor的宽高配置
        CVI_S32 getDevAttr(VI_DEV ViDev, VI_DEV_ATTR_S *pstViDevAttr); -> 增加新Sensor的Dev属性配置
        CVI_S32 getPipeAttr(VI_DEV ViDev, VI_PIPE_ATTR_S *pstViPipeAttr) -> 增加新Sensor的Pipe属性配置
        CVI_S32 getChnAttr(VI_DEV ViDev, VI_CHN_ATTR_S *pstViChnAttr) -> 增加新Sensor的Chn属性配置
# AliOS 开机 AE 快速收敛

## 快速收敛原理

- 正常开机时AE收敛从默认设置的曝光参数生效的亮度开始收敛，收敛至平稳过程大概十几帧，前几帧可能过暗或过亮不能使用

- 一般情况下，开机AE快速收敛可以根据第一帧的亮度，直接设置收敛平稳的大概曝光参数值，使画面过完一个反应帧周期就能正常使用

- 这个过程需要预先标定若干组不同环境亮度梯度收敛平稳的 AE 曝光参数，正常使用时，AE 根据这几组不同的参数，在不同环境亮度下内插设置相对应的曝光参数，效果与手动曝光参数设置类似

## 标定

### 环境准备

设备加载参数为**调整 ISP 后的定版 PQ 参数**，从而确保不同环境亮度下收敛的目标亮度符合画面预期；标定过程需要固定设备（固定设备在纸盒上且摆放平稳），同时需要确保画面均匀（白墙或灯箱均匀背景等）：
- IR 通道：IR 补光灯需与正常使用时一致，开启设备 IR 补光灯（因为 IR 补光灯亮度与距离相关，正常室内光源影响不大）
- RGB 通道：需要可变亮度光源

### 获取标定值

- IR 通道标定：背景距镜头不同距离（ 如：5、10、15、20、30cm）
- RGB 通道标定：调节不同亮度获取不同节点

获取标定节点值：
- 在上电前，需要将手掌或人脸位于待标定距离
- 上电后等待一段时间2-3s，直至完成AE自动收敛
- 使用串口指令 `calibrate_ae_lut` 获取对应通道第一帧亮度值(Luma)，和收敛平稳 Bv 值（曝光各个参数根据 Bv 值计算），示例如下：
```shell
(cli-uart)# calibrate_ae_lut
vipipe =0 firstFrameLuma =60 stableBv = 112
```
- 标定时尽可能多标定几组数值，从其中选取合适的数值作为标定参数，每开机一次获取一次节点值
- 节点尽量包含正常亮度范围（AE 亮度区间为 0~ 1024） ，快速收敛只能适用正常亮度，过亮或过暗环境快速收敛只能取标定两端的值
  
> [!WARNING]
> 部分sensor可能正常出流的第一帧或前几帧是黑帧或固定帧，此时可采用sensor快启的方式在uboot阶段提前初始化sensor，避免黑帧影响；或者设定标定帧从第x帧开始，可联系相关技术人员协助修改
> 

### 选取标定值

- 从亮度开始变化时开始选取标定点，Bv差距小的多个点可以合为一个标定值
- 亮度变化大的区间需多进行取点，从而使得曲线更贴合实际情况

## 设置开机快启参数

### 更新 libcvi_mw_isp_ae.a
将`boards/tools/fastconverge`目录下的libcvi_mw_isp_ae.a替换`components/cvi_mmf_sdk_cv18xxx/lib`下的原始静态库libcvi_mw_isp_ae.a ，从而更新静态库来输出前 30 帧的 Bv 值、luma 值打印，可用于测试开启 AE 快速收敛前后图像分别在第几帧收敛

### 使能 AE 快速收敛

相关代码路径： `solutions/smart_doorbell/customization/xxxx/param/custom_viparam.c`

```C
PARAM_VI_CFG_S g_stViCtx = {
    .u32WorkSnsCnt = 1,
    .pstSensorCfg  = PARAM_CLASS(SENSORCFG, CTX, Sensor),
    .pstIspCfg     = PARAM_CLASS(ISPCFG, CTX, ISP),
    .pstDevInfo    = PARAM_CLASS(VIDEVCFG, CTX, VI),
    .bFastConverge = true, //使能AE开机收敛
};
```
### 修改节点数

sdk 中默认是 5 组节点参数对，可根据实际情况适当增加节点数： `FAST_CONVERGE_NODES `

路径： `components/cvi_mmf_sdk_cv18xxx/include/cvi_middleware/include/isp/cv18xx/cvi_comm_isp.h`

```C
#define FAST_CONVERGE_NODES  (5)
typedef struct _ISP_AE_BOOT_FAST_CONVERGE_S {
    CVI_BOOL bEnable;
    CVI_U8  availableNode;
    CVI_S16 firstFrLuma[FAST_CONVERGE_NODES];
    CVI_S16 targetBv[FAST_CONVERGE_NODES];
} ISP_AE_BOOT_FAST_CONVERGE_S;
```

### 设置快速收敛参数

将选取出来的标定值（Luma 与 Bv 值）在 sdk 中进行修改

路径： `components/cvi_platform/media/src/media_video.c`

函数名：`setFastConvergeAttr` ，单通道只需设置ViPipe等于0时的Luma和Bv值，双通道则需设置两组Luma和Bv值
```C
CVI_S16 firstFrLuma[5] = {357, 368, 417, 499, 557};
CVI_S16 targetBv[5] = {373, 382, 423, 450, 500};
```
> [!NOTE]
> 第一帧亮度节点 firstFrLuma 按从低到高顺序填写，Bv 值与 Luma 值对应

### 重新生成固件进行烧录升级

## 开机AE快速收敛结果对比
根据替换libcvi_mw_isp_ae.a后输出的Log对比开启AE快速收敛前后的收敛帧数，这里以sc2356为例：
- 未开启AE快速收敛时的日志如下（只截取前30帧的部分日志），可以看出AE在17帧时收敛
```shell
08:00:04.564 fid(0, 0):     1  Luma: 528 Lv: 1143 T:16613 L:1246 ISO:   400 AG:  4096 DG:  1024 IG: 1024
Bv(0, 0):0 pBv:0 Tv:612 Sv:200
AeL(0, 0):303 528 TL:196 ATL:132 LO:-64 LB:-104 HB:-64
HistError = -43
sid:0 tBv:445 fLuma:486
08:00:04.574 fid(0, 0):     2 Luma: 486 Lv: 1143 T:16613 L:1246 ISO:   318 AG:  3258 DG:  1024 IG: 1024 (cli-uart)#
Bv(0, 0):445 pBv:0 Tv:612 Sv:167
AeL(0, 0):286 486 TL:196 ATL:131 LO:-66 LB:-104 HB:-64
HistError = -39
sid:0 tBv:445 fLuma:976
08:00:04.717 fid(0, 0):     3 Luma: 976 Lv: 1143 T:16613 L:1246 ISO:   318 AG:  3258 DG:  1024 IG: 1024
Bv(0, 0):445 pBv:0 Tv:612 Sv:167
AeL(0, 0):754 976 TL:196 ATL:130 LO:-68 LB:-104 HB:-64
HistError = -157
sid:0 tBv:445 fLuma:973
......
08:00:05.161 fid(0, 0):    14 Luma: 948 Lv: 1161 T:  499 L:  37 ISO:   101 AG:  1024 DG:  1024 IG: 1024
Bv(0, 0):1118 pBv:1118 Tv:1118 Sv:0
AeL(0, 0):647 948 TL:196 ATL:82 LO:-104 LB:-104 HB:-64
HistError = -141
08:00:05.190 fid(0, 0):    15 Luma: 947 Lv: 1169 T:  499 L:  37 ISO:   101 AG:  1024 DG:  1024 IG: 1024
Bv(0, 0):1118 pBv:1118 Tv:1118 Sv:0
AeL(0, 0):647 947 TL:196 ATL:87 LO:-104 LB:-104 HB:-64
HistError = -141
08:00:05.220 fid(0, 0):    16 Luma: 944 Lv: 1173 T:  499 L:  37 ISO:   101 AG:  1024 DG:  1024 IG: 1024
Bv(0, 0):1118 pBv:1118 Tv:1118 Sv:0
AeL(0, 0):647 944 TL:196 ATL:89 LO:-104 LB:-104 HB:-64
HistError = -140
08:00:05.249 fid(0, 0):    17 Luma: 142 Lv: 1166 T:  503 L:  37 ISO:   101 AG:  1024 DG:  1024 IG: 1024
Bv(0, 0):1117 pBv:1118 Tv:1117 Sv:0
AeL(0, 0):85 142 TL:196 ATL:91 LO:-102 LB:-104 HB:-64
HistError = -1
08:00:05.279 fid(0, 0):    18 Luma: 142 Lv: 1166 T:  506 L:  37 ISO:   102 AG:  1024 DG:  1024 IG: 1024
Bv(0, 0):1116 pBv:1117 Tv:1116 Sv:0
AeL(0, 0):85 142 TL:196 ATL:93 LO:-100 LB:-104 HB:-64
HistError = -2
08:00:05.309 fid(0, 0):    19 Luma: 143 Lv: 1166 T:  510 L:  38 ISO:   100 AG:  1024 DG:  1024 IG: 1024
Bv(0, 0):1115 pBv:1116 Tv:1115 Sv:0
AeL(0, 0):85 143 TL:196 ATL:95 LO:-98 LB:-104 HB:-64
HistError = -2
08:00:05.339 fid(0, 0):    20 Luma: 143 Lv: 1166 T:  513 L:  38 ISO:   101 AG:  1024 DG:  1024 IG: 1024
Bv(0, 0):1114 pBv:1115 Tv:1114 Sv:0
AeL(0, 0):85 143 TL:196 ATL:97 LO:-96 LB:-104 HB:-64
HistError = -3
```
- 开启AE快速收敛的日志如下（只截取前30帧的部分日志），可以看出AE在第7帧收敛，即过完一个反应帧周期（6帧）后收敛
```shell
08:00:02.456 fid(0, 0):     1 Luma: 490 Lv: 1153 T:16613 L:1246 ISO:   400 AG:  4096 DG:  1024 IG: 1024
Bv(0, 0):0 pBv:0 Tv:612 Sv:200
AeL(0, 0):324 490 TL:196 ATL:132 LO:-64 LB:-104 HB:-64
HistError = -49
08:00:02.901 fid(0, 0):     2 Luma: 479 Lv: 1153 T:  623 L:  46 ISO:   101 AG:  1024 DG:  1024 IG: 1024
fast boot enable
Bv(0, 0):1086 pBv:0 Tv:1086 Sv:0
AeL(0, 0):322 479 TL:196 ATL:131 LO:-66 LB:-104 HB:-64
HistError = -48
sid:0 tBv:1086 fLuma:914
08:00:02.909 fid(0, 0):     3 Luma: 914 Lv: 1153 T:  623 L:  46 ISO:   101 AG:  1024 DG:  1024 IG: 1024
Bv(0, 0):1086 pBv:0 Tv:1086 Sv:0
AeL(0, 0):845 914 TL:196 ATL:130 LO:-68 LB:-104 HB:-64
HistError = -179
sid:0 tBv:1086 fLuma:914
08:00:02.939 fid(0, 0):     4 Luma: 914 Lv: 1153 T:  623 L:  46 ISO:   101 AG:  1024 DG:  1024 IG: 1024
Bv(0, 0):1086 pBv:0 Tv:1086 Sv:0
AeL(0, 0):845 914 TL:196 ATL:128 LO:-70 LB:-104 HB:-64
HistError = -179
sid:0 tBv:1086 fLuma:912
08:00:02.969 fid(0, 0):     5 Luma: 912 Lv: 1153 T:  623 L:  46 ISO:   101 AG:  1024 DG:  1024 IG: 1024
Bv(0, 0):1086 pBv:0 Tv:1086 Sv:0
AeL(0, 0):838 912 TL:196 ATL:126 LO:-72 LB:-104 HB:-64
HistError = -178
sid:0 tBv:1086 fLuma:914
08:00:02.998 fid(0, 0):     6 Luma: 914 Lv: 1464 T:  623 L:  46 ISO:   101 AG:  1024 DG:  1024 IG: 1024
Bv(0, 0):1086 pBv:0 Tv:1086 Sv:0
AeL(0, 0):840 914 TL:196 ATL:124 LO:-74 LB:-104 HB:-64
HistError = -179
08:00:03.027 fid(0, 0):     7 Luma: 178 Lv: 1195 T:  619 L:  46 ISO:   100 AG:  1024 DG:  1024 IG: 1024
Bv(0, 0):1087 pBv:1086 Tv:1087 Sv:0
AeL(0, 0):130 178 TL:196 ATL:124 LO:-72 LB:-104 HB:-64
HistError = -2
08:00:03.057 fid(0, 0):     8 Luma: 178 Lv: 1195 T:  615 L:  46 ISO:   100 AG:  1024 DG:  1024 IG: 1024
Bv(0, 0):1088 pBv:1087 Tv:1088 Sv:0
AeL(0, 0):130 178 TL:196 ATL:125 LO:-70 LB:-104 HB:-64
HistError = -1
08:00:03.086 fid(0, 0):     9 Luma: 178 Lv: 1195 T:  615 L:  46 ISO:   100 AG:  1024 DG:  1024 IG: 1024
Bv(0, 0):1088 pBv:1088 Tv:1088 Sv:0
AeL(0, 0):130 178 TL:196 ATL:126 LO:-68 LB:-104 HB:-64
HistError = -1
08:00:03.116 fid(0, 0):    10 Luma: 179 Lv: 1196 T:  615 L:  46 ISO:   100 AG:  1024 DG:  1024 IG: 1024
Bv(0, 0):1088 pBv:1088 Tv:1088 Sv:0
AeL(0, 0):131 179 TL:196 ATL:128 LO:-66 LB:-104 HB:-64
HistError = -1
08:00:03.146 fid(0, 0):    11 Luma: 179 Lv: 1197 T:  615 L:  46 ISO:   100 AG:  1024 DG:  1024 IG: 1024
Bv(0, 0):1088 pBv:1088 Tv:1088 Sv:0
AeL(0, 0):131 179 TL:196 ATL:130 LO:-64 LB:-104 HB:-64
HistError = -0
......
```
- 由上述对比可以看出，开启AE快速收敛之后，收敛速度快了10帧。实际上，不同sensor型号、不同的标定参数都会影响最终效果
> [!NOTE]
> fid(0,0)后面的数字代表帧序号，当亮度值Luma趋于稳定的时候即代表收敛，过亮或过暗则表示未收敛



# CVI Alios SmartAE

## 背景

- 线性模式的 sensor 无法避免在逆光等特殊环境下，由于背景过亮会导致前景偏暗
- 在此情境下 ISP 没有太多的可调空间
- 用户关注人脸、掌纹等前景的图像质量

<br>

## 原理

- SmartAE (Smart Auto Exposure) 让 AE 以指定区域的亮度统计值来进行计算，优先保证指定区域的亮度效果
- 以人脸举例，设置人脸区域的期望亮度并持续更新坐标，使原来偏暗的地方拉亮，不影响到算法检测

> [!WARNING]
> 该方法存在一定局限性，亮度的提升会对整张画面进行处理
>
> 在拉亮了人脸区域亮度的同时可能会让背景过曝，导致光晕溢出到人脸上影响算法检测准确性

<br>

## 使用说明

### SmartAE 初始化参数设置

VI 初始化后配置智能模式下的 AE 属性. 参考配置如下

```C
  ISP_SMART_EXPOSURE_ATTR_S stSmartExpAttr;
  CVI_ISP_GetSmartExposureAttr(ViPipe, &stSmartExpAttr);

  stSmartExpAttr.bEnable = CVI_TRUE;
  stSmartExpAttr.bIRMode = CVI_TRUE;
  stSmartExpAttr.enSmartExpType = OP_TYPE_AUTO;
  /* Target Luma of ROI */
  stSmartExpAttr.u8LumaTarget = 46;
  stSmartExpAttr.u16ExpCoef = 1024;
  stSmartExpAttr.u16ExpCoefMax = 4096;
  stSmartExpAttr.u16ExpCoefMin = 256;
  stSmartExpAttr.u8SmartInterval = 1;
  stSmartExpAttr.u8SmartSpeed = 32;
  stSmartExpAttr.u16SmartDelayNum = 5;
  /* Area brightness reference weight */
  stSmartExpAttr.u8Weight = 80;
  stSmartExpAttr.u8NarrowRatio = 75;

  CVI_ISP_SetSmartExposureAttr(ViPipe, &stSmartExpAttr);
```

> [!NOTE]
> 其中, u8LumaTarget 越大表示期望区域画面亮度越高
>
> u8Weight 表示区域亮度参考权重，如果权重为 100，则仅以设置的局部区域亮度来进行判断，否则会兼顾整体画面亮度
>
> ISP_SMART_EXPOSURE_ATTR_S 的具体参数说明请参考[\[ISP 开发参考\]ISP_SMART_EXPOSURE_ATTR_S](https://doc.sophgo.com/cvitek-develop-docs/master/docs_latest_release/CV180x_CV181x/zh/01.software/ISP/ISP_Development_Reference/build/html/4_AE.html#isp-smart-exposure-attr-s)

### ROI 设置

ROI (Region Of Interest) 用于指定 SmartAE 关注的区域

AE 会根据提供的人脸（掌等）坐标在 Raw 的位置来找出人脸位置的 AE 窗口, 并使用此 AE Window 的亮度来执行 SmartAE 的曝光策略

可参考 SDK 中物体检测 Demo（object_detection.c）中人形侦测案例对 ROI 坐标进行更新

```C
  /* Only Need to set stROI[0] */
  ISP_SMART_ROI_S* pstROI = &stSmartInfo.stROI[0];
  cvtdl_bbox_t* pstObjBbox = &pObjMeta->info[index]->bbox;
  pstROI->bEnable = 1;
  pstROI->bAvailable = 1;
  pstROI->u8Num = pObjMeta->size > SMART_MAX_NUM ? SMART_MAX_NUM : pObjMeta->size;

  /* Update ROI position */
  for (int i = 0; i < pstROI->u8Num; i++) {
      pstROI->u16PosX[i] = (CVI_U16)(pstObjBbox->x1);
      pstROI->u16PosY[i] = (CVI_U16)(pstObjBbox->y1);
      pstROI->u16Width[i] = (CVI_U16)(pstObjBbox->x2 - pstObjBbox->x1);
      pstROI->u16Height[i] = (CVI_U16)(pstObjBbox->y2 - pstObjBbox->y1);
  }

  /* Frame's width/height of raw */
  pstROI->u16FrameWidth = (CVI_U16)pObjMeta->width;
  pstROI->u16FrameHeight = (CVI_U16)pObjMeta->height;
  CVI_ISP_SetSmartInfo(CVIAI_SMARTAE_PIPE, &stSmartInfo, CVIAI_SMARTAE_KEEPFRAME_NUM);
```

> 上述代码片段仅作为实际应用开发参考
>
> ISP_SMART_ROI_S 具体参数说明请参考[\[ISP 开发参考\]ISP_SMART_ROI_S](https://doc.sophgo.com/cvitek-develop-docs/master/docs_latest_release/CV180x_CV181x/zh/01.software/ISP/ISP_Development_Reference/build/html/3_System_Control.html#isp-smart-roi-s)

### AE 收敛情况判断

判断当前 AE 收敛情况，进行抓拍/取图等操作

```C
  CVI_ISP_QueryExposureInfo(ViPipe, &stExpInfo[ViPipe]);

  /* The closer to 0, the more complete the convergence */
  if (stExpInfo[ViPipe].s16HistError < 5 && stExpInfo[ViPipe].s16HistError > -5) {
    /* Dump frame or other operation */
  }
```

> [!NOTE]
> 区域坐标更新后不能立刻抓图，此时 AE 可能还未收敛完成
>
> 根据以上方式来查询收敛情况，可根据实际需求对收敛时间/效果进行平衡

<br>

## 实际测试结果

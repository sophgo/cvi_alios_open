<!-- 简体中文 | [English](README-en.md) -->

# 目录

- [目录](#目录)
- [项目简介](#项目简介)
  - [处理器规格](#处理器规格)
  - [硬件资料](#硬件资料)
- [SDK 目录结构](#sdk-目录结构)
  - [Solutions 说明](#solutions-说明)
    - [解决方案目录结构](#解决方案目录结构)
- [SDK 编译使用说明](#sdk-编译使用说明)
  - [编译环境准备](#编译环境准备)
    - [Ubuntu 安装 Python \& pip](#ubuntu-安装-python--pip)
    - [YOCTOOL 安装](#yoctool-安装)
    - [编译工具链下载](#编译工具链下载)
  - [编译步骤及说明](#编译步骤及说明)
    - [编译步骤](#编译步骤)
    - [镜像文件说明](#镜像文件说明)
  - [固件烧录](#固件烧录)
  - [运行说明](#运行说明)
- [User Manual](#user-manual)
  - [USB](#usb)
  - [OTA](#ota)
  - [Peripheral](#peripheral)
  - [ISP](#isp)
- [FAQ](#faq)
- [关于算能](#关于算能)

<br>

# 项目简介

- 本仓库提供[算能科技](https://www.sophgo.com/)端侧处理器`CV181x`和`CV180x`两个系列处理器的软件开发包(SDK)
- 主要适用于官方 EVB

<br>

## 处理器规格

- [处理器产品简介](https://www.sophgo.com/product/index.html)

<br>

## 硬件资料

- [《CV180xB EVB 板硬件指南》](https://sophon-file.sophon.cn/sophon-prod-s3/drive/23/03/14/14/CV180xB_EVB%E6%9D%BF%E7%A1%AC%E4%BB%B6%E6%8C%87%E5%8D%97_V1.0.pdf)
- [《CV180xC EVB 板硬件指南》](https://sophon-file.sophon.cn/sophon-prod-s3/drive/23/03/18/18/CV180xC_EVB%E6%9D%BF%E7%A1%AC%E4%BB%B6%E6%8C%87%E5%8D%97_V1.0.pdf)
- [《CV181xC EVB 板硬件指南》](https://sophon-file.sophon.cn/sophon-prod-s3/drive/23/03/15/14/CV181xC_QFN_EVB%E6%9D%BF%E7%A1%AC%E4%BB%B6%E6%8C%87%E5%8D%97_V1.0.pdf)
- [《CV181xH EVB 板硬件指南》](https://sophon-file.sophon.cn/sophon-prod-s3/drive/23/03/15/15/CV181xH_EVB%E6%9D%BF%E7%A1%AC%E4%BB%B6%E6%8C%87%E5%8D%97_V1.0.pdf)

<br>

# SDK 目录结构

```
.
├── boards      //板卡相关配置
├── components  //各种类型组件，例如aos内核，cli命令行，网络组件，以及CSI驱动
├── docs        //使用说明文档
├── host-tools  //交叉编译工具链
└── solutions   //解决方案目录，主要存放业务逻辑，运行bin等
```

<br>

## Solutions 说明

| Solution Name  | Description  | Link                                        |
| -------------- | ------------ | ------------------------------------------- |
| cv181x_boot    | boot 配置    | [cv181x_boot.md](docs/cv181x_boot.md)       |
| smart_doorbell | 智慧门锁项目 | [smart_doorbell.md](docs/smart_doorbell.md) |

<br>

### 解决方案目录结构

```
.
├── application     //应用相关部分
├── customization   //客制化应用相关以及不同产品形态配置
├── generated       //编译产生的镜像存储目录
├── py_tool         //python的工具
├── script          //编译的脚本
├── yoc_sdk         //编译产生的静态库文件目录
└── package.yaml    //用于定义和开启功能宏
```

package.yaml 配置及使用可参考 https://help.aliyun.com/document_detail/308617.html

<br>

# SDK 编译使用说明

<br>

## 编译环境准备

### Ubuntu 安装 Python & pip

- 安装 Python3
- 确保 Python3 优先级最高

  ```shell
    // 显示python可用版本列表
    $ update-alternatives --list python
    // Python使用版本优先级设置
    $ update-alternatives --install /usr/bin/python python /usr/bin/python3.X 2
  ```

- 安装 pip，执行如下命令

  ```shell
    $ sudo apt-get install python3-pip
    $ sudo pip3 install --upgrade pip
  ```

- 安装 python 的 serial 和 usb 驱动模块，执行如下命令

  ```shell
    $ sudo pip3 install pyserial
    $ sudo pip3 install pyusb
  ```

- 安装 yaml 模块，执行如下命令

  ```shell
    $ sudo pip3 install pyyaml
  ```

  > [!NOTE]
  > 如果按上述步骤 pyyaml 安装失败，可以使用下面命令安装
  >
  > `$ sudo pip3 install pyyaml -i http://pypi.douban.com/simple --trusted-host pypi.douban.com`
  >
  > Ubuntu 版本推荐高于 20.0.4

<br>

### YOCTOOL 安装

- Linux 环境中使用指令安装 yoctool
  ```shell
    $ sudo pip3 install yoctools -U
  ```
- 使用`yoc -V`查看对应版本，确保版本处于或者高于**2.0.54**

  ```shell
    cvi_tek@WORKSTATION:cvi_alios_open$ yoc -V
    2.0.54
  ```

- 输入 product version 查看 product 的版本，确保 product 安装成功
  ```shell
    cvi_tek@WORKSTATION:cvi_alios_open$ product version
    v1.0.45
    Mar 17 2023,14:16:38
  ```

<br>

### 编译工具链下载

在[Release](https://github.com/sophgo/cvi_alios_open/releases)板块下载玄铁交叉编译工具链`Xuantie-900-gcc-elf-newlib-x86_64-V2.6.1`

解压后，将编译工具链文件夹放置于 SDK 的`host-tools`目录下

<br>

## 编译步骤及说明

本文说明中采用相关配置如下

| Name     | Type               |
| -------- | ------------------ |
| board    | CV1810C-WEVB-0006A |
| flash    | SPINOR             |
| sensor   | 双目 GC2053+GC2093 |
| solution | smart_doorbell     |
| project  | turnkey_qfn        |

<br>

### 编译步骤

- 在顶层目录进行 yoc 初始化（只需执行一次）

  ```shell
    $ yoc init
  ```

- 在顶层目录下使用以下指令进行编译操作

  ```shell
    $ make smart_doorbell PROJECT=turnkey_qfn
  ```

  生成的镜像位于/solutions/solution_name/generated/images 目录下

  ```shell
    cvi_tek@WORKSTATION:cvi_alios_open$ ls -gGh solutions/smart_doorbell/generated/images
    total 6.1M
    -rw-rw-r-- 1 113K 12月 31  1979 boot
    -rw-rw-r-- 1  44K 12月 31  1979 boot0
    -rw-rw-r-- 1 2.6K 12月 31  1979 config.yaml
    -rw-rw-r-- 1  33K 12月 31  1979 fip_fsbl.bin
    -rw-rw-r-- 1 8.0K 12月 31  1979 imtb
    -rw-rw-r-- 1  404 12月 31  1979 partition_alios_spinor.xml
    -rw-rw-r-- 1 3.0M 12月 31  1979 prim
    -rw-rw-r-- 1 3.0M 12月 31  1979 yoc.bin
  ```

  生成的 fip.bin 位于/boards/tools/fip/board_type 目录下

  ```shell
    cvi_tek@WORKSTATION:cvi_alios_open$ ls -gGh boards/tools/fip/181x
    total 888K
    -rw-rw-r-- 1 397K 7月   6 20:11 fip.bin
    -rw-rw-r-- 1 485K 7月   6 20:11 fip.bin.secureboot
  ```

<br>

### 镜像文件说明

| Name         | Description                               |
| ------------ | ----------------------------------------- |
| fip_fsbl.bin | 一级 boot，文件 boot0 为其重命名          |
| boot         | 二级 bootloader                           |
| imtb         | 分区表                                    |
| yoc.bin      | 运行程序 bin 执行档，文件 prim 为其重命名 |
| config.yaml  | flash 分区表，固件烧录时需要使用          |

<br>

## 固件烧录

**TF 卡烧录**

请参考 [CVI AliOS TF Burn](docs/tf_burn.md)

**USB 烧录**

请参考 [CVI AliOS USB Burn](docs/usb_burn.md)

<br>

## 运行说明

- 烧录完毕后关闭电源等待 1-2s，存在电容放电需要等释放完毕
- 上电有如下打印`###YOC###`证明内核启动完毕

  ```shell
  ##cur_ms:0
  j 0x80040000
  ##cur_ms:94
  ###YoC###[Aug 23 2023,10:11:27]
  spinor: ID = c8 40 18
  [   0.201]\<I>app app_main.c[129]: app start........
  APP_CustomEventStart 12.
  (cli-uart)#
  (cli-uart)#
  ```

- 输入串口回车可以正常输入输出，并且有 cli_uart 打印即运行正常

<br>

# User Manual

<!-- ## Project -->
<!-- - [新增客制化Project配置指南]() -->

## USB

- [UVC & RTSP 出流指南](docs/uvc_rtsp_streaming_manual.md)
- [USB 配置指南](docs/usb_cfg_manual.md)
- [WINUSB 配置指南](docs/winusb_cfg_manual.md)

## OTA

- [CDC OTA 配置使用指南](docs/cdc_ota_manual.md)
- [FOTA 配置使用指南](docs/fota_manual.md)

## Peripheral

- [Flash 配置使用指南](docs/flash_manual.md)

## ISP

- [PQtool 使用手册](docs/pqtool_manual.md)
- [SmartAE 使用指南](docs/smartAE_manual.md)

<!-- ## Algo

- [视频算法 Demo 使用指南](docs/video_algo_demo_manual.md)
- [音频算法 Demo 使用指南](docs/audio_algo_demo_manual.md) -->

<br>

# FAQ

请参考 https://developer.sophgo.com/thread/513.html

<br>

# 关于算能

- 算能致力于成为全球领先的定制算力提供商，专注于 RISC-V、TPU 处理器等算力产品的研发和推广应用。
- 公司遵循全面开源开放的生态理念，携手行业伙伴推动 RISC-V 高性能通用计算产业落地；打造覆盖“云、边、端”的全场景产品矩阵，为数据中心、AIGC、城市运营、智能制造、智能终端等多元场景提供算力产品及整体解决方案。
- 算能在北京、上海、深圳、青岛、厦门等国内 10 多个城市及美国、新加坡等国家设有研发中心。
- 自 2016 年以来，旗下品牌算丰 SOPHON 系列产品已完成多次迭代，每代产品相较于前代产品均实现了能耗比倍数级提升。
- 官方网站：https://www.sophgo.com/
- 开发社区：https://developer.sophgo.com/forum/index.html

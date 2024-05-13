<!-- 简体中文 | [English](README-en.md) -->

# 目录
- [目录](#目录)
- [项目简介](#项目简介)
  - [硬件资料](#硬件资料)
  - [处理器规格](#处理器规格)
- [SDK目录结构](#sdk目录结构)
  - [Solutions](#solutions)
    - [解决方案目录结构](#解决方案目录结构)
      - [customization说明](#customization说明)
- [SDK编译使用说明](#sdk编译使用说明)
  - [编译环境准备](#编译环境准备)
    - [Ubuntu安装Python \& pip](#ubuntu安装python--pip)
    - [YOCTOOL安装](#yoctool安装)
  - [编译步骤及说明](#编译步骤及说明)
    - [编译步骤](#编译步骤)
    - [镜像文件说明](#镜像文件说明)
  - [烧录](#烧录)
    - [TF卡烧录](#tf卡烧录)
    - [USB烧录](#usb烧录)
  - [运行说明](#运行说明)
- [FAQ](#faq)
- [关于算能](#关于算能)

<br>

<!-- # 版本说明
master：稳定版本

v2.0.0_golden：最新版本 -->


# 项目简介
- 本仓库提供[算能科技](https://www.sophgo.com/)端侧处理器`CV181x`和`CV180x`两个系列处理器的软件开发包(SDK)
- 主要适用于官方EVB

<br>

## 硬件资料
- [《CV180xB EVB板硬件指南》](https://sophon-file.sophon.cn/sophon-prod-s3/drive/23/03/14/14/CV180xB_EVB%E6%9D%BF%E7%A1%AC%E4%BB%B6%E6%8C%87%E5%8D%97_V1.0.pdf)
- [《CV180xC EVB板硬件指南》](https://sophon-file.sophon.cn/sophon-prod-s3/drive/23/03/18/18/CV180xC_EVB%E6%9D%BF%E7%A1%AC%E4%BB%B6%E6%8C%87%E5%8D%97_V1.0.pdf)
- [《CV181xC EVB板硬件指南》](https://sophon-file.sophon.cn/sophon-prod-s3/drive/23/03/15/14/CV181xC_QFN_EVB%E6%9D%BF%E7%A1%AC%E4%BB%B6%E6%8C%87%E5%8D%97_V1.0.pdf)
- [《CV181xH EVB板硬件指南》](https://sophon-file.sophon.cn/sophon-prod-s3/drive/23/03/15/15/CV181xH_EVB%E6%9D%BF%E7%A1%AC%E4%BB%B6%E6%8C%87%E5%8D%97_V1.0.pdf)

<br>

## 处理器规格
- [处理器产品简介](https://www.sophgo.com/product/index.html)

<br>

# SDK目录结构
```
.
├── boards      //板卡相关配置
├── components  //各种类型组件，例如aos内核，cli命令行，网络组件，以及CSI驱动
├── docs        //使用说明文档
├── host-tools  //交叉编译工具链
└── solutions   //解决方案目录，主要存放业务逻辑，运行bin等
```

<br>

## Solutions
<!-- | Solution Name | Description | Link |
| --- | ---| --- |
| barcode_scan | | [barcode_scan.md](docs/barcode_scan.md)|
| cv181x_boot | | [cv181x_boot.md](docs/cv181x_boot.md)|
| cv181xh_boot | | [cv181xh_boot.md](docs/cv181xh_boot.md)|
| ipc | | [ipc.md](docs/ipc.md)|
| peripherals_test | | [peripherals_test.md](docs/peripherals_test.md)|
| smart_doorbell | | [smart_doorbell.md](docs/smart_doorbell.md)|
| smart_pad | | [smart_pad.md](docs/smart_pad.md)|
| usb_cam | | [usb_cam.md](docs/usb_cam.md)|

<br> -->

### 解决方案目录结构
```
.
├── application     //应用相关部分
├── customization   //客制化应用相关以及不同产品形态配置
├── generated       //编译产生的镜像存储目录
├── py_tool         //python的工具
├── script          //编译的脚本
└── package.yaml    //用于定义和开启功能宏
```
package.yaml 请参考 https://help.aliyun.com/document_detail/308617.html
<br>
#### customization说明
  | Name                  | Description                                   |
  | ---                   | ---                                           |
  | custom_sysparam.c     | vb配置                                        |
  | custom_viparam.c      | vi配置 sensor相关配置                          |
  | custom_voparam.c      | vo配置                                        |
  | custom_vpssparam.c    | vpss配置                                      |
  | custom_vencparam.c    | venc的配置                                    |
  | custom_platform.c     | 平台相关配置，如pinmux设定，GPIO拉高等操作      |

<br>

# SDK编译使用说明

## 编译环境准备
### Ubuntu安装Python & pip
- 安装Python3
- 确保Python3优先级最高
  ```
  // 显示python可用版本列表
  $ update-alternatives --list python
  // Python使用版本优先级设置
  $ update-alternatives --install /usr/bin/python python /usr/bin/python3.X 2
  ```
- 安装pip，执行如下命令
  ```
  $ sudo apt-get install python3-pip
  $ sudo pip3 install --upgrade pip
  ```
- 安装python的serial和usb驱动模块，执行如下命令
  ```
  $ sudo pip3 install pyserial
  $ sudo pip3 install pyusb
  ```
- 安装yaml模块，执行如下命令
  ```
  $ sudo pip3 install pyyaml
  ```
> [!NOTE]
> 如果上面pyyaml安装失败，可以使用下面命令安装
>
> `$ sudo pip3 install pyyaml -i http://pypi.douban.com/simple --trusted-host pypi.douban.com`
>
> Ubuntu版本推荐高于20.0.4

### YOCTOOL安装
- Linux环境中使用指令安装yoctool
  ```
  $ sudo pip3 install yoctools -U
  ```
- 使用`yoc -V`查看对应版本，确保版本处于或者高于**2.0.54**
  > cvi_tek@WORKSTATION:cvi_alios_open$ ls -gGh boards/tools/fip/181x/
total 888K
    -rw-rw-r-- 1 397K 7月   6 20:11 fip.bin
    -rw-rw-r-- 1 485K 7月   6 20:11 fip.bin.secureboot
- 输入product version 查看product的版本，确保product安装成功
  > cvi_tek@WORKSTATION:cvi_alios_open$ product version
    v1.0.45
    Mar 17 2023,14:16:38


## 编译步骤及说明

本文说明中采用相关配置如下
| Name | Type |
| ---  | ---  |
| board | CV1810C-WEVB-0006A-SPINOR |
| sensor | 双目GC2053+GC2093 |
| git branch | v2.0.0_golden |
| solution | smart_doorbell |
| project | turnkey_qfn |

### 编译步骤
- 在顶层目录进行yoc初始化（只需执行一次）
  ```
  $ yoc init
  ```
- 在顶层目录下使用以下指令进行编译操作
  ```
  $ make smart_doorbell PROJECT=turnkey_qfn
  ```
  生成的镜像位于/solutions/solution_name/generated/images目录下
  > cvi_tek@WORKSTATION:cvi_alios_open$ ls -gGh solutions/smart_doorbell/generated/images
    total 6.1M
    -rw-rw-r-- 1 113K 12月 31  1979 boot
    -rw-rw-r-- 1  44K 12月 31  1979 boot0
    -rw-rw-r-- 1 2.6K 12月 31  1979 config.yaml
    -rw-rw-r-- 1  33K 12月 31  1979 fip_fsbl.bin
    -rw-rw-r-- 1 8.0K 12月 31  1979 imtb
    -rw-rw-r-- 1  404 12月 31  1979 partition_alios_spinor.xml
    -rw-rw-r-- 1 3.0M 12月 31  1979 prim
    -rw-rw-r-- 1 3.0M 12月 31  1979 yoc.bin

  生成的fip.bin位于/boards/tools/fip/board_type目录下
  > cvi_tek@WORKSTATION:cvi_alios_open$ ls -gGh boards/tools/fip/181x/
    total 888K
    -rw-rw-r-- 1 397K 7月   6 20:11 fip.bin
    -rw-rw-r-- 1 485K 7月   6 20:11 fip.bin.secureboot




### 镜像文件说明
| Name | Description |
| --- | --- |
| fip_fsbl.bin | 一级boot，文件boot0为其重命名 |
| boot | 二级bootloader |
| imtb | 分区表 |
| yoc.bin | 运行程序bin执行档，文件prim为其重命名 |
| partition_alios_spinor.xml | xml分区表，usb烧录时需要使用 |

<br>

## 烧录

### TF卡烧录
- TF卡格式化为FAT32
- 将[编译步骤](#编译步骤)中生成的镜像文件以及fip.bin拷贝至TF卡
- 设备断电后插入TF卡，重新上电，等待**Start SD downloading**烧录提示
- 有以下对应boot, imtb, yoc.bin等烧录记录则代表烧录成功
  > \## Resetting to default environment
    Start SD downloading...
    spinor id = C8 40 18
    SF: Detected GD25Q128E with page size 256 Bytes, erase size 4 KiB, total 16 MiB
    8192 bytes read in 1 ms (7.8 MiB/s)
    flash->erase_size 0x1000
    get flash erase size = 0x1000
    45056 bytes read in 22 ms (2 MiB/s)
    SF: 53248 bytes @ 0x0 Erased: OK
    device 0 offset 0x0, size 0xc000
    SF: 49152 bytes @ 0x0 Written: OK
    sf write speed 0.722 MB/s
    program boot0 success
    115400 bytes read in 54 ms (2 MiB/s)
    SF: 131072 bytes @ 0xd000 Erased: OK
    device 0 offset 0xc000, size 0x20000
    SF: 131072 bytes @ 0xc000 Written: OK
    sf write speed 0.771 MB/s
    program boot success
    8192 bytes read in 5 ms (1.6 MiB/s)
    SF: 8192 bytes @ 0x2d000 Erased: OK
    device 0 offset 0x2c000, size 0x2000
    SF: 8192 bytes @ 0x2c000 Written: OK
    sf write speed 0.819 MB/s
    program imtb success
    2931072 bytes read in 1380 ms (2 MiB/s)
    SF: 4915200 bytes @ 0x2f000 Erased: OK
    device 0 offset 0x2e000, size 0x4b0000
    SF: 4915200 bytes @ 0x2e000 Written: OK
    sf write speed 0.784 MB/s
    program prim success
    ** Unable to read file weight **
    Failed to load 'weight'
- 固件升级完成后拔掉TF卡，重新上电启动

<br>

### USB烧录
请参考 [CVI AliOS USB Burn](docs/cvi_alios_usb_burn.md)

<br>

## 运行说明
- 烧录完毕后关闭电源等待1-2s，存在电容放电需要等释放完毕
- 上电有如下打印###YOC###证明内核启动完毕
  > ##cur_ms:0
    j 0x80040000
    ##cur_ms:94
    ###YoC###[Aug 23 2023,10:11:27]
    spinor: ID = c8 40 18
    [VI-ERR] gc2053_1l_cmos.c:528:cmos_set_wdr_mode(): WDR_MODE_NONE
    [ISP-ERR] [E] AE_GetStatisticsConfig::4237 : pstAeStatisticsCfg is NULL.
    [ISP-ERR] [E] AE_SetStatisticsConfig::4223 : pstAeStatisticsCfg is NULL.
    ViPipe:0,===GC2093 1080P 30fps 10bit LINE Init OK!===
    [ISP-ERR] [E] AE_SetStatisticsConfig::4223 : pstAeStatisticsCfg is NULL.
    ViPipe:1,===GC2053_1L 1080P 30fps 10bit LINE Init OK!===
    get hw version 0 !!!
    [   0.201]\<I>app app_main.c[129]: app start........(cli-uart)#
    APP_CustomEventStart 12.
    (cli-uart)#
    (cli-uart)#
- 输入串口回车可以正常输入输出，并且有cli_uart打印即运行正常

<br>

# FAQ
请参考 https://developer.sophgo.com/thread/513.html

<br>

# 关于算能
- 算能致力于成为全球领先的定制算力提供商，专注于RISC-V、TPU处理器等算力产品的研发和推广应用。
- 公司遵循全面开源开放的生态理念，携手行业伙伴推动RISC-V高性能通用计算产业落地；打造覆盖“云、边、端”的全场景产品矩阵，为数据中心、AIGC、城市运营、智能制造、智能终端等多元场景提供算力产品及整体解决方案。
- 算能在北京、上海、深圳、青岛、厦门等国内10多个城市及美国、新加坡等国家设有研发中心。
- 自2016年以来，旗下品牌算丰SOPHON系列产品已完成多次迭代，每代产品相较于前代产品均实现了能耗比倍数级提升。
- 官方网站：https://www.sophgo.com/
- 开发社区：https://developer.sophgo.com/forum/index.html

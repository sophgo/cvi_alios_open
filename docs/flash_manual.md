# Cvi Alios Flash 说明手册

## Flash 配置文件说明

Flash 分区配置文件路径：

- /boards/cv180xb_qfn/configs/config.yaml
- /boards/cv181xc_qfn/configs/config.yaml

<!-- Flash支持列表文件路径：
- /components/chip_cv180x/src/drivers/spiflash/v2.0/src/spi_nor.c -->

<!-- ## 新增Flash支持 -->
<br>

## Platform

**Board：**
CV1810C-WEVB-0006A

**Solution：**
smart_doorbell

**Project：**
turnkey_qfn

<br>

## 分区配置说明

本文以`/boards/cv181xc_qfn/configs/config.yaml`中`prim`分区的配置进行说明

```yaml
- {
    name: prim,
    address: 0x02E000,
    size: 0x4B0000,
    index: 0,
    filename: yoc.bin,
    load_addr: 0x80040000,
    update: FULL,
    car: xz,
  } # 4.5 MB
```

| Name      | Description                  |
| --------- | ---------------------------- |
| name      | 分区名称                     |
| address   | 该分区相对于整块 Flash 偏移  |
| size      | 分区大小                     |
| index     | 分区类型，参照 storages 序列 |
| filename  | 写入该分区的文件名           |
| load_addr | 分区内存加载起始地址         |
| update    | 分区升级类型（FULL/DIFF）    |
| car       | 文件压缩方式（NULL/XZ）      |

<br>

## Flash 分区文件系统配置

**LittleFS 说明**

LittleFS 是一个为微控制器设计的开源嵌入式文件系统

**文件系统打包**

- 升级编译环境的 YoC 至最新版本
- 执行`product lfs`命令可查看相关参数说明
- 创建待打包文件夹，并放入打包文件
- 执行`product lfs ./lfs.bin -c my_lfs -b 4096 -s 0x400000`
  - 该命令将`my_lfs`文件夹打包成`lfs.bin`镜像，文件系统大小为 4MB，块大小为 4KB

**分区配置修改**

在`config.yaml`中新增`lfs`分区配置，大小为 4MB，烧录文件`lfs.bin`

```yaml
- {
    name: lfs,
    address: 0x4DE000,
    size: 0x400000,
    index: 0,
    filename: lfs.bin,
  } # 4 MB
```

> [!NOTE]
> 配置Flash分区地址和大小时，请确保未与其他分区空间重叠

**添加VFS挂载**

修改对应solution下`common_yocsystem.c`文件配置，增加VFS挂载，将`lfs`分区挂载到`/mnt/data`下

```c
#if CONFIG_SUPPORT_NORFLASH || CONFIG_PARTITION_SUPPORT_SPINANDFLASH
	int fatfs_en = 0;
    ret = partition_init();
    if(ret >= 0)
    {
        fatfs_en = app_sd_detect_check();
        LOGE("app", fatfs_en ? "fatfs enable.\n" : "fatfs disable.\n");

        // mount lfs flash to /mnt/data
        ret = vfs_lfs_register_with_path("lfs", "/mnt/data");

        if (ret != 0) {
           LOGE("app", "lfs register failed(%d)", ret);
        } else {
           LOGE("app", "lfs register succeed.");
        }
    }
    else
        printf("partition_init failed! ret = %d\n" , ret);
#endif
```

挂载成功后可对`/mnt/data`中文件内容进行读写
> cat功能使用需在yaml中配置`CONFIG_EXT4VFS_SUPPORT: 1`
```shell
(cli-uart)# ls /mnt/data
 .
 ..
 test.txt

(cli-uart)# cat /mnt/data/test.txt
Hello SOPHGO!
```


> [!NOTE]
> 若分区烧录文件为空，无需挂载，可进行裸读写
>
> 在应用代码中可使用VFS对文件内容进行读写，相关接口可参考[_Alios VFS API说明_](https://help.aliyun.com/zh/alios-things/developer-reference/vfs?spm=a2c4g.11186623.0.0.698f3726rc17EC#section-9sd-4nf-m40)


**文件系统烧录**

将`lfs.bin`放置于固件升级目录`images`下，采用TF卡或USB方式进行烧录

在开发过程中，若不存在`lfs.bin`升级的需求，可不重复烧写该文件，以避免分区被覆写并降低烧录耗时
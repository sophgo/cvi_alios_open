# FOTA 固件升级

- 当前 SDK 的 FOTA 逻辑在 boot 阶段进行
- FOTA 原理请参考[_空中固件升级_](https://yoc.docs.t-head.cn/yocbook/Chapter5-%E7%BB%84%E4%BB%B6/%E7%B3%BB%E7%BB%9F%E5%8D%87%E7%BA%A7/)

## 配置 FOTA 支持

### boot yaml 配置

FOTA yaml 相关配置文件路径：

- solutions/cv181x_boot/package.yaml

YAML 文件中需要开启的配置如下所示：

```yaml
  - sec_crypto: develop
  - key_mgr: develop
  - bootab: develop
  - xz: develop

  CONFIG_NO_OTA_UPGRADE: 0

  CONFIG_SEC_CRYPTO_AES_SW: 1
  CONFIG_SEC_CRYPTO_SHA_SW: 1
  CONFIG_SEC_CRYPTO_RSA_SW: 1
  CONFIG_SEC_CRYPTO_RNG_SW: 1
```

### flash yaml 配置

配置文件路径：

- boards/cv181xc_qfn/configs/config.yaml

对于有 FOTA 需求的分区，需增加`update`和`car`配置

以下为供参考的分区配置，在该配置中挂载了新的分区 `lfs`并进行 FOTA

```yaml
partitions:
  - {
      name: lfs,
      address: 0x4DE000,
      size: 0x850000,
      index: 0,
      filename: lfs.bin,
      update: FULL,
      car: xz,
    }
```

> [!NOTE]
> 其中，`misc`分区用作升级备份分区，升级固件通过网络下载后默认存放至该分区中

### 升级固件放置

需要把固件上传到平头哥芯片开发社区生成升级包，并选择合适的升级策略

具体操作流程请参考[_升级方案_](https://yoc.docs.t-head.cn/yocbook/Chapter5-%E7%BB%84%E4%BB%B6/%E7%B3%BB%E7%BB%9F%E5%8D%87%E7%BA%A7/%E5%8D%87%E7%BA%A7%E6%96%B9%E6%A1%88.html)

### boot更新

- 配置完成后使用 `make cv181x_boot` 命令，编译生成支持 FOTA 的 **boot.bin**文件
  - 文件路径为`solutions/cv181x_boot`
- 将其重命名为 **boot** ，替换原固件中的 **boot** 文件并进行烧录
  > 正常编译流程默认会将`boards/cv18*/bootimgs/boot`文件拷贝至升级固件`images`文件夹下，可使用新**boot**进行替换

## FOTA 升级

FOTA 运行参考日志如下：

```shell
Welcome boot4.0!
build: May 13 2024 17:39:54
cpu clock is 0Hz
[mtb][D] [mtb_init, 50]mtb try to find dev type:0, id:0
[mtb][D] [mtb_init, 50]mtb try to find dev type:1, id:0
[mtb][D] [mtb_init, 53]find type:1, id:0 ok.
[mtb][D] [mtb_init, 113]find imtb 4
[mtb][D] [mtb_init, 121]imtb_size:444
[mtb][D] [mtbv4_init, 34]imtb using:0x800238d8, valid offset:0x2c000
[mtb][D] [mtbv4_init, 40]part_info.storage_info.type:1
[mtb][D] [mtbv4_init, 41]part_info.storage_info.id:0
[mtb][D] [mtbv4_init, 42]part_info.storage_info.area:0
[mtb][D] [mtbv4_init, 48]part_info.end_addr:0x2E000
[mtb][D] [mtbv4_init, 49]part_info.start_addr:0x2C000
[mtb][D] [mtbv4_init, 50]flash_info.base_addr:0x0
[mtb][D] [mtbv4_init, 58]imtb backup_offset:0x2d000
[mtb][D] [mtbv4_init, 59]imtb one-size:0x1000
[mtb][D] [mtbv4_init, 60]mtb init over
[mtb][D] [mtbv4_crc_check, 197]imtb crc verify ok
[mtb][D] [get_sys_partition, 288]get_sys_partition----------
[mtb][D] [get_sys_partition, 289]mtb.prim_offset: 0x2c000
[mtb][D] [get_sys_partition, 290]mtb.backup_offset: 0x2d000
[mtb][D] [get_sys_partition, 291]mtb.using_addr: 0x800238d8
[mtb][D] [get_sys_partition, 292]mtb.one_size: 0x1000
[mtb][D] [get_sys_partition, 293]mtb.version: 0x4
[mtb][D] [get_sys_partition, 294]mtb.device_type: 0x1
[mtb][D] [get_sys_partition, 295]mtb.device_id: 0x0
[mtb][D] [get_sys_partition, 296]mtb.device_area: 0x0
line 48 -- func --boot_main
line 18 -- func --boot_pre_ota_hook
line 59 -- func --boot_main
line 126 -- func --update_init
line 133 -- func --update_init
[boot][D] g_flash_misc_addr:0xc2e000
[boot][D] g_flash_misc_fota_data_addr:0xc30000
[boot][D] g_flash_misc_section_size(erase_size):0x1000
[boot][D] g_flash_misc_size:0x380000
line 140 -- func --update_init
[boot][D] misc_addr=0x80022220
[boot][D] misc_control->status:0xffffffff
[boot][D] is_continue:0
[boot][D] signature_addr:0xf2759c, hash_addr:0xf2769c, hash_len:20
[boot][D] fota_data_addr:0xc30000, fota_data_size:3110300
[mtb][D] [hash_calc_start, 225]ds:1, ilen:3110300, from_mem:0
[mtb][D] [hash_calc_start, 252]sha ok, type:1, outlen:20
[boot][I] fota data hash verify ok
[boot][D] pmtb
[boot][D] misc check ret:1
[boot][D] update mode
[boot][I] start to upgrade
[boot][D] control_state:0
[boot][D] type=3, g_flash_misc_addr=0xc2e000, g_flash_misc_section_size=0x1000
[boot][D] sum:1,imgstart:0x80022250,first_fd:0x80022250
[boot][D] misc_first_fd=0x80022250
[boot][D] ==================>>>>prim
[boot][D] misc get fd over, 0x8002b6a0
[boot][D] continue fd:0x80022250,status:0
line 39 -- func --update_process
[boot][D] check the needed upgrade partition first.
[boot][D] got fd 0x80022250
[boot][D] got img_addr:0xc30040,img_type:10,img_size:3110236,
[boot][D] fd in:0x80022250
[boot][D] fd no
[boot][D] check the needed upgrade partition ok.
[boot][D] got fd 0x80022250
[boot][D] got img_addr:0xc30040,img_type:10,img_size:3110236,
line 79 -- func --update_process
[boot][D] update_type.type:0x10,update_type.type_t.img_type:1,update_type.type_t.update_type:0,reserve:0
[boot][D] PAD type:0
[boot][D] img_type 1,type_flag:0
[boot][D] type=1, g_flash_misc_addr=0xc2e000, g_flash_misc_section_size=0x1000
[boot][D] set fd:0x80022250,stas:0
[boot][I] fd:0x80022250,fd_num:0
[mtb][D] [get_section_buf, 244]addr:0xc30040, img_len:3110236
[mtb][D] [get_section_buf, 245]device_type:1, device_id:0, area:0
[mtb][D] [get_section_buf, 251]r offset:0xc30040
[mtb][D] [get_section_buf, 261]fatype=0,sontype=0,size=304
[boot][D] tmp_addr:0x170,update_img_addr:0xc31000,update_img_size:3106204,tmp_size:304
[boot][D] update img part_addr:0x2e000-img_name:prim-img_addr:c31000-img_size:3106204-max_size:8388608-update_type:0
[boot][D] update image update_type:0 car:1 0x2e000
[boot][I] start FULL update[prim]
[boot][I] start to erase ...
[boot][I] start xz decompress
[boot][I] FULL update over[prim]
[boot][D] type=1, g_flash_misc_addr=0xc2e000, g_flash_misc_section_size=0x1000
[boot][D] set fd:0x80022250,stas:1
[boot][I] fd:0x80022250,fd_num:0
[mtb][D] [get_section_buf, 244]addr:0xc30040, img_len:3110236
[mtb][D] [get_section_buf, 245]device_type:1, device_id:0, area:0
[mtb][D] [get_section_buf, 251]r offset:0xc30040
[mtb][D] [get_section_buf, 261]fatype=0,sontype=0,size=304
[boot][D] update mate info img_update->img_addr:c30040-img_name:prim-img_size:304
[boot][D] part_info.img_size:0x5a0b40, scn_img.imginfo.image_size:0x5a0b40
[boot][I] update backup mtb, prim
[boot][D] part_info.img_size:0x5a0b40, partition_new_info->img_size:0x5a0b40
[boot][I] version:[1.0.1-20240513.1846-R-S2_4G_0]
[boot][D] type=1, g_flash_misc_addr=0xc2e000, g_flash_misc_section_size=0x1000
[boot][D] set fd:0x80022250,stas:2
[boot][I] fd:0x80022250,fd_num:0
[boot][I] update valid mtb
[boot][D] fd in:0x80022250
[boot][D] fd no
[boot][I] suc update ^_^
[boot][I] misc reset at 0xc2e000, erase_len:0x3000
[boot][D] update ok. goto reboot.
C.SCS/0/0.WD.URPL.USBI.BS/NOR.PS.PE.BS.BE.J.some error occur!
```

在升级完成后，`misc`分区数据被擦除，在后续板端重新启动过程中，由于检测到该分区数据被清空，从而不会重复升级

重启日志如下：

```shell
Welcome boot4.0!
build: May 13 2024 17:39:54
cpu clock is 0Hz
[mtb][D] [mtb_init, 50]mtb try to find dev type:0, id:0
[mtb][D] [mtb_init, 50]mtb try to find dev type:1, id:0
[mtb][D] [mtb_init, 53]find type:1, id:0 ok.
[mtb][D] [mtb_init, 113]find imtb 4
[mtb][D] [mtb_init, 121]imtb_size:444
[mtb][D] [mtbv4_init, 34]imtb using:0x800238d8, valid offset:0x2c000
[mtb][D] [mtbv4_init, 40]part_info.storage_info.type:1
[mtb][D] [mtbv4_init, 41]part_info.storage_info.id:0
[mtb][D] [mtbv4_init, 42]part_info.storage_info.area:0
[mtb][D] [mtbv4_init, 48]part_info.end_addr:0x2E000
[mtb][D] [mtbv4_init, 49]part_info.start_addr:0x2C000
[mtb][D] [mtbv4_init, 50]flash_info.base_addr:0x0
[mtb][D] [mtbv4_init, 58]imtb backup_offset:0x2d000
[mtb][D] [mtbv4_init, 59]imtb one-size:0x1000
[mtb][D] [mtbv4_init, 60]mtb init over
[mtb][D] [mtbv4_crc_check, 197]imtb crc verify ok
[mtb][D] [get_sys_partition, 288]get_sys_partition----------
[mtb][D] [get_sys_partition, 289]mtb.prim_offset: 0x2c000
[mtb][D] [get_sys_partition, 290]mtb.backup_offset: 0x2d000
[mtb][D] [get_sys_partition, 291]mtb.using_addr: 0x800238d8
[mtb][D] [get_sys_partition, 292]mtb.one_size: 0x1000
[mtb][D] [get_sys_partition, 293]mtb.version: 0x4
[mtb][D] [get_sys_partition, 294]mtb.device_type: 0x1
[mtb][D] [get_sys_partition, 295]mtb.device_id: 0x0
[mtb][D] [get_sys_partition, 296]mtb.device_area: 0x0
line 48 -- func --boot_main
line 18 -- func --boot_pre_ota_hook
line 59 -- func --boot_main
line 126 -- func --update_init
line 133 -- func --update_init
[boot][D] g_flash_misc_addr:0xc2e000
[boot][D] g_flash_misc_fota_data_addr:0xc30000
[boot][D] g_flash_misc_section_size(erase_size):0x1000
[boot][D] g_flash_misc_size:0x380000
line 140 -- func --update_init
[boot][D] misc_addr=0x80022220
[boot][W] misc fota data e
[boot][D] misc check ret:-1
[boot][W] e misc file
load img & jump to [prim]
load&jump 0x2e000,0x80040000,5901120
start copy 5901120 bytes
##cur_ms:169
##cur_ms:348
all copy over..
j m
j 0x80040000
[03]###YoC### [May 11 2024,17:23:00]
 YOC_SYSTEM_FsVfsInit
spinor: ID = ef 40 18

```

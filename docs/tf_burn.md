# CVI AliOS TF Burn

- TF卡格式化为FAT32
- 将生成的镜像文件以及fip.bin拷贝至TF卡
- 设备断电后插入TF卡，重新上电，等待**Start SD downloading**烧录提示
- 有以下对应boot, imtb, yoc.bin等烧录记录则代表烧录成功
    ```
    ## Resetting to default environment
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
    ```
- 固件升级完成后拔掉TF卡，重新上电启动
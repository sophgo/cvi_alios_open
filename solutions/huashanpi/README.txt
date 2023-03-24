#STEP1,拷贝solutions/ipc/generated/images下的固件到sd卡；
boot  boot0.emmc  imtb  prim


#STEP2,使用uboot升级固件

mmc dev 0 1#切到emmc—boot1分区，升级一级boot
fatload mmc 1:0 0x80200000 boot0.emmc
mmc write 0x80200000 0 0x400

mmc dev 0 2#切到emmc—boot2分区，升级二级boot
fatload mmc 1:0 0x80200000 boot
mmc write 0x80200000 0 0x400

mmc dev 0 0##切到emmc—user分区，升级分区表和应用
mmc erase 0 0x8000
fatload mmc 1:0 0x80200000 imtb
mmc write 0x80200000 0x0 0x500 
fatload mmc 1:0 0x80200000 prim
mmc write 0x80200000 0x1400 0x2000


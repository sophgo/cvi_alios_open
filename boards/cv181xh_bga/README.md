# the bsp for cv181xh

# 关于KP文件和签名

keystore目录下的key.pem和pubkey.pem是一对公私钥，由用户自己生成并保管。

## RSA公私钥生成

1. PEM私钥格式文件

```bash
openssl genrsa -out key.pem 1024
```

2. PEM公钥格式文件

```bash
openssl rsa -in key.pem -pubout -out pubkey.pem
```

## KP文件生成

执行以下命令生成kp.bin文件，并拷贝到`bootimgs`目录下，重命名为kp。

```bash
product kp kp.bin -t RSA1024 -dt SHA1 -k keystore/pubkey.pem
cp kp.bin bootimgs/kp
```

## 固件签名

固件签名已经在`script/aft_build.sh`脚本中默认集成，会使用`keystore/key.pem`文件进行签名。

## FOTA镜像签名

FOTA镜像签名已经在`script/aft_build.sh`脚本中默认集成，会使用`keystore/key.pem`文件进行签名。

# 支持EMMC

替换config.yaml文件
cp configs/config.emmc.yaml configs/config.yaml

# 支持SPI NAND Flash
当需要使用 NAND Flash 作为固件存储介质时，需要将configs/config.yaml文件的内容替换为configs/config.nand.yaml文件的内容。

# 支持SPI NOR Flash
当需要使用 NOR Flash 作为固件存储介质时，需要将configs/config.yaml文件的内容替换为configs/config.flash.yaml文件的内容。

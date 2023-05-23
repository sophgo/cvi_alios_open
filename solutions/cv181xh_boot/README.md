# the bootloader for cr182x

#how to build cv181xh_boot
CHIP_COMP=../../components/cvi_mmf_sdk/chip_cv181x

cp $CHIP_COMP/package.yaml $CHIP_COMP/package.yaml.bak
cp $CHIP_COMP/package.yaml.boot $CHIP_COMP/package.yaml

make

cp $CHIP_COMP/package.yaml.bak $CHIP_COMP/package.yaml

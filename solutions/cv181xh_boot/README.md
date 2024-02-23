# the bootloader for cr182x

#how to build cv181xh_boot
CHIP_COMP=../../components/cvi_mmf_sdk/chip_cv181x

cp $CHIP_COMP/package.yaml $CHIP_COMP/package.yaml.bak
cp $CHIP_COMP/package.boot.yaml $CHIP_COMP/package.yaml

if [[ $1 = "nand" ]];then
    echo "make for nand flash"
    cp package.yaml package.yaml.bak -arf
    cp package.nand.yaml package.yaml -arf
    make;
    mv package.yaml.bak package.yaml
else
    make;
fi

cp $CHIP_COMP/package.yaml.bak $CHIP_COMP/package.yaml

#!/bin/sh

BASE_PWD=`pwd`
MK_BOARD_PATH=$BOARD_PATH
MK_CHIP_PATH=$CHIP_PATH
MK_SOLUTION_PATH=$SOLUTION_PATH
MK_SOLUTION_PARTITION_NAME=
MK_OPTARG=
echo "[INFO] Generated output files ..."
echo $BASE_PWD

EXE_EXT=`which ls | grep -o .exe`
if [ -n "$EXE_EXT" ]; then
    echo "I am in CDK."
    OBJCOPY=riscv64-unknown-elf-objcopy
    ELF_NAME=`ls Obj/*.elf`
    $OBJCOPY -O binary $ELF_NAME yoc.bin
    PRODUCT=$MK_BOARD_PATH/configs/product$EXE_EXT
else
    echo "I am in Linux."
    while getopts ":s:b:c:a:" optname
    do
        case "$optname" in
        "s")
            MK_SOLUTION_PATH=$OPTARG
            ;;
        "b")
            MK_BOARD_PATH=$OPTARG
            ;;
        "c")
            MK_CHIP_PATH=$OPTARG
            ;;
        "a")
            # echo "the all variables from yoctools, value is $OPTARG"
            MK_OPTARG=$OPTARG
            ;;
        "h")
            ;;
        ":")
            echo "No argument value for option $OPTARG"
            ;;
        "?")
            echo "Unknown option $OPTARG"
            ;;
        *)
            echo "Unknown error while processing options"
            ;;
        esac
        #echo "option index is $OPTIND"
    done
    PRODUCT=product
fi

MK_SOLUTION_PARTITION_NAME=$(echo ${MK_OPTARG} | grep "CONFIG_CUSTOM_PARTITION_NAME" | awk -F: '{print $2}')
MK_GENERATED_PATH=${MK_SOLUTION_PATH}/generated
rm -fr $MK_GENERATED_PATH
mkdir -p $MK_GENERATED_PATH/data/

echo $MK_SOLUTION_PATH
echo $MK_BOARD_PATH
echo $MK_CHIP_PATH
echo $MK_GENERATED_PATH

if [ -d data ]; then
    LFS_SIZE=$(cat $MK_BOARD_PATH/configs/config.yaml | grep lfs | sed 's/[[:space:]\"]//g' | awk -F 'size:|}' '{print $2}' | xargs printf "%d\n")
	$PRODUCT lfs ${MK_GENERATED_PATH}/data/lfs -c data -b 4096 -s ${LFS_SIZE}
	cp -arf ${MK_GENERATED_PATH}/data/lfs  ${MK_GENERATED_PATH}/littlefs.bin
fi

if [ -d bin ]; then
	cp -arf bin/*  ${MK_GENERATED_PATH}/data/
fi

[ -f "${MK_BOARD_PATH}/bootimgs/boot" ] && cp -arf ${MK_BOARD_PATH}/bootimgs/boot ${MK_GENERATED_PATH}/data/
[ -f "${MK_BOARD_PATH}/bootimgs/boot0" ] && cp -arf ${MK_BOARD_PATH}/bootimgs/boot0 ${MK_GENERATED_PATH}/data/

[ -f yoc.bin ] && cp -arf yoc.bin ${MK_GENERATED_PATH}/data/prim
[ -f yoc.bin ] && cp -arf yoc.bin ${MK_GENERATED_PATH}/data/prima
if [ ${MK_SOLUTION_PARTITION_NAME} != "" ]; then
    echo "MK_SOLUTION_PARTITION_NAME is ${MK_SOLUTION_PARTITION_NAME}"
    cp -arf ${MK_BOARD_PATH}/configs/config.${MK_SOLUTION_PARTITION_NAME}.yaml ${MK_GENERATED_PATH}/data/config.yaml
    cp -arf ${MK_BOARD_PATH}/bootimgs/${MK_SOLUTION_PARTITION_NAME}/boot ${MK_GENERATED_PATH}/data/
    cp -arf ${MK_BOARD_PATH}/bootimgs/${MK_SOLUTION_PARTITION_NAME}/boot0 ${MK_GENERATED_PATH}/data/
else
    echo "default config.yaml copy"
    cp -arf ${MK_BOARD_PATH}/configs/config.yaml ${MK_GENERATED_PATH}/data/
fi
${PRODUCT} image ${MK_GENERATED_PATH}/images.zip -i ${MK_GENERATED_PATH}/data -l -p
${PRODUCT} image ${MK_GENERATED_PATH}/images.zip -e ${MK_GENERATED_PATH} -x

#fota image
echo "fota image generate..."
$PRODUCT diff -f ${MK_GENERATED_PATH}/images.zip ${MK_GENERATED_PATH}/images.zip -r -v "1.1" -o ${MK_GENERATED_PATH}/fota.bin

if [ ! -f gdbinitflash ]; then
    cp -arf $MK_BOARD_PATH/script/gdbinitflash $BASE_PWD
fi

if [ ! -f cdkinitflash ]; then
    cp -arf $MK_BOARD_PATH/script/cdkinitflash $BASE_PWD
fi

if [ ! -f gdbinit ]; then
    cp -arf $MK_BOARD_PATH/script/gdbinit $BASE_PWD
fi

cp -arf $MK_BOARD_PATH/script/mkflash.sh $BASE_PWD


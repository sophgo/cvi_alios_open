#!/bin/sh

BASE_PWD=`pwd`
MK_BOARD_PATH=$BOARD_PATH
MK_CHIP_PATH=$CHIP_PATH
MK_SOLUTION_PATH=$SOLUTION_PATH

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
cp -arf ${MK_BOARD_PATH}/configs/config.smart_pad.yaml ${MK_GENERATED_PATH}/data/config.yaml
${PRODUCT} image ${MK_GENERATED_PATH}/images.zip -i ${MK_GENERATED_PATH}/data -l -p
${PRODUCT} image ${MK_GENERATED_PATH}/images.zip -e ${MK_GENERATED_PATH} -x

#fota image
echo "fota image generate..."
#$PRODUCT diff -f ${MK_GENERATED_PATH}/images.zip ${MK_GENERATED_PATH}/images.zip -r -v "1.1" -o ${MK_GENERATED_PATH}/fota.bin


[ -f "${MK_BOARD_PATH}/bootimgs/fip_fsbl.bin" ] && cp -arf ${MK_BOARD_PATH}/bootimgs/fip_fsbl.bin ${MK_GENERATED_PATH}/data/
[ -f "${MK_BOARD_PATH}/configs/partition_alios_spinor.xml" ] && cp -arf ${MK_BOARD_PATH}/configs/partition_alios_spinor.xml ${MK_GENERATED_PATH}/data/
[ -f yoc.bin ] && cp -arf yoc.bin ${MK_GENERATED_PATH}/data/

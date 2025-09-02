#!/bin/bash

COLOR_END="\033[0m"
COLOR_RED="\033[1;31;40m"
COLOR_GREEN="\033[1;32;40m"

pr_info()
{
	echo -e ${COLOR_GREEN}${1}${COLOR_END}
}

pr_error()
{
	echo -e ${COLOR_RED}${1}${COLOR_END}
}


# env check
if [ -z $TOP_DIR ]; then
	pr_error "Error: You have to config the build environment and then build all first!"
	exit 1
fi
if [ -z $SDK_VER ]; then
	pr_error "Error: You have to config the build environment and then build all first!"
	exit 1
fi

# check input
if [[ -z "$1" ]]; then
	pr_error "Please enter install path !"
	exit 1
else
	pr_info "INSTALL_FOLDER: $1"
	INSTALL_FOLDER=$1
fi

LOCAL_DIR=${TOP_DIR}/"cvi_alios"

LIB_RLS_LIST=(
	"libvc_drv.a"
	"libcvi_mw_isp_ae.a"
	"libcvi_mw_isp_af.a"
	"libcvi_mw_isp_awb.a"
	"libcvi_mw_isp_algo.a"
	"libcvi_mw_isp.a"
	"libcvi_mw_audio.a"
)

pushd $LOCAL_DIR &>/dev/null

#copy .a
mkdir -p "$INSTALL_FOLDER"/components/cvi_mmf_sdk/lib
for lib in ${LIB_RLS_LIST[@]}
do
	find ./ -name "$lib" -type f -exec cp -lf {} "$INSTALL_FOLDER"/components/cvi_mmf_sdk/lib/ \;
done

#copy isp include
mkdir -p "$INSTALL_FOLDER"/components/cvi_mmf_sdk/cvi_middleware/include/isp
cp -rf ${TOP_DIR}/cvi_mpi/modules/isp/include/cv184x  "$INSTALL_FOLDER"/components/cvi_mmf_sdk/cvi_middleware/include/isp/
cp -f ${TOP_DIR}/cvi_mpi/modules/isp/algo/3A_internal.h  "$INSTALL_FOLDER"/components/cvi_mmf_sdk/cvi_middleware/include/isp/cv184x/

#copy audio include
mkdir -p "$INSTALL_FOLDER"/components/cvi_mmf_sdk/cvi_middleware/cvi_mw_audio
cp -rf ./components/cvi_mmf_sdk/cvi_middleware/cvi_mw_audio/include "$INSTALL_FOLDER"/components/cvi_mmf_sdk/cvi_middleware/cvi_mw_audio/

#copy vc include
cp components/cvi_mmf_sdk/cvi_osdrv/vc_drv/platform/alios/driver_vc.h "$INSTALL_FOLDER"/components/cvi_mmf_sdk/cvi_middleware/cvi_mw_venc/include/

if [ -f "./solutions/fastboot/package_yamls/package.yaml.turnkey.release" ]; then
cp ./solutions/fastboot/package_yamls/package.yaml.turnkey.release "$INSTALL_FOLDER"/solutions/fastboot/package_yamls/package.yaml.turnkey
fi
if [ -f "./solutions/normboot/package_yamls/package.yaml.turnkey.release" ]; then
cp ./solutions/normboot/package_yamls/package.yaml.turnkey.release "$INSTALL_FOLDER"/solutions/normboot/package_yamls/package.yaml.turnkey
fi

popd &>/dev/null

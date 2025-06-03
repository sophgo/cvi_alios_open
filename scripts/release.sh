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
	"libosal.a"
	"libbase.a"
	"libsys.a"
	"libsnsr_i2c.a"
	"libcif.a"
	"libvi.a"
	"libvpss.a"
	"librgn.a"
	"libvc_drv.a"
	"libldc.a"
	"libmipi_tx.a"
	"libvo.a"
	"libcvi_mw_sys.a"
	"libcvi_mw_cif.a"
	"libcvi_mw_sensor.a"
	"libcvi_mw_vi.a"
	"libcvi_mw_isp_ae.a"
	"libcvi_mw_isp_af.a"
	"libcvi_mw_isp_awb.a"
	"libcvi_mw_isp_algo.a"
	"libcvi_mw_isp.a"
	"libcvi_mw_vpss.a"
	"libcvi_mw_mipi_tx.a"
	"libcvi_mw_vo.a"
	"libcvi_mw_rgn.a"
	"libcvi_mw_gdc.a"
	"libcvi_mw_venc.a"
	"libcvi_mw_vdec.a"
	"libcvi_mw_audio.a"
	"libcvi_link.a"
	"libcvi_msg.a"
)

pushd $LOCAL_DIR &>/dev/null

#copy .a
mkdir -p "$INSTALL_FOLDER"/components/cvi_mmf_sdk/lib
for lib in ${LIB_RLS_LIST[@]}
do
	find ./ -name "$lib" -type f -exec cp -lf {} "$INSTALL_FOLDER"/components/cvi_mmf_sdk/lib/ \;
done

#copy mw include
mkdir -p "$INSTALL_FOLDER"/components/cvi_mmf_sdk/cvi_middleware
cp -rlf ./components/cvi_mmf_sdk/cvi_middleware/include "$INSTALL_FOLDER"/components/cvi_mmf_sdk/cvi_middleware/

#copy msg include
mkdir -p "$INSTALL_FOLDER"/components/cvi_mmf_sdk/cvi_msg
cp -rlf ./components/cvi_mmf_sdk/cvi_msg/include "$INSTALL_FOLDER"/components/cvi_mmf_sdk/cvi_msg/

#copy panel
cp -rlf ./components/cvi_mmf_sdk/cvi_panel "$INSTALL_FOLDER"/components/cvi_mmf_sdk/

#copy sensor
cp -rlf ./components/cvi_mmf_sdk/cvi_sensor "$INSTALL_FOLDER"/components/cvi_mmf_sdk/

#copy ipcm softlink
mkdir -p "$INSTALL_FOLDER"/components/cvi_mmf_sdk/cvi_osdrv
ln -sf ../../../../osdrv/interdrv/ipcm "$INSTALL_FOLDER"/components/cvi_mmf_sdk/cvi_osdrv/

if [ -f "./solutions/fastboot/package_yamls/package.yaml.turnkey.release" ]; then
cp ./solutions/fastboot/package_yamls/package.yaml.turnkey.release "$INSTALL_FOLDER"/solutions/fastboot/package_yamls/package.yaml.turnkey
fi
if [ -f "./solutions/normboot/package_yamls/package.yaml.turnkey.release" ]; then
mv ./solutions/normboot/package_yamls/package.yaml.turnkey.release "$INSTALL_FOLDER"/solutions/normboot/package_yamls/package.yaml.turnkey
fi

popd &>/dev/null

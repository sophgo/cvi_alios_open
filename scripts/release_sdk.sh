#!/bin/bash
set -ex

#指定使用BOARD
BOARD_NAME="cv181xc_evb"
#指定使用solution
SOLUTION_NAME=${ALIOS_SOLUTION}
#指定solution 二级PROJECT默认为空
PROJECT_NAME="turnkey"
#默认使用的CUSTOMIZATION
CUSTOMIZATION_DIR1="turnkey"
CUSTOMIZATION_DIR2="cv180zb_2331_2331_1l_switch"
CUSTOMIZATION_DIR3="cv1811c_cv2003_1l_triple"
CUSTOMIZATION_DIR4="cv180zb_gc2053"

if [ -n "$1" ]; then
BOARD_NAME=$1
fi

if [ -n "$2" ]; then
SOLUTION_NAME=$2
fi

if [ -n "$3" ]; then
PROJECT_NAME=$3
fi

RELEASE_TIME=$(date '+%Y-%m-%d')

#RELEASE脚本路径
RELEASE_SH_DIR="$(cd "$( dirname "${BASH_SOURCE[0]}" )" > /dev/null 2>&1 && pwd )"
#顶层路径
TOP_DIR=${RELEASE_SH_DIR}/..
#solution目录
SDK_RELEASE_PATH=${TOP_DIR}/../${BOARD_NAME}_${SOLUTION_NAME}_${PROJECT_NAME}_cvitek_sdk
#板端配置
BOARD_CONFIG_FILE=${RELEASE_SH_DIR}/release_config/board/${BOARD_NAME}.config
#SOLUTION配置
SOLUTION_CONFIG_FILE=${RELEASE_SH_DIR}/release_config/solutions/${SOLUTION_NAME}_${PROJECT_NAME}.config
#MMF中cvi_middleware会存在depend的文件夹
MMF_DEPNED_ARRAY=""
#MMF中cvi_middleware会存在的多级include文件夹
MMF_INCLUDE_ARRAY=""
#TPU 会存在的depend文件夹
TPU_DEPNED_ARRAY=""
#TPU 会存在的多级include文件夹
TPU_INCLUDE_ARRAY=""

echo "BOARD_NAME=${BOARD_NAME}"
echo "SOLUTION_NAME=${SOLUTION_NAME}"
echo "PROJECT_NAME=${PROJECT_NAME}"
echo "TOP_DIR=${TOP_DIR}"
echo "SDK_RELEASE_PATH=${SDK_RELEASE_PATH}"

release_load_config()
{
	echo "Run ${FUNCNAME[0]}() function"
	pushd ${RELEASE_SH_DIR}
	#导入默认board配置
	source ${RELEASE_SH_DIR}/release_config/board/default.config
	#导入专属BOARD_CONFIG配置 如有替换直接覆盖变量
	if [ -f ${BOARD_CONFIG_FILE} ] ; then
		source ${BOARD_CONFIG_FILE}
	fi
	#导入默认SOLUTION配置
	source ${RELEASE_SH_DIR}/release_config/solutions/default.config
	#导入专属SOLUTION配置 如有替换直接覆盖变量
	if [ -f ${SOLUTION_CONFIG_FILE} ] ; then
		source ${SOLUTION_CONFIG_FILE}
	fi
	popd
}


release_pre_pack()
{
	echo "Run ${FUNCNAME[0]}() function"
	#先清空原来得环境
	pushd ${TOP_DIR}
	if [ -f .yoc ] ; then
		make clean PROJECT=${PROJECT_NAME}
		rm -f .yoc
	fi
	popd
	#新建立文件
	if [ -d ${SDK_RELEASE_PATH} ] ; then
		echo "rm SDK_RELEASE_PATH"
		rm -rf ${SDK_RELEASE_PATH}
	fi
	mkdir -p ${SDK_RELEASE_PATH}
	pushd ${SDK_RELEASE_PATH}
	ls ${TOP_DIR}/ |xargs -i cp -a ${TOP_DIR}/{} ./
	ls ./scripts | grep -v defconfig | grep -v yaml_update.py |xargs -i rm -rf ./scripts/{}
	ls ./boards | grep -v ${BOARD_NAME} |xargs -i rm -rf ./boards/{}
	ls ./solutions | grep -v ${SOLUTION_NAME} | xargs -i rm -rf ./solutions/{}

	mkdir -p solutions/${SOLUTION_NAME}/cvi_mmf_sdk
	mkdir -p solutions/${SOLUTION_NAME}/cvi_mmf_sdk/include
	popd
}


release_build()
{
	echo "Run ${FUNCNAME[0]}() function"
	pushd ${SDK_RELEASE_PATH}
	echo y | yoc init
	make clean PROJECT=${PROJECT_NAME};make ${SOLUTION_NAME} PROJECT=${PROJECT_NAME}
	if [ $? != 0 ] ; then
		popd
		echo "release build err"
		rm -rf ${SDK_RELEASE_PATH}
		exit 1
	fi
	popd
	echo "release_pre_build end"
}


release_components() 
{
	echo "run ${FUNCNAME[0]}() function. "
	#规则 mmf_sdk下面得 MMFSDK_CHIP_NAME 需要放components下
	#cvi_middleware需要用libs cvi_osdrv直接删掉吃libs 需要修改对应得目录
	local SDK_RELEASE_COMPONETS_PATH=${SDK_RELEASE_PATH}/components
	local COMPONECTS_MMFSDK_LIBS="$(ls ${SDK_RELEASE_COMPONETS_PATH}/cvi_mmf_sdk/cvi_middleware | grep -v "include")"
	local SDK_RELEASE_SOLUTIONS_PATH=${SDK_RELEASE_PATH}/solutions/${SOLUTION_NAME}
	pushd ${SDK_RELEASE_COMPONETS_PATH}
	MMF_DEPNED_ARRAY=$(ls ./cvi_mmf_sdk/cvi_middleware/ | grep -v "include")
	MMF_INCLUDE_ARRAY=$(find ./cvi_mmf_sdk/cvi_middleware/include/ -type d |\
		grep  "^./cvi_mmf_sdk/cvi_middleware/include" | \
		sed "{s/^\.\/cvi_mmf_sdk\/cvi_middleware\/include/include/g}")
	TPU_DEPNED_ARRAY=$(ls ./cvi_mmf_sdk/cvi_tpu/ | grep -v "include")
	TPU_INCLUDE_ARRAY=$(find ./cvi_mmf_sdk/cvi_tpu/include/ -type d |\
		 grep  "^./cvi_mmf_sdk/cvi_tpu/include" | sed "{s/^\.\/cvi_mmf_sdk\/cvi_tpu\/include/include/g}")
	cp -rf ./cvi_mmf_sdk/${MMFSDK_CHIP_NAME} ./
	rm -rf ./cvi_mmf_sdk/chip_*
	for mmfdir_array in ${DELETE_MMFSDK_DIRS[@]};
	do
		pushd ./cvi_mmf_sdk/${mmfdir_array}
		cp -rf ./include/* ${SDK_RELEASE_PATH}/solutions/${SOLUTION_NAME}/cvi_mmf_sdk/include/
		popd
		ls ./cvi_mmf_sdk | grep ${mmfdir_array} | xargs -i rm -rf ./cvi_mmf_sdk/{}
	done
	for components_array in ${DELETE_COMPONENTS_DIRS[@]};
	do
		pushd ./${components_array}
		mkdir ${SDK_RELEASE_PATH}/solutions/${SOLUTION_NAME}/cvi_mmf_sdk/include/${components_array}
		cp -rf ./include/* ${SDK_RELEASE_PATH}/solutions/${SOLUTION_NAME}/cvi_mmf_sdk/include/${components_array}/
		popd
		rm -rf ./${components_array}
	done
	#拷贝其余不需要依赖成库的组件出来
	cp -rf ./cvi_mmf_sdk/* ./
	rm -rf ./cvi_mmf_sdk
	popd
}

build_cvimmfsdk()
{
	echo "run ${FUNCNAME[0]}() function. "
	local SDK_RELAESE_COMPONENTS_PATH=${SDK_RELEASE_PATH}/components
	local SDK_RELEASE_SOLUTIONS_PATH=${SDK_RELEASE_PATH}/solutions/${SOLUTION_NAME}
	pushd ${SDK_RELAESE_COMPONENTS_PATH}
	mkdir cvi_mmf_sdk
	cd ./cvi_mmf_sdk
	mkdir lib
	mkdir include
	touch package.yaml
	echo "##" > package.yaml
	sed -i "1a\    source:" package.yaml
	sed -i "1a\  - dest: \"include/\"" package.yaml
	sed -i "1a\install:" package.yaml
	sed -i "1a\def_config:" package.yaml
	sed -i "1a\source_file:" package.yaml
	if [[ "$SOLUTION_NAME" == "normboot" ]]; then
	sed -i "1a\    - lib" package.yaml
	else
	sed -i "1a\    - lib/fastboot" package.yaml
	fi
	sed -i "1a\  libpath:" package.yaml
	sed -i "1a\  libs:" package.yaml
	sed -i "1a\  include:" package.yaml
	sed -i "1a\build_config:" package.yaml
	sed -i "1a\license: Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved." package.yaml
	sed -i "1a\  - base" package.yaml
	sed -i "1a\keywords:" package.yaml
	sed -i "1a\type: common" package.yaml
	sed -i "1a\description: Cvitek mmf SDK library." package.yaml
	sed -i "1a\version: develop" package.yaml
	sed -i "1a\name: cvi_mmf_sdk" package.yaml
	popd
	pushd ${SDK_RELEASE_SOLUTIONS_PATH}
	local SOLUTION_YAML_LIST="$(find ./ -name "package.yaml.${PROJECT_NAME}*")"
	for YAML_FILE_ITEM in ${SOLUTION_YAML_LIST[@]};
	do
		local YAML_FILE=${YAML_FILE_ITEM: 2}
		local PACKYAML_DEPEND_LINENUM=$(grep -rn "depends:" ${YAML_FILE} | grep -v "#" | awk -F: '{ print $1 }')
		local DEPNED_LINENUM=${PACKYAML_DEPEND_LINENUM}
		sed -i "${DEPNED_LINENUM}a\  - cvi_mmf_sdk: develop" ${YAML_FILE}
	done
	popd
}

build_Makefile()
{
	echo "run ${FUNCNAME[0]}() function. "
	local SDK_RELEASE_SOLUTIONS_PATH=${SDK_RELEASE_PATH}/solutions/${SOLUTION_NAME}
	pushd ${SDK_RELEASE_SOLUTIONS_PATH}
	sed -i '/^all:/,/^endif/{/ifeq (\$(MEDIA_SUPPORT_PROC),y)/,/endif/d;}' Makefile
	sed -i '/^clean:/,/^endif/{/ifeq (\$(MEDIA_SUPPORT_PROC),y)/,/endif/d;}' Makefile
	sed -i '/^all:/a endif' Makefile
	sed -i '/^all:/a \	ln -sf ../../components/cvi_mmf_sdk/package_fastboot.yaml ../../components/cvi_mmf_sdk/package.yaml' Makefile
	sed -i '/^all:/a else' Makefile
	sed -i '/^all:/a \	ln -sf ../../components/cvi_mmf_sdk/package_normal.yaml ../../components/cvi_mmf_sdk/package.yaml' Makefile
	sed -i '/^all:/a ifeq ($(ALIOS_SOLUTION),normboot)' Makefile
	popd
}

release_solutions()
{
	echo "run ${FUNCNAME[0]}() function. "
	local SDK_RELEASE_SOLUTIONS_PATH=${SDK_RELEASE_PATH}/solutions/${SOLUTION_NAME}
    local SDK_RELEASE_CVIMMFSDK_PATH=${SDK_RELEASE_PATH}/components/cvi_mmf_sdk
	#删除除了指定solution的其他文件夹
	pushd ${SDK_RELEASE_PATH}/solutions
	ls ./ | grep -v ${SOLUTION_NAME} | xargs -i rm -rf ./{}
	popd
	pushd ${SDK_RELEASE_CVIMMFSDK_PATH}
	#添加include的依赖
	for includeName in ${DEPEND_SOLUTIONS_INCLUDE[@]};
	do
		local includeNum=$(grep -rn "include" package.yaml | grep -v "#\|\/\|internal_include" | awk -F: '{ print $1 }')
		sed -i "${includeNum}a\    - ${includeName}" package.yaml
	done
	#添加cvi_mmf_sdk middleware自动匹配的include文件内容
	for includeName in ${MMF_INCLUDE_ARRAY[@]};
	do
		local includeNum=$(grep -rn "include" package.yaml | grep -v "#\|\/\|internal_include" | awk -F: '{ print $1 }')
		sed -i "${includeNum}a\    - ${includeName}" package.yaml
	done
	#添加cvi_tpu 自动匹配include文件内容
	for includeName in ${TPU_INCLUDE_ARRAY[@]};
	do
		local includeNum=$(grep -rn "include" package.yaml | grep -v "#\|\/\|internal_include" | awk -F: '{ print $1 }')
		sed -i "${includeNum}a\    - ${includeName}" package.yaml
	done
	#添加commponent需要匹配的include文件内容
	for includeName in ${DELETE_COMPONENTS_DIRS[@]};
	do
		local includeNum=$(grep -rn "include" package.yaml | grep -v "#\|\/\|internal_include" | awk -F: '{ print $1 }')
		sed -i "${includeNum}a\    - include/${includeName}" package.yaml
	done
	popd
	pushd ${SDK_RELEASE_SOLUTIONS_PATH}
	local SOLUTION_YAML_LIST="$(find ./ -name "package.yaml.${PROJECT_NAME}*")"
	for YAML_FILE_ITEM in ${SOLUTION_YAML_LIST[@]};
	do
		local YAML_FILE=${YAML_FILE_ITEM: 2}
		#自动匹配文件夹目录内容和pack.yaml的depend内容
		local PACKYAML_DEPEND_LINENUM=$(grep -rn "depends:" ${YAML_FILE} | grep -v "#" | awk -F: '{ print $1 }')
		local PACKYAML_BUILDCONFIG_LINENUM=$(grep -rn "build_config:" ${YAML_FILE} | grep -v "#" | awk -F: '{ print $1 }')
		local DEPNED_LINENUM=${PACKYAML_DEPEND_LINENUM}
		local BUILDCONFIG_LINENUM=${PACKYAML_BUILDCONFIG_LINENUM}
		local DEPEND_NAME_ARRAY=$(sed -n "${DEPNED_LINENUM},${BUILDCONFIG_LINENUM}{/cvi_*/p}" ${YAML_FILE} | grep -v "#" | awk '{printf $2}' | sed 's/:/ /g')
		#添加指定的libs依赖
		for libs_name in ${DEPEND_SOLUTIONS_LIBS[@]};
		do
			#删除掉原来的依赖
			sed -i "/${libs_name}/{/:/d}" ${YAML_FILE}
			pushd ${SDK_RELEASE_CVIMMFSDK_PATH}
			local libsExist=$(grep -rwn "\- ${libs_name}" package.yaml | grep -v "#")
			if [ -z "${libsExist}" ];
			then
				echo "[release debug]: ${libs_name} isnot exist"

				local libsNum=$(grep -rn "libs" package.yaml | grep -v "#" | awk -F: '{ print $1 }')
				echo "DEPEND_SOLUTIONS_LIBS libsNum is ${libsNum}"
				#追加.a
				sed -i "${libsNum}a\    - ${libs_name}" package.yaml
			fi
			echo "[release debug]: ${libsExist}"
			popd
			cp -f ./yoc_sdk/lib/lib${libs_name}.a ${SDK_RELEASE_CVIMMFSDK_PATH}/lib
		done
		#添加pack.yaml中与cvi_middleware目录相互匹配的name 插入到${YAML_FILE}的libs中并且删除对应依赖
		for depend_name in ${DEPEND_NAME_ARRAY[@]};
		do
			for mmf_dirname in ${MMF_DEPNED_ARRAY[@]};
			do
				if [ "$depend_name" = "$mmf_dirname" ] ; then
					sed -i "/${mmf_dirname}/{/:/d}" ${YAML_FILE}
					pushd ${SDK_RELEASE_CVIMMFSDK_PATH}
					local libsExist=$(grep -rwn "\- ${mmf_dirname}" package.yaml | grep -v "#")
					if [ -z "${libsExist}" ];
					then
						echo "[release debug]: ${mmf_dirname} isnot exist"

						local libsNum=$(grep -rn "libs" package.yaml | grep -v "#" | awk -F: '{ print $1 }')
						echo "mmf_dirname libsNum is ${libsNum}"
						sed -i "${libsNum}a\    - ${mmf_dirname}" package.yaml
					fi
					echo "[release debug]: ${libsExist}"
					popd
					cp -f ./yoc_sdk/lib/lib${mmf_dirname}.a ${SDK_RELEASE_CVIMMFSDK_PATH}/lib
				fi
			done
		done
		for depend_name in ${DEPEND_NAME_ARRAY[@]};
		do
			for dirname in ${TPU_DEPNED_ARRAY[@]};
			do
				if [ "$depend_name" = "$dirname" ] ; then
					sed -i "/${dirname}/{/:/d}" ${YAML_FILE}
					pushd ${SDK_RELEASE_CVIMMFSDK_PATH}
					local libsExist=$(grep -rwn "\- ${dirname}" package.yaml | grep -v "#")
					if [ -z "${libsExist}" ];
					then
						echo "[release debug]: ${dirname} isnot exist"
						local libsNum=$(grep -rn "libs" package.yaml | grep -v "#" | awk -F: '{ print $1 }')
						echo "TPU libsNum is ${libsNum}"
						sed -i "${libsNum}a\    - ${dirname}" package.yaml
					fi
					echo "[release debug]: ${libsExist}"
					popd
					cp -f ./yoc_sdk/lib/lib${dirname}.a ${SDK_RELEASE_CVIMMFSDK_PATH}/lib
				fi
			done
		done
	done

	rm -rf ./yoc_sdk
	cp -rf ./cvi_mmf_sdk/include/* ${SDK_RELEASE_CVIMMFSDK_PATH}/include/
	rm -rf ./cvi_mmf_sdk
	find ./ -name "package.yaml.*" | grep -v "package.yaml.${PROJECT_NAME}*" | xargs rm -f
	ls ./customization/ | grep -wv "${CUSTOMIZATION_DIR1}\|${CUSTOMIZATION_DIR2}\|${CUSTOMIZATION_DIR3}\|${CUSTOMIZATION_DIR4}\|include" | xargs -i rm -rf ./customization/{}
	rm -f prebuild.sh
	popd
}

pack_release_sdk()
{
	echo "run ${FUNCNAME[0]}() function. "
	local SDK_RELEASE_CVIMMFSDK_PATH=${SDK_RELEASE_PATH}/components/cvi_mmf_sdk
	pushd ${SDK_RELEASE_CVIMMFSDK_PATH}
	if [[ "$SOLUTION_NAME" == "normboot" ]]; then
	mv package.yaml package_normal.yaml
	ln -sf package_normal.yaml package.yaml
	else
	mv package.yaml package_fastboot.yaml
	ln -sf package_fastboot.yaml package.yaml
	fi
	popd
	pushd ${TOP_DIR}
	tar -zcvf ../${BOARD_NAME}_${SOLUTION_NAME}_${PROJECT_NAME}_cvitek_sdk.tar.gz ${SDK_RELEASE_PATH}
	rm -rf ${SDK_RELEASE_PATH}
	popd
	echo "pack_release_sdk done"
}

clean_release_build()
{
	echo "run ${FUNCNAME[0]}() function. "
	pushd ${SDK_RELEASE_PATH}
	rm -rf .yoc
	echo y | yoc init
	make clean PROJECT=${PROJECT_NAME}
	rm -rf .yoc
	popd
	echo "clean_release_build end"
}

release_load_config
release_pre_pack
release_build
release_components
build_cvimmfsdk
build_Makefile
release_solutions
clean_release_build
pack_release_sdk
if [[ "$SOLUTION_NAME" == "fastboot" ]]; then
	pushd ${TOP_DIR}/..
	tar -xvf ${BOARD_NAME}_${SOLUTION_NAME}_${PROJECT_NAME}_cvitek_sdk.tar.gz
		pushd ${BOARD_NAME}_${SOLUTION_NAME}_${PROJECT_NAME}_cvitek_sdk
		echo y | yoc init
		make clean PROJECT=${PROJECT_NAME}
		popd
	mv ${BOARD_NAME}_${SOLUTION_NAME}_${PROJECT_NAME}_cvitek_sdk cvi_alios_source
else
	pushd ${TOP_DIR}/..
	tar -xvf ${BOARD_NAME}_${SOLUTION_NAME}_${PROJECT_NAME}_cvitek_sdk.tar.gz
		pushd ${BOARD_NAME}_${SOLUTION_NAME}_${PROJECT_NAME}_cvitek_sdk
		echo y | yoc init
		make clean PROJECT=${PROJECT_NAME};make ${SOLUTION_NAME} PROJECT=${PROJECT_NAME}
		if [ $? != 0 ]; then
			popd
			rm -rf ${BOARD_NAME}_${SOLUTION_NAME}_${PROJECT_NAME}_cvitek_sdk
			echo "release sdk build err"
			exit 1
		fi
		make clean PROJECT=${PROJECT_NAME}
		popd
	mv ${BOARD_NAME}_${SOLUTION_NAME}_${PROJECT_NAME}_cvitek_sdk cvi_alios_source
fi
echo "release sdk build success"
popd

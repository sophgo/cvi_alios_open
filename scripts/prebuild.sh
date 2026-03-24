#!/bin/bash
# isp generate pqtool_definition.json
# ----------- TODO: alios not support ispd2 yet
#ISP_EXECSH_DIR=../../components/cvi_mmf_sdk/cvi_middleware/cvi_mw_isp_common/toolJsonGenerator
#ISP_DAEMON_DIR=../../components/cvi_mmf_sdk/cvi_middleware/cvi_mw_isp_daemon/src
#ISP_PQTOOLJSON_FILE=${ISP_EXECSH_DIR}/pqtool_definition.json
#OUTPUT=pqtool_definition.json
#
#pushd ${ISP_EXECSH_DIR}
#if [ ! -f ${TOP_DIR}/cvi_mpi/include/cvi_comm.h ]; then
#	rm -rf ${TOP_DIR}/cvi_mpi/include
#	cp -r ${TOP_DIR}/build/media/include/release ${TOP_DIR}/cvi_mpi/include
#fi
#./generate_toolJson.sh cv184x
#if [ ! "$?" -eq 0 ]; then
#	echo "Error: generate_toolJson fail! Exit..."
#	exit 1
#fi
#popd
#
#cp -f ${ISP_PQTOOLJSON_FILE} ${ISP_DAEMON_DIR}
#cd ${ISP_DAEMON_DIR}
#xxd -i ${OUTPUT} > cvi_pqtool_json.h
#
#if [ -f ${OUTPUT} ]
#then
#	rm ${OUTPUT}
#fi

# cvi_bin run auto gen code
CVI_BIN_PATH=${TOP_DIR}/cvi_mpi/modules/cvi_bin
HEAD_I_PATH=${CVI_BIN_PATH}/tmp/cvi_bin_param_header.i
HEAD_D_PATH=${CVI_BIN_PATH}/tmp/cvi_bin_param_header.d

pushd $CVI_BIN_PATH
if [ ! -d tmp ]; then
	mkdir -p tmp/
fi

MPI_INC=${TOP_DIR}/cvi_mpi/include
ISP_INC=${TOP_DIR}/cvi_mpi/include/isp
VPSS_INC=${TOP_DIR}/cvi_mpi/modules/vpss/include

if [ ! -f ${ISP_INC}/cvi_comm_isp.h ]; then
	ISP_INC=${TOP_DIR}/cvi_mpi/modules/isp/include/cv184x
fi

if [ ! -f ${MPI_INC}/cvi_bin.h ]; then
	MPI_INC=${TOP_DIR}/build/media/include/release
fi

${CROSS_COMPILE}gcc -E -MMD \
	-I${MPI_INC} \
	-I${ISP_INC} \
	-I${VPSS_INC} \
	-fpack-struct=8 \
	-DINDEX_STRATEGY -c ${CVI_BIN_PATH}/include/cvi_bin_param_header.h \
	-o ${HEAD_I_PATH}

sed -i "1s|.*|${HEAD_I_PATH}: \\\\|" "${HEAD_D_PATH}"

echo "begin generate cvi_bin autogen code..."
if [ -f ../isp/common/toolJsonGenerator/cv184x/cvi_bin_profile.json ]; then
	python3 python/gencode.py ${HEAD_I_PATH} --profile ../isp/common/toolJsonGenerator/cv184x/cvi_bin_profile.json
	if [ ! "$?" -eq 0 ]; then
		echo "Error: generate cvi_bin fail! Exit..."
		exit 1
	fi
	cp -f ../isp/common/toolJsonGenerator/cv184x/cvi_bin_profile.json python/cvi_bin_profile.json
else
	python3 python/gencode.py ${HEAD_I_PATH} --profile python/cvi_bin_profile.json
	if [ ! "$?" -eq 0 ]; then
		echo "Error: generate cvi_bin fail! Exit..."
		exit 1
	fi
fi
echo "end generate cvi_bin autogen code..."

popd

#!/bin/bash
#ISP预build脚本
ISP_EXECSH_DIR=../../components/cvi_mmf_sdk/cvi_middleware/cvi_mw_isp_common/toolJsonGenerator
ISP_DAEMON_DIR=../../components/cvi_mmf_sdk/cvi_middleware/cvi_mw_isp_daemon/src
ISP_BIN_EXECSH_DIR=../../components/cvi_mmf_sdk/cvi_middleware/cvi_mw_isp_bin
ISP_PQTOOLJSON_FILE=${ISP_EXECSH_DIR}/pqtool_definition.json
OUTPUT=pqtool_definition.json

pushd ${ISP_EXECSH_DIR}
source generate_toolJson.sh cv181x
popd
pushd ${ISP_BIN_EXECSH_DIR}
source CalPqBinMD5.sh
popd

cp -f ${ISP_PQTOOLJSON_FILE} ${ISP_DAEMON_DIR}
cd ${ISP_DAEMON_DIR}
xxd -i ${OUTPUT} > cvi_pqtool_json.h

if [ -f ${OUTPUT} ]
then
	rm ${OUTPUT}
fi

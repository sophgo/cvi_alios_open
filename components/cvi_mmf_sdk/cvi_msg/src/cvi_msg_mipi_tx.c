#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "cvi_type.h"
#include "cvi_sys.h"
#include "cvi_ipcmsg.h"
#include "msg_mipi_tx.h"
#include "msg_ctx.h"
#include "cvi_debug.h"
#include "cvi_comm_mipi_tx.h"
#include "driver_mipi_tx.h"

#define MIPI_TX_CHECK_RET(ret, fmt, arg...)         \
	do {                                            \
		if ((ret) != 0) {                           \
			CVI_TRACE_MSG(CVI_DBG_ERR, "[%d]:%s():ret=%d " fmt,     \
			__LINE__, __func__, ret, ## arg);       \
			return -1;                              \
		}                                           \
	} while (0)

#define MIPI_TX_CHECK_RET_NULL(ret, fmt, arg...)    \
	do {                                            \
		if ((ret) != 0) {                           \
			CVI_TRACE_MSG(CVI_DBG_ERR, "[%d]:%s():ret=%d " fmt,     \
			__LINE__, __func__, ret, ## arg);       \
			return NULL;                              \
		}                                           \
	} while (0)

static void _cal_htt_extra(struct combo_dev_cfg_s *dev_cfg)
{
	unsigned char lane_num = 0, bits = 0;
	unsigned short htt_old, htt_new, htt_new_extra;
	unsigned short vtt;
	float fps;
	float bit_rate_MHz;
	float clk_hs_MHz;
	float clk_hs_ns;
	float line_rate_KHz, line_time_us;
	float over_head;
	float t_period_max, t_period_real;
	struct sync_info_s *sync_info = &dev_cfg->sync_info;

	for (int i = 0; i < LANE_MAX_NUM; i++) {
		if ((dev_cfg->lane_id[i] < 0) || (dev_cfg->lane_id[i] >= MIPI_TX_LANE_MAX))
			continue;
		if (dev_cfg->lane_id[i] != MIPI_TX_LANE_CLK)
			++lane_num;
	}

	switch (dev_cfg->output_format) {
	case OUT_FORMAT_RGB_16_BIT:
		bits = 16;
		break;

	case OUT_FORMAT_RGB_18_BIT:
		bits = 18;
		break;

	case OUT_FORMAT_RGB_24_BIT:
		bits = 24;
		break;

	case OUT_FORMAT_RGB_30_BIT:
		bits = 30;
		break;

	default:
		break;
	}

	htt_old = sync_info->vid_hsa_pixels + sync_info->vid_hbp_pixels
			+ sync_info->vid_hline_pixels + sync_info->vid_hfp_pixels;
	vtt = sync_info->vid_vsa_lines + sync_info->vid_vbp_lines
			+ sync_info->vid_active_lines + sync_info->vid_vfp_lines;
	fps = dev_cfg->pixel_clk * 1000.0 / (htt_old * vtt);
	bit_rate_MHz = dev_cfg->pixel_clk / 1000.0 * bits / lane_num;
	clk_hs_MHz = bit_rate_MHz / 2;
	clk_hs_ns = 1000 / clk_hs_MHz;
	line_rate_KHz = vtt * fps / 1000;
	line_time_us = 1000 / line_rate_KHz;
	over_head = (3 * 50 * 2 * 3) + clk_hs_ns * 360;
	t_period_max = line_time_us * 1000 - over_head;
	t_period_real = clk_hs_ns * sync_info->vid_hline_pixels * bits / 4 / 2;
	htt_new = (unsigned short)(htt_old * t_period_real / t_period_max);
	if (htt_new > htt_old) {
		if (htt_new & 0x0003)
			htt_new += (4 - (htt_new & 0x0003));
		htt_new_extra = htt_new - htt_old;
		sync_info->vid_hfp_pixels += htt_new_extra;
		dev_cfg->pixel_clk = htt_new * vtt * fps / 1000;
	}
}

int MSG_mipi_tx_cfg(CVI_S32 s32Id, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	struct combo_dev_cfg_s *dev_cfg = (struct combo_dev_cfg_s *)pstMsg->pBody;
	struct combo_dev_cfg_s dev_cfg_t = *dev_cfg;

	_cal_htt_extra(&dev_cfg_t);

	MIPI_TX_CHECK_RET(s32Ret = driver_mipi_tx_ioctl(MIPI_TX_SET_DEV_CFG, (unsigned long)&dev_cfg_t), "MIPI_TX_SET_DEV_CFG failed!\n");

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "RGN_Create call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(s32Id, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "RGN_Create call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return s32Ret;
}

int MSG_mipi_tx_send_cmd(CVI_S32 s32Id, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	struct cmd_info_msg_s cmd_info_msg = *(struct cmd_info_msg_s *)pstMsg->pBody;
	struct cmd_info_s cmd_info;

	cmd_info.devno = cmd_info_msg.devno;
	cmd_info.data_type = cmd_info_msg.data_type;
	cmd_info.cmd_size = cmd_info_msg.cmd_size;
	cmd_info.cmd = cmd_info_msg.cmd;

	if (cmd_info.cmd_size == 0)
		return -1;

	MIPI_TX_CHECK_RET(s32Ret = driver_mipi_tx_ioctl(MIPI_TX_SET_CMD, (unsigned long)&cmd_info), "MIPI_TX_SET_CMD failed!\n");

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "RGN_Create call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(s32Id, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "RGN_Create call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return s32Ret;
}

int MSG_mipi_tx_recv_cmd(CVI_S32 s32Id, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	struct get_cmd_info_msg_s get_cmd_info_msg = *(struct get_cmd_info_msg_s *)pstMsg->pBody;
	struct get_cmd_info_s cmd_info;

	cmd_info.devno = get_cmd_info_msg.devno;
	cmd_info.data_type = get_cmd_info_msg.data_type;
	cmd_info.data_param = get_cmd_info_msg.data_param;
	cmd_info.get_data_size = get_cmd_info_msg.get_data_size;
	cmd_info.get_data = get_cmd_info_msg.get_data;

	if (cmd_info.get_data_size == 0)
		return -1;

	MIPI_TX_CHECK_RET(s32Ret = driver_mipi_tx_ioctl(MIPI_TX_GET_CMD, (unsigned long)&cmd_info), "MIPI_TX_GET_CMD failed!\n");

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &get_cmd_info_msg, sizeof(struct get_cmd_info_msg_s));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "RGN_Create call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(s32Id, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "RGN_Create call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return s32Ret;
}

int MSG_mipi_tx_enable(CVI_S32 s32Id, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	MIPI_TX_CHECK_RET(s32Ret = driver_mipi_tx_ioctl(MIPI_TX_ENABLE, 0), "MIPI_TX_ENABLE failed!\n");

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "RGN_Create call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(s32Id, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "RGN_Create call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return s32Ret;
}

int MSG_mipi_tx_disable(CVI_S32 s32Id, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;

	MIPI_TX_CHECK_RET(s32Ret = driver_mipi_tx_ioctl(MIPI_TX_DISABLE, 0), "MIPI_TX_DISABLE failed!\n");

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "RGN_Create call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(s32Id, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "RGN_Create call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return s32Ret;
}

int MSG_mipi_tx_set_hs_settle(CVI_S32 s32Id, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	struct hs_settle_s *hs_cfg = (struct hs_settle_s *)pstMsg->pBody;

	MIPI_TX_CHECK_RET(s32Ret = driver_mipi_tx_ioctl(MIPI_TX_SET_HS_SETTLE, (unsigned long)hs_cfg), "MIPI_TX_SET_HS_SETTLE failed!\n");

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, NULL, 0);
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "RGN_Create call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(s32Id, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "RGN_Create call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return s32Ret;
}

int MSG_mipi_tx_get_hs_settle(CVI_S32 s32Id, CVI_IPCMSG_MESSAGE_S *pstMsg)
{
	CVI_S32 s32Ret;
	CVI_IPCMSG_MESSAGE_S *respMsg = CVI_NULL;
	struct hs_settle_s hs_cfg;

	MIPI_TX_CHECK_RET(s32Ret = driver_mipi_tx_ioctl(MIPI_TX_GET_HS_SETTLE, (unsigned long)&hs_cfg), "MIPI_TX_GET_HS_SETTLE failed!\n");

	respMsg = CVI_IPCMSG_CreateRespMessage(pstMsg, s32Ret, &hs_cfg, sizeof(struct hs_settle_s));
	if (respMsg == CVI_NULL) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "RGN_Create call CVI_IPCMSG_CreateRespMessage fail\n");
	}

	s32Ret = CVI_IPCMSG_SendOnly(s32Id, respMsg);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_MSG(CVI_DBG_ERR, "RGN_Create call CVI_IPCMSG_SendOnly fail,ret:%x\n", s32Ret);
	}

	CVI_IPCMSG_DestroyMessage(respMsg);

	return s32Ret;
}

static MSG_MODULE_CMD_S g_stMipitxCmdTable[] = {
	{ MSG_CMD_MIPI_TX_SET_DEV_CFG,              MSG_mipi_tx_cfg },
	{ MSG_CMD_MIPI_TX_SET_CMD,                  MSG_mipi_tx_send_cmd },
	{ MSG_CMD_MIPI_TX_GET_CMD,                  MSG_mipi_tx_recv_cmd },
	{ MSG_CMD_MIPI_TX_ENABLE,                   MSG_mipi_tx_enable },
	{ MSG_CMD_MIPI_TX_DISABLE,                  MSG_mipi_tx_disable },
	{ MSG_CMD_MIPI_TX_SET_HS_SETTLE,            MSG_mipi_tx_set_hs_settle },
	{ MSG_CMD_MIPI_TX_GET_HS_SETTLE,            MSG_mipi_tx_get_hs_settle },
};

MSG_SERVER_MODULE_S g_stModuleMipitx = {
	CVI_ID_MIPI_TX,
	"mipi_tx",
	sizeof(g_stMipitxCmdTable) / sizeof(MSG_MODULE_CMD_S),
	&g_stMipitxCmdTable[0]
};

MSG_SERVER_MODULE_S *MSG_GetMipitxMod(CVI_VOID)
{
	return &g_stModuleMipitx;
}

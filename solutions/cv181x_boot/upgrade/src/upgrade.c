

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <drv/tick.h>
#include "drv/spiflash.h"

#include "usb_cdc_acm.h"
#include "crc_check.h"
#include "md5.h"

#define LOG(fmt, args...)                                        	\
	do {                                                       		\
		printf("[%s:%d]" fmt , __FUNCTION__, __LINE__, ##args);  	\
	} while(0)

#define ACTION_DONE		0
#define ACTION_WAIT		1
#define ACTION_FAILED	-1

#define MAX_PACKET_SIZE	(4096)
#define MAX_BIN_SIZE	(16 * 1024 * 1024 + MAX_PACKET_SIZE * 8)
#define MAX_PACKET_CNT	(MAX_BIN_SIZE / MAX_PACKET_SIZE)
#define MAX_BITMAP_SIZE	(MAX_PACKET_CNT >> 3)
#define FLASH_OFF_ENV 0x00ffd000
#define FLASH_LEN_ENV 0x1000

#define PROTOCOL_LEN	(8)

#define CVI_UPGRADE_HEAD_OFFSET			(128)
#define CVI_UPGRADE_CHUNK_HEAD_OFFSET	(64)
#define UPGRADE_MAGIC_NUM			(0xA5A5A5A5)
#define UPGRADE_MAX_BIN_NUM			(14)
#define UPGRADE_HEAD_OFFSET			(64)


#define CMD_TIMEOUT_MS	(20 * 1000)

#define FORCE_UPGRADE	0
#define CHECK_MD5		1

typedef enum {
	STATE_INIT = 1,			// 初始状态
  	STATE_UPGRADE_READY,	// 升级准备状态
  	STATE_UPGRADE_START,	// 升级启动状态
	STATE_GET_MD5,
	STATE_DATA_TRANS,		// 数据传输状态
	STATE_DATA_TRANS_DONE,	// 数据传输完成状态
	STATE_DATA_UPDATE,		// 数据更新状态
	STATE_DATA_UPDATE_DONE, // 数据更新完成状态

	STATE_BUTT,
} STATE;

typedef enum {
	EVENT_GET_VERSION = 1,	// 获取版本事件
  	EVENT_ENTER_UPGRADE,	// 进入升级事件
  	EVENT_START_TRANS,		// 启动传输事件
	EVENT_GET_MD5,
	EVENT_DATA_TRANS,		// 数据传输事件
	EVENT_DATA_UPDATE,		// 数据更新事件
	EVENT_EXIT_UPGRADE,		// 退出升级事件
	EVENT_UPLOAD_VERSION,	// 上传版本事件

	EVENT_BUTT,
} EVENT;

struct state_transform {
	STATE cur_state;
	EVENT cur_event;
	STATE next_state;
	int (*action)(void *args);
};

struct state_machine {
	STATE state;
	int trans_num;
	struct state_transform* transform;
};

struct cvi_upgrade_chunk_header {
	uint32_t  chunk_type;
	uint32_t  chunk_data_size;
	uint32_t  part_offset;
	uint32_t  part_size;
	uint32_t  crc_checksum;
	uint32_t  reserved[12];
};

struct upgrade_chunk_header {
	uint32_t  magic_num;
	uint32_t  bin_num;
	uint32_t  bin_size[UPGRADE_MAX_BIN_NUM];
};

struct packet_param {
	uint8_t version;
	uint8_t crc_sum;
	uint16_t packet_num;
	uint16_t packet_seq;
	uint16_t packet_len;
};

struct upgrade_ctx {
	uint8_t run_flag;
	uint8_t local_version;
	uint8_t handle_data_trans;
	uint8_t handle_data_update_done;
	uint8_t *bin_bitmap;
	uint8_t **bin_data;
	uint16_t **bin_data_len;
	uint32_t recv_packet_num;
	uint32_t recv_packet_len;
	csi_spiflash_info_t spiflash_info;
	csi_spiflash_t spiflash_handle;
	uint8_t *env_buffer;
	uint8_t packet_md5_sum[MD5_DIGEST_LENGTH];
	MD5_CTX md5_ctx;
	uint8_t upgrade_status; // 0x0: done, 0x1: failed, 0x2: timeout
};

static struct upgrade_ctx s_ctx;

static int get_upgrade_flag(void)
{
#if FORCE_UPGRADE
	return 0;
#endif
	struct upgrade_ctx *ctx = &s_ctx;

	if (!ctx->env_buffer) {
		return -1;
	}
	if (ctx->env_buffer[1] == 1) {
		return 0;
	}
	return -1;
}

// static void clear_upgrade_flag(void)
// {
// 	struct upgrade_ctx *ctx = &s_ctx;

// 	if (!ctx->env_buffer) {
// 		return -1;
// 	}

// 	ctx->env_buffer[1] = 0;
// 	csi_spiflash_erase(&ctx->spiflash_handle, FLASH_OFF_ENV, ctx->spiflash_info.page_size);
// 	csi_spiflash_program(&ctx->spiflash_handle, FLASH_OFF_ENV, ctx->env_buffer, ctx->spiflash_info.page_size);
// }

static int ctx_init(void)
{
	int ret = 0;
	struct upgrade_ctx *ctx = &s_ctx;

	// init flash
	ret = csi_spiflash_spi_init(&ctx->spiflash_handle, 0, NULL);
	if (ret != 0) {
		LOG("csi_spiflash_spi_init failed\n");
		return -1;
	}
	ret = csi_spiflash_get_flash_info(&ctx->spiflash_handle, &ctx->spiflash_info);
	if (ret != 0) {
		LOG("csi_spiflash_get_flash_info failed\n");
		return -1;
	}
	// read env data
	ctx->env_buffer = (uint8_t *)malloc(ctx->spiflash_info.page_size);
	if (!ctx->env_buffer) {
        LOG("Failed to allocate memory for size %u\n", ctx->spiflash_info.page_size);
		return -1;
	}
	memset(ctx->env_buffer, 0, ctx->spiflash_info.page_size);

	ret = csi_spiflash_read(&ctx->spiflash_handle, FLASH_OFF_ENV, ctx->env_buffer, ctx->spiflash_info.page_size);
	if (ret < 0) {
		LOG("csi_spiflash_read failed with %d\n", ret);
		return -1;
	}

	if (get_upgrade_flag() != 0) {
		LOG("skip upgrade\n");
		return -1;
	}
	LOG("flash_size:%u, page_size:%u, sector_size:%u\n", ctx->spiflash_info.flash_size, ctx->spiflash_info.page_size,
		ctx->spiflash_info.sector_size);

	// init md5 ctx
	MD5_Init(&ctx->md5_ctx);

	// alloc memory for upgrade file
	ctx->bin_data = (uint8_t **)malloc(MAX_PACKET_CNT * sizeof(uint8_t *));
    if (ctx->bin_data == NULL) {
        LOG("Failed to allocate memory for size %lu\n", MAX_PACKET_CNT * sizeof(uint8_t *));
        return -1;
    }
	uint64_t bin_data_size = MAX_PACKET_CNT * MAX_PACKET_SIZE * sizeof(uint8_t);
	ctx->bin_data[0] = (uint8_t *)malloc(bin_data_size);
	if (ctx->bin_data[0] == NULL) {
        LOG("Failed to allocate memory for size %lu\n", bin_data_size);
        return -1;
    }
	memset(ctx->bin_data[0], 0, bin_data_size);

#if 0
	// length of each chunk
	ctx->bin_data_len = (uint16_t **)malloc(MAX_PACKET_CNT * sizeof(uint16_t*));
	if (ctx->bin_data_len == NULL) {
        LOG("Failed to allocate memory for size %lu\n", MAX_PACKET_CNT * sizeof(uint16_t *));
        return -1;
    }
	uint64_t bin_data_len_size = MAX_PACKET_CNT * sizeof(uint16_t);
	ctx->bin_data_len[0] = (uint16_t *)malloc(bin_data_len_size);
	if (ctx->bin_data_len[0] == NULL) {
        LOG("Failed to allocate memory for size %lu\n", bin_data_len_size);
        return -1;
    }
	memset(ctx->bin_data_len[0], 0, bin_data_len_size);
#endif
	// init chunk
    for (int i = 1; i < MAX_PACKET_CNT; i++) {
        ctx->bin_data[i] = (uint8_t *)(ctx->bin_data[0] + i * MAX_PACKET_SIZE * sizeof(uint8_t));
		// ctx->bin_data_len[i] = (uint16_t *)(ctx->bin_data_len[0] + i * sizeof(uint16_t));
    }

	// chunk bitmap
	ctx->bin_bitmap = (uint8_t *)malloc(MAX_BITMAP_SIZE * sizeof(uint8_t));
	if (ctx->bin_bitmap == NULL) {
		LOG("Failed to allocate memory for size %lu\n", MAX_BITMAP_SIZE * sizeof(uint8_t));
		return -1;
    }
	memset(ctx->bin_bitmap, 0, MAX_BITMAP_SIZE * sizeof(uint8_t));

	ctx->run_flag = 1;
	ctx->local_version = ctx->env_buffer[0];
	ctx->upgrade_status = 1;

	LOG("MAX_PACKET_SIZE:%d, local_version:%d\n", MAX_PACKET_SIZE, ctx->local_version);

	return 0;
}

static void ctx_reset(void)
{
	struct upgrade_ctx *ctx = &s_ctx;

	ctx->recv_packet_num = 0;
	ctx->handle_data_trans = 0;
	ctx->handle_data_update_done = 0;
	ctx->upgrade_status = 1;
	if (ctx->bin_bitmap) {
		memset(ctx->bin_bitmap, 0, MAX_BITMAP_SIZE * sizeof(uint8_t));
	}
#if 0
	ctx->recv_packet_len = 0;
	if (ctx->bin_data_len && ctx->bin_data_len[0]) {
		memset(ctx->bin_data_len[0], 0, MAX_PACKET_CNT * sizeof(uint16_t));
	}
#endif
	MD5_Init(&ctx->md5_ctx);
	memset(ctx->packet_md5_sum, 0, MD5_DIGEST_LENGTH);
}

static int check_md5_sum(const uint8_t *src, uint32_t len, uint8_t *dst)
{
	for (uint8_t i = 0; i < MD5_DIGEST_LENGTH; i++) {
		if (src[i] != dst[i]) {
			LOG("md5 check failed\n");
			LOG("src:");
			for (uint8_t i = 0; i < MD5_DIGEST_LENGTH; i++) {
				printf("%#x ", src[i]);
			}
			printf("\n");
			LOG("dst:");
			for (uint8_t i = 0; i < MD5_DIGEST_LENGTH; i++) {
				printf("%#x ", dst[i]);
			}
			printf("\n");
			return -1;
		}
	}

	return 0;
}

static int action_get_version(void *args)
{
	struct upgrade_ctx *ctx = &s_ctx;
	uint8_t ret_msg[PROTOCOL_LEN] = {0xAA, 0x55, 0xF8, 0x00, ctx->local_version, 0x00, 0x00, 0x00};

	if (cdc_acm_write(ret_msg, PROTOCOL_LEN) != 0) {
		return ACTION_FAILED;
	}
	return ACTION_DONE;
}

static int action_enter_upgrade(void *args)
{
	uint8_t ret_msg[PROTOCOL_LEN] = {0xAA, 0x55, 0xF2, 0x00, 0x00, 0x00, 0x00, 0x00};

	if (cdc_acm_write(ret_msg, PROTOCOL_LEN) != 0) {
		return ACTION_FAILED;
	}
	ctx_reset();
	return ACTION_DONE;
}

static int action_get_md5(void *args)
{
	int ret = 0;
	struct upgrade_ctx *ctx = &s_ctx;
	uint8_t md5_ret_msg[PROTOCOL_LEN] = {0xAA, 0x55, 0xEB, 0x00, 0x00, 0x00, 0x00, 0x00};

	// recv md5
	ret = cdc_acm_read(ctx->packet_md5_sum, MD5_DIGEST_LENGTH, 1000);
	if (ret != MD5_DIGEST_LENGTH) {
		LOG("read failed, %d, %d\n", ret, MD5_DIGEST_LENGTH);
		md5_ret_msg[3] = 0x1;
		cdc_acm_write(md5_ret_msg, PROTOCOL_LEN);
		return ACTION_FAILED;
	}

	if (cdc_acm_write(md5_ret_msg, PROTOCOL_LEN) != 0) {
		return ACTION_FAILED;
	}
	printf("recv md5: ");
	for (uint8_t i = 0; i < MD5_DIGEST_LENGTH; i++) {
		printf("%#x ", ctx->packet_md5_sum[i]);
	}
	printf("\n");

	return ACTION_DONE;
}

static int action_start_trans(void *args)
{
	uint8_t ret_msg[PROTOCOL_LEN] = {0xAA, 0x55, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00};

	if (cdc_acm_write(ret_msg, PROTOCOL_LEN) != 0) {
		return ACTION_FAILED;
	}

	return ACTION_DONE;
}

static int action_data_trans(void *args)
{
	int ret = 0;
	struct upgrade_ctx *ctx = &s_ctx;
	struct packet_param *pkt = (struct packet_param *)(args);
	uint8_t ret_msg[PROTOCOL_LEN] = {0xAA, 0x55, 0xF4, 0x01, 0x00, 0x00, 0x00, 0x00};
	static unsigned int cur_ms1, cur_ms2;

	// first packet
	if (pkt->packet_seq == 0) {
		cur_ms1 = csi_tick_get_ms();
		LOG("recv packet start...\n");
	}

	if (pkt->packet_seq >= MAX_PACKET_CNT || pkt->packet_len > MAX_PACKET_SIZE) {
		LOG("over range, seq:%d, len:%d\n", pkt->packet_seq, pkt->packet_len);
		goto failed;
	}

	// 1 byte can identify 8 packets
	uint16_t bank = pkt->packet_seq >> 3;
	uint16_t offset = pkt->packet_seq % 8;

	if ((ctx->bin_bitmap[bank] & (1 << offset)) == 1) {
		LOG("seq %d already set\n", pkt->packet_seq);
		goto wait;
	}

	// recv packet data
	ret = cdc_acm_read(ctx->bin_data[pkt->packet_seq], pkt->packet_len, 1000);
	if (ret != pkt->packet_len) {
		LOG("read failed, %d/%d\n", ret, pkt->packet_len);
		goto wait;
	}

	// check crc
	if (calculate_crc8(ctx->bin_data[pkt->packet_seq], pkt->packet_len) != pkt->crc_sum) {
		LOG("crc check failed\n");
		goto failed;
	}

	// update md5
	MD5_Update(&ctx->md5_ctx, ctx->bin_data[pkt->packet_seq], pkt->packet_len);

	// LOG("recv packet %d/%d\n", ctx->recv_packet_num, pkt->packet_seq);

	ret_msg[3] = 0x0;
	ret_msg[4] = pkt->packet_seq >> 8;
	ret_msg[5] = pkt->packet_seq & 0xff;
	ret_msg[6] = pkt->packet_len >> 8;
	ret_msg[7] = pkt->packet_len & 0xff;
	if (cdc_acm_write(ret_msg, PROTOCOL_LEN) != 0) {
		return ACTION_FAILED;
	}
	// recv one packet done
	ctx->bin_bitmap[bank] |= (1 << offset);
#if 0
	*(ctx->bin_data_len[pkt->packet_seq]) = pkt->packet_len;
	ctx->recv_packet_len += pkt->packet_len;
#endif
	ctx->recv_packet_num++;

	// recv all packet done
	if (ctx->recv_packet_num == pkt->packet_num) {
		cur_ms2 = csi_tick_get_ms();
		LOG("recv packet done, use time:%u ms\n", cur_ms2 - cur_ms1);

		uint8_t result[MD5_DIGEST_LENGTH];
		MD5_Final(result, &ctx->md5_ctx);
	#if CHECK_MD5
		ret = check_md5_sum(result, MD5_DIGEST_LENGTH, ctx->packet_md5_sum);
		if (ret != 0) {
			LOG("check_md5_sum failed\n");
			goto failed;
		}
		LOG("check md5 success\n");
	#endif
		return ACTION_DONE;
	}

wait:
	return ACTION_WAIT;

failed:
	ret_msg[3] = 0x1;
	cdc_acm_write(ret_msg, PROTOCOL_LEN);
	return ACTION_FAILED;
}

static int action_exit_upgrade(void *args)
{
	uint8_t ret_msg[PROTOCOL_LEN] = {0xAA, 0x55, 0xF6, 0x00, 0x00, 0x00, 0x00, 0x00};
	struct upgrade_ctx *ctx = &s_ctx;

	ctx->run_flag = 0;

	if (cdc_acm_write(ret_msg, PROTOCOL_LEN) != 0) {
		return ACTION_FAILED;
	}
	return ACTION_DONE;
}

static int cvi_do_update(uint8_t *bin_data, uint32_t bin_size)
{
	int writesize = 0;
	struct upgrade_ctx *ctx = &s_ctx;
	csi_spiflash_info_t *info = &ctx->spiflash_info;
	csi_spiflash_t *spiflash_handle = &ctx->spiflash_handle;
	struct cvi_upgrade_chunk_header *header =
		(struct cvi_upgrade_chunk_header *)(bin_data + CVI_UPGRADE_CHUNK_HEAD_OFFSET);


	uint32_t offset = header->part_offset;
	uint32_t size = header->part_size;

	LOG("offset:%#x, size:%#x \n", offset, size);

	if (size == 0) {
		goto EXIT;
	}
	if (size & (info->sector_size - 1))
		size = (size + (info->sector_size - 1)) &~((info->sector_size - 1));

	if ((offset + size) > info->flash_size) {
		LOG("the range is greater flash size:%#x \n", info->flash_size);
		goto EXIT;
	}

    unsigned int cur_ms1, cur_ms2;
    cur_ms1 = csi_tick_get_ms();
	csi_spiflash_erase(spiflash_handle, offset, size);
	cur_ms2 = csi_tick_get_ms();
	LOG("flash erase size:%d, use time:%ums\n\n", size, cur_ms2 - cur_ms1);

	cur_ms1 = csi_tick_get_ms();
	writesize = csi_spiflash_program(spiflash_handle, offset, bin_data + CVI_UPGRADE_HEAD_OFFSET, bin_size - CVI_UPGRADE_HEAD_OFFSET);
	if (writesize != bin_size - CVI_UPGRADE_HEAD_OFFSET) {
		LOG("nor flash program failed!\n");
		goto EXIT;
	}
	cur_ms2 = csi_tick_get_ms();
	LOG("flash write use time:%ums\n\n", cur_ms2 - cur_ms1);

	return 0;

EXIT:
	return -1;
}

static int do_update(uint8_t *bin_data)
{
	int ret = 0;
	uint8_t *bin_data_t = bin_data + UPGRADE_HEAD_OFFSET;
	struct upgrade_chunk_header *header = (struct upgrade_chunk_header *)bin_data;

	if (UPGRADE_MAGIC_NUM != header->magic_num) {
		LOG("magic num error %#x\n", header->magic_num);
		return -1;
	}

	uint32_t bin_num = header->bin_num > UPGRADE_MAX_BIN_NUM ? UPGRADE_MAX_BIN_NUM : header->bin_num;
	uint32_t already_size = 0;
	for (uint32_t i = 0; i < bin_num; i++) {
		ret = cvi_do_update(bin_data_t + already_size, header->bin_size[i]);
		if (ret != 0) {
			LOG("cvi_do_update part %d failed\n", i);
			return -1;
		}
		already_size += header->bin_size[i];
	}

	return 0;
}

static int action_data_update(void *args)
{
	int ret = 0;
	struct upgrade_ctx *ctx = &s_ctx;
	struct packet_param *pkt = (struct packet_param *)(args);

	ret = do_update(ctx->bin_data[0]);
	if (ret != 0) {
		LOG("do_update failed\n");
		return ACTION_FAILED;
	}

	ctx->local_version = pkt->version;
	ctx->upgrade_status = 0;

	return ACTION_DONE;
}

static int action_upload_version(void *args)
{
	int ret = 0;
	struct upgrade_ctx *ctx = &s_ctx;

	uint8_t ret_msg[PROTOCOL_LEN] = {0xAA, 0x55, 0xF9, ctx->local_version, 0x00, 0x00, 0x00, 0x00};

	if (cdc_acm_write(ret_msg, PROTOCOL_LEN) != 0) {
		return ACTION_FAILED;
	}

	ret = cdc_acm_read(ret_msg, PROTOCOL_LEN, 5000);
	if (ret != PROTOCOL_LEN) {
		LOG("read failed, ret:%d\n", ret);
		ctx->handle_data_update_done = 0;
		return ACTION_FAILED;
	}

	if (ret_msg[2] != 0xFA || ret_msg[3] != 0x00) {
		LOG("recv msg error\n");
		ctx->handle_data_update_done = 0;
		return ACTION_FAILED;
	}

	LOG("upload version(%#x) done\n", ctx->local_version);

	return ACTION_DONE;
}


struct state_transform state_table[] = {
	{STATE_INIT, EVENT_ENTER_UPGRADE, STATE_UPGRADE_READY, action_enter_upgrade},

	{STATE_UPGRADE_READY, EVENT_GET_VERSION, STATE_UPGRADE_START, action_get_version},
	{STATE_UPGRADE_READY, EVENT_EXIT_UPGRADE, STATE_BUTT, action_exit_upgrade},

	{STATE_UPGRADE_START, EVENT_START_TRANS, STATE_GET_MD5, action_start_trans},
	{STATE_UPGRADE_START, EVENT_EXIT_UPGRADE, STATE_BUTT, action_exit_upgrade},

	{STATE_GET_MD5, EVENT_GET_MD5, STATE_DATA_TRANS, action_get_md5},
	{STATE_GET_MD5, EVENT_EXIT_UPGRADE, STATE_BUTT, action_exit_upgrade},

	{STATE_DATA_TRANS, EVENT_DATA_TRANS, STATE_DATA_TRANS_DONE, action_data_trans},
	{STATE_DATA_TRANS, EVENT_EXIT_UPGRADE, STATE_BUTT, action_exit_upgrade},

	{STATE_DATA_TRANS_DONE, EVENT_DATA_UPDATE, STATE_DATA_UPDATE_DONE, action_data_update},
	{STATE_DATA_TRANS_DONE, EVENT_EXIT_UPGRADE, STATE_BUTT, action_exit_upgrade},

	{STATE_DATA_UPDATE_DONE, EVENT_UPLOAD_VERSION, STATE_INIT, action_upload_version},
};

static struct state_transform* find_transform(struct state_machine* p_sm, EVENT evt)
{
	for (int i = 0; i < p_sm->trans_num; i++) {
	  if ((p_sm->transform[i].cur_state == p_sm->state) && (p_sm->transform[i].cur_event == evt)) {
	    return &p_sm->transform[i];
	  }
	}
	return NULL;
}

static int run_state_machine(struct state_machine* p_sm, EVENT evt, void *args)
{
	struct state_transform* p_trans;
	p_trans = find_transform(p_sm, evt);
	if (p_trans == NULL) {
		LOG("state:%d, event:%d, not found transform\n", p_sm->state, evt);
		return -1;
	}

	if (p_trans->action == NULL) {
		LOG("state:%d, event:%d, no action\n", p_sm->state, evt);
		return -1;
	}

	int ret = p_trans->action(args);
	// if (p_trans->action == action_data_trans) {
		// LOG("ret:%d,%d,%d\n", ret, p_sm->state, p_trans->next_state);
	// }
	if (ret == ACTION_DONE) {
		p_sm->state = p_trans->next_state;
	}
	else if (ret == ACTION_FAILED){
		LOG("state:%d, event:%d, action failed\n", p_sm->state, evt);
		return -1;
	}

	return 0;
}

int upgrade_init(void)
{
	int ret = 0;
	uint8_t buffer[PROTOCOL_LEN];
	EVENT event = EVENT_BUTT;
	struct state_machine sm;
	struct packet_param pkt = {0};
	struct upgrade_ctx *ctx = &s_ctx;
	unsigned int cur_ms1, cur_ms2;

	ret = ctx_init();
	if (ret != 0) {
		LOG("ctx_init failed\n");
		return -1;
	}
	LOG("upgrade ctx int done\n");

	cdc_acm_init();

	sm.state = STATE_INIT;
	sm.trans_num = sizeof(state_table) / sizeof(state_table[0]);
	sm.transform = state_table;

	LOG("start upgrade\n");
	LOG("compile time %s\n", __TIME__);

	cur_ms1 = csi_tick_get_ms();
	cur_ms2 = cur_ms1;

    while(ctx->run_flag) {

		// internel state
		if (sm.state == STATE_DATA_TRANS_DONE && !ctx->handle_data_trans) {
			LOG("data update start!\n");
			ctx->handle_data_trans = 1;
			event = EVENT_DATA_UPDATE;
		}
		else if (sm.state == STATE_DATA_UPDATE_DONE && !ctx->handle_data_update_done) {
			LOG("data update done!\n");
			ctx->handle_data_update_done = 1;
			event = EVENT_UPLOAD_VERSION;
		}
		else {
			// extern cmd
			ret = cdc_acm_read(buffer, PROTOCOL_LEN, 1000);
			if (ret == PROTOCOL_LEN) {
				cur_ms2 = csi_tick_get_ms();
				if (buffer[0] == 0xAA || buffer[1] == 0x55) {
					if (buffer[2] == 0xF7) {
						event = EVENT_GET_VERSION;
					}
					else if (buffer[2] == 0xF1) {
						LOG("enter upgrade!\n");
						event = EVENT_ENTER_UPGRADE;
					}
					else if (buffer[2] == 0xFB) {
						pkt.packet_num = (buffer[3] << 8) | buffer[4];
						pkt.version = buffer[5];
						LOG("packet_num:%d, version:%d\n", pkt.packet_num, pkt.version);
						ctx->handle_data_trans = 0;
						ctx->handle_data_update_done = 0;
						event = EVENT_START_TRANS;
					}
					else if (buffer[2] == 0xF3) {
						pkt.packet_seq = (buffer[3] << 8) | buffer[4];
						pkt.packet_len = (buffer[5] << 8) | buffer[6];
						pkt.crc_sum = buffer[7];
						event = EVENT_DATA_TRANS;
					}
					else if (buffer[2] == 0xF5) {
						LOG("exit upgrade!\n");
						event = EVENT_EXIT_UPGRADE;
					}
					else if (buffer[2] == 0xEC) {
						LOG("get md5!\n");
						event = EVENT_GET_MD5;
					}
				}
			}

			cur_ms1 = csi_tick_get_ms();
			if (cur_ms1 - cur_ms2 > CMD_TIMEOUT_MS) {
				LOG("upgrade time out\n");
				ctx->upgrade_status = 2;
				break;
			}
		}

		run_state_machine(&sm, event, (void *)&pkt);
		event = EVENT_BUTT;
    }

	// notify host
	uint8_t ret_msg[PROTOCOL_LEN] = {0xAA, 0x55, 0xE9, ctx->upgrade_status, 0x0, 0x00, 0x00, 0x00};
	cdc_acm_write(ret_msg, PROTOCOL_LEN);

	cdc_acm_deinit();
	LOG("cdc acm deinit\n");

	mdelay(1000);

	LOG("exit upgrade\n");

	return 0;
}

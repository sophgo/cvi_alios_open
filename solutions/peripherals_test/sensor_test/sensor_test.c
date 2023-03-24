#include <aos/kernel.h>
#include <stdio.h>
#include <ulog/ulog.h>
#include <unistd.h>
#include <aos/cli.h>
#include "fatfs_vfs.h"
#include "vfs.h"

#include "cvi_vi.h"
#include "media_video.h"
#include "cvi_param.h"
#include "common_vi.h"
#include "ae_test.h"

#if CONFIG_PQTOOL_SUPPORT == 1
#include "cvi_ispd2.h"
#endif

#define RET_CHAR '\n'
#define END_CHAR '\r'
#define PROMPT "# "

#define ALIGN(x, a) (((x) + ((a)-1)) & ~((a)-1))

#define CLI_CMD_STR_SIZE 64
#define TEST_DUMP_PATH SD_FATFS_MOUNTPOINT

extern int32_t cli_getchar(char *inbuf);
extern int32_t cli_handle_input(char *inbuf);

struct input_status
{
	char inbuf[64];
	unsigned int echo_disabled;
	unsigned int cmd_cur_pos;
	unsigned int cmd_end_pos;
};

//get console input
static int32_t get_input(char *inbuf, unsigned int size)
{
	char c;
	unsigned int i;
	struct input_status cli;

	if (inbuf == NULL) {
		aos_cli_printf("input null\r\n");
		return 0;
	}

	memset(&cli, 0, sizeof(struct input_status));
	cli.echo_disabled = 0; //default enable

	while (cli_getchar(&c) == 1) {
		if (cli.cmd_end_pos >= size) {
			aos_cli_printf("\r\nError: input buffer overflow\r\n");
			//cli_prefix_print();
			return 0;
		}

		/* received null or error */
		if (c == '\0' || c == 0xFF) {
			continue;
		}

		/* handle end of line, break */
		if (c == RET_CHAR || c == END_CHAR) { /* end of input line */
			cli.inbuf[cli.cmd_end_pos] = '\0';
			memcpy(inbuf, cli.inbuf, size);
			memset(cli.inbuf, 0, size);
			cli.cmd_cur_pos = 0;
			cli.cmd_end_pos = 0;
			return 1;
		}

		/* handle backspace or Ctrl H or Del key*/
		if ((c == 0x08) || (c == 0x7F)) {
			if (cli.cmd_cur_pos == 0) {
				continue;
			}

			cli.cmd_cur_pos--;
			cli.cmd_end_pos--;

			if (cli.cmd_end_pos > cli.cmd_cur_pos) {
				memmove(&cli.inbuf[cli.cmd_cur_pos],
						&cli.inbuf[cli.cmd_cur_pos + 1],
						cli.cmd_end_pos - cli.cmd_cur_pos);

				cli.inbuf[cli.cmd_end_pos] = 0;
				if (!cli.echo_disabled) {
					aos_cli_printf("\b%s  \b", &cli.inbuf[cli.cmd_cur_pos]);
				}
				/* move cursor */
				for (i = cli.cmd_cur_pos; i <= cli.cmd_end_pos; i++) {
					aos_cli_printf("\b");
				}
			} else {
				aos_cli_printf("\b \b");
				cli.inbuf[cli.cmd_end_pos] = 0;
			}
			continue;
		}

		/* discard large cmd */
		if (cli.cmd_end_pos >= size) {
			cli.cmd_end_pos = 0;
		}

		/* others: handle normal character */
		if (cli.cmd_cur_pos < cli.cmd_end_pos) {
			memmove(&cli.inbuf[cli.cmd_cur_pos + 1],
					&cli.inbuf[cli.cmd_cur_pos],
					cli.cmd_end_pos - cli.cmd_cur_pos);
			cli.inbuf[cli.cmd_cur_pos] = c;

			if (!cli.echo_disabled) {
				aos_cli_printf("%s", &cli.inbuf[cli.cmd_cur_pos]);
			}

			/* move cursor to new position */
			for (i = cli.cmd_cur_pos; i < cli.cmd_end_pos; i++) {
				aos_cli_printf("\b");
			}
		} else {
			cli.inbuf[cli.cmd_end_pos] = c;
			if (!cli.echo_disabled) {
				aos_cli_printf("%c", c);
			}
		}
		cli.cmd_cur_pos++;
		cli.cmd_end_pos++;

		if (cli.cmd_end_pos >= size) {
			cli.cmd_cur_pos = 0;
			cli.cmd_end_pos = 0;
		}
	}
	return 0;
}

//return :split number
int split(char *argv[], char *str, char separator)
{
	char *start_c = str;
	char *end_c = str;

	int index = 0;

	while (*end_c != '\0') {
		if (*end_c == separator) {
			if (start_c == end_c) {
				argv[index] = NULL;
			}
			else {
				argv[index] = start_c;
				*end_c = '\0';
				index++;
			}
			start_c = end_c + 1;
		}
		end_c++;
	}
	if (start_c != end_c) {
		argv[index] = start_c;
	} else {
		index--;
	}
	return index + 1;
}

int self_scanf(const char *s_type, void *data)
{

	char c[CLI_CMD_STR_SIZE];

	get_input(c, CLI_CMD_STR_SIZE);
	if (strcmp(s_type, "%d") == 0) {
		*(int *)data = atoi(c);
	} else if (strcmp(s_type, "%s") == 0) {
		memcpy((char *)data, c, strlen(c));
	} else if (strcmp(s_type, "%lx") == 0) {
		*(unsigned long int *)data = strtoul(c, NULL, 0);
	} else if (strcmp(s_type, "%x") == 0) {
		*(unsigned int *)data = strtoul(c, NULL, 0);
	} else {
		aos_cli_printf("don't support type[%s]\r\n", s_type);
		return CVI_FAILURE;
	}
	aos_cli_printf("\r\n");
	return CVI_SUCCESS;
}

int parse_str2int_space(const char *str, int *data, int num)
{

#define SPLIT_MAX_NUM 12
	char *argv_index[SPLIT_MAX_NUM] = {NULL,};
	char strbuf[CLI_CMD_STR_SIZE] = {0,};

	if (num > SPLIT_MAX_NUM) {
		aos_cli_printf("console input number[%d] > %d \r\n", num, SPLIT_MAX_NUM);
		return CVI_FAILURE;
	}
	memcpy(strbuf, str, strlen(str));
	int split_num = split(argv_index, strbuf, ' ');
	if (split_num < num) {
		aos_cli_printf("console input number[%d] < setting num[%d] \r\n", split_num, num);
		return CVI_FAILURE;
	}

	for (int i = 0; i < num; i++) {
		*(data + i) = atoi(argv_index[i]);
	}
	return CVI_SUCCESS;
}

static int sys_vi_init(void)
{
	//return CVI_SUCCESS;
	PARAM_SYS_CFG_S *pstSysCtx = PARAM_getSysCtx();
	PARAM_VI_CFG_S *pstViCfg = PARAM_getViCtx();

	//media vi init
	MEDIA_CHECK_RET(MEDIA_VIDEO_SysVbInit(pstSysCtx), "MEDIA_VIDEO_SysVbInit failed");
	MEDIA_CHECK_RET(MEDIA_VIDEO_ViInit(pstViCfg), "MEDIA_VIDEO_ViInit failed");
	return CVI_SUCCESS;
}

static void sys_vi_deinit(void)
{
	PARAM_VI_CFG_S *pstViCfg = PARAM_getViCtx();

	MEDIA_VIDEO_ViDeinit(pstViCfg);
	MEDIA_VIDEO_SysVbDeinit();
}

static CVI_S32 _vi_get_chn_frame(CVI_U8 chn)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	VI_CROP_INFO_S crop_info = {0};

	if (CVI_VI_GetChnFrame(0, chn, &stVideoFrame, 3000) == 0) {
		size_t image_size = stVideoFrame.stVFrame.u32Length[0] + 
						stVideoFrame.stVFrame.u32Length[1] + stVideoFrame.stVFrame.u32Length[2];
		CVI_VOID *vir_addr;
		CVI_U32 plane_offset, u32LumaSize, u32ChromaSize;
		CVI_CHAR img_name[128] = {
			0,
		};

		CVI_TRACE_LOG(CVI_DBG_WARN, "width: %d, height: %d, total_buf_length: %zu\r\n",
					  stVideoFrame.stVFrame.u32Width,
					  stVideoFrame.stVFrame.u32Height, image_size);

		snprintf(img_name, sizeof(img_name), "%s/sample_%d.yuv", TEST_DUMP_PATH, chn);

		aos_remove(img_name);
		int output = aos_open(img_name, O_CREAT | O_RDWR);
		if (output < 0) {
			CVI_VI_ReleaseChnFrame(0, chn, &stVideoFrame);
			CVI_TRACE_LOG(CVI_DBG_ERR, "aos_open %s failed\r\n", img_name);
			CVI_TRACE_LOG(CVI_DBG_ERR, "please check whether the sdcard exists?\r\n");
			return CVI_FAILURE;
		}

		u32LumaSize = stVideoFrame.stVFrame.u32Stride[0] * stVideoFrame.stVFrame.u32Height;
		u32ChromaSize = stVideoFrame.stVFrame.u32Stride[1] * stVideoFrame.stVFrame.u32Height / 2;
		CVI_VI_GetChnCrop(0, chn, &crop_info);
		if (crop_info.bEnable) {
			u32LumaSize = ALIGN((crop_info.stCropRect.u32Width * 8 + 7) >> 3, DEFAULT_ALIGN) *
						  ALIGN(crop_info.stCropRect.u32Height, 2);
			u32ChromaSize = (ALIGN(((crop_info.stCropRect.u32Width >> 1) * 8 + 7) >> 3, DEFAULT_ALIGN) *
							 ALIGN(crop_info.stCropRect.u32Height, 2)) >>
							1;
		}
		vir_addr = (CVI_U8 *)stVideoFrame.stVFrame.u64PhyAddr[0];
		plane_offset = 0;
		for (int i = 0; i < 3; i++) {
			if (stVideoFrame.stVFrame.u32Length[i] != 0) {
				stVideoFrame.stVFrame.pu8VirAddr[i] = vir_addr + plane_offset;
				plane_offset += stVideoFrame.stVFrame.u32Length[i];
				CVI_TRACE_LOG(CVI_DBG_WARN,
							  "plane(%d): paddr(%#lx) vaddr(%p) stride(%d) length(%d)\r\n",
							  i, stVideoFrame.stVFrame.u64PhyAddr[i],
							  stVideoFrame.stVFrame.pu8VirAddr[i],
							  stVideoFrame.stVFrame.u32Stride[i],
							  stVideoFrame.stVFrame.u32Length[i]);
				aos_write(output, stVideoFrame.stVFrame.pu8VirAddr[i], 
									(i == 0) ? u32LumaSize : u32ChromaSize);
			}
		}
		if (CVI_VI_ReleaseChnFrame(0, chn, &stVideoFrame) != 0)
			CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_ReleaseChnFrame NG\r\n");

		aos_close(output);
		return CVI_SUCCESS;
	}
	CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_GetChnFrame NG\r\n");
	return CVI_FAILURE;
}

static long diff_in_us(struct timespec t1, struct timespec t2)
{
	struct timespec diff;

	if (t2.tv_nsec - t1.tv_nsec < 0) {
		diff.tv_sec = t2.tv_sec - t1.tv_sec - 1;
		diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
	}
	else {
		diff.tv_sec = t2.tv_sec - t1.tv_sec;
		diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
	}
	return (diff.tv_sec * 1000000.0 + diff.tv_nsec / 1000.0);
}

static CVI_S32 sensor_dump_yuv(void)
{
	CVI_S32 loop = 0;
	CVI_U32 ok = 0, ng = 0;
	CVI_U8 chn = 0;
	int tmp;
	struct timespec start, end;

	CVI_TRACE_LOG(CVI_DBG_WARN, "Get frm from which chn(0~1): ");
	self_scanf("%d", &tmp);
	chn = tmp;
	CVI_TRACE_LOG(CVI_DBG_WARN, "how many loops to do(11111 is infinite: ");
	self_scanf("%d", &loop);
	while (loop > 0) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		if (_vi_get_chn_frame(chn) == CVI_SUCCESS) {
			++ok;
			clock_gettime(CLOCK_MONOTONIC, &end);
			CVI_TRACE_LOG(CVI_DBG_WARN, "ms consumed: %f\r\n",
						  (CVI_FLOAT)diff_in_us(start, end) / 1000);
		}
		else
			++ng;
		//sleep(1);
		if (loop != 11111)
			loop--;
	}
	CVI_TRACE_LOG(CVI_DBG_WARN, "VI GetChnFrame OK(%d) NG(%d)\r\n", ok, ng);

	CVI_TRACE_LOG(CVI_DBG_WARN, "Dump VI yuv TEST-PASS\r\n");

	return CVI_SUCCESS;
}

static CVI_S32 sensor_flip_mirror(void)
{
	int flip;
	int mirror;
	int chnID;
	int pipeID;

	CVI_TRACE_LOG(CVI_DBG_WARN, "chn(0~1): ");
	self_scanf("%d", &chnID);
	CVI_TRACE_LOG(CVI_DBG_WARN, "Flip enable/disable(1/0): ");
	self_scanf("%d", &flip);
	CVI_TRACE_LOG(CVI_DBG_WARN, "Mirror enable/disable(1/0): ");
	self_scanf("%d", &mirror);
	pipeID = chnID;
	CVI_VI_SetChnFlipMirror(pipeID, chnID, flip, mirror);

	return CVI_SUCCESS;
}

static CVI_S32 sensor_dump_raw(void)
{
	VIDEO_FRAME_INFO_S stVideoFrame[2];
	VI_DUMP_ATTR_S attr;
	struct timeval tv1;
	int frm_num = 1, j = 0;
	CVI_U32 dev = 0, loop = 0;
	struct timespec start, end;
	CVI_S32 s32Ret = CVI_SUCCESS;

	memset(stVideoFrame, 0, sizeof(stVideoFrame));

	stVideoFrame[0].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;
	stVideoFrame[1].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;

	CVI_TRACE_LOG(CVI_DBG_WARN, "To get raw dump from dev(0~1): ");
	self_scanf("%d", &dev);

	attr.bEnable = 1;
	attr.u32Depth = 0;
	attr.enDumpType = VI_DUMP_TYPE_RAW;

	CVI_VI_SetPipeDumpAttr(dev, &attr);

	attr.bEnable = 0;
	attr.enDumpType = VI_DUMP_TYPE_IR;

	CVI_VI_GetPipeDumpAttr(dev, &attr);

	CVI_TRACE_LOG(CVI_DBG_WARN, "Enable(%d), DumpType(%d):\r\n", attr.bEnable, attr.enDumpType);
	CVI_TRACE_LOG(CVI_DBG_WARN, "how many loops to do (1~60)");
	self_scanf("%d", &loop);

	if (loop > 60)
		return s32Ret;

	while (loop > 0) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		frm_num = 1;

		CVI_VI_GetPipeFrame(dev, stVideoFrame, 1000);

		if (stVideoFrame[1].stVFrame.u64PhyAddr[0] != 0)
			frm_num = 2;

		gettimeofday(&tv1, NULL);

		for (j = 0; j < frm_num; j++) {
			size_t image_size = stVideoFrame[j].stVFrame.u32Length[0];
			unsigned char *ptr = calloc(1, image_size);
			char img_name[128] = {
				0,
			},
				 order_id[8] = {
					 0,
				 };

			if (attr.enDumpType == VI_DUMP_TYPE_RAW) {
				stVideoFrame[j].stVFrame.pu8VirAddr[0] = (CVI_U8 *)stVideoFrame[j].stVFrame.u64PhyAddr[0];
				CVI_TRACE_LOG(CVI_DBG_WARN, "paddr(%#lx) vaddr(%p)\r\n",
							  stVideoFrame[j].stVFrame.u64PhyAddr[0],
							  stVideoFrame[j].stVFrame.pu8VirAddr[0]);

				memcpy(ptr, (const void *)stVideoFrame[j].stVFrame.pu8VirAddr[0],
					   stVideoFrame[j].stVFrame.u32Length[0]);

				switch (stVideoFrame[j].stVFrame.enBayerFormat)
				{
				default:
				case BAYER_FORMAT_BG:
					snprintf(order_id, sizeof(order_id), "BG");
					break;
				case BAYER_FORMAT_GB:
					snprintf(order_id, sizeof(order_id), "GB");
					break;
				case BAYER_FORMAT_GR:
					snprintf(order_id, sizeof(order_id), "GR");
					break;
				case BAYER_FORMAT_RG:
					snprintf(order_id, sizeof(order_id), "RG");
					break;
				}

				snprintf(img_name, sizeof(img_name),
						 "%s/vi_%d_%s_%s_w_%d_h_%d_x_%d_y_%d_tv_%ld_%ld.raw",
						 TEST_DUMP_PATH, dev, (j == 0) ? "LE" : "SE", order_id,
						 stVideoFrame[j].stVFrame.u32Width,
						 stVideoFrame[j].stVFrame.u32Height,
						 stVideoFrame[j].stVFrame.s16OffsetLeft,
						 stVideoFrame[j].stVFrame.s16OffsetTop,
						 tv1.tv_sec, tv1.tv_usec);

				CVI_TRACE_LOG(CVI_DBG_WARN, "dump image %s\r\n", img_name);
				aos_remove(img_name);

				int output = aos_open(img_name, O_CREAT | O_RDWR);
				if (output < 0)
				{
					CVI_TRACE_LOG(CVI_DBG_ERR, "aos_open %s failed\r\n", img_name);
					CVI_TRACE_LOG(CVI_DBG_ERR, "please check whether the sdcard exists?\r\n");
					return CVI_FAILURE;
				}
				aos_write(output, ptr, image_size);
				aos_close(output);
				free(ptr);
			}
		}

		CVI_VI_ReleasePipeFrame(dev, stVideoFrame);

		clock_gettime(CLOCK_MONOTONIC, &end);
		CVI_TRACE_LOG(CVI_DBG_WARN, "ms consumed: %f\r\n",
					  (CVI_FLOAT)diff_in_us(start, end) / 1000);

		loop--;
	}

	CVI_TRACE_LOG(CVI_DBG_WARN, "Dump VI raw TEST-PASS\r\n");

	return s32Ret;
}

static CVI_S32 sensor_linear_wdr_switch(void)
{
	int tmp;
	CVI_U8 wdrMode = 0;
	CVI_S32 s32Ret = CVI_SUCCESS;

	PARAM_VI_CFG_S *pstViCfg = PARAM_getViCtx();
	MEDIA_VIDEO_ViDeinit(pstViCfg);
	// select which mode want to switch.
	aos_cli_printf("\r\nPlease select sensor input mode (0:linear/1:wdr) :");
	self_scanf("%d", &tmp);
	wdrMode = tmp;
	if (wdrMode == 0) {
		// Reset main sensor initial config to linear setting.
		pstViCfg->pstSensorCfg->enSnsType = SMS_SC200AI_MIPI_2M_30FPS_10BIT;
		pstViCfg->pstSensorCfg->s32WDRMode = WDR_MODE_NONE;

	} else {
		// Reset main sensor initial config to wdr setting
		pstViCfg->pstSensorCfg->enSnsType = SMS_SC200AI_MIPI_2M_30FPS_10BIT_WDR2TO1;
		pstViCfg->pstSensorCfg->s32WDRMode = WDR_MODE_2To1_LINE;
	}
	// Reconfig VI setting for different mode Re-initial correctly.
	s32Ret = MEDIA_VIDEO_ViInit(pstViCfg);
	if (s32Ret != CVI_SUCCESS) {
		aos_cli_printf("MEDIA_VIDEO_ViInit fail \r\n");
	}
	return s32Ret;
}

int sensor_dump(void)
{
	CVI_U64 addr = 0;
	CVI_U32 size = 0;

	CVI_TRACE_LOG(CVI_DBG_WARN, "dump addr: \r\n");
	self_scanf("%lx", &addr);
	CVI_TRACE_LOG(CVI_DBG_WARN, "dump size: \r\n");
	self_scanf("%d", &size);

	aos_cli_printf("addr: 0x%08x,size:%d\r\n", (unsigned int *)addr, size);

	for (int i = 0; i < size; i++) {
		if (i % 4 == 0) {
			aos_cli_printf("%08p:", addr);
		}
		aos_cli_printf(" %08x", *(unsigned int *)addr);
		addr += 4;
		if (i % 4 == 3) {
			aos_cli_printf("\r\n");
		}
	}
	aos_cli_printf("\r\n");
	return CVI_SUCCESS;
}

int sensor_proc(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 op;

	aos_cli_printf("---debug_info------------------------------------------------\r\n");
	aos_cli_printf("1: proc_vi_dbg\r\n");
	aos_cli_printf("2: proc_vi\r\n");
	aos_cli_printf("3: proc_mipi_rx\r\n");
	self_scanf("%d", &op);

	switch (op) {
	case 1:
		cli_handle_input("proc_vi_dbg");
		break;
	case 2:
		cli_handle_input("proc_vi");
		break;
	case 3:
		cli_handle_input("proc_mipi_rx");
		break;
	default:
		break;
	}

	return s32Ret;
}

int sensor_i2c_rw(void)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	CVI_S32 op = 0;
	CVI_S32 chn = 0;
	CVI_S32 value = 0;
	CVI_U32 addr;
	CVI_U8 devNum = 2;
	CVI_S32 snsr_type[2];
	ISP_SNS_OBJ_S *pSnsObj[2];

	aos_cli_printf("sel which vi chn(0~1)	: ");
	self_scanf("%d", &chn);
	if ((op > 1))
		aos_cli_printf("vi chn [%d] invaild \r\n", op);

	aos_cli_printf("dir  (1:read/2:write)	: ");
	self_scanf("%d", &op);
	if ((op != 1) && (op != 2))
		aos_cli_printf("input [%d] invaild \r\n", op);

	aos_cli_printf("input register addr  	: ");
	self_scanf("%x", &addr);

	getSnsType(snsr_type, &devNum);
	pSnsObj[chn] = getSnsObj(snsr_type[chn]);

	if (op == 1) {
		if (pSnsObj[chn]->pfnReadReg) {
			aos_cli_printf("\r\nread addr: 0x%04x, value : 0x%02x\r\n\r\n",
								 addr, pSnsObj[chn]->pfnReadReg(chn, addr));
		}
	} else if (op == 2) {
		aos_cli_printf("input write value 	: ");
		self_scanf("%x", &value);
		if (pSnsObj[chn]->pfnWriteReg) {
			if (CVI_SUCCESS == pSnsObj[chn]->pfnWriteReg(chn, addr, value)) {
				aos_cli_printf("write success\r\n");
			}
		}
	}
	return s32Ret;
}

void sensor_test(int32_t argc, char **argv)
{
	/**
	 * for example:
	 * sensor_test
	 */
	CVI_S32 s32Ret = CVI_SUCCESS;
	int op;

	s32Ret = sys_vi_init();
	if (s32Ret != CVI_SUCCESS)
		return;

#if (CONFIG_PQTOOL_SUPPORT == 1)
	usleep(12 * 1000);
	isp_daemon2_init(5566);
#endif

	usleep(500 * 1000);

	do {
		aos_cli_printf("---Basic------------------------------------------------\r\n");
		aos_cli_printf("1: dump vi raw frame\r\n");
		aos_cli_printf("2: dump vi yuv frame\r\n");
		aos_cli_printf("3: set chn flip/mirror\r\n");
		aos_cli_printf("4: linear wdr switch\r\n");
		aos_cli_printf("5: AE debug\r\n");
		aos_cli_printf("6: sensor dump\r\n");
		aos_cli_printf("7: sensor proc\r\n");
		aos_cli_printf("8: sensor i2c read/write\r\n");
		aos_cli_printf("255: exit\r\n");
		self_scanf("%d", &op);

		aos_cli_printf("input op num is [%d]\r\n", op);
		switch (op) {
		case 1:
			s32Ret = sensor_dump_raw();
			break;
		case 2:
			s32Ret = sensor_dump_yuv();
			break;
		case 3:
			s32Ret = sensor_flip_mirror();
			break;
		case 4:
			s32Ret = sensor_linear_wdr_switch();
			break;
		case 5:
			s32Ret = sensor_ae_test();
			break;
		case 6:
			s32Ret = sensor_dump();
			break;
		case 7:
			s32Ret = sensor_proc();
			break;
		case 8:
			s32Ret = sensor_i2c_rw();
			break;
		default:
			break;
		}
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "op(%d) failed with %#x!\r\n", op, s32Ret);
			break;
		}
	} while (op != 255);

	sys_vi_deinit();
#if (CONFIG_PQTOOL_SUPPORT == 1)
	isp_daemon2_uninit();
#endif
}

ALIOS_CLI_CMD_REGISTER(sensor_test, sensor_test, sensor test);

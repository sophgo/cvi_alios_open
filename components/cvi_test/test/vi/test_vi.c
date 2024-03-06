#include "board.h"
#include <aos/kernel.h>
#include <stdio.h>
#include <ulog/ulog.h>
#include <aos/cli.h>
#include "fatfs_vfs.h"
#include "vfs.h"
#include <cvi_base.h>
#include "vi_uapi.h"
#include "cvi_vb.h"
#include "cvi_sys.h"
#include "cvi_vi.h"
#include "cvi_isp.h"
#include "cvi_buffer.h"
#include "test_vi.h"
#include "cvi_sns_ctrl.h"
#include "cif_uapi.h"
#include "common_vi.h"
#include <pinctrl-mars.h>
#include "vi_isp.h"
#include <inttypes.h>

#define S_CTRL_PTR(_cfg, _ioctl)\
	do {\
		struct vi_ext_control ec1;\
		memset(&ec1, 0, sizeof(ec1));\
		ec1.id = _ioctl;\
		ec1.ptr = (void *)_cfg;\
		if (vi_ioctl(VI_IOC_S_CTRL, &ec1) < 0) {\
			fprintf(stderr, "VI_IOC_S_CTRL - %s NG, %s\n", __func__, strerror(errno));\
			return -1;\
		} \
		return 0;\
	} while (0)

static int _vi_set_snsr_info(CVI_U8 mode, CVI_U8 raw_num)
{
	struct cvi_isp_snr_info cfg;
	CVI_U8 i = 0;
//	CVI_U8 snsr_type = 0;

	cfg.raw_num = raw_num;
	cfg.color_mode = BAYER_RGGB;
	if (raw_num == 0) {
		get_sensor_type(raw_num);
		if (mode == WDR_MODE_2To1_LINE)
			cfg.snr_fmt.frm_num = 2;
		else
			cfg.snr_fmt.frm_num = 1;
		for (i = 0; i < cfg.snr_fmt.frm_num; i++) {
#if (snsr_type == SONY_IMX307_MIPI_2M_30FPS_12BIT)
			cfg.snr_fmt.img_size[i].max_width = 1948;
			cfg.snr_fmt.img_size[i].max_height = 1097;
			cfg.snr_fmt.img_size[i].start_x = 12;
			cfg.snr_fmt.img_size[i].start_y = 8;
			cfg.snr_fmt.img_size[i].active_w = 1920;
			cfg.snr_fmt.img_size[i].active_h = 1080;
			cfg.snr_fmt.img_size[i].width = 1948;
			cfg.snr_fmt.img_size[i].height = 1097;
#else
			cfg.snr_fmt.img_size[i].max_width = 1920;
			cfg.snr_fmt.img_size[i].max_height = 1080;
			cfg.snr_fmt.img_size[i].start_x = 0;
			cfg.snr_fmt.img_size[i].start_y = 0;
			cfg.snr_fmt.img_size[i].active_w = 1920;
			cfg.snr_fmt.img_size[i].active_h = 1080;
			cfg.snr_fmt.img_size[i].width = 1920;
			cfg.snr_fmt.img_size[i].height = 1080;
#endif
		}
	} else {
		if (mode == WDR_MODE_2To1_LINE)
			cfg.snr_fmt.frm_num = 2;
		else
			cfg.snr_fmt.frm_num = 1;
		for (i = 0; i < cfg.snr_fmt.frm_num; i++) {
			cfg.snr_fmt.img_size[i].max_width = 1920;
			cfg.snr_fmt.img_size[i].max_height = 1080;
			cfg.snr_fmt.img_size[i].start_x = 0;
			cfg.snr_fmt.img_size[i].start_y = 0;
			cfg.snr_fmt.img_size[i].active_w = 1920;
			cfg.snr_fmt.img_size[i].active_h = 1080;
			cfg.snr_fmt.img_size[i].width = 1920;
			cfg.snr_fmt.img_size[i].height = 1080;
		}
	}

	S_CTRL_PTR(&cfg, VI_IOCTL_SET_SNR_INFO);
}

void start_snsr(CVI_U8 *devNum)
// void start_snsr(int32_t argc, char **argv)
{
	SNS_COMBO_DEV_ATTR_S devAttr = {0};
	CVI_S32 snsr_type[2];
	ISP_SNS_OBJ_S *pSnsObj[2];
	ISP_CMOS_SENSOR_IMAGE_MODE_S stSnsrMode[2] = {
		{1920, 1080, 30, 0}
	};
	CVI_U8 dev_num;
	ISP_SNS_COMMBUS_U unSnsrBusInfo[2] = {
		{.s8I2cDev = 2,},
		{.s8I2cDev = 2,}
	};
	// RX_INIT_ATTR_S stRxInitAttr;
	ISP_SENSOR_EXP_FUNC_S stSnsrSensorFunc[2] = {0};
	ISP_INIT_ATTR_S InitAttr = {0};
	ALG_LIB_S stAeLib = {0};
	ALG_LIB_S stAwbLib = {0};
	CVI_S32 s32Ret;
	static csi_gpio_t snsr0_rst_gpio;
	static csi_gpio_t snsr1_rst_gpio;
	struct snsr_rst_gpio_s snsr_gpio[2] = {
		{
			.snsr_rst_gpio = snsr0_rst_gpio,
			.snsr_rst_port_idx = 2, //GPIOC_13
			.snsr_rst_pin = 13,
			.snsr_rst_pol = OF_GPIO_ACTIVE_LOW,
		},
		{
			.snsr_rst_gpio = snsr1_rst_gpio,
			.snsr_rst_port_idx = 2,
			.snsr_rst_pin = 13,
			.snsr_rst_pol = OF_GPIO_ACTIVE_LOW,
		},
	};

	PINMUX_CONFIG(PAD_MIPIRX4P, XGPIOC_3);
	PINMUX_CONFIG(PAD_MIPIRX4N, XGPIOC_2);

	PINMUX_CONFIG(PAD_MIPI_TXP1, IIC2_SCL);
	PINMUX_CONFIG(PAD_MIPI_TXM1, IIC2_SDA);
	PINMUX_CONFIG(PAD_MIPI_TXP0, XGPIOC_13);
	PINMUX_CONFIG(PAD_MIPI_TXM0, CAM_MCLK1);

	APP_CHECK_RET(getSnsType(snsr_type, devNum), "getSnsType fail");
	dev_num = *devNum;

	s32Ret = getSnsType(snsr_type, &dev_num);
	if (s32Ret != CVI_SUCCESS) {
		printf ("getSnsType fail\r\n");
		return;
	}

	for (CVI_U8 i = 0; i < dev_num; i++) {
		pSnsObj[i] = getSnsObj(snsr_type[i]);
		APP_CHECK_RET(getSnsMode(i, &stSnsrMode[i]), "stSnsrMode fail");
	}

	/* clock enable */
	// vip_clk_en();

	/************************************************
	 * start sensor
	 ************************************************/
	InitAttr.enGainMode = SNS_GAIN_MODE_SHARE;

	for (CVI_U8  i = 0; i < dev_num; ++i) {
		if (!pSnsObj[i]) {
			printf("sns obj[%d] is null.\n", i);
			return;
		}

		cvi_cif_reset_snsr_gpio_init(i, &snsr_gpio[i]);

		pSnsObj[i]->pfnSetInit(i, &InitAttr);

		printf("bus info:%d\n", unSnsrBusInfo[i].s8I2cDev);
		pSnsObj[i]->pfnSetBusInfo(i, unSnsrBusInfo[i]);

		pSnsObj[i]->pfnRegisterCallback(i, &stAeLib, &stAwbLib);

		pSnsObj[i]->pfnExpSensorCb(&stSnsrSensorFunc[i]);

		stSnsrSensorFunc[i].pfn_cmos_sensor_global_init(i);

		s32Ret = stSnsrSensorFunc[i].pfn_cmos_set_image_mode(i, &stSnsrMode[i]);
		if (s32Ret != CVI_SUCCESS) {
			printf("sensor set image mode failed!\n");
			return;
		}

		s32Ret = stSnsrSensorFunc[i].pfn_cmos_set_wdr_mode(i, stSnsrMode[i].u8SnsMode);
		if (s32Ret != CVI_SUCCESS) {
			printf("sensor set wdr mode failed!\n");
			return;
		}
		if(i==0) {
			cif_reset_snsr_gpio(i, 1);
		}

		cif_reset_mipi(i);

		usleep(100);

		pSnsObj[i]->pfnGetRxAttr(i, &devAttr);

		cif_set_dev_attr(&devAttr);

		cif_enable_snsr_clk(i, 1);

		usleep(100);

		cif_reset_snsr_gpio(i, 0);
		usleep(1000);
		if (pSnsObj[i]->pfnSnsProbe) {
			s32Ret = pSnsObj[i]->pfnSnsProbe(i);
			if (s32Ret) {
				printf("sensor probe failed!\n");
				return;
			}
		}
	}
	for (int i = 0; i < dev_num; ++i) {
		stSnsrSensorFunc[i].pfn_cmos_sensor_init(i);
		// vip_csi_bdg_dma_cfg(i, WDR_MODE_NONE);
		// printf("[%s][intr_status:0x%x]\n", __func__, i == 0 ? mmio_read_32(0x0A0C2440) : mmio_read_32(0x0A0C4440));
	}
}
// ALIOS_CLI_CMD_REGISTER(start_snsr, start_snsr, start snsr);


void start_vi(int32_t argc, char **argv)
{
	//CVI_U32 s32Ret = CVI_SUCCESS;
	CVI_U8 devNum = 1;
	VI_DEV ViDev = 0;
	VI_CHN ViChn = 0;
	VI_DEV_ATTR_S stViDevAttr = {
		VI_MODE_MIPI,
		VI_WORK_MODE_1Multiplex,
		VI_SCAN_PROGRESSIVE,
		{-1, -1, -1, -1},
		VI_DATA_SEQ_YUYV,
		{
		/*port_vsync   port_vsync_neg     port_hsync        port_hsync_neg        */
		VI_VSYNC_PULSE, VI_VSYNC_NEG_LOW, VI_HSYNC_VALID_SINGNAL, VI_HSYNC_NEG_HIGH,
		VI_VSYNC_VALID_SIGNAL, VI_VSYNC_VALID_NEG_HIGH,

		/*hsync_hfb    hsync_act    hsync_hhb*/
		{0,            1920,        0,
		/*vsync0_vhb vsync0_act vsync0_hhb*/
		0,            1080,        0,
		/*vsync1_vhb vsync1_act vsync1_hhb*/
		0,            0,            0}
		},
		VI_DATA_TYPE_RGB,
		VI_PATH_DEFAULT,
		{1920, 1080},
		{
			WDR_MODE_NONE,
			1080
		},
		.enBayerFormat = BAYER_FORMAT_BG,
	};

	ISP_CMOS_SENSOR_IMAGE_MODE_S stSnsrMode[2] = {
		{1920, 1080, 30, 0}
	};

	//APP_CHECK_RET(CVI_SYS_VI_Open(), "CVI_SYS_VI_Open fail");

	// if (devNum == 1)
	start_snsr(&devNum);

	CVI_VI_SetDevNum(devNum);

	// //set vi online
	// VI_VPSS_MODE_S stVIVPSSMode;

	// stVIVPSSMode.aenMode[0] = VI_ONLINE_VPSS_OFFLINE;

	//s32Ret = CVI_SYS_SetVIVPSSMode(&stVIVPSSMode);
	//if (s32Ret != CVI_SUCCESS) {
	//	printf("CVI_SYS_SetVIVPSSMode failed with %#x\n", s32Ret);
	//	return;
	//}

	for (int i = 0; i < devNum; i++) {
		ViDev = i;
		//check it's wdr or linear
		APP_CHECK_RET(getSnsMode(i, &stSnsrMode[i]), "stSnsrMode fail");
		stViDevAttr.stWDRAttr.enWDRMode = stSnsrMode[i].u8SnsMode;

		_vi_set_snsr_info(stSnsrMode[i].u8SnsMode, i);
		APP_CHECK_RET(CVI_VI_SetDevAttr(ViDev, &stViDevAttr), "CVI_VI_SetDevAttr fail");
		APP_CHECK_RET(CVI_VI_EnableDev(ViDev), "CVI_VI_EnableDev fail");
	}

	VI_PIPE_ATTR_S stPipeAttr;

	stPipeAttr.bYuvSkip = CVI_FALSE;
	stPipeAttr.u32MaxW = 1920;
	stPipeAttr.u32MaxH = 1080;
	stPipeAttr.enPixFmt = PIXEL_FORMAT_RGB_BAYER_12BPP;
	stPipeAttr.enBitWidth = DATA_BITWIDTH_12;
	stPipeAttr.stFrameRate.s32SrcFrameRate = -1;
	stPipeAttr.stFrameRate.s32DstFrameRate = -1;
	stPipeAttr.bNrEn = CVI_TRUE;
	stPipeAttr.bYuvBypassPath = CVI_FALSE;
	stPipeAttr.enCompressMode = COMPRESS_MODE_TILE;

	for (int i = 0; i < devNum; i++) {
		ViDev = i;

		APP_CHECK_RET(CVI_VI_CreatePipe(ViDev, &stPipeAttr), "CVI_VI_CreatePipe fail");
		APP_CHECK_RET(CVI_VI_StartPipe(ViDev), "CVI_VI_StartPipe fail");
	}

	VI_CHN_ATTR_S stChnAttr = {
		{1920, 1080},
		PIXEL_FORMAT_YUV_PLANAR_420,
		DYNAMIC_RANGE_SDR8,
		VIDEO_FORMAT_LINEAR,
		COMPRESS_MODE_NONE,
		CVI_FALSE, CVI_FALSE,
		0,
		{ -1, -1}
	};

	for (int i = 0; i < devNum; i++) {
		ViChn = ViDev = i;

		APP_CHECK_RET(CVI_VI_SetChnAttr(ViDev, ViChn, &stChnAttr), "CVI_VI_SetChnAttr fail");
		//Sensor support single sensor;For dual mode, use csi_bdg_pattern
		// if (devNum == 1)
		// 	APP_CHECK_RET(CVI_VI_EnablePatt(ViDev), "CVI_VI_SetPattern fail");
		APP_CHECK_RET(CVI_VI_EnableChn(ViDev, ViChn), "CVI_VI_SetChnAttr fail");
	}
}
ALIOS_CLI_CMD_REGISTER(start_vi, start_vi, start vi);

void stop_vi(int32_t argc, char **argv)
{
	VI_DEV ViDev = 0;
	VI_CHN ViChn = 0;
	CVI_S32 ret = CVI_SUCCESS;
	CVI_U8 i = 0, devNum = 1;

	for (i = 0; i < devNum; i++) {
		ViChn = ViDev = i;
		ret = CVI_VI_DisableChn(ViDev, ViChn);
		if (ret != CVI_SUCCESS) {
			aos_cli_printf("CVI_VI_DisableChn TEST-FAIL!\n");
			return;
		}

		//disable vi_dev
		APP_CHECK_RET(CVI_VI_DisableDev(ViDev), "CVI_VI_DisableDev fail");
	}

	APP_CHECK_RET(CVI_SYS_VI_Close(), "CVI_SYS_VI_Close fail");
	aos_cli_printf("******stop vi******\n");
}
ALIOS_CLI_CMD_REGISTER(stop_vi, stop_vi, stop vi);

static CVI_S32 _vi_get_chn_frame(CVI_U8 chn, CVI_U8 loop)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	VI_CROP_INFO_S crop_info = {0};

	if (CVI_VI_GetChnFrame(0, chn, &stVideoFrame, 3000) == 0) {
		size_t image_size = stVideoFrame.stVFrame.u32Length[0] + stVideoFrame.stVFrame.u32Length[1]
				  + stVideoFrame.stVFrame.u32Length[2];
		CVI_VOID *vir_addr;
		CVI_U32 plane_offset, u32LumaSize, u32ChromaSize;
		CVI_CHAR img_name[128] = {0, };

		printf("width: %d, height: %d, total_buf_length: %zu\n",
			   stVideoFrame.stVFrame.u32Width,
			   stVideoFrame.stVFrame.u32Height, image_size);

		snprintf(img_name, sizeof(img_name), SD_FATFS_MOUNTPOINT"/sample_ch%d_%d.yuv", chn, loop);

		int fd = aos_open(img_name, O_CREAT | O_RDWR | O_TRUNC);
		if (fd <= 0) {
			printf("aos_open dst file failed\n");
			return CVI_FAILURE;
		}

		u32LumaSize =  stVideoFrame.stVFrame.u32Stride[0] * stVideoFrame.stVFrame.u32Height;
		if (stVideoFrame.stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_422) {
			u32ChromaSize =  stVideoFrame.stVFrame.u32Stride[0] * stVideoFrame.stVFrame.u32Height / 2;
		} else {
			u32ChromaSize =  stVideoFrame.stVFrame.u32Stride[1] * stVideoFrame.stVFrame.u32Height / 2;
		}
		CVI_VI_GetChnCrop(0, chn, &crop_info);
		if (crop_info.bEnable) {
			u32LumaSize = ALIGN((crop_info.stCropRect.u32Width * 8 + 7) >> 3, DEFAULT_ALIGN) *
				ALIGN(crop_info.stCropRect.u32Height, 2);
			u32ChromaSize = (ALIGN(((crop_info.stCropRect.u32Width >> 1) * 8 + 7) >> 3, DEFAULT_ALIGN) *
				ALIGN(crop_info.stCropRect.u32Height, 2)) >> 1;
		}
		vir_addr = (CVI_U8 *)stVideoFrame.stVFrame.u64PhyAddr[0];
		plane_offset = 0;
		for (int i = 0; i < 3; i++) {
			if (stVideoFrame.stVFrame.u32Length[i] != 0) {
				stVideoFrame.stVFrame.pu8VirAddr[i] = vir_addr + plane_offset;
				plane_offset += stVideoFrame.stVFrame.u32Length[i];
				printf("plane(%d): paddr(%ld) vaddr(%p) stride(%d) length(%d)\n",
					   i, stVideoFrame.stVFrame.u64PhyAddr[i],
					   stVideoFrame.stVFrame.pu8VirAddr[i],
					   stVideoFrame.stVFrame.u32Stride[i],
					   stVideoFrame.stVFrame.u32Length[i]);
				aos_write(fd, (CVI_U8 *)stVideoFrame.stVFrame.pu8VirAddr[i],
						(i == 0) ? u32LumaSize : u32ChromaSize);
			}
		}

		if (CVI_VI_ReleaseChnFrame(0, chn, &stVideoFrame) != 0) {
			printf("CVI_VI_ReleaseChnFrame NG\n");
		}

		aos_sync(fd);
		aos_close(fd);
		return CVI_SUCCESS;
	}

	return CVI_FAILURE;
}

static long diff_in_us(struct timespec t1, struct timespec t2)
{
	struct timespec diff;

	if (t2.tv_nsec-t1.tv_nsec < 0) {
		diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
		diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
	} else {
		diff.tv_sec  = t2.tv_sec - t1.tv_sec;
		diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
	}

	return (diff.tv_sec * 1000000.0 + diff.tv_nsec / 1000.0);
}

void dump_vi_frame(int32_t argc, char **argv)
{
	CVI_S32 loop = 0;
	CVI_U32 ok = 0, ng = 0;
	CVI_U8  chn = 0;
	struct timespec start, end;

	if (argc != 3) {
		printf("invailed param\n usage: %s [chn (0/1)] [loop (11111 is infinite)]\n", argv[0]);
		return;
	}

	chn = atoi(argv[1]);
	loop = atoi(argv[2]);
	while (loop > 0) {
		clock_gettime(1, &start);
		if (_vi_get_chn_frame(chn, loop) == CVI_SUCCESS) {
			++ok;
			clock_gettime(1, &end);
			printf("ms consumed: %f\n",
						(CVI_FLOAT)diff_in_us(start, end)/1000);
		} else
			++ng;

		if (loop != 11111)
			loop--;
	}

	printf("VI GetChnFrame OK(%d) NG(%d)\n", ok, ng);
}
ALIOS_CLI_CMD_REGISTER(dump_vi_frame, dump_vi_frame, dump vi frame function);

void dump_vi_raw(int32_t argc, char **argv)
{
	VIDEO_FRAME_INFO_S stVideoFrame[2];
	VI_DUMP_ATTR_S attr;
	struct timeval tv1;
	int frm_num = 1, j = 0, fd = -1;
	CVI_U32 dev = 0, loop = 0;
	struct timespec start, end;
	COMPRESS_MODE_E enMode = COMPRESS_MODE_NONE;

	if (argc != 4) {
		printf("invailed param\nusage: %s [dev (0/1)] [loop (1~60)] [dump_type (0/1/2)]\n", argv[0]);
		return;
	}

	dev = atoi(argv[1]);
	loop = atoi(argv[2]);
	enMode = atoi(argv[3]);

	if (loop > 60) {
		printf("invailed param\n");
		return;
	}

	memset(stVideoFrame, 0, sizeof(stVideoFrame));
	stVideoFrame[0].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;
	stVideoFrame[1].stVFrame.enPixelFormat = PIXEL_FORMAT_RGB_BAYER_12BPP;

	stVideoFrame[0].stVFrame.enCompressMode = enMode;
	stVideoFrame[1].stVFrame.enCompressMode = enMode;

	attr.bEnable = 1;
	attr.u32Depth = 0;
	attr.enDumpType = VI_DUMP_TYPE_RAW;

	CVI_VI_SetPipeDumpAttr(dev, &attr);

	attr.bEnable = 0;
	attr.enDumpType = VI_DUMP_TYPE_IR;

	CVI_VI_GetPipeDumpAttr(dev, &attr);

	while (loop > 0) {
		clock_gettime(CLOCK_MONOTONIC, &start);
		frm_num = 1;

		CVI_VI_GetPipeFrame(dev, stVideoFrame, 1000);

		if (stVideoFrame[1].stVFrame.u64PhyAddr[0] != 0)
			frm_num = 2;

		aos_cli_printf("image size %d\n", stVideoFrame[0].stVFrame.u32Length[0]);
		gettimeofday(&tv1, NULL);

		for (j = 0; j < frm_num; j++) {
			size_t image_size = stVideoFrame[j].stVFrame.u32Length[0];
			unsigned char *ptr = calloc(1, image_size);
			char img_name[128] = {0,}, order_id[8] = {0,};

			if (attr.enDumpType == VI_DUMP_TYPE_RAW) {
				stVideoFrame[j].stVFrame.pu8VirAddr[0] =
							(CVI_U8 *)stVideoFrame[j].stVFrame.u64PhyAddr[0];
				printf("paddr(%ld) vaddr(%p)\n",
							stVideoFrame[j].stVFrame.u64PhyAddr[0],
							stVideoFrame[j].stVFrame.pu8VirAddr[0]);

				memcpy(ptr, (const void *)stVideoFrame[j].stVFrame.pu8VirAddr[0],
					stVideoFrame[j].stVFrame.u32Length[0]);

				switch (stVideoFrame[j].stVFrame.enBayerFormat) {
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
						SD_FATFS_MOUNTPOINT"/vi_%d_%s_%s_w_%d_h_%d_x_%d_y_%d_tv_%ld_%ld.raw",
						dev, (j == 0) ? "LE" : "SE", order_id,
						stVideoFrame[j].stVFrame.u32Width,
						stVideoFrame[j].stVFrame.u32Height,
						stVideoFrame[j].stVFrame.s16OffsetLeft,
						stVideoFrame[j].stVFrame.s16OffsetTop,
						tv1.tv_sec, tv1.tv_usec);

				aos_cli_printf("dump image %s, image size %d\n", img_name, image_size);

				fd = aos_open(img_name, O_CREAT | O_RDWR | O_TRUNC);
				if (fd <= 0) {
					printf("aos_open dst file failed\n");
					return;
				}

				aos_write(fd, ptr, image_size);

				aos_close(fd);
				free(ptr);
			}
		}

		CVI_VI_ReleasePipeFrame(dev, stVideoFrame);

		clock_gettime(CLOCK_MONOTONIC, &end);
		printf("ms consumed: %f\n",
					(CVI_FLOAT)diff_in_us(start, end) / 1000);

		loop--;
	}
}
ALIOS_CLI_CMD_REGISTER(dump_vi_raw, dump_vi_raw, dump vi raw function);

static void dump_register(void *para)
{
	CVI_S32 s32Ret = CVI_SUCCESS;
	VI_PIPE ViPipe = 0;
	char name[64] = {0};
	FILE *fp = NULL;
	VI_DUMP_REGISTER_TABLE_S	reg_tbl;
	ISP_INNER_STATE_INFO_S		*pstInnerStateInfo;

	pstInnerStateInfo = (ISP_INNER_STATE_INFO_S *)malloc(sizeof(ISP_INNER_STATE_INFO_S));
	if (pstInnerStateInfo == NULL) {
		CVI_TRACE_LOG(CVI_DBG_WARN, "malloc %d fail!!!\n", sizeof(ISP_INNER_STATE_INFO_S));
		return;
	}

	if (CVI_ISP_QueryInnerStateInfo(ViPipe, pstInnerStateInfo) != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_ISP_QueryInnerStateInfo fail");
		goto free_innner;
	}

	reg_tbl.MlscGainLut.RGain = pstInnerStateInfo->mlscGainTable.RGain;
	reg_tbl.MlscGainLut.GGain = pstInnerStateInfo->mlscGainTable.GGain;
	reg_tbl.MlscGainLut.BGain = pstInnerStateInfo->mlscGainTable.BGain;

#define DUMP_SIZE 300*1024
	void *json = malloc(DUMP_SIZE);

	if (json == NULL) {
		CVI_TRACE_LOG(CVI_DBG_WARN, "malloc %d fail!!!\n", DUMP_SIZE);
		goto free_innner;
	}

	snprintf(name, 64, "/mnt/sd/vi_dump_register.json");
	fp = fopen(name, "w+");
	if (fp == NULL) {
		CVI_TRACE_LOG(CVI_DBG_WARN, "open %s fail!!!\n", name);
		goto free_json;
	}

	s32Ret = CVI_VI_DumpHwRegisterToFile(ViPipe, json, &reg_tbl);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "CVI_VI_DumpHwRegisterToFile failed with %#x\n", s32Ret);
		goto free_file;
	}

	fwrite(json, 1, DUMP_SIZE, fp);
free_file:
	fclose(fp);
free_json:
	free(json);
free_innner:
	free(pstInnerStateInfo);
	CVI_TRACE_LOG(CVI_DBG_WARN, "Dump registera pass\n");
}

void dump_vi_register(int32_t argc, char **argv)
{
	aos_task_t task;

	if (0 != aos_task_new_ext(&task,
			"test",
			dump_register,
			NULL,
			819200,
			32)) {
		printf("Fail to create task_log_save\n");
	}
}

ALIOS_CLI_CMD_REGISTER(dump_vi_register, dump_vi_register, dump vi register function);

static int call_back_test0(VI_SYNC_TASK_DATA_S *data)
{
	int local_data = data->ViPipe;

	aos_debug_printf("call_back_test0 0x%x\n", local_data);

	return 0;
}

static int call_back_test1(VI_SYNC_TASK_DATA_S *data)
{
	int local_data = data->ViPipe;

	aos_debug_printf("call_back_test1 0x%x\n", local_data);

	return 0;
}

VI_SYNC_TASK_NODE_S g_sync_node[2] = {
	{
		.isp_sync_task_call_back = call_back_test0,
		.data = 0x10,
		.name = "test0",
	},
	{
		.isp_sync_task_call_back = call_back_test1,
		.data = 0x11,
		.name = "test1",
	},
};

void vi_reg_sync_task(int32_t argc, char **argv)
{
	VI_PIPE ViPipe = 0;

	CVI_VI_RegSyncTask(ViPipe, &g_sync_node[0]);
	printf("vi_reg_sync_task ViPipe:%d\n", ViPipe);
}
ALIOS_CLI_CMD_REGISTER(vi_reg_sync_task, test_vi_reg_sync_task, vi register sync task);

void vi_unreg_sync_task(int32_t argc, char **argv)
{
	VI_PIPE ViPipe = 0;

	CVI_VI_UnRegSyncTask(ViPipe, &g_sync_node[0]);
	printf("vi_unreg_sync_task ViPipe:%d\n", ViPipe);
}
ALIOS_CLI_CMD_REGISTER(vi_unreg_sync_task, test_vi_unreg_sync_task, vi unregister sync task);

void smooth_dump_raw(int32_t argc, char **argv)
{
	CVI_S32 s32Ret = CVI_SUCCESS;

	VI_PIPE ViPipe = 0;
	VB_POOL PoolID;
	VB_POOL_CONFIG_S cfg;
	VI_SMOOTH_RAW_DUMP_INFO_S stDumpInfo;
	VIDEO_FRAME_INFO_S stVideoFrame[2];
	VI_DEV_ATTR_S stDevAttr;
	VI_PIPE_ATTR_S stPipeAttr;
	VI_CHN_ATTR_S stChnAttr;
	COMPRESS_MODE_E enMode = COMPRESS_MODE_NONE;
	CVI_U8 BlkCnt, TotalFrameCnt;
	CVI_U64 u64PhyAddr, *phy_addr_list = CVI_NULL;
	VB_BLK vb_blk;
	int frm_num = 1, j = 0;
	struct timeval tv1;
	CVI_U32 dev_frm_w, dev_frm_h, frm_w, frm_h;
	CVI_U32 crop_x = 0, crop_y = 0, crop_w = 0, crop_h = 0;

	if (argc != 5) {
		printf("invailed param\nusage: %s [dev (0/1)] [ringbuf (2~n)] [num (1~60)] [dump_type (0/1/2)]\n",
			argv[0]);
		return;
	}

	ViPipe = atoi(argv[1]);
	BlkCnt = atoi(argv[2]);
	TotalFrameCnt = atoi(argv[3]);
	enMode = atoi(argv[4]);

	CVI_VI_GetDevAttr((VI_DEV)ViPipe, &stDevAttr);
	CVI_VI_GetChnAttr(0, (VI_CHN)ViPipe, &stChnAttr);
	CVI_VI_GetPipeAttr(ViPipe, &stPipeAttr);

	dev_frm_w = stDevAttr.stSize.u32Width;
	dev_frm_h = stDevAttr.stSize.u32Height;
	CVI_TRACE_LOG(CVI_DBG_WARN, "Set smooth rawdump crop or not? (0:no, 1:yes): ");

	frm_w = dev_frm_w;
	frm_h = dev_frm_h;

	frm_num = (stDevAttr.stWDRAttr.enWDRMode == WDR_MODE_2To1_LINE) ? 2 : 1;
	cfg.u32BlkCnt = frm_num * BlkCnt;
	cfg.u32BlkSize = VI_GetRawBufferSize(frm_w, frm_h,
					PIXEL_FORMAT_RGB_BAYER_12BPP,
					enMode ? enMode : stPipeAttr.enCompressMode,
					16,
					(stChnAttr.stSize.u32Width > 2304) ? CVI_TRUE : CVI_FALSE);

	printf("Create VB pool cnt(%d) blksize(0x%x)\n",
			cfg.u32BlkCnt, cfg.u32BlkSize);

	PoolID = CVI_VB_CreatePool(&cfg);
	if (PoolID == VB_INVALID_POOLID) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "create vb pool failed\n");
		s32Ret = CVI_FAILURE;
		return;
	}

	phy_addr_list = malloc(sizeof(*phy_addr_list) * cfg.u32BlkCnt);
	if (phy_addr_list == CVI_NULL) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "malloc phy_addr_list failed\n");
		s32Ret = CVI_FAILURE;
		return;
	}

	for (CVI_U32 i = 0; i < cfg.u32BlkCnt; i++) {
		vb_blk = CVI_VB_GetBlock(PoolID, cfg.u32BlkSize);
		if (vb_blk == VB_INVALID_HANDLE) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "get VB blk failed\n");
			s32Ret = CVI_FAILURE;
			return;
		}
		u64PhyAddr = CVI_VB_Handle2PhysAddr(vb_blk);
		*(phy_addr_list + i) = u64PhyAddr;
		printf("i=%d, vb_blk=%"PRIx64", addr(0x%"PRIx64"), phy_addr(0x%"PRIx64")\n",
					i, (intmax_t)vb_blk, u64PhyAddr, *(phy_addr_list + i));
	}

	memset(&stDumpInfo, 0, sizeof(stDumpInfo));
	stDumpInfo.ViPipe = ViPipe;
	stDumpInfo.u8BlkCnt = BlkCnt;
	stDumpInfo.phy_addr_list = phy_addr_list;
	// set rawdump crop info in stDumpInfo
	stDumpInfo.stCropRect.s32X = crop_x;
	stDumpInfo.stCropRect.s32Y = crop_y;
	stDumpInfo.stCropRect.u32Width = crop_w;
	stDumpInfo.stCropRect.u32Height = crop_h;
	stDumpInfo.enDumpRaw = enMode;

	s32Ret = CVI_VI_StartSmoothRawDump(&stDumpInfo);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "start failed\n");
		s32Ret = CVI_FAILURE;
		return;
	}

	for (int i = 0; i < TotalFrameCnt; i++) {
		memset(stVideoFrame, 0, sizeof(stVideoFrame));
		s32Ret = CVI_VI_GetSmoothRawDump(ViPipe, stVideoFrame, 1000);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "[%d] get frame failed\n", i);
			continue;
		}

		printf("[%d] get roi frame addr(0x%"PRIx64") length(%d), number(%d)\n", i,
				stVideoFrame[0].stVFrame.u64PhyAddr[0],
				stVideoFrame[0].stVFrame.u32Length[0],
				stVideoFrame[0].stVFrame.u32TimeRef);

		if (stVideoFrame[1].stVFrame.u64PhyAddr[0] != 0) {
			printf("[%d] get roi frame addr(0x%"PRIx64") length(%d) number(%d)\n", i,
					stVideoFrame[1].stVFrame.u64PhyAddr[0],
					stVideoFrame[1].stVFrame.u32Length[0],
					stVideoFrame[1].stVFrame.u32TimeRef);
		}

		s32Ret = CVI_VI_PutSmoothRawDump(ViPipe, stVideoFrame);
		if (s32Ret != CVI_SUCCESS) {
			CVI_TRACE_LOG(CVI_DBG_ERR, "[%d] release frame failed\n", i);
			continue;
		}
	}

	s32Ret = CVI_VI_StopSmoothRawDump(&stDumpInfo);
	if (s32Ret != CVI_SUCCESS) {
		CVI_TRACE_LOG(CVI_DBG_ERR, "stop failed\n");
		s32Ret = CVI_FAILURE;
		return;
	}

	// save raw file
	gettimeofday(&tv1, NULL);

	for (j = 0; j < frm_num; j++) {
		size_t image_size = stVideoFrame[j].stVFrame.u32Length[0];
		unsigned char *ptr = calloc(1, image_size);
		FILE *output;
		char img_name[128] = {0,}, order_id[8] = {0,};

		CVI_TRACE_LOG(CVI_DBG_WARN, "paddr(%#"PRIx64") vaddr(%p)\n",
					stVideoFrame[j].stVFrame.u64PhyAddr[0],
					stVideoFrame[j].stVFrame.pu8VirAddr[0]);

		memcpy(ptr, (const void *)stVideoFrame[j].stVFrame.u64PhyAddr[0],
			stVideoFrame[j].stVFrame.u32Length[0]);

		switch (stVideoFrame[j].stVFrame.enBayerFormat) {
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
				"/mnt/sd/vi_%d_compress_mode_%d_%s_%s_w_%d_h_%d_x_%d_y_%d_tv_%ld_%ld.raw",
				ViPipe, enMode, (j == 0) ? "LE" : "SE", order_id,
				stVideoFrame[j].stVFrame.u32Width,
				stVideoFrame[j].stVFrame.u32Height,
				stVideoFrame[j].stVFrame.s16OffsetLeft,
				stVideoFrame[j].stVFrame.s16OffsetTop,
				tv1.tv_sec, tv1.tv_usec);

		CVI_TRACE_LOG(CVI_DBG_WARN, "dump image %s\n", img_name);

		output = fopen(img_name, "wb");

		fwrite(ptr, image_size, 1, output);
		fclose(output);
		free(ptr);
	}

	for (CVI_U32 i = 0; i < cfg.u32BlkCnt; i++) {
		u64PhyAddr = *(phy_addr_list + i);
		vb_blk = CVI_VB_PhysAddr2Handle(u64PhyAddr);
		if (vb_blk != VB_INVALID_HANDLE) {
			CVI_VB_ReleaseBlock(vb_blk);
		}
	}

	if (phy_addr_list != CVI_NULL) {
		free(phy_addr_list);
		phy_addr_list = CVI_NULL;
	}
	s32Ret = CVI_VB_DestroyPool(PoolID);

}
ALIOS_CLI_CMD_REGISTER(smooth_dump_raw, smooth_dump_raw, vi smooth_dump_raw);

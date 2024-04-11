#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <inttypes.h>
#include <aos/cli.h>
#include "cvi_base.h"
// #include "sample_comm.h"
#include "base_ctx.h"
#include "fatfs_vfs.h"
#include "vfs.h"

#include "cvi_ive.h"

#define IVE_FILENAME_IN                SD_FATFS_MOUNTPOINT"/data/00_352x288_y.yuv"
#define IVE_FILENAME_OUT1              SD_FATFS_MOUNTPOINT"/data/sample_Filter_Y3x3.yuv"
#define IVE_FILENAME_OUT2              SD_FATFS_MOUNTPOINT"/data/sample_Filter_Y5x5.yuv"
#define IVE_FILENAME_TEST_READ         SD_FATFS_MOUNTPOINT"/data/test_read_00_352x288_y.yuv"
#define IVE_FILENAME_IN_64             SD_FATFS_MOUNTPOINT"/data/00_64x64_y.yuv"
#define IVE_FILENAME_OUT_64            SD_FATFS_MOUNTPOINT"/data/test_00_64x64_y.yuv"



CVI_CHAR szInFileName[128];
CVI_CHAR szOutFileName[128];
CVI_CHAR szOutFileName2[128];


void test_ive_filter(int argc, char **argv)
{

	// 00_352x288_y.yuv  00_352x288_SP420.yuv  00_352x288_SP422.yuv
	//const char *filename = argv[3];

	int ret = CVI_SUCCESS;
	int input_w, input_h;
	//unsigned long elapsed_cpu;
	//struct timeval t0, t1;

	snprintf(szInFileName, 128, IVE_FILENAME_IN);
	input_w = 352;
	input_h = 288;
	//snprintf(szInFileName, 128, IVE_FILENAME_IN_64);
	//input_w = 64;
	//input_h = 64;
	//gettimeofday(&t0, NULL);

	if (access("/mnt/sd", F_OK) != 0) {
		printf("SD card no exist\n");
		return ;
	}

	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	if (handle == NULL) {
		printf("open ive device failed\n");
		ret = -1;
	}
	ret = ret;
	// Create src image.
	IVE_IMAGE_S src;

	CVI_IVE_ReadRawImage(handle, &src, szInFileName, IVE_IMAGE_TYPE_U8C1, input_w, input_h);

	//snprintf(szOutFileName, 128, IVE_FILENAME_TEST_READ);

	//CVI_IVE_WriteImg(handle, szOutFileName, &src);
#if 1

	CVI_S8 arr3by3[25] = {
		0, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0, 2, 4,
		2, 0, 0, 1, 2, 1, 0, 0, 0, 0, 0, 0,
	};


	// Create dst image.
	IVE_DST_IMAGE_S dst_y;

	CVI_IVE_CreateImage(handle, &dst_y, IVE_IMAGE_TYPE_U8C1, input_w,
			    input_h);

	// Config Setting.
	IVE_FILTER_CTRL_S iveFltCtrl;

	memcpy(iveFltCtrl.as8Mask, arr3by3, 5 * 5 * sizeof(CVI_S8));
	iveFltCtrl.u8Norm = 4;

	// Run HW IVE.
	printf("Run HW IVE Filter 3x3 U8C1.\n");
	ret |= CVI_IVE_Filter(handle, &src, &dst_y, &iveFltCtrl, 0);
	//gettimeofday(&t1, NULL);
	//elapsed_cpu = ((t1.tv_sec - t0.tv_sec) * 1000000 + t1.tv_usec - t0.tv_usec);
	//printf("%s CPU time %lu\n", __func__, elapsed_cpu);
	snprintf(szOutFileName, 128, IVE_FILENAME_OUT1);
	//snprintf(szOutFileName, 128, IVE_FILENAME_OUT_64);

	CVI_IVE_WriteImg(handle, szOutFileName, &dst_y);

#if 0
	CVI_S8 arr5by5[25] = {
		1, 2, 3, 2, 1, 2, 5, 6, 5, 2, 3, 6, 8,
		6, 3, 2, 5, 6, 5, 2, 1, 2, 3, 2, 1,
	};

	memcpy(iveFltCtrl.as8Mask, arr5by5, 5 * 5 * sizeof(CVI_S8));
	iveFltCtrl.u8Norm = 7;

	printf("Run HW IVE Filter 5x5 U8C1.\n");
	ret |= CVI_IVE_Filter(handle, &src, &dst_y, &iveFltCtrl, 0);


	snprintf(szOutFileName2, 128, IVE_FILENAME_OUT2);
	CVI_IVE_WriteImg(handle, szOutFileName2, &dst_y);
#endif
	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeI(handle, &dst_y);
	CVI_IVE_DestroyHandle(handle);

	if (ret == -1) {
		printf("ive fliter test fail\n");
	}
#endif

   return ;
}

ALIOS_CLI_CMD_REGISTER(test_ive_filter, ive_filter_test, test_ive);


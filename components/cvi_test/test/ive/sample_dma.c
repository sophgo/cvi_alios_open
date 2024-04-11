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

#define IVE_DMA_FILENAME_REF1			   SD_FATFS_MOUNTPOINT"/data/sample_DMA_Direct.bin"

typedef struct _IVE_FILENAME {
	CVI_CHAR InFileName[128];
	CVI_CHAR RefFileName[128];
	CVI_CHAR szOutFileName[128];
} IVE_FILENAME;

void test_ive_dma(int argc, char **argv)
{
	IVE_FILENAME *fn;
	int ret = CVI_SUCCESS;
	int input_w, input_h;

	printf("1\n");
	input_w = atoi(argv[1]);
	input_h = atoi(argv[2]);

	fn = (IVE_FILENAME * ) malloc (sizeof(IVE_FILENAME));
	memset(fn, 0, sizeof(IVE_FILENAME));


	snprintf(fn->InFileName, 128, argv[3]);
	snprintf(fn->szOutFileName, 128, argv[4]);
	snprintf(fn->RefFileName, 128, IVE_DMA_FILENAME_REF1);


	if (access("/mnt/sd", F_OK) != 0) {
		printf("SD card no exist\n");
		return ;
	}

	IVE_HANDLE handle = CVI_IVE_CreateHandle();
	printf("2\n");
	if (handle == NULL) {
		printf("open ive device failed\n");
		ret = -1;
	}
	ret = ret;
	// Create src image.
	IVE_IMAGE_S src;

	CVI_IVE_ReadRawImage(handle, &src, fn->InFileName, IVE_IMAGE_TYPE_U8C1, input_w,
			  input_h);
	IVE_DATA_S src_data;

	src_data.u64PhyAddr = src.u64PhyAddr[0];
	src_data.u64VirAddr = src.u64VirAddr[0];
	src_data.u32Stride = src.u32Stride[0];
	src_data.u32Width = src.u32Width;
	src_data.u32Height = src.u32Height;

	// Create ref image.
	IVE_DST_DATA_S ref_dir;
	printf("3\n");


	CVI_IVE_ReadData(handle, &ref_dir, fn->RefFileName, src_data.u32Width,
			      src_data.u32Height);

	// Create dst image.
	IVE_DST_DATA_S dst_data;
	printf("4\n");
	CVI_IVE_CreateDataInfo(handle, &dst_data, input_w, input_h);

	// Config Setting.
	IVE_DMA_CTRL_S iveDmaCtrl;

	iveDmaCtrl.enMode = IVE_DMA_MODE_DIRECT_COPY;
	// Run HW IVE.
	printf("5\n");
	printf("Run HW IVE DMA Direct Copy.\n");
	ret |= CVI_IVE_DMA(handle, &src_data, &dst_data, &iveDmaCtrl, 1);
	printf("6\n");
	ret |= CVI_IVE_CompareIveData(&dst_data, &ref_dir);

	CVI_IVE_WriteData(handle, fn->szOutFileName, &dst_data);

	CVI_SYS_FreeI(handle, &src);
	CVI_SYS_FreeD(handle, &dst_data);
	CVI_SYS_FreeD(handle, &ref_dir);
	CVI_IVE_DestroyHandle(handle);
	printf("15\n");


	if (ret == -1) {
		printf("ive dma test fail\n");
	} else {
		printf("ive dma test pass\n");
	}

   return ;
}

ALIOS_CLI_CMD_REGISTER(test_ive_dma, ive_dma_test, test_ive);


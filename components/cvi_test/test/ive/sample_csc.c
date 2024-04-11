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

typedef struct IVE_IMAGE_FILL_U8C3_PLANAR_S {
	CVI_U8 u8Type;
	CVI_U32 u32SrcPhyAddr[3];
	CVI_U32 u32DstPhyAddr[3];
	CVI_U16 u16SrcStride[3];
	CVI_U16 u16DstStride[3];
	CVI_U16 u16SrcWidth;
	CVI_U16 u16SrcHeight;
	CVI_U16 u16DstWidth;
	CVI_U16 u16DstHeight;
	CVI_U16 u16XScale;
	CVI_U16 u16YScale;
} IVE_IMAGE_FILL_U8C3_PLANAR_S;


typedef struct _IVE_FILENAME {
	CVI_CHAR InFileName[128];
	CVI_CHAR RefFileName[128];
	CVI_CHAR szOutFileName[128];

} IVE_FILENAME;

int test_ive_csc(int argc, char **argv)
{
	if (argc != 4) {
		printf("Incorrect loop value. Usage: %s <w> <h> <file_name>\n",
		       argv[0]);
		printf("Example: %s 352 288 data/00_352x288_SP420.yuv\n", argv[0]);
		return CVI_FAILURE;
	}
	// campus_352x288.rgb
	int ret = CVI_SUCCESS;
	int input_w, input_h;
	unsigned long elapsed_cpu;
	struct timeval t0, t1;
	IVE_FILENAME *fn;

	input_w = atoi(argv[1]);
	input_h = atoi(argv[2]);

	fn = (IVE_FILENAME * ) malloc (sizeof(IVE_FILENAME));
	memset(fn, 0, sizeof(IVE_FILENAME));

	snprintf(fn->InFileName, 128, argv[3]);
	snprintf(fn->szOutFileName, 128, "/mnt/sd/csc_out.bin");
	snprintf(fn->RefFileName, 128, "/mnt/sd/data/sample_CSC_YUV2RGB.rgb");

	IVE_DST_IMAGE_S astDst;
	IVE_SRC_IMAGE_S astSrc;


	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	CVI_IVE_ReadRawImage(handle, &astSrc, fn->InFileName,
			  IVE_IMAGE_TYPE_YUV420SP, input_w, input_h);

	// Create dst image.
	CVI_IVE_CreateImage(handle, &astDst, IVE_IMAGE_TYPE_U8C3_PACKAGE,
			    astSrc.u32Width, astSrc.u32Height);

	// Create ref image.
	IVE_IMAGE_S ref_yuv2rgb;

	CVI_IVE_ReadRawImage(handle, &ref_yuv2rgb, fn->RefFileName,
			IVE_IMAGE_TYPE_U8C3_PACKAGE, astSrc.u32Width, astSrc.u32Height);


	// Config Setting.
	IVE_CSC_CTRL_S stCtrl;

	stCtrl.enMode = IVE_CSC_MODE_VIDEO_BT601_YUV2RGB;

	// Run HW IVE.
	printf("Run HW IVE CSC .\n");
	gettimeofday(&t0, NULL);
	CVI_IVE_CSC(handle, &astSrc, &astDst, &stCtrl, 1);
	gettimeofday(&t1, NULL);
	elapsed_cpu =
		((t1.tv_sec - t0.tv_sec) * 1000000 + t1.tv_usec - t0.tv_usec);
	printf("%s CPU time %lu\n", __func__, elapsed_cpu);

	CVI_IVE_WriteImg(handle, fn->szOutFileName, &astDst);
	ret |= CVI_IVE_CompareIveImage(&astDst, &ref_yuv2rgb);


	CVI_SYS_FreeI(handle, &astSrc);
	CVI_SYS_FreeI(handle, &astDst);
	CVI_SYS_FreeI(handle, &ref_yuv2rgb);
	free(fn);
	CVI_IVE_DestroyHandle(handle);



	if (ret == -1) {
		printf("ive csc test fail\n");
	} else {
		printf("ive csc test pass\n");
	}

	return ret;
}


ALIOS_CLI_CMD_REGISTER(test_ive_csc, ive_csc_test, test_ive);
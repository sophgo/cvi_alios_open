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

int test_ive_resize(int argc, char **argv)
{
	if (argc != 4) {
		printf("Incorrect loop value. Usage: %s <w> <h> <file_name>\n",
		       argv[0]);
		printf("Example: %s 352 288 data/campus_352x288.rgb\n", argv[0]);
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
	snprintf(fn->szOutFileName, 128, "/mnt/sd/resize_out.bin");
	snprintf(fn->RefFileName, 128, "/mnt/sd/data/sample_Resize_Bilinear_rgb.rgb");

	IVE_DST_IMAGE_S astDst[1];
	IVE_SRC_IMAGE_S astSrc[1];
	CVI_U16 au16ResizeWidth[1];
	CVI_U16 au16ResizeHeight[1];

	au16ResizeWidth[0] = 440;
	au16ResizeHeight[0] = 360;
	printf("width = %d\n", au16ResizeWidth[0]);
	printf("height = %d\n", au16ResizeHeight[0]);


	IVE_HANDLE handle = CVI_IVE_CreateHandle();

	// Create src image.
	CVI_IVE_ReadRawImage(handle, &astSrc[0], fn->InFileName,
			  IVE_IMAGE_TYPE_U8C3_PLANAR, input_w, input_h);

	// Create dst image.
	CVI_IVE_CreateImage(handle, &astDst[0], IVE_IMAGE_TYPE_U8C3_PLANAR,
			    au16ResizeWidth[0], au16ResizeHeight[0]);

	// Create ref image.
	IVE_IMAGE_S ref_bilinear_rgb;

	CVI_IVE_ReadRawImage(handle, &ref_bilinear_rgb, fn->RefFileName,
			IVE_IMAGE_TYPE_U8C3_PLANAR, au16ResizeWidth[0], au16ResizeHeight[0]);


	// Config Setting.
	IVE_RESIZE_CTRL_S stCtrl;

	stCtrl.u16Num = 1;
	CVI_IVE_CreateMemInfo(handle, &(stCtrl.stMem),
			      stCtrl.u16Num *
				      sizeof(IVE_IMAGE_FILL_U8C3_PLANAR_S) * 2);

	// Run HW IVE.
	printf("Run HW IVE Resize LINEAR.\n");
	stCtrl.enMode = IVE_RESIZE_MODE_LINEAR;
	gettimeofday(&t0, NULL);
	CVI_IVE_Resize(handle, astSrc, astDst, &stCtrl, 1);
	gettimeofday(&t1, NULL);
	elapsed_cpu =
		((t1.tv_sec - t0.tv_sec) * 1000000 + t1.tv_usec - t0.tv_usec);
	printf("%s CPU time %lu\n", __func__, elapsed_cpu);

	CVI_IVE_WriteImg(handle, fn->szOutFileName, &astDst[0]);
	ret |= CVI_IVE_CompareIveImage(&astDst[0], &ref_bilinear_rgb);


	CVI_SYS_FreeI(handle, &astSrc[0]);
	CVI_SYS_FreeI(handle, &astDst[0]);
	CVI_SYS_FreeI(handle, &ref_bilinear_rgb);
	CVI_SYS_FreeM(handle, &(stCtrl.stMem));
	CVI_IVE_DestroyHandle(handle);

	return ret;

	if (ret == -1) {
		printf("ive resize test fail\n");
	} else {
		printf("ive resize test pass\n");
	}
}


ALIOS_CLI_CMD_REGISTER(test_ive_resize, ive_resize_test, test_ive);
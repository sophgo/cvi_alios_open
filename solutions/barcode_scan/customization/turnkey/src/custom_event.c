#include <stdio.h>
#include <unistd.h>
#include "custom_event.h"
#include "cvi_tpu_interface.h"
#include "rtsp_func.h"
#include "gui_display.h"
#include "app_ai.h"
#include "rtsp_func.h"
#include "zbar.h"
#include "cvi_vpss.h"
#include "cvi_sys.h"
#include <aos/kernel.h>
#include "fatfs_vfs.h"
#include "vfs.h"
#include "debug/dbg.h"

pthread_t pstevent_thread;

CVI_VOID *zbar_event_handler(CVI_VOID *data)
{
	VIDEO_FRAME_INFO_S stVideoFrame;
	CVI_S32 s32Ret = CVI_SUCCESS;
	int width = 0, height = 0;
	//int num = 10;
	// int fd;
	// char filename[64];
	zbar_image_scanner_t *scanner = NULL;
	scanner = zbar_image_scanner_create();

	zbar_image_scanner_set_config(scanner, 0, ZBAR_CFG_ENABLE, 1);

	while(1) {

		// get frame and parse
		s32Ret = CVI_VPSS_GetChnFrame(0, 1, &stVideoFrame, 50);
		if (s32Ret != CVI_SUCCESS) {
			printf("CVI_VPSS_GetChnFrame for grp%d chn%d. s32Ret: 0x%x !\r\n"
				, 0, 0, s32Ret);
			continue;
		}
		//aos_debug_printf("stVideoFrame.stVFrame.u32Width:%d-:%d--%d-%d-%d \r\n", stVideoFrame.stVFrame.u32Width, stVideoFrame.stVFrame.u32Height,stVideoFrame.stVFrame.u32Length[0],stVideoFrame.stVFrame.u32Length[1],stVideoFrame.stVFrame.u32Length[2]);
		width = stVideoFrame.stVFrame.u32Width;
		height = stVideoFrame.stVFrame.u32Height;
		// memset(filename, 0, sizeof(filename));
		// sprintf(filename, "/mnt/sd/dump_%d.yuv", num);
		// fd = aos_open(filename, O_CREAT | O_RDWR | O_TRUNC);
		//CVI_U8 *buf = (CVI_U8 *)malloc(stVideoFrame.stVFrame.u32Length[0] + stVideoFrame.stVFrame.u32Length[1] + stVideoFrame.stVFrame.u32Length[2]);

		//if (fd > 0) {
		// 	for (i = 0; i < 3; ++i) {
		// 	u32len = stVideoFrame.stVFrame.u32Stride[i] * stVideoFrame.stVFrame.u32Height;
		// 	if (u32len == 0)
		// 		continue;
		// 	if (i > 0 && ((stVideoFrame.stVFrame.enPixelFormat == PIXEL_FORMAT_YUV_PLANAR_420) ||
		// 		(stVideoFrame.stVFrame.enPixelFormat == PIXEL_FORMAT_NV12) ||
		// 		(stVideoFrame.stVFrame.enPixelFormat == PIXEL_FORMAT_NV21)))
		// 		u32len >>= 1;

		// 	// printf("plane(%d): paddr(%lx) vaddr(%p) stride(%d)\n",
		// 	// 	   i, stVideoFrame.stVFrame.u64PhyAddr[i],
		// 	// 	   stVideoFrame.stVFrame.pu8VirAddr[i],
		// 	// 	   stVideoFrame.stVFrame.u32Stride[i]);
		// 	// printf(" data_len(%d) plane_len(%d)\n",
		// 	// 		  u32len, stVideoFrame.stVFrame.u32Length[i]);
		// 	aos_write(fd, (CVI_U8 *)stVideoFrame.stVFrame.u64PhyAddr[i], u32len);
		// 	//memcpy(buf, (CVI_U8 *)stVideoFrame.stVFrame.u64PhyAddr[i], u32len);
		// 	//u32lenth += u32len;
		// }

		//}

		// if (u32len <= 0) {
		// 	printf("write error\n");
		// }

		/* wrap image data */
		zbar_image_t *image = zbar_image_create();
		zbar_image_set_format(image, *(int*)"Y800");
		zbar_image_set_size(image, width, height);
		zbar_image_set_data(image, (void *)stVideoFrame.stVFrame.u64PhyAddr[0], stVideoFrame.stVFrame.u32Length[0], NULL);

		/* scan the image for barcodes */
		int n = zbar_scan_image(scanner, image);
		// printf("%s %d %d\n", __func__, __LINE__, n);
		n = n;

		/* extract results */
		const zbar_symbol_t *symbol = zbar_image_first_symbol(image);
		if (NULL == symbol) {
			// printf("NULL == symbol\r\n");
			goto exit;
		}
		for(; symbol; symbol = zbar_symbol_next(symbol)) {
			/* do something useful with results */
			zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
			const char *data = zbar_symbol_get_data(symbol);
			if(data != NULL)  {
				printf("decoded %s symbol \"%s\"\n",
					zbar_get_symbol_name(typ), data);
			}
		}
		// printf("%s %d \n", __func__, __LINE__);
		/* clean up */
exit:
		zbar_image_destroy(image);
		// printf("%s %d \n", __func__, __LINE__);
		// finish
		CVI_VPSS_ReleaseChnFrame(0, 1, &stVideoFrame);

		// if (fd) {
		// 	aos_sync(fd);
		// 	aos_close(fd);
		// }
		// free(buf);
		aos_msleep(100);
	}

	zbar_image_scanner_destroy(scanner);
	return (CVI_VOID *)CVI_SUCCESS;
}

int APP_CustomEventStart(void)
{
    printf("%s %d.\n", __FUNCTION__, __LINE__);

    struct sched_param param;
    pthread_attr_t attr;

    param.sched_priority = 28;
    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    pthread_attr_setschedparam(&attr, &param);
    pthread_attr_setstacksize(&attr, 8192);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_create(&pstevent_thread,
            &attr, zbar_event_handler, NULL);
#if (CONFIG_APP_RTSP_SUPPORT == 1)
    cvi_rtsp_init();
#endif
    return 0;
}

#include "csi_cv.h"
#include <stdio.h>

unsigned char psrc[3840*2160*3/2] = {0};

int main(int argc, char **argv)
{
	if(argc < 2) {
		printf("usage : %s + file path\n");
		return -1;
	}

    // load yuv image
    FILE *fp = NULL;
    fp  = fopen(argv[1], "rb");

    if (fp == NULL)
    {
        return 0;
    }

    int width, height;
    sscanf(argv[2], "%d", &width);
    sscanf(argv[3], "%d", &height);

    int len = fread(psrc, 1, width*height*3/2, fp);

    csi_cv_pointf_t csi_cv_sp[3];
    csi_cv_pointf_t csi_cv_dp[3];

    csi_cv_sp[0].x = 0;
    csi_cv_sp[0].y = 0;
    csi_cv_sp[1].x = 0;
    csi_cv_sp[1].y = height;
    csi_cv_sp[2].x = width;
    csi_cv_sp[2].y = 0;

    csi_cv_dp[0].x = width;
    csi_cv_dp[0].y = 0;
    csi_cv_dp[1].x = width;
    csi_cv_dp[1].y = height;
    csi_cv_dp[2].x = 0;
    csi_cv_dp[2].y = 0;

    // get the transform tensor
    csi_cv_tensor_t map_tensor = get_affine_transform(csi_cv_sp, csi_cv_dp, false);

    // create the yuv tensor
    //int y_stride = (float)width / 128 
    csi_cv_tensor_t y_tensor = csi_cv_create_im_tensor(NULL, (csi_cv_void_t *)psrc, CSI_CV_DTYPE_UINT8, 1, width, height, 0, NULL);
    csi_cv_tensor_t u_tensor = csi_cv_create_im_tensor(NULL, (csi_cv_void_t *)(psrc + width * height), CSI_CV_DTYPE_UINT8, 1, width/2, height/2, 0, NULL);
    csi_cv_tensor_t v_tensor = csi_cv_create_im_tensor(NULL, (csi_cv_void_t *)(psrc + width * height * 5 / 4), CSI_CV_DTYPE_UINT8, 1, width/2, height/2, 0, NULL);

    // create the rgb tensor
    csi_cv_tensor_t rgb_tensor = csi_cv_create_im_tensor(NULL, NULL, CSI_CV_DTYPE_UINT8, 3, width, height, 0, NULL);

    // convert yuv to rgb
    int ret = csi_cv_csc_yuv420p_bgr(y_tensor, u_tensor, v_tensor, rgb_tensor, 0, 0);
    if (ret != 0)
    {
        printf("yuv to rgb error.\n");
        return 1;
    }

    // create the result tensor
    csi_cv_tensor_t affine_tensor = csi_cv_create_im_tensor(NULL, NULL, CSI_CV_DTYPE_UINT8, 1, width, height, 0, NULL);

    // affine
    double border_value[4] = {};
    ret = csi_cv_warp_affine(rgb_tensor, affine_tensor, map_tensor, CSI_CV_INTER_LINEAR, CSI_CV_BORDER_CONSTANT, border_value);
    if (ret != 0)
    {
        printf("affine error.\n");
        return 1;
    }

    csi_cv_destroy_tensor(map_tensor);
    csi_cv_destroy_tensor(y_tensor);
    csi_cv_destroy_tensor(u_tensor);
    csi_cv_destroy_tensor(v_tensor);
    csi_cv_destroy_tensor(rgb_tensor);
    csi_cv_destroy_tensor(affine_tensor);

    printf("affine done.\n");
    return 0;
}

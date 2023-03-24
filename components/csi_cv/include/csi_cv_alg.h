#ifndef __CSI_CV_ALG_H__
#define __CSI_CV_ALG_H__

enum csi_cv_border_types {
    CSI_CV_BORDER_CONSTANT      = 0,
    CSI_CV_BORDER_REPLICATE     = 1,
    CSI_CV_BORDER_REFLECT       = 2,
    CSI_CV_BORDER_WRAP          = 3,
    CSI_CV_BORDER_REFLECT_101   = 4,
    CSI_CV_BORDER_TRANSPARENT   = 5,

    CSI_CV_BORDER_REFLECT101    = CSI_CV_BORDER_REFLECT_101,
    CSI_CV_BORDER_DEFAULT       = CSI_CV_BORDER_REFLECT_101,
    CSI_CV_BORDER_ISOLATED      = 16
};

enum csi_cv_threshold_types {
    CSI_CV_THRESH_BINARY        = 0,
    CSI_CV_THRESH_BINARY_INV    = 1,
    CSI_CV_THRESH_TRUNC         = 2,
    CSI_CV_THRESH_TOZERO        = 3,
    CSI_CV_THRESH_TOZERO_INV    = 4,
    CSI_CV_THRESH_MASK          = 7,
    CSI_CV_THRESH_OTSU          = 8,
    CSI_CV_THRESH_TRIANGLE      = 16
};

enum csi_cv_interpolation_flags{
    CSI_CV_INTER_NEAREST        = 0,
    CSI_CV_INTER_LINEAR         = 1,
    CSI_CV_INTER_CUBIC          = 2,
    CSI_CV_INTER_AREA           = 3,
    CSI_CV_INTER_LANCZOS4       = 4,
    CSI_CV_INTER_LINEAR_EXACT   = 5,
    CSI_CV_INTER_MAX            = 7,
    CSI_CV_WARP_FILL_OUTLIERS   = 8,
    CSI_CV_WARP_INVERSE_MAP     = 16
};

enum csi_cv_cmp_enum {
    CSI_CV_CMP_EQ  = 0,
    CSI_CV_CMP_GT  = 1,
    CSI_CV_CMP_GE  = 2,
    CSI_CV_CMP_LT  = 3,
    CSI_CV_CMP_LE  = 4,
    CSI_CV_CMP_NE  = 5,
};

enum csi_cv_fast_type_enum {
    CSI_CV_FAST_TYPE_5_8  = 0,
    CSI_CV_FAST_TYPE_7_12 = 0,
    CSI_CV_FAST_TYPE_9_16 = 0,
};

enum csi_cv_inter_coeff {
    CSI_CV_INTER_REMAP_COEF_BITS    = 15,
    CSI_CV_INTER_REMAP_COEF_SCALE   =(1 << CSI_CV_INTER_REMAP_COEF_BITS)
};

enum csi_cv_interpolation_masks {
       CSI_CV_INTER_BITS        = 5,
       CSI_CV_INTER_BITS2       = CSI_CV_INTER_BITS * 2,
       CSI_CV_INTER_TAB_SIZE    = 1 << CSI_CV_INTER_BITS,
       CSI_CV_INTER_TAB_SIZE2   = CSI_CV_INTER_TAB_SIZE * CSI_CV_INTER_TAB_SIZE
};

enum csi_cv_norm_types {
    CSI_CV_INF                  = 1,
    CSI_CV_L1                   = 2,
    CSI_CV_L2                   = 4,
    CSI_CV_L2SQR                = 5,
    CSI_CV_HAMMING              = 6,
    CSI_CV_HAMMING2             = 7,
    CSI_CV_TYPE_MASK            = 7,
    CSI_CV_RELATIVE             = 8,
    CSI_CV_MINMAX               = 32
};

enum csi_cv_contour_mode {
    CSI_CV_RETR_EXTERNAL            = 0,
    CSI_CV_RETR_LIST                = 1,
    CSI_CV_RETR_CCOMP               = 2,
    CSI_CV_RETR_TREE                = 3,
    CSI_CV_RETR_FLOODFILL           = 4
};

enum csi_cv_contour_method {
    CSI_CV_CHAIN_CODE               = 0,
    CSI_CV_CHAIN_APPROX_NONE        = 1,
    CSI_CV_CHAIN_APPROX_SIMPLE      = 2,
    CSI_CV_CHAIN_APPROX_TC89_L1     = 3,
    CSI_CV_CHAIN_APPROX_TC89_KCOS   = 4,
    CSI_CV_LINK_RUNS = 5
};

enum csi_cv_dft1d_mode {
    CSI_CV_COMPLEX2COMPLEX_1D = 0,
    CSI_CV_REAL2CCS_1D = 1,
    CSI_CV_INV_CCS2REAL_1D = 2,
};

enum csi_cv_dft2d_mode {
    CSI_CV_INVALID_DFT      = 0,
    CSI_CV_REAL2CCS         = 1,
    CSI_CV_REAL2COMPLEX     = 2,
    CSI_CV_COMPLEX2COMPLEX  = 3,
    CSI_CV_INV_CCS2REAL     = 4,
    CSI_CV_INV_COMPLEX2REAL = 5,
    CSI_CV_INV_COMPLEX2COMPLEX = 6,
};

enum csi_cv_dft_dims {
    CSI_CV_INVALID_DIM = 0,
    CSI_CV_ONE_DIM,
    CSI_CV_ONE_DIM_COLWISE,
    CSI_CV_TWO_DIMS
};

enum csi_cv_dft_flag {
    CSI_CV_DFT_INVERSE         = 1,
    CSI_CV_DFT_SCALE           = 2,
    CSI_CV_DFT_ROWS            = 4,
    CSI_CV_DFT_COMPLEX_OUTPUT  = 16,
    CSI_CV_DFT_REAL_OUTPUT     = 32,
    CSI_CV_DFT_TWO_STAGE       = 64,
    CSI_CV_DFT_STAGE_COLS      = 128,
    CSI_CV_DFT_IS_CONTINUOUS   = 512,
    CSI_CV_DFT_IS_INPLACE      = 1024
};

enum csi_cv_temp_method {
    CSI_CV_TM_SQDIFF        =0,
    CSI_CV_TM_SQDIFF_NORMED =1,
    CSI_CV_TM_CCORR         =2,
    CSI_CV_TM_CCORR_NORMED  =3,
    CSI_CV_TM_CCOEFF        =4,
    CSI_CV_TM_CCOEFF_NORMED =5
};

typedef struct csi_cv_keypoint_tag {
    csi_cv_pointf_t   pt;
    csi_cv_f32_t      size;
    csi_cv_f32_t      angle;
    csi_cv_f32_t      response;
    csi_cv_s32_t      octave;
    csi_cv_s32_t      class_id;
} csi_cv_keypoint_t, *pcsi_cv_keypoint_t;

typedef struct csi_cv_kp_group_tag {
    csi_cv_keypoint_t   kp[KP_DEFAULT_NUM];
    csi_cv_void_t       *pnext_kp_group;
} csi_cv_kp_group_t, *pcsi_cv_kp_group_t;

typedef void *                      csi_cv_dft1d_t;
typedef void *                      csi_cv_dft2d_t;
typedef void (*dft_func)(const csi_cv_dft1d_t tag, const void* src, void* dst);

typedef struct _dft1d_handle_tag {
    csi_cv_s32_t nf;
    csi_cv_s32_t *factors;
    csi_cv_s32_t factor_tab[34];
    csi_cv_f64_t scale;
    csi_cv_s32_t *itab;
    void *wave;
    csi_cv_s32_t tab_size;
    csi_cv_s32_t n;

    csi_cv_s32_t mode;
    bool inverse;
    bool no_permute;
    bool is_complex;
    bool need_buf;
    dft_func dfunc;

}dft1d_handle_t, *pdft1d_handle_t;

typedef struct _stage_tag {
    csi_cv_s32_t value[2];
    csi_cv_s32_t size;
} stage_tag, *pstage_tag;

typedef struct _dft2d_handle_tag {
    bool need_bufa;
    bool need_bufb;
    bool inverse;

    csi_cv_s32_t width;
    csi_cv_s32_t height;
    csi_cv_s32_t mode;
    csi_cv_s32_t dtype;
    csi_cv_s32_t src_channels;
    csi_cv_s32_t dst_channels;
    csi_cv_s32_t nonzero_rows;
    csi_cv_s32_t elem_size;
    csi_cv_s32_t complex_elem_size;

    bool real_transform;
    bool row_transform;
    bool is_scaled;
    csi_cv_s32_t dim;
    stage_tag stage;
    pdft1d_handle_t dft1d_a;
    pdft1d_handle_t dft1d_b;
    csi_cv_u8_t *buffa;
    csi_cv_u8_t *buffb;
    csi_cv_u8_t *buff0;
    csi_cv_u8_t *buff1;

}dft2d_handle_t, *pdft2d_handle_t;

#endif
#ifndef CVIKERNEL_H
#define CVIKERNEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
 * Type Definition
 */
typedef enum CVIKERNEL_FMT_E {
  CVK_FMT_F32 = 0,
  CVK_FMT_F16,
  CVK_FMT_I32,
  CVK_FMT_I16,
  CVK_FMT_I8,
  CVK_FMT_I4,
  CVK_FMT_I2,
  CVK_FMT_I1,
  CVK_FMT_U32,
  CVK_FMT_U16,
  CVK_FMT_U8,
  CVK_FMT_BF16,
  CVK_FMT_INVALID
} cvk_fmt_t;


/*
 *  CVI TPU Chip Name Definition
 */
#define CVI_TPU_VERSION_183X  "cv183x"
#define CVI_TPU_VERSION_182X  "cv182x"
#define CVI_TPU_VERSION_MARS  "cv181x"

/*
 * System information
 */
typedef enum CVIKERNEL_HW_FEATURE_E {
  CVK_HWF_NONE            = 0,
  CVK_HWF_FC_OP1_CONST    = 1,        // FC op1 const
  CVK_HWF_8B_ADD_SUB      = 1 << 1,   // 8b add/sub
  CVK_HWF_MIN_POOL        = 1 << 2,   // Min pooling
  CVK_HWF_M_BRADCAST      = 1 << 3,   // Multi broadcast
  CVK_HWF_QM_LSHIFT       = 1 << 4,   // Left shift of quan_m op
  CVK_HWF_GE              = 1 << 5,   // Greater than or equal to
  CVK_HWF_CMD_PRE_EXE     = 1 << 6    // Command pre-execute
} cvk_hw_feature_t;


typedef struct cvikernel_chip_info {
  uint32_t version;
  uint32_t node_num;
  uint32_t node_shift;
  uint32_t npu_num;
  uint32_t npu_shift;
  uint32_t eu_num;
  uint32_t eu_shift;
  uint32_t lmem_size;
  uint32_t lmem_shift;
  uint32_t lmem_banks;
  uint32_t lmem_bank_size;
  uint64_t lmem_start;
  uint64_t gmem_start;
  uint64_t gmem_size;
  uint64_t features;
} cvk_chip_info_t;

/*
 * Fundamental structures for tensor and matrix
 */
typedef struct cvikernel_matrix_lmem_shape {
  uint32_t n, c, w, col;
} cvk_ml_shape_t;

typedef struct cvikernel_matrix_gmem_shape {
  uint32_t row, col;
} cvk_mg_shape_t;

typedef struct cvikernel_matrix_lmem_stride {
  uint32_t n, c, h;
} cvk_ml_stride_t;

typedef struct cvikernel_matrix_tgmem_stride {
  uint32_t row;
} cvk_mg_stride_t;

typedef struct cvikernel_tensor_lmem_shape {
  uint32_t n, c, h, w;
} cvk_tl_shape_t;

typedef struct cvikernel_tensor_tgmem_shape {
  uint32_t n, c, h, w;
} cvk_tg_shape_t;

typedef struct cvikernel_tensor_lmem_stride {
  uint32_t n, c, h, w;
} cvk_tl_stride_t;

// Even though width stride is not in TDMA configuration,
// The strides of all dimensions is enough to calculate correct position in
// global memory, especially in bf16.
typedef struct cvikernel_tensor_tgmem_stride {
  uint32_t n, c, h, w;
} cvk_tg_stride_t;

typedef struct cvikernel_tensor_lmem {
  uint32_t start_address;
  cvk_fmt_t fmt;
  cvk_fmt_t cmprs_fmt;
  cvk_tl_shape_t shape;
  cvk_tl_stride_t stride;
  uint8_t int8_rnd_mode; // 0 is round to nearset even, 1 is toward zero, currently used by lut
  uint8_t eu_align;
} cvk_tl_t;

typedef struct cvikernel_matrix_lmem {
  uint32_t start_address;
  cvk_fmt_t fmt;
  cvk_ml_shape_t shape;
  cvk_ml_stride_t stride;
  uint8_t int8_rnd_mode; // 0 is round to nearset even, 1 is toward zero
  uint8_t eu_align;
} cvk_ml_t;

typedef struct cvikernel_tensor_gmem {
  uint8_t  base_reg_index;
  uint64_t start_address;
  cvk_fmt_t fmt;
  cvk_tg_shape_t shape;
  cvk_tg_stride_t stride;
  uint8_t int8_rnd_mode; // 0 is round to nearset even, 1 is toward zero
} cvk_tg_t;

typedef struct cvikernel_compressed_tensor_gmem {
  cvk_tg_t t;
  uint64_t reserved_size;
  uint8_t bit_length; // deprecated for zero compress
  uint8_t bias0;
  uint8_t bias1;
  int zero_guard_en;
} cvk_cmpr_tg_t;

typedef struct cvikernel_matrix_gmem {
  uint8_t  base_reg_index;
  uint64_t start_address;
  cvk_fmt_t fmt;
  cvk_mg_shape_t shape;
  cvk_mg_stride_t stride;
  uint8_t int8_rnd_mode; // 0 is round to nearset even, 1 is toward zero
} cvk_mg_t;

typedef struct cvikernel_compressed_matrix_gmem {
  cvk_mg_t m;
  uint8_t bias0;
  uint8_t bias1;
  int zero_guard_en;
} cvk_cmpr_mg_t;

/*
 * TDMA Engine APIs: LMEM to LMEM (L2L)
 */
typedef struct {
  uint8_t mv_lut_idx;
  uint8_t mv_lut_base;
  const cvk_tl_t *src;
  const cvk_tl_t *dst;
  uint8_t outstanding;  // Concurrent TDMA LD/ST and TDM L2L
  uint16_t layer_id;
} cvk_tdma_l2l_tensor_copy_param_t;

typedef struct {
  const cvk_tl_t *src;
  const cvk_tl_t *dst;
  int right_shift;
  uint32_t lrn_step;
  uint16_t layer_id;
} cvk_tdma_l2l_tensor_lrn_shift_param_t;

/*
 * TDMA Engine APIs: LMEM to GMEM (L2G)
 */
typedef struct {
  const cvk_tl_t *src;
  const cvk_tg_t *dst;
  uint16_t layer_id;
  uint32_t intra_cmd_paral; // [0]: disable
                            // [1]: enable TDMA/TIU intra-command parallelism
} cvk_tdma_l2g_tensor_copy_param_t;

typedef struct {
  const cvk_tl_t *src;
  const cvk_tg_t *dst;
  uint16_t layer_id;
} cvk_tdma_l2g_tensor_copy_nc_transposed_param_t;

typedef struct {
  const cvk_tl_t *src;
  const cvk_tg_t *dst;
  uint16_t layer_id;
} cvk_tdma_l2g_tensor_copy_cw_transposed_param_t;

typedef struct {
  const cvk_tl_t *src;
  const cvk_cmpr_tg_t *dst;
  uint16_t layer_id;
  uint32_t intra_cmd_paral; // [0]: disable
                            // [1]: enable TDMA/TIU intra-command parallelism
} cvk_tdma_l2g_tensor_copy_compressed_param_t;

typedef struct {
  uint16_t constant;
  const cvk_tg_t *dst;
  uint16_t layer_id;
} cvk_tdma_l2g_tensor_fill_constant_param_t;

typedef struct {
  const cvk_ml_t *src;
  const cvk_mg_t *dst;
  uint16_t layer_id;
} cvk_tdma_l2g_matrix_copy_param_t;

typedef struct {
  uint32_t src_address;
  uint8_t dst_base_reg_index;
  uint64_t dst_address;
  uint32_t bytes;
  uint16_t layer_id;
} cvk_tdma_l2g_general_copy_param_t;

typedef struct {
  uint32_t src_address;
  uint8_t dst_base_reg_index;
  uint64_t dst_address;
  uint32_t src_bytes;
  cvk_fmt_t src_fmt;
  cvk_fmt_t dst_fmt;
  uint16_t layer_id;
} cvk_tdma_l2g_bf16_general_copy_param_t;

/*
 * TDMA Engine APIs: GMEM to LMEM (G2L)
 */
typedef struct {
  const cvk_tg_t *src;
  const cvk_tl_t *dst;
  uint16_t layer_id;
  uint32_t intra_cmd_paral; // [0]: disable
                            // [1]: enable TDMA/TIU intra-command parallelism
} cvk_tdma_g2l_tensor_copy_param_t;

typedef struct {
  const cvk_tg_t *src;
  const cvk_tl_t *dst;
  uint16_t layer_id;
} cvk_tdma_g2l_tensor_copy_nc_transposed_param_t;

typedef struct {
  const cvk_tg_t *src;
  const cvk_tl_t *dst;
  uint16_t layer_id;
} cvk_tdma_g2l_tensor_copy_chw_rotated_param_t;

typedef struct {
  const cvk_cmpr_tg_t *src;
  const cvk_tl_t *dst;
  uint16_t layer_id;
  uint32_t intra_cmd_paral; // [0]: disable
                            // [1]: enable TDMA/TIU intra-command parallelism
} cvk_tdma_g2l_tensor_copy_decompressed_param_t;

typedef struct {
  uint16_t constant;
  const cvk_tl_t *dst;
  uint16_t layer_id;
} cvk_tdma_g2l_tensor_fill_constant_param_t;

typedef struct {
  const cvk_cmpr_mg_t *src;
  const cvk_ml_t *dst;
  uint16_t layer_id;
} cvk_tdma_g2l_matrix_copy_decompressed_param_t;

typedef struct {
  const cvk_ml_t *src;
  const cvk_cmpr_mg_t *dst;
  uint16_t layer_id;
} cvk_tdma_l2g_matrix_copy_compressed_param_t;

typedef struct {
  const cvk_mg_t *src;
  const cvk_ml_t *dst;
  uint16_t layer_id;
} cvk_tdma_g2l_matrix_copy_param_t;

typedef struct {
  const cvk_mg_t *src;
  const cvk_ml_t *dst;
  uint16_t layer_id;
} cvk_tdma_g2l_matrix_copy_row_col_transposed_param_t;

typedef struct {
  uint8_t src_base_reg_index;
  uint64_t src_address;
  uint32_t dst_address;
  uint32_t bytes;
  uint16_t layer_id;
} cvk_tdma_g2l_general_copy_param_t;

typedef struct {
  uint8_t src_base_reg_index;
  uint64_t src_address;
  uint32_t dst_address;
  uint32_t src_bytes;
  cvk_fmt_t src_fmt;
  cvk_fmt_t dst_fmt;
  uint16_t layer_id;
} cvk_tdma_g2l_bf16_general_copy_param_t;

/*
 * TDMA Engine APIs: GEM to GEM (G2G)
 */
typedef struct {
  const cvk_tg_t *src;
  const cvk_tg_t *dst;
  uint16_t layer_id;
} cvk_tdma_g2g_tensor_copy_param_t;

/*
 * TIU Engine APIs
 *
 * General rules for tensor arithmetic APIs:
 *
 * 1, All tensors can be either signed or unsigned
 *    if not mentioned otherwise.
 * 2, A tensor @x with both @x_high and @x_low as
 *    parameters can optionally be 8-bit (when @x_high
 *    is NULL) or 16-bit (otherwise).
 */
typedef struct {
  const cvk_tl_t *res_high;
  const cvk_tl_t *res_low;
  const cvk_tl_t *a;
  int b_is_const;
  union {
    const cvk_tl_t *b;
    struct {
      int16_t val;
      int is_signed;
    } b_const;
  };
  uint8_t rshift_bits;
  int relu_enable;
  uint16_t layer_id;
} cvk_tiu_mul_param_t;

// Multiplier in quantization down
typedef struct {
  const cvk_tl_t *res_high;
  const cvk_tl_t *res_low;
  const cvk_tl_t *a;
  int b_is_const;
  union {
    const cvk_tl_t *b;
    struct {
      int8_t val;
      int is_signed;
    } b_const;
  };
  uint8_t rshift_bits;
  int relu_enable;
  uint32_t multiplier;
  uint16_t layer_id;
} cvk_tiu_mul_qm_param_t;

/*
 * @res = @a * @b + @res
 *
 * 1, @res_high must not be NULL since input @res must be 16-bit.
 * 2, If output @res is 8-bit (@res_is_int8 == 1), only @res_low
 *    is used as output tensor.
 */
typedef struct {
  const cvk_tl_t *res_high;
  const cvk_tl_t *res_low;
  const cvk_tl_t *a;
  int b_is_const;
  union {
    const cvk_tl_t *b;
    struct {
      int16_t val;
      int is_signed;
    } b_const;
  };
  int res_is_int8;
  int relu_enable;
  uint8_t lshift_bits;
  uint8_t rshift_bits;
  uint16_t layer_id;
} cvk_tiu_mac_param_t;

/*
 * @a and @b must all be 16-bit.
 */
typedef struct {
  const cvk_tl_t *res_high;
  const cvk_tl_t *res_low;
  const cvk_tl_t *a_high;
  const cvk_tl_t *a_low;
  int b_is_const;
  union {
    struct {
      const cvk_tl_t *high;
      const cvk_tl_t *low;
    } b;
    struct {
      int16_t val;
      int is_signed;
    } b_const;
  };
  uint8_t rshift_bits;
  int relu_enable;
  uint16_t layer_id;
} cvk_tiu_add_param_t;

/*
 * 1, @a and @b must all be 16-bit.
 * 2, @res must be signed.
 */
typedef struct {
  const cvk_tl_t *res_high;
  const cvk_tl_t *res_low;
  const cvk_tl_t *a_high;
  const cvk_tl_t *a_low;
  const cvk_tl_t *b_high;
  const cvk_tl_t *b_low;
  uint8_t rshift_bits;
  uint16_t layer_id;
} cvk_tiu_sub_param_t;

/*
 * @a and @b must both be signed or unsigned.
 */
typedef struct {
  const cvk_tl_t *max;
  const cvk_tl_t *a;
  int b_is_const;
  union {
    const cvk_tl_t *b;
    struct {
      int16_t val;
      int is_signed;
    } b_const;
  };
  uint16_t layer_id;
} cvk_tiu_max_param_t;

/*
 * @a and @b must both be signed or unsigned.
 */
typedef struct {
  const cvk_tl_t *min;
  const cvk_tl_t *a;
  int b_is_const;
  union {
    const cvk_tl_t *b;
    struct {
      int16_t val;
      int is_signed;
    } b_const;
  };
  uint16_t layer_id;
} cvk_tiu_min_param_t;

/*
 * @a and @b must both be signed or unsigned.
 */
typedef struct {
  const cvk_tl_t *ge;
  const cvk_tl_t *a;
  int b_is_const;
  union {
    const cvk_tl_t *b;
    struct {
      int16_t val;
      int is_signed;
    } b_const;
  };
  uint16_t layer_id;
} cvk_tiu_ge_param_t;

/*
 * 1, @a must be 16-bit and signed.
 * 2, @res must be 16-bit.
 * 3, @bits must be signed and must range in [-16, 16].
 */
typedef struct {
  const cvk_tl_t *res_high;
  const cvk_tl_t *res_low;
  const cvk_tl_t *a_high;
  const cvk_tl_t *a_low;
  const cvk_tl_t *bits;
  uint16_t layer_id;
} cvk_tiu_arith_shift_param_t;

typedef struct {
  const cvk_tl_t *res;
  const cvk_tl_t *a;
  const cvk_tl_t *b;
  uint16_t layer_id;
} cvk_tiu_and_int8_param_t;

/*
 * All parameters must be 16-bit.
 */
typedef struct {
  const cvk_tl_t *res_high;
  const cvk_tl_t *res_low;
  const cvk_tl_t *a_high;
  const cvk_tl_t *a_low;
  const cvk_tl_t *b_high;
  const cvk_tl_t *b_low;
} cvk_tiu_and_int16_param_t;

typedef struct {
  const cvk_tl_t *res;
  const cvk_tl_t *a;
  const cvk_tl_t *b;
  uint16_t layer_id;
} cvk_tiu_or_int8_param_t;

/*
 * All parameters must be 16-bit.
 */
typedef struct {
  const cvk_tl_t *res_high;
  const cvk_tl_t *res_low;
  const cvk_tl_t *a_high;
  const cvk_tl_t *a_low;
  const cvk_tl_t *b_high;
  const cvk_tl_t *b_low;
} cvk_tiu_or_int16_param_t;

typedef struct {
  const cvk_tl_t *res;
  const cvk_tl_t *a;
  const cvk_tl_t *b;
  uint16_t layer_id;
} cvk_tiu_xor_int8_param_t;

/*
 * All parameters must be 16-bit.
 */
typedef struct {
  const cvk_tl_t *res_high;
  const cvk_tl_t *res_low;
  const cvk_tl_t *a_high;
  const cvk_tl_t *a_low;
  const cvk_tl_t *b_high;
  const cvk_tl_t *b_low;
} cvk_tiu_xor_int16_param_t;

typedef struct {
  const cvk_tl_t *src;
  const cvk_tl_t *dst;
  uint16_t layer_id;
} cvk_tiu_copy_param_t;

/*
 * NOTE:
 *   @table is treated logically as a linear list of
 *   length @table_n, where @table_n is a multiple of
 *   16 and is smaller than or equal to 256.
 *   When stored in local memory, @table is a tensor
 *   of shape (1, npu_num, 1, @table_n), that is, the
 *   data of the linear list should be copied across
 *   each NPU's local memory by user. The behavior when
 *   these copies differ is undefined.
 */
typedef struct {
  const cvk_tl_t *ofmap;
  const cvk_tl_t *ifmap;
  const cvk_tl_t *table;
  uint16_t layer_id;
} cvk_tiu_lookup_table_param_t;

typedef struct {
  const cvk_tl_t *ifmap;
  const cvk_tl_t *buf;
  const cvk_tl_t *tbl_answer;
  const cvk_tl_t *tbl_answer_mantissa;
  const cvk_tl_t *ofmap;
  uint16_t layer_id;
  /*
   * \brief
   * we support 2 method of lut depends on \is_scientific:
   * scientific: \tbl_answer_mantissa as mantissa part
   * interpolation: \tbl_answer_mantissa as slope part
   * e.g:
   * interpolation we use activation function to achieve high accuracy
   * scientific uses to calucate reciprocal or sqrt
   * \is_scientific 1 means set scientific, otherwise is interpolation
   */
  uint8_t is_scientific;
  uint8_t eu_align;
  /*
   * for achieving high accuracy, we quant activation function
   * with is constrained by a pair ofhorizontal asymptotes that x->infinity
   * from [-infinity, infinity] to [\min, \max]
   */
  float min;
  float max;
} cvk_tiu_bf16_lookup_interp_table_param_t;

/*
 * Convolution weight shape:
 *   Calibration output (oc, ic, kh, kw)
 *   bm_build transforms (oc, ic, kh, kw) -> (1, oc, kh*kw, ic)
 *   TDMA load global (1, oc, kh*w, ic) -> local (1, oc, kh*kw, ic)
 *   TIU conv opd1 (ic, oc, kh, kw)
 *
 * Bias (2, oc, 1, 1)
 *   int8: int16, n=0 [7:0], n=1 [15:8]
 *   bf16: fp32, n=0 [31:16], n=1 [15:0]
 */
typedef struct {
  const cvk_tl_t *ofmap;
  const cvk_tl_t *ifmap;
  const cvk_tl_t *weight;
  const cvk_tl_t *bias;
  uint8_t ins_h, ins_last_h;
  uint8_t ins_w, ins_last_w;
  uint8_t pad_top, pad_bottom;
  uint8_t pad_left, pad_right;
  uint8_t stride_h, stride_w;
  uint8_t dilation_h, dilation_w;
  int relu_enable;
  uint8_t rshift_bits;
  uint8_t ps32_mode;
  uint8_t w_is_const;
  uint16_t layer_id;
  uint8_t fp_round_typ;
  uint8_t cmd_pre_exe_typ;  // tiu execute cmd when channel data is ready
                            // wait type:
                            // 0: activation
                            // 1: weight
  uint8_t cmd_pre_exe;      // tiu execute cmd when channel data is ready
                            // 0: disable
                            // 1: load pre exec
                            // 2: store pre exec
                            // 3: load and store pre exec
  int8_t ins_val;   // padding value for int8
  uint16_t ins_fp;  // padding value for bf16
} cvk_tiu_pt_convolution_param_t;

typedef struct {
  const cvk_tl_t *ofmap;
  const cvk_tl_t *ifmap;
  const cvk_tl_t *weight;
  const cvk_tl_t *chl_quan_param;
  uint8_t ins_h, ins_last_h;
  uint8_t ins_w, ins_last_w;
  uint8_t pad_top, pad_bottom;
  uint8_t pad_left, pad_right;
  uint8_t stride_h, stride_w;
  uint8_t dilation_h, dilation_w;
  uint8_t has_bias;
  uint8_t relu_enable;
  uint8_t ps32_mode;
  uint8_t w_is_const;
  uint16_t layer_id;
  uint8_t cmd_pre_exe_typ;  // tiu execute cmd when channel data is ready
                            // wait type:
                            // 0: activation
                            // 1: weight
  uint8_t cmd_pre_exe;      // tiu execute cmd when channel data is ready
                            // 0: disable
                            // 1: load pre exec
                            // 2: store pre exec
                            // 3: load and store pre exec
  int8_t ins_val;   // padding value for int8
  uint16_t ins_fp;  // padding value for bf16
} cvk_tiu_convolution_param_t;

typedef struct {
  const cvk_tl_t *ofmap;
  const cvk_tl_t *ifmap;
  uint16_t kh, kw;
  uint8_t pad_top, pad_bottom;
  uint8_t pad_left, pad_right;
  uint8_t stride_h, stride_w;
  int8_t ins_val;   // padding value for int8
  uint16_t ins_fp;  // padding value for bf16
  uint16_t layer_id;
} cvk_tiu_max_pooling_param_t;

typedef struct {
  const cvk_tl_t *ofmap;
  const cvk_tl_t *ifmap;
  uint16_t kh, kw;
  uint8_t pad_top, pad_bottom;
  uint8_t pad_left, pad_right;
  uint8_t stride_h, stride_w;
  uint16_t ins_fp;
  uint16_t layer_id;
} cvk_tiu_min_pooling_param_t;

typedef struct {
  const cvk_tl_t *ofmap;
  const cvk_tl_t *ifmap;
  uint16_t kh, kw;
  uint8_t ins_h, ins_last_h;
  uint8_t ins_w, ins_last_w;
  uint8_t pad_top, pad_bottom;
  uint8_t pad_left, pad_right;
  uint8_t stride_h, stride_w;
  uint16_t avg_pooling_const;
  uint8_t rshift_bits;
  uint16_t layer_id;
  int8_t ins_val;   // padding value for int8
  uint16_t ins_fp;  // padding value for bf16
} cvk_tiu_average_pooling_param_t;

typedef struct {
  const cvk_tl_t *ofmap;
  const cvk_tl_t *ifmap;
  const cvk_tl_t *weight;
  const cvk_tl_t *bias;
  int weight_is_const;
  struct {
    int16_t val;
    int is_signed;
  } weight_const;
  uint8_t ins_h, ins_last_h;
  uint8_t ins_w, ins_last_w;
  uint8_t dilation_h, dilation_w;
  uint8_t pad_top, pad_bottom;
  uint8_t pad_left, pad_right;
  uint8_t stride_h, stride_w;
  uint8_t rshift_bits;
  int relu_enable;
  uint16_t layer_id;
  uint8_t cmd_pre_exe_typ;  // tiu execute cmd when channel data is ready
                            // wait type:
                            // 0: activation
                            // 1: weight
  uint8_t cmd_pre_exe;      // tiu execute cmd when channel data is ready
                            // 0: disable
                            // 1: load pre exec
                            // 2: store pre exec
                            // 3: load and store pre exec
  uint8_t ps32_mode;  // output fp32 result if ps32_mode == 2
  int8_t ins_val;   // padding value for int8
  uint16_t ins_fp;  // padding value for bf16
} cvk_tiu_depthwise_pt_convolution_param_t;

typedef struct {
  const cvk_tl_t *ofmap;
  const cvk_tl_t *ifmap;
  const cvk_tl_t *weight;
  const cvk_tl_t *chl_quan_param;
  int weight_is_const;
  struct {
    int16_t val;
    int is_signed;
  } weight_const;
  uint8_t ins_h, ins_last_h;
  uint8_t ins_w, ins_last_w;
  uint8_t dilation_h, dilation_w;
  uint8_t pad_top, pad_bottom;
  uint8_t pad_left, pad_right;
  uint8_t stride_h, stride_w;
  uint8_t has_bias;
  uint8_t relu_enable;
  uint16_t layer_id;
  uint8_t cmd_pre_exe_typ;  // tiu execute cmd when channel data is ready
                            // wait type:
                            // 0: activation
                            // 1: weight
  uint8_t cmd_pre_exe;      // tiu execute cmd when channel data is ready
                            // 0: disable
                            // 1: load pre exec
                            // 2: store pre exec
                            // 3: load and store pre exec
  int8_t ins_val;   // padding value for int8
  uint16_t ins_fp;  // padding value for bf16
} cvk_tiu_depthwise_convolution_param_t;

typedef struct {
  const cvk_ml_t *res;
  const cvk_ml_t *left;
  const cvk_ml_t *right;
  const cvk_ml_t *bias;
  uint8_t lshift_bits;
  uint8_t rshift_bits;
  int res_is_int8;
  int relu_enable;
  int add_result;
  uint8_t  ps32_mode;
  uint16_t layer_id;
} cvk_tiu_matrix_multiplication_param_t;

typedef struct {
  const cvk_ml_t *res;
  const cvk_ml_t *left;
  const cvk_ml_t *right;
  const cvk_ml_t *bias;
  uint8_t lshift_bits;
  uint8_t rshift_bits;
  int res_is_int8;
  int relu_enable;
  int add_result;
  uint8_t  ps32_mode;
  int32_t quan_m;
  uint16_t layer_id;
} cvk_tiu_matrix_multiplication_qm_param_t;

/*
 * Kernel operations
 */
struct cvikernel_context;

typedef struct cvikernel_operations {
  void (*cleanup)(struct cvikernel_context *ctx);
  void (*reset)(struct cvikernel_context *ctx);
  uint8_t *(*acquire_cmdbuf)(struct cvikernel_context *ctx, uint32_t *size);
  void (*dmabuf_size)(uint8_t *cmdbuf, uint32_t sz, uint32_t *psize, uint32_t *pmu_size);
  void (*dmabuf_convert)(uint8_t *cmdbuf, uint32_t sz, uint8_t *dmabuf);

  // Concurrent TDMA and TIU command execution:
  //   TDMA command runs without waiting previous TIU command:
  //     1. parallel_disable
  //     2. parallel_enable
  //     3. tiu command
  //     4. tdma command (not wait TIU command)
  //     5. tdma command (not wait TIU command)
  void (*parallel_enable)(struct cvikernel_context *ctx);
  void (*parallel_disable)(struct cvikernel_context *ctx);

  void (*set_layer_id)(
      struct cvikernel_context *ctx,
      uint16_t layer_id);

  cvk_tl_t *(*lmem_alloc_tensor)(
      struct cvikernel_context *ctx,
      cvk_tl_shape_t shape,
      cvk_fmt_t fmt,
      int eu_align);

  cvk_ml_t *(*lmem_alloc_matrix)(
      struct cvikernel_context *ctx,
      cvk_ml_shape_t shape,
      cvk_fmt_t fmt,
      int eu_align);

  cvk_ml_t *(*lmem_alloc_ps32_matrix)(
      struct cvikernel_context *ctx,
      cvk_ml_shape_t shape,
      cvk_fmt_t fmt,
      int eu_align);

  void (*lmem_free_tensor)(
      struct cvikernel_context *ctx,
      const cvk_tl_t *tl);

  void (*lmem_free_matrix)(
      struct cvikernel_context *ctx,
      const cvk_ml_t *ml);

  void (*lmem_init_tensor)(
      struct cvikernel_context *ctx,
      cvk_tl_t *tl,
      cvk_tl_shape_t shape,
      cvk_fmt_t fmt,
      int eu_align);

  void (*lmem_init_matrix)(
      struct cvikernel_context *ctx,
      cvk_ml_t *ml,
      cvk_ml_shape_t shape,
      cvk_fmt_t fmt,
      int eu_align);

  cvk_tl_stride_t (*tl_default_stride)(
      struct cvikernel_context *ctx,
      cvk_tl_shape_t shape,
      cvk_fmt_t fmt,
      int eu_align);

  cvk_tg_stride_t (*tg_default_stride)(
      struct cvikernel_context *ctx,
      cvk_tg_shape_t shape,
      cvk_fmt_t fmt);

  cvk_ml_shape_t (*ml_default_shape)(
      struct cvikernel_context *ctx,
      uint32_t row,
      uint32_t col,
      cvk_fmt_t fmt);

  cvk_ml_stride_t (*ml_default_stride)(
      struct cvikernel_context *ctx,
      cvk_ml_shape_t shape,
      cvk_fmt_t fmt,
      int eu_align);

  cvk_ml_shape_t (*ml_shape_t1)(
      struct cvikernel_context *ctx,
      uint32_t len,
      cvk_fmt_t fmt);

  uint32_t (*lmem_tensor_to_size)(
      struct cvikernel_context *ctx,
      cvk_tl_shape_t shape,
      cvk_fmt_t fmt,
      int eu_align);

  uint32_t (*lmem_matrix_to_size)(
      struct cvikernel_context *ctx,
      cvk_ml_shape_t shape,
      cvk_fmt_t fmt,
      int eu_align);

  uint32_t (*lmem_ps32_matrix_to_size)(
      struct cvikernel_context *ctx,
      cvk_ml_shape_t shape,
      cvk_fmt_t fmt,
      int eu_align);

  void (*gmem_init_tensor)(
      struct cvikernel_context *ctx,
      cvk_tg_t *tg,
      cvk_tg_shape_t shape,
      cvk_fmt_t fmt);

  /* Local to Local DMA API */
  void (*tdma_l2l_tensor_copy)(
      struct cvikernel_context *ctx,
      const cvk_tdma_l2l_tensor_copy_param_t *param);
  void (*tdma_l2l_bf16_tensor_copy)(
      struct cvikernel_context *ctx,
      const cvk_tdma_l2l_tensor_copy_param_t *param);
  void (*tdma_l2l_tensor_lrn_shift)(
      struct cvikernel_context *ctx,
      const cvk_tdma_l2l_tensor_lrn_shift_param_t *param);

  /* Local to Global DMA API */
  void (*tdma_l2g_tensor_copy)(
      struct cvikernel_context *ctx,
      const cvk_tdma_l2g_tensor_copy_param_t *param);
  void (*tdma_l2g_bf16_tensor_copy)(
      struct cvikernel_context *ctx,
      const cvk_tdma_l2g_tensor_copy_param_t *param);
  void (*tdma_l2g_tensor_copy_nc_transposed)(
      struct cvikernel_context *ctx,
      const cvk_tdma_l2g_tensor_copy_nc_transposed_param_t *param);
  void (*tdma_l2g_bf16_tensor_copy_nc_transposed)(
      struct cvikernel_context *ctx,
      const cvk_tdma_l2g_tensor_copy_nc_transposed_param_t *param);
  void (*tdma_l2g_tensor_copy_compressed)(
      struct cvikernel_context *ctx,
      const cvk_tdma_l2g_tensor_copy_compressed_param_t *param);
  void (*tdma_l2g_tensor_fill_constant)(
      struct cvikernel_context *ctx,
      const cvk_tdma_l2g_tensor_fill_constant_param_t *param);
  void (*tdma_l2g_tensor_copy_cw_transposed)(
      struct cvikernel_context *ctx,
      const cvk_tdma_l2g_tensor_copy_cw_transposed_param_t *param);
  void (*tdma_l2g_bf16_tensor_copy_cw_transposed)(
      struct cvikernel_context *ctx,
      const cvk_tdma_l2g_tensor_copy_cw_transposed_param_t *param);
  void (*tdma_l2g_matrix_copy)(
      struct cvikernel_context *ctx,
      const cvk_tdma_l2g_matrix_copy_param_t *param);
  void (*tdma_l2g_bf16_matrix_copy)(
      struct cvikernel_context *ctx,
      const cvk_tdma_l2g_matrix_copy_param_t *param);
  void (*tdma_l2g_general_copy)(
      struct cvikernel_context *ctx,
      const cvk_tdma_l2g_general_copy_param_t *param);
  void (*tdma_l2g_bf16_general_copy)(
      struct cvikernel_context *ctx,
      const cvk_tdma_l2g_bf16_general_copy_param_t *param);

  /* Global to Local DMA API */
  void (*tdma_g2l_tensor_copy)(
      struct cvikernel_context *ctx,
      const cvk_tdma_g2l_tensor_copy_param_t *param);
  void (*tdma_g2l_bf16_tensor_copy)(
      struct cvikernel_context *ctx,
      const cvk_tdma_g2l_tensor_copy_param_t *param);
  void (*tdma_g2l_tensor_copy_nc_transposed)(
      struct cvikernel_context *ctx,
      const cvk_tdma_g2l_tensor_copy_nc_transposed_param_t *param);
  void (*tdma_g2l_bf16_tensor_copy_nc_transposed)(
      struct cvikernel_context *ctx,
      const cvk_tdma_g2l_tensor_copy_nc_transposed_param_t *param);
  void (*tdma_g2l_tensor_copy_chw_rotated)(
      struct cvikernel_context *ctx,
      const cvk_tdma_g2l_tensor_copy_chw_rotated_param_t *param);
  void (*tdma_g2l_tensor_copy_decompressed)(
      struct cvikernel_context *ctx,
      const cvk_tdma_g2l_tensor_copy_decompressed_param_t *param);
  void (*tdma_g2l_tensor_fill_constant)(
      struct cvikernel_context *ctx,
      const cvk_tdma_g2l_tensor_fill_constant_param_t *param);
  void (*tdma_g2l_bf16_tensor_fill_constant)(
      struct cvikernel_context *ctx,
      const cvk_tdma_g2l_tensor_fill_constant_param_t *param);
  void (*tdma_g2l_matrix_copy_decompressed)(
      struct cvikernel_context *ctx,
      const cvk_tdma_g2l_matrix_copy_decompressed_param_t *param);
  void (*tdma_l2g_matrix_copy_compressed)(
      struct cvikernel_context *ctx,
      const cvk_tdma_l2g_matrix_copy_compressed_param_t *param);
  void (*tdma_g2l_matrix_copy)(
      struct cvikernel_context *ctx,
      const cvk_tdma_g2l_matrix_copy_param_t *param);
  void (*tdma_g2l_bf16_matrix_copy)(
      struct cvikernel_context *ctx,
      const cvk_tdma_g2l_matrix_copy_param_t *param);
  void (*tdma_g2l_matrix_copy_row_col_transposed)(
      struct cvikernel_context *ctx,
      const cvk_tdma_g2l_matrix_copy_row_col_transposed_param_t *param);
  void (*tdma_g2l_general_copy)(
      struct cvikernel_context *ctx,
      const cvk_tdma_g2l_general_copy_param_t *param);
  void (*tdma_g2l_bf16_general_copy)(
      struct cvikernel_context *ctx,
      const cvk_tdma_g2l_bf16_general_copy_param_t *param);

  /* Global to Global DMA API */
  void (*tdma_g2g_tensor_copy)(
      struct cvikernel_context *ctx,
      const cvk_tdma_g2g_tensor_copy_param_t *param);
  void (*tdma_g2g_general_copy)(
      struct cvikernel_context *ctx,
      const cvk_tdma_g2g_tensor_copy_param_t *param);
  void (*tdma_g2g_bf16_general_copy)(
      struct cvikernel_context *ctx,
      const cvk_tdma_g2g_tensor_copy_param_t *param);
  void (*tdma_g2g_bf16_tensor_copy)(
      struct cvikernel_context *ctx,
      const cvk_tdma_g2g_tensor_copy_param_t *param);

  /* TIU API */
  void (*tiu_mul)(
      struct cvikernel_context *ctx,
      const cvk_tiu_mul_param_t *param);
  void (*tiu_mul_qm)(
      struct cvikernel_context *ctx,
      const cvk_tiu_mul_qm_param_t *param);
  void (*tiu_mac)(
      struct cvikernel_context *ctx,
      const cvk_tiu_mac_param_t *param);
  void (*tiu_add)(
      struct cvikernel_context *ctx,
      const cvk_tiu_add_param_t *param);
  void (*tiu_sub)(
      struct cvikernel_context *ctx,
      const cvk_tiu_sub_param_t *param);
  void (*tiu_max)(
      struct cvikernel_context *ctx,
      const cvk_tiu_max_param_t *param);
  void (*tiu_min)(
      struct cvikernel_context *ctx,
      const cvk_tiu_min_param_t *param);
  void (*tiu_and_int8)(
      struct cvikernel_context *ctx,
      const cvk_tiu_and_int8_param_t *param);
  void (*tiu_arith_shift)(
      struct cvikernel_context *ctx,
      const cvk_tiu_arith_shift_param_t *param);
  void (*tiu_and_int16)(
      struct cvikernel_context *ctx,
      const cvk_tiu_and_int16_param_t *param);
  void (*tiu_or_int8)(
      struct cvikernel_context *ctx,
      const cvk_tiu_or_int8_param_t *param);
  void (*tiu_or_int16)(
      struct cvikernel_context *ctx,
      const cvk_tiu_or_int16_param_t *param);
  void (*tiu_xor_int8)(
      struct cvikernel_context *ctx,
      const cvk_tiu_xor_int8_param_t *param);
  void (*tiu_xor_int16)(
      struct cvikernel_context *ctx,
      const cvk_tiu_xor_int16_param_t *param);
  void (*tiu_copy)(
      struct cvikernel_context *ctx,
      const cvk_tiu_copy_param_t *param);
  void (*tiu_lookup_table)(
      struct cvikernel_context *ctx,
      const cvk_tiu_lookup_table_param_t *param);
  void (*tiu_bf16_lookup_interp_table)(
      struct cvikernel_context *ctx,
      const cvk_tiu_bf16_lookup_interp_table_param_t *param);
  void (*tiu_pt_convolution)(
      struct cvikernel_context *ctx,
      const cvk_tiu_pt_convolution_param_t *param);
  void (*tiu_convolution)(
      struct cvikernel_context *ctx,
      const cvk_tiu_convolution_param_t *param);
  void (*tiu_max_pooling)(
      struct cvikernel_context *ctx,
      const cvk_tiu_max_pooling_param_t *param);
  void (*tiu_average_pooling)(
      struct cvikernel_context *ctx,
      const cvk_tiu_average_pooling_param_t *param);
  void (*tiu_pt_depthwise_convolution)(
      struct cvikernel_context *ctx,
      const cvk_tiu_depthwise_pt_convolution_param_t *param);
  void (*tiu_depthwise_convolution)(
      struct cvikernel_context *ctx,
      const cvk_tiu_depthwise_convolution_param_t *param);
  void (*tiu_matrix_multiplication)(
      struct cvikernel_context *ctx,
      const cvk_tiu_matrix_multiplication_param_t *param);
  void (*tiu_matrix_multiplication_qm)(
      struct cvikernel_context *ctx,
      const cvk_tiu_matrix_multiplication_qm_param_t *param);
  void (*tiu_ge)(
      struct cvikernel_context *ctx,
      const cvk_tiu_ge_param_t *param);
  void (*tiu_min_pooling)(
      struct cvikernel_context *ctx,
      const cvk_tiu_min_pooling_param_t *param);
} cvk_operations_t;

/*
 * Miscellaneous helper function
 *   Not directly related to tiu/tdma operation
 *   or not ready to move into official kernel operation yet.
 */
typedef struct {
  uint16_t (*float_to_bfloat16) (
      struct cvikernel_context *ctx,
      float data);
  void (*bf16_table_shape)(
      struct cvikernel_context *ctx,
      cvk_tl_shape_t *shape);
} cvk_misc_operations_t;

/*
 * Kernel Context
 */
typedef struct cvikernel_context {
  cvk_chip_info_t info;
  cvk_operations_t *ops;
  cvk_misc_operations_t *misc_ops;
  void *priv_data;
} cvk_context_t;

/*
 * Register information
 */
typedef struct cvikernel_register_info {
  char chip_ver_str[16];
  uint32_t cmdbuf_size;
  uint8_t *cmdbuf;
} cvk_reg_info_t;

cvk_context_t *cvikernel_register(cvk_reg_info_t *req_info);

#ifdef __cplusplus
}
#endif

#endif /* CVIKERNEL_H */

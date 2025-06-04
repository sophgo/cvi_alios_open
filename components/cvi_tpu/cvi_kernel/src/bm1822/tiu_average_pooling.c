#include "kernel_1822.h"
#include <bmkernel/bm1822/1822_fp_convert.h>

bmk1822_op_t * bmk1822_tiu_average_pooling(
    ctx_t *ctx,
    const bmk1822_tiu_average_pooling_param_t *p)
{
  int bf16_enable = (p->ifmap->fmt == FMT_BF16) ? 1 : 0;

  ASSERT(p->ifmap->shape.n == p->ofmap->shape.n);
  ASSERT(p->ifmap->shape.c == p->ofmap->shape.c);
  ASSERT(p->stride_h < 32 && p->stride_h > 0);
  ASSERT(p->stride_w < 32 &&  p->stride_w > 0);
  ASSERT(p->pad_top < 16);
  ASSERT(p->pad_bottom < 16);
  ASSERT(p->pad_left < 16);
  ASSERT(p->pad_right < 16);
  ASSERT(p->ins_h < 15);
  ASSERT(p->ins_last_h < 15);
  ASSERT(p->ins_w < 15);
  ASSERT(p->ins_last_w < 15);

  check_tiu_tensor_2(p->ifmap, p->ofmap);
  if (bf16_enable) {
    assert_bf16_stride_type_0(ctx, p->ifmap);
    assert_bf16_stride_type_0(ctx, p->ofmap);
  } else {
    assert_stride_type_0(ctx, p->ifmap);
    assert_stride_type_0(ctx, p->ofmap);
  }

  int opd0_sign = tensor_is_signed(p->ifmap);

  tiu_reg_t reg;
  reset_tiu_reg(&reg);
  reg.cmd_en = 1;
  reg.tsk_typ = DCR_TYPE_DEPTHWISE_POOL_FIX8B;
  reg.tsk_eu_typ = 1;
  reg.opt_shift_typ = opd0_sign;
  reg.opt_res_shift = p->rshift_bits;
  reg.opt_relu_typ = 0; /* hardware relu function not verified. */
  reg.tsk_opd_num = 2;
  reg.opd_typ = bf16_enable ? 1: 0;

  reg.res0_addr = p->ofmap->start_address;
  reg.opt_res0_sign = opd0_sign;
  reg.opt_res0_seg = 1;
  reg.res0_n = p->ofmap->shape.n;
  reg.res0_c = p->ofmap->shape.c;
  reg.res0_h = p->ofmap->shape.h;
  reg.res0_w = p->ofmap->shape.w;

  reg.opd0_addr = p->ifmap->start_address;
  reg.opt_opd0_sign = opd0_sign;
  reg.opt_opd0_seg = 1;
  reg.opd0_n = p->ifmap->shape.n;
  reg.opd0_c = p->ifmap->shape.c;
  reg.opd0_h = p->ifmap->shape.h;
  reg.opd0_w = p->ifmap->shape.w;
  reg.conv_opd0_up_pad = p->pad_top;
  reg.conv_opd0_dn_pad = p->pad_bottom;
  reg.conv_opd0_lf_pad = p->pad_left;
  reg.conv_opd0_rt_pad = p->pad_right;
  reg.conv_opd0_x_ins0 = p->ins_w;
  reg.conv_opd0_y_ins0 = p->ins_h;
  reg.conv_opd0_x_ins0_last = p->ins_last_w;
  reg.conv_opd0_y_ins0_last = p->ins_last_h;

  reg.opt_opd1_const = 1;
  /*HW does not have dive, we need to calculate value here*/
  if (bf16_enable)
    reg.opd1_addr =
        convert_fp32_bf16(
            (float)(convert_bf16_fp32(p->avg_pooling_const) / (p->kh * p->kw)));
  else
    reg.opd1_addr = p->avg_pooling_const;

  reg.opd1_h = p->kh;
  reg.opd1_w = p->kw;
  reg.opt_opd1_sign = 0;
  reg.opt_opd1_seg = 1;
  reg.conv_op_x_str = p->stride_w;
  reg.conv_op_y_str = p->stride_h;
  reg.opd0_ins_val = bf16_enable ?
                     (uint32_t)p->ins_fp : (uint32_t)p->ins_val;

  /* [15:0] layer id */
  reg.layer_info = p->layer_id;
  return emit_tiu_cmdbuf(ctx, &reg);
}

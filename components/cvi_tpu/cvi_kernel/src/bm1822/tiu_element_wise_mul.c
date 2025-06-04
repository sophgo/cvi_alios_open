#include "kernel_1822.h"

bmk1822_op_t * bmk1822_tiu_element_wise_mul(
    ctx_t *ctx,
    const bmk1822_tiu_element_wise_mul_param_t *p)
{
  int bf16_enable = (p->res_low->fmt == FMT_BF16) ? 1 : 0;

  check_tiu_tensor_2(p->res_low, p->a);
  assert_same_shape(p->res_low, p->a);
  if (!p->b_is_const) {
    check_tiu_tensor(p->b);
    assert_same_shape(p->res_low, p->b);
  }
  if (p->res_high)
    check_16bit_tiu_tensor(p->res_low, p->res_high);
  ASSERT(p->relu_enable == 0 || p->relu_enable == 1);

  tiu_reg_t reg;
  reset_tiu_reg(&reg);

  reg.cmd_en = 1;
  reg.tsk_typ = DCR_TYPE_TENSOR_ARITH_FIX8B;
  reg.tsk_eu_typ = TENSOR_MUL_FIX8B;
  reg.tsk_opd_num = 2;
  reg.opd_typ = bf16_enable ? 1: 0;
  int arith_shift = tensor_is_signed(p->res_low);
  reg.opt_shift_typ = arith_shift;
  reg.opt_res_shift = p->rshift_bits;
  reg.opt_relu_typ = p->relu_enable;
  fill_same_tensor_shape(&reg, p->a->shape);
  fill_same_tensor_stride_type(&reg, 0b11);

  reg.opd0_addr = p->a->start_address;
  reg.opt_opd0_sign = tensor_is_signed(p->a);
  fill_opd0_stride(&reg, &p->a->stride);

  if (p->b_is_const) {
    reg.opt_opd1_const = 1;
    reg.opd1_addr = bf16_enable ? p->b_const.val : (p->b_const.val & 0xFF);
    reg.opt_opd1_sign = !!p->b_const.is_signed;
  } else {
    reg.opt_opd1_const = 0;
    reg.opd1_addr = p->b->start_address;
    reg.opt_opd1_sign = tensor_is_signed(p->b);
    fill_opd1_stride(&reg, &p->b->stride);
  }

  reg.res0_addr = p->res_low->start_address;
  reg.opt_res0_sign = tensor_is_signed(p->res_low);
  reg.opt_res0_seg = (p->res_high == NULL);
  fill_res0_stride(&reg, &p->res_low->stride);
  if (p->res_high)
    reg.res0_b_str = (p->res_high->start_address - p->res_low->start_address);
  if (p->relu_enable)
    ASSERT(reg.opt_res0_seg);

  ASSERT((
    p->b_is_const || (!reg.opt_opd1_sign && !reg.opt_opd0_sign && !reg.opt_shift_typ) ||
    ((reg.opt_opd1_sign || reg.opt_opd0_sign) && reg.opt_shift_typ)
  ));

  /* [15:0] layer id */
  reg.layer_info = p->layer_id;

  return emit_tiu_cmdbuf(ctx, &reg);
}

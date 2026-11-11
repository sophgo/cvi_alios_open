#include <cassert>
#include <iostream>
#include <cmath>
#include <runtime/kernel_function.hpp>

namespace cvi {
namespace runtime {

static constexpr uint32_t NPU_NUM = 32;

static cvk_tl_t *load_and_convert_to_bf16(cvk_context_t *cvk_ctx, cvk_tl_shape_t &shape,
                                          int x_base_ga_idx, uint64_t x_ga) {
  cvk_tl_t *input_x = cvk_ctx->ops->lmem_alloc_tensor(cvk_ctx, shape, CVK_FMT_BF16, 1);
  //std::cout << "load input shape: (" << shape.n << ", " << shape.c << ", "
  //                                   << shape.h << ", " << shape.w << ")\n";
  assert(input_x);

  cvk_tdma_g2l_tensor_copy_param_t p1;
  cvk_tg_t tg_x;
  tg_x.start_address = x_ga;
  tg_x.base_reg_index = x_base_ga_idx;
  tg_x.int8_rnd_mode = 0;
  tg_x.fmt = CVK_FMT_U8;
  tg_x.shape = {shape.n, shape.c, shape.h, shape.w};
  tg_x.stride = cvk_ctx->ops->tg_default_stride(cvk_ctx, tg_x.shape, tg_x.fmt);
  p1.src = &tg_x;
  p1.dst = input_x;
  cvk_ctx->ops->tdma_g2l_bf16_tensor_copy(cvk_ctx, &p1);

  return input_x;
}

static cvk_tl_t *broadcast_tensor_to_all_lane(cvk_context_t *cvk_ctx, cvk_tl_t *input_x) {
  assert(input_x->shape.n == 1);
  assert(input_x->shape.c == 1);

  cvk_tl_t reshape_x;
  reshape_x.start_address = input_x->start_address;
  reshape_x.fmt = input_x->fmt;
  reshape_x.shape = {1, 1, NPU_NUM, (uint32_t)(input_x->shape.h * input_x->shape.w)};
  reshape_x.stride =
      cvk_ctx->ops->tl_default_stride(cvk_ctx, reshape_x.shape, input_x->fmt, 1);
  reshape_x.stride.h = 0;

  cvk_tl_t broadcast_x;
  broadcast_x.start_address = input_x->start_address;
  broadcast_x.fmt = input_x->fmt;
  broadcast_x.shape = {1, NPU_NUM, 1, (uint32_t)(input_x->shape.h * input_x->shape.w)};
  broadcast_x.stride =
      cvk_ctx->ops->tl_default_stride(cvk_ctx, broadcast_x.shape, input_x->fmt, 1);

  cvk_tdma_l2l_tensor_copy_param_t p3;
  p3.src = &reshape_x;
  p3.dst = &broadcast_x;
  cvk_ctx->ops->tdma_l2l_bf16_tensor_copy(cvk_ctx, &p3);

  input_x->shape = broadcast_x.shape;
  input_x->stride =
      cvk_ctx->ops->tl_default_stride(cvk_ctx, input_x->shape, input_x->fmt, 1);
  return input_x;
}

static void convert_ps32_to_fp32(cvk_context_t *cvk_ctx, uint32_t la_addr, uint32_t n,
                                 uint32_t c, uint32_t h, uint32_t w) {
  assert(n == 2); // Exclude lower part
  assert((h == 1) && (w == 1) && "Only support h=1, w=1");

  uint32_t la_high = la_addr;
  cvk_tl_t tl_src;
  tl_src.start_address = la_high;
  tl_src.fmt = CVK_FMT_BF16;
  tl_src.shape = {1, c, 1, 1};
  tl_src.stride = cvk_ctx->ops->tl_default_stride(cvk_ctx, tl_src.shape, tl_src.fmt, 1);

  uint32_t la_low = la_addr + tl_src.stride.n;
  cvk_tl_t tl_dst;
  tl_dst.start_address = la_low + sizeof(uint16_t); // concat higher part
  tl_dst.fmt = CVK_FMT_BF16;
  tl_dst.shape = {1, c, 1, 1};
  tl_dst.stride = cvk_ctx->ops->tl_default_stride(cvk_ctx, tl_dst.shape, tl_dst.fmt, 1);

  cvk_tiu_copy_param_t param;
  param.src = &tl_src;
  param.dst = &tl_dst;
  cvk_ctx->ops->tiu_copy(cvk_ctx, &param);
}

static void store_fp32(cvk_context_t *cvk_ctx, int base_ga_idx, uint64_t ga_dst,
                       uint32_t la_src, uint32_t n, uint32_t c, uint32_t h, uint32_t w) {
  assert(n == 2); // Exclude lower part
  assert(h == 1 && w == 1);

  cvk_tl_t src;
  src.fmt = CVK_FMT_BF16;
  src.shape = {1, c, 1, 2};
  src.stride = cvk_ctx->ops->tl_default_stride(cvk_ctx, src.shape, src.fmt, 1);
  src.start_address = la_src + src.stride.n;

  cvk_tg_t dst;
  dst.fmt = CVK_FMT_BF16;
  dst.shape = {1, c, 1, 2};
  dst.stride = cvk_ctx->ops->tg_default_stride(cvk_ctx, dst.shape, dst.fmt);
  dst.base_reg_index = base_ga_idx;
  dst.start_address = ga_dst;

  cvk_tdma_l2g_tensor_copy_param_t param;
  param.src = &src;
  param.dst = &dst;
  cvk_ctx->ops->tdma_l2g_bf16_tensor_copy(cvk_ctx, &param);
}

CVI_RT_MEM runtimeJitEuclideanDistance(CVI_RT_HANDLE ctx, void *cvk_ctx,
                                       uint32_t records, uint32_t feature_size) {
  uint32_t c_step = 32 * NPU_NUM;

  int x_ga_base_reg_idx = 2;
  int y_ga_base_reg_idx = 3;
  int o_ga_base_reg_idx = 4;

  uint64_t x_ga = 0;
  uint64_t y_ga = 0;
  uint64_t o_ga = 0;

  cvk_tl_shape_t input_x_shape = {1, 1, 1, feature_size};
  cvk_tl_shape_t output_shape = {2, c_step, 1, 1};

  auto cvk = (cvk_context_t *)cvk_ctx;
  cvk_tl_t *output = cvk->ops->lmem_alloc_tensor(cvk, output_shape, CVK_FMT_BF16, 1);
  //std::cout << "output shape: (" << output->shape.n << ", " << output->shape.c << ", "
  //                               << output->shape.h << ", " << output->shape.w << ")\n";
  assert(output);

  auto input_x = load_and_convert_to_bf16(cvk, input_x_shape, x_ga_base_reg_idx, x_ga);
  input_x = broadcast_tensor_to_all_lane(cvk, input_x);

  for (uint32_t c_pos = 0; c_pos < records; c_pos += c_step) {
    cvk_tl_shape_t input_y_shape = {1, c_step, 1, feature_size};
    auto input_y = load_and_convert_to_bf16(cvk, input_y_shape, y_ga_base_reg_idx, y_ga + c_pos * feature_size);

    cvk_tl_t b;
    b.start_address = input_x->start_address;
    b.shape = input_y->shape;
    b.stride = input_y->stride;
    b.stride.c = 0;
    b.stride.n = 0;
    b.fmt = input_x->fmt;

    cvk_tiu_sub_param_t p1;
    p1.res_high = 0;
    p1.res_low = input_y;
    p1.a_high = 0;
    p1.a_low = input_y;
    p1.b_high = 0;
    p1.b_low = &b;
    p1.rshift_bits = 0;
    p1.layer_id = 0;
    cvk->ops->tiu_sub(cvk, &p1);

    cvk_tiu_depthwise_pt_convolution_param_t p2;
    p2.ofmap = output;
    p2.ifmap = input_y;
    p2.weight = input_y;
    p2.bias = nullptr;
    p2.ins_h = 0;
    p2.ins_w = 0;
    p2.ins_last_h = 0;
    p2.ins_last_w = 0;
    p2.pad_top = 0;
    p2.pad_bottom = 0;
    p2.pad_left = 0;
    p2.pad_right = 0;
    p2.stride_h = 1;
    p2.stride_w = 1;
    p2.dilation_h = 1;
    p2.dilation_w = 1;
    p2.relu_enable = false;
    p2.rshift_bits = 0;
    p2.ps32_mode = 2;
    p2.layer_id = 0;
    cvk->ops->tiu_pt_depthwise_convolution(cvk, &p2);

    uint32_t on, oc, oh, ow;
    on = output->shape.n;
    oc = output->shape.c;
    oh = output->shape.h;
    ow = output->shape.w;
    convert_ps32_to_fp32(cvk, output->start_address, on, oc, oh, ow);

    store_fp32(cvk, o_ga_base_reg_idx, o_ga + c_pos * sizeof(float), output->start_address,
               on, oc, oh, ow);

    cvk->ops->lmem_free_tensor(cvk, input_y);
  }

  cvk->ops->lmem_free_tensor(cvk, input_x);

  CVI_RT_MEM cmdbuf_mem;
  uint32_t size;
  auto cmdbuf = cvk->ops->acquire_cmdbuf(cvk, &size);
  int ret = CVI_RT_LoadCmdbuf(ctx, cmdbuf, size, 0, 0, false, &cmdbuf_mem);
  assert(ret == 0);
  cvk->ops->reset(cvk);
  return cmdbuf_mem;
}

}
}